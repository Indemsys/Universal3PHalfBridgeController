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
*   This file contains the implementation of the
*   packet driver interface.
*
*
*END************************************************************************/

#include <rtcs.h>
#include "rtcs_prv.h"
#include "tcpip.h"
#include "ip_prv.h"
#include <arp.h>

IP_IF_PTR         IP_IF_LIST =  NULL;     /* linked list of IP_IFs */ 

static void ip_if_list_add( IP_IF_PTR if_ptr );
static void ip_if_list_del( IP_IF_PTR if_ptr );

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IPIF_add
*  Parameters     :
*
*     pointer              mhandle     [IN] the packet driver handle
*     _rtcs_if_handle      ihandle     [OUT] the IP interface handle
*     RTCS_IF_STRUCT_PTR   if_ptr      [IN] the call table
*     _ip_address          address     not used
*     _ip_address          locmask     not used
*     _ip_address          network     not used
*     _ip_address          netmask     not used
*
*  Comments       :
*        Registers a hardware interface with RTCS.
*
*END*-----------------------------------------------------------------*/
void IPIF_add(IPIF_PARM_PTR  parms)
{ 
    IP_IF_PTR           ipif;
    uint32_t            error;

    /* Allocate interface structure */
    ipif = RTCS_mem_alloc_zero(sizeof(IP_IF));

    /* Dequeue a free entry */
    if (!ipif)
    {
        RTCSCMD_complete(parms, RTCSERR_IP_IF_ALLOC);
        return;
    } 
    
    _mem_set_type(ipif, MEM_TYPE_IP_IF);

    /* Initialize the interface */
    ipif->HANDLE = parms->mhandle;
    ipif->DEVICE = *parms->if_ptr;

    ip_if_list_add(ipif);

    error = ipif->DEVICE.OPEN(ipif);
    if (error)
    {
        ip_if_list_del(ipif);
        _mem_free(ipif);
        RTCSCMD_complete(parms, error);
        return;
    }

#if RTCSCFG_ENABLE_IGMP && RTCSCFG_ENABLE_IP4
    /* If IGMP_ipif_add fails, it isn't sufficient reason to abort IPIF_add */
    IGMP_ipif_add(ipif);
#endif

    parms->ihandle = (_rtcs_if_handle)ipif;
    RTCSCMD_complete(parms, RTCS_OK);
} 


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IPIF_bind
*  Parameters     :
*
*     pointer              mhandle     not used
*     _rtcs_if_handle      ihandle     [IN] the IP interface handle
*     RTCS_IF_STRUCT_PTR   if_ptr      not used
*     _ip_address          address     [IN] the local address
*     _ip_address          locmask     not used
*     _ip_address          network     not used
*     _ip_address          netmask     [IN] the network address mask
*
*  Comments       :
*        Starts bind of an IP address and network to a hardware interface.
*
*END*-----------------------------------------------------------------*/

void IPIF_bind
   (
      IPIF_PARM_PTR  parms
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   _ip_address       address, netmask;
   uint32_t           error = RTCS_OK;

   /* Start CR 2303 */
   if (parms->ihandle==NULL) {
      RTCSCMD_complete(parms, RTCSERR_INVALID_PARAMETER);
      return;
   } /* Endif */
   /* End CR 2303 */

   address = parms->address;
   netmask = parms->netmask;

   /*
   ** Make sure the netmask is valid.  We use the fact that
   ** (x & x+1) == 0  <=>  x = 2^n-1.
   */
   if (~netmask & (~netmask + 1)) {
      RTCSCMD_complete(parms, RTCSERR_IP_BIND_MASK);
      return;
   } /* Endif */

   /*
   ** Make sure the address is valid.
   */
   if (((address & ~netmask) == 0)
    || ((address & ~netmask) == ~netmask)) {
      RTCSCMD_complete(parms, RTCSERR_IP_BIND_ADDR);
      return;
   } /* Endif */

   if (parms->probe) {
      // probe IP address here, set error if in use
      if (ARP_request(parms->ihandle, address, address ) != RTCS_OK) {
         error = RTCSERR_SEND_FAILED;
      }
      RTCSCMD_complete(parms, error);
   }
   else
   {
      IPIF_bind_finish (parms);
   }
#else

    RTCSCMD_complete(parms, RTCSERR_IP_IS_DISABLED);

#endif /* RTCSCFG_ENABLE_IP4 */
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IPIF_bind_finish
*  Parameters     :
*
*     pointer              mhandle     not used
*     _rtcs_if_handle      ihandle     [IN] the IP interface handle
*     RTCS_IF_STRUCT_PTR   if_ptr      not used
*     _ip_address          address     [IN] the local address
*     _ip_address          locmask     not used
*     _ip_address          network     not used
*     _ip_address          netmask     [IN] the network address mask
*
*  Comments       :
*        Finishes bind of an IP address and network to a hardware interface.
*
*END*-----------------------------------------------------------------*/

void IPIF_bind_finish
   (
      IPIF_PARM_PTR  parms
   )
{
#if RTCSCFG_ENABLE_IP4   
   uint32_t           error = RTCS_OK;
   
   if (parms->probe)
   {
      if (ARP_is_complete (parms->ihandle, parms->address)) {
         error = RTCSERR_TCP_ADDR_IN_USE;
      }
   }
   if (!error) {
      /* Create the route entry for the directly connected network */
      error = IP_route_add_direct(parms->address, parms->netmask, (IP_IF_PTR)parms->ihandle,
         (IP_IF_PTR)parms->ihandle);
   }
   /* Add the IP and the LOCALHOST interface into the routing table */
   if (!error) {
      error = IP_route_add_direct(parms->address, 0xFFFFFFFF, (IP_IF_PTR)parms->ihandle,
         RTCS_IF_LOCALHOST_PRV);
   } /* Endif */

   if (error) {
      RTCSCMD_complete(parms, error);
      return;
   } /* Endif */

#if RTCSCFG_ENABLE_IGMP && RTCSCFG_ENABLE_IP4
   /* IGMP_ipif_bind failure isn't sufficient reason to abort IPIF_bind */
   IGMP_ipif_bind((IP_IF_PTR)parms->ihandle, parms->address);
#endif

   RTCSCMD_complete(parms, error);

#else

    RTCSCMD_complete(parms, RTCSERR_IP_IS_DISABLED);

#endif /* RTCSCFG_ENABLE_IP4 */
   

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IPIF_bind_ppp
*  Parameters     :
*
*     pointer              mhandle     not used
*     _rtcs_if_handle      ihandle     [IN] the IP interface handle
*     RTCS_IF_STRUCT_PTR   if_ptr      not used
*     _ip_address          address     [IN] the local address
*     _ip_address          locmask     not used
*     _ip_address          network     [IN] the peer address
*     _ip_address          netmask     not used
*
*  Comments       :
*        Binds an IP address and network to a hardware interface.
*
*END*-----------------------------------------------------------------*/

void IPIF_bind_ppp
   (
      IPIF_PARM_PTR  parms
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4
   _ip_address       address, peer;
   uint32_t           error;

   address = parms->address;
   peer    = parms->network;
   
   /* Create the route entry for the directly connected peer */
   error = IP_route_add_virtual(peer, 0xFFFFFFFF, address,
      INADDR_ANY, INADDR_ANY, parms->ihandle, NULL);

   /* Add the IP and the LOCALHOST interface into the routing table */
   if (!error) {
      error = IP_route_add_direct(address, 0xFFFFFFFF, parms->ihandle, RTCS_IF_LOCALHOST_PRV);
   } /* Endif */

   if (error) {
      RTCSCMD_complete(parms, error);
      return;
   } /* Endif */

#if RTCSCFG_ENABLE_IGMP
   /* IGMP_ipif_bind failure isn't sufficient reason to abort IPIF_bind */
   IGMP_ipif_bind((IP_IF_PTR)parms->ihandle, address);
#endif

   RTCSCMD_complete(parms, RTCS_OK);

#else

    RTCSCMD_complete(parms, RTCSERR_IP_IS_DISABLED);

#endif /* RTCSCFG_ENABLE_IP4 */
   

} /* Endbody */

#if RTCSCFG_ENABLE_GATEWAYS
/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IPIF_gate_add
*  Parameters     :
*
*     pointer              mhandle     not used
*     _rtcs_if_handle      ihandle     not used
*     RTCS_IF_STRUCT_PTR   if_ptr      not used
*     _ip_address          address     [IN] the gateway address
*     _ip_address          locmask     [IN] the gateway metric
*     _ip_address          network     [IN] the network address
*     _ip_address          netmask     [IN] the network address mask
*
*  Comments       :
*        Adds a gateway to the routing table.
*
*END*-----------------------------------------------------------------*/

void IPIF_gate_add
   (
      IPIF_PARM_PTR  parms
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   _ip_address          netmask = parms->netmask;
   uint32_t              error;

   /*
   ** Make sure the netmask is valid.  We use the fact that
   ** (x & x+1) == 0  <=>  x = 2^n-1.
   */
   if (~netmask & (~netmask + 1)) {
      RTCSCMD_complete(parms, RTCSERR_IP_BIND_MASK);
      return;
   } /* Endif */

   /* Start CR 1133 */
   error = IP_route_add_indirect(parms->address, netmask, parms->network, RTF_STATIC, parms->locmask);
   /* End CR 1133 */

   RTCSCMD_complete(parms, error);

#else

    RTCSCMD_complete(parms, RTCSERR_IP_IS_DISABLED);

#endif /* RTCSCFG_ENABLE_IP4 */   

} /* Endbody */

/* Start CR 1016 */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IPIF_gate_add_redirect
*  Parameters     :
*
*     pointer              mhandle     not used
*     _rtcs_if_handle      ihandle     not used
*     RTCS_IF_STRUCT_PTR   if_ptr      not used
*     _ip_address          address     [IN] the gateway address
*     _ip_address          locmask     [IN] the gateway metric
*     _ip_address          network     [IN] the network address
*     _ip_address          netmask     [IN] the network address mask
*
*  Comments       :
*        Adds a gateway to the routing table with the REDIRECT flag set.
*
*END*-----------------------------------------------------------------*/

void IPIF_gate_add_redirect
   (
      IPIF_PARM_PTR  parms
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   _ip_address          netmask = parms->netmask;
   uint32_t              error;

   /*
   ** Make sure the netmask is valid.  We use the fact that
   ** (x & x+1) == 0  <=>  x = 2^n-1.
   */
   if (~netmask & (~netmask + 1)) {
      RTCSCMD_complete(parms, RTCSERR_IP_BIND_MASK);
      return;
   } /* Endif */

   /* Start CR 1133 */
   error = IP_route_add_indirect(parms->address, netmask, parms->network, RTF_REDIRECT, parms->locmask);
   /* End CR 1133 */

   RTCSCMD_complete(parms, error);

#else

    RTCSCMD_complete(parms, RTCSERR_IP_IS_DISABLED);

#endif /* RTCSCFG_ENABLE_IP4 */    

} /* Endbody */

/* End CR 1016 */
#endif

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IPIF_remove
*  Parameters     :
*
*     pointer              mhandle     not used
*     _rtcs_if_handle      ihandle     [IN] the IP interface handle
*     RTCS_IF_STRUCT_PTR   if_ptr      not used
*     _ip_address          address     not used
*     _ip_address          locmask     not used
*     _ip_address          network     not used
*     _ip_address          netmask     not used
*
*  Comments       :
*        Unregisters a hardware interface with RTCS.
*
*END*-----------------------------------------------------------------*/
void IPIF_remove(IPIF_PARM_PTR  parms)
{
    IP_IF_PTR           ipif;
    uint32_t            error;

    ipif = (IP_IF_PTR)parms->ihandle;
    error = ipif->DEVICE.CLOSE(ipif);
    
    ip_if_list_del(ipif);
    
    _mem_free(ipif);

    RTCSCMD_complete(parms, error);
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IPIF_unbind
*  Parameters     :
*
*     pointer              mhandle     not used
*     _rtcs_if_handle      ihandle     [IN] the IP interface handle
*     RTCS_IF_STRUCT_PTR   if_ptr      not used
*     _ip_address          address     [IN] the local address
*     _ip_address          locmask     not used
*     _ip_address          network     not used
*     _ip_address          netmask     not used
*
*  Comments       :
*        Unbinds an IP address and network from a hardware interface.
*
*END*-----------------------------------------------------------------*/

void IPIF_unbind
   (
      IPIF_PARM_PTR  parms
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   _ip_address    mask;

   IP_route_remove_direct(parms->address, 0xFFFFFFFF, parms->ihandle);
   
#if RTCSCFG_ENABLE_IGMP 
   if (parms->ihandle) {
      ((IP_IF_PTR)(parms->ihandle))->IGMP_UNBIND((IP_IF_PTR)parms->ihandle,
         parms->address);
   } /* Endif */
#endif
   IP_get_netmask(parms->ihandle, parms->address, &mask);
   
   /* If mask is invalid do not try to remove direct route */
   if (mask != 1)
   {
        IP_route_remove_direct(parms->address, mask, parms->ihandle);
   }
   RTCSCMD_complete(parms, RTCS_OK);

#else

    RTCSCMD_complete(parms, RTCSERR_IP_IS_DISABLED);

#endif /* RTCSCFG_ENABLE_IP4 */
   

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IPIF_unbind_ppp
*  Parameters     :
*
*     pointer              mhandle     not used
*     _rtcs_if_handle      ihandle     [IN] the IP interface handle
*     RTCS_IF_STRUCT_PTR   if_ptr      not used
*     _ip_address          address     [IN] the local address
*     _ip_address          locmask     not used
*     _ip_address          network     [IN] the peer address
*     _ip_address          netmask     not used
*
*  Comments       :
*        Unbinds an IP address and network from a hardware interface.
*
*END*-----------------------------------------------------------------*/

void IPIF_unbind_ppp
   (
      IPIF_PARM_PTR  parms
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   IP_route_remove_virtual(parms->network, 0xFFFFFFFF, parms->address,
      INADDR_ANY, INADDR_ANY, parms->ihandle);
      
#if RTCSCFG_ENABLE_IGMP
   if (parms->ihandle) {
      ((IP_IF_PTR)(parms->ihandle))->IGMP_UNBIND((IP_IF_PTR)parms->ihandle,
         parms->address);
   } /* Endif */
#endif

   IP_route_remove_direct(parms->address, 0xFFFFFFFF, parms->ihandle);

   RTCSCMD_complete(parms, RTCS_OK);

#else

    RTCSCMD_complete(parms, RTCSERR_IP_IS_DISABLED);

#endif /* RTCSCFG_ENABLE_IP4 */
   

} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IPIF_gate_remove
*  Parameters     :
*
*     pointer              mhandle     not used
*     _rtcs_if_handle      ihandle     not used
*     RTCS_IF_STRUCT_PTR   if_ptr      not used
*     _ip_address          address     [IN] the gateway address
*     _ip_address          locmask     not used
*     _ip_address          network     [IN] the network address
*     _ip_address          netmask     [IN] the network address mask
*
*  Comments       :
*        Removes a gateway from the routing table.
*
*END*-----------------------------------------------------------------*/

void IPIF_gate_remove
   (
      IPIF_PARM_PTR  parms
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

    /* Start CR 1133 */
    IP_route_remove_indirect(parms->address, parms->netmask, parms->network, RTF_STATIC, parms->locmask);
    /* End CR 1133 */

    RTCSCMD_complete(parms, RTCS_OK);

#else

    RTCSCMD_complete(parms, RTCSERR_IP_IS_DISABLED);

#endif /* RTCSCFG_ENABLE_IP4 */
   

} /* Endbody */

/************************************************************************
* NAME: ip_if_add_dns_addr
*
* DESCRIPTION: This function adds address to the DNS Server list.
*************************************************************************/
void ip_if_add_dns_addr(IPIF_PARM_PTR  parms)
{ 
    IP_IF_PTR           if_ptr = (IP_IF_PTR)parms->ihandle;
    _ip_address         dns_addr = parms->address;
    IP_DNS_ENTRY_PTR    dns_addr_entry = NULL;
    int                 i;
    uint32_t            result;

    /* Check input parameters. */
    if ((if_ptr == NULL) || (dns_addr == INADDR_ANY))
    {
        result = RTCSERR_INVALID_PARAMETER;
        goto COMPLETE;
    } 
   
    /* Find free address entry. */
    for(i = 0; i < RTCSCFG_IP_IF_DNS_MAX; i++)
    {
        if( (if_ptr->dns_address[i].dns_addr == INADDR_ANY) /* Empty entry. */
            || (if_ptr->dns_address[i].dns_addr == dns_addr)  /* Already exist.*/ )
        {
            dns_addr_entry = &if_ptr->dns_address[i];
            break; /* Found free entry.*/
        }
    }
    
    if(dns_addr_entry == NULL)
    {   /* No free DNS Server List entry.*/ 
        uint32_t    current_time = RTCS_time_get_sec();

        dns_addr_entry = &if_ptr->dns_address[0];        
        
        /* Find the oldest one.*/
        for(i = 1; i < RTCSCFG_IP_IF_DNS_MAX; i++)
        {
            if( RTCS_timer_get_interval(dns_addr_entry->creation_time, current_time) 
                < RTCS_timer_get_interval(if_ptr->dns_address[i].creation_time, current_time) )
            {
                dns_addr_entry = &if_ptr->dns_address[i];
            }
        }
    }

    /* Add DNS Server address.*/
    dns_addr_entry->creation_time = RTCS_time_get_sec();
    dns_addr_entry->dns_addr = dns_addr;

    result = RTCS_OK;

COMPLETE:

    RTCSCMD_complete(parms, result); 
}   

/************************************************************************
* NAME: ip_if_del_dns_addr
*
* DESCRIPTION: This function deletes address from the DNS Server list.
*************************************************************************/
void ip_if_del_dns_addr(IPIF_PARM_PTR  parms)
{ 
    IP_IF_PTR           if_ptr = (IP_IF_PTR)parms->ihandle;
    _ip_address         dns_addr = parms->address;
    int                 i;
    uint32_t            result;

    /* Check input parameters. */
    if ((if_ptr == NULL) || (dns_addr == INADDR_ANY))
    {
        result = RTCSERR_INVALID_PARAMETER;
        goto COMPLETE;
    } 
   
    /* Find  address entry. */
    for(i = 0; i < RTCSCFG_IP_IF_DNS_MAX; i++)
    {
        if( dns_addr == if_ptr->dns_address[i].dns_addr )
        {
            if_ptr->dns_address[i].dns_addr = INADDR_ANY;  /* Free element.*/
            break;
        }
    }

    result = RTCS_OK;

COMPLETE:

    RTCSCMD_complete(parms, result); 
}   

/************************************************************************
* NAME: ip_if_list_add
*
* DESCRIPTION: This function adds interface into the list.
*************************************************************************/
static void ip_if_list_add( IP_IF_PTR if_ptr )
{
    if_ptr->next_if = IP_IF_LIST;

    if(if_ptr->next_if != NULL)
        if_ptr->next_if->prev_if = if_ptr;

    if_ptr->prev_if = NULL;
    
    IP_IF_LIST = if_ptr;
}

/************************************************************************
* NAME: ip_if_list_del
*
* DESCRIPTION: This function removes interafce from the list. 
*************************************************************************/
static void ip_if_list_del( IP_IF_PTR if_ptr )
{
    if(if_ptr->prev_if == NULL)
        IP_IF_LIST = if_ptr->next_if;
    else
        if_ptr->prev_if->next_if = if_ptr->next_if;

    if(if_ptr->next_if != NULL)
        if_ptr->next_if->prev_if = if_ptr->prev_if;
}

/************************************************************************
* NAME: ip_if_list_get
*
* DESCRIPTION: This function returns pointer to n-th interafce according 
*              its index (from zero). 
*              It returns NULL if id-th interface is not available.
*************************************************************************/
IP_IF_PTR ip_if_list_get( uint32_t n )
{
    IP_IF_PTR           result = NULL;
    IP_IF_PTR           current;
   
    for (current = IP_IF_LIST; current; current = current->next_if, n--)
    {
        if(n == 0)
        {
            result = current;
            break;     
        }
    }
    
    return result;
}

/************************************************************************
* NAME: ip_if_is_joined
*
* DESCRIPTION: Check if the interface is joined to the multicast group. 
*************************************************************************/
bool ip_if_is_joined(IP_IF_PTR if_ptr, _ip_address  group_addr )
{
    bool            result = false;
#if RTCSCFG_ENABLE_IGMP && RTCSCFG_ENABLE_IP4
    MC_MEMBER_PTR   phead;

    /* Look for a given group through a list of joined group */
    for (phead = if_ptr->IGMP_MEMBER; phead ; phead = phead->NEXT)
    {
        if (phead->IGRP.imr_multiaddr.s_addr == group_addr)
        {
            /* the joined group is found */
            result = true;
            break;
        }
    } 
#endif

   return result;
}





