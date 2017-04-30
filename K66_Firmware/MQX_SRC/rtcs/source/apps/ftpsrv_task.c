/*HEADER**********************************************************************
*
* Copyright 2013 Freescale Semiconductor, Inc.
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
*   This file contains an implementation of an FTP server tasks.
*
*
*END************************************************************************/

#include <ctype.h>
#include <string.h>
#include <rtcs.h>

#if MQX_USE_IO_OLD
#include <fio.h>
#else
#include <stdio.h>
#include <fs_supp.h>
#endif
#include "ftpsrv_msg.h"
#include "ftpsrv_prv.h"
#include <lwmsgq.h>

#define FTPSRV_SESSION_TASK_NAME "FTP server session"

extern const FTPSRV_COMMAND_STRUCT ftpsrv_commands[];

static void ftpsrv_session_task(void* param, void* creator);
static FTPSRV_SESSION_STRUCT* ftpsrv_ses_alloc(FTPSRV_STRUCT *server);
static void ftpsrv_ses_init(FTPSRV_STRUCT *server, FTPSRV_SESSION_STRUCT *session, const int sock);
static void ftpsrv_ses_free(FTPSRV_SESSION_STRUCT *session);
static int ftpsrv_read_cmd(FTPSRV_SESSION_STRUCT* session);
static void ftpsrv_process_cmd(FTPSRV_SESSION_STRUCT* session);
static void ftpsrv_ses_close(FTPSRV_SESSION_STRUCT* session);

/*TASK*-----------------------------------------------------------------
*
* Function Name    : ftpsrv_server_task
* Returned Value   : none
* Comments  :  FTP server.
*
*END*-----------------------------------------------------------------*/

void ftpsrv_server_task(void* init_ptr, void* creator)
{
    FTPSRV_SERVER_TASK_PARAM *server_params = (FTPSRV_SERVER_TASK_PARAM *) init_ptr;
    FTPSRV_STRUCT*            server;
    _mqx_uint                 res;

    server = ftpsrv_create_server(server_params->params);
    if (server == NULL)
    {
        RTCS_task_resume_creator(creator, (uint32_t)RTCS_ERROR);
        goto QUIT;
    }

    server->server_tid = _task_get_id();

    res = _lwsem_create(&server->ses_cnt, server->params.max_ses);
    if (res != MQX_OK)
    {
        RTCS_task_resume_creator(creator, (uint32_t)RTCS_ERROR);
        goto ERROR;
    }

    server_params->handle = (uint32_t) server;
    RTCS_task_resume_creator(creator, RTCS_OK);

    while (1) 
    {
        uint32_t connsock = 0;
        uint32_t new_sock = 0;

        /* limit number of opened sessions */
        _lwsem_wait(&server->ses_cnt);

        /* Get socket with incoming connection (IPv4 or IPv6) */
        connsock = ftpsrv_wait_for_conn(server);
        if (connsock == RTCS_SOCKET_ERROR)
        {
            _sched_yield();
            break;
        }

        new_sock = ftpsrv_accept(connsock);

        if (new_sock != RTCS_SOCKET_ERROR)
        {
            FTPSRV_SESSION_PARAM ses_param;
            int32_t              error;

            ses_param.server = server;
            ses_param.sock = new_sock;
            /* Try to create task for session */
            error = RTCS_task_create(FTPSRV_SESSION_TASK_NAME, server->params.server_prio, FTPSRV_SESSION_STACK_SIZE, ftpsrv_session_task, &ses_param);
            if (MQX_OK != error)
            {
                ftpsrv_abort(new_sock);
                _lwsem_post(&server->ses_cnt);
            }
        }
        else
        {
            _lwsem_post(&server->ses_cnt);
            /* We probably run out of sockets. Wait some time then try again to prevent session tasks resource starvation */
            _time_delay(150);
        }
    }
    ERROR:
    ftpsrv_destroy_server(server);
    _lwsem_destroy(&server->ses_cnt);
    server->server_tid = 0;
    QUIT:
    return;
} 

/*TASK*-----------------------------------------------------------------
*
* Function Name    : ftpsrv_session_task
* Returned Value   : none
* Comments  :  FTP server session.
*
*END*-----------------------------------------------------------------*/

static void ftpsrv_session_task(void* init_ptr, void* creator)
{
    FTPSRV_SESSION_STRUCT* session;
    FTPSRV_STRUCT*         server = ((FTPSRV_SESSION_PARAM*) init_ptr)->server;
    uint32_t               sock = ((FTPSRV_SESSION_PARAM*) init_ptr)->sock;
    uint32_t               i;
    _task_id               tid =  _task_get_id();

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
    
    /* Allocate session */
    session = ftpsrv_ses_alloc(server);

    if (session) 
    {
        server->session[i] = session;

        RTCS_task_resume_creator(creator, RTCS_OK);       
        ftpsrv_ses_init(server, session, sock);
        ftpsrv_send_msg(session, ftpsrvmsg_banner);
        
        /* Read and process commands */
        while (session->connected)
        {
            if (ftpsrv_read_cmd(session) != FTPSRV_OK)
            {
                _mem_zero(session->buffer, FTPSRV_BUF_SIZE);
                continue;
            }
            ftpsrv_process_cmd(session);
            _sched_yield();
        }
        /* cleanup session */
        ftpsrv_ses_close(session);
        ftpsrv_ses_free(session);
        server->session[i] = NULL;
    }
    else 
    {
        RTCS_task_resume_creator(creator, (uint32_t) RTCS_ERROR);
    }

    /* Cleanup and end task */
    _lwsem_post(&server->ses_cnt);
    /* Null tid => task is no longer running */
    _lwsem_wait(&server->tid_sem);
    server->ses_tid[i] = 0;
    _lwsem_post(&server->tid_sem);
}

/* Task for reading/writing file */
void ftpsrv_transfer_task(void* init_ptr, void* creator)
{
    FTPSRV_TRANSFER_PARAM* param = (FTPSRV_TRANSFER_PARAM*) init_ptr;
    #if MQX_USE_IO_OLD
    MQX_FILE_PTR           file = param->file;
    #else
    FILE *                 file = param->file;
    #endif
    uint32_t               mode = param->mode;
    uint32_t               sock = param->sock;
    FTPSRV_SESSION_STRUCT* session = param->session;
    void*                  data_buffer;
    char*                  msg_str;

    if (session->state == FTPSRV_STATE_TRANSFER)
    {
        RTCS_task_resume_creator(creator, (uint32_t) RTCS_ERROR);
        return;
    }

    data_buffer = RTCS_mem_alloc(FTPSRVCFG_DATA_BUFFER_SIZE);
    if (data_buffer == NULL)
    {
        RTCS_task_resume_creator(creator, (uint32_t) RTCS_ERROR);
        return;
    }

    session->transfer_tid = _task_get_id();
    session->state = FTPSRV_STATE_TRANSFER;

    RTCS_task_resume_creator(creator, (uint32_t) RTCS_OK);
    
    while (1)
    {
        FTPSRV_TRANSFER_MSG message;
        _mqx_uint           retval;

        /* Reading from file */
        if (mode == FTPSRV_MODE_READ)
        {
            uint32_t length;
            int32_t  error;

            length = fread((void*) data_buffer, 1, FTPSRVCFG_DATA_BUFFER_SIZE, file);
            if ((length != FTPSRVCFG_DATA_BUFFER_SIZE) && ferror(file) && !feof(file))
            {
                msg_str = (char*) ftpsrvmsg_locerr;
                break;
            }
            error = send(sock, data_buffer, length, 0);
            if (error == RTCS_ERROR)
            {
                msg_str = (char*) ftpsrvmsg_locerr;
                break;
            }

            if (feof(file))
            {
                msg_str = (char*) ftpsrvmsg_trans_complete;
                break;
            }
        }
        /* Writing to file */
        else if ((mode == FTPSRV_MODE_WRITE) || (mode == FTPSRV_MODE_APPEND))
        {
            uint32_t length;
            int32_t  received;

            received = recv(sock, data_buffer, FTPSRVCFG_DATA_BUFFER_SIZE, 0);
            if (received == RTCS_ERROR)
            {
                msg_str = (char*) ftpsrvmsg_trans_complete;
                break;
            }
            
            length = fwrite((void*) data_buffer, 1, received, file);
            if (length != received)
            {
                #if MQX_USE_IO_OLD
                if (length == (uint32_t)IO_ERROR)
                #else
                if((ferror(file)) || (feof(file)))            
                #endif
                {
                    msg_str = (char*) ftpsrvmsg_no_space;
                }
                else
                {
                    msg_str = (char*) ftpsrvmsg_writefail;
                }
                break;
            }
        }
        retval = _lwmsgq_receive((void*) session->msg_queue, (uint32_t *)&message, LWMSGQ_TIMEOUT_FOR, 1, NULL);
        if (retval != MQX_OK)
        {
            continue;
        }
        if (message.command == FTPSRV_CMD_ABORT)
        {
            msg_str = (char*) ftpsrvmsg_trans_abort;
            break;
        }
        else if (message.command == FTPSRV_CMD_STAT)
        {
            /* TODO: Add code to print out transfer statistics */
        }
    }
    _mem_free(data_buffer);
    fflush(file);
    fclose(file);
    ftpsrv_send_msg(session, msg_str);
    closesocket(sock);
    session->state = FTPSRV_STATE_IDLE;
}

/* Read command from control socket */
static int ftpsrv_read_cmd(FTPSRV_SESSION_STRUCT* session)
{
    int32_t  result;
    uint32_t sock = session->control_sock;
    uint32_t received = 0;
    char*    end;
    char*    buffer = session->buffer;
    uint32_t option;
    uint32_t error;

    /* Wait 250ms for command and then check if session is still connected. */
    option = 250;
    error = setsockopt(sock, SOL_TCP, OPT_RECEIVE_TIMEOUT, &option, sizeof(option));
    if (error != RTCS_OK)
    {
        return(FTPSRV_ERROR);
    }

    end = strstr(buffer, "\r\n");
    while ((received < FTPSRV_BUF_SIZE) && (!end) && session->connected)
    {
        end = strstr(buffer, "\r\n");
        result = recv(sock, buffer+received, FTPSRV_BUF_SIZE-received, 0);
        if (result == RTCS_ERROR)
        {
            if (RTCS_geterror(sock) == RTCSERR_TCP_TIMED_OUT)
            {
                continue;
            }
            session->connected = FALSE;
            return(FTPSRV_ERROR);
        }
        received += result;
    }

    /* Null-terminate received data */
    if (end != NULL)
    {
        end[0] = '\0';
    }
    else
    {
        return(FTPSRV_ERROR);
    }

    /* Find a command and store it */
    session->command = strtok(buffer, " ");
    if (session->command == NULL)
    {
        return(FTPSRV_ERROR);
    }
    /* Find a parameter and store it */
    session->cmd_arg = strtok(NULL, " ");

    /* Restore socket to wait forever for data. */
    option = 0;
    error = setsockopt(sock, SOL_TCP, OPT_RECEIVE_TIMEOUT, &option, sizeof(option));
    if (error != RTCS_OK)
    {
        return(FTPSRV_ERROR);
    }
    return(FTPSRV_OK);
}

/* Run command, check authentication first */
static void ftpsrv_process_cmd(FTPSRV_SESSION_STRUCT* session)
{
    const FTPSRV_COMMAND_STRUCT* row = ftpsrv_commands;
    char* cp = session->command;
    uint32_t auth_valid;

    /* Convert command to upper case */
    while (*cp)
    {
        *cp = toupper((int) *cp);
        cp++;
    }

    /* Find function corresponding to command */
    while ((row->command != NULL) && strncmp(session->command, row->command, strlen(row->command)))
    {
        row++;
    }
    
    /* Process authentication sequence if required */
    auth_valid = ftpsrv_process_auth(session, row->auth_req);
    
    /* Run command if authentication is valid */
    if (auth_valid)
    {
        if (row->function != NULL)
        {
            row->function(session);
        }
        else
        {
            session->message = (char*) ftpsrvmsg_unimp;
        }
    }
    else
    {
        if (session->message == NULL)
        {
            session->message = (char*) ftpsrvmsg_not_logged;
        }
        if (session->auth_input.uid != NULL)
        {
            _mem_free(session->auth_input.uid);
            session->auth_input.uid = NULL;
        }
    }

    ftpsrv_send_msg(session, session->message);
    session->message = NULL;
    _mem_zero(session->buffer, FTPSRV_BUF_SIZE);
}

/*
** Function for session allocation
**
** IN:
**      FTPSRV_STRUCT *server - pointer to server structure (needed for session parameters).
**
** OUT:
**      none
**
** Return Value: 
**      FTPSRV_SESSION_STRUCT* - pointer to allocated session. Non-zero if allocation was OK, NULL otherwise
*/
static FTPSRV_SESSION_STRUCT* ftpsrv_ses_alloc(FTPSRV_STRUCT *server)
{
    FTPSRV_SESSION_STRUCT *session = NULL;

    if (server)
    {
        session = RTCS_mem_alloc_zero(sizeof(FTPSRV_SESSION_STRUCT));
        if (session)
        {
            session->buffer = RTCS_mem_alloc_zero(sizeof(char)*FTPSRV_BUF_SIZE);
        }
    }

    if (session && (session->buffer == NULL))
    {
        _mem_free(session);
        session = NULL;
    }
    return session;
}

/*
** Function used to free session structure
**
** IN:
**      FTPSRV_SESSION_STRUCT* session - session structure pointer
**
** OUT:
**      none
**
** Return Value: 
**      none
*/
static void ftpsrv_ses_free(FTPSRV_SESSION_STRUCT *session)
{
    if (session)
    {
        if(session->buffer)
        {
            _mem_free(session->buffer);
            session->buffer = NULL;
        }
        if(session->auth_input.uid)
        {
            _mem_free(session->auth_input.uid);
            session->auth_input.uid = NULL;
        }
        if(session->auth_input.pass && strcmp(session->auth_input.pass, ""))
        {
            _mem_free(session->auth_input.pass);
            session->auth_input.pass = NULL;
        }
        if(session->cur_dir)
        {
            _mem_free(session->cur_dir);
        }
        if (session->msg_queue != NULL)
        {
            _lwmsgq_deinit(session->msg_queue);
            _mem_zero(session->msg_queue, sizeof(LWMSGQ_STRUCT) + FTPSRV_NUM_MESSAGES*sizeof(FTPSRV_TRANSFER_MSG)*sizeof(_mqx_max_type));
            _mem_free(session->msg_queue);
        }
        _mem_free(session);
    }
}

/*
** Function used to init session structure
**
** IN:
**      FTPSRV_SESSION_STRUCT* session - session structure pointer
**      FTPSRV_STRUCT *server - pointer to server structure (needed for session parameters)
**      const int sock - socket handle used for communication with client
**
** OUT:
**      none
**
** Return Value: 
**      none
*/
static void ftpsrv_ses_init(FTPSRV_STRUCT *server, FTPSRV_SESSION_STRUCT *session, const int sock)
{
    if (server && session)
    {
        /* Some buffer of arbitrary size so we can get filesystem pointer */
        char      dev_name[16] = {0};

        /* Init session structure */
        session->control_sock = sock;
        session->connected = TRUE;
        session->auth_tbl = server->params.auth_table;
        session->root_dir = (char*) server->params.root_dir;
        session->cur_dir = RTCS_mem_alloc_zero(sizeof("\\"));
        session->cur_dir[0] = '\\';
        session->start_time = RTCS_time_get();

        _io_get_dev_for_path(dev_name, NULL, 16, session->root_dir, NULL);
        session->fs_ptr = _io_get_fs_by_name(dev_name);
        session->msg_queue = RTCS_mem_alloc_zero(sizeof(LWMSGQ_STRUCT) + FTPSRV_NUM_MESSAGES*sizeof(FTPSRV_TRANSFER_MSG)*sizeof(_mqx_max_type));
        if (session->msg_queue != NULL)
        {
            _lwmsgq_init(session->msg_queue, FTPSRV_NUM_MESSAGES, sizeof(FTPSRV_TRANSFER_MSG)/sizeof(_mqx_max_type));
        }
    }
}

/*
** Function used to close session
**
** IN:
**      FTPSRV_SESSION_STRUCT* session - session structure pointer
**
** OUT:
**      none
**
** Return Value: 
**      none
*/
static void ftpsrv_ses_close(FTPSRV_SESSION_STRUCT *session)
{
    if (session != NULL)
    {
        ftpsrv_abor(session);
        session->connected = FALSE;
        closesocket(session->control_sock);
        closesocket(session->data_sock);
    }
}

