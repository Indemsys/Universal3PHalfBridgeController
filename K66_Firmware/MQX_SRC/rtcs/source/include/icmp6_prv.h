#ifndef __icmp6_prv_h__
#define __icmp6_prv_h__
/*HEADER**********************************************************************
*
* Copyright 2011-2013 Freescale Semiconductor, Inc.
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
*   Private definitions for the ICMP6 protocol layer.
*
*
*END************************************************************************/

#include "icmp6.h"
#include "ip6_prv.h"     /* For IP_HEADER */

#if RTCSCFG_ENABLE_ICMP6_STATS 
   #define IF_ICMP6_STATS_ENABLED(x) x
#else
   #define IF_ICMP6_STATS_ENABLED(x)
#endif

/***********************************************************************
*
* Type definitions
*
***********************************************************************/

/***********************************************************************
 * Generic ICMP packet header
 ***********************************************************************
 *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     Type      |     Code      |          Checksum             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * +                         Message Body                          +
 * |                                                               |
 *
 ***********************************************************************/      
typedef struct icmp6_header
{
   unsigned char    TYPE[1];
   unsigned char    CODE[1];
   unsigned char    CHECKSUM[2];
} ICMP6_HEADER, * ICMP6_HEADER_PTR;

/***********************************************************************
 * ICMPv6 Echo packet
 ***********************************************************************
 * RFC4443 4:
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |    Type       |       Code    |             Checksum          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |             Identifier        |       Sequence Number         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | Data ...
 * +-+-+-+-+- 
 ***********************************************************************/
typedef struct icmp6_echo_header
{
   ICMP6_HEADER HEAD;
   unsigned char       ID[2];
   unsigned char       SEQ[2];
} ICMP6_ECHO_HEADER, * ICMP6_ECHO_HEADER_PTR;

/***********************************************************************
 * ICMPv6 Error packet
 ***********************************************************************/
typedef struct icmp6_err_header
{
   ICMP6_HEADER HEAD;
   unsigned char        DATA[4];
} ICMP6_ERR_HEADER, * ICMP6_ERR_HEADER_PTR;

/***********************************************************************
 * ICMP Configuration.  This information is persistent for the ICMP layer.
 ***********************************************************************/
typedef struct icmp6_cfg_struct
{
#if RTCSCFG_ENABLE_ICMP6_STATS 
   ICMP6_STATS          STATS;
#endif
   ICMP_ECHO_PARAM_PTR  ECHO_PARAM_HEAD;
   uint16_t             ECHO_SEQ;
   uint16_t             RESERVED;
} ICMP6_CFG_STRUCT, * ICMP6_CFG_STRUCT_PTR;

/*
* Error messages are identified as such by a
* zero in the high-order bit of their message Type field values. Thus,
* error messages have message types from 0 to 127; informational
* messages have message types from 128 to 255.
*/
#define ICMP6_TYPE_IS_ERROR(t) (((t) & 0x80) == 0x00)

#endif /* __icmp6_prv_h__ */


