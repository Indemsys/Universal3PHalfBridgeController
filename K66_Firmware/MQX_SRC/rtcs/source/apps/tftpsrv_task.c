/*HEADER**********************************************************************
*
* Copyright 2014 Freescale Semiconductor, Inc.
*
* This software is owned or controlled by Freescale Semiconductor.
* Use of this software is governed by the Freescale MQX RTOS License
* distributed with this Material.
* See the MQX_RTOS_LICENSE file distributed for more details.
*
* Brief License Summary:
* This software is provided in source form for you to use free of charge,
* but it is not open source software. You are allowed to use this software
* but you cannot redistribute it or derivative works of it in source form.
* The software may be used only in connection with a product containing
* a Freescale microprocessor, microcontroller, or digital signal processor.
* See license agreement file for full license terms including other
* restrictions.
*****************************************************************************
*
* Comments:
*
*   This file contains the TFTP server implementation.
*
*
*END************************************************************************/

#include "tftp_prv.h"
#include "tftpsrv_prv.h"

#if MQX_USE_IO_OLD
#include <fio.h>
#else
#include <stdio.h>
#include <nio.h>
#endif

#define TFTPSRV_SESSION_TASK_NAME "TFTP server session"

static void tftpsrv_session_task(void *init_ptr, void *creator);
static void tftpsrv_ses_close(TFTPSRV_SESSION_STRUCT *session);
static TFTPSRV_SESSION_STRUCT* tftpsrv_ses_alloc(TFTPSRV_STRUCT *server);
static void tftpsrv_ses_free(TFTPSRV_SESSION_STRUCT *session);
static uint32_t tftpsrv_ses_init(TFTPSRV_STRUCT* server, TFTPSRV_SESSION_STRUCT *session, uint32_t sock);
static uint32_t tftpsrv_wait_for_req(TFTPSRV_STRUCT *server);
static void tftpsrv_process_transaction(void* server, void *session);

/*
 * Server task - listening for incoming connections and starting sessions.
 */
void tftpsrv_server_task(void *init_ptr, void *creator)
{
    TFTPSRV_STRUCT            *server;
    uint32_t                  res;
    TFTPSRV_SERVER_TASK_PARAM *server_params;

    server_params = (TFTPSRV_SERVER_TASK_PARAM *) init_ptr;

    server = tftpsrv_create_server(server_params->params);
    if (server == NULL)
    {
        RTCS_task_resume_creator(creator, (uint32_t)RTCS_ERROR);
        goto QUIT;
    }
    
    server->server_tid = _task_get_id();
    
    res = _lwsem_create(&server->ses_cnt, server->params.max_ses);  
    if (res != MQX_OK)
    {
        goto ERROR;
    }
    
    server_params->handle = (uint32_t) server;
    RTCS_task_resume_creator(creator, RTCS_OK);

    while(1)
    {
        uint32_t connsock = 0;

        /* limit number of opened sessions */
        _lwsem_wait(&server->ses_cnt);

        /* Get socket with incoming connection (IPv4 or IPv6) */
        connsock = tftpsrv_wait_for_req(server);

        if (connsock != RTCS_SOCKET_ERROR)
        {
            TFTPSRV_SES_TASK_PARAM  ses_param;
            uint32_t                 error;

            ses_param.server = server;
            ses_param.sock = connsock;
            
            /* Try to create task for session */
            error = RTCS_task_create(TFTPSRV_SESSION_TASK_NAME, server->params.server_prio, TFTPSRV_SESSION_STACK_SIZE, tftpsrv_session_task, &ses_param);
            if (MQX_OK != error)
            {
                _lwsem_post(&server->ses_cnt);
            }
        }
        else
        {
            _lwsem_post(&server->ses_cnt);
            /* 
             * tftpsrv_wait_for_req() returned error, that indicates socket 
             * shutdown so server will terminate.
             */
            break;
        }
    }
    ERROR:
    tftpsrv_destroy_server(server);
    _lwsem_destroy(&server->ses_cnt);
    server->server_tid = 0;
    
    QUIT:
    return;
}

/*
 * Wait for incoming request server sockets, return socket with activity.
 */
static uint32_t tftpsrv_wait_for_req(TFTPSRV_STRUCT *server)
{
    rtcs_fd_set       rfds;
    int32_t           retval;

    RTCS_FD_ZERO(&rfds);
    RTCS_FD_SET(server->sock_v4, &rfds);
    RTCS_FD_SET(server->sock_v6, &rfds);

    retval = select(2, &rfds, NULL, NULL, 0);
    if (retval == RTCS_ERROR)
    {
        retval = RTCS_SOCKET_ERROR;
        goto EXIT;
    }

    if(RTCS_FD_ISSET(server->sock_v4, &rfds))
    {
        retval = server->sock_v4;
    }
    else if(RTCS_FD_ISSET(server->sock_v6, &rfds))
    {
        retval = server->sock_v6;
    }
    EXIT:
    return(retval);
}

/*
 * Session task for request processing.
 */
static void tftpsrv_session_task(void *init_ptr, void *creator)
{
    TFTPSRV_STRUCT*        server;
    uint32_t               sock;
    TFTPSRV_SESSION_STRUCT *session;
    uint32_t               i;
    _task_id               tid;

    sock = ((TFTPSRV_SES_TASK_PARAM*) init_ptr)->sock;
    server = ((TFTPSRV_SES_TASK_PARAM*) init_ptr)->server;
    tid = _task_get_id();

    /* Find empty session */
    _lwsem_wait(&server->tid_sem);

    for (i = 0; i < server->params.max_ses; i++)
    {
        if (server->session[i] == NULL)
        {
            break;
        }
    }
    
    if (i == server->params.max_ses)
    {
        RTCS_task_resume_creator(creator, (uint32_t) RTCS_ERROR);
        _lwsem_post(&server->tid_sem);
        _lwsem_post(&server->ses_cnt);
        return;
    }
  
    /* Save task ID - used for indication of running task */
    server->ses_tid[i] = tid;
    /* Access to array done. Unblock other tasks. */
    _lwsem_post(&server->tid_sem);
    
    /* Create session */
    session = tftpsrv_ses_alloc(server);
        
    if (session)
    {
        server->session[i] = session;
       
        if (tftpsrv_ses_init(server, session, sock) != TFTPSRV_OK)
        {
            RTCS_task_resume_creator(creator, (uint32_t) RTCS_ERROR);
            _lwsem_post(&server->ses_cnt);
            return;
        }
        else
        {
            RTCS_task_resume_creator(creator, (uint32_t) RTCS_OK);
        }
    
        while (session->valid) 
        {
            /* Run state machine for session */
            session->process_func(server, session);
            _sched_yield();
        }
        tftpsrv_ses_close(session);
        tftpsrv_ses_free(session);
        server->session[i] = NULL;
    }
    else 
    {
        RTCS_task_resume_creator(creator, (uint32_t) RTCS_ERROR);
    }

    /* Null tid => task is no longer running */
    _lwsem_wait(&server->tid_sem);
    server->ses_tid[i] = 0;
    _lwsem_post(&server->tid_sem);
    /* Cleanup and end task */
    _lwsem_post(&server->ses_cnt);
}

/* 
 * Close session
 */
static void tftpsrv_ses_close(TFTPSRV_SESSION_STRUCT *session)
{
    if (session->trans.file)
    {
        fflush(session->trans.file);
        fclose(session->trans.file);
        session->trans.file = NULL;
    }
    shutdown(session->trans.sock, FLAG_CLOSE_TX); 
}

/* 
 * Allocate required memory
 */
static TFTPSRV_SESSION_STRUCT* tftpsrv_ses_alloc(TFTPSRV_STRUCT *server)
{
    TFTPSRV_SESSION_STRUCT *session = NULL;

    if (server)
    {
        session = _mem_alloc_zero(sizeof(TFTPSRV_SESSION_STRUCT));
        if (session)
        {
            _mem_set_type(session, MEM_TYPE_TFTPSRV_SESSION_STRUCT);
            session->trans.buffer = _mem_alloc_zero(TFTP_MAX_MESSAGE_SIZE);
            if (session->trans.buffer != NULL)
            {
                _mem_set_type(session->trans.buffer, MEM_TYPE_TFTPSRV_SESSION_BUFFER);
            }
            else
            {
                _mem_free(session);
                session = NULL;
            }
        }
    }

    return(session);
}

static void tftpsrv_ses_free(TFTPSRV_SESSION_STRUCT *session)
{
    if (session != NULL)
    {
        if (session->trans.buffer != NULL)
        {
            _mem_free(session->trans.buffer);
            session->trans.buffer = NULL;
        }
        _mem_free(session);
    }
}

/*
 * Initialize session. Create socket for transmission, set socket options, etc. 
 */
static uint32_t tftpsrv_ses_init(TFTPSRV_STRUCT* server, TFTPSRV_SESSION_STRUCT *session, uint32_t sock)
{
    uint16_t family;
    sockaddr remote_sin = {0};
    uint16_t size;
    int32_t  retval;

    size = sizeof(remote_sin);

    /* Read first request, find out information about remote host. */
    retval = recvfrom(sock, session->trans.buffer, TFTP_MAX_MESSAGE_SIZE, 0, &remote_sin, &size);
    if (retval == RTCS_ERROR)
    {
        goto FAIL;
    }
    family = remote_sin.sa_family;

    /* Create socket for data transfer. */
    session->trans.sock = socket(family, SOCK_DGRAM, 0);
    if (session->trans.sock == RTCS_ERROR)
    {
        goto FAIL;
    }

    /* Set remote host for new socket. */
    if (connect(session->trans.sock, &remote_sin, sizeof(remote_sin)) != RTCS_OK)
    {
        goto FAIL;
    }

    session->trans.remote_sa = remote_sin;
    session->valid = 1;
    session->process_func = tftpsrv_transaction_prologue;
    session->trans.recv_timeout = TFTP_RC_INIT;
    return(TFTPSRV_OK);
    FAIL:
    return(TFTPSRV_ERR);
}

/*
 * Prologue for transaction processing - first request handling.
 */
void tftpsrv_transaction_prologue(void* server_v, void *session_v)
{
    TFTPSRV_SESSION_STRUCT *session;
    char                   *buffer;
    uint16_t               opcode;
    char                   *filename;
    char                   *mode;
    uint32_t               size;
    int32_t                error;
    char                   *root_dir;
    TFTPSRV_STRUCT         *server;
    
    server = (TFTPSRV_STRUCT *) server_v;
    session = (TFTPSRV_SESSION_STRUCT *) session_v;
    buffer = session->trans.buffer;

    opcode = ntohs(*((uint16_t*) buffer));
    filename = buffer+sizeof(uint16_t);
    size = (strlen(filename) < TFTPSRV_FILENAME_MAX_LENGTH) ? strlen(filename) : TFTPSRV_FILENAME_MAX_LENGTH;
    mode = filename+size+1;
    root_dir = server->params.root_dir;

    /* Open requested file with correct mode. */
    error = tftpsrv_open_file(opcode, filename, root_dir, mode, &session->trans.file);
    if (error != TFTPSRV_OK)
    {
        tftp_send_error(&session->trans, error);
        session->process_func = tftpsrv_transaction_epilogue;
    }
    else
    {
        session->process_func = tftpsrv_process_transaction;
    }

    if (opcode == TFTP_OPCODE_RRQ)
    {
        session->trans.state = TFTP_STATE_SEND_DATA;
        session->trans.n_block = 1;
    }
    else if (opcode == TFTP_OPCODE_WRQ)
    {
        session->trans.state = TFTP_STATE_SEND_ACK;
        session->trans.n_block = 0;
    }
}

/*
 * Epilogue for transaction processing.
 */
void tftpsrv_transaction_epilogue(void* server_v, void *session_v)
{
    TFTPSRV_SESSION_STRUCT *session;
    
    session = (TFTPSRV_SESSION_STRUCT *) session_v;
    session->valid = 0;
}

/*
 * Process transaction. Called in loop from session task.
 */
static void tftpsrv_process_transaction(void* server_v, void *session_v)
{
    TFTPSRV_SESSION_STRUCT *session;
    int32_t                data_length;

    session = (TFTPSRV_SESSION_STRUCT *) session_v;

    switch(session->trans.state)
    {
        case TFTP_STATE_SEND_DATA:
            data_length = tftp_send_data(&session->trans);
            if (data_length == RTCS_ERROR)
            {
                session->process_func = tftpsrv_transaction_epilogue;
            }
            else if (data_length > 0)
            {
                session->trans.state = TFTP_STATE_READ_ACK;
            }
            break;
        case TFTP_STATE_READ_DATA:
            data_length = tftp_recv_data(&session->trans);
            if (data_length == RTCS_ERROR)
            {
                session->process_func = tftpsrv_transaction_epilogue;
            }
            else if (data_length >= 0)
            {
                session->trans.state = TFTP_STATE_SEND_ACK;
            }
            break;
        case TFTP_STATE_READ_ACK:
            data_length = tftp_recv_ack(&session->trans);
            if ((data_length == RTCS_ERROR) || session->trans.last)
            {
                session->process_func = tftpsrv_transaction_epilogue;
            }
            else if (data_length >= 0)
            {
                session->trans.state = TFTP_STATE_SEND_DATA;
            }
            break;
        case TFTP_STATE_SEND_ACK:
            data_length = tftp_send_ack(&session->trans);
            if ((data_length == RTCS_ERROR) || session->trans.last)
            {
                session->process_func = tftpsrv_transaction_epilogue;
            }
            else if (data_length > 0)
            {
                session->trans.state = TFTP_STATE_READ_DATA;
            }
            break;
        default:
            break;
    }
}
