#ifndef __igmp_h__
#define __igmp_h__
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
*   This file contains the Internet Group Mangement protocol
*   User Datagram Protocol definitions.
*   For more details, refer to RFC1112 and RFC2236.
*
*
*END************************************************************************/


#if 0
#  define IGMP_V2      /* to define if the host has to support IGMPv2 */
#endif

#if RTCSCFG_ENABLE_IGMP & RTCSCFG_ENABLE_IP4

/***************************************
**
** Constants
**
*/

/* IGMP packet types */
#define IGMPTYPE_QUERY        0x11
#define IGMPTYPE_V1_REPORT    0x12
#define IGMPTYPE_V2_REPORT    0x16
#define IGMPTYPE_LEAVE        0x17

/* IGMP constants */
#define IGMP_V1_ROUTER_TIMEOUT_VALUE     (400*1000)
#define IGMP_V1_QUERY_RESPONSE_INTERVAL  ( 10*1000)   /* default max_resp_time in 1/1000 sec fro IGMPv1 */
#define IGMP_UNSOLICITED_REPORT_INTERVAL ( 10*1000)
#define IGMP_UNSOLICITED_REPORT_COUNT    2      /* number of repetition. if == 2, 3 reports will be sent */


/***************************************
**
** Code macros
**
*/

#if RTCSCFG_ENABLE_IGMP_STATS
#define IF_IGMP_STATS_ENABLED(x) x
#else
#define IF_IGMP_STATS_ENABLED(x)
#endif

/***************************************
**
** Type definitions
**
*/

/*
** IGMP parameters for [gs]etsockopt
*/
typedef struct igmp_parm_set {
   TCPIP_PARM              COMMON;
   struct igmp_parm       *NEXT;
   SOCKET_STRUCT_PTR       sock;
   uint32_t                 option;   
   const void * option_value;
   uint32_t                 optlen;
} IGMP_PARM_SET, * IGMP_PARM_SET_PTR;

typedef struct igmp_parm_get {
   TCPIP_PARM              COMMON;
   struct igmp_parm       *NEXT;
   SOCKET_STRUCT_PTR       sock;
   uint32_t                 option;
   void * option_value;
   uint32_t                 optlen;
} IGMP_PARM_GET, * IGMP_PARM_GET_PTR;

/*
** IGMP Configuration.  This information is persistent for the IGMP layer.
*/
typedef struct igmp_cfg_struct {
#if RTCSCFG_ENABLE_IGMP_STATS
   IGMP_STATS     STATS;         /* the statistic for IGMP */
#else
   unsigned char          Reserved;
#endif
} IGMP_CFG_STRUCT, * IGMP_CFG_STRUCT_PTR;

typedef struct igmp_header {
   unsigned char    TYPE[1];
   unsigned char    MAX_RESP_TIME[1];
   unsigned char    CHECKSUM[2];
   unsigned char    GROUP_ADDRESS[4];
} IGMP_HEADER, * IGMP_HEADER_PTR;


/***************************************
**
** Prototypes
**
*/

struct ip_if;

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t IGMP_init (void);
extern bool IGMP_send_report (const ip_mreq *, uint16_t);

extern uint32_t IGMP_filter_add (struct ip_if *, _ip_address);
extern uint32_t IGMP_filter_rm  (struct ip_if *, _ip_address);

extern MC_MEMBER_PTR      *IGMP_member_find   (MC_MEMBER_PTR *, const ip_mreq *);
extern MC_MEMBER_PTR      *IGMP_member_create (MC_MEMBER_PTR *, const ip_mreq *);
extern uint32_t             IGMP_member_delete (MC_MEMBER_PTR *);

extern void IGMP_init_timer   (MC_MEMBER_PTR);
extern void IGMP_launch_timer (MC_MEMBER_PTR, uint32_t);
extern void IGMP_stop_timer   (MC_MEMBER_PTR);

extern uint32_t IGMP_ipif_add  (struct ip_if *);
extern uint32_t IGMP_ipif_bind (struct ip_if *, _ip_address);

extern bool IGMP_is_member
(
   MC_MEMBER_PTR   *phead,      /* [IN] head of the MCB list */
   struct ip_if    *ipif,       /* [IN] the incoming interface */
   _ip_address          multiaddr   /* [IN] the multicast ip */
);

extern uint32_t IGMP_join_group  (MC_MEMBER_PTR *, const ip_mreq *, uint32_t (_CODE_PTR_ *)(MC_MEMBER_PTR *));
extern uint32_t IGMP_leave_group (MC_MEMBER_PTR *, const ip_mreq *, uint32_t (_CODE_PTR_ *)(MC_MEMBER_PTR *));

#ifdef __cplusplus
}
#endif


#endif
#endif
/* EOF */
