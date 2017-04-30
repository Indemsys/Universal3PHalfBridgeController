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
*   This file contains the Telnet server implementation.
*
*
*END************************************************************************/

#include <rtcs.h>
#include "telnetsrv_prv.h"

#if MQX_USE_IO_OLD
    #include <fio.h>
    #include <serial.h>
#else
    #include <mqx_inc.h>
    #include <stdio.h>
    #include <nio.h>
    #include <nio/ioctl.h>

    #if PLATFORM_SDK_ENABLED
        #include <nio/drivers/nio_tty/nio_tty.h>
        #define IO_SERIAL_ECHO NIO_TTY_FLAGS_ECHO
    #else
        #include <ntty.h>
        #define IO_SERIAL_ECHO NTTY_FLAGS_ECHO
    #endif
#endif

#include "telnet.h"

#define TELNETSRV_SESSION_TASK_NAME "Telnet server session"

static void telnetsrv_session_task(void *init_ptr, void *creator);
static TELNETSRV_SESSION_STRUCT* telnetsrv_ses_alloc(TELNETSRV_STRUCT *server);
static void telnetsrv_ses_free(TELNETSRV_SESSION_STRUCT *session);
static void telnetsrv_ses_close(TELNETSRV_SESSION_STRUCT *session);
static uint32_t telnetsrv_ses_init(TELNETSRV_STRUCT *server, TELNETSRV_SESSION_STRUCT *session, const int sock);
static void telnetsrv_process(void *server, void *session);

/*TASK*-----------------------------------------------------------------
*
* Function Name : telnetsrv_server_task
* Returned Value: none
* Comments      : Telnet server  main task which creates new task for each new
*              client request
*
*END*-----------------------------------------------------------------*/
void telnetsrv_server_task(void *init_ptr, void *creator) 
{
    TELNETSRV_STRUCT            *server;
    _mqx_uint                   res;
    TELNETSRV_SERVER_TASK_PARAM *server_params;

    server_params = (TELNETSRV_SERVER_TASK_PARAM *) init_ptr;

    server = telnetsrv_create_server(server_params->params);
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
    
    while (1) 
    {
        uint32_t connsock = 0;
        uint32_t new_sock = 0;

        /* limit number of opened sessions */
        _lwsem_wait(&server->ses_cnt);

        /* Get socket with incoming connection (IPv4 or IPv6) */
        connsock = telnetsrv_wait_for_conn(server);
        if (connsock == RTCS_SOCKET_ERROR)
        {
            break;
        }

        new_sock = telnetsrv_accept(connsock);

        if (new_sock != RTCS_SOCKET_ERROR)
        {
            TELNETSRV_SES_TASK_PARAM  ses_param;
            _mqx_uint                 error;

            ses_param.server = server;
            ses_param.sock = new_sock;
            /* Try to create task for session */
            error = RTCS_task_create(TELNETSRV_SESSION_TASK_NAME, server->params.server_prio, TELNETSRV_SESSION_STACK_SIZE, telnetsrv_session_task, &ses_param);
            if (MQX_OK != error)
            {
                telnetsrv_abort(new_sock);
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
    telnetsrv_destroy_server(server);
    _lwsem_destroy(&server->ses_cnt);
    server->server_tid = 0;
    
    QUIT:
    return;
}

/*TASK*-----------------------------------------------------------------
*
* Task Name      : telnetsrv_session_task
* Returned Value : void
* Comments       : Telnet child task.
*
*END*-----------------------------------------------------------------*/
static void telnetsrv_session_task(void *init_ptr, void *creator)
{
    TELNETSRV_STRUCT* server = ((TELNETSRV_SES_TASK_PARAM*) init_ptr)->server;
    uint32_t sock = ((TELNETSRV_SES_TASK_PARAM*) init_ptr)->sock;
    TELNETSRV_SESSION_STRUCT *session;
    uint32_t i;
    _task_id tid = _task_get_id();

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
    session = telnetsrv_ses_alloc(server);
        
    if (session)
    {
        server->session[i] = session;
       
        if (telnetsrv_ses_init(server, session, sock) != TELNETSRV_OK)
        {
            RTCS_task_resume_creator(creator, (uint32_t) RTCS_ERROR);
            _lwsem_post(&server->ses_cnt);
            return;
        }
        else
        {
            RTCS_task_resume_creator(creator, RTCS_OK);
        }
    
        while (session->valid) 
        {
            /* Run state machine for session */
            session->process_func(server, session);
            _sched_yield();
        }
        telnetsrv_ses_close(session);
        telnetsrv_ses_free(session);
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

#if MQX_USE_IO_OLD
static uint32_t telnetsrv_ses_init(TELNETSRV_STRUCT *server, TELNETSRV_SESSION_STRUCT *session, const int sock)
{
    MQX_FILE_PTR     sockfd;
    MQX_FILE_PTR     telnetfd;
    uint32_t         option;

    option = 0;
    sockfd = fopen("socket:", (char *) sock);

    if (sockfd == NULL)
    {
        return(TELNETSRV_ERR);
    }

    ioctl(sockfd, IO_IOCTL_SET_BLOCK_MODE, &option);
   
    telnetfd = fopen("telnet:", (char *)sockfd);

    if (telnetfd == NULL)
    {
        return(TELNETSRV_ERR);
    } 

    option = IO_SERIAL_ECHO;
    ioctl(telnetfd, IO_IOCTL_SERIAL_SET_FLAGS, &option);

    _io_set_handle(IO_STDIN, telnetfd);
    _io_set_handle(IO_STDOUT, telnetfd);

    session->sock = sock;
    session->sockfd = sockfd;
    session->telnetfd = telnetfd;
    session->valid = 1;
    session->process_func = telnetsrv_process;
    return(TELNETSRV_OK);
}

static void telnetsrv_process(void *server_ptr, void *session_ptr)
{
    TELNETSRV_SESSION_STRUCT *session;
    TELNETSRV_STRUCT         *server;

    session = (TELNETSRV_SESSION_STRUCT *) session_ptr;
    server = (TELNETSRV_STRUCT *) server_ptr;
    /*
    ** Send a greeting message to the user
    */
    printf(TELNETRSV_WELCOME_STRING, RTCS_VERSION_MAJOR, RTCS_VERSION_MINOR, RTCS_VERSION_REV);

    /*
    ** Run shell
    */
    if (server->params.shell != NULL)
    {
        server->params.shell(server->params.shell_commands, NULL);
    }
    /* Cleanup and end task */
    printf(TELNETSRV_GOODBYE_STRING);
    session->valid = 0;
}

#else

static uint32_t telnetsrv_ses_init(TELNETSRV_STRUCT *server, TELNETSRV_SESSION_STRUCT *session, const int sock)
{
    register KERNEL_DATA_STRUCT_PTR  kernel_data;
    register TD_STRUCT_PTR           active_ptr;
    FILE                             *sockfd;
    FILE                             *telnetfd;
    uint32_t                         option;
    char                             dev_name[20] = {0};

    _GET_KERNEL_DATA(kernel_data);
    active_ptr = kernel_data->ACTIVE_PTR;

    snprintf(dev_name, sizeof(dev_name), "socket:%i", sock);
    sockfd = fopen(dev_name, "r+");
    if (sockfd == NULL)
    {
        return(TELNETSRV_ERR);
    }

    ioctl(fileno(sockfd), IO_IOCTL_SET_BLOCK_MODE, &option);

    memset(dev_name, 0, sizeof(dev_name));
    snprintf(dev_name, sizeof(dev_name), "telnet:%i", sockfd);
    telnetfd = fopen(dev_name, "r+");

    if (telnetfd == NULL)
    {
        return(TELNETSRV_ERR);
    } 

    option = IO_SERIAL_ECHO;
    ioctl(fileno(telnetfd), IO_IOCTL_SERIAL_SET_FLAGS, &option);

    active_ptr->STDIN_STREAM = telnetfd;
    active_ptr->STDOUT_STREAM = telnetfd;

    session->sock = sock;
    session->sockfd = sockfd;
    session->telnetfd = telnetfd;
    session->valid = 1;
    session->process_func = telnetsrv_process;
    return(TELNETSRV_OK);
}

static void telnetsrv_process(void *server_ptr, void *session_ptr)
{
    TELNETSRV_SESSION_STRUCT *session;
    TELNETSRV_STRUCT         *server;
    
    server = (TELNETSRV_STRUCT *) server_ptr;
    session = (TELNETSRV_SESSION_STRUCT *) session_ptr;
    /*
    ** Send a greeting message to the user
    */
    fprintf(session->telnetfd, TELNETRSV_WELCOME_STRING, RTCS_VERSION_MAJOR, RTCS_VERSION_MINOR, RTCS_VERSION_REV);

    /*
    ** Run shell
    */
    if (server->params.shell != NULL)
    {
        server->params.shell(server->params.shell_commands, NULL);
    }

    /* Cleanup and end task */
    fprintf(session->telnetfd, TELNETSRV_GOODBYE_STRING);
    session->valid = 0;
}

#endif

static void telnetsrv_ses_close(TELNETSRV_SESSION_STRUCT *session)
{
    if (session->telnetfd != 0)
    {
        fclose(session->telnetfd);
    }
    if (session->sockfd != 0)
    {
        fclose(session->sockfd);
    }
    closesocket(session->sock); 
}

static TELNETSRV_SESSION_STRUCT* telnetsrv_ses_alloc(TELNETSRV_STRUCT *server)
{
    TELNETSRV_SESSION_STRUCT *session = NULL;

    if (server)
    {
        session = _mem_alloc_zero(sizeof(TELNETSRV_SESSION_STRUCT));
        if (session)
        {
            _mem_set_type(session, MEM_TYPE_TELNETSRV_SESSION_STRUCT);
        }
    }

    return(session);
}

static void telnetsrv_ses_free(TELNETSRV_SESSION_STRUCT *session)
{
    if (session)
    {
        _mem_free(session);
    }
}
