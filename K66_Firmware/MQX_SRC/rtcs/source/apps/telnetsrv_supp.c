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
*   This file contains the Telnet server support functions.
*
*
*END************************************************************************/

#include "telnetsrv_prv.h"
#if !MQX_USE_IO_OLD
#include <stdio.h>
#include <nio.h>
#endif

#if !MQX_USE_IO_OLD
    extern const NIO_DEV_FN_STRUCT _io_telnet_dev_fn;
    extern const NIO_DEV_FN_STRUCT _io_socket_dev_fn;
#endif

static uint32_t telnetsrv_init_socket(TELNETSRV_STRUCT *server, uint16_t family);
static uint32_t telnetsrv_set_params(TELNETSRV_STRUCT *server, TELNETSRV_PARAM_STRUCT *params);

/*
 * Allocate server structure, init sockets, etc.
 */
TELNETSRV_STRUCT* telnetsrv_create_server(TELNETSRV_PARAM_STRUCT* params)
{
    TELNETSRV_STRUCT *server = NULL;
    uint32_t error;
    uint32_t error4 = TELNETSRV_OK;
    uint32_t error6 = TELNETSRV_OK;

    /* Install device drivers for socket and telnet I/O */
    #if MQX_USE_IO_OLD
       _io_socket_install("socket:");
       _io_telnet_install("telnet:");
    #else
       _nio_dev_install("socket:", &_io_socket_dev_fn, NULL, NULL);
       _nio_dev_install("telnet:", &_io_telnet_dev_fn, NULL, NULL);
    #endif

    if ((server = _mem_alloc_system_zero(sizeof(TELNETSRV_STRUCT))) == NULL)
    {
        return(NULL);
    }
    _mem_set_type(server, MEM_TYPE_TELNETSRV_STRUCT);

    error = _lwsem_create(&server->tid_sem, 1);
    if (error != MQX_OK)
    {
        goto EXIT;
    }

    error = telnetsrv_set_params(server, params);
    if (error != TELNETSRV_OK)
    {
        goto EXIT;
    }
    
    /* Allocate space for session pointers */
    server->session = _mem_alloc_zero(sizeof(TELNETSRV_SESSION_STRUCT*) * server->params.max_ses);
    if (server->session == NULL)
    {
        goto EXIT;
    }

    /* Allocate space for session task IDs */
    server->ses_tid = _mem_alloc_zero(sizeof(_rtcs_taskid) * server->params.max_ses);
    if (server->ses_tid == NULL)
    {
        goto EXIT;
    }

    /* Init sockets. */
    if (server->params.af & AF_INET)
    {
        /* Setup IPv4 server socket */
        error4 = telnetsrv_init_socket(server, AF_INET);
    }
    if (server->params.af & AF_INET6)
    {
        /* Setup IPv6 server socket */
        error6 = telnetsrv_init_socket(server, AF_INET6);
    }

    if ((error4 != TELNETSRV_OK) || (error6 != TELNETSRV_OK))
    {
        goto EXIT;
    }

    return(server);
    EXIT:
    telnetsrv_destroy_server(server);
    return(server);
}

/*
 * Close sockets, free memory etc.
 */
int32_t telnetsrv_destroy_server(TELNETSRV_STRUCT* server)
{
    uint32_t n = 0;
    bool     wait = FALSE;
    
    if (server == NULL)
    {
        return(RTCS_ERROR);
    }
    if (server->sock_v4)
    {
        closesocket(server->sock_v4);
    }
    if (server->sock_v6)
    {
        closesocket(server->sock_v6);
    }
    /* Invalidate sessions (this is signal for session tasks to end them) */
    for (n = 0; n < server->params.max_ses; n++)
    {
        if (server->session[n])
        {
            server->session[n]->valid = FALSE;
        }
    }
    /* Wait until all session tasks end */
    do
    {
        wait = FALSE;
        for (n = 0; n < server->params.max_ses; n++)
        {
            if (server->ses_tid[n])
            {
                wait = TRUE;
                break;
            }
        }
        _sched_yield();
    }while(wait);

    
    _lwsem_destroy(&server->tid_sem);
    
    _mem_free((void*) server->ses_tid);
    server->ses_tid = NULL;
    _mem_free(server->session);
    server->session = NULL;
    return(RTCS_OK);
}

/*
** Internal function for server parameters initialization
**
** IN:
**      TELNETSRV_STRUCT* server - server structure pointer
**
**      TELNETSRV_PARAM_STRUCT* params - pointer to user parameters if there are any
** OUT:
**      none
**
** Return Value: 
**      uint32_t      error code. TELNETSRV_OK if everything went right, positive number otherwise
*/
static uint32_t telnetsrv_set_params(TELNETSRV_STRUCT *server, TELNETSRV_PARAM_STRUCT *params)
{
    server->params.port = IPPORT_TELNET;
    #if RTCSCFG_ENABLE_IP4
    server->params.ipv4_address.s_addr = 0;
    server->params.af |= AF_INET;
    #endif
    #if RTCSCFG_ENABLE_IP6  
    server->params.ipv6_address = in6addr_any;
    server->params.ipv6_scope_id = 0;
    server->params.af |= AF_INET6;
    #endif
    server->params.max_ses = RTCSCFG_TELNETSRV_SES_CNT;
    server->params.server_prio = RTCSCFG_TELNETSRV_SERVER_PRIO;
    server->params.use_nagle = 0;
    server->params.shell = NULL;
    server->params.shell_commands = NULL;
    /* If there is parameters structure copy nonzero values to server */
    if (params != NULL)
    {
        if (params->port)
            server->params.port = params->port;
        #if RTCSCFG_ENABLE_IP4
        if (params->ipv4_address.s_addr != 0)
            server->params.ipv4_address = params->ipv4_address;
        #endif
        #if RTCSCFG_ENABLE_IP6
        if (params->ipv6_address.s6_addr != NULL)
            server->params.ipv6_address = params->ipv6_address;
        if (params->ipv6_scope_id)
            server->params.ipv6_scope_id = params->ipv6_scope_id;
        #endif
        if (params->af)
            server->params.af = params->af;
        if (params->max_ses)
            server->params.max_ses = params->max_ses;
        if (params->server_prio)
            server->params.server_prio = params->server_prio;
        if (params->use_nagle)
            server->params.use_nagle = params->use_nagle;
        if (params->shell)
            server->params.shell = params->shell;
        if (params->shell_commands)
            server->params.shell_commands = params->shell_commands;
    }

    return(TELNETSRV_OK);
}

/*
** Function for socket initialization (both IPv4 and IPv6)
**
** IN:
**      TELNETSRV_STRUCT* server - server structure pointer
**
**      uint16_t      family - IP protocol family
** OUT:
**      none
**
** Return Value: 
**      uint32_t      error code. TELNETSRV_OK if everything went right, positive number otherwise
*/
static uint32_t telnetsrv_init_socket(TELNETSRV_STRUCT *server, uint16_t family)
{
    uint32_t option;
    uint32_t error;
    sockaddr sin_sock;
    uint32_t sock = 0;
    uint32_t is_error = 0;

    _mem_zero(&sin_sock, sizeof(sockaddr));
    #if RTCSCFG_ENABLE_IP4
    if (family == AF_INET) /* IPv4 */
    {
       
        if ((server->sock_v4 = socket(AF_INET, SOCK_STREAM, 0)) == (uint32_t)RTCS_ERROR)
        {
            return(TELNETSRV_CREATE_FAIL);
        }
        ((sockaddr_in *)&sin_sock)->sin_port   = server->params.port;
        ((sockaddr_in *)&sin_sock)->sin_addr   = server->params.ipv4_address;
        ((sockaddr_in *)&sin_sock)->sin_family = AF_INET;
        sock = server->sock_v4;
    }
    else
    #endif    
    #if RTCSCFG_ENABLE_IP6   
    if (family == AF_INET6) /* IPv6 */
    {
        if ((server->sock_v6 = socket(AF_INET6, SOCK_STREAM, 0)) == (uint32_t)RTCS_ERROR)
        {
            return(TELNETSRV_CREATE_FAIL);
        }
        ((sockaddr_in6 *)&sin_sock)->sin6_port      = server->params.port;
        ((sockaddr_in6 *)&sin_sock)->sin6_family    = AF_INET6;
        ((sockaddr_in6 *)&sin_sock)->sin6_scope_id  = server->params.ipv6_scope_id;
        ((sockaddr_in6 *)&sin_sock)->sin6_addr      = server->params.ipv6_address;
        sock = server->sock_v6;
    }
    else
    #endif    
    {
        return(TELNETSRV_BAD_FAMILY);
    }
    /* Set socket options */
    option = RTCSCFG_TELNETSRV_SEND_TIMEOUT;
    is_error = is_error || setsockopt(sock, SOL_TCP, OPT_SEND_TIMEOUT, &option, sizeof(option));
    option = RTCSCFG_TELNETSRV_CONNECT_TIMEOUT;
    is_error = is_error || setsockopt(sock, SOL_TCP, OPT_CONNECT_TIMEOUT, &option, sizeof(option));
    option = RTCSCFG_TELNETSRV_TIMEWAIT_TIMEOUT;
    is_error = is_error || setsockopt(sock, SOL_TCP, OPT_TIMEWAIT_TIMEOUT, &option, sizeof(option));
    option = FALSE; 
    is_error = is_error || setsockopt(sock, SOL_TCP, OPT_RECEIVE_NOWAIT, &option, sizeof(option));
    option = TRUE;  
    is_error = is_error || setsockopt(sock, SOL_TCP, OPT_RECEIVE_PUSH, &option, sizeof(option));
    option = !server->params.use_nagle;
    is_error = is_error || setsockopt(sock, SOL_TCP, OPT_NO_NAGLE_ALGORITHM, &option, sizeof(option));
    option = RTCSCFG_TELNETSRV_TX_BUFFER_SIZE;
    is_error = is_error || setsockopt(sock, SOL_TCP, OPT_TBSIZE, &option, sizeof(option));
    option = RTCSCFG_TELNETSRV_RX_BUFFER_SIZE;
    is_error = is_error || setsockopt(sock, SOL_TCP, OPT_RBSIZE, &option, sizeof(option));

    if (is_error)
    {
        return(TELNETSRV_SOCKOPT_FAIL);
    }

    /* Bind socket */
    error = bind(sock, &sin_sock, sizeof(sin_sock));
    if(error != RTCS_OK)
    {
        return(TELNETSRV_BIND_FAIL);
    }

    /* Listen */
    error = listen(sock, server->params.max_ses);
    if (error != RTCS_OK)
    {
        return(TELNETSRV_LISTEN_FAIL);
    }
    return(TELNETSRV_OK);
}

/*
 * Wait for incoming connection, return socket with activity or error.
 */
uint32_t telnetsrv_wait_for_conn(TELNETSRV_STRUCT *server)
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
 * Accept connection from client.
 */
uint32_t telnetsrv_accept(uint32_t sock)
{
    struct sockaddr  remote_addr;
    unsigned short   length;

    _mem_zero(&remote_addr, sizeof(remote_addr));
    length = sizeof(remote_addr);
    return(accept(sock, (sockaddr *) &remote_addr, &length));
}

/*
 * Abort connection on socket.
 */
void telnetsrv_abort(uint32_t sock)
{
    struct linger l_options;

    /* Set linger options for RST flag sending. */
    l_options.l_onoff = 1;
    l_options.l_linger_ms = 0;
    setsockopt(sock, SOL_SOCKET, SO_LINGER, &l_options, sizeof(l_options));
    closesocket(sock);
}
