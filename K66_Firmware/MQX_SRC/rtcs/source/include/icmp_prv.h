#ifndef __icmp_prv_h__
#define __icmp_prv_h__
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
*   Private definitions for the ICMP protocol layer.
*
*
*END************************************************************************/

#include "ip_prv.h"     /* For IP_HEADER */


/***************************************
**
** Code macros
**
*/

#define ICMPTYPE_ISQUERY(t)   (((t) == ICMPTYPE_ECHO_REPLY) \
                            || ((t) == ICMPTYPE_ECHO_REQ)   \
                            || (((t) >= ICMPTYPE_TIME_REQ) && ((t) <= ICMPTYPE_AM_REPLY)))


#if RTCSCFG_ENABLE_ICMP_STATS 
   #define IF_ICMP_STATS_ENABLED(x) x
#else
   #define IF_ICMP_STATS_ENABLED(x)
#endif


/***************************************
**
** Type definitions
**
*/

/*
** Generic ICMP packet header
*/
typedef struct icmp_header
{
   unsigned char    TYPE[1];
   unsigned char    CODE[1];
   unsigned char    CHECKSUM[2];
} ICMP_HEADER, * ICMP_HEADER_PTR;

/*
** ICMP Redirect packet
*/
typedef struct icmp_err_header
{
   ICMP_HEADER HEAD;
   unsigned char       DATA[4];
   IP_HEADER   IP;
} ICMP_ERR_HEADER, * ICMP_ERR_HEADER_PTR;

/*
** ICMP Echo packet
*/
typedef struct icmp_echo_header
{
   ICMP_HEADER HEAD;
   unsigned char       ID[2];
   unsigned char       SEQ[2];
} ICMP_ECHO_HEADER, * ICMP_ECHO_HEADER_PTR;

/*
** ICMP Configuration.  This information is persistent for the ICMP layer.
*/
typedef struct icmp_cfg_struct {
#if RTCSCFG_ENABLE_ICMP_STATS
   ICMP_STATS           STATS;
#endif
   ICMP_ECHO_PARAM_PTR  ECHO_PARAM_HEAD;
   uint16_t              ECHO_SEQ;
   uint16_t              RESERVED;

} ICMP_CFG_STRUCT, * ICMP_CFG_STRUCT_PTR;


/***************************************
**
** Prototypes
**
*/

#ifdef __cplusplus
extern "C" {
#endif

extern void ICMP_send_error_internal
(
   uint8_t         type,    /* [IN] the type to send */
   uint8_t         code,    /* [IN] the code to send */
   uint32_t        param,   /* [IN] the icmp parameter */
   IP_HEADER_PTR  iph,     /* [IN] the IP header */
   RTCSPCB_PTR    origpcb, /* [IN] the bad packet pcb */
   uint32_t        maxlen   /* [IN[ the max data len to send, 0 = default */
);

#ifdef __cplusplus
}
#endif



#endif
/* EOF */
