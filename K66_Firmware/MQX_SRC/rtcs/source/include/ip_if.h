#ifndef __ip_if_h__
#define __ip_if_h__
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
* See license agreement file for full license terms including other
* restrictions.
*****************************************************************************
*
* Comments:
*
*   Internal definitions for the Internet Protocol Network Interfaces.
*
*
*END************************************************************************/

 
#include "ip6_prv.h"
#include "ip6_if.h"     
#include "tcpip.h"

/***************************************
**
** Type definitions
**
*/

#if RTCSCFG_ENABLE_IPIF_STATS
#define IF_IPIF_STATS_ENABLED(x) x
#else
#define IF_IPIF_STATS_ENABLED(x)
#endif

/******************************************************************************
 *    Interface features.
 ******************************************************************************/
typedef enum
{
    IP_IF_FEATURE_NONE                      = 0x00,     /* No special feature.*/
    IP_IF_FEATURE_HW_TX_IP_CHECKSUM         = 0x01,     /* If an IP frame is transmitted, the checksum is inserted automatically. The IP header checksum field
                                                         * must be cleared. If a non-IP frame is transmitted the frame is not modified.*/
    IP_IF_FEATURE_HW_TX_PROTOCOL_CHECKSUM   = 0x02,     /* If an IP frame with a known protocol is transmitted (UDP,TCP,ICMP), the checksum is inserted automatically into the
                                                         * frame. The checksum field must be cleared. The other frames are not modified.*/
    IP_IF_FEATURE_HW_RX_IP_CHECKSUM         = 0x04,     /* If an IPv4 frame is received with a mismatching header checksum, 
                                                         * the frame is discarded.*/
    IP_IF_FEATURE_HW_RX_PROTOCOL_CHECKSUM   = 0x08      /* If a TCP/IP, UDP/IP, or ICMP/IP frame is received that has a wrong TCP, UDP, or ICMP checksum,
                                                         * the frame is discarded.*/
} ip_if_feature_t;

/***********************************************************************
* DNS Server List entry.
***********************************************************************/
typedef struct ip_dns_entry
{
    _ip_address     dns_addr;           /* IPv4 address of a DNS Server. */
    uint32_t        creation_time;      /* Time of entry creation, in seconds.*/    
//TBD add DNS lifetime.
} IP_DNS_ENTRY, *IP_DNS_ENTRY_PTR;

/******************************************************************************
 * The IP interface
 *
 * This structure contains enough information to allow IP to
 * send packets through a packet driver.
 ******************************************************************************/
typedef struct ip_if
{
    struct ip_if            *next_if;                           /* Pointer to the next IF.*/
    struct ip_if            *prev_if;                           /* Pointer to the previous IF.*/
#if RTCSCFG_ENABLE_IPIF_STATS
    IPIF_STATS              STATS;
#endif
    void                    *HANDLE;
    RTCS_IF_STRUCT          DEVICE;
    uint32_t                MTU;
    void                    *ARP;

    /* fields for BOOTP/DHCP */
    void (_CODE_PTR_        BOOTFN)(RTCSPCB_PTR);
    void                    *BOOT;

    /* fields for ARP/BOOTP/DHCP */
    uint32_t                DEV_TYPE;
    uint32_t                DEV_ADDRLEN;
    unsigned char           DEV_ADDR[16];

    IP_DNS_ENTRY            dns_address[RTCSCFG_IP_IF_DNS_MAX]; /* DNSv4 Server List.*/ 
    
#if RTCSCFG_ENABLE_IGMP && RTCSCFG_ENABLE_IP4
    /* fields for IGMP */
    struct mc_member        *IGMP_MEMBER;
    uint32_t (_CODE_PTR_    IGMP_UNBIND)(struct ip_if *, _ip_address);
#ifdef IGMP_V2
    bool                    IGMP_V1_ROUTER_FLAG;
    uint32_t                IGMP_V1_ROUTER_TIMEOUT;
#endif
#endif

#if RTCSCFG_ENABLE_OSPF
    /* fields for Virata OSPF */
    void                    *IFO_ID;
#endif
#if RTCSCFG_ENABLE_SNMP && RTCSCFG_ENABLE_IP4
    /* fields for SNMP */
    uint32_t                SNMP_IF_TYPE;
#endif
    /* Calculates MTU. Needs DATA pointer from IP_ROUTE_VIRTUAL struct */
    uint32_t (_CODE_PTR_    MTU_FN)(void *);
#if RTCSCFG_ENABLE_IP6
    IP6_IF_STRUCT           IP6_IF;     
#endif
    uint32_t                FEATURES;
} IP_IF, * IP_IF_PTR;

/* the different types of interface. IP_IF.TYPE. (from ospf rfc2328.9) */
#define IP_IFT_POINT2POINT    0x01
#define IP_IFT_BROADCAST      0x02
#define IP_IFT_NBMA           0x03
#define IP_IFT_POINT2MULTI    0x04
#define IP_IFT_VIRTUALLINK    0x05

#ifdef __cplusplus
extern "C" {
#endif


extern IP_IF_PTR            IP_IF_LIST;    /* linked list of IP_IFs */

extern void IPIF_add         (IPIF_PARM_PTR);
extern void IPIF_bind        (IPIF_PARM_PTR);
extern void IPIF_bind_finish (IPIF_PARM_PTR);
extern void IPIF_bind_ppp    (IPIF_PARM_PTR);
extern void IPIF_gate_add    (IPIF_PARM_PTR);
/* Start CR 1016 */
extern void IPIF_gate_add_redirect(IPIF_PARM_PTR);
/* End CR 1016 */
extern void IPIF_remove      (IPIF_PARM_PTR);
extern void IPIF_unbind      (IPIF_PARM_PTR);
extern void IPIF_unbind_ppp  (IPIF_PARM_PTR);
extern void IPIF_gate_remove (IPIF_PARM_PTR);

void ip_if_add_dns_addr(IPIF_PARM_PTR);
void ip_if_del_dns_addr(IPIF_PARM_PTR);

IP_IF_PTR ip_if_list_get(uint32_t n);
bool ip_if_is_joined(IP_IF_PTR if_ptr, _ip_address group_addr);

/* Local Host Interface.*/
extern IP_IF_PTR   RTCS_IF_LOCALHOST_PRV;

/*
** The local host interface
*/

uint32_t IPLOCAL_open
(
   IP_IF_PTR   if_ptr      /* [IN] the IP interface structure */
);

uint32_t IPLOCAL_close
(
   IP_IF_PTR   if_ptr      /* [IN] the IP interface structure */
);

uint32_t IPLOCAL_send
(
   IP_IF_PTR   if_ptr   ,  /* [IN] the IP interface structure */
   RTCSPCB_PTR pcb      ,  /* [IN] the packet to send */
   _ip_address src      ,  /* [IN] the next-hop source address */
   _ip_address dest     ,  /* [IN] the next-hop destination address */
   void       *data        /* [IN] unused */
);

uint32_t IPLOCAL_service
(
   RTCSPCB_PTR pcb         /* [IN] the packet to deliver */
);

uint32_t IPLOCAL_init (void);

#ifdef __cplusplus
}
#endif

#endif /* __ip_if_h__ */

