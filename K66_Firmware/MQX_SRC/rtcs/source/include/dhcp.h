#ifndef __dhcp_h__
#define __dhcp_h__
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
*   Dynamic Host Configuration Protocol definitions.
*
*
*END************************************************************************/


/***************************************
**
** Constants
**
*/

#define DHCPOP_BOOTREQUEST    1
#define DHCPOP_BOOTREPLY      2

#define DHCPTYPE_DHCPDISCOVER 1
#define DHCPTYPE_DHCPOFFER    2
#define DHCPTYPE_DHCPREQUEST  3
#define DHCPTYPE_DHCPDECLINE  4
#define DHCPTYPE_DHCPACK      5
#define DHCPTYPE_DHCPNAK      6
#define DHCPTYPE_DHCPRELEASE  7
#define DHCPTYPE_DHCPINFORM   8

#define DHCPTYPE_ISVALID(type)   ((type) >= 1 && (type) <= 8)

#define DHCPFLAG_BROADCAST                0x8000

#define DHCP_MAGIC            0x63825363L
#define DHCP_MIN_MESSAGE_SIZE 576
/*
** DHCP options
*/
#define DHCPOPT_PAD           0
#define DHCPOPT_END           255
#define DHCPOPT_MASK          1
/* Start CR 1152 */
#define DHCPOPT_TIMEOFFSET    2
#define DHCPOPT_ROUTER        3
#define DHCPOPT_TIMESERVER    4
#define DHCPOPT_NAMESERVER    5
#define DHCPOPT_DNSSERVER     6
#define DHCPOPT_HOSTNAME      12
#define DHCPOPT_BOOTFILESIZE  13
#define DHCPOPT_DOMAINNAME    15
#define DHCPOPT_ADDRESS       50
#define DHCPOPT_LEASE         51
#define DHCPOPT_OVERLOAD      52
#define DHCPOPT_MSGTYPE       53
#define DHCPOPT_SERVERID      54
#define DHCPOPT_PARAMLIST     55
#define DHCPOPT_MSGSIZE       57
#define DHCPOPT_RENEWALL      58
#define DHCPOPT_REBINDING     59
#define DHCPOPT_CLASSID       60
#define DHCPOPT_CLIENTID      61
#define DHCPOPT_SERVERNAME    66
#define DHCPOPT_TFTPSERVER    66
#define DHCPOPT_FILENAME      67
#define DHCPOPT_BOOTFILENAME  67
/* End CR 1152 */
#define DHCPOPT_WWW_SRV       72
#define DHCPOPT_FINGER_SRV    73

#define DHCPSIZE_MAGIC        4
#define DHCPSIZE_MASK         4
#define DHCPSIZE_ADDRESS      4
#define DHCPSIZE_LEASE        4
#define DHCPSIZE_MSGTYPE      1
#define DHCPSIZE_SERVERID     4
#define DHCPSIZE_MSGSIZE      2

#define DHCP_MSGSIZE_MIN      576

#define DHCP_LEASE_INFINITE   0xFFFFFFFF
/*
**  Flags
*/
#define DHCP_SEND_INFORM_MESSAGE              0x00000001
#define DHCP_MAINTAIN_STATE_ON_INFINITE_LEASE 0x00000002
/* Start CR 1547 */
#define DHCP_SEND_PROBE                       0x00000004
/* End CR 1547 */

/*
** Initialization Structure for DHCP Client
*/

typedef struct  dhcp_data_struct {
   int32_t      (_CODE_PTR_ CHOICE_FUNC)(unsigned char *, uint32_t);
   void        (_CODE_PTR_ BIND_FUNC)  (unsigned char *, uint32_t, _rtcs_if_handle);
   void        (_CODE_PTR_ REBIND_FUNC)(unsigned char *, uint32_t, _rtcs_if_handle);
   bool     (_CODE_PTR_ UNBIND_FUNC)(_rtcs_if_handle);
   bool     (_CODE_PTR_ FAILURE_FUNC)(_rtcs_if_handle);
   uint32_t     FLAGS;
/* Start CR 2199 */   
   bool     (_CODE_PTR_ NAK_FUNC)  (unsigned char *, uint32_t, _rtcs_if_handle);
/* End CR 2199 */   
} DHCP_DATA_STRUCT, * DHCP_DATA_STRUCT_PTR;

/***************************************
** Contents of a DHCP Packet Header
*/

typedef struct dhcp_header
{
   unsigned char    OP[1];
   unsigned char    HTYPE[1];
   unsigned char    HLEN[1];
   unsigned char    HOPS[1];
   unsigned char    XID[4];
   unsigned char    SECS[2];
   unsigned char    FLAGS[2];
   unsigned char    CIADDR[4];
   unsigned char    YIADDR[4];
   unsigned char    SIADDR[4];
   unsigned char    GIADDR[4];
   unsigned char    CHADDR[16];
   unsigned char    SNAME[64];
   unsigned char    FILE[128];
} DHCP_HEADER, * DHCP_HEADER_PTR;

/***************************************
**
** Function definitions
**
*/
#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t RTCS_if_bind_DHCP_flagged(_rtcs_if_handle, DHCP_DATA_STRUCT_PTR, unsigned char *, uint32_t);
extern uint32_t RTCS_if_bind_DHCP_timed(_rtcs_if_handle, DHCP_DATA_STRUCT_PTR, unsigned char *, uint32_t, uint32_t);
extern uint32_t RTCS_if_bind_DHCP(_rtcs_if_handle, DHCP_DATA_STRUCT_PTR, unsigned char *, uint32_t);
extern uint32_t RTCS_if_rebind_DHCP(_rtcs_if_handle, _ip_address, _ip_address, uint32_t, _ip_address, DHCP_DATA_STRUCT_PTR, unsigned char *, uint32_t);
extern uint32_t RTCS_request_DHCP_inform(_rtcs_if_handle, unsigned char *, uint32_t, _ip_address, _ip_address, void (_CODE_PTR_ inform_func)(unsigned char *, uint32_t, _rtcs_if_handle));
extern void    DHCPCLNT_release     (_rtcs_if_handle);

extern unsigned char      *DHCPCLNT_find_option(unsigned char *, uint32_t, unsigned char);
extern unsigned char      *DHCP_find_option(unsigned char *, uint32_t, unsigned char);

extern bool DHCP_option_int8     (unsigned char **, uint32_t *, unsigned char, unsigned char);
extern bool DHCP_option_int16    (unsigned char **, uint32_t *, unsigned char, uint16_t);
extern bool DHCP_option_int32    (unsigned char **, uint32_t *, unsigned char, uint32_t);
extern bool DHCP_option_variable (unsigned char **, uint32_t *, unsigned char, unsigned char *, uint32_t);
extern bool DHCP_option_addr     (unsigned char **, uint32_t *, unsigned char, _ip_address);
extern bool DHCP_option_addrlist (unsigned char **, uint32_t *, unsigned char, _ip_address *, uint32_t);
extern bool DHCP_option_string   (unsigned char **, uint32_t *, unsigned char, char *);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
