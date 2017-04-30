#ifndef __arp_h__
#define __arp_h__
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
*   Address Resolution Protocol definitions.
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

extern uint32_t ARP_open
(
   IP_IF_PTR      if_ptr      /* [IN] IP interface structure */
);

extern uint32_t ARP_close
(
   IP_IF_PTR      if_ptr      /* [IN] IP interface structure */
);

extern void ARP_service
(
   RTCSPCB_PTR    rtcs_pcb    /* [IN] received packet */
);

extern uint32_t ARP_resolve
(
   IP_IF_PTR      if_ptr,     /* [IN] IP interface structure */
   RTCSPCB_PTR    rtcs_pcb,   /* [IN] packet to send */
   _ip_address    isrc,       /* [IN] source address */
   _ip_address    idest       /* [IN] destination address */
);

extern uint32_t ARP_request
(
   IP_IF_PTR      if_ptr,
   _ip_address    isrc,
   _ip_address    idest
);

extern bool ARP_do_proxy
(
   IP_IF_PTR      iflocal,    /* [IN] the local interface */
   _ip_address    iplocal     /* [IN] the IP address to test */
);

extern bool ARP_is_complete
(
  IP_IF_PTR      if_ptr,      /* [IN] IP interface structure */
  _ip_address    addr         /* [IN] the IP address to search for */
);

extern bool  ARP_kill_entry
(
  IP_IF_PTR      if_ptr,       /* [IN] IP interface structure */
  _ip_address    addr          /* [IN] the IP address to remove from arp Q */
        
);

extern  RTCSPCB_PTR ARP_find_waiting
(
  IP_IF_PTR      if_ptr,       /* [IN] IP interface structure */
  _ip_address    addr          /* [IN] the IP address to remove from */
);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
