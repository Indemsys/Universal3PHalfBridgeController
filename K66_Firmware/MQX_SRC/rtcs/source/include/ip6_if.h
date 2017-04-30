#ifndef __ip6_if_h__
#define __ip6_if_h__
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
*   Definitions for the Internet Protocol 6 Network Interfaces.
*
*
*END************************************************************************/

#include <rtcs.h>
#include "nd6.h"
#include "ipcfg.h"
#include "ip6.h"
/* Maxinmum number of IPv6 addresses per interface.*/ 
#define IP6_IF_ADDRESSES_MAX    (RTCSCFG_IP6_IF_ADDRESSES_MAX)

/* A lifetime value of all one bits (0xffffffff) represents infinity. */
#define IP6_IF_ADDRESS_LIFETIME_INFINITE    ND6_PREFIX_LIFETIME_INFINITE   

/*********************************************************************
 * Address information structure.
 *********************************************************************/
typedef struct ip6_if_addr_info
{
    in6_addr            ip_addr;                    /* IPv6 address.*/
    rtcs6_if_addr_state ip_addr_state;              /* Address current state.*/
    rtcs6_if_addr_type  ip_addr_type;               /* How the address was acquired.*/
    in6_addr            solicited_multicast_addr;   /* Solicited-node multicast 
                                                     * group-address for assigned ip_addr.*/
    uint32_t            creation_time;              /* Time of entry creation (in seconds).*/    
    uint32_t            lifetime;                   /* Address lifetime (in seconds). 0xFFFFFFFF = Infinite Lifetime
                                                     * RFC4862. A link-local address has an infinite preferred and valid lifetime; it
                                                     * is never timed out.*/
    uint32_t            prefix_length;              /* Prefix length (in bits). The number of leading bits
                                                     * in the Prefix that are valid. */                                        
    uint32_t            dad_transmit_counter;       /* Counter used by DAD. Equals to the number 
                                                     * of NS transmits till DAD is finished.*/                                                    
    uint32_t            state_time;                 /* Time of last state event.*/  
                                                    
} IP6_IF_ADDR_INFO, * IP6_IF_ADDR_INFO_PTR;

struct ip_if; /* Just to avoid header conflicts.*/ //TBD 

/***********************************************************************
* DNS Server List entry.
***********************************************************************/
typedef struct ip6_dns_entry
{
    in6_addr    dns_addr;           /* IPv6 address of a DNS Server. */
    uint32_t    creation_time;      /* Time of entry creation, in seconds.*/    
//TBD add DNS lifetime.
} IP6_DNS_ENTRY, *IP6_DNS_ENTRY_PTR;

/**********************************************************************
* Extension to the ip_if structure (in ip_prv.h).
***********************************************************************/
typedef struct ip6_if_struct
{
    uint32_t                scope_id;
    /* The IPv6 addressing architecture [1] allows multiple unicast
     * addresses to be assigned to interfaces. */
    IP6_IF_ADDR_INFO        address[IP6_IF_ADDRESSES_MAX];      /* RFC4862 5.2: A host maintains a list of addresses together with their
                                                                * corresponding lifetimes. The address list contains both
                                                                * autoconfigured addresses and those configured manually.*/
    IP6_DNS_ENTRY           dns_address[RTCSCFG_IP6_IF_DNS_MAX];/* DNSv6 Server List.*/ 

    ND6_CFG_PTR             ND6;                                /* Pointer to ND control structure.*/
    void                    (*route)(RTCSPCB_PTR rtcs_pcb);     /* Routing ptotocol service function.*/
    bool                    ip6_disabled;                       /* IP operation on the interface is disabled.*/
#if RTCSCFG_ENABLE_MLD
    bool                    mld_invalid;                        /* Flag that, MLD message was sent with the unspecified address.
                                                                 * Once a valid link-local address is available, a node SHOULD generate
                                                                 * new MLD Report messages for all multicast addresses joined on the
                                                                 * interface.*/
#endif
#if RTCSCFG_IP6_PMTU_DISCOVERY
    uint32_t                pmtu;                               /* Path MTU, changed by Path MTU Discovery for IPv6.*/
    uint32_t                pmtu_timestamp;                     /* The timestamp, in seconds, when PMTU was changed last time.*/
    TCPIP_EVENT             pmtu_timer;                         /* PMTU timer,used to detect increases in PMTU.*/
#endif
} IP6_IF_STRUCT, * IP6_IF_STRUCT_PTR;

/************************************************************************
*     Function Prototypes
*************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

bool ip6_if_is_my_addr(struct ip_if *if_ptr, in6_addr *ip_addr);
bool ip6_if_is_my_solicited_multicast_addr(struct ip_if *ihandle, in6_addr *ip_addr);
bool ip6_if_get_addr(struct ip_if *if_ptr, uint32_t n, IP6_IF_ADDR_INFO_PTR addr_info); 
IP6_IF_ADDR_INFO_PTR ip6_if_get_addr_info(struct ip_if *if_ptr, in6_addr *ip_addr);
in6_addr *ip6_if_get_addr_valid_link_local(struct ip_if *if_ptr);
const in6_addr * ip6_if_select_src_addr(struct ip_if *if_ptr, in6_addr *dest_addr);
void ip6_if_unbind_addr ( struct ip_if *if_ptr, IP6_IF_ADDR_INFO_PTR if_addr );
uint32_t ip6_if_bind_addr (struct ip_if *if_ptr, in6_addr *addr, rtcs6_if_addr_type addr_type, uint32_t lifetime, uint32_t prefix_length);
uint32_t ip6_if_join (struct ip_if *if_ptr, const in6_addr *group_ip);
uint32_t ip6_if_leave(struct ip_if *if_ptr, const in6_addr *group_ip);
void ip6_multicast_leave_all(struct ip_if *netif);
struct ip_if *ip6_if_get_by_addr(in6_addr *addr);
struct ip_if *ip6_if_get_by_scope_id(uint32_t scope_id);
void ip6_if_assign_scope_id(struct ip_if *if_ptr);
int ip6_if_addr_autoconf_set(struct ip_if *if_ptr, struct in6_addr *ipaddr);
void ip6_if_addr_timer(struct ip_if *if_ptr);
void ip6_if_route_instal(struct ip_if *if_ptr, void (*route)(RTCSPCB_PTR rtcs_pcb));  /* Instalation of Routing ptotocol service function.*/
#if RTCSCFG_IP6_PMTU_DISCOVERY
void ip6_if_pmtu_init(struct ip_if *if_ptr);
void ip6_if_pmtu_release(struct ip_if *if_ptr);
void ip6_if_pmtu_set(struct ip_if *if_ptr, uint32_t pmtu);
#endif
void ip6_if_bind_addr_cmd(IP6_IF_PARM_PTR);
void ip6_if_unbind_addr_cmd(IP6_IF_PARM_PTR);
void ip6_if_add_dns_addr(IP6_IF_PARM_PTR);
void ip6_if_del_dns_addr(IP6_IF_PARM_PTR);

void IP6LOCAL_open(struct ip_if *if_ptr);
uint32_t IP6LOCAL_send(struct ip_if *if_ptr, RTCSPCB_PTR rtcs_pcb, in6_addr *src, in6_addr *dest);


/* For DEBUG needs only. */
void ip6_if_debug_print_addr_info( struct ip_if *if_ptr );

#ifdef __cplusplus
}
#endif

#endif /* __ip6_if_h__ */

