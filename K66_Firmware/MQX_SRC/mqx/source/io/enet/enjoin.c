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
*  Function Name  : ENET_join
*  Returned Value : ENET_OK or error code
*  Comments       :
*        Joins a multicast group on an Ethernet channel.
*
*END*-----------------------------------------------------------------*/

uint32_t ENET_join
   (
      /* [IN] the Ethernet state structure */
      _enet_handle      handle,

      /* [IN] the protocol */
      uint16_t           type,

      /* [IN] the multicast group */
      _enet_address     address
   )
{ 
   ENET_CONTEXT_STRUCT_PTR enet_ptr = (ENET_CONTEXT_STRUCT_PTR)handle;
   ENET_ECB_STRUCT_PTR     ecb_ptr;
   ENET_MCB_STRUCT_PTR     mcb_ptr;
   #ifdef ENET_ALLMCAST
      bool  needjoin;
   #endif

   /*
   ** Make sure it's a multicast group
   */
   #if MQX_CHECK_ERRORS
      if ((address[0] & 1) == 0) {
         return ENETERR_JOIN_MULTICAST;
      } /* Endif */
   #endif

   /*
   ** This function can be called from any context, and it needs mutual
   ** exclusion with itself and with ENET_leave.
   */
   ENET_lock_context(enet_ptr);

   /*
   ** Make sure it's an open protocol
   */
   for (ecb_ptr = enet_ptr->ECB_HEAD;
        ecb_ptr;
        ecb_ptr = ecb_ptr->NEXT) {

      if (ecb_ptr->TYPE == type) {
         /* Found an existing entry */
         break;
      } 
   } 

   /*
   ** No existing entry found
   */
   if (!ecb_ptr) {
      ENET_unlock_context(enet_ptr);
      return ENETERR_CLOSE_PROT;
   } 

   /*
   ** Create an entry for this group
   */
   mcb_ptr = _mem_alloc_system_zero(sizeof(ENET_MCB_STRUCT));
   if (!mcb_ptr) {
      ENET_unlock_context(enet_ptr);
      return ENETERR_ALLOC_MCB;
   } /* Endif */
   _mem_set_type(mcb_ptr, MEM_TYPE_IO_ENET_MCB);
   
   eaddrcpy(mcb_ptr->GROUP,address);

   mcb_ptr->NEXT = ecb_ptr->MCB_HEAD;
   ecb_ptr->MCB_HEAD = mcb_ptr;

   #ifdef ENET_ALLMCAST
      needjoin = enet_ptr->MCOUNT < ENET_ALLMCAST;
      enet_ptr->MCOUNT++;
      if (needjoin) {
   #endif
   
         (*enet_ptr->PARAM_PTR->ENET_IF->MAC_IF->JOIN)(handle, mcb_ptr);
   
   #ifdef ENET_ALLMCAST
      } 
   #endif

   ENET_unlock_context(enet_ptr);

   return ENET_OK;

} 

/* EOF */
