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
*   This file contains the TFTP server support functions.
*
*
*END************************************************************************/

#include "tftp_prv.h"
#include "tftpsrv_prv.h"
#include "rtcs_util.h"
#include <limits.h>

static uint32_t tftpsrv_set_params(TFTPSRV_STRUCT *server, TFTPSRV_PARAM_STRUCT *params);
static uint32_t tftpsrv_init_socket(TFTPSRV_STRUCT *server, uint16_t family);

/*
 * Allocate server structure, init sockets, etc.
 */
TFTPSRV_STRUCT* tftpsrv_create_server(TFTPSRV_PARAM_STRUCT* params)
{
    TFTPSRV_STRUCT *server = NULL;
    uint32_t error;
    uint32_t error4 = TFTPSRV_OK;
    uint32_t error6 = TFTPSRV_OK;


    if ((server = _mem_alloc_system_zero(sizeof(TFTPSRV_STRUCT))) == NULL)
    {
        return(NULL);
    }
    _mem_set_type(server, MEM_TYPE_TFTPSRV_SERVER_STRUCT);

    error = _lwsem_create(&server->tid_sem, 1);
    if (error != MQX_OK)
    {
        goto EXIT;
    }

    error = tftpsrv_set_params(server, params);
    if (error != TFTPSRV_OK)
    {
        goto EXIT;
    }
    
    /* Allocate space for session pointers */
    server->session = _mem_alloc_zero(sizeof(TFTPSRV_SESSION_STRUCT*) * server->params.max_ses);
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
        error4 = tftpsrv_init_socket(server, AF_INET);
    }
    if (server->params.af & AF_INET6)
    {
        /* Setup IPv6 server socket */
        error6 = tftpsrv_init_socket(server, AF_INET6);
    }

    if ((error4 != TFTPSRV_OK) || (error6 != TFTPSRV_OK))
    {
        goto EXIT;
    }

    return(server);
    EXIT:
    tftpsrv_destroy_server(server);
    return(server);
}

/*
 * Close sockets, free memory etc.
 */
int32_t tftpsrv_destroy_server(TFTPSRV_STRUCT* server)
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
**      TFTPSRV_STRUCT* server - server structure pointer
**
**      TFTPSRV_PARAM_STRUCT* params - pointer to user parameters if there are any
** OUT:
**      none
**
** Return Value: 
**      uint32_t      error code. TFTPSRV_OK if everything went right, positive number otherwise
*/
static uint32_t tftpsrv_set_params(TFTPSRV_STRUCT *server, TFTPSRV_PARAM_STRUCT *params)
{
    server->params.port = IPPORT_TFTP;
    #if RTCSCFG_ENABLE_IP4
    server->params.ipv4_address.s_addr = 0;
    server->params.af |= AF_INET;
    #endif
    #if RTCSCFG_ENABLE_IP6  
    server->params.ipv6_address = in6addr_any;
    server->params.ipv6_scope_id = 0;
    server->params.af |= AF_INET6;
    #endif
    server->params.max_ses = RTCSCFG_TFTPSRV_SES_CNT;
    server->params.server_prio = RTCSCFG_TFTPSRV_SERVER_PRIO;
    server->params.root_dir = "tfs:";
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
        if (params->root_dir)
        {
            server->params.root_dir = params->root_dir;  
        }
    }

    return(TFTPSRV_OK);
}

/*
** Function for socket initialization (both IPv4 and IPv6)
**
** IN:
**      TFTPSRV_STRUCT* server - server structure pointer
**
**      uint16_t      family - IP protocol family
** OUT:
**      none
**
** Return Value: 
**      uint32_t      error code. TFTPSRV_OK if everything went right, positive number otherwise
*/
static uint32_t tftpsrv_init_socket(TFTPSRV_STRUCT *server, uint16_t family)
{
    uint32_t error;
    sockaddr sin_sock;
    uint32_t sock = 0;

    _mem_zero(&sin_sock, sizeof(sockaddr));
    #if RTCSCFG_ENABLE_IP4
    if (family == AF_INET) /* IPv4 */
    {
       
        if ((server->sock_v4 = socket(AF_INET, SOCK_DGRAM, 0)) == (uint32_t)RTCS_ERROR)
        {
            return(TFTPSRV_CREATE_FAIL);
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
        if ((server->sock_v6 = socket(AF_INET6, SOCK_DGRAM, 0)) == (uint32_t)RTCS_ERROR)
        {
            return(TFTPSRV_CREATE_FAIL);
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
        return(TFTPSRV_BAD_FAMILY);
    }

    /* Bind socket */
    error = bind(sock, &sin_sock, sizeof(sin_sock));
    if(error != RTCS_OK)
    {
        return(TFTPSRV_BIND_FAIL);
    }

    return(TFTPSRV_OK);
}

/*
 * Open file from filesystem in specified mode.
 */
int32_t tftpsrv_open_file(uint16_t opcode, char *filename, char *root, char *filemode,
        #if MQX_USE_IO_OLD
        MQX_FILE_PTR  *file_ptr
        #else
        FILE  **file_ptr
        #endif
   )
{
    int32_t   error;
    uint32_t  file_error;
    char      open_flags[4] = {0};
    char      *path;

    error = TFTPSRV_OK;

    path = rtcs_path_create(root, filename);
    if (path == NULL)
    {
        error = TFTP_ERR_UNKNOWN;
        goto EXIT;
    }
    /* 
     * The new C standard (C2011, which is not part of C++) adds a new standard 
     * subspecifier ("x"), that can be appended to any "w" specifier (to form "wx",
     * "wbx", "w+x" or "w+bx"/"wb+x"). This subspecifier forces the function to 
     * fail if the file exists, instead of overwriting it.
     */
    #if MQX_USE_IO_OLD
    open_flags[1] = 'x';
    #endif

    if (opcode == TFTP_OPCODE_RRQ)
    {
        open_flags[0] = 'r';
        error = TFTP_ERR_FILE_NOT_FOUND;
    }
    else if (opcode == TFTP_OPCODE_WRQ)
    {
        open_flags[0] = 'w';
        error = TFTP_ERR_FILE_EXISTS;
    }
    else
    {
        error = TFTP_ERR_ILLEGAL_OP;
        goto EXIT;
    }

    if (strcmp(filemode, "octet") == 0)
    {
        open_flags[1] = 'b';
        #if MQX_USE_IO_OLD
        open_flags[2] = 'x';
        #endif
    }

    *file_ptr = RTCS_io_open(path, open_flags, &file_error);
    _mem_free(path);
    path = NULL;
    if (*file_ptr != NULL)
    {
        error = TFTPSRV_OK;
    }

    EXIT:
    return(error);
}
