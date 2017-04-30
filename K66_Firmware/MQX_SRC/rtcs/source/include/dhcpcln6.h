#ifndef __dhcpcln6_h__
#define __dhcpcln6_h__
/*HEADER**********************************************************************
*
* Copyright 2013-2014 Freescale Semiconductor, Inc.
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
*   Public header for DHCPv6 client.
*
*END************************************************************************/

#include <stdint.h>
#include <rtcs.h>

#if RTCSCFG_ENABLE_IP6

#define DHCPCLN6_FLAG_STATELESS  (0x00000001)
#define DHCPCLN6_FLAG_CHECK_LINK (0x00000002)

/*
 * DHCPv6 client states
 */
typedef enum dhcpcln6_status 
{
    DHCPCLN6_STATUS_BOUND,
    DHCPCLN6_STATUS_UNBOUND,
    DHCPCLN6_STATUS_NOT_RUNNING
}DHCPCLN6_STATUS;

/*
 * DHCPv6 client initialization structure.
 */
typedef struct dhcpcln6_param_struct
{
    /* How long should client wait before address confirmation in milliseconds. */
    uint32_t                      confirm_wait;
    /* Preferred IPv6 address. */
    in6_addr                      *preferred;
    /* Interface to configure DHCP for. */
    _rtcs_if_handle               interface;
    /* Client flags. */
    uint32_t                      flags;
}DHCPCLN6_PARAM_STRUCT;

/*
 * API functions.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Start DHCPv6 client. */
uint32_t DHCPCLN6_init(DHCPCLN6_PARAM_STRUCT *params);
/* Stop DHCPv6 client. */
uint32_t DHCPCLN6_release(uint32_t handle);
/* Read client status */
DHCPCLN6_STATUS DHCPCLN6_get_status(uint32_t handle);

#ifdef __cplusplus
}
#endif

#endif /* RTCSCFG_ENABLE_IP6 */
#endif
