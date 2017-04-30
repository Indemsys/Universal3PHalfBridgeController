#ifndef __rtcs_sock_h__
#define __rtcs_sock_h__
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
*   RTCS Socket API.
*
*
*END************************************************************************/

/*
** Socket types
*/
#define SOCK_DGRAM      ((uint32_t)&SOCK_DGRAM_CALL)
#define SOCK_STREAM     ((uint32_t)&SOCK_STREAM_CALL)

/*
** Validity check for socket structures
*/
#define SOCKET_VALID       0x52544353  /* "rtcs" */

/*
** Default number of tasks that can simultaneously own a socket
*/
#if RTCSCFG_BACKWARD_COMPATIBILITY_RTCSSELECT==1
#define SOCKET_NUMOWNERS   8
#endif

/*
** protocol level at which option resides
*/
#define SOL_SOCKET      1
#define SOL_UDP         2
#define SOL_TCP         3
#define SOL_IP          4
#define SOL_IGMP        5
#define SOL_LINK        6
#define SOL_IP6         7

/* SOL_SOCKET options */
#define SO_SNDTIMEO                    100
#define SO_RCVTIMEO                    101
#define SO_ERROR                       102
#define SO_KEEPALIVE                   103
#define SO_RCVNUM                      104
#define SO_LINGER                      105
#define SO_EXCEPTION                   106
  /* backward compatible OPT_ at SOL_SOCKET level */
  #define OPT_SOCKET_ERROR               SO_ERROR
  #define OPT_SOCKET_TYPE                130
  

/* SOL_TCP options */
#define TCP_KEEPIDLE                   200
#define TCP_KEEPINTVL                  201
#define TCP_KEEPCNT                    202
  /* backward compatible OPT_ at SOL_TCP level */
  #define OPT_DELAY_ACK                  230
  #define OPT_TCPSECUREDRAFT_0           231
  #define OPT_TIMEWAIT_TIMEOUT           232
  #define OPT_NOSWRBUF                   233
  #define OPT_NO_NAGLE_ALGORITHM         234
  #define OPT_NOWAIT                     235
  #define OPT_KEEPALIVE                  236
  #define OPT_MAXRCV_WND                 237
  #define OPT_MAXRTO                     238
  #define OPT_TBSIZE                     240
  #define OPT_SEND_PUSH                  241
  #define OPT_RECEIVE_PUSH               242
  #define OPT_RECEIVE_TIMEOUT            243
  #define OPT_SEND_TIMEOUT               244
  #define OPT_RETRANSMISSION_TIMEOUT     245
  #define OPT_CONNECT_TIMEOUT            246

/* SOL_UDP */
  #define OPT_CHECKSUM_BYPASS            300

/* socket options that should be supported at SOL_TCP and SOL_UDP levels 
 * 
 */  
  #define OPT_SEND_NOWAIT                500
  #define OPT_RECEIVE_NOWAIT             501
  #define OPT_RBSIZE                     502

/*
**   default option values
*/
/* per RFC 1122 4.2.3.5, R2 for a SYN segment MUST be set large enough to provide retransmission of the segment for at least 3 minutes. */
/* note: CONNECT_TIMEOUT is effectively used for R2 value in milliseconds inside the RTCS TCP code */
#define DEFAULT_CONNECT_TIMEOUT        (180000L)   /* 3 mins */
#define DEFAULT_RETRANSMISSION_TIMEOUT (3000)      /* 3 sec  */
#define DEFAULT_SEND_TIMEOUT           (DEFAULT_CONNECT_TIMEOUT/2)
#define DEFAULT_RECEIVE_TIMEOUT        (0)         /* no timeout */
#define DEFAULT_NOWAIT                 FALSE
#define DEFAULT_TCP_SEND_NOWAIT        FALSE
#define DEFAULT_UDP_SEND_NOWAIT        TRUE        /* UDP non-blocking by default */
#define DEFAULT_RECEIVE_NOWAIT         FALSE
#define DEFAULT_WAIT                   FALSE       /* don't wait for ack */
#define DEFAULT_PUSH                   TRUE        /* push */
#define DEFAULT_CHECKSUM_BYPASS        FALSE       /* perform checksum */
#define DEFAULT_TBSIZE                 (-1)
#define DEFAULT_RBSIZE                 (-1)
#define DEFAULT_UDP_RBSIZE             (RTCSCFG_UDP_RX_BUFFER_SIZE)
#define DEFAULT_MAXRTO                 (0)
#define DEFAULT_MAXRCV_WND             (0)
#define DEFAULT_KEEPALIVE              (0)
#define DEFAULT_NO_NAGLE_ALGORITHM     TRUE        /* do not use Nagle by default */
#define DEFAULT_NOSWRBUF               FALSE
#define DEFAULT_TIMEWAIT_TIMEOUT       (2*1000) /* msec */ /* 0 = default 4 mins.*/
#define DEFAULT_TCPSECUREDRAFT_0       FALSE
#define DEFAULT_DELAY_ACK              (TCP_ACKDELAY)
#define DEFAULT_IP6_MULTICAST_HOPS     (1)
#define DEFAULT_KEEPIDLE               (2*60*60) /* 2 hours, in seconds */
#define DEFAULT_KEEPINTVL              (75)      /* in seconds */
#define DEFAULT_KEEPCNT                (8)       /* send 8 keepalives before connection drop */

/*
** Socket options
*/
#define RTCS_SO_TYPE                   OPT_SOCKET_TYPE
#define RTCS_SO_ERROR                  OPT_SOCKET_ERROR

#define RTCS_SO_IGMP_ADD_MEMBERSHIP    600
#define RTCS_SO_IGMP_DROP_MEMBERSHIP   601
#define RTCS_SO_IGMP_GET_MEMBERSHIP    602

#define RTCS_SO_IP_RX_DEST             620
#define RTCS_SO_IP_RX_TTL              621
#define RTCS_SO_IP_RX_TOS              622
#define RTCS_SO_IP_TX_TTL              623
#define RTCS_SO_IP_LOCAL_ADDR          624
#define RTCS_SO_IP_TX_TOS              625

#define RTCS_SO_LINK_TX_8023           640
#define RTCS_SO_LINK_TX_8021Q_PRIO     641
#define RTCS_SO_LINK_RX_8023           642
#define RTCS_SO_LINK_RX_8021Q_PRIO     643
#define RTCS_SO_LINK_TX_8021Q_VID      644
#define RTCS_SO_LINK_RX_8021Q_VID      645

#define RTCS_SO_UDP_NONBLOCK           OPT_SEND_NOWAIT
#define RTCS_SO_UDP_NONBLOCK_TX        OPT_SEND_NOWAIT
#define RTCS_SO_UDP_NONBLOCK_RX        OPT_RECEIVE_NOWAIT
#define RTCS_SO_UDP_NOCHKSUM           OPT_CHECKSUM_BYPASS

#define RTCS_SO_NAT_TIMEOUTS           700
#define RTCS_SO_NAT_PORTS              701
/* SOL_IP6 Options.*/
#define RTCS_SO_IP6_UNICAST_HOPS       720
#define RTCS_SO_IP6_MULTICAST_HOPS     721
#define RTCS_SO_IP6_JOIN_GROUP         722
#define RTCS_SO_IP6_LEAVE_GROUP        723


/*
** Flags for send[to]/recv[from]
*/
#define RTCS_MSG_O_NONBLOCK   0x0001
#define RTCS_MSG_S_NONBLOCK   0x0002
#define RTCS_MSG_BLOCK        (RTCS_MSG_O_NONBLOCK)
#define RTCS_MSG_NONBLOCK     (RTCS_MSG_O_NONBLOCK | RTCS_MSG_S_NONBLOCK)
#define RTCS_MSG_O_NOCHKSUM   0x0004
#define RTCS_MSG_S_NOCHKSUM   0x0008
#define RTCS_MSG_CHKSUM       (RTCS_MSG_O_NOCHKSUM)
#define RTCS_MSG_NOCHKSUM     (RTCS_MSG_O_NOCHKSUM | RTCS_MSG_S_NOCHKSUM)
#define RTCS_MSG_PEEK         0x0010
#define RTCS_MSG_NOLOOP       0x0020
/* options for stream socket */
#define MSG_DONTWAIT          (0x0100)
/* send() only: */
#define MSG_WAITACK           (0x0200)
/* recv() only: */
#define MSG_WAITALL           (0x0400)

/*
** Close methods for shutdown()
*/
#define FLAG_ABORT_CONNECTION       (0x0010)
#define FLAG_CLOSE_TX               (0x0001)

/* Types for compatibility.*/
typedef uint32_t    socklen_t;
typedef	uint16_t    sa_family_t;

/*
** IP address definition
*/
typedef uint32_t _ip_address;

typedef struct in_addr {
   _ip_address s_addr;
} in_addr;

/*
** IP multicast group
*/
typedef struct ip_mreq {
   in_addr  imr_multiaddr;
   in_addr  imr_interface;
} ip_mreq;

/*
** Socket Address Structure
*/
typedef struct sockaddr_in {
   uint16_t  sin_family;
   uint16_t  sin_port;
   in_addr  sin_addr;
} sockaddr_in;


#if RTCSCFG_ENABLE_IP6
    typedef struct sockaddr
    {
        uint16_t	sa_family;
        char 		sa_data[22];
    } sockaddr;
#else
    #if RTCSCFG_ENABLE_IP4
        #define sockaddr   sockaddr_in
        #define sa_family   sin_family
    #endif
#endif

/****************************************************************
* IPv6 address definition (BSD-like).
*****************************************************************/
typedef struct in6_addr
{
    union
    {
        uint8_t     __u6_addr8[16];
        uint16_t    __u6_addr16[8];
        uint32_t 	__u6_addr32[4];
    } __u6_addr;			/* 128-bit IP6 address */
}in6_addr;
#define s6_addr   __u6_addr.__u6_addr8
/* Not RFC */
#define s6_addr8  __u6_addr.__u6_addr8
#define s6_addr16 __u6_addr.__u6_addr16
#define s6_addr32 __u6_addr.__u6_addr32

/****************************************************************
* Socket address for IPv6 (BSD-like).
*****************************************************************/
typedef struct sockaddr_in6
{
    uint16_t    sin6_family;    /* AF_INET6 */
    uint16_t    sin6_port;      /* Transport layer port # */
    in6_addr	sin6_addr;      /* IP6 address */
    uint32_t    sin6_scope_id;  /* scope zone index */
}sockaddr_in6;


/*
** IPv6 multicast group
*/
typedef struct ipv6_mreq
{
    in6_addr      ipv6imr_multiaddr; /* IPv6 multicast address of group. */
    unsigned int  ipv6imr_interface; /* Interface index. It equals to the scope zone index, defining network interface.*/
} ipv6_mreq;


/****************************************************************
*
* Helpful macros.
*
*****************************************************************/
#define RTCS_SOCKADDR_PORT(a)   (((sockaddr_in*)(a))->sin_port)
#define RTCS_SOCKADDR_ADDR(a)   (((sockaddr_in*)(a))->sin_addr)


#define IN6ADDR(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16) \
    	    { (a1), (a2), (a3), (a4), (a5), (a6), (a7), (a8),       \
            (a9), (a10), (a11), (a12), (a13), (a14), (a15), (a16) }

/*
 * Construct an IPv6 address from eight 16-bit words.
*/
#define in6addr(a, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15)    \
                         {                                                                  \
                            (a)->s6_addr[0] = a0;                                           \
                            (a)->s6_addr[1] = a1;                                           \
                            (a)->s6_addr[2] = a2;                                           \
                            (a)->s6_addr[3] = a3;                                           \
                            (a)->s6_addr[4] = a4;                                           \
                            (a)->s6_addr[5] = a5;                                           \
                            (a)->s6_addr[6] = a6;                                           \
                            (a)->s6_addr[7] = a7;                                           \
                            (a)->s6_addr[8] = a8;                                           \
                            (a)->s6_addr[9] = a9;                                           \
                            (a)->s6_addr[10] = a10;                                         \
                            (a)->s6_addr[11] = a11;                                         \
                            (a)->s6_addr[12] = a12;                                         \
                            (a)->s6_addr[13] = a13;                                         \
                            (a)->s6_addr[14] = a14;                                         \
                            (a)->s6_addr[15] = a15;                                         \
                        }

/*
* Definition of some useful macros to handle IP6 addresses (BSD-like)
*/
#define IN6ADDR_ANY_INIT                                    \
    {{{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }}}
#define IN6ADDR_LOOPBACK_INIT                               \
    {{{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }}}
#define IN6ADDR_NODELOCAL_ALLNODES_INIT                     \
    {{{ 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }}}
#define IN6ADDR_INTFACELOCAL_ALLNODES_INIT                  \
    {{{ 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }}}
#define IN6ADDR_LINKLOCAL_ALLNODES_INIT                     \
    {{{ 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }}}
#define IN6ADDR_LINKLOCAL_ALLROUTERS_INIT                   \
    {{{ 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02 }}}
#define IN6ADDR_LINKLOCAL_ALLV2ROUTERS_INIT                 \
    {{{ 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16 }}}

extern const struct in6_addr in6addr_any;
extern const struct in6_addr in6addr_loopback;
extern const struct in6_addr in6addr_nodelocal_allnodes;
extern const struct in6_addr in6addr_linklocal_allnodes;
extern const struct in6_addr in6addr_linklocal_allrouters;
extern const struct in6_addr in6addr_linklocal_allv2routers;

/*
* Equality (BSD-like)
*/
#define IN6_ARE_ADDR_EQUAL(a, b)			\
        (memcmp(&(a)->s6_addr[0], &(b)->s6_addr[0], sizeof(struct in6_addr)) == 0)

/*
* Copying address
*/
extern void IN6_ADDR_COPY(const in6_addr *from_addr, in6_addr *to_addr);


/*
 * Unspecified (BSD-like)
 */
#define IN6_IS_ADDR_UNSPECIFIED(a)	                                \
    ((*(const uint32_t *)(const void *)(&(a)->s6_addr[0]) == 0) &&	\
    (*(const uint32_t *)(const void *)(&(a)->s6_addr[4]) == 0) &&	\
    (*(const uint32_t *)(const void *)(&(a)->s6_addr[8]) == 0) &&	\
    (*(const uint32_t *)(const void *)(&(a)->s6_addr[12]) == 0))

/*
 * Loopback (BSD-like)
 */
#define IN6_IS_ADDR_LOOPBACK(a)	                                	\
    ((*(const uint32_t *)(const void *)(&(a)->s6_addr[0]) == 0) &&	\
    (*(const uint32_t *)(const void *)(&(a)->s6_addr[4]) == 0) &&	\
    (*(const uint32_t *)(const void *)(&(a)->s6_addr[8]) == 0) &&	\
    (*(const uint32_t *)(const void *)(&(a)->s6_addr[12]) == mqx_ntohl(1)))

/*
 * Multicast (BSD-like)
 */
#define IN6_IS_ADDR_MULTICAST(a)	((a)->s6_addr[0] == 0xff)

/*
 * Unicast Scope (BSD-like)
 * Note that we must check topmost 10 bits only, not 16 bits (see RFC2373).
 */
#define IN6_IS_ADDR_LINKLOCAL(a)	\
    	(((a)->s6_addr[0] == 0xfe) && (((a)->s6_addr[1] & 0xc0) == 0x80))
#define IN6_IS_ADDR_SITELOCAL(a)	\
    	(((a)->s6_addr[0] == 0xfe) && (((a)->s6_addr[1] & 0xc0) == 0xc0))

/* IPv4 address string size. */
#define RTCS_IP4_ADDR_STR_SIZE sizeof("255.255.255.255")

/* IPv6 address string size. */
#define RTCS_IP6_ADDR_STR_SIZE sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")

/* Scope ID size. */
#define RTCS_SCOPEID_STR_SIZE sizeof("%4294967295")

/*
** Type definitions for socket()
*/

typedef struct rtcs_socket_call_struct {
   uint32_t (_CODE_PTR_ SOCK_SOCKET)      (uint32_t);
   uint32_t (_CODE_PTR_ SOCK_BIND)        (uint32_t, const sockaddr*, uint16_t);
   uint32_t (_CODE_PTR_ SOCK_CONNECT)     (uint32_t, const sockaddr*, uint16_t);
   uint32_t (_CODE_PTR_ SOCK_LISTEN)      (uint32_t, int32_t);
   uint32_t (_CODE_PTR_ SOCK_ACCEPT)      (uint32_t, sockaddr*, uint16_t*);
   uint32_t (_CODE_PTR_ SOCK_GETSOCKNAME) (uint32_t, sockaddr*, uint16_t*);
   uint32_t (_CODE_PTR_ SOCK_GETPEERNAME) (uint32_t, sockaddr*, uint16_t*);
    int32_t (_CODE_PTR_ SOCK_RECV)        (uint32_t, void*, uint32_t, uint32_t);
    int32_t (_CODE_PTR_ SOCK_RECVFROM)    (uint32_t, void*, uint32_t, uint32_t, sockaddr*, uint16_t*);
    int32_t (_CODE_PTR_ SOCK_RECVMSG)     (uint32_t, void*, uint32_t);
    int32_t (_CODE_PTR_ SOCK_SEND)        (uint32_t, void*, uint32_t, uint32_t);
    int32_t (_CODE_PTR_ SOCK_SENDTO)      (uint32_t, void*, uint32_t, uint32_t, sockaddr*, uint16_t);
    int32_t (_CODE_PTR_ SOCK_SENDMSG)     (uint32_t, void*, uint32_t);
   uint32_t (_CODE_PTR_ SOCK_SOCKATMARK)  (uint32_t);
   uint32_t (_CODE_PTR_ SOCK_SHUTDOWN)    (uint32_t, uint32_t);
} RTCS_SOCKET_CALL_STRUCT, * RTCS_SOCKET_CALL_STRUCT_PTR;

extern const RTCS_SOCKET_CALL_STRUCT  SOCK_DGRAM_CALL;
extern const RTCS_SOCKET_CALL_STRUCT  SOCK_STREAM_CALL;

/*
** Type definitions for [gs]etsockopt()
*/
typedef struct rtcs_sockopt_call_struct  {
   uint32_t (_CODE_PTR_ SOCK_GETSOCKOPT)  (uint32_t, uint32_t, uint32_t, void*, uint32_t*);
   uint32_t (_CODE_PTR_ SOCK_SETSOCKOPT)  (uint32_t, uint32_t, uint32_t, const void*, uint32_t);
} RTCS_SOCKOPT_CALL_STRUCT, * RTCS_SOCKOPT_CALL_STRUCT_PTR;

extern const RTCS_SOCKOPT_CALL_STRUCT  SOL_IP_CALL;
extern const RTCS_SOCKOPT_CALL_STRUCT  SOL_IGMP_CALL;
extern const RTCS_SOCKOPT_CALL_STRUCT  SOL_LINK_CALL;

#if RTCSCFG_ENABLE_IP6
extern const RTCS_SOCKOPT_CALL_STRUCT  SOL_IP6_CALL;
#endif

/*
** The socket state structure
*/

typedef struct rtcs_linkopt_struct
{
    unsigned    OPT_8023 : 1;
    unsigned    OPT_PRIO : 1;
    unsigned    OPT_VID  : 1;
    _ip_address DEST;
    unsigned char TTL;
    unsigned char TOS;
    unsigned char RESERVED[2];
#if RTCSCFG_ENABLE_IP6 /* IPv6 Options.*/
    unsigned char HOP_LIMIT_UNICAST;    /* Unicast hop limit.*/
    unsigned char HOP_LIMIT_MULTICAST;  /* Multicast hop limit.*/
#endif
#if RTCSCFG_ENABLE_8021Q
    uint32_t PRIO;  /* Is a 3-bit field called the Priority Code Point (PCP) 
                     * within an Ethernet frame header when using VLAN tagged frames
                     * as defined by IEEE 802.1Q. It specifies a priority value of between 
                     * 0 and 7 inclusive that can be used by QoS disciplines to differentiate traffic.*/
    uint32_t VID;   /* VLAN Identifier (VID): a 12-bit field specifying the VLAN 
                     * to which the frame belongs.*/
#endif
} RTCS_LINKOPT_STRUCT, * RTCS_LINKOPT_STRUCT_PTR;

#if RTCSCFG_BACKWARD_COMPATIBILITY_RTCSSELECT==1
typedef struct socket_owner_struct {
   struct socket_owner_struct  *NEXT;
   _rtcs_taskid                 TASK[SOCKET_NUMOWNERS];
} SOCKET_OWNER_STRUCT, * SOCKET_OWNER_STRUCT_PTR;
#endif

/* Forward declaration. */
struct mc_member;
struct ucb_struct;
struct tcb_struct;

struct linger
{
  int32_t l_onoff;
  int32_t l_linger_ms;
};

/* Socket control structure.*/
typedef struct socket_struct {

   struct   socket_struct      * NEXT;
   struct   socket_struct      * PREV;

   uint32_t                      VALID;
   uint16_t                      STATE;
   uint16_t                      AF;
   RTCS_SOCKET_CALL_STRUCT_PTR   PROTOCOL;

   /*
   ** Socket options.
   **
   **  The order of these options must match the order of the option ids
   **  defined above, and CONNECT_TIMEOUT must be the first one.
   */
   uint32_t                      CONNECT_TIMEOUT;
   uint32_t                      RETRANSMISSION_TIMEOUT;
   uint32_t                      SEND_TIMEOUT;
   uint32_t                      RECEIVE_TIMEOUT;
   uint32_t                      RECEIVE_PUSH;
   uint32_t                      SEND_NOWAIT;
   uint32_t                      SEND_PUSH;
   uint32_t                      RECEIVE_NOWAIT;
   int32_t                       TBSIZE;
   int32_t                       RBSIZE;
   int32_t                       MAXRTO;
   uint32_t                      MAXRCV_WND;
   int32_t                       KEEPALIVE;
   uint32_t                      NOWAIT;
   uint32_t                      NO_NAGLE_ALGORITHM;
   uint32_t                      NOSWRBUF;
   uint32_t                      TIMEWAIT_TIMEOUT;
   uint32_t                      TCPSECUREDRAFT_0;
   uint32_t                      DELAY_ACK;
   uint32_t                      ERROR_CODE; /* last option */
   
   struct linger                 so_linger;
   int32_t                       disallow_mask;
   int32_t                       exceptsignal;  /* socket exception: value */
   int32_t                       exceptpending; /* socket exception: pending flag */

   struct tcb_struct            *TCB_PTR;
   struct ucb_struct            *UCB_PTR;
   
#if RTCSCFG_BACKWARD_COMPATIBILITY_RTCSSELECT==1
#if RTCSCFG_SOCKET_OWNERSHIP
   SOCKET_OWNER_STRUCT           OWNERS;
#endif
#endif

   struct mc_member            **MCB_PTR;
   uint32_t (_CODE_PTR_ * IGMP_LEAVEALL)(struct mc_member**);

   struct {
      RTCS_LINKOPT_STRUCT        RX;
      RTCS_LINKOPT_STRUCT        TX;
   }                             LINK_OPTIONS;
} SOCKET_STRUCT, * SOCKET_STRUCT_PTR;

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t  RTCS_socket
(
   uint32_t              ,  /* [IN] protocol family */
   uint32_t              ,  /* [IN] type of communication */
   uint32_t                 /* [IN] select a specific protocol */
);
extern uint32_t  RTCS_shutdown
(
   uint32_t              ,  /* [IN] socket handle */
   uint32_t                 /* [IN] shutdown method */
);


#define RTCS_CHECKLEVEL(v,err) \
                       (v) == 0 \
                     ? (err) \
                     :

#define RTCS_API_SOCKOPT(f,s,v,p)   RTCS_CHECKLEVEL(v,RTCSERR_SOCK_INVALID_OPTION) \
                                    ((RTCS_SOCKOPT_CALL_STRUCT_PTR)(v))->SOCK_ ## f p

#define getsockopt_legacy(s,v,on,op,ol) (RTCS_API_SOCKOPT(GETSOCKOPT, s, v, (s,v,on,op,ol)))
#define setsockopt_legacy(s,v,on,op,ol) (RTCS_API_SOCKOPT(SETSOCKOPT, s, v, (s,v,on,op,ol)))
#define socket          RTCS_socket
#define shutdown        RTCS_shutdown


/*
** RTCS (non-BSD) prototypes
*/
#if RTCSCFG_BACKWARD_COMPATIBILITY_RTCSSELECT==1

extern uint32_t  RTCS_attachsock
(
   uint32_t                       /* [IN] socket handle */
);
extern uint32_t  RTCS_detachsock
(
   uint32_t                       /* [IN] socket handle */
);
extern uint32_t  RTCS_transfersock
(
   uint32_t     in_sock,          /*[IN] specifies the handle of the existing socket */
   _task_id    new_owner
);
extern uint32_t  RTCS_selectall
(
   uint32_t                       /* [IN] time to wait for data, in milliseconds */
);

extern uint32_t RTCS_selectset(void *sockset, uint32_t size, uint32_t timeout);

#endif /* RTCSCFG_BACKWARD_COMPATIBILITY_RTCSSELECT==1 */

extern uint32_t  RTCS_geterror
(
   uint32_t                       /* [IN] socket handle */
);

/* *** select() *** */
typedef struct tag_rtcs_fd_set
{
  uint32_t  fd_count;
  uint32_t  fd_array[RTCSCFG_FD_SETSIZE];
} rtcs_fd_set;

void RTCS_FD_SET(const uint32_t sock, rtcs_fd_set * const p_fd_set);
void RTCS_FD_CLR(const uint32_t sock, rtcs_fd_set * const p_fd_set);
void RTCS_FD_ZERO(rtcs_fd_set * const p_fd_set);
bool RTCS_FD_ISSET(const uint32_t sock, const rtcs_fd_set * const p_fd_set);

int32_t select(int32_t nfds,
                rtcs_fd_set *restrict readfds,
                rtcs_fd_set *restrict writefds,
                rtcs_fd_set *restrict exceptfds,
                uint32_t timeout_ms);
                
/* *** setsockopt/getsockopt *** */
int32_t setsockopt(uint32_t socket, uint32_t level, uint32_t option_name, 
                     const void * option_value, socklen_t option_len);

int32_t getsockopt(uint32_t socket, uint32_t level, uint32_t option_name,
                     void *restrict option_value, socklen_t *restrict option_len);

/* *** shutdownsocket/closesocket *** */
#define SHUT_RD    0
#define SHUT_WR    1
#define SHUT_RDWR  2                     
                     
int32_t shutdownsocket(uint32_t sock, int32_t how);
int32_t closesocket(uint32_t sock);

uint32_t bind(uint32_t sock, const sockaddr *addr, uint16_t addrlen);
uint32_t connect(uint32_t sock, const sockaddr *addr, uint16_t addrlen);
uint32_t getsockname(uint32_t sock, struct sockaddr *addr, uint16_t *addrlen);
uint32_t getpeername(uint32_t sock, struct sockaddr *addr, uint16_t *addrlen);

uint32_t listen(uint32_t, int32_t);
uint32_t accept(uint32_t, sockaddr *, uint16_t *);
int32_t recv(uint32_t, void*, uint32_t, uint32_t);
int32_t send(uint32_t, void*, uint32_t, uint32_t);
int32_t sendto(uint32_t, void*, uint32_t, uint32_t, sockaddr*, uint16_t);
int32_t recvfrom(uint32_t, void*, uint32_t, uint32_t, sockaddr*, uint16_t*);
                     
#ifdef __cplusplus
}
#endif

#endif /* __rtcs_sock_h__ */


/* EOF */
