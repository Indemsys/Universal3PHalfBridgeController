#ifndef __ipcfg_prv_h__
#define __ipcfg_prv_h__
/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
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
*   Private definitions for the IPCFG layer.
*
*
*END************************************************************************/


#include <rtcs.h>


typedef struct ipcfg_context
{
    /* runtime context */
    LWSEM_STRUCT                control_semaphore;
    LWSEM_STRUCT                request_semaphore;
#if RTCSCFG_IPCFG_ENABLE_DHCP && RTCSCFG_ENABLE_IP4
    LWSEM_STRUCT                dhcp_response_semaphore;
    uint32_t                     dhcp_retries;
#endif
    _rtcs_if_handle             ihandle;
    
    /* parameters for monitoring task */
    IPCFG_STATE                 desired_state;
    IPCFG_IP_ADDRESS_DATA       desired_ip_data;
    
    /* results */ 
    IPCFG_STATE                 actual_state;
    IPCFG_IP_ADDRESS_DATA       actual_ip_data;
#if RTCSCFG_IPCFG_ENABLE_DNS
    _ip_address                 dns;
#endif
#if RTCSCFG_IPCFG_ENABLE_BOOT
    _ip_address                 tftp_serveraddress;
    unsigned char                       tftp_servername[RTCS_SNAME_LEN];
    unsigned char                       boot_filename[RTCS_BOOTFILE_LEN];
#endif  
} IPCFG_CONTEXT, * IPCFG_CONTEXT_PTR;


#endif
/* EOF */
