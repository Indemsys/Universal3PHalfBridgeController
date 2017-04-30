#ifndef __bootp_h__
#define __bootp_h__
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
*   Bootstrap Protocol definitions.
*
*
*END************************************************************************/

#include "ip_prv.h"


/***************************************
**
** Constants
**
*/

#define BOOTP_TIMEOUT_MIN     4000     /* 4 sec */
#define BOOTP_TIMEOUT_MAX     64000    /* 64 sec */

#define BOOTPOP_BOOTREQUEST   1
#define BOOTPOP_BOOTREPLY     2

#define BOOTPFLAG_BROADCAST   0x8000

#define BOOTP_MAGIC           0x63825363L

/*
** BOOTP options
*/
#define BOOTPOPT_PAD    0
#define BOOTPOPT_END    255
#define BOOTPOPT_MASK   1


/***************************************
**
** Type definitions
**
*/

/*
** Contents of a BOOTP packet
*/
typedef struct bootp_header
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
} BOOTP_HEADER, * BOOTP_HEADER_PTR;

typedef struct bootp_data
{
   unsigned char    SNAME[64];
   unsigned char    FILE[128];
   unsigned char    VEND[64];
} BOOTP_DATA, * BOOTP_DATA_PTR;

typedef struct bootp_packet
{
   BOOTP_HEADER   HEAD;
   BOOTP_DATA     DATA;
} BOOTP_PACKET, * BOOTP_PACKET_PTR;

/*
** BOOTP Configuration
*/
typedef struct bootp_cfg
{
   uint32_t        XID;
   uint32_t        TIMEOUT;
   uint32_t        SECS;
   TCPIP_EVENT    RESEND;
   BOOTP_HEADER   PACKET;
} BOOTP_CFG, * BOOTP_CFG_PTR;


/***************************************
**
** Function-specific type definitions
**
*/

/* BOOTP_open() */
typedef struct tcpip_parm_bootp {
   TCPIP_PARM              COMMON;
   _rtcs_if_handle         handle;  /* [IN] the RTCS interface state structure */
   BOOTP_DATA_STRUCT_PTR   data;    /* [IN/OUT] BOOTP parameters */
   BOOTP_CFG               config;
} TCPIP_PARM_BOOTP;


/***************************************
**
** Prototypes
**
*/
#ifdef __cplusplus
extern "C" {
#endif

void BOOTP_open
(
   TCPIP_PARM_BOOTP      *parms
);

uint32_t BOOTP_close
(
   IP_IF_PTR                     /* [IN]     IP interface structure */
);

bool BOOTP_send
(
   TCPIP_EVENT_PTR               /* [IN/OUT] the resend event       */
);

void BOOTP_service
(
   RTCSPCB_PTR                   /* [IN]     BOOTREPLY packet       */
);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
