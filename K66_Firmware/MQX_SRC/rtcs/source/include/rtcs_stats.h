#ifndef __rtcs_stats_h__
#define __rtcs_stats_h__
/*HEADER**********************************************************************
*
* Copyright 2013 Freescale Semiconductor, Inc.
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
* See license agreement file for full license terms including other restrictions.
*****************************************************************************
*
* Comments:
*
*   RTCS Statistics User API.
*
*
*END************************************************************************/

/*
** Protocol statistics
*/

/*
** Common statistics kept across all protocols.
*/
typedef struct rtcs_stats_struct {
   uint32_t  ST_RX_TOTAL;
   uint32_t  ST_RX_MISSED;
   uint32_t  ST_RX_DISCARDED;
   uint32_t  ST_RX_ERRORS;

   uint32_t  ST_TX_TOTAL;
   uint32_t  ST_TX_MISSED;
   uint32_t  ST_TX_DISCARDED;
   uint32_t  ST_TX_ERRORS;
} RTCS_STATS_STRUCT, * RTCS_STATS_STRUCT_PTR;

/*
** The error structure embedded in statistics structures
*/
typedef struct rtcs_error_struct {
   uint32_t    ERROR;
   uint32_t    PARM;
   _task_id    TASK_ID;
   uint32_t    TASKCODE;
   void       *MEMPTR;
   bool     STACK;
} RTCS_ERROR_STRUCT, * RTCS_ERROR_STRUCT_PTR;

/*
** In the structures below, the following four fields are common:
**
** [RT]X_TOTAL      is the total number of packets sent/received
** [RT]X_MISSED     is the number of packets discarded due to lack of resources
** [RT]X_DISCARDED  is the number of packets discarded for all other reasons
** [RT]X_ERRORS     is the number of times an internal error occurred
**
** Internal errors are errors that should _never_ happen.  However,
** if one does occur, information is recorded in ERR_[RT]X.
*/
#if RTCSCFG_ENABLE_IPIF_STATS
typedef struct ipif_stats {
   RTCS_STATS_STRUCT COMMON;

   RTCS_ERROR_STRUCT ERR_RX;
   RTCS_ERROR_STRUCT ERR_TX;

   uint32_t  ST_RX_OCTETS;           /* total bytes received       */
   uint32_t  ST_RX_UNICAST;          /* unicast packets received   */
   uint32_t  ST_RX_MULTICAST;        /* multicast packets received */
   uint32_t  ST_RX_BROADCAST;        /* broadcast packets received */

   uint32_t  ST_TX_OCTETS;           /* total bytes sent           */
   uint32_t  ST_TX_UNICAST;          /* unicast packets sent       */
   uint32_t  ST_TX_MULTICAST;        /* multicast packets sent     */
   uint32_t  ST_TX_BROADCAST;        /* broadcast packets sent     */

} IPIF_STATS, * IPIF_STATS_PTR;
#endif

#if RTCSCFG_ENABLE_ARP_STATS
typedef struct arp_stats {
   RTCS_STATS_STRUCT COMMON;

   RTCS_ERROR_STRUCT ERR_RX;
   RTCS_ERROR_STRUCT ERR_TX;

   uint32_t  ST_RX_REQUESTS;         /* valid ARP requests received */
   uint32_t  ST_RX_REPLIES;          /* valid ARP replies received  */

   uint32_t  ST_TX_REQUESTS;         /* ARP requests sent           */
   uint32_t  ST_TX_REPLIES;          /* ARP replies received        */

   uint32_t  ST_ALLOCS_FAILED;       /* ARP_alloc returned NULL     */
   uint32_t  ST_CACHE_HITS;          /* ARP cache hits              */
   uint32_t  ST_CACHE_MISSES;        /* ARP cache misses            */
   uint32_t  ST_PKT_DISCARDS;        /* data packets discarded due  */
                                    /*    to missing ARP entry     */

} ARP_STATS, * ARP_STATS_PTR;
#endif

#if RTCSCFG_ENABLE_IP_STATS
typedef struct ip_stats {
   RTCS_STATS_STRUCT COMMON;

   RTCS_ERROR_STRUCT ERR_RX;
   RTCS_ERROR_STRUCT ERR_TX;

   uint32_t  ST_RX_HDR_ERRORS;       /* Discarded -- error in IP header    */
   uint32_t  ST_RX_ADDR_ERRORS;      /* Discarded -- illegal destination   */
   uint32_t  ST_RX_NO_PROTO;         /* Discarded -- unrecognized protocol */
   uint32_t  ST_RX_DELIVERED;        /* Datagrams delivered to upper layer */
   uint32_t  ST_RX_FORWARDED;        /* Datagrams forwarded                */

   /* These are included in ST_RX_DISCARDED and ST_RX_HDR_ERRORS */
   uint32_t  ST_RX_BAD_VERSION;      /* Datagrams with version != 4        */
   uint32_t  ST_RX_BAD_CHECKSUM;     /* Datagrams with invalid checksum    */
   uint32_t  ST_RX_BAD_SOURCE;       /* Datagrams with invalid src address */
   uint32_t  ST_RX_SMALL_HDR;        /* Datagrams with header too small    */
   uint32_t  ST_RX_SMALL_DGRAM;      /* Datagrams smaller than header      */
   uint32_t  ST_RX_SMALL_PKT;        /* Datagrams larger than frame        */
   uint32_t  ST_RX_TTL_EXCEEDED;     /* Datagrams to route with TTL = 0    */

   uint32_t  ST_RX_FRAG_RECVD;       /* Number of received IP fragments    */
   uint32_t  ST_RX_FRAG_REASMD;      /* Number of reassembled datagrams    */
   uint32_t  ST_RX_FRAG_DISCARDED;   /* Number of discarded fragments      */

   uint32_t  ST_TX_FRAG_SENT;        /* Number of sent fragments           */
   uint32_t  ST_TX_FRAG_FRAGD;       /* Number of fragmented datagrams     */
   uint32_t  ST_TX_FRAG_DISCARDED;   /* Number of fragmentation failures   */

} IP_STATS, * IP_STATS_PTR;
#endif

#if RTCSCFG_ENABLE_IP6_STATS
typedef struct ip6_stats {
   RTCS_STATS_STRUCT COMMON;

   RTCS_ERROR_STRUCT ERR_RX;
   RTCS_ERROR_STRUCT ERR_TX;

   uint32_t  ST_RX_HDR_ERRORS;       /* Discarded -- error in IP header    */
   uint32_t  ST_RX_ADDR_ERRORS;      /* Discarded -- illegal destination   */
   uint32_t  ST_RX_NO_PROTO;         /* Discarded -- unrecognized protocol */
   uint32_t  ST_RX_DELIVERED;        /* Datagrams delivered to upper layer */
   uint32_t  ST_RX_FORWARDED;        /* Datagrams forwarded                */

   /* These are included in ST_RX_DISCARDED and ST_RX_HDR_ERRORS */
   uint32_t  ST_RX_BAD_VERSION;      /* Datagrams with version != 4        */
   uint32_t  ST_RX_BAD_SOURCE;       /* Datagrams with invalid src address */
   uint32_t  ST_RX_SMALL_PKT;        /* Datagrams larger than frame        */
   uint32_t  ST_RX_TTL_EXCEEDED;     /* Datagrams to route with TTL = 0    */

   uint32_t  ST_RX_FRAG_RECVD;       /* Number of received IP fragments    */
   uint32_t  ST_RX_FRAG_REASMD;      /* Number of reassembled datagrams    */
   uint32_t  ST_RX_FRAG_DISCARDED;   /* Number of discarded fragments      */

   uint32_t  ST_TX_FRAG_SENT;        /* Number of sent fragments           */
   uint32_t  ST_TX_FRAG_FRAGD;       /* Number of fragmented datagrams     */
   uint32_t  ST_TX_FRAG_DISCARDED;   /* Number of fragmentation failures   */

} IP6_STATS, * IP6_STATS_PTR;
#endif

#if RTCSCFG_ENABLE_ICMP_STATS
typedef struct icmp_stats {
   RTCS_STATS_STRUCT COMMON;

   RTCS_ERROR_STRUCT ERR_RX;
   RTCS_ERROR_STRUCT ERR_TX;

   /* These are included in ST_RX_DISCARDED */
   uint32_t  ST_RX_BAD_CODE;         /* Datagrams with unrecognized code     */
   uint32_t  ST_RX_BAD_CHECKSUM;     /* Datagrams with invalid checksum      */
   uint32_t  ST_RX_SMALL_DGRAM;      /* Datagrams smaller than header        */
   uint32_t  ST_RX_RD_NOTGATE;       /* Redirects received from non-gateway  */

   /* Statistics on each ICMP type */
   uint32_t  ST_RX_DESTUNREACH;      /* Received Destination Unreachables    */
   uint32_t  ST_RX_TIMEEXCEED;       /* Received Time Exceededs              */
   uint32_t  ST_RX_PARMPROB;         /* Received Parameter Problems          */
   uint32_t  ST_RX_SRCQUENCH;        /* Received Source Quenches             */

   uint32_t  ST_RX_REDIRECT;         /* Received Redirects                   */
   uint32_t  ST_RX_ECHO_REQ;         /* Received Echo Requests               */
   uint32_t  ST_RX_ECHO_REPLY;       /* Received Echo Replys                 */
   uint32_t  ST_RX_TIME_REQ;         /* Received Timestamp Requests          */

   uint32_t  ST_RX_TIME_REPLY;       /* Received Timestamp Replys            */
   uint32_t  ST_RX_INFO_REQ;         /* Received Information Requests        */
   uint32_t  ST_RX_INFO_REPLY;       /* Received Information Replys          */
   uint32_t  ST_RX_OTHER;            /* All other types                      */

   uint32_t  ST_TX_DESTUNREACH;      /* Transmitted Destination Unreachables */
   uint32_t  ST_TX_TIMEEXCEED;       /* Transmitted Time Exceededs           */
   uint32_t  ST_TX_PARMPROB;         /* Transmitted Parameter Problems       */
   uint32_t  ST_TX_SRCQUENCH;        /* Transmitted Source Quenches          */

   uint32_t  ST_TX_REDIRECT;         /* Transmitted Redirects                */
   uint32_t  ST_TX_ECHO_REQ;         /* Transmitted Echo Requests            */
   uint32_t  ST_TX_ECHO_REPLY;       /* Transmitted Echo Replys              */
   uint32_t  ST_TX_TIME_REQ;         /* Transmitted Timestamp Requests       */

   uint32_t  ST_TX_TIME_REPLY;       /* Transmitted Timestamp Replys         */
   uint32_t  ST_TX_INFO_REQ;         /* Transmitted Information Requests     */
   uint32_t  ST_TX_INFO_REPLY;       /* Transmitted Information Replys       */
   uint32_t  ST_TX_OTHER;            /* All other types                      */

} ICMP_STATS, * ICMP_STATS_PTR;
#endif

#if RTCSCFG_ENABLE_ICMP6_STATS
typedef struct icmp6_stats {
   RTCS_STATS_STRUCT COMMON;

   RTCS_ERROR_STRUCT ERR_RX;
   RTCS_ERROR_STRUCT ERR_TX;

   /* These are included in ST_RX_DISCARDED */
   uint32_t  ST_RX_BAD_CHECKSUM;     /* Datagrams with invalid checksum      */
   uint32_t  ST_RX_SMALL_DGRAM;      /* Datagrams smaller than header        */

   /* Statistics on each ICMP type */
   uint32_t  ST_RX_DESTUNREACH;      /* Received Destination Unreachables    */
   uint32_t  ST_RX_TIMEEXCEED;       /* Received Time Exceededs              */
   uint32_t  ST_RX_PARMPROB;         /* Received Parameter Problems          */
   uint32_t  ST_RX_REDIRECT;         /* Received Redirects                   */
   uint32_t  ST_RX_ECHO_REQ;         /* Received Echo Requests               */
   uint32_t  ST_RX_ECHO_REPLY;       /* Received Echo Replys                 */
   uint32_t  ST_RX_OTHER;            /* All other types                      */
   uint32_t  ST_TX_DESTUNREACH;      /* Transmitted Destination Unreachables */
   uint32_t  ST_TX_TIMEEXCEED;       /* Transmitted Time Exceededs           */
   uint32_t  ST_TX_PARMPROB;         /* Transmitted Parameter Problems       */
   uint32_t  ST_TX_REDIRECT;         /* Transmitted Redirects                */
   uint32_t  ST_TX_ECHO_REQ;         /* Transmitted Echo Requests            */
   uint32_t  ST_TX_ECHO_REPLY;       /* Transmitted Echo Replys              */
   uint32_t  ST_TX_OTHER;            /* All other types                      */

} ICMP6_STATS, * ICMP6_STATS_PTR;
#endif

#if RTCSCFG_ENABLE_IGMP_STATS
typedef struct igmp_stats {
   RTCS_STATS_STRUCT COMMON;

   RTCS_ERROR_STRUCT ERR_RX;
   RTCS_ERROR_STRUCT ERR_TX;

   uint32_t  ST_RX_BAD_TYPE;         /* Datagrams with unrecognized code     */
   uint32_t  ST_RX_BAD_CHECKSUM;     /* Datagrams with invalid checksum      */
   uint32_t  ST_RX_SMALL_DGRAM;      /* Datagrams smaller than header        */

   uint32_t  ST_RX_QUERY;            /* Received Queries                     */
   uint32_t  ST_RX_REPORT;           /* Received Reports                     */

   uint32_t  ST_TX_QUERY;            /* Transmitted Queries                  */
   uint32_t  ST_TX_REPORT;           /* Transmitted Reports                  */

} IGMP_STATS, * IGMP_STATS_PTR;
#endif

#if RTCSCFG_ENABLE_UDP_STATS
typedef struct udp_stats {
   RTCS_STATS_STRUCT COMMON;

   RTCS_ERROR_STRUCT ERR_RX;
   RTCS_ERROR_STRUCT ERR_TX;

   /* These are all included in ST_RX_DISCARDED */
   uint32_t  ST_RX_BAD_PORT;         /* Datagrams with dest port zero   */
   uint32_t  ST_RX_BAD_CHECKSUM;     /* Datagrams with invalid checksum */
   uint32_t  ST_RX_SMALL_DGRAM;      /* Datagrams smaller than header   */
   uint32_t  ST_RX_SMALL_PKT;        /* Datagrams larger than frame     */
   uint32_t  ST_RX_NO_PORT;          /* Datagrams for a closed port     */

} UDP_STATS, * UDP_STATS_PTR;
#endif

#if RTCSCFG_ENABLE_TCP_STATS
typedef struct tcp_stats {
   RTCS_STATS_STRUCT COMMON;

   RTCS_ERROR_STRUCT ERR_RX;
   RTCS_ERROR_STRUCT ERR_TX;

   /* These are all included in ST_RX_DISCARDED */
   uint32_t  ST_RX_BAD_PORT;         /* Segments with dest port zero   */
   uint32_t  ST_RX_BAD_CHECKSUM;     /* Segments with invalid checksum */
   uint32_t  ST_RX_BAD_OPTION;       /* Segments with invalid options  */
   uint32_t  ST_RX_BAD_SOURCE;       /* Segments with invalid source   */
   uint32_t  ST_RX_SMALL_HDR;        /* Segments with header too small */
   uint32_t  ST_RX_SMALL_DGRAM;      /* Segments smaller than header   */
   uint32_t  ST_RX_SMALL_PKT;        /* Segments larger than frame     */
   uint32_t  ST_RX_BAD_ACK;          /* Received ack for unsent data   */
   uint32_t  ST_RX_BAD_DATA;         /* Received data outside window   */
   uint32_t  ST_RX_LATE_DATA;        /* Received data after close      */
   uint32_t  ST_RX_OPT_MSS;          /* Segments with MSS option set   */
   uint32_t  ST_RX_OPT_OTHER;        /* Segments with other options    */

   uint32_t  ST_RX_DATA;             /* Data segments received         */
   uint32_t  ST_RX_DATA_DUP;         /* Duplicate data received        */
   uint32_t  ST_RX_ACK;              /* Acks received                  */
   uint32_t  ST_RX_ACK_DUP;          /* Duplicate acks received        */
   uint32_t  ST_RX_RESET;            /* RST segments received          */
   uint32_t  ST_RX_PROBE;            /* Window probes received         */
   uint32_t  ST_RX_WINDOW;           /* Window updates received        */

   uint32_t  ST_RX_SYN_EXPECTED;     /* Expected SYN, not received     */
   uint32_t  ST_RX_ACK_EXPECTED;     /* Expected ACK, not received     */
   uint32_t  ST_RX_SYN_NOT_EXPECTED; /* Received SYN, not expected     */
   uint32_t  ST_RX_MULTICASTS;       /* Multicast packets              */
   uint32_t  ST_RX_BROADCASTS;       /* Broadcast packets              */


   uint32_t  ST_TX_DATA;             /* Data segments sent             */
   uint32_t  ST_TX_DATA_DUP;         /* Data segments retransmitted    */
   uint32_t  ST_TX_ACK;              /* Ack-only segments sent         */
   uint32_t  ST_TX_ACK_DELAYED;      /* Delayed acks sent              */
   uint32_t  ST_TX_RESET;            /* RST segments sent              */
   uint32_t  ST_TX_PROBE;            /* Window probes sent             */
   uint32_t  ST_TX_WINDOW;           /* Window updates sent            */

   uint32_t  ST_CONN_ACTIVE;         /* Active opens                   */
   uint32_t  ST_CONN_PASSIVE;        /* Passive opens                  */
   uint32_t  ST_CONN_OPEN;           /* Established connections        */
   uint32_t  ST_CONN_CLOSED;         /* Graceful shutdowns             */
   uint32_t  ST_CONN_RESET;          /* Ungraceful shutdowns           */
   uint32_t  ST_CONN_FAILED;         /* Failed opens                   */
   uint32_t  ST_CONN_ABORTS;         /* Aborts                         */

} TCP_STATS, * TCP_STATS_PTR;
#endif

#if RTCSCFG_ENABLE_RIP_STATS
typedef struct rip_stats {
   RTCS_STATS_STRUCT COMMON;

   RTCS_ERROR_STRUCT ERR_RX;
   RTCS_ERROR_STRUCT ERR_TX;

} RIP_STATS, * RIP_STATS_PTR;
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if RTCSCFG_ENABLE_IP_STATS
extern IP_STATS_PTR   IP_stats   (void);
#endif
#if RTCSCFG_ENABLE_ICMP_STATS
extern ICMP_STATS_PTR ICMP_stats (void);
#endif
#if RTCSCFG_ENABLE_UDP_STATS
extern UDP_STATS_PTR  UDP_stats  (void);
#endif
#if RTCSCFG_ENABLE_TCP_STATS
extern TCP_STATS_PTR  TCP_stats  (void);
#endif
#if RTCSCFG_ENABLE_IPIF_STATS
extern IPIF_STATS_PTR IPIF_stats (_rtcs_if_handle);
#endif
#if RTCSCFG_ENABLE_ARP_STATS
extern ARP_STATS_PTR  ARP_stats  (_rtcs_if_handle);
#endif
#if RTCSCFG_ENABLE_IP6_STATS
extern IP6_STATS_PTR   IP6_stats (void);
#endif
#if RTCSCFG_ENABLE_ICMP6_STATS
extern ICMP6_STATS_PTR ICMP6_stats  (void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __rtcs_stats_h__ */


/* EOF */
