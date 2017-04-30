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
*   This file contains the Ethernet multicasting
*   interface functions.
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>

#include "enet.h"
#include "enetprv.h"


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_leave
*  Returned Value : ENET_OK or error code
*  Comments       :
*        Leaves a multicast group on an Ethernet channel.
*
*END*-----------------------------------------------------------------*/

uint32_t ENET_leave
   (
      /* [IN] the Ethernet state structure */
      _enet_handle      handle,

      /* [IN] the protocol */
      uint16_t           type,

      /* [IN] the multicast group */
      _enet_address     address
   )
{ 
   ENET_CONTEXT_STRUCT_PTR  enet_ptr = (ENET_CONTEXT_STRUCT_PTR)handle;
   ENET_ECB_STRUCT_PTR  ecb_ptr;
   ENET_MCB_STRUCT_PTR  mcb_ptr, * search_ptr;
   #ifdef ENET_ALLMCAST
      bool  needrejoin;
   #endif

   /*
   ** This function can be called from any context, and it needs mutual
   ** exclusion with itself and with ENET_Join.
   */
   ENET_lock_context(enet_ptr);

   // Make sure it's an open protocol
   for (ecb_ptr = enet_ptr->ECB_HEAD;ecb_ptr;ecb_ptr = ecb_ptr->NEXT) {
      if (ecb_ptr->TYPE == type) {
         /* Found an existing entry */
         break;
      } 
   } 

   // No existing entry found
   if (!ecb_ptr) {
      ENET_unlock_context(enet_ptr);
      return ENETERR_CLOSE_PROT;
   } 

   // Make sure it's a joined group
   for (search_ptr = &ecb_ptr->MCB_HEAD;
       *search_ptr;
        search_ptr = &(*search_ptr)->NEXT) {

      if (((*search_ptr)->GROUP[0] == address[0])
       && ((*search_ptr)->GROUP[1] == address[1])
       && ((*search_ptr)->GROUP[2] == address[2])
       && ((*search_ptr)->GROUP[3] == address[3])
       && ((*search_ptr)->GROUP[4] == address[4])
       && ((*search_ptr)->GROUP[5] == address[5])) {
         /* Found the entry -- delete it */
         break;
      } 
   } 

   if (!*search_ptr) {
      ENET_unlock_context(enet_ptr);
      return ENETERR_LEAVE_GROUP;
   } 

   mcb_ptr = *search_ptr;
   *search_ptr = mcb_ptr->NEXT;

   #ifdef ENET_ALLMCAST
      enet_ptr->MCOUNT--;
      needrejoin = enet_ptr->MCOUNT < ENET_ALLMCAST;

      if (needrejoin) {
   #endif
         (*enet_ptr->PARAM_PTR->ENET_IF->MAC_IF->REJOIN)(handle);
   #ifdef ENET_ALLMCAST
      } 
   #endif

   ENET_unlock_context(enet_ptr);

   _mem_free(mcb_ptr);

   return ENET_OK;
} 


/* EOF */
