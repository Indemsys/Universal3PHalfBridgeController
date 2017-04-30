/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
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
*   Definition for the route core functions
*
*
*END************************************************************************/

#ifndef __route_h__
#define __route_h__

/***************************************
**
** Constants
**
*/


/***************************************
**
** Type definitions
**
*/


/***************************************
**
** Prototypes
**
*/

#ifdef __cplusplus
extern "C" {
#endif

void ROUTE_register(
   IP_ROUTE_FN_PTR   routefn  /* [IN] list of routing functions */
);

uint32_t ROUTE_new_bindif(
   IP_ROUTE_DIRECT_PTR   bindif   /* [IN] the outgoing binded interface */
);

IP_ROUTE_INDIRECT_PTR ROUTE_get(
   _ip_address    netaddr, /* [IN] the network address */
   _ip_address    netmask  /* [IN] the network mask */
);

uint32_t ROUTE_insert(
   IP_ROUTE_INDIRECT_PTR    gate,    /* [IN] the route entry to insert */
   _ip_address              netaddr, /* [IN] the network address */
   _ip_address              netmask  /* [IN] the network mask */

);

uint32_t ROUTE_remove(
   IP_ROUTE_INDIRECT_PTR    gate     /* [IN] the route entry to insert */
);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
