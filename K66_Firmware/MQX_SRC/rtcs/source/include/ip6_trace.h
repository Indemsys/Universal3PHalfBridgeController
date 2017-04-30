#ifndef __ip6_trace_h__
#define __ip6_trace_h__
/*HEADER**********************************************************************
*
* Copyright 2011-2013 Freescale Semiconductor, Inc.
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
*   This file contains the implementation of the TCP/IP tracing feature.
*   It is based on FNET project.
*
*
*END************************************************************************/

#include <rtcs.h>
#include "rtcs_prv.h"
#include "tcpip.h"
#include "ip_prv.h"
#include "icmp6_prv.h"

/******************************************************************
* Function Prototypes
*******************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#if RTCSCFG_DEBUG_TRACE_ETH
    void trace_eth(char *str, ENET_HEADER_PTR eth_hdr);
#else
    #define trace_eth(str, eth_hdr)
#endif 

#if RTCSCFG_DEBUG_TRACE_IP6
    void trace_ip6(char *str, IP6_HEADER_PTR ip_hdr);
#else
    #define trace_ip6(str, ip_hdr)
#endif 

#if RTCSCFG_DEBUG_TRACE_ICMP6
    void trace_icmp6(char *str, ICMP6_HEADER_PTR icmp_hdr);
#else
    #define trace_icmp6(str, icmp_hdr)
#endif 

#ifdef __cplusplus
}
#endif

#endif /*__ip6_trace_h__*/

