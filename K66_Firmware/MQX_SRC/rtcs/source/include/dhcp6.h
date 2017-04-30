#ifndef __dhcp6_h__
#define __dhcp6_h__
/*HEADER**********************************************************************
*
* Copyright 2013-2014 Freescale Semiconductor, Inc.
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
*    Header for DHCPv6.
*
*END************************************************************************/

/*
 * DHCPv6 Options
 */
#define DHCP6_OPTION_CLIENTID       (1)
#define DHCP6_OPTION_SERVERID       (2)
#define DHCP6_OPTION_IA_NA          (3)
#define DHCP6_OPTION_IA_TA          (4)
#define DHCP6_OPTION_IAADDR         (5)
#define DHCP6_OPTION_ORO            (6)
#define DHCP6_OPTION_PREFERENCE     (7)
#define DHCP6_OPTION_ELAPSED_TIME   (8)
#define DHCP6_OPTION_RELAY_MSG      (9)
#define DHCP6_OPTION_AUTH           (11)
#define DHCP6_OPTION_UNICAST        (12)
#define DHCP6_OPTION_STATUS_CODE    (13)
#define DHCP6_OPTION_RAPID_COMMIT   (14)
#define DHCP6_OPTION_USER_CLASS     (15)
#define DHCP6_OPTION_VENDOR_CLASS   (16)
#define DHCP6_OPTION_VENDOR_OPTS    (17)
#define DHCP6_OPTION_INTERFACE_ID   (18)
#define DHCP6_OPTION_RECONF_MSG     (19)
#define DHCP6_OPTION_RECONF_ACCEPT  (20)
#define DHCP6_OPTION_DNS_SERVERS    (23)
#define DHCP6_OPTION_DOMAIN_LIST    (24)
#define DHCP6_OPTION_SOL_MAX_RT     (82)
#define DHCP6_OPTION_INF_MAX_RT     (83)

/*
 * DHCP6 base option lengths
 */
#define DHCP6_OPTION_HEADER_SIZE          (4)
#define DHCP6_OPTION_IA_NA_LENGTH         (12)
#define DHCP6_OPTION_IA_TA_LENGTH         (4)
#define DHCP6_OPTION_IAADDR_LENGTH        (24)
#define DHCP6_OPTION_ORO_LENGTH           (2)
#define DHCP6_OPTION_PREFERENCE_LENGTH    (1)
#define DHCP6_OPTION_ELAPSED_TIME_LENGTH  (2)
#define DHCP6_OPTION_AUTH_LENGTH          (11)
#define DHCP6_OPTION_UNICAST_LENGTH       (16)
#define DHCP6_OPTION_STATUS_CODE_LENGTH   (2)
#define DHCP6_OPTION_RAPID_COMMIT_LENGTH  (0)
#define DHCP6_OPTION_VENDOR_CLASS_LENGTH  (4)
#define DHCP6_OPTION_VENDOR_OPTS_LENGTH   (4)
#define DHCP6_OPTION_RECONF_MSG_LENGTH    (1)
#define DHCP6_OPTION_RECONF_ACCEPT_LENGTH (20)
#define DHCP6_OPTION_DNS_SERVERS_LENGTH   (16)
#define DHCP6_DOMAIN_LIST_MAX_LENGTH      (255)
#define DHCP6_OPTION_SOL_MAX_RT_LENGTH    (4)         
#define DHCP6_OPTION_INF_MAX_RT_LENGTH    (4)

/*
 * DHCPv6 Options limits
 */
#define DHCP6_PREFERENCE_MAX              (255)
#define DHCP6_TIMEOUT_MAX                 (0xFFFFFFFF)

/*
 * DHCPv6 Multicast addresses
 */
#define DHCP6_ALL_RELAYS_AND_SERVERS    "FF02::1:2"
#define DHCP6_ALL_SERVERS               "FF05::1:3"

/*
 * DHCPv6 Message types
 */
#define DHCP6_MSG_SOLICIT               (1)
#define DHCP6_MSG_ADVERTISE             (2)
#define DHCP6_MSG_REQUEST               (3)
#define DHCP6_MSG_CONFIRM               (4)
#define DHCP6_MSG_RENEW                 (5)
#define DHCP6_MSG_REBIND                (6)
#define DHCP6_MSG_REPLY                 (7)
#define DHCP6_MSG_RELEASE               (8)
#define DHCP6_MSG_DECLINE               (9)
#define DHCP6_MSG_RECONFIGURE           (10)
#define DHCP6_MSG_INFORMATION_REQUEST   (11)
#define DHCP6_MSG_RELAY_FORW            (12)
#define DHCP6_MSG_RELAY_REPL            (13)

/*
 * DHCPv6 Status codes
 */
#define DHCP6_STATUS_SUCCESS        (0)
#define DHCP6_STATUS_UNSPECFAIL     (1)
#define DHCP6_STATUS_NOADDRSAVAIL   (2)
#define DHCP6_STATUS_NOBINDING      (3)
#define DHCP6_STATUS_NOTONLINK      (4)
#define DHCP6_STATUS_USEMULTICAST   (5)

/*
 * DHCPv6 port definitions
 */
#define DHCP6_CLIENT_LISTEN_PORT    (546)
#define DHCP6_SERVER_LISTEN_PORT    (547)

/*
 * DHCPv6 Transmission and Retransmission Parameters
 * Times in seconds.
 */
#define DHCP6_SOL_MAX_DELAY     (1)
#define DHCP6_SOL_TIMEOUT       (1)
#define DHCP6_SOL_MAX_RT        (120)
#define DHCP6_REQ_TIMEOUT       (1)
#define DHCP6_REQ_MAX_RT        (30)
#define DHCP6_REQ_MAX_RC        (10)
#define DHCP6_CNF_MAX_DELAY     (1)
#define DHCP6_CNF_TIMEOUT       (1)
#define DHCP6_CNF_MAX_RT        (4)
#define DHCP6_CNF_MAX_RD        (10)
#define DHCP6_REN_TIMEOUT       (10)
#define DHCP6_REN_MAX_RT        (600)
#define DHCP6_REB_TIMEOUT       (10)
#define DHCP6_REB_MAX_RT        (600)
#define DHCP6_INF_MAX_DELAY     (1)
#define DHCP6_INF_TIMEOUT       (1)
#define DHCP6_INF_MAX_RT        (120)
#define DHCP6_REL_TIMEOUT       (1)
#define DHCP6_DEC_TIMEOUT       (1)
#define DHCP6_REC_TIMEOUT       (2)
#define DHCP6_SOL_MAX_RT_MIN    (60)
#define DHCP6_SOL_MAX_RT_MAX    (86400)
#define DHCP6_INF_MAX_RT_MIN    (60)
#define DHCP6_INF_MAX_RT_MAX    (86400)

#define DHCP6_REL_MAX_RC        (5)
#define DHCP6_DEC_MAX_RC        (5)
#define DHCP6_REC_MAX_RC        (8)
#define DHCP6_HOP_COUNT_LIMIT   (32)

/* 128 octets as specified in RFC3315 plus one octet for DUID type code */
#define DHCP6_DUID_MAX_LENGTH   (128+2)

/* DUID prefix. Type 3 - DUID Based on Link-layer Address */
#define DHCP6_DUID_PREFIX       (0x00030000UL)

/* DHCPv6 message header size */
#define DHCP6_MSG_HEADER_SIZE   (sizeof(uint32_t))

#endif
