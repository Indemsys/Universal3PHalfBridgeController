#ifndef __rtcs_if_h__
#define __rtcs_if_h__
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
*   RTCS Network Interface API.
*
*
*END************************************************************************/


/******************************************************************
 * How the address was acquired.
 ******************************************************************/
typedef enum
{
    IP6_ADDR_TYPE_MANUAL = 0,           /* Manually.*/
    IP6_ADDR_TYPE_AUTOCONFIGURABLE = 1,  /* Autoconfigurable address. */
    IP6_ADDR_TYPE_DHCP = 2               /* Address set by DHCP. */
} rtcs6_if_addr_type;

/******************************************************************
 * Possible states for the address of an interface.
 ******************************************************************/
typedef enum
{
    IP6_ADDR_STATE_NOT_USED = 0,    /* Not used.*/
    IP6_ADDR_STATE_TENTATIVE = 1,	/* Tentative address - (RFC4862) an address whose uniqueness on a link is being
                                     * verified, prior to its assignment to an interface. A tentative
                                     * address is not considered assigned to an interface in the usual
                                     * sense. An interface discards received packets addressed to a
                                     * tentative address, but accepts Neighbor Discovery packets related
                                     * to Duplicate Address Detection for the tentative address.
                                     */
    IP6_ADDR_STATE_PREFERRED = 2 	/* Prefered address - (RFC4862) an address assigned to an interface whose use by
                                     * upper-layer protocols is unrestricted. Preferred addresses may be
                                     * used as the source (or destination) address of packets sent from
                                     * (or to) the interface.
                                     */
} rtcs6_if_addr_state;

#define IP6_ADDR_LIFETIME_INFINITE  (0xFFFFFFFF)    /* A lifetime value of all one bits (0xffffffff) represents infinity. */

/**********************************************************************
 * Link-layer address.
 * For example, Ethernet interafce uses the address with size set to 6.
***********************************************************************/
typedef unsigned char ll_addr_t[16];
/*
 * Copying Link-layer address.
 */
#define LL_ADDR_COPY(from_addr, to_addr, ll_size)   \
        (_mem_copy(&from_addr[0], &to_addr[0], ll_size))

/*
 * Equality.
 */
#define LL_ADDR_ARE_EQUAL(a, b, size)               \
        (memcmp(&a[0], &b[0], size) == 0)


/*
** Type definitions for RTCS_if_XXX()
*/

typedef void * _rtcs_if_handle;
struct ip_if;       /* Forward declaration.*/
struct rtcspcb;     /* Forward declaration.*/     
struct in6_addr;    /* Forward declaration.*/

typedef struct  rtcs_if_struct
{
    uint32_t (_CODE_PTR_  OPEN) (struct ip_if*);
    uint32_t (_CODE_PTR_  CLOSE)(struct ip_if*);
#if RTCSCFG_ENABLE_IP4
    uint32_t (_CODE_PTR_  SEND) (struct ip_if*, struct rtcspcb*, _ip_address, _ip_address, void*);
    uint32_t (_CODE_PTR_  JOIN) (struct ip_if*, _ip_address);
    uint32_t (_CODE_PTR_  LEAVE)(struct ip_if*, _ip_address);
#endif
#if RTCSCFG_ENABLE_IP6
    uint32_t (_CODE_PTR_  SEND6) (struct ip_if*, struct rtcspcb*, struct in6_addr*, struct in6_addr*);
    uint32_t (_CODE_PTR_  JOIN6) (struct ip_if*, const struct in6_addr*);
    uint32_t (_CODE_PTR_  LEAVE6)(struct ip_if*, const struct in6_addr*);
#endif
    bool (_CODE_PTR_  LINK_STATUS)(struct ip_if*); /* Optional.*/
} RTCS_IF_STRUCT, * RTCS_IF_STRUCT_PTR;

/***********************************************************************
* Neighbor Cache entry, returned by RTCS6_if_get_neighbor_cache_entry()
***********************************************************************/
typedef struct rtc6_if_neighbor_cache_entry
{
    in6_addr        ip_addr;                    /* Neighbor’s on-link unicast IP address. */
    ll_addr_t       ll_addr;                    /* Its link-layer address. Actual size is defined by ll_addr_size.*/
    uint32_t        ll_addr_size;               /* Size of link-layer address.*/
    bool            is_router;                  /* A flag indicating whether the neighbor is a router (TRUE) or a host (FALSE).*/
} RTCS6_IF_NEIGHBOR_CACHE_ENTRY, *RTCS6_IF_NEIGHBOR_CACHE_ENTRY_PTR;

/***********************************************************************
* Prefix List entry, returned by RTCS6_if_get_prefix_list_entry()
***********************************************************************/
typedef struct rtc6_if_prefix_list_entry
{
    in6_addr            prefix;         /* Prefix of an IP address. */
    uint32_t            prefix_length;  /* Prefix length (in bits). The number of leading bits
                                         * in the Prefix that are valid. */
} RTCS6_IF_PREFIX_LIST_ENTRY, *RTCS6_IF_PREFIX_LIST_ENTRY_PTR;




extern const RTCS_IF_STRUCT_PTR  RTCS_IF_LOCALHOST;
extern const RTCS_IF_STRUCT_PTR  RTCS_IF_ENET;
extern const RTCS_IF_STRUCT_PTR  RTCS_IF_PPP;

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t RTCS_if_add
(
   void*                      ,  /* [IN] the packet driver handle */
   RTCS_IF_STRUCT_PTR         ,  /* [IN] call table for the interface */
   _rtcs_if_handle*              /* [OUT] the RTCS interface state structure */
);
extern uint32_t RTCS_if_remove
(
   _rtcs_if_handle               /* [IN] the RTCS interface state structure */
);
extern uint32_t RTCS_if_bind
(
   _rtcs_if_handle   handle   ,  /* [IN] the RTCS interface state structure */
   _ip_address       address  ,  /* [IN] the IP address for the interface */
   _ip_address       netmask     /* [IN] the IP (sub)network mask for the interface */
);
extern uint32_t RTCS_if_probe_and_bind
(
   _rtcs_if_handle   handle   ,  /* [IN] the RTCS interface state structure */
   _ip_address       address  ,  /* [IN] the IP address for the interface */
   _ip_address       netmask     /* [IN] the IP (sub)network mask for the interface */
);


extern uint32_t RTCS_arp_add
(
   _rtcs_if_handle *ihandle,  /* [IN] the RTCS interface state structure */
   _ip_address      paddr,    /* [IN] the address to add */
   char             laddr[6]
);
extern uint32_t RTCS_arp_delete
(
   _rtcs_if_handle  *ihandle,  /* [IN] the RTCS interface state structure */
   _ip_address       paddr     /* [IN] the address to add */
);

extern uint32_t RTCS_if_unbind
(
   _rtcs_if_handle   handle   ,  /* [IN] the RTCS interface state structure */
   _ip_address       address     /* [IN] the IP address for the interface */
);

extern void *RTCS_get_enet_handle
(
   _rtcs_if_handle   ihandle           /* [IN] Interface */
);

extern bool RTCS_if_get_dns_addr(_rtcs_if_handle ihandle, uint32_t n, _ip_address *dns_addr);
extern uint32_t RTCS_if_add_dns_addr(_rtcs_if_handle ihandle, _ip_address dns_addr);
extern uint32_t RTCS_if_del_dns_addr(_rtcs_if_handle ihandle, _ip_address dns_addr);
extern _rtcs_if_handle RTCS_if_get_handle(uint32_t n);
extern bool RTCS_if_get_link_status(_rtcs_if_handle ihandle);
extern uint32_t RTCS_if_get_mtu(_rtcs_if_handle ihandle);
extern _ip_address RTCS_if_get_addr(_rtcs_if_handle ihandle);

/*********************************************************************
 * IPv6 Address information structure, used by RTCS6_if_get_addr.
 *********************************************************************/
typedef struct rtcs6_if_addr_info
{
    in6_addr            ip_addr;                    /* IPv6 address.*/
    rtcs6_if_addr_state ip_addr_state;              /* Address current state.*/
    rtcs6_if_addr_type  ip_addr_type;               /* How the address was acquired.*/
} RTCS6_IF_ADDR_INFO, * RTCS6_IF_ADDR_INFO_PTR;

extern uint32_t RTCS6_if_get_addr(_rtcs_if_handle ihandle, uint32_t n, RTCS6_IF_ADDR_INFO_PTR addr_info);
extern uint32_t RTCS6_if_bind_addr (_rtcs_if_handle ihandle, in6_addr *addr, rtcs6_if_addr_type addr_type, uint32_t addr_lifetime);
extern uint32_t RTCS6_if_unbind_addr (_rtcs_if_handle  ihandle, in6_addr *addr);
extern uint32_t RTCS6_if_get_scope_id (_rtcs_if_handle ihandle); 
extern bool RTCS6_if_get_dns_addr(_rtcs_if_handle ihandle, uint32_t n, in6_addr *addr_dns);
extern uint32_t RTCS6_if_add_dns_addr(_rtcs_if_handle ihandle, in6_addr *dns_addr);
extern uint32_t RTCS6_if_del_dns_addr(_rtcs_if_handle ihandle, in6_addr *dns_addr);
extern bool RTCS6_if_get_neighbor_cache_entry(_rtcs_if_handle ihandle, uint32_t n, RTCS6_IF_NEIGHBOR_CACHE_ENTRY_PTR neighbor_cache_entry);
extern bool RTCS6_if_get_prefix_list_entry(_rtcs_if_handle ihandle, uint32_t n, RTCS6_IF_PREFIX_LIST_ENTRY_PTR prefix_list_entry);
extern bool RTCS6_if_is_disabled(_rtcs_if_handle ihandle);

#ifdef __cplusplus
}
#endif

#endif /* __rtcs_if_h__ */


/* EOF */
