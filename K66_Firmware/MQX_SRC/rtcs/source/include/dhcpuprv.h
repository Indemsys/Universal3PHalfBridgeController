#ifndef __dhcpuprv_h__
#define __dhcpuprv_h__
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
*   Dynamic Host Configuration Protocol utility prototypes.
*
*
*END************************************************************************/

/***************************************
**
** Prototypes
**
*/

#ifdef __cplusplus
extern "C" {
#endif

static void DHCPCLNT_fill_header ( DHCP_CLNT_STRUCT *);
static void DHCPCLNT_fill_options (DHCP_CLNT_STRUCT *,
                                   TCPIP_PARM_IF_DHCP *);
static void DHCPCLNT_open ( DHCP_CLNT_STRUCT *);
static void DHCPCLNT_modify_options ( DHCP_CLNT_STRUCT *);
static void DHCPCLNT_send_one_shot ( DHCP_CLNT_STRUCT *);
static bool DHCPCLNT_rebind_lease ( TCPIP_EVENT *);
static bool DHCPCLNT_renew_lease ( TCPIP_EVENT *);
static bool DHCPCLNT_terminate_lease ( TCPIP_EVENT *);
static bool DHCPCLNT_bind_attempt_timeout( TCPIP_EVENT *);
static void DHCPCLNT_decline ( DHCP_CLNT_STRUCT *, uint32_t );
static void DHCPCLNT_parse_offer ( DHCP_CLNT_STRUCT *, RTCSPCB *);
static void DHCPCLNT_copy_binding ( DHCP_CLNT_STRUCT *);
static bool DHCPCLNT_verify_address ( DHCP_CLNT_STRUCT *);
static unsigned char DHCPCLNT_verify_packet (DHCP_CLNT_STRUCT *, RTCSPCB_PTR, unsigned char *, uint32_t *, _ip_address *, _ip_address *, uint32_t *);
static void DHCPCLNT_service_offer (DHCP_CLNT_STRUCT *, RTCSPCB_PTR, unsigned char *, uint32_t, _ip_address, _ip_address);
static void DHCPCLNT_service_ack (DHCP_CLNT_STRUCT *, RTCSPCB_PTR, unsigned char *, uint32_t, IP_IF_PTR);
/* Start CR 2199 */
static void DHCPCLNT_service_nak (DHCP_CLNT_STRUCT *, unsigned char *, uint32_t);
/* End CR 2199 */  
static void DHCPCLNT_set_timed_events(DHCP_CLNT_STRUCT *);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
