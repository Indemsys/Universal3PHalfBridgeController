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

#include "telnetsrv_prv.h"

#define TELNETSRV_SERVER_TASK_NAME "Telnet server"

/*
** Function for starting the HTTP server 
**
** IN:
**      TELNETSRV_PARAM_STRUCT*   params - server parameters (port, ip, etc.)
**
** OUT:
**      none
**
** Return Value: 
**      uint32_t      server handle if successful, NULL otherwise
*/
uint32_t TELNETSRV_init(TELNETSRV_PARAM_STRUCT *params)
{
    TELNETSRV_SERVER_TASK_PARAM server_params;

    server_params.params = params;

    /* Server must run with lower priority than TCP/IP task. */
    if (params->server_prio == 0)
    {
        params->server_prio = RTCSCFG_TELNETSRV_SERVER_PRIO;
    }
    else if (params->server_prio < _RTCSTASK_priority)
    {
        return(0);
    }
    
    /* Run server task. */
    if (RTCS_task_create(TELNETSRV_SERVER_TASK_NAME, params->server_prio, TELNETSRV_SERVER_STACK_SIZE, telnetsrv_server_task, &server_params) != RTCS_OK)
    {
        TELNETSRV_release(server_params.handle);
        return(0);
    }

    return(server_params.handle);
}

/*
** Function for releasing/stopping telnet server 
**
** IN:
**      uint32_t       server_h - server handle
**
** OUT:
**      none
**
** Return Value: 
**      uint32_t      error code. TELNETSRV_OK if everything went right, positive number otherwise
*/
uint32_t TELNETSRV_release(uint32_t server_h)
{
    TELNETSRV_STRUCT* server = (void *) server_h;

    /* Shutdown server task and wait for its termination. */
    if (server->sock_v4)
    {
        shutdownsocket(server->sock_v4, SHUT_RDWR);
    }
    if (server->sock_v6)
    {
        shutdownsocket(server->sock_v6, SHUT_RDWR);
    }
    
    while(server->server_tid)
    {
        _sched_yield();
    }
    _mem_free(server);
    return(RTCS_OK);
}
