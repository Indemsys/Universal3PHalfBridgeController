#ifndef __ftpsrv_h__
#define __ftpsrv_h__
/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
* Copyright 2004-2008 Embedded Access Inc.
* Copyright 1989-2008 ARC International
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
*   This file contains the FTP server.
*
*
*END************************************************************************/
#include <rtcs.h>

#define FTPSRV_OK 0
#define FTPSRV_BIND_FAIL 1 
#define FTPSRV_LISTEN_FAIL 2
#define FTPSRV_ERROR 3
#define FTPSRV_CREATE_FAIL 4
#define FTPSRV_BAD_FAMILY 5
#define FTPSRV_SOCKOPT_FAIL 6

typedef struct ftpsrv_auth_struct
{
    char* uid;                /* User ID (name) */
    char* pass;               /* Password */
    char* path;               /* Path to change directory to after user logs in */
}FTPSRV_AUTH_STRUCT;

/*
** FTP server parameters
*/
typedef struct ftpsrv_param_struct
{
    uint16_t                   af;             /* Address family */
    unsigned short             port;           /* Listening port */
  #if RTCSCFG_ENABLE_IP4
    in_addr                    ipv4_address;   /* Listening IPv4 address */
  #endif
  #if RTCSCFG_ENABLE_IP6    
    in6_addr                   ipv6_address;   /* Listening IPv6 address */
    uint32_t                   ipv6_scope_id;  /* Scope ID for IPv6 */
  #endif
    _mqx_uint                  max_ses;        /* maximal sessions count */
    bool                       use_nagle;      /* enable/disable nagle algorithm for server sockets */
    uint32_t                   server_prio;    /* server main task priority */
    char*                      root_dir;       /* root directory */
    
    FTPSRV_AUTH_STRUCT*        auth_table;     /* Table of authentication data */
} FTPSRV_PARAM_STRUCT;

#ifdef __cplusplus
extern "C" {
#endif

uint32_t FTPSRV_init(FTPSRV_PARAM_STRUCT* params);
uint32_t FTPSRV_release(uint32_t server_h);

#ifdef __cplusplus
}
#endif

#endif

/* EOF*/
