/*HEADER**********************************************************************
*
* Copyright 2008, 2014 Freescale Semiconductor, Inc.
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
*   This file contains various IP layer functions.
*
*
*END************************************************************************/

#include <rtcs.h>
#include "rtcs_prv.h"
#include "tcpip.h"
#include "ip_prv.h"

#if RTCSCFG_ENABLE_IP4 

/* Used for most of the IP utility functions */
struct IP_util_struct {
   IP_IF_PTR      ipif;
   _ip_address    ipaddr;
   bool           iputil_bool;
   uint32_t       num;
};


/*FUNCTION*-------------------------------------------------------------
*
* Function Name    : IP_get_netmask
* Returned Value   : bool
* Comments         : Gets the network mask. Returns TRUE if found, or
*                    FALSE if no netmask is found.
*
*END*-----------------------------------------------------------------*/

struct IPIF_getmask_struct {
   IP_IF_PTR      netif;
   _ip_address    address;
   _ip_address    mask;
};

static bool IP_get_netmask_test
   (
      void    *node,
      void    *data
   )
{ /* Body */
   IP_ROUTE_PTR                      route = node;
   struct IPIF_getmask_struct       *getmask = data;
   IP_ROUTE_DIRECT_PTR               search_ptr;

   if (route->DIRECT) {
      search_ptr = route->DIRECT;
      do {
         if (search_ptr->ADDRESS == getmask->address &&
             search_ptr->DESTIF == getmask->netif)
         {
            getmask->mask = route->NODE.MASK;
            return TRUE;
         } /* Endif */
         search_ptr = search_ptr->NEXT;
      } while(search_ptr != route->DIRECT);
   } /* Endif */

   return FALSE;

} /* Endbody */


bool IP_get_netmask
   (
      _rtcs_if_handle   ihandle,    /* [IN] Interface */
      _ip_address       address,    /* [IN] IP address */
      _ip_address  *mask_ptr    /* [OUT] netwask for the IP and interface */
   )
{ /* Body */
   IP_CFG_STRUCT_PTR       IP_cfg_ptr = RTCS_getcfg( IP );
   struct IPIF_getmask_struct    getmask;

   getmask.netif     = ihandle;
   getmask.address   = address;
   getmask.mask      = 1;   /* Invalid mask */

   IPRADIX_findbest(&IP_cfg_ptr->ROUTE_ROOT.NODE, address,
      IP_get_netmask_test, &getmask);

   if (getmask.mask == 1) {
      return FALSE;
   } /* Endif */

   *mask_ptr = getmask.mask;
   return TRUE;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IP_get_ipif_addr
*  Returned Value : the ip address
*  Comments       : return the ip address of the interface
*
*END*-----------------------------------------------------------------*/

static bool IP_get_ipif_addr_test
   (
      _ip_address    node_ip,
      _ip_address    node_mask,
      void          *node_data,
      void          *data
   )
{ /* Body */
   struct IP_util_struct        *testdata = data;
   IP_ROUTE_PTR                  route = node_data;
   IP_ROUTE_DIRECT_PTR           search_ptr;

   /* Make sure there are direct interfaces, with full IP addresses */
   if (!~node_mask && route && route->DIRECT) {
      search_ptr = route->DIRECT;
      do {
         if (search_ptr->NETIF == testdata->ipif) {
            testdata->ipaddr = node_ip;
            return TRUE;
         } /* Endif */
         search_ptr = search_ptr->NEXT;
      } while(search_ptr != route->DIRECT);
   } /* Endif */

   return FALSE;

} /* Endbody */


_ip_address IP_get_ipif_addr
   (
      IP_IF_PTR      ipif        /* [IN] the local interface */
   )
{ /* Body */
   IP_CFG_STRUCT_PTR       IP_cfg_ptr = RTCS_getcfg( IP );
   struct IP_util_struct   testdata;

   testdata.ipif     = ipif;
   testdata.ipaddr   = INADDR_ANY;

   /* Check all nodes of the tree */
   IPRADIX_walk(&IP_cfg_ptr->ROUTE_ROOT.NODE, IP_get_ipif_addr_test, &testdata);

   return testdata.ipaddr;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IP_is_local
*  Returned Value : TRUE or FALSE
*  Comments       :
*        Decides whether or not an IP address is local to an interface.
*        if iflocal is null, check all the interfaces.
*
*END*-----------------------------------------------------------------*/

static void IP_is_local_test
   (
      void     **node,
      void          *data
   )
{ /* Body */
   struct IP_util_struct        *testdata = data;
   IP_ROUTE_PTR                  route = NULL;
   IP_ROUTE_DIRECT_PTR           search_ptr = NULL;

   if (*node) {
      route = *node;
      search_ptr = route->DIRECT;
   } /* Endif */

   if (search_ptr) {
      do {
         if ((search_ptr->NETIF == testdata->ipif || testdata->ipif == NULL) &&
             (testdata->ipaddr == search_ptr->ADDRESS))
         {
            testdata->iputil_bool = TRUE;
            break;
         } /* Endif */
         search_ptr = search_ptr->NEXT;
      } while(search_ptr != route->DIRECT);
   } /* Endif */

} /* Endbody */


bool IP_is_local
   (
      IP_IF_PTR      iflocal,    /* [IN] the local interface */
      _ip_address    iplocal     /* [IN] the IP address to test */
   )
{ /* Body */
   IP_CFG_STRUCT_PTR       IP_cfg_ptr = RTCS_getcfg(IP);
   struct IP_util_struct   testdata;

   testdata.ipif        = iflocal;
   testdata.ipaddr      = iplocal;
   testdata.iputil_bool = FALSE;

   /* Will make testdata.iputil_bool TRUE if ip is local to interface */
   IPRADIX_insert(&IP_cfg_ptr->ROUTE_ROOT.NODE, iplocal, 0xFFFFFFFF, 0,
      IP_is_local_test, &testdata);

   return testdata.iputil_bool;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IP_is_direct
*  Returned Value : TRUE or FALSE
*  Comments       :
*        Decides whether or not an IP address can be directly reached
*        through a given local interface.
*
*END*-----------------------------------------------------------------*/

static bool IP_is_direct_test
   (
      void          *node_data,
      void          *data
   )
{ /* Body */
   struct IP_util_struct        *testdata = data;
   IP_ROUTE_PTR                  route = node_data;

   if (route->DIRECT) {
      IP_ROUTE_DIRECT_PTR direct = route->DIRECT;
      do {
         if ((direct->DESTIF == testdata->ipif || testdata->ipif == NULL)) {
            testdata->iputil_bool = TRUE;
            return TRUE;
         } /* Endif */
         direct = direct->NEXT;
      } while(direct != route->DIRECT);
   } /* Endif */

   if (route->VIRTUAL) {
      IP_ROUTE_VIRTUAL_PTR virtual = route->VIRTUAL;
      do {
         if ((virtual->DESTIF == testdata->ipif || testdata->ipif == NULL)) {
            testdata->iputil_bool = TRUE;
            return TRUE;
         } /* Endif */
         virtual = virtual->NEXT;
      } while(virtual != route->VIRTUAL);
   } /* Endif */

   return FALSE;
} /* Endbody */


bool IP_is_direct
   (
      IP_IF_PTR      iflocal,
       /* [IN] the local interface */
      _ip_address    iplocal
       /* [IN] the IP address to test */
   )
{ /* Body */
   IP_CFG_STRUCT_PTR       IP_cfg_ptr = RTCS_getcfg(IP);
   struct IP_util_struct   testdata;

   testdata.ipif        = iflocal;
   testdata.ipaddr      = iplocal;
   testdata.iputil_bool = FALSE;

   IPRADIX_findbest(&IP_cfg_ptr->ROUTE_ROOT.NODE, iplocal,
      IP_is_direct_test, &testdata);

   return testdata.iputil_bool;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IP_is_gate
*  Returned Value : TRUE or FALSE
*  Comments       :
*        Decides whether or not an IP address is a gateway for another.
*
*END*-----------------------------------------------------------------*/

static bool IP_is_gate_test
   (
      void          *node_data,
      void          *data
   )
{ /* Body */
   struct IP_util_struct        *testdata = data;
   IP_ROUTE_PTR                  route = node_data;
   IP_ROUTE_INDIRECT_PTR         indirect;

   if (route->INDIRECT) {
      indirect = route->INDIRECT;
      do {
         if (indirect->GATEWAY == testdata->ipaddr) {
            testdata->iputil_bool = TRUE;
            return TRUE;
         } /* Endif */
         indirect = indirect->NEXT;
      } while(indirect != route->INDIRECT);
   } /* Endif */

   return FALSE;

} /* Endbody */


bool IP_is_gate
   (
      _ip_address    gateway,
            /* [IN] the gateway */
      _ip_address    ipremote
            /* [IN] the IP address to test */
   )
{ /* Body */
   IP_CFG_STRUCT_PTR       IP_cfg_ptr = RTCS_getcfg(IP);
   struct IP_util_struct   data;

   data.ipaddr      = gateway;
   data.iputil_bool = FALSE;

   IPRADIX_findbest(&IP_cfg_ptr->ROUTE_ROOT.NODE, ipremote,
      IP_is_gate_test, &data);

   return data.iputil_bool;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IP_MTU
*  Returned Value : MTU for destination
*  Comments       :
*        Determines the Maximum Transmission Unit for a destination;
*        returns MTU of local ip_address if destination is not directly connected.
*
*END*-----------------------------------------------------------------*/

static bool IP_MTU_test
   (
      void          *node_data,
      void          *data
   )
{ /* Body */
   struct IP_util_struct        *testdata = data;
   IP_ROUTE_PTR                  route = node_data;
   IP_ROUTE_VIRTUAL_PTR          virtual;

   if (route->DIRECT) {
      testdata->num = route->DIRECT->DESTIF->MTU;
      return TRUE;
   } /* Endif */

   virtual = route->VIRTUAL;
   if (virtual) {
      if ((testdata->ipaddr & virtual->SOURCE_MASK) == virtual->SOURCE_NET) {
         if (virtual->DESTIF->MTU_FN) {
            testdata->num = virtual->DESTIF->MTU_FN(virtual->DATA);
         } else {
            testdata->num = virtual->DESTIF->MTU;
         } /* Endif */
         return TRUE;
      } /* Endif */
   } /* Endif */

   return FALSE;

} /* Endbody */


uint32_t IP_MTU
   (
      _ip_address    iplocal,
            /* [IN] the local IP address */
      _ip_address    ipremote
            /* [IN] the remote IP address to test */
   )
{ /* Body */
   IP_CFG_STRUCT_PTR       IP_cfg_ptr = RTCS_getcfg(IP);
   struct IP_util_struct   data;
   uint32_t local_if_mtu = IP_DEFAULT_MTU;
   IP_IF_PTR if_ptr = NULL;
   
   /* default MTU will be the MTU of the iplocal interface. */
   if_ptr = IP_find_if(iplocal);
   if(if_ptr)
   {
      local_if_mtu = if_ptr->MTU;
   }

   data.ipaddr = iplocal;   
   data.num = local_if_mtu;

   IPRADIX_findbest(&IP_cfg_ptr->ROUTE_ROOT.NODE, ipremote,
      IP_MTU_test, &data);

   return data.num;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IP_find_if
*  Returned Value : interface handle
*  Comments       :
*        Returns the interface handle for a local IP address.
*
*END*-----------------------------------------------------------------*/

static void IP_find_if_test
   (
      void     **node,
      void          *data
   )
{ /* Body */
   struct IP_util_struct        *testdata = data;
   IP_ROUTE_PTR                  route;

   if (*node) {
      route = *node;
      if (route->DIRECT) {
         testdata->ipif = route->DIRECT->NETIF;
      } /* Endif */
   } /* Endif */

} /* Endbody */


IP_IF_PTR IP_find_if
   (
      _ip_address    iplocal
            /* [IN] the IP address to test */
   )
{ /* Body */
   IP_CFG_STRUCT_PTR       IP_cfg_ptr = RTCS_getcfg(IP);
   struct IP_util_struct   data;

   data.ipif     = NULL;
   data.ipaddr   = iplocal;

   IPRADIX_insert(&IP_cfg_ptr->ROUTE_ROOT.NODE, iplocal, 0xFFFFFFFF, 0,
      IP_find_if_test, &data);

   return data.ipif;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IP_route_find
*  Returned Value : source IP address
*  Comments       :
*        Determine the source IP address to use when routing to a
*        specific destination.
*
*END*-----------------------------------------------------------------*/

static bool IP_route_find_test
   (
      void    *node,
      void    *data
   )
{ /* Body */
   struct IP_util_struct        *testdata = data;
   IP_ROUTE_PTR                  route = node;

   /* Check for a direct route. */
   if (route->DIRECT) {
      testdata->ipaddr = route->DIRECT->ADDRESS;
      return TRUE;
   } /* Endif */

   /* Check for a numbered virtual route. */
   if (route->VIRTUAL) {
      IP_ROUTE_VIRTUAL_PTR virtual = route->VIRTUAL;
      do {
         if (virtual->ADDRESS) {
            testdata->ipaddr = virtual->ADDRESS;
            return TRUE;
         } /* Endif */
         virtual = virtual->NEXT;
      } while (virtual != route->VIRTUAL);
   } /* Endif */

   /*
   ** If no direct route was found, search for a gateway.
   */
   if (testdata->iputil_bool && route->INDIRECT) {
      IP_ROUTE_INDIRECT_PTR indirect = route->INDIRECT;
      do {
         if (indirect->FLAGS & RTF_UP) {
            testdata->ipaddr      = indirect->GATEWAY;
            testdata->iputil_bool = FALSE;
            return TRUE;
         } /* Endif */
         indirect = indirect->NEXT;
      } while (indirect != route->INDIRECT);
   } /* Endif */

   return FALSE;
} /* Endbody */


_ip_address IP_route_find
   (
      _ip_address    ipdest,
            /* [IN] the ultimate destination */
      uint32_t        flags
            /* [IN] optional flags */
   )
{ /* Body */
   IP_CFG_STRUCT_PTR       IP_cfg_ptr = RTCS_getcfg(IP);
   struct IP_util_struct   data;
   _ip_address             gateway;

   data.ipaddr  = INADDR_ANY;
   if (!flags) {
      data.iputil_bool = TRUE;       /* Check gateways */
   } else {
      data.iputil_bool = FALSE;
   } /* Endif */

   IPRADIX_findbest(&IP_cfg_ptr->ROUTE_ROOT.NODE, ipdest,
      IP_route_find_test, &data);

   /* If we have found a gateway, find source addr for the gateway */
   if (!data.iputil_bool) {
      gateway = data.ipaddr;
      data.ipaddr = INADDR_ANY;
      IPRADIX_findbest(&IP_cfg_ptr->ROUTE_ROOT.NODE, gateway,
         IP_route_find_test, &data);
   } /* Endif */

   return data.ipaddr;
} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : RTCS_get_enet_handle
*  Returned Value : ethernet handle (void *)
*  Comments       :
*        Returns the ethernet handle associated with given interface.
*
*END*-----------------------------------------------------------------*/

void *RTCS_get_enet_handle
   (
      _rtcs_if_handle   ihandle    /* [IN] Interface */
   )
{
   return ((IP_IF_PTR)ihandle)->HANDLE;
}

#if RTCSCFG_ENABLE_IP6

/************************************************************************
* NAME: IP6_MTU
* RETURNS : MTU for destination
* DESCRIPTION: Determines the Maximum Transmission Unit for a destination;
*        returns IP6_DEFAULT_MTU if destination is not directly connected
*        or source is not our address.
*************************************************************************/
uint32_t IP6_MTU ( in6_addr *iplocal     /* [IN] the local IP address */,
                  in6_addr *ipremote    /* [IN] the remote IP address to test */ )
{ 
    uint32_t mtu = IP_DEFAULT_MTU;
    IP_IF_PTR dest_if; 

    if( iplocal == NULL)
    /* Determine a source address.*/
    {
        iplocal = (in6_addr *)ip6_if_select_src_addr(NULL, ipremote);
    }

    dest_if = ip6_if_get_by_addr(iplocal);
        
    if(dest_if)
    {
        mtu = dest_if->IP6_IF.ND6->mtu;
    }
   
    return mtu;
}

/************************************************************************
* NAME: IP6_route_find
* RETURNS : Source IPv6 address.
* DESCRIPTION: Determine the source IPv6 address to use when routing to a
*              specific destination.
*************************************************************************/
in6_addr * IP6_route_find( in6_addr *ipdest   /* [IN] the ultimate destination */)
{
    in6_addr * addr = (in6_addr *)ip6_if_select_src_addr(NULL, ipdest); 
    
    if(IN6_IS_ADDR_UNSPECIFIED(addr))
        addr = NULL;

    return addr;
}

#endif /* RTCSCFG_ENABLE_IP6 */


/* EOF */
