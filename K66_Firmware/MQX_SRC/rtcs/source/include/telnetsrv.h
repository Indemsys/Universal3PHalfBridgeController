#ifndef __telnetsrv_h__
#define __telnetsrv_h__
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
*   This file contains the definitions for the Telnet server.
*
*
*END************************************************************************/

#include <rtcs.h>

/*
** Telnet server status codes
*/
#define TELNETSRV_OK           (0)
#define TELNETSRV_BIND_FAIL    (1) 
#define TELNETSRV_LISTEN_FAIL  (2)
#define TELNETSRV_ERR          (3)
#define TELNETSRV_CREATE_FAIL  (4)
#define TELNETSRV_BAD_FAMILY   (5)
#define TELNETSRV_SOCKOPT_FAIL (6)
#define TELNETSRV_SES_INVALID  (7)

typedef int32_t (*TELNET_SHELL_FUNCTION)(void* shell_commands, void *start_file);

typedef struct telnetsrv_param_struct
{
    uint16_t              af;             /* Inet protocol family */
    unsigned short        port;           /* Listening port */
    #if RTCSCFG_ENABLE_IP4
    in_addr               ipv4_address;   /* Listening IPv4 address */
    #endif
    #if RTCSCFG_ENABLE_IP6    
    in6_addr              ipv6_address;   /* Listening IPv6 address */
    uint32_t              ipv6_scope_id;  /* Scope ID for IPv6 */
    #endif
    uint32_t              max_ses;        /* maximal sessions count */
    bool                  use_nagle;      /* enable/disable nagle algorithm for server sockets */
    uint32_t              server_prio;    /* server main task priority */
    TELNET_SHELL_FUNCTION shell;          /* Pointer to shell to run */
    void                  *shell_commands;/* Pointer to shell commands */
}TELNETSRV_PARAM_STRUCT;

#ifdef __cplusplus
extern "C" {
#endif

uint32_t TELNETSRV_init(TELNETSRV_PARAM_STRUCT *params);
uint32_t TELNETSRV_release(uint32_t server_h);

#ifdef __cplusplus
}
#endif

#endif
