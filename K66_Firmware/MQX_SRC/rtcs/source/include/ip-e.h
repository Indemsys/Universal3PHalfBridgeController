#ifndef __ipe_h__
#define __ipe_h__
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
*   This file contains the private defines, externs and
*   data structure definitions required by the IP-E layer.
*
*
*END************************************************************************/

#include "ip_prv.h"


/***************************************
**
** Prototypes
**
*/

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t IPE_open  (IP_IF_PTR);
extern uint32_t IPE_close (IP_IF_PTR);
extern uint32_t IPE_send  (IP_IF_PTR, RTCSPCB_PTR, uint16_t, unsigned char[6] );
extern uint32_t IPE_send_IP (IP_IF_PTR, RTCSPCB_PTR, _ip_address, _ip_address, void *);
extern uint32_t IPE_send_IP_ucast  (IP_IF_PTR, RTCSPCB_PTR, unsigned char[6]);
extern uint32_t IPE_send_ARP_bcast (IP_IF_PTR, RTCSPCB_PTR);
extern uint32_t IPE_send_ARP_ucast (IP_IF_PTR, RTCSPCB_PTR, unsigned char[6]);

extern void IPE_recv (PCB_PTR, void *, void (_CODE_PTR_  service)(RTCSPCB_PTR));
extern void IPE_recv_IP  (PCB_PTR, void *);
extern void IPE_recv_ARP (PCB_PTR, void *);

extern uint32_t IPE_join  (IP_IF_PTR, _ip_address);
extern uint32_t IPE_leave (IP_IF_PTR, _ip_address);

#if RTCSCFG_ENABLE_IP6
extern uint32_t IP6E_open (IP_IF_PTR if_ptr);    /* Called by IPE_open.*/
extern uint32_t IP6E_close (IP_IF_PTR if_ptr);   /* Called by IPE_open.*/
extern void IP6E_recv_IP  (PCB_PTR, void *);
extern uint32_t IP6E_send( IP_IF_PTR, RTCSPCB_PTR, in6_addr *, in6_addr *);
extern uint32_t IP6E_join  (IP_IF_PTR, const in6_addr *);
extern uint32_t IP6E_leave (IP_IF_PTR, const in6_addr *);
#endif /* RTCSCFG_ENABLE_IP6 */

extern bool IPE_get_link_status(IP_IF_PTR if_ptr);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
