#ifndef __udp_h__
#define __udp_h__
/*HEADER**********************************************************************
*
* Copyright 2008-2014 Freescale Semiconductor, Inc.
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
*   This file contains the User Datagram Protocol definitions.
*   For more details, refer to RFC768.
*
*
*END************************************************************************/


/***************************************
**
** Constants
**
*/

/*
** UDP Open Options
*/
#define UDPOPT_CHKSUM_RX   0x0001      /* Option to bypass UDP checksum on Rx */
#define UDPOPT_CHKSUM_TX   0x0002      /* Option to bypass UDP checksum on Tx */
#define UDPOPT_NOBLOCK     0x0004      /* Option to send non-blocking         */


/***************************************
**
** Type definitions
**
*/
typedef struct udp_rx_dgram_header
{
  struct sockaddr fromaddr;  /* ip addr and port of the remote host we received the datagram from */
  struct rtcs_linkopt_struct rx_linkopts;
  struct udp_rx_dgram_header * NEXT; /* linked list */
  uint32_t size; /* size in bytes of datagram that follows */
  char dgram[1]; /* first byte of datagram. the rest of the datagram bytes follow, memory is allocated dynamically. */
} udp_rx_dgram_header;

/*
** UDP Channel Block for listeners
*/
struct ucb_parm;
struct MCB;
struct ip6_multicast_member;
typedef struct ucb_struct {

   struct ucb_struct        *NEXT;
   uint32_t                 SOCKET;
   
   uint16_t                 PCOUNT;

   void (_CODE_PTR_ SERVICE)(RTCSPCB_PTR, struct ucb_struct *);

   /*
   ** Queue of incoming packets
   */
   struct udp_rx_dgram_header * PHEAD;
   struct udp_rx_dgram_header * PTAIL;

   /*
   ** Queue of waiting receive requests
   */
   struct ucb_parm        *RHEAD;
   struct ucb_parm        *RTAIL;
   
   /* Checksum bypass on reception */
   bool        BYPASS_RX;
   /* Checksum bypass on transmission */
   bool        BYPASS_TX;
   
   /* local IP address */
   sockaddr    LADDR;
   /* Remote IP address and port when connected. (0 when not connected) */
   sockaddr    RADDR;

   /* list of joined multicast groups */
   struct mc_member         *MCB_PTR;
   uint32_t (_CODE_PTR_     IGMP_LEAVEALL)(struct mc_member  **);

#if RTCSCFG_ENABLE_IP6
   struct ip6_multicast_member   *IP6_MULTICAST_PTR[RTCSCFG_IP6_MULTICAST_SOCKET_MAX];
#endif
   
   /*
   ** Determines if a connection failure keeps the ucb's local IP,
   ** or if it resets it to INADDR_ANY
   */
   bool            KEEP_IPADDR;
} UCB_STRUCT, * UCB_STRUCT_PTR;

/*
** UDP parameters
*/
typedef struct ucb_parm {
   TCPIP_PARM              COMMON;
   struct ucb_parm *       NEXT;
   UCB_STRUCT_PTR          ucb;
   void (_CODE_PTR_        udpservice)(RTCSPCB_PTR, UCB_STRUCT_PTR);
   struct sockaddr *       saddr_ptr;
   uint16_t                udpflags;
   void *                  udpptr;
   uint32_t                udpword;
   TCPIP_EVENT             EXPIRE;
} UDP_PARM, * UDP_PARM_PTR;


/***************************************
**
** Prototypes
**
*/

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t UDP_init(void);
extern void UDP_open     (UDP_PARM_PTR);
extern void UDP_bind     (UDP_PARM_PTR);
extern void UDP_connect  (UDP_PARM_PTR);
extern void UDP_close    (UDP_PARM_PTR);
extern void UDP_send     (UDP_PARM_PTR);
extern void UDP_receive  (UDP_PARM_PTR);
extern void UDP_getopt   (UDP_PARM_PTR);
extern void UDP_setopt   (UDP_PARM_PTR);
extern void UDP_shutdown (UDP_PARM_PTR);

extern uint32_t UDP_openbind_internal
(
   uint16_t              localport,
   void (_CODE_PTR_     service)(RTCSPCB_PTR, UCB_STRUCT_PTR),
   UCB_STRUCT_PTR      *ucb
);

extern uint32_t UDP_close_internal
(
   UCB_STRUCT_PTR       ucb
);

extern uint32_t UDP_send_internal
(
  UCB_STRUCT_PTR    ucb,        /* [IN] UDP layer context      */
  sockaddr *        srcaddr,    /* [IN] source IPv6 address      */
  sockaddr *        destaddr,   /* [IN] destination IPv6 address and port */        
  RTCSPCB_PTR       pcb_ptr,    /* [IN] packet to send         */
  uint32_t          flags       /* [IN] optional flags         */
);

extern uint32_t UDP_send_IF
(
   UCB_STRUCT_PTR,
   void *,
   uint16_t,
   RTCSPCB_PTR
);

extern void UDP_service         /* Use function like service for UDP v4 v6*/
(
  RTCSPCB_PTR,
  void *
);

extern void UDP_process
(
   RTCSPCB_PTR,
   UCB_STRUCT_PTR
);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
