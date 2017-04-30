#ifndef __udp_prv_h__
#define __udp_prv_h__
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
*   This file contains the private User Datagram Protocol
*   definitions.  For more details, refer to RFC768.
*
*
*END************************************************************************/

/*
** This is used to signal the waiting task after 
** socket closed by other tasks. 
*/
#define UDP_SOCKET_ENQUEUED      20
#define UDP_SOCKET_CLOSE         21

#if RTCSCFG_ENABLE_UDP_STATS
#define IF_UDP_STATS_ENABLED(x) x
#else
#define IF_UDP_STATS_ENABLED(x)
#endif

/***************************************
**
** Type definitions
**
*/

typedef struct udp_header {
   unsigned char    SRC_PORT[2];
   unsigned char    DEST_PORT[2];
   unsigned char    LENGTH[2];
   unsigned char    CHECKSUM[2];
} UDP_HEADER, * UDP_HEADER_PTR;

/*
** UDP Configuration.  This information is persistent for the UDP layer.
*/
typedef struct udp_cfg_struct {
#if RTCSCFG_ENABLE_UDP_STATS
   UDP_STATS      STATS;
#endif
   UCB_STRUCT_PTR BOUND_UCB_HEAD;   /* The head of the bound UCB chain      */
   uint16_t        LAST_PORT;        /* Last used UDP port                   */
   uint16_t        RESERVED0;

   UCB_STRUCT_PTR GROUND_UCB_HEAD;  /* The head of the ground UCB chain     */
   UCB_STRUCT_PTR LBOUND_UCB_HEAD;  /* The head of local IP bound UCB chain */
   UCB_STRUCT_PTR OPEN_UCB_HEAD;    /* The head of the open UCB chain       */

} UDP_CFG_STRUCT, * UDP_CFG_STRUCT_PTR;


/***************************************
**
** Prototypes
**
*/

#ifdef __cplusplus
extern "C" {
#endif

uint16_t UDP_source
(
   RTCSPCB_PTR                /* [IN] packet to find source of */
);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
