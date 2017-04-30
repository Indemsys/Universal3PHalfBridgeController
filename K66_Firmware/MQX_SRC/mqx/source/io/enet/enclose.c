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
*   This file contains the Ethernet close
*   interface function.
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>

#include "enet.h"
#include "enetprv.h"

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_close
*  Returned Value : ENET_OK or error code
*  Comments       :
*        Unregisters a protocol type on an Ethernet channel.
*
*END*-----------------------------------------------------------------*/

uint32_t ENET_close
   (
      /* [IN] the Ethernet state structure */
      _enet_handle      handle,

      /* [IN] the protocol */
      uint16_t           type
   )
{ 
   ENET_CONTEXT_STRUCT_PTR enet_ptr = (ENET_CONTEXT_STRUCT_PTR)handle;
   ENET_ECB_STRUCT_PTR     ecb_ptr, * search_ptr;
   ENET_MCB_STRUCT_PTR     mcb_ptr, next_ptr;

      #ifdef ENET_ALLMCAST
            uint32_t        mcount;
            bool        needrejoin;
      #endif


   /*
   ** This function can be called from any context, and it needs mutual
   ** exclusion with itself, ENET_open(), ENET_join(), ENET_leave(),
   ** and ENET_ISR().
   */
   ENET_lock_context(enet_ptr);

   // Search for an existing entry for type
   for (search_ptr = (ENET_ECB_STRUCT_PTR *)&enet_ptr->ECB_HEAD;
       *search_ptr; search_ptr = &(*search_ptr)->NEXT) 
   {

      if ((*search_ptr)->TYPE == type) {
         // Found an existing entry -- delete it
         break;
      }  
   } 

   if (!*search_ptr) {
      // No existing entry found
      ENET_unlock_context(enet_ptr);
      return ENETERR_CLOSE_PROT;
   } 

   ecb_ptr = *search_ptr;
   *search_ptr = ecb_ptr->NEXT;

      #ifdef ENET_ALLMCAST
         mcb_ptr = ecb_ptr->MCB_HEAD;
         mcount = 0;
         while (mcb_ptr) {
            mcount++;
            mcb_ptr = mcb_ptr->NEXT;
         } 
         enet_ptr->MCOUNT -= mcount;
         needrejoin = mcount && (enet_ptr->MCOUNT < ENET_ALLMCAST);
      #endif

   ENET_unlock_context(enet_ptr);

   mcb_ptr = ecb_ptr->MCB_HEAD;
   _mem_free(ecb_ptr);
   while (mcb_ptr) {
      next_ptr = mcb_ptr->NEXT;
      _mem_free(mcb_ptr);
      mcb_ptr = next_ptr;
   } 

      #ifdef ENET_ALLMCAST
         if (needrejoin) {
      #endif
            ENET_lock_context(enet_ptr);
            (*enet_ptr->PARAM_PTR->ENET_IF->MAC_IF->REJOIN)(handle);
            ENET_unlock_context(enet_ptr);
      #ifdef ENET_ALLMCAST
         }
      #endif

   return ENET_OK;

}


/* EOF */
