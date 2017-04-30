/**HEADER********************************************************************
* 
* Copyright (c) 2013 Freescale Semiconductor;
* All Rights Reserved
*
*************************************************************************** 
*
* THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR 
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  
* IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
* THE POSSIBILITY OF SUCH DAMAGE.
*
**************************************************************************
*
* Comments:
*
*   This file contains the interface functions to the
*   packet driver interface.
*
*END************************************************************************/

#include <rtcs.h>
#include "rtcs_prv.h"
#include "tcpip.h"
#include "ip_prv.h"


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_if_add
* Returned Value  : RTCS_OK or error code
* Comments        :
*        Register a hardware interface with RTCS.
*
*END*-----------------------------------------------------------------*/

uint32_t RTCS_if_add
   (
      void                   *mhandle,
         /* [IN] the packet driver handle */
      RTCS_IF_STRUCT_PTR      if_ptr,
         /* [IN] call table for the interface */
      _rtcs_if_handle    *handle
         /* [OUT] the RTCS interface state structure */
   )
{ 
   IPIF_PARM    parms;
   uint32_t     error;

   if (mhandle==NULL) 
   {
      return RTCSERR_INVALID_PARAMETER; 
   }
   
    _mem_zero(&parms, sizeof(parms));
    parms.mhandle = mhandle;
    parms.if_ptr  = if_ptr;

    error = RTCSCMD_issue(parms, IPIF_add);

    if (!error)
    {
        *handle = parms.ihandle;
    } 
    return error;
} 


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_if_bind
* Returned Value  : RTCS_OK or error code
* Comments        :
*        Bind an IP address to a registered hardware interface.
*
*END*-----------------------------------------------------------------*/

uint32_t RTCS_if_bind
   (
      _rtcs_if_handle   handle,
         /* [IN] the RTCS interface state structure */
      _ip_address       address,
         /* [IN] the IP address for the interface */
      _ip_address       netmask
         /* [IN] the IP (sub)network mask for the interface */
   )
{ /* Body */
   IPIF_PARM   parms;

   if (handle==NULL) 
   {
      return RTCSERR_INVALID_PARAMETER; 
   }
   
   parms.ihandle = handle;
   parms.address = address;
   parms.locmask = 0xFFFFFFFFL;
   parms.netmask = netmask;
   parms.probe   = FALSE;

   return RTCSCMD_issue(parms, IPIF_bind);

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_if_probe_and_bind
* Returned Value  : RTCS_OK or error code
* Comments        :
*        Bind an IP address to a registered hardware interface.
*
*END*-----------------------------------------------------------------*/

uint32_t RTCS_if_probe_and_bind
   (
      _rtcs_if_handle   handle,
         /* [IN] the RTCS interface state structure */
      _ip_address       address,
         /* [IN] the IP address for the interface */
      _ip_address       netmask
         /* [IN] the IP (sub)network mask for the interface */
   )
{ /* Body */
   IPIF_PARM   parms;
   uint32_t     error;

   if (handle==NULL) 
   {
      return RTCSERR_INVALID_PARAMETER; 
   }

   parms.ihandle = handle;
   parms.address = address;
   parms.locmask = 0xFFFFFFFFL;
   parms.netmask = netmask;
   parms.probe   = TRUE;
   error = RTCSCMD_issue(parms, IPIF_bind);
   
   if (RTCS_OK == error)
   {
       _time_delay (500);
       
       parms.ihandle = handle;
       parms.address = address;
       parms.locmask = 0xFFFFFFFFL;
       parms.netmask = netmask;
       parms.probe   = TRUE;
       error = RTCSCMD_issue(parms, IPIF_bind_finish);
   }

   return error;

} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_gate_add
* Returned Value  : RTCS_OK or error code
* Comments        :
*        Register a gateway with RTCS.
*
*END*-----------------------------------------------------------------*/

uint32_t RTCS_gate_add
   (
      _ip_address       gateway,
         /* [IN] the IP address of the gateway */
      _ip_address       network,
         /* [IN] the IP (sub)network to route */
      _ip_address       netmask
         /* [IN] the IP (sub)network mask to route */
   )
{ /* Body */
#if RTCSCFG_ENABLE_GATEWAYS
   IPIF_PARM   parms;

   parms.address = gateway;
   parms.network = network;
   parms.netmask = netmask;
   /* Start CR 1133 */
   parms.locmask = 0;
   /* End CR 1133 */

   return RTCSCMD_issue(parms, IPIF_gate_add);
#else
   return RTCSERR_FEATURE_NOT_ENABLED;
#endif
} /* Endbody */



/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_gate_add_metric
* Returned Value  : RTCS_OK or error code
* Comments        :
*        Register a gateway with RTCS.
*
*END*-----------------------------------------------------------------*/
uint32_t RTCS_gate_add_metric
   (
      _ip_address       gateway,
         /* [IN] the IP address of the gateway */
      _ip_address       network,
         /* [IN] the IP (sub)network to route */
      _ip_address       netmask,
         /* [IN] the IP (sub)network mask to route */
      uint16_t           metric
         /* [IN] the metric of the gateway */
   )
{ 
#if RTCSCFG_ENABLE_GATEWAYS
   IPIF_PARM   parms;

   parms.address = gateway;
   parms.network = network;
   parms.netmask = netmask;
   parms.locmask = metric;

   return RTCSCMD_issue(parms, IPIF_gate_add);
#else
   return RTCSERR_FEATURE_NOT_ENABLED;
#endif

} 

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_if_remove
* Returned Value  : RTCS_OK or error code
* Comments        :
*        Unregister a hardware interface with RTCS.
*
*END*-----------------------------------------------------------------*/

uint32_t RTCS_if_remove
   (
      _rtcs_if_handle   handle
         /* [IN] the RTCS interface state structure */
   )
{ /* Body */
   IPIF_PARM   parms;

   parms.ihandle = handle;

   return RTCSCMD_issue(parms, IPIF_remove);

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_if_unbind
* Returned Value  : RTCS_OK or error code
* Comments        :
*        Unbind an IP address from a registered hardware interface.
*
*END*-----------------------------------------------------------------*/

uint32_t RTCS_if_unbind
   (
      _rtcs_if_handle   handle,
         /* [IN] the RTCS interface state structure */
      _ip_address       address
         /* [IN] the IP address for the interface */
   )
{ /* Body */
   IPIF_PARM   parms;

   parms.ihandle = handle;
   parms.address = address;

   return RTCSCMD_issue(parms, IPIF_unbind);

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_gate_remove
* Returned Value  : RTCS_OK or error code
* Comments        :
*        Register a gateway with RTCS.
*
*END*-----------------------------------------------------------------*/

uint32_t RTCS_gate_remove
   (
      _ip_address       gateway,
         /* [IN] the IP address of the gateway */
      _ip_address       network,
         /* [IN] the IP (sub)network to route */
      _ip_address       netmask
         /* [IN] the IP (sub)network mask to route */
   )
{ /* Body */
#if RTCSCFG_ENABLE_GATEWAYS
   IPIF_PARM   parms;

   parms.address = gateway;
   parms.network = network;
   parms.netmask = netmask;
   /* Start CR 1133 */
   parms.locmask = 0;
   /* End CR 1133 */

   return RTCSCMD_issue(parms, IPIF_gate_remove);
#else
   return RTCSERR_FEATURE_NOT_ENABLED;
#endif
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_gate_remove_metric
* Returned Value  : RTCS_OK or error code
* Comments        :
*        Removes a gateway from RTCS if the metric matches the route.
*
*END*-----------------------------------------------------------------*/

uint32_t RTCS_gate_remove_metric
   (
      _ip_address       gateway,
         /* [IN] the IP address of the gateway */
      _ip_address       network,
         /* [IN] the IP (sub)network to route */
      _ip_address       netmask,
         /* [IN] the IP (sub)network mask to route */
      uint16_t           metric
         /* [IN] the metric of the gateway */
   )
{ /* Body */
#if RTCSCFG_ENABLE_GATEWAYS
   IPIF_PARM   parms;

   parms.address = gateway;
   parms.network = network;
   parms.netmask = netmask;
   parms.locmask = metric;

   return RTCSCMD_issue(parms, IPIF_gate_remove);
#else
   return RTCSERR_FEATURE_NOT_ENABLED;
#endif
} 

/************************************************************************
* NAME: RTCS_if_get_dns_addr
*
* DESCRIPTION: This function returns an address from the DNSv4 Server list.
*************************************************************************/
bool RTCS_if_get_dns_addr(_rtcs_if_handle ihandle, uint32_t n, _ip_address *dns_addr)
{
    IP_IF_PTR   if_ptr = (IP_IF_PTR)ihandle;
    bool        result = FALSE;
    int         i;

    if(if_ptr && dns_addr)
    {
        /* Get address from DNS Server list (Manual, DHCP).*/
        for(i=0; i<RTCSCFG_IP_IF_DNS_MAX; i++)
        {
            /* Skip NOT_USED addresses. */
            if(if_ptr->dns_address[i].dns_addr != INADDR_ANY)
            {    
                if(n == 0)
                {
                    *dns_addr = if_ptr->dns_address[i].dns_addr;
                    result = TRUE;
                    break;     
                }
                n--;
            }    
        } 
    }
    
    return result;
}    

/************************************************************************
* NAME: RTCS_if_add_dns_addr
*
* DESCRIPTION: This function adds address to the DNS Server list.
*************************************************************************/
uint32_t RTCS_if_add_dns_addr(_rtcs_if_handle ihandle, _ip_address dns_addr)
{
   IPIF_PARM   parms;

   if ((ihandle==NULL) || (dns_addr == INADDR_ANY) ) 
   {
      return RTCSERR_INVALID_PARAMETER; 
   }
   
   parms.ihandle = ihandle;
   parms.address = dns_addr;
   
   return RTCSCMD_issue(parms, ip_if_add_dns_addr);
}    

/************************************************************************
* NAME: RTCS_if_get_handle
*
* DESCRIPTION: This function returns handle of n-th interface (from zero). 
*              It returns 0 if n-th interface is not available.
*************************************************************************/
_rtcs_if_handle RTCS_if_get_handle(uint32_t n)
{
    return (_rtcs_if_handle)ip_if_list_get(n);
}   

/************************************************************************
* NAME: RTCS_if_del_dns_addr
*
* DESCRIPTION: This function deletes address from the DNS Server list.
*************************************************************************/
uint32_t RTCS_if_del_dns_addr(_rtcs_if_handle ihandle, _ip_address dns_addr)
{
   IPIF_PARM   parms;

   if ((ihandle==NULL) || (dns_addr==INADDR_ANY) ) 
   {
      return RTCSERR_INVALID_PARAMETER; 
   }
   
   parms.ihandle = ihandle;
   parms.address = dns_addr;
   
   return RTCSCMD_issue(parms, ip_if_del_dns_addr);
}  

/************************************************************************
* NAME: RTCS_if_get_link_status
*
* RETURN        : TRUE if link active, FALSE otherwise
* DESCRIPTION   : Get actual link status.
*************************************************************************/
bool RTCS_if_get_link_status(_rtcs_if_handle ihandle)
{
    IP_IF_PTR   if_ptr = (IP_IF_PTR)ihandle;
    bool        result;

    if(if_ptr != NULL) 
    {
        if(if_ptr->DEVICE.LINK_STATUS == NULL)
        {
            result = TRUE; /*Always connected. For example, loopback interface. */
        }
        else
        {
            result = if_ptr->DEVICE.LINK_STATUS(if_ptr);
        }
    }
    else
    {
        result = FALSE;
    }
    
    return result;
}

/************************************************************************
* NAME: RTCS_if_get_mtu
*
* RETURN        : Maximum Transmission Unit 
* DESCRIPTION   : Get Maximum Transmission Unit of interface.
*************************************************************************/
uint32_t RTCS_if_get_mtu(_rtcs_if_handle ihandle)
{
    IP_IF_PTR   if_ptr = (IP_IF_PTR)ihandle;
    uint32_t    mtu;

    if(if_ptr != NULL) 
    {
        mtu = if_ptr->MTU;
    }
    else
    {
        mtu = 0;
    }
    
    return mtu;
}

/************************************************************************
* NAME: RTCS_if_get_addr
*
* RETURN        : IPv4 address bound to interface. 
* DESCRIPTION   : Get Pv4 address bound to interface.
*************************************************************************/
_ip_address RTCS_if_get_addr(_rtcs_if_handle ihandle)
{
    _ip_address     addr = 0;
  
#if RTCSCFG_ENABLE_IP4
    IP_IF_PTR       if_ptr = (IP_IF_PTR)ihandle;

    if(if_ptr != NULL) 
    {
        addr =  IP_get_ipif_addr(if_ptr);
    }
#endif
    
    return addr;
}

/************************************************************************
* RTCS6_if_xxx
*************************************************************************/


/************************************************************************
* NAME: RTCS6_if_get_addr
*
* DESCRIPTION: This function is used to retrieve all IP addresses registerred 
*              with the given interface.
*              Returns RTCS_OK if successful and data structure filled. FALSE in case of error.
*              It returns RTCS_ERROR if n-th address is not available.
*************************************************************************/
uint32_t RTCS6_if_get_addr(_rtcs_if_handle ihandle, uint32_t n, RTCS6_IF_ADDR_INFO_PTR addr_info)
{
#if RTCSCFG_ENABLE_IP6
    IP6_IF_ADDR_INFO        ip6_addr_info;
    uint32_t                result = (uint32_t)RTCS_ERROR;

    if(ip6_if_get_addr((IP_IF_PTR)ihandle, n, &ip6_addr_info) == TRUE)
    {
        addr_info->ip_addr_type = ip6_addr_info.ip_addr_type;
        addr_info->ip_addr_state = ip6_addr_info.ip_addr_state;
        _mem_copy(ip6_addr_info.ip_addr.s6_addr, addr_info->ip_addr.s6_addr, sizeof(addr_info->ip_addr));
        result = RTCS_OK;
    }
    else
        result = (uint32_t)RTCS_ERROR;
    
    return result;
#else
    return RTCSERR_IP_IS_DISABLED;
#endif
}

/************************************************************************
* NAME: RTCS6_if_bind_addr
* RETURNS : RTCS_OK or error code
* DESCRIPTION: Bind an IPv6 address to a registered hardware interface.
*************************************************************************/
uint32_t RTCS6_if_bind_addr( _rtcs_if_handle    handle              /* [IN] the RTCS interface state structure */,
                            in6_addr            *ip_addr            /* [IN] the IP address for the interface */,
                            rtcs6_if_addr_type  ip_addr_type        /* [IN] Type.*/,
                            uint32_t            ip_addr_lifetime    /* [IN] Valid lifetime, in seconds*/)
{
#if RTCSCFG_ENABLE_IP6
    IP6_IF_PARM   parms;

    if ((handle==NULL) || (ip_addr==NULL) ) 
    {
        return RTCSERR_INVALID_PARAMETER; 
    }
   
    parms.ihandle = handle;
    parms.ip_addr = ip_addr;
    parms.ip_addr_type = ip_addr_type;
    parms.ip_addr_lifetime = ip_addr_lifetime;
   
    return RTCSCMD_issue(parms, ip6_if_bind_addr_cmd);
#else
    return RTCSERR_IP_IS_DISABLED;
#endif
}

/************************************************************************
* NAME: RTCS6_if_unbind_addr
* RETURNS : RTCS_OK or error code
* DESCRIPTION: Unbind an IPv6 address from a registered hardware interface.
*************************************************************************/
uint32_t RTCS6_if_unbind_addr ( _rtcs_if_handle handle   /* [IN] the RTCS interface state structure */,
                               in6_addr *ip_addr         /* [IN] the IP address for the interface */)
{
#if RTCSCFG_ENABLE_IP6
    IP6_IF_PARM   parms;

    parms.ihandle = handle;
    parms.ip_addr = ip_addr;

    return RTCSCMD_issue(parms, ip6_if_unbind_addr_cmd);
#else
    return RTCSERR_IP_IS_DISABLED;
#endif
}

/************************************************************************
* NAME: RTCS6_if_get_dns_addr
*
* DESCRIPTION: This function returns an address from the DNS Server list.
*************************************************************************/
bool RTCS6_if_get_dns_addr(_rtcs_if_handle ihandle, uint32_t n, in6_addr *dns_addr)
{
    bool        result = FALSE;

#if RTCSCFG_ENABLE_IP6
    IP_IF_PTR   if_ptr = (IP_IF_PTR)ihandle;
    int         i;

    if(if_ptr && dns_addr)
    {
        /* Get address from DNS Server list (Manual, DHCP).*/
        for(i=0; i<RTCSCFG_IP6_IF_DNS_MAX; i++)
        {
            /* Skip NOT_USED addresses. */
            if(!IN6_ARE_ADDR_EQUAL(&if_ptr->IP6_IF.dns_address[i].dns_addr, &in6addr_any))
            {    
                if(n == 0)
                {
                    IN6_ADDR_COPY(&if_ptr->IP6_IF.dns_address[i].dns_addr, dns_addr);
                    result = TRUE;
                    break;     
                }
                n--;
            }    
        } 

    #if RTCSCFG_ND6_RDNSS
        if(result == FALSE)
        {
            result =  nd6_rdnss_get_addr(if_ptr, n, dns_addr);
        }
    #endif
    }
#endif  /* RTCSCFG_ENABLE_IP6 */   

    return result;
}    

/************************************************************************
* NAME: RTCS6_if_add_dns_addr
*
* DESCRIPTION: This function adds address to the DNS Server list.
*************************************************************************/
uint32_t RTCS6_if_add_dns_addr(_rtcs_if_handle ihandle, in6_addr *dns_addr)
{
#if RTCSCFG_ENABLE_IP6 
    IP6_IF_PARM   parms;

    if ((ihandle==NULL) || (dns_addr==NULL) ) 
    {
        return RTCSERR_INVALID_PARAMETER; 
    }
   
    parms.ihandle = ihandle;
    parms.ip_addr = dns_addr;
   
    return RTCSCMD_issue(parms, ip6_if_add_dns_addr);
#else
    return RTCSERR_IP_IS_DISABLED;
#endif
}    

/************************************************************************
* NAME: RTCS6_if_del_dns_addr
*
* DESCRIPTION: This function deletes address from the DNS Server list.
*************************************************************************/
uint32_t RTCS6_if_del_dns_addr(_rtcs_if_handle ihandle, in6_addr *dns_addr)
{
#if RTCSCFG_ENABLE_IP6
   IP6_IF_PARM   parms;

   if ((ihandle==NULL) || (dns_addr==NULL) ) 
   {
      return RTCSERR_INVALID_PARAMETER; 
   }
   
   parms.ihandle = ihandle;
   parms.ip_addr = dns_addr;
   
   return RTCSCMD_issue(parms, ip6_if_del_dns_addr);
#else
    return RTCSERR_IP_IS_DISABLED;
#endif
}  

/************************************************************************
* NAME: RTCS6_if_get_scope_id
*
* DESCRIPTION: Gets Scope ID assigned to the interface.
*************************************************************************/
uint32_t RTCS6_if_get_scope_id(_rtcs_if_handle ihandle)
{
    uint32_t    result = 0;

#if RTCSCFG_ENABLE_IP6   
    IP_IF_PTR   if_ptr =(IP_IF_PTR)ihandle;

    if(if_ptr)
    {
        result = if_ptr->IP6_IF.scope_id;
  	}
#endif    

    return result;
}

/************************************************************************
* NAME: RTCS6_if_get_neighbor_cache_entry
*
* DESCRIPTION: This function returns an entry from the neighbor cache.
*************************************************************************/
bool RTCS6_if_get_neighbor_cache_entry(_rtcs_if_handle ihandle, uint32_t n, RTCS6_IF_NEIGHBOR_CACHE_ENTRY_PTR neighbor_cache_entry)
{
    bool        result = FALSE;

#if RTCSCFG_ENABLE_IP6
    IP_IF_PTR       if_ptr = (IP_IF_PTR)ihandle;
    int             i;
    ND6_CFG_PTR     nd6_cfg;

    if(if_ptr && neighbor_cache_entry)
    {
        nd6_cfg = if_ptr->IP6_IF.ND6;

        if(nd6_cfg)
        {
            for(i=0; i<ND6_NEIGHBOR_CACHE_SIZE; i++)
            {
                if(nd6_cfg->neighbor_cache[i].state != ND6_NEIGHBOR_STATE_NOTUSED)
                {    
                    if(n == 0)
                    {
                        ND6_NEIGHBOR_ENTRY_PTR  entry = &nd6_cfg->neighbor_cache[i];

                        IN6_ADDR_COPY(&entry->ip_addr, &neighbor_cache_entry->ip_addr); /* Neighbor’s on-link unicast IP address. */
                        LL_ADDR_COPY(entry->ll_addr, neighbor_cache_entry->ll_addr, if_ptr->DEV_ADDRLEN); /* Its link-layer address. Actual size is defiined by ll_addr_size.*/
                        neighbor_cache_entry->ll_addr_size = if_ptr->DEV_ADDRLEN;   /* Size of link-layer address.*/
                        neighbor_cache_entry->is_router = (entry->is_router == 1);
                        result = TRUE;
                        break;     
                    }
                    n--;
                }    
            }
        }

    }
#endif  /* RTCSCFG_ENABLE_IP6 */   

    return result;
}   

/************************************************************************
* NAME: RTCS6_if_get_prefix_list_entry
*
* DESCRIPTION: This function returns an entry from the prefix list.
*************************************************************************/
bool RTCS6_if_get_prefix_list_entry(_rtcs_if_handle ihandle, uint32_t n, RTCS6_IF_PREFIX_LIST_ENTRY_PTR prefix_list_entry)
{
    bool        result = FALSE;

#if RTCSCFG_ENABLE_IP6
    IP_IF_PTR       if_ptr = (IP_IF_PTR)ihandle;
    int             i;
    ND6_CFG_PTR     nd6_cfg;

    if(if_ptr && prefix_list_entry)
    {
        nd6_cfg = if_ptr->IP6_IF.ND6;

        if(nd6_cfg)
        {
            for(i=0; i<ND6_PREFIX_LIST_SIZE; i++)
            {
                if(nd6_cfg->prefix_list[i].state != ND6_PREFIX_STATE_NOTUSED)
                {    
                    if(n == 0)
                    {
                        ND6_PREFIX_ENTRY_PTR  entry = &nd6_cfg->prefix_list[i];

                        IN6_ADDR_COPY(&entry->prefix, &prefix_list_entry->prefix); /* Prefix of an IP address. */
                        prefix_list_entry->prefix_length = entry->prefix_length;    /* Prefix length (in bits).*/
                        result = TRUE;
                        break;     
                    }
                    n--;
                }    
            }
        }
    }
#endif  /* RTCSCFG_ENABLE_IP6 */   

    return result;
}   

/************************************************************************
* NAME: RTCS6_if_is_disabled
*
* DESCRIPTION: This function detects if IPv6 is disabled for the interface.
*************************************************************************/
bool RTCS6_if_is_disabled(_rtcs_if_handle ihandle)
{
    bool        result = TRUE;

#if RTCSCFG_ENABLE_IP6
    IP_IF_PTR       if_ptr = (IP_IF_PTR)ihandle;
    if(if_ptr)
    {
        result = if_ptr->IP6_IF.ip6_disabled;
    }
#endif  /* RTCSCFG_ENABLE_IP6 */   

    return result;
} 
