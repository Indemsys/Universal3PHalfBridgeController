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
*   Header for ECHOSRV.
*
*
*END************************************************************************/

#ifndef ECHOSRV_H_
#define ECHOSRV_H_

#include <rtcs.h>

/*
** ECHO server parameters
*/
typedef struct echosrv_param_struct
{
  uint16_t                   af;             /* Inet protocol family */
  uint16_t                   port;           /* Listening port */
#if RTCSCFG_ENABLE_IP4
  in_addr                    ipv4_address;   /* Listening IPv4 address */
#endif
#if RTCSCFG_ENABLE_IP6
  in6_addr                   ipv6_address;   /* Listening IPv6 address */
  uint32_t                   ipv6_scope_id;  /* Scope ID for IPv6 */
#endif
  uint32_t     server_prio;    /* server main task priority */
} ECHOSRV_PARAM_STRUCT;

#ifdef __cplusplus
extern "C" {
#endif

/*
** Initialize and run ECHO server
** Returns server handle when successful, zero otherwise.
*/
void* ECHOSRV_init(ECHOSRV_PARAM_STRUCT *);

/*
** Stop and release ECHO server
** Returns RTCS_OK when successful, error code otherwise.
*/
uint32_t ECHOSRV_release(void*);

#ifdef __cplusplus
}
#endif


#endif /* ECHOSRV_H_ */
