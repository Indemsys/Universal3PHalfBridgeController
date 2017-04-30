#ifndef __ipcfg_h__
#define __ipcfg_h__
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
*   Definitions for the IP config layer.
*
*   RTCSCFG_ENABLE_GATEWAYS must be set to work with gateways
*   RTCSCFG_IPCFG_ENABLE_DNS must be set to enable DNS resolving
*   RTCSCFG_IPCFG_ENABLE_DHCP must be set for DHCP
*   RTCSCFG_IPCFG_ENABLE_BOOT must be set to enable tftp/boot names processing
*
*
*END************************************************************************/

#include <rtcs.h>

/* configuration defines */
#define IPCFG_DEBUG_LEVEL           0

#define IPCFG_MAX_NAK_RETRIES       6
#define IPCFG_MAX_AUTOIP_ATTEMPTS   256

#define IPCFG_TASK_NAME             "IPCFG_TASK"
#define IPCFG_TASK_STACK_SIZE       1000


#define IPCFG_DEVICE_COUNT  BSP_ENET_DEVICE_COUNT

/* error codes - backward compatibility */
#define IPCFG_ERROR_OK              IPCFG_OK
#define IPCFG_ERROR_BUSY            RTCSERR_IPCFG_BUSY
#define IPCFG_ERROR_DEVICE_NUMBER   RTCSERR_IPCFG_DEVICE_NUMBER
#define IPCFG_ERROR_INIT            RTCSERR_IPCFG_INIT
#define IPCFG_ERROR_BIND            RTCSERR_IPCFG_BIND


/* types */
typedef enum ipcfg_state
{
    IPCFG_STATE_INIT = 0,
    IPCFG_STATE_UNBOUND,
    IPCFG_STATE_BUSY,
    IPCFG_STATE_STATIC_IP,
    IPCFG_STATE_DHCP_IP,
    IPCFG_STATE_AUTO_IP,
    IPCFG_STATE_DHCPAUTO_IP,
    IPCFG_STATE_BOOT
} IPCFG_STATE;

typedef struct ipcfg_ip_address_data
{
    _ip_address ip;
    _ip_address mask;
    _ip_address gateway;
} IPCFG_IP_ADDRESS_DATA, * IPCFG_IP_ADDRESS_DATA_PTR;

#ifdef __cplusplus
extern "C" {
#endif


/* ipcfg general API */
uint32_t         ipcfg_init_device (uint32_t, _enet_address);
uint32_t         ipcfg_release_device(uint32_t);
uint32_t         ipcfg_init_interface(uint32_t, _rtcs_if_handle);
uint32_t         ipcfg_release_interface(uint32_t);

/* immediate action API */
uint32_t         ipcfg_unbind (uint32_t);
uint32_t         ipcfg_bind_staticip (uint32_t, IPCFG_IP_ADDRESS_DATA_PTR);
uint32_t         ipcfg_bind_autoip (uint32_t, IPCFG_IP_ADDRESS_DATA_PTR);
uint32_t         ipcfg_bind_dhcp (uint32_t, bool);
uint32_t         ipcfg_poll_dhcp (uint32_t, bool, IPCFG_IP_ADDRESS_DATA_PTR);
uint32_t         ipcfg_bind_boot (uint32_t);
uint32_t         ipcfg_bind_dhcp_wait (uint32_t, bool, IPCFG_IP_ADDRESS_DATA_PTR);

/* monitoring-task API */
uint32_t         ipcfg_task_create (uint32_t, uint32_t);
void            ipcfg_task_destroy (bool);
bool         ipcfg_task_status (void);
bool         ipcfg_task_poll (void);

/* general information retrieval */
uint32_t         ipcfg_get_device_number(_rtcs_if_handle);
uint32_t         ipcfg_add_interface(uint32_t, _rtcs_if_handle);
uint32_t         ipcfg_del_interface (uint32_t);
_rtcs_if_handle ipcfg_get_ihandle (uint32_t);
bool         ipcfg_get_mac (uint32_t, _enet_address);
const char *    ipcfg_get_state_string (IPCFG_STATE);
IPCFG_STATE     ipcfg_get_state (uint32_t);
bool         ipcfg_phy_registers(uint32_t,uint32_t,uint32_t *);
IPCFG_STATE     ipcfg_get_desired_state (uint32_t);
bool         ipcfg_get_link_active (uint32_t);
_ip_address     ipcfg_get_dns_ip (uint32_t, uint32_t);
bool         ipcfg_add_dns_ip (uint32_t, _ip_address);
bool         ipcfg_del_dns_ip (uint32_t, _ip_address);
bool         ipcfg_get_ip (uint32_t, IPCFG_IP_ADDRESS_DATA_PTR);
_ip_address     ipcfg_get_tftp_serveraddress (uint32_t);
unsigned char       *ipcfg_get_tftp_servername (uint32_t);
unsigned char       *ipcfg_get_boot_filename (uint32_t);

#define IPCFG_INC_IP(ip, mask, val)     ((ip & mask) | ((ip + val + ((((ip + val) & 0xff) == 0xff) ? 2 : 0)) & ~mask))


/******************************************************************
* IPv6 ipcfg
*******************************************************************/

/******************************************************************
* Structures
*******************************************************************/
typedef struct ipcfg6_bind_addr_data
{
    in6_addr            ip_addr;    /* is IP address if "type" is MANUAL, 
                                     * or is prefix/64 if "type" is AUTOCONFIGURABLE"*/
    rtcs6_if_addr_type  ip_addr_type;
} IPCFG6_BIND_ADDR_DATA, * IPCFG6_BIND_ADDR_DATA_PTR;

typedef struct ipcfg6_unbind_addr_data
{
    in6_addr        ip_addr;
} IPCFG6_UNBIND_ADDR_DATA, * IPCFG6_UNBIND_ADDR_DATA_PTR;

typedef struct ipcfg6_get_addr_data
{
    in6_addr            ip_addr;        /* Address.*/
    rtcs6_if_addr_state ip_addr_state;  /* Address current state.*/
    rtcs6_if_addr_type  ip_addr_type;   /* Address type.*/
} IPCFG6_GET_ADDR_DATA, * IPCFG6_GET_ADDR_DATA_PTR;


/******************************************************************
* Function Prototypes
*******************************************************************/
/* Immediate action API. */
uint32_t         ipcfg6_unbind_addr (uint32_t, IPCFG6_UNBIND_ADDR_DATA_PTR);
uint32_t         ipcfg6_bind_addr (uint32_t, IPCFG6_BIND_ADDR_DATA_PTR);

/* General information retrieval. */
uint32_t ipcfg6_get_addr( uint32_t device, uint32_t n /*>0*/, IPCFG6_GET_ADDR_DATA_PTR data/*out*/);
uint32_t ipcfg6_get_scope_id( uint32_t device /* in */ );
bool ipcfg6_get_dns_ip(uint32_t device, uint32_t n, in6_addr *dns_addr);
bool ipcfg6_add_dns_ip(uint32_t device, in6_addr *dns_addr);
bool ipcfg6_del_dns_ip(uint32_t device, in6_addr *dns_addr);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
