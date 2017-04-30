#ifndef __icmp_h__
#define __icmp_h__
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
*   Definitions for the ICMP protocol layer.
*
*
*END************************************************************************/


/***************************************
**
** Constants
**
*/

/*
** ICMP message types
*/
#define ICMPTYPE_ECHO_REPLY   0
#define ICMPTYPE_DESTUNREACH  3
#define ICMPTYPE_SRCQUENCH    4
#define ICMPTYPE_REDIRECT     5
#define ICMPTYPE_ECHO_REQ     8
#define ICMPTYPE_TIMEEXCEED   11
#define ICMPTYPE_PARMPROB     12
#define ICMPTYPE_TIME_REQ     13
#define ICMPTYPE_TIME_REPLY   14
#define ICMPTYPE_INFO_REQ     15    /* obsolete */
#define ICMPTYPE_INFO_REPLY   16    /* obsolete */
#define ICMPTYPE_AM_REQ       17
#define ICMPTYPE_AM_REPLY     18

/*
** Destination Unreachable codes
*/
#define ICMPCODE_DU_NET_UNREACH     0
#define ICMPCODE_DU_HOST_UNREACH    1
#define ICMPCODE_DU_PROTO_UNREACH   2
#define ICMPCODE_DU_PORT_UNREACH    3
#define ICMPCODE_DU_NEEDFRAG        4
#define ICMPCODE_DU_SRCROUTE        5
#define ICMPCODE_DU_NET_UNKNOWN     6
#define ICMPCODE_DU_HOST_UNKNOWN    7
#define ICMPCODE_DU_HOST_ISOLATED   8
#define ICMPCODE_DU_NET_PROHIB      9
#define ICMPCODE_DU_HOST_PROHIB     10
#define ICMPCODE_DU_NET_TOS         11
#define ICMPCODE_DU_HOST_TOS        12

/*
** Redirect codes
*/
#define ICMPCODE_RD_NET             0
#define ICMPCODE_RD_HOST            1
#define ICMPCODE_RD_NET_TOS         2
#define ICMPCODE_RD_HOST_TOS        3

/*
** Time Exceeded codes
*/
#define ICMPCODE_TE_TTL             0
#define ICMPCODE_TE_REASM           1


/***************************************
**
** Type definitions
**
*/

/*
** ICMP echo parameters
*/

typedef struct icmp_echo_param
{
    TCPIP_PARM                          COMMON;
    struct icmp_echo_param             *NEXT;
    struct icmp_echo_param            **PREV;
    TCPIP_EVENT                         EXPIRE;
    PING_PARAM_STRUCT_PTR               ping_param;
    uint16_t                             seq;
    uint32_t                             start_time;
} ICMP_ECHO_PARAM, * ICMP_ECHO_PARAM_PTR;


/***************************************
**
** Prototypes
**
*/

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t ICMP_init
(
   void
);

extern void ICMP_send_echo
(
   ICMP_ECHO_PARAM_PTR
);

extern void ICMP_send_error
(
   uint8_t         type,    /* [IN] the type to send */
   uint8_t         code,    /* [IN] the code to send */
   uint32_t        param,   /* [IN] the icmp parameter */
   RTCSPCB_PTR    pcb,     /* [IN] the packet which caused the error */
   int32_t         layer    /* [IN] IP layer, relative to current */
);

extern void ICMP_service
(
   RTCSPCB_PTR,
   void *
);

#ifdef __cplusplus
}
#endif


#endif
/* EOF */
