#ifndef __icmp6_h__
#define __icmp6_h__
/*HEADER**********************************************************************
*
* Copyright 2011 Freescale Semiconductor, Inc.
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
*   Definitions for the ICMP6 protocol layer.
*
*
*END************************************************************************/

#include "icmp.h"

/******************************************************************
* ICMPv6 message types (RFC 4443)
******************************************************************/

/* ICMPv6 error messages:*/
#define ICMP6_TYPE_DEST_UNREACH                 (1)	    /* Destination Unreachable. */
#define ICMP6_TYPE_PACKET_TOOBIG                (2)	    /* Packet Too Big. */
#define ICMP6_TYPE_TIME_EXCEED                  (3)	    /* Time Exceeded. */
#define ICMP6_TYPE_PARAM_PROB                   (4)	    /* Parameter Problem. */

/* ICMPv6 informational messages:*/
#define ICMP6_TYPE_ECHO_REQ                     (128)   /* Echo Request. */
#define ICMP6_TYPE_ECHO_REPLY                   (129)   /* Echo Reply. */

/* MLD messages (RFC2710):*/
#define ICMP6_TYPE_MULTICAST_LISTENER_QUERY     (130)   /* Multicast Listener Query */
#define ICMP6_TYPE_MULTICAST_LISTENER_REPORT    (131)   /* Multicast Listener Report */
#define ICMP6_TYPE_MULTICAST_LISTENER_DONE      (132)   /* Multicast Listener Done */

/*  Neighbor Discovery defines five different ICMP packet types (RFC4861):*/
#define ICMP6_TYPE_ROUTER_SOLICITATION          (133)   /* Router Solicitation. */
#define ICMP6_TYPE_ROUTER_ADVERTISEMENT	        (134)   /* Router Advertisement. */
#define ICMP6_TYPE_NEIGHBOR_SOLICITATION        (135)   /* Neighbor Solicitation. */
#define ICMP6_TYPE_NEIGHBOR_ADVERTISEMENT       (136)   /* Neighbor Advertisement. */
#define ICMP6_TYPE_REDIRECT                     (137)   /* Redirect.*/

/* Destination Unreachable codes */
#define ICMP6_CODE_DU_NO_ROUTE          (0)   /* No route to destination. */
#define ICMP6_CODE_DU_ADMIN_PROHIBITED  (1)   /* Communication with destination administratively prohibited. */
#define ICMP6_CODE_DU_BEYOND_SCOPE      (2)   /* Beyond scope of source address.*/
#define ICMP6_CODE_DU_ADDR_UNREACH      (3)   /* Address unreachable.*/
#define ICMP6_CODE_DU_PORT_UNREACH      (4)   /* Port unreachable.*/
#define ICMP6_CODE_DU_ADDR_FAILED       (5)   /* Source address failed ingress/egress policy.*/
#define ICMP6_CODE_DU_REJECT_ROUTE      (6)   /* Reject route to destination.*/

/* Packet Too Big codes */
#define ICMP6_CODE_PTB                  (0)  

/* Time Exceeded codes */
#define ICMP6_CODE_TE_HOP_LIMIT         (0)   /* Hop limit exceeded in transit.*/
#define ICMP6_CODE_TE_FRG_REASSEMBLY    (1)   /* Fragment reassembly time exceeded.*/

/* Parameter Problem codes */
#define ICMP6_CODE_PP_HEADER            (0)   /* Erroneous header field encountered.*/
#define ICMP6_CODE_PP_NEXT_HEADER       (1)   /* Unrecognized Next Header type encountered.*/
#define ICMP6_CODE_PP_OPTION            (2)   /* Unrecognized IPv6 option encountered.*/

/******************************************************************
* Function Prototypes
*******************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t ICMP6_init ( void );
extern void ICMP6_service (RTCSPCB_PTR, void *);
extern void ICMP6_send_echo (ICMP_ECHO_PARAM_PTR);

void ICMP6_send_error ( uint8_t type            /* [IN] the ICMPv6 type to send */,    
                        uint8_t code            /* [IN] the ICMPv6 code to send */,
                        uint32_t param          /* [IN] error parameter */,
                        RTCSPCB_PTR origpcb     /* [IN] the packet which caused the error */ );
uint32_t ICMP6_send (RTCSPCB_PTR rtcs_pcb, in6_addr *ipsrc, in6_addr *ipdest, _rtcs_if_handle ihandle_dest, unsigned char hop_limit);

#ifdef __cplusplus
}
#endif

#endif /* __icmp6_h__ */

