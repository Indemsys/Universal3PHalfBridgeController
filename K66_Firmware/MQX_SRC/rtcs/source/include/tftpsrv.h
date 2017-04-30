#ifndef __tftpsrv_h__
#define __tftpsrv_h__
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
*   This file contains the definitions for the TFTP server.
*
*
*END************************************************************************/

#include <rtcs.h>

/*
** TFTP server status codes
*/
#define TFTPSRV_OK           (0)
#define TFTPSRV_BIND_FAIL    (1) 
#define TFTPSRV_ERR          (2)
#define TFTPSRV_CREATE_FAIL  (3)
#define TFTPSRV_BAD_FAMILY   (4)
#define TFTPSRV_SES_INVALID  (5)

typedef struct tftpsrv_param_struct
{
    uint16_t              af;             /* Inet protocol family */
    uint16_t              port;           /* Listening port */
    #if RTCSCFG_ENABLE_IP4
    in_addr               ipv4_address;   /* Listening IPv4 address */
    #endif
    #if RTCSCFG_ENABLE_IP6    
    in6_addr              ipv6_address;   /* Listening IPv6 address */
    uint32_t              ipv6_scope_id;  /* Scope ID for IPv6 */
    #endif
    uint32_t              max_ses;        /* maximal sessions count */
    uint32_t              server_prio;    /* server main task priority */
    char*                 root_dir;       /* Server root directory */
}TFTPSRV_PARAM_STRUCT;

#ifdef __cplusplus
extern "C" {
#endif

uint32_t TFTPSRV_init(TFTPSRV_PARAM_STRUCT *params);
uint32_t TFTPSRV_release(uint32_t server_h);

#ifdef __cplusplus
}
#endif

#endif
