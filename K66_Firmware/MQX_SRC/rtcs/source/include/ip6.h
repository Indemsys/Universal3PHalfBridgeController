#ifndef __ip6_h__
#define __ip6_h__
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
*   Definitions for the Internet Protocol 6.
*
*
*END************************************************************************/

#include "rtcspcb.h"
/******************************************************************
* Definitions
*******************************************************************/
#define IP6_HEADSIZE            40      /* sizeof(IP6_HEADER)                 */
#define IP6_DEFAULT_MTU         1280    /* Minimum IPv6 datagram size which    */
                                        /* must be supported by all IPv6 hosts */
#define IP6_DEFAULT_HOPLIMIT    64      /* Default Hop Limit. */

/* IPIF_*() params.*/
typedef struct ip6_if_parm
{
    TCPIP_PARM          COMMON;    
    _rtcs_if_handle     ihandle;
    in6_addr            *ip_addr;
    rtcs6_if_addr_type  ip_addr_type;
    uint32_t            ip_addr_lifetime; /*in seconds*/
} 
IP6_IF_PARM, * IP6_IF_PARM_PTR;

/* IP6 parameters for [gs]etsockopt */
typedef struct ip6_sock_parm_get
{
    TCPIP_PARM             COMMON;
    struct ip6_sock_parm  *NEXT;
    SOCKET_STRUCT_PTR      sock;
    uint32_t               option;
    void                   *optptr;
    void * option_value;
    uint32_t               optlen;
} IP6_SOCK_PARM_GET, * IP6_SOCK_PARM_GET_PTR;

typedef struct ip6_sock_parm_set
{
    TCPIP_PARM             COMMON;
    struct ip6_sock_parm  *NEXT;
    SOCKET_STRUCT_PTR      sock;
    uint32_t               option;
    void                   *optptr;
    const void * option_value;
    uint32_t               optlen;
} IP6_SOCK_PARM_SET, * IP6_SOCK_PARM_SET_PTR;


/******************************************************************
* Function Prototypes
*******************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

uint32_t IP6_init(void);
ICB_STRUCT_PTR IP6_open (unsigned char  protocol, IP_SERVICE_FN service, void *protocol_cfg, uint32_t *status);
void IP6_service (RTCSPCB_PTR);
uint32_t IP6_send (RTCSPCB_PTR rtcs_pcb, uint32_t protocol, in6_addr *ipsrc, in6_addr *ipdest, _rtcs_if_handle ihandle_dest);

#ifdef __cplusplus
}
#endif

#endif /* __ip6_h__ */

