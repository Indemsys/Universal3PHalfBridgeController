#ifndef __nd6_h__
#define __nd6_h__
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
*   Definitions for the Neighbor Discovery for IP6.
*
*
*END************************************************************************/
#include "rtcspcb.h"
#include "rtcstime.h"
#include "icmp6_prv.h"

/**************************************************************
* RFC4861. 10. Protocol Constants.
***************************************************************
*    Host constants:
*        MAX_RTR_SOLICITATION_DELAY 1 second
*        RTR_SOLICITATION_INTERVAL 4 seconds
*        MAX_RTR_SOLICITATIONS 3 transmissions
*    Node constants:
*        MAX_MULTICAST_SOLICIT 3 transmissions
*        MAX_UNICAST_SOLICIT 3 transmissions
*        MAX_ANYCAST_DELAY_TIME 1 second
*        MAX_NEIGHBOR_ADVERTISEMENT 3 transmissions
*        REACHABLE_TIME 30,000 milliseconds
*        RETRANS_TIMER 1,000 milliseconds
*        DELAY_FIRST_PROBE_TIME 5 seconds
*        MIN_RANDOM_FACTOR .5
*        MAX_RANDOM_FACTOR 1.5
***************************************************************/

/* If a host sends MAX_RTR_SOLICITATIONS solicitations, and receives no
 * Router Advertisements after having waited MAX_RTR_SOLICITATION_DELAY
 * seconds after sending the last solicitation, the host concludes that
 * there are no routers on the link for the purpose of [ADDRCONF].
 * However, the host continues to receive and process Router
 * Advertisements messages in the event that routers appear on the link.
 */
#define ND6_MAX_RTR_SOLICITATIONS       (3)         /* transmissions */
#define ND6_MAX_RTR_SOLICITATION_DELAY  (1000)      /* ms */
#define ND6_RTR_SOLICITATION_INTERVAL   (4000)      /* ms */

/* If no Neighbor Advertisement is received after MAX_MULTICAST_SOLICIT
* solicitations, address resolution has failed. The sender MUST return
* ICMP destination unreachable indications with code 3 (Address
* Unreachable) for each packet queued awaiting address resolution.
*/
#define ND6_MAX_MULTICAST_SOLICIT       (3)     /* transmissions */

/*
 * Default value of the time between retransmissions of Neighbor
 * Solicitation messages to a neighbor when
 * resolving the address or when probing the
 * reachability of a neighbor. Also used during Duplicate
 * Address Detection (RFC4862).
 */
#define ND6_RETRANS_TIMER               (1000)  /* ms */

/*
 * Default value of the time a neighbor is considered reachable after
 * receiving a reachability confirmation.
 *
 * This value should be a uniformly distributed
 * random value between MIN_RANDOM_FACTOR and
 * MAX_RANDOM_FACTOR times BaseReachableTime
 * milliseconds. A new random value should be
 * calculated when BaseReachableTime changes (due to
 * Router Advertisements) or at least every few
 * hours even if
 */
#define ND6_REACHABLE_TIME              (30000) /* ms */

/*
 * If no reachability confirmation is received
 * within DELAY_FIRST_PROBE_TIME seconds of entering the
 * DELAY state, send a Neighbor Solicitation and change
 * the state to PROBE.
 */
#define ND6_DELAY_FIRST_PROBE_TIME      (5000)  /*ms*/

/*
 * If no response is
 * received after waiting RetransTimer milliseconds after sending the
 * MAX_UNICAST_SOLICIT solicitations, retransmissions cease and the
 * entry SHOULD be deleted.
 */
#define ND6_MAX_UNICAST_SOLICIT         (3)     /*times*/

/*
 * ND6 general timer resolution.
 */
#define ND6_TIMER_PERIOD                (100)   /* ms */

/* Real Sizes:*/
#define ND6_NEIGHBOR_CACHE_SIZE         (RTCSCFG_ND6_NEIGHBOR_CACHE_SIZE + RTCSCFG_ND6_ROUTER_LIST_SIZE) /* Neighbor Cache and Default Router List combined to one list.*/
#define ND6_PREFIX_LIST_SIZE            (RTCSCFG_ND6_PREFIX_LIST_SIZE + 1) /* One more for link-local prefix.*/
#define ND6_REDIRECT_TABLE_SIZE         (4) /* TBD config parameter.*/
#define ND6_PREFIX_LENGTH_DEFAULT       (64)            /* Default prefix length, in bits.*/
#define ND6_PREFIX_LIFETIME_INFINITE    (0xFFFFFFFF)    /* A lifetime value of all one bits (0xffffffff) represents infinity. */
#define ND6_RDNSS_LIFETIME_INFINITE     (0xFFFFFFFF)    /* A lifetime value of all one bits (0xffffffff) represents infinity. */


/**************************************************************
* Neighbor’s reachability state., based on RFC4861.
**************************************************************/
typedef enum nd6_neighbor_state
{
    ND6_NEIGHBOR_STATE_NOTUSED = 0,     /* The entry is not used - free.*/
    ND6_NEIGHBOR_STATE_INCOMPLETE = 1,  /* Address resolution is in progress and the link-layer
                                         * address of the neighbor has not yet been determined.*/
    ND6_NEIGHBOR_STATE_REACHABLE = 2,   /* Roughly speaking, the neighbor is known to have been
                                         * reachable recently (within tens of seconds ago).*/
    ND6_NEIGHBOR_STATE_STALE = 3,       /* The neighbor is no longer known to be reachable but
                                         * until traffic is sent to the neighbor, no attempt
                                         * should be made to verify its reachability.*/                                         
    ND6_NEIGHBOR_STATE_DELAY = 4,       /* The neighbor is no longer known to be reachable, and
                                         * traffic has recently been sent to the neighbor.
                                         * Rather than probe the neighbor immediately, however,
                                         * delay sending probes for a short while in order to
                                         * give upper-layer protocols a chance to provide
                                         * reachability confirmation.*/                                          
    ND6_NEIGHBOR_STATE_PROBE = 5        /* The neighbor is no longer known to be reachable, and
                                         * unicast Neighbor Solicitation probes are being sent to
                                         * verify reachability.*/                                          
} nd6_neighbor_state_t;

/**********************************************************************
* Neighbor Solicitation Message Format
***********************************************************************
* Nodes send Neighbor Solicitations to request the link-layer address
* of a target node while also providing their own link-layer address to
* the target.
*
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |     Type      |     Code      |          Checksum             |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                           Reserved                            |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                                                               |
*    +                                                               +
*    |                                                               |
*    +                       Target Address                          +
*    |                                                               |
*    +                                                               +
*    |                                                               |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |   Options ...
*    +-+-+-+-+-+-+-+-+-+-+-+-
***********************************************************************/
typedef struct nd6_ns_header
{
   ICMP6_HEADER         HEAD;
   unsigned char       	RESERVED[4];
   unsigned char		TARGET_ADDR[16];
} ND6_NS_HEADER, * ND6_NS_HEADER_PTR;

/**********************************************************************
* Neighbor Advertisement Message Format
***********************************************************************
* A node sends Neighbor Advertisements in response to Neighbor
* Solicitations and sends unsolicited Neighbor Advertisements in order
* to (unreliably) propagate new information quickly.
*
*	 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |     Type      |     Code      |          Checksum             |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |R|S|O|                     Reserved                            |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                                                               |
*    +                                                               +
*    |                                                               |
*    +                       Target Address                          +
*    |                                                               |
*    +                                                               +
*    |                                                               |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |   Options ...
*    +-+-+-+-+-+-+-+-+-+-+-+-
***********************************************************************/
typedef struct nd6_na_header
{
   ICMP6_HEADER         HEAD;
   unsigned char        FLAG[1];
   unsigned char       	RESERVED[3];
   unsigned char		TARGET_ADDR[16];
} ND6_NA_HEADER, * ND6_NA_HEADER_PTR;

/* NA flags.*/
#define ND6_NA_FLAG_ROUTER      (0x80)  /* Router flag. When set, the R-bit indicates that
                                         * the sender is a router. The R-bit is used by
                                         * Neighbor Unreachability Detection to detect a
                                         * router that changes to a host.*/
#define ND6_NA_FLAG_SOLICITED   (0x40)  /* Solicited flag. When set, the S-bit indicates that
                                         * the advertisement was sent in response to a
                                         * Neighbor Solicitation from the Destination address.
                                         * The S-bit is used as a reachability confirmation
                                         * for Neighbor Unreachability Detection. It MUST NOT
                                         * be set in multicast advertisements or in
                                         * unsolicited unicast advertisements.*/
#define ND6_NA_FLAG_OVERRIDE    (0x20)  /* Override flag. When set, the O-bit indicates that
                                         * the advertisement should override an existing cache
                                         * entry and update the cached link-layer address.
                                         * When it is not set the advertisement will not
                                         * update a cached link-layer address though it will
                                         * update an existing Neighbor Cache entry for which
                                         * no link-layer address is known. It SHOULD NOT be
                                         * set in solicited advertisements for anycast
                                         * addresses and in solicited proxy advertisements.
                                         * It SHOULD be set in other solicited advertisements
                                         * and in unsolicited advertisements.*/                                                  

/**********************************************************************
* Redirect Message Format (RFC 4861)
***********************************************************************
* Routers send Redirect packets to inform a host of a better first-hop
* node on the path to a destination.
*
*	 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |     Type      |     Code      |          Checksum             |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                           Reserved                            |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                                                               |
*    +                                                               +
*    |                                                               |
*    +                       Target Address                          +
*    |                                                               |
*    +                                                               +
*    |                                                               |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                                                               |
*    +                                                               +
*    |                                                               |
*    +                    Destination Address                        +
*    |                                                               |
*    +                                                               +
*    |                                                               |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |   Options ...
*    +-+-+-+-+-+-+-+-+-+-+-+-
***********************************************************************/
typedef struct nd6_rd_header
{
   ICMP6_HEADER         HEAD;
   unsigned char       	RESERVED[4];
   unsigned char		TARGET_ADDR[16];
   unsigned char		DESTINATION_ADDR[16];
} ND6_RD_HEADER, * ND6_RD_HEADER_PTR;

/**********************************************************************
* Router Solicitation Message Format
***********************************************************************
* Hosts send Router Solicitations in order to prompt routers to
* generate Router Advertisements quickly.
*
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |     Type      |     Code      |          Checksum             |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                           Reserved                            |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |   Options ...
*    +-+-+-+-+-+-+-+-+-+-+-+-
***********************************************************************/
typedef struct nd6_rs_header
{
   ICMP6_HEADER         HEAD;           /* ICMPv6 header.*/
   unsigned char       	RESERVED[4];
} ND6_RS_HEADER, * ND6_RS_HEADER_PTR;

/**********************************************************************
* Router Advertisement Message Format
***********************************************************************
* Routers send out Router Advertisement messages periodically, or in
* response to Router Solicitations.
*
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |     Type      |     Code      |          Checksum             |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    | Cur Hop Limit |M|O|  Reserved |       Router Lifetime         |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                      Reachable Time                           |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                      Retrans Timer                            |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |   Options ...
*    +-+-+-+-+-+-+-+-+-+-+-+-
***********************************************************************/
typedef struct nd6_ra_header
{
   ICMP6_HEADER         HEAD;               /* ICMPv6 header.*/
   unsigned char       	CUR_HOP_LIMIT[1];   /* 8-bit unsigned integer. The default value that
                                            * should be placed in the Hop Count field of the IP
                                            * header for outgoing IP packets. A value of zero
                                            * means unspecified (by this router). */
   unsigned char       	FLAG[1];            /* ND6_RS_FLAG_M and/or ND6_RS_FLAG_O flags.*/
   unsigned char       	ROUTER_LIFETIME[2]; /* 16-bit unsigned integer. The lifetime associated
                                            * with the default router in units of seconds. The
                                            * field can contain values up to 65535 and receivers
                                            * should handle any value, while the sending rules in
                                            * Section 6 limit the lifetime to 9000 seconds. A
                                            * Lifetime of 0 indicates that the router is not a
                                            * default router and SHOULD NOT appear on the default
                                            * router list. The Router Lifetime applies only to
                                            * the router’s usefulness as a default router; it
                                            * does not apply to information contained in other
                                            * message fields or options. Options that need time
                                            * limits for their information include their own
                                            * lifetime fields.*/
    unsigned char       REACHABLE_TIME[4];  /* 32-bit unsigned integer. The time, in
                                            * milliseconds, that a node assumes a neighbor is
                                            * reachable after having received a reachability
                                            * confirmation. Used by the Neighbor Unreachability
                                            * Detection algorithm (see Section 7.3). A value of
                                            * zero means unspecified (by this router). */                                       
    unsigned char       RETRANS_TIMER[4];   /* 32-bit unsigned integer. The time, in
                                            * milliseconds, between retransmitted Neighbor
                                            * Solicitation messages. Used by address resolution
                                            * and the Neighbor Unreachability Detection algorithm
                                            * (see Sections 7.2 and 7.3). A value of zero means
                                            * unspecified (by this router).*/   
} ND6_RA_HEADER, * ND6_RA_HEADER_PTR;

/* RA flags */
#define ND6_RA_FLAG_M   (0x80)  /* 1-bit "Managed address configuration" flag. When
                                 * set, it indicates that addresses are available via
                                 * Dynamic Host Configuration Protocol [DHCPv6].
                                 * If the M flag is set, the O flag is redundant and
                                 * can be ignored because DHCPv6 will return all
                                 * available configuration information.*/
#define ND6_RA_FLAG_O   (0x40)  /* 1-bit "Other configuration" flag. When set, it
                                 * indicates that other configuration information is
                                 * available via DHCPv6. Examples of such information
                                 * are DNS-related information or information on other
                                 * servers within the network.*/
                                /* Note: If neither M nor O flags are set, this indicates that no
                                 * information is available via DHCPv6.*/


/* Hop Limit when sending/receiving Neighbor Discovery messages. */
#define ND6_HOP_LIMIT                   255

/* ND option types (RFC4861). */
#define ND6_OPTION_SOURCE_LLA           1 /* Source Link-layer Address.*/
#define ND6_OPTION_TARGET_LLA           2 /* Target Link-layer Address.*/
#define ND6_OPTION_PREFIX               3 /* Prefix Information.*/
#define ND6_OPTION_REDIRECTED_HEADER    4 /* Redirected Header.*/
#define ND6_OPTION_MTU                  5 /* MTU */
#define ND6_OPTION_RDNSS               25 /* RDNSS RFC6106. */

/***********************************************************************
 * ND option header
 ***********************************************************************/
typedef struct nd6_option_header
{
    unsigned char TYPE[1];     /* Identifier of the type of option.*/
    unsigned char LENGTH[1];   /* The length of the option
                                * (including the type and length fields) in units of
                                * 8 octets.  The value 0 is invalid.  Nodes MUST
                                * silently discard an ND packet that contains an
                                * option with length zero.*/
} ND6_OPTION_HEADER, * ND6_OPTION_HEADER_PTR;

/***********************************************************************
 * Source/Target Link-layer Address option header:
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |     Type      |     Length    |       Link-Layer Address ...
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
 ***********************************************************************/
typedef struct nd6_option_lla_header
{
    ND6_OPTION_HEADER   HEAD;       /* Option general header.*/
    unsigned char       addr[6];    /* The length of the option. Can be more or less than 6.*/
}ND6_OPTION_LLA_HEADER, * ND6_OPTION_LLA_HEADER_PTR;

/***********************************************************************
 * MTU option header:
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |     Type      |     Length    |           Reserved            |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                          MTU                                  |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 ***********************************************************************/
typedef struct nd6_option_mtu_header
{
    ND6_OPTION_HEADER   HEAD;       /* Option general header.*/
    unsigned char       RESEVED[2];
    unsigned char       MTU[4];     /* The recommended MTU for the link.*/
}ND6_OPTION_MTU_HEADER, * ND6_OPTION_MTU_HEADER_PTR;

/***********************************************************************
 * Prefix Information option header:
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |     Type      |     Length    | Prefix Length |L|A| Reserved1 |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                         Valid Lifetime                        |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                       Preferred Lifetime                      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                           Reserved2                           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                                                               |
 *  +                                                               +
 *  |                                                               |
 *  +                             Prefix                            +
 *  |                                                               |
 *  +                                                               +
 *  |                                                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 ***********************************************************************/
typedef struct nd6_option_prefix_header
{
    ND6_OPTION_HEADER   HEAD;               /* Option general header.*/
    unsigned char       PREFIX_LENGTH[1];   /* The number of leading bits
                                            * in the Prefix that are valid. The value ranges
                                            * from 0 to 128. The prefix length field provides
                                            * necessary information for on-link determination
                                            * (when combined with the L flag in the prefix
                                            * information option). It also assists with address
                                            * autoconfiguration as specified in [ADDRCONF], for
                                            * which there may be more restrictions on the prefix
                                            * length.*/
    unsigned char       FLAG[1];            /* ND6_OPTION_FLAG_L and/or ND6_OPTION_FLAG_O flags.*/
    unsigned char       VALID_LIFETIME[4];  /* The length of time in
                                            * seconds (relative to the time the packet is sent)
                                            * that the prefix is valid for the purpose of on-link
                                            * determination. A value of all one bits
                                            * (0xffffffff) represents infinity. The Valid
                                            * Lifetime is also used by [ADDRCONF].*/
    unsigned char       PREFERED_LIFETIME[4];/* The length of time in
                                            * seconds (relative to the time the packet is sent)
                                            * that addresses generated from the prefix via
                                            * stateless address autoconfiguration remain
                                            * preferred [ADDRCONF]. A value of all one bits
                                            * (0xffffffff) represents infinity. See [ADDRCONF].
                                            * Note that the value of this field MUST NOT exceed
                                            * the Valid Lifetime field to avoid preferring
                                            * addresses that are no longer valid.*/  
    unsigned char       RESERVED[4];        
    unsigned char       PREFIX[16];          /* An IP address or a prefix of an IP address. The
                                            * Prefix Length field contains the number of valid
                                            * leading bits in the prefix. The bits in the prefix
                                            * after the prefix length are reserved and MUST be
                                            * initialized to zero by the sender and ignored by
                                            * the receiver. A router SHOULD NOT send a prefix
                                            * option for the link-local prefix and a host SHOULD
                                            * ignore such a prefix option.*/                                    
                                                                
}ND6_OPTION_PREFIX_HEADER, * ND6_OPTION_PREFIX_HEADER_PTR;

#define ND6_OPTION_FLAG_L   0x80    /* 1-bit on-link flag. When set, indicates that this
                                     * prefix can be used for on-link determination. When
                                     * not set the advertisement makes no statement about
                                     * on-link or off-link properties of the prefix. In
                                     * other words, if the L flag is not set a host MUST
                                     * NOT conclude that an address derived from the
                                     * prefix is off-link. That is, it MUST NOT update a
                                     * previous indication that the address is on-link.*/    
#define ND6_OPTION_FLAG_A   0x40    /* 1-bit autonomous address-configuration flag. When
                                     * set indicates that this prefix can be used for
                                     * stateless address configuration as specified in
                                     * [ADDRCONF].*/    

/***********************************************************************
 * Recursive DNS Server header (RFC 6106):
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |     Type      |     Length    |           Reserved            |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                           Lifetime                            |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                                                               |
 *   :            Addresses of IPv6 Recursive DNS Servers            :
 *   |                                                               |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 ***********************************************************************/
typedef struct nd6_option_rdnss_header
{
    ND6_OPTION_HEADER   HEAD;           /* Option general header.*/
    unsigned char       RESERVED[2];
    unsigned char       LIFETIME[4];   /* The maximum time, in
                                        * seconds (relative to the time the packet is sent),
                                        * over which this RDNSS address MAY be used for name
                                        * resolution.*/
    unsigned char       ADDRESS[16];   /* One or more 128-bit IPv6 addresses of the recursive
                                        * DNS servers.  The number of addresses is determined
                                        * by the Length field.  That is, the number of
                                        * addresses is equal to (Length - 1) / 2.*/                                    
} ND6_OPTION_RDNSS_HEADER, *ND6_OPTION_RDNSS_HEADER_PTR;
                                 

/***********************************************************************
* Neighbor Cache entry, based on RFC4861.
***********************************************************************/
typedef struct nd6_neighbor_entry
{
 
    in6_addr                ip_addr;                    /* Neighbor’s on-link unicast IP address. */
    ll_addr_t               ll_addr;                    /* Its link-layer address. Actual size is defiined by IP_IF->DEV_ADDRLEN. */
    nd6_neighbor_state_t    state;                      /* Neighbor’s reachability state.*/
    uint32_t                state_time;                 /* Time of last state event.*/
    RTCSPCB_PTR             waiting_pcb;                /* Pointer to any queued packet/PCB waiting for address resolution to complete.*/
                                                        /* RFC 4861 7.2.2: While waiting for address resolution to complete, the sender MUST,
                                                        * for each neighbor, retain a small queue of packets waiting for
                                                        * address resolution to complete. The queue MUST hold at least one
                                                        * packet, and MAY contain more.
                                                        * When a queue  overflows, the new arrival SHOULD replace the oldest entry.*/
    int                     solicitation_send_counter;  /* Counter - how many soicitations where sent.*/
    in6_addr                solicitation_src_ip_addr;   /* IP address used during AR solicitation messages. */
    uint32_t                creation_time;              /* Time of entry creation, in seconds.*/
           
    /* Default Router list entry info.*/
    unsigned char           is_router;                  /* A flag indicating whether the neighbor is a router or a host.*/
    uint32_t                router_lifetime;            /* The lifetime associated
                                                        * with the default router in units of seconds. The
                                                        * field can contain values up to 65535 and receivers
                                                        * should handle any value, while the sending rules in
                                                        * Section 6 limit the lifetime to 9000 seconds. A
                                                        * Lifetime of 0 indicates that the router is not a
                                                        * default router and SHOULD NOT appear on the default router list.
                                                        * It is used only if "is_router" is 1.*/

} ND6_NEIGHBOR_ENTRY, * ND6_NEIGHBOR_ENTRY_PTR;

/***********************************************************************
* Redirect Table entry.
***********************************************************************/
typedef struct nd6_redirect_entry
{
    in6_addr     destination_addr;   /* Destination Address. The IP address of the destination that is
                                      * redirected to the target. */
    in6_addr     target_addr;        /* Target Address. An IP address that is a better first hop to use for
                                      * the ICMP Destination Address. When the target is
                                      * the actual endpoint of communication, i.e., the
                                      * destination is a neighbor, the Target Address field
                                      * MUST contain the same value as the ICMP Destination
                                      * Address field. Otherwise, the target is a better
                                      * first-hop router and the Target Address MUST be the
                                      * router’s link-local address so that hosts can
                                      * uniquely identify routers. */
    uint32_t       creation_time;     /* Time of entry creation.*/
} ND6_REDIRECT_ENTRY, * ND6_REDIRECT_ENTRY_PTR;

/***********************************************************************
* Recursive DNS Server List entry (RFC6106).
***********************************************************************/
typedef struct nd6_rdnss_entry
{
    in6_addr    rdnss_addr;         /* IPv6 address of the Recursive
                                    * DNS Server, which is available for recursive DNS resolution
                                    * service in the network advertising the RDNSS option. */
    uint32_t    creation_time;      /* Time of entry creation, in seconds.*/    
    uint32_t    lifetime;           /* The maximum time, in
                                    * seconds (relative to the time the packet is sent),
                                    * over which this DNSSL domain name MAY be used for
                                    * name resolution.
                                    * A value of all one bits (0xffffffff) represents
                                    * infinity.  A value of zero means that the DNSSL
                                    * domain name MUST no longer be used.*/    
} ND6_RDNSS_ENTRY, *ND6_RDNSS_ENTRY_PTR;


/***********************************************************************
* Prefix state.
***********************************************************************/
typedef enum nd6_prefix_state
{
    ND6_PREFIX_STATE_NOTUSED = 0,   /* The entry is not used - free.*/
    ND6_PREFIX_STATE_USED = 1       /* The entry is used.*/
} nd6_prefix_state_t;

/***********************************************************************
* Prefix List entry, based on RFC4861.
* Prefix List entries are created from information received in Router
* Advertisements.
***********************************************************************/
typedef struct nd6_prefix_entry
{
 
    in6_addr            prefix;         /* Prefix of an IP address. */
    uint32_t            prefix_length;  /* Prefix length (in bits). The number of leading bits
                                        * in the Prefix that are valid. */
    nd6_prefix_state_t  state;          /* Prefix state.*/                                 
    uint32_t            lifetime;       /* Valid Lifetime
                                        * 32-bit unsigned integer. The length of time in
                                        * seconds (relative to the time the packet is sent)
                                        * that the prefix is valid for the purpose of on-link
                                        * determination. A value of all one bits
                                        * (0xffffffff) represents infinity. The Valid
                                        * Lifetime is also used by [ADDRCONF].*/
    uint32_t            creation_time;  /* Time of entry creation, in seconds.*/                                     
} ND6_PREFIX_ENTRY, * ND6_PREFIX_ENTRY_PTR;

/***********************************************************************
* Router state.
***********************************************************************/
typedef enum nd6_router_state
{
    ND6_ROUTER_STATE_NOTUSED = 0,   /* The entry is not used - free.*/
    ND6_ROUTER_STATE_USED = 1       /* The entry is used.*/
} nd6_router_state_t;

/***********************************************************************
* Forward declaration. Just to avoid header conflicts.
***********************************************************************/
struct ip_if; 
struct ip6_if_addr_info;

/***********************************************************************
* Neighbor Discovery API function
***********************************************************************/
typedef struct nd6_if
{
    uint32_t (*neighbor_solicitation_receive)(RTCSPCB_PTR rtcs_pcb);
    uint32_t (*neighbor_advertisement_receive)(RTCSPCB_PTR rtcs_pcb);
    uint32_t (*router_advertisement_receive)(RTCSPCB_PTR rtcs_pcb);
    uint32_t (*redirect_receive)(RTCSPCB_PTR rtcs_pcb);
    void (*dad_start)(struct ip_if *if_ptr, struct ip6_if_addr_info *addr_info);
} ND6_IF, * ND6_IF_PTR;

/***********************************************************************
* Neighbor Discovery Configuration
***********************************************************************/
typedef struct nd6_cfg
{
    /*************************************************************
    * Neighbor Cache.
    * RFC4861 5.1: A set of entries about individual neighbors to 
    * which traffic has been sent recently. 
    **************************************************************/
    /*************************************************************
    * Combined with Default Router List.
    * RFC4861 5.1: A list of routers to which packets may be sent.. 
    **************************************************************/    
    ND6_NEIGHBOR_ENTRY  neighbor_cache [ND6_NEIGHBOR_CACHE_SIZE];
    
    /*************************************************************
    * Prefix List.
    * RFC4861 5.1: A list of the prefixes that define a set of
    * addresses that are on-link. 
    **************************************************************/
    ND6_PREFIX_ENTRY    prefix_list [ND6_PREFIX_LIST_SIZE]; 

    /* Redirect Table. Used only when target address != destination address. */
    ND6_REDIRECT_ENTRY  redirect_table[ND6_REDIRECT_TABLE_SIZE];  

#if RTCSCFG_ND6_RDNSS
    ND6_RDNSS_ENTRY     rdnss_list[RTCSCFG_ND6_RDNSS_LIST_SIZE];
#endif
   
    TCPIP_EVENT         timer;                  /* General ND timer.*/
    
    /* Router Discovery variables.*/
    uint32_t            rd_transmit_counter;    /* Counter used by RD. Equals to the number 
                                                 * of RS transmits till RD is finished.*/                                                    
    uint32_t            rd_time;                /* Time of last RS transmit.*/    
    
    /* Interface variables */  
    uint32_t            mtu;                    /* The recommended MTU for the link.
                                                 * Updated by RA messages.*/
    uint8_t             cur_hop_limit;          /* The default value that
                                                 * should be placed in the Hop Count field of the IP
                                                 * header for outgoing IP packets.*/
    uint32_t            reachable_time;         /* The time, in milliseconds,
                                                 * that a node assumes a neighbor is
                                                 * reachable after having received a reachability
                                                 * confirmation. Used by the Neighbor Unreachability
                                                 * Detection algorithm.*/ 
    uint32_t            retrans_timer;          /* The time, in milliseconds,
                                                 * between retransmitted Neighbor
                                                 * Solicitation messages. Used by address resolution
                                                 * and the Neighbor Unreachability Detection algorithm
                                                 * (see Sections 7.2 and 7.3).*/ 
    const ND6_IF        *nd6_interface;         /* Pointer to ND6 interface functions.*/
} ND6_CFG, * ND6_CFG_PTR;

/***********************************************************************
* Globals
***********************************************************************/
extern const ND6_IF nd6_if_standard;

/***********************************************************************
* Function Prototypes
***********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

uint32_t nd6_open(struct ip_if *if_ptr);
void nd6_close (struct ip_if *if_ptr);
ND6_NEIGHBOR_ENTRY_PTR nd6_neighbor_cache_get(struct ip_if *if_ptr, in6_addr *ip_addr);
ND6_NEIGHBOR_ENTRY_PTR nd6_neighbor_cache_add(struct ip_if *if_ptr, in6_addr *ip_addr, ll_addr_t ll_addr, nd6_neighbor_state_t state);
void nd6_neighbor_cache_del(struct ip_if *if_ptr, ND6_NEIGHBOR_ENTRY_PTR neighbor_entry);
ND6_PREFIX_ENTRY_PTR nd6_prefix_list_get(struct ip_if *if_ptr, in6_addr *prefix);
ND6_PREFIX_ENTRY_PTR nd6_prefix_list_add(struct ip_if *if_ptr, in6_addr *prefix, uint32_t prefix_length, uint32_t lifetime);
void nd6_prefix_list_del(ND6_PREFIX_ENTRY_PTR prefix_entry);
bool nd6_addr_is_onlink(struct ip_if *if_ptr, in6_addr *addr);
void nd6_router_list_add( ND6_NEIGHBOR_ENTRY_PTR neighbor_entry, uint32_t invalidation_time);
void nd6_router_list_del( ND6_NEIGHBOR_ENTRY_PTR neighbor_entry);
ND6_NEIGHBOR_ENTRY_PTR nd6_default_router_get(struct ip_if *if_ptr);
uint32_t nd6_neighbor_advertisement_send(struct ip_if *if_ptr, in6_addr *ipsrc, in6_addr *ipdest, uint8_t flags);
uint32_t nd6_neighbor_advertisement_receive(RTCSPCB_PTR rtcs_pcb);
uint32_t nd6_neighbor_solicitation_send(struct ip_if *if_ptr, in6_addr *ipsrc, in6_addr *ipdest, in6_addr *target_addr);
uint32_t nd6_neighbor_solicitation_receive(RTCSPCB_PTR rtcs_pcb);
uint32_t nd6_redirect_receive(RTCSPCB_PTR rtcs_pcb);
void nd6_redirect_addr(struct ip_if *if_ptr, in6_addr **destination_addr_p);
uint32_t nd6_router_solicitation_send(struct ip_if *if_ptr);
uint32_t nd6_router_advertisement_receive(RTCSPCB_PTR rtcs_pcb);
void nd6_neighbor_enqueue_waiting_pcb(ND6_NEIGHBOR_ENTRY_PTR neighbor_entry, RTCSPCB_PTR  waiting_pcb);
void nd6_neighbor_send_waiting_pcb(struct ip_if *if_ptr, ND6_NEIGHBOR_ENTRY_PTR neighbor_entry);
void nd6_dad_start(struct ip_if *if_ptr, struct ip6_if_addr_info *addr_info);
void nd6_rd_start(struct ip_if *if_ptr);

#if RTCSCFG_ND6_RDNSS
bool nd6_rdnss_get_addr(struct ip_if *netif, unsigned int n, in6_addr *addr_dns);
#endif

/* For Debug needs only */
void nd6_debug_print_prefix_list(struct ip_if *if_ptr );
void nd6_debug_print_neighbor_cache(struct ip_if *if_ptr );

#ifdef __cplusplus
}
#endif

#endif /* __nd6_h__ */

