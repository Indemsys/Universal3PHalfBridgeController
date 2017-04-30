/*HEADER**********************************************************************
*
* Copyright 2011 Freescale Semiconductor, Inc.
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
*   High level IPv6 config functions and task.
*
*
*END************************************************************************/

#include <rtcs.h>
#include "rtcs_prv.h"
#include "tcpip.h"
#include "ip_prv.h"
#include "ipcfg.h"
#include "ipcfg_prv.h"

#ifdef BSP_ENET_DEVICE_COUNT
#if  (BSP_ENET_DEVICE_COUNT > 0) 

/* Hidden global data. */
extern IPCFG_CONTEXT    ipcfg_data[IPCFG_DEVICE_COUNT];

/************************************************************************
* NAME: ipcfg6_get_addr
* RETURNS : RTCS_OK if successful. 
*           It returns FALSE if n-th address is not available.
* DESCRIPTION: Gets address information which corresponds to the n-th address.
*************************************************************************/
uint32_t ipcfg6_get_addr(uint32_t device, uint32_t n /*=>0*/, IPCFG6_GET_ADDR_DATA_PTR data /*out*/)
{
#if RTCSCFG_ENABLE_IP6

    uint32_t result = (uint32_t)RTCS_ERROR;
    
    if (device < IPCFG_DEVICE_COUNT)
    {
        if (ipcfg_data[device].actual_state != IPCFG_STATE_INIT) 
        {
            RTCS6_IF_ADDR_INFO addr_info;
            
            /* Get address inormation.*/
            if((result = RTCS6_if_get_addr(ipcfg_data[device].ihandle, n, &addr_info)) == RTCS_OK)
            {
                data->ip_addr_type = addr_info.ip_addr_type;
                data->ip_addr_state = addr_info.ip_addr_state;
                _mem_copy(addr_info.ip_addr.s6_addr, data->ip_addr.s6_addr, sizeof(data->ip_addr));
            }
        }
    }
    return result;
    
#else

    return RTCSERR_IP_IS_DISABLED;    
    
#endif /* RTCSCFG_ENABLE_IP6 */
}

/************************************************************************
* NAME: ipcfg6_get_scope_id
* RETURNS : Scope id of device.
*************************************************************************/
uint32_t ipcfg6_get_scope_id (uint32_t device /* in */)
{
#if RTCSCFG_ENABLE_IP6
   
    if (device < IPCFG_DEVICE_COUNT)
    {
        if (ipcfg_data[device].actual_state != IPCFG_STATE_INIT) 
        {
            /* Get scope_id inormation.*/
            return RTCS6_if_get_scope_id(ipcfg_data[device].ihandle);
        }
    }
    return 0;
    
#else

    return 0;    
    
#endif /* RTCSCFG_ENABLE_IP6 */
}

/************************************************************************
* NAME: ipcfg6_unbind_addr
* RETURNS : ipcfg error code
* DESCRIPTION: Releases an IP address associated with given device.
*************************************************************************/
uint32_t ipcfg6_unbind_addr ( uint32_t device, IPCFG6_UNBIND_ADDR_DATA_PTR  ip_data)
{
#if RTCSCFG_ENABLE_IP6
 
    uint32_t result;

    if (device >= IPCFG_DEVICE_COUNT) return RTCSERR_IPCFG_DEVICE_NUMBER;

    if (ipcfg_data[device].actual_state == IPCFG_STATE_INIT) 
        return RTCSERR_IPCFG_INIT;
    
    if (! _lwsem_poll (&(ipcfg_data[device].control_semaphore))) 
        return RTCSERR_IPCFG_BUSY;
    
    /* Unbind.*/
    result = RTCS6_if_unbind_addr (ipcfg_data[device].ihandle, &ip_data->ip_addr);

    if (result == IPCFG_OK)
    {
        ipcfg_data[device].desired_ip_data = ipcfg_data[device].actual_ip_data;
        ipcfg_data[device].desired_state = ipcfg_data[device].actual_state;
    }

    _lwsem_post (&(ipcfg_data[device].control_semaphore));

    return result; 
     
#else

    return RTCSERR_IP_IS_DISABLED;

#endif /* RTCSCFG_ENABLE_IP6 */
    
}

/************************************************************************
* NAME: ipcfg6_bind_addr
* RETURNS : ipcfg error code
* DESCRIPTION: Tries to bind given ip for given device.
*************************************************************************/
uint32_t ipcfg6_bind_addr ( uint32_t device, IPCFG6_BIND_ADDR_DATA_PTR   ip_data)
{
#if RTCSCFG_ENABLE_IP6

    uint32_t result;

    if (device >= IPCFG_DEVICE_COUNT) 
        return RTCSERR_IPCFG_DEVICE_NUMBER;

    if (ipcfg_data[device].actual_state == IPCFG_STATE_INIT) 
        return RTCSERR_IPCFG_INIT;

    if (! _lwsem_poll (&(ipcfg_data[device].control_semaphore))) 
        return RTCSERR_IPCFG_BUSY;
    
    if (! _lwsem_poll (&(ipcfg_data[device].request_semaphore))) return RTCSERR_IPCFG_BUSY;

    result = RTCSERR_IPCFG_BIND;
    
    /* Bind.*/
    if (RTCS6_if_bind_addr (ipcfg_data[device].ihandle, &ip_data->ip_addr, ip_data->ip_addr_type, IP6_ADDR_LIFETIME_INFINITE) == RTCS_OK)
    {
        ipcfg_data[device].desired_state = IPCFG_STATE_STATIC_IP;
        ipcfg_data[device].actual_state = ipcfg_data[device].desired_state;
        result = IPCFG_OK;
    }

    _lwsem_post (&(ipcfg_data[device].request_semaphore));    
    

    _lwsem_post (&(ipcfg_data[device].control_semaphore));
    
    return result;
#else

    return RTCSERR_IP_IS_DISABLED;

#endif /* RTCSCFG_ENABLE_IP6 */    
}

/************************************************************************
* NAME: ipcfg6_get_dns_ip
* RETURN: TRUE if successful, FALSE otherwise
* DESCRIPTION: Returns N-th DNS IPv6 address from DNS list for given N.
*************************************************************************/
bool ipcfg6_get_dns_ip(uint32_t device, uint32_t n, in6_addr *dns_addr)
{
    uint32_t result = FALSE;

#if RTCSCFG_IPCFG_ENABLE_DNS && RTCSCFG_ENABLE_IP6
    if (device < IPCFG_DEVICE_COUNT)
    {
        if (ipcfg_data[device].actual_state != IPCFG_STATE_INIT) 
        {
            result = RTCS6_if_get_dns_addr(ipcfg_data[device].ihandle, n, dns_addr);
        }
    }
#endif
    return result;
}

/************************************************************************
* NAME: ipcfg6_add_dns_ip
* RETURN: TRUE if successful, FALSE otherwise
* DESCRIPTION: Adds/updates DNS ip address info inside DNS list.
*************************************************************************/
bool ipcfg6_add_dns_ip(uint32_t device, in6_addr *dns_addr)
{
    bool result = FALSE;

#if RTCSCFG_IPCFG_ENABLE_DNS && RTCSCFG_ENABLE_IP6
    if (device < IPCFG_DEVICE_COUNT)
    {
        if (ipcfg_data[device].actual_state != IPCFG_STATE_INIT) 
        {
            if( RTCS6_if_add_dns_addr(ipcfg_data[device].ihandle, dns_addr) == RTCS_OK)
            {
                result = TRUE;
            }
        }
    }
#endif
    return result;
}

/************************************************************************
* NAME: ipcfg6_del_dns_ip
* RETURN: TRUE if successful, FALSE otherwise
* DESCRIPTION: Removes given IPv6 address from DNS list, if exists.
*************************************************************************/
bool ipcfg6_del_dns_ip(uint32_t device, in6_addr *dns_addr)
{
    bool result = FALSE;

#if RTCSCFG_IPCFG_ENABLE_DNS && RTCSCFG_ENABLE_IP6
    if (device < IPCFG_DEVICE_COUNT)
    {
        if (ipcfg_data[device].actual_state != IPCFG_STATE_INIT) 
        {
            if(RTCS6_if_del_dns_addr(ipcfg_data[device].ihandle, dns_addr) == RTCS_OK)
            {
                result = TRUE;
            }
        }
    }
#endif
    return result;
}


#endif /* (BSP_ENET_DEVICE_COUNT > 0) */
#endif /* BSP_ENET_DEVICE_COUNT */
