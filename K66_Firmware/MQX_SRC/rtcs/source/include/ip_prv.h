#ifndef __ip_prv_h__
#define __ip_prv_h__
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
*   Internal definitions for the Internet Protocol layer.
*
*
*END************************************************************************/

#include "tcpip.h"
#include "ip_if.h"     


/***************************************
**
** Constants
**
*/

/*
** Fragment field bits
*/
#define IP_FRAG_DF               0x4000   /* If true, do not fragment datagram */
#define IP_FRAG_MF               0x2000   /* If true, this is not last fragment */
#define IP_FRAG_MASK             0x1FFF   /* for offset (/8) of frag in dgram */

#define IP_FRAG_SHIFT            3
#define IP_FRAG_MIN              (1<<IP_FRAG_SHIFT)

/*
** IP option kinds                        length (dec) incl. opt+len bytes
*/
#define IPOPT_COPIED_MASK        0x80     /* set if should be copied on fragm. */
#define IPOPT_CLASS_MASK         0x60
#define IPOPT_CLASS_CONTROL      0x00
#define IPOPT_CLASS_DEBUG_MEASUR 0x40
#define IPOPT_SECURITY           0x02     /*  11                        */
#define IPOPT_LOOSE_ROUTE        0x03     /* var                        */
#define IPOPT_TIMESTAMP          0x44     /* var                        */
#define IPOPT_RECORD_ROUTE       0x07     /* var                        */
#define IPOPT_STREAM_ID          0x08     /*   4 (must not be used )    */
#define IPOPT_STRICT_ROUTE       0x09     /* var                        */

/* Time to live of a datagram awaiting reassembly (no relation to the IP TTL) */
#define IPREASM_TTL              60000       /* 60 secs */


/*
** Options for IP_route
*/

/*
** When we send a packet, first we check all directly connected networks
** for the destination.  If the destination is not directly connected, we
** scan the gateway table for a match.  If we find a match, we once again
** scan all directly connected networks, this time for the gateway.
**
** However, if the gateway fails this test, we do NOT recursively scan the
** gateway table for another gateway to reach the gateway.  IP_route()
** internally uses IPROUTEOPT_GATE to control the level of recursion.
*/
#define IPROUTEOPT_GATE       0x40

/*
** IPROUTEOPT_NOVIRTUAL instructs IP_route() to skip the virtual route
** check when routing a packet.  It's used by IPIP_send() to avoid
** tunneling loops.
*/
#define IPROUTEOPT_NOVIRTUAL  0x80

/*
** When we forward a packet, sometimes we want to make sure it doesn't
** go out the interface it was received on.  This occurs with PPP --
** we never forward a PPP packet back to the peer, because the peer has
** already seen the packet.  (If we did, directed broadcasts on a PPP
** link would bounce between the peers until the TTL expired.)
**
** IP_service() sets IPROUTEOPT_RECVIF in the call to IP_route() when
** it wants to indicate that the received PCB has already been seen
** by all hosts on pcb->IFSRC.  As a result, if IP_route() decides that
** the packet should go out pcb->IFSRC, it will return RTCS_OK without
** sending it (because the next hop has already seen it).
*/
#define IPROUTEOPT_RECVIF     RTCS_MSG_NOLOOP


/***************************************
**
** Code macros
**
*/

/*
** For IP_send[_IF]
*/
#define IPPROTO_GET(i)  ( (i)        & 0xFF)
#define IPTTL_GET(i)    (((i) >>  8) & 0xFF)
#define IPTOS_GET(i)    (((i) >> 16) & 0xFF)
#define IPDFRAG_GET(i)  (((i) >> 24) & 0x01)

#if RTCSCFG_ENABLE_IP_STATS
#define IF_IP_STATS_ENABLED(x) x
#else
#define IF_IP_STATS_ENABLED(x)
#endif


/***************************************
**
** Type definitions
**
*/

/*
** IP packet header
*/
typedef struct IP_HEADER {

   unsigned char    VERSLEN[1];    /* hi-nybble=Version, lo-nybble=header len/4 */
   unsigned char    TOS[1];        /* Type of service (see TOS_... #define's) */
   unsigned char    LENGTH[2];     /* Length of packet (header+data) in bytes */

   unsigned char    ID[2];         /* Packet identification */
   unsigned char    FRAGMENT[2];   /* Fragment offset & flags */

   unsigned char    TTL[1];        /* Time to live, in secs or hops */
   unsigned char    PROTOCOL[1];   /* Protocol */
   unsigned char    CHECKSUM[2];   /* IP_checksum */

   unsigned char    SOURCE[4];     /* sender of packet */
   unsigned char    DEST[4];       /* destination of packet */

} IP_HEADER, * IP_HEADER_PTR;


#define BIGGEST_IP_HEADER  (15*4)
#define IPH_LEN(iph)       ((mqx_ntohc((iph)->VERSLEN) & 0xF) << 2)

/* Internet options:*/
#define IP_OPTION_TYPE_EOL          (0)     /* End of Option List*/
#define IP_OPTION_TYPE_NOP          (1)     /* No Operation*/
#define IP_OPTION_TYPE_RR           (7)     /* Record Route*/
#define IP_OPTION_TYPE_TIMESTAMP    (68)    /* Internet Timestamp*/
#define IP_OPTION_TYPE_SECURITY     (130)   /* Security*/
#define IP_OPTION_TYPE_LSRR         (131)   /* LSRR*/
#define IP_OPTION_TYPE_STREAMID     (136)   /* Stream Identifier*/
#define IP_OPTION_TYPE_SSRR         (137)   /* SSRR*/

/* Min value of option pointers.*/
#define IP_OPTION_TIMESTAMP_POINTER_MIN    (5)    /* Internet Timestamp pointer*/
#define IP_OPTION_RR_POINTER_MIN           (4)    /* Record Route pointer*/

/* RFC791, p23:
*      Internet Timestamp
*
*        +--------+--------+--------+--------+
*        |01000100| length | pointer|oflw|flg|
*        +--------+--------+--------+--------+
*        |         internet address          |
*        +--------+--------+--------+--------+
*        |             timestamp             |
*        +--------+--------+--------+--------+
*        |                 .                 |
*/
typedef struct IP_OPTION_TIMESTAMP
{
   unsigned char    TYPE[1];        /* Option Type = 68 */
   unsigned char    LENGTH[1];      /* The Option Length is the number of octets in the option counting
                                    * the type, length, pointer, and overflow/flag octets (maximum
                                    * length 40). */
   unsigned char    POINTER[1];     /* The Pointer is the number of octets from the beginning of this
                                    * option to the end of timestamps plus one (i.e., it points to the
                                    * octet beginning the space for next timestamp).  The smallest
                                    * legal value is 5.  The timestamp area is full when the pointer
                                    * is greater than the length. */
   unsigned char    OFLW_FLG[1];    /* The Overflow (oflw) [4 bits] is the number of IP modules that
                                    * cannot register timestamps due to lack of space.
                                    * The Flag (flg) [4 bits] values are
                                    *           0 -- time stamps only, stored in consecutive 32-bit words,
                                    *           1 -- each timestamp is preceded with internet address of the
                                    *                registering entity,
                                    *           3 -- the internet address fields are prespecified.  An IP
                                    *                module only registers its timestamp if it matches its own
                                    *                address with the next specified internet address.  */
   unsigned char    TIMESTAMP[1][4];/* The Timestamp is a right-justified, 32-bit timestamp in
                                    * milliseconds since midnight UT.  If the time is not available in
                                    * milliseconds or cannot be provided with respect to midnight UT
                                    * then any time may be inserted as a timestamp provided the high
                                    * order bit of the timestamp field is set to one to indicate the
                                    * use of a non-standard value. */
} IP_OPTION_TIMESTAMP, * IP_OPTION_TIMESTAMP_PTR;

/* RFC791, p20:
*      Record Route
*
*        +--------+--------+--------+---------//--------+
*        |00000111| length | pointer|     route data    |
*        +--------+--------+--------+---------//--------+
*/
typedef struct IP_OPTION_RECORDROUTE
{
   unsigned char    TYPE[1];        /* Option Type = 7 */
   unsigned char    LENGTH[1];      /* the option length which includes the option type code and the
                                    * length octet, the pointer octet, and length-3 octets of route
                                    * data. */
   unsigned char    POINTER[1];     /* The pointer into the route data
                                    * indicating the octet which begins the next area to store a route
                                    * address.  The pointer is relative to this option, and the
                                    * smallest legal value for the pointer is 4. */
   unsigned char    ROUTE[1][4];    /* A recorded route is composed of a series of internet addresses.
                                    * Each internet address is 32 bits or 4 octets. */
} IP_OPTION_RECORDROUTE, * IP_OPTION_RECORDROUTE_PTR;

#if RTCSCFG_ENABLE_RIP && RTCSCFG_ENABLE_IP4 
#include "rip_prv.h" /* included here because it requires IP_IF_PTR */
#endif

/*
** The IP routing table entry
*/

typedef struct ip_route_direct {
   struct ip_route_direct       *NEXT;
   _ip_address                   ADDRESS;
   IP_IF_PTR                     NETIF;
#if RTCSCFG_ENABLE_RIP && RTCSCFG_ENABLE_IP4 
   RIP_INFO                      RIP;
#endif
   uint32_t                       FLAGS;
   IP_IF_PTR                     DESTIF;
} IP_ROUTE_DIRECT, * IP_ROUTE_DIRECT_PTR;

typedef struct ip_route_virtual {
   struct ip_route_virtual      *NEXT;       /* --\                          */
   _ip_address                   ADDRESS;    /* ---\  All of these fields    */
   IP_IF_PTR                     IS_DIRECT;  /* ----\ have the same location */
#if RTCSCFG_ENABLE_RIP && RTCSCFG_ENABLE_IP4 
   RIP_INFO                      RIP;        /* ----/ as in IP_ROUTE_DIRECT. */
#endif
   uint32_t                       FLAGS;      /* ---/  This must not change   */
   IP_IF_PTR                     DESTIF;     /* --/                          */
   _ip_address                   SOURCE_NET;
   _ip_address                   SOURCE_MASK;
   void                         *DATA;
} IP_ROUTE_VIRTUAL, * IP_ROUTE_VIRTUAL_PTR;

typedef struct ip_route_indirect {
   struct ip_route_indirect        *NEXT;        /* Aligned with IP_ROUTE_DIRECT.NEXT    */
   _ip_address                      GATEWAY;     /* Aligned with IP_ROUTE_DIRECT.ADDRESS */
   void                            *IS_DIRECT;   /* Aligned with IP_ROUTE_DIRECT.NETIF   */
#if RTCSCFG_ENABLE_RIP && RTCSCFG_ENABLE_IP4 
   RIP_INFO                         RIP;         /* Aligned with IP_ROUTE_DIRECT.RIP     */
#endif
   uint32_t                          FLAGS;       /* Aligned with IP_ROUTE_DIRECT.FLAGS   */
   /* Start CR 1133 */
   uint16_t                          METRIC;
   /* End CR */
} IP_ROUTE_INDIRECT, * IP_ROUTE_INDIRECT_PTR;

/*
** This struct relies on the fact that all the different route types start
** start with a NEXT pointer
*/
typedef struct ip_route_prv {
   IPRADIX_NODE              NODE;
   IP_ROUTE_DIRECT_PTR       ROUTETYPE[3];
} IP_ROUTE_PRV, * IP_ROUTE_PRV_PTR;

typedef struct ip_route {
   IPRADIX_NODE            NODE;
   IP_ROUTE_DIRECT_PTR     DIRECT;
   IP_ROUTE_INDIRECT_PTR   INDIRECT;
   IP_ROUTE_VIRTUAL_PTR    VIRTUAL;
} IP_ROUTE, * IP_ROUTE_PTR;


/* IP_ROUTE_INDIRECT.FLAGS = RTF_* */
#define RTF_UP          0x0001      /* !UP => unreachable */
#define RTF_STATIC      0x0002      /* created by IPIF_(bind|gate) */
#define RTF_REDIRECT    0x0004      /* created for a ICMP redirect */

/*
** IP routing extensions
*/
typedef struct ip_route_fn {
   struct ip_route_fn      *NEXT;
   void (_CODE_PTR_ INIT_IF)(IP_ROUTE_DIRECT_PTR);
   void (_CODE_PTR_ INIT_RT)(IP_ROUTE_INDIRECT_PTR, uint16_t);
   void (_CODE_PTR_ REM_RT)(IP_ROUTE_INDIRECT_PTR);
} IP_ROUTE_FN, * IP_ROUTE_FN_PTR;

/*
** IP Configuration.  This information is persistent for the IP layer.
*/
typedef struct ip_cfg_struct {
#if RTCSCFG_ENABLE_IP_STATS
   IP_STATS          STATS;
#endif

   ICB_STRUCT_PTR    ICB_HEAD;      /* Head of open ICBs */
   uint16_t           NEXT_ID;       /* next packet identifier */
   uint16_t           DEFAULT_TTL;   /* default TTL */
   _rtcs_part        ROUTE_PARTID;  /* the partition descriptor of IP_ROUTEs */
#if RTCSCFG_ENABLE_GATEWAYS
   _rtcs_part        GATE_PARTID;   /* the descriptor of IP_ROUTE_INDIRECTS */
#endif
   IP_ROUTE_FN_PTR   ROUTE_FN;      /* function hook for routing protocols */

#if RTCSCFG_ENABLE_IGMP && RTCSCFG_ENABLE_IP4
   _rtcs_part        MCB_PARTID;    /* the partition descriptor of MCBs */
#endif

   _rtcs_part        RADIX_PARTID;
   IP_ROUTE          ROUTE_ROOT;

#if RTCSCFG_ENABLE_VIRTUAL_ROUTES
   _rtcs_part        VIRTUAL_PARTID;   /* parition for virtual routes */
#endif
} IP_CFG_STRUCT, * IP_CFG_STRUCT_PTR;


/***************************************
**
** Prototypes
**
*/

#ifdef __cplusplus
extern "C" {
#endif

/*
** the IP router
*/

void IP_route_init
(
   IP_ROUTE_PTR   head        /* Pointer to the head of the IPRADIX tree */
);

uint32_t IP_route_add_direct
(
   _ip_address    address,    /* Address or network address of destif   */
   _ip_address    netmask,    /* Mask for the address parameter         */
   IP_IF_PTR      netif,      /* Interface for incomming packets        */
   IP_IF_PTR      destif      /* Interface for outgoing packets         */
);

void IP_route_remove_direct
(
   _ip_address    address,    /* IP address of route to remove    */
   _ip_address    netmask,    /* IP netmask of route to remove    */
   IP_IF_PTR      netif       /* pointer to the route interface   */
);

uint32_t IP_route_add_indirect
(
   _ip_address    address,    /* Gateway address */
   _ip_address    netmask,    /* Network mask    */
   _ip_address    network,    /* Network address */
   uint32_t        flag,       /* [IN] RTF_* */
   uint16_t        metric      /* [IN] the route metric [0,65535] */
);

void IP_route_remove_indirect
(
   _ip_address    gateway,    /* Gateway address   */
   _ip_address    netmask,    /* Network mask      */
   /* Start CR 1133 */
   _ip_address    network,    /* Network address   */
   uint32_t        flag,       /* [IN] RTF_* */
   uint16_t        metric      /* [IN] the route metric [0,65535] */
   /* End CR */
);

uint32_t IP_route_add_virtual
(
   _ip_address   address,        /* Destination address              */
   _ip_address   netmask,        /* Mask for the address parameter   */
   _ip_address   source,         /* Interface source address         */
   _ip_address   source_net,     /* Allowed source network           */
   _ip_address   source_mask,    /* Allowed source network mask      */
   IP_IF_PTR     destif,         /* Interface for outgoing packets   */
   void         *data            /* Route information                */
);

void IP_route_remove_virtual
(
   _ip_address    address,       /* IP address of route to remove    */
   _ip_address    netmask,       /* IP netmask of route to remove    */
   _ip_address    source,        /* IP of interface                  */
   _ip_address    source_net,    /* Allowed source network           */
   _ip_address    source_mask,   /* Allowed source source netmak     */
   IP_IF_PTR      netif          /* pointer to the route interface   */
);

uint32_t IP_complete_send
(
   void             *ifdest,
         /* [IN] the destination interface */
   RTCSPCB_PTR  *pcb,
         /* [IN] the packet to send */
   _ip_address       hopsrc,
         /* [IN] the destination interface */
   _ip_address       ipdest,
         /* [IN] the ultimate dest */
   uint32_t           protocol
         /* [IN] the transport layer protocol */

);

uint32_t IP_complete_recv
(
   void             *ifdest,
         /* [IN] the destination interface */
   RTCSPCB_PTR  *pcb,
         /* [IN] the packet to send */
   _ip_address       hopsrc,
         /* [IN] the destination interface */
   _ip_address       ipdest,
         /* [IN] the ultimate dest */
   uint32_t           protocol
         /* [IN] the transport layer protocol */

);

bool IP_is_local
(
   IP_IF_PTR      iflocal,
         /* [IN] the local interface */
   _ip_address    iplocal
         /* [IN] the IP address to test */
);

bool IP_is_direct
   (
      IP_IF_PTR      iflocal,
            /* [IN] the local interface */
      _ip_address    iplocal
            /* [IN] the IP address to test */
   );

bool IP_is_gate
(
   _ip_address    gateway,
         /* [IN] the gateway */
   _ip_address    ipremote
         /* [IN] the IP address to test */
);

bool IP_get_netmask
(
   _rtcs_if_handle   ihandle,    /* [IN] Interface */
   _ip_address       address,    /* [IN] IP address */
   _ip_address  *mask_ptr    /* [OUT] netwask for the IP and interface */
) ;

_ip_address IP_get_ipif_addr
(
   IP_IF_PTR      ipif        /* [IN] the local interface */
);

IP_ROUTE_PTR IP_ipif_get_iproute
(
   IP_IF_PTR      ipif        /* [IN] the local interface */
);

uint32_t IP_MTU
(
   _ip_address    iplocal,
         /* [IN] the local IP address */
   _ip_address    ipremote
         /* [IN] the remote IP address to test */
);

IP_IF_PTR IP_find_if
(
   _ip_address    iplocal
         /* [IN] the IP address to test */
);

_ip_address IP_route_find
(
   _ip_address    ipdest,
         /* [IN] the ultimate destination */
   uint32_t        flags
         /* [IN] optional flags */
);

uint32_t IP_route_local
(
   RTCSPCB_PTR    pcb,
         /* [IN] the packet to send */
   _ip_address    ipdest
         /* [IN] the ultimate destination */
);

uint32_t IP_route_multi
(
   RTCSPCB_PTR    pcb,
         /* [IN] the packet to send */
   uint32_t        protocol,
            /* [IN] the transport layer protocol */
   _ip_address    ipsrc,
         /* [IN] the destination interface (0 = any) */
   _ip_address    ipdest,
         /* [IN] the ultimate destination */
   uint32_t        flags
         /* [IN] optional flags */
);

uint32_t IP_route
(
   RTCSPCB_PTR    pcb,
         /* [IN] the packet to send */
   uint32_t        protocol,
         /* [IN] the transport layer protocol */
   _ip_address    hopsrc,
         /* [IN] the destination interface (0 = any) */
   _ip_address    ipsrc,
         /* [IN] the ultimate source */
   _ip_address    ipdest,
         /* [IN] the ultimate destination */
   uint32_t        flags
         /* [IN] optional flags */
);

uint32_t IP_send_dgram
(
   IP_IF_PTR      ifdest,     /* [IN] the outgoing interface */
   RTCSPCB_PTR    inpcb,      /* [IN] the packet to send */
   _ip_address    hopsrc,     /* [IN] the hop src */
   _ip_address    hopdest,    /* [IN] the hop dest */
   _ip_address    ipdest,     /* [IN] the ultimate dest */
   uint32_t        protocol,   /* [IN] the transport layer protocol */
   void          *data        /* [IN] routing entry data */
);

bool IP_will_fragment( IP_IF_PTR  ifdest, uint32_t protocol_message_size);
bool IP_addr_is_broadcast (RTCSPCB_PTR rtcs_pcb, _ip_address ip_addr);

/*
** the reassembler
*/

/*
** Datagrams awaiting reassembly are stored in sets of 256-byte
** blocks.  We allocate a partition of memory blocks; each block
** can be used in one of three ways:
**
**    1) It can store up to 256 data bytes from the datagram.
**       This is an IPREASM_BLK, called a 'direct block'.
**       This block also has 32-bit bitfield indicating which
**       part(s) of the direct block contain(s) valid data.
**       Each bit represents 8 bytes, so (32 bits) * (8 bytes/bit)
**       = 256 bytes.
**
**    2) It can be used as an array of pointers to IPREASM_BLKs
**       This is an IPREASM_INDIDX, called an 'indirect block'.
**       The indirect block could hold up to 65 direct block
**       pointers (257 in 32-bit wide memory), but we only
**       store 64 (or 256) to simplify the math.  i.e.
**       IPREASM_IND_SIZE could be
**          sizeof(IPREASM_BLK)/sizeof(IPREASM_BLK_PTR),
**       but we only set it to
**          IPREASM_BLK_SIZE/sizeof(IPREASM_BLK_PTR),
**
**    3) It can hold state information about an IP datagram, an
**       IP_DGRAM structure.  In particular, it has the datagram
**       'key' (the IP source, dest, protocol and ID), and as
**       many pointers to indirect blocks as it needs to address
**       a maximum sized datagram (256 direct blocks), i.e. 4
**       (or 1).  However, the IP_DGRAM structure is quite a
**       bit smaller than an IPREASM_BLK, so difference is
**       filled with pointers to direct blocks -- as many as
**       will fit without exceeding the size of an IPREASM_BLK.
**       When allocating blocks for incoming IP datagram fragments,
**       the direct block pointers in the IP_DGRAM are used first,
**       followed by the indirect block pointers.
*/


/* The size of the IP reassembly partition */
#define IPREASM_PART_MAXSIZE     (40*1024)
#define IPREASM_PART_GROW        32          /* 8 KB */
#define IPREASM_PART_MAX         (IPREASM_PART_MAXSIZE / sizeof(IPREASM_BLK))


/* Size of a block in octets */
#define IPREASM_BLK_SIZE         256
#define IPREASM_BLK_NUM(offset)  ((offset) / IPREASM_BLK_SIZE)
#define IPREASM_BLK_OFS(offset)  ((offset) % IPREASM_BLK_SIZE)
#define IPREASM_NB_BLK           (0x10000 / IPREASM_BLK_SIZE)

/* Number of direct block pointers that will fit in an IP_DGRAM */
#define IPREASM_NB_DIRECT        ((sizeof(IPREASM_BLK)-offsetof(IP_DGRAM,DIRECT)) \
                                  / sizeof(IPREASM_BLK_PTR))
#define IPREASM_IS_DIRECT(block) ((block) < IPREASM_NB_DIRECT)

/* Number of direct block pointers in each indirect block */
#define IPREASM_IND_SIZE         (IPREASM_BLK_SIZE / sizeof(IPREASM_BLK_PTR))
#define IPREASM_IND_NUM(block)   (((block)-IPREASM_NB_DIRECT) / IPREASM_IND_SIZE)
#define IPREASM_IND_OFS(block)   (((block)-IPREASM_NB_DIRECT) % IPREASM_IND_SIZE)
#define IPREASM_NB_IND           (IPREASM_NB_BLK / IPREASM_IND_SIZE)

#define IPREASM_IP_OPTIONS_SIZE     (60) 

typedef struct ipreasm_blk {
   uint32_t  BUSYINT; /* the bitfield, 1 bit set = 8 bytes copied */
                     /* the LSB is DATA[0-7] */
                     /* the MSB is DATA[249-256] */
   unsigned char    DATA[IPREASM_BLK_SIZE];
} IPREASM_BLK, * IPREASM_BLK_PTR;

/* the structure of an indirect block */
typedef struct ipreasm_indidx {
   IPREASM_BLK_PTR   DIRECT[IPREASM_IND_SIZE];
} IPREASM_INDIDX, * IPREASM_INDIDX_PTR;

typedef struct ip_dgram
{
    /* the "key" of the dgram.  rfc815.7 */
    union
    {
    #if RTCSCFG_ENABLE_IP4        
        struct
        {
            _ip_address IPSRC;
            _ip_address IPDST;
            uint8_t      PROTO;
            uint16_t     ID;
            /* the IP header of the complete datagram */
            IP_HEADER   IPH;
             /* leave rooms for the ip options. MUST follow the IPH field */
            unsigned char       IP_OPTIONS[60 - sizeof(IP_HEADER)];
            unsigned char       ICMP_DATA[8];
        }IP4;
    #endif
    #if RTCSCFG_ENABLE_IP6
        struct
        {
            in6_addr    IPSRC;
            in6_addr    IPDST;
            uint8_t      PROTO;
            uint32_t     ID;
            /* the IP header of the complete datagram */
            IP6_HEADER  IPH;
            unsigned char       IP_OPTIONS[60];
            uint32_t     IPH_OPT_LENGTH;
        }IP6;
    #endif
    }header;
    uint16_t     family;    /* AF_INET6/AF_INET */

    /* move-to-front list of IP_DGRAMs */
    struct ip_dgram            *NEXT;
    struct ip_dgram           **PREV;

    /* Information from the PCB with the first fragment */
    uint32_t              TYPE;
    void                *IFSRC;
    RTCS_LINKOPT_STRUCT  LINKOPT;

    uint32_t    TOTLEN; /* the data of the complete ip datagram payload */
    uint32_t    CURLEN; /* the current data size */
    uint32_t    MAXLEN; /* the last fragment seen */

    TCPIP_EVENT TIMER; /* the timer to handle datagram expiry */

    IPREASM_INDIDX_PTR   INDIR[IPREASM_NB_IND];
    IPREASM_BLK_PTR      DIRECT[1];
} IP_DGRAM, * IP_DGRAM_PTR;

extern IP_DGRAM_PTR  IPREASM_head;  /* list of all IP_DGRAMs */
extern _rtcs_part    IPREASM_part;  /* the partition id for the buffer */

IPREASM_BLK_PTR IPREASM_blk_get      (IP_DGRAM_PTR, uint32_t);
uint32_t         IPREASM_blk_write    (IP_DGRAM_PTR, IPREASM_BLK_PTR, uint32_t, unsigned char *, uint32_t);
void            IPREASM_blk_read_all (IP_DGRAM_PTR, unsigned char *, uint32_t);
void            IPREASM_blk_del_all  (IP_DGRAM_PTR);
void            IPREASM_del_dgram   (IP_DGRAM_PTR);

uint32_t IP_reasm_init (void);
uint32_t IP_reasm      (RTCSPCB_PTR, RTCSPCB_PTR *);

#ifdef __cplusplus
}
#endif

#endif /* __ip_prv_h__ */

