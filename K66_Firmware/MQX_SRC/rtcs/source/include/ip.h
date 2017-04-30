#ifndef __ip_h__
#define __ip_h__
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
*   Definitions for the Internet protocol layer.
*
*
*END************************************************************************/

#include "rtcstime.h"
#include "rtcspcb.h"
/***************************************
**
** Constants
**
*/

/*
** IP datagram sizes
*/

#define IP_HEADSIZE        20       /* sizeof(IP_HEADER)                 */

#define IP_DEFAULT_MTU     576      /* minimum IP datagram size which    */
                                    /* must be supported by all IP hosts */
#define IP_MAX_MTU         0xFFFF   /* size of largest IP datagram       */

/*
** default IP parameters for outgoing packets
*/
#define IP_VERSION         4        /* Version 4               */
#define IP_VERSION_4       4        /* Version 4               */
#define IP_VERSION_6       6        /* Version 6               */
/* Start CR 1899 */
#define IPTTL_DEFAULT      64       /* Time to Live (60 hops)  */
/* End CR 1899 */
#define IPTOS_DEFAULT      0        /* Type of Service         */

#define IP_HEADER_LENGTH_MASK  0x0F

#define IPTTL(ttl)         ((uint32_t)(ttl) <<  8)
#define IPTOS(tos)         ((uint32_t)(tos) << 16)
#define IPDF(df)           ((uint32_t)(df)  << 24)

/*
**
** Ports < IPPORT_RESERVED are reserved for
**  privileged processes (e.g. root).
**
** Ports > IPPORT_USERRESERVED are reserved
**  for servers, not necessarily privileged.
*/
#define IPPORT_RESERVED          1024
#define IPPORT_USERRESERVED      5000


/***************************************
**
** Type definitions
**
*/

/*
** Multicast group members
*/

typedef struct mc_member {
   ip_mreq           IGRP;       /* the group description */
   uint32_t           UCOUNT;     /* usage counter */
   struct mc_member       *NEXT;       /* pointer to the next in the list */

   /* used only by the interface part */
   TCPIP_EVENT TIMER;
   bool     RUNNING_TIMER; /* true if the timer is running */
   bool     LAST_REPORTER; /* true if we are the last host to report on this group */
   uint32_t     UNSOLICITED_REPORT_COUNTER;   /* the number of unsolicited reports remaining to send */

   /*
   ** it's unnecessary to store a state (i.e. non-member, delaying-member, idle-member)
   ** because non-members don't have a MC_MEMBER, delaying-member is when
   ** the running_timer is true, and idle-member when running_timer is true
   */

} MC_MEMBER, * MC_MEMBER_PTR;

/*
** Function for servicing packets
*/

typedef void (_CODE_PTR_ IP_SERVICE_FN)(RTCSPCB_PTR, void *);

/*
** Internet Channel Block
*/
typedef struct icb_struct {

   struct icb_struct       *NEXT;          /* next ICB in the chain      */
   uint32_t           PROTOCOL;      /* protocol for connection    */

   IP_SERVICE_FN     SERVICE;       /* Owner's service function    */
   void             *PRIVATE;       /* Owner's configuration block */

} ICB_STRUCT, * ICB_STRUCT_PTR;


/***************************************
**
** Prototypes
**
*/

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t IP_init
(
   void
);

extern ICB_STRUCT_PTR IP_open
(
   unsigned char             ,     /* [IN]  Protocol to use */
   IP_SERVICE_FN     ,     /* [IN]  Packet receive function */
   void             *,     /* [IN]  Owner's config block */
   uint32_t             * /* [OUT] return code */
);

extern uint32_t IP_send
(
   RTCSPCB_PTR       ,     /* [IN] the packet to send */
   uint32_t           ,     /* [IN] transport protocol, TTL and TOS */
   _ip_address       ,     /* [IN] the destination interface (0 = any) */
   _ip_address       ,     /* [IN] the ultimate destination */
   uint32_t                 /* [IN] optional flags */
);

extern uint32_t IP_send_IF
(
   RTCSPCB_PTR       ,     /* [IN] the packet to send */
   uint32_t           ,     /* [IN] transport protocol, TTL and TOS */
   void                   * /* [IN] the destination interface */
);

extern void IP_service
(
   RTCSPCB_PTR             /* [IN] received packet */
);

extern _ip_address IP_source
(
   RTCSPCB_PTR             /* [IN] packet to find source of */
);

extern _ip_address IP_dest
(
   RTCSPCB_PTR             /* [IN] packet to find destination of */
);

#ifdef __cplusplus
}
#endif

#if RTCSCFG_ENABLE_IP6
    #include "ip6.h"
#endif


#endif
/* EOF */
