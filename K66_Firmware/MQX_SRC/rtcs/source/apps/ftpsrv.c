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
*   This file contains an implementation of FTP server.
*
*
*END************************************************************************/

#include <rtcs.h>
#include <string.h>

#include "ftpsrv_prv.h"

#define FTPSRV_SERVER_TASK_NAME "FTP server task"
/*
** Function for starting the HTTP server 
**
** IN:
**      FTPSRV_PARAM_STRUCT*   params - server parameters (port, ip, root directory etc.)
**
** OUT:
**      none
**
** Return Value: 
**      uint32_t      server handle if successful, NULL otherwise
*/
uint32_t FTPSRV_init(FTPSRV_PARAM_STRUCT *params)
{
    FTPSRV_SERVER_TASK_PARAM server_params;

    server_params.params = params;
    if (params->server_prio == 0)
    {
        params->server_prio = FTPSRVCFG_DEF_SERVER_PRIO;
    }
    else if (params->server_prio < _RTCSTASK_priority)
    {
        /* Server must run with lower priority than TCP/IP task. */
        return(0);
    }
    /* Run server */
    if (RTCS_task_create(FTPSRV_SERVER_TASK_NAME, params->server_prio, FTPSRV_SERVER_STACK_SIZE, ftpsrv_server_task, &server_params) != RTCS_OK)
    {
        FTPSRV_release(server_params.handle);
        return(0);
    }

    return(server_params.handle);
}

/*
** Function for releasing/stopping FTP server 
**
** IN:
**      uint32_t       server_h - server handle
**
** OUT:
**      none
**
** Return Value: 
**      uint32_t      error code. FTPSRV_OK if everything went right, positive number otherwise
*/
uint32_t FTPSRV_release(uint32_t server_h)
{
    FTPSRV_STRUCT* server = (FTPSRV_STRUCT*) server_h;

    RTCS_ASSERT(server != NULL);
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
