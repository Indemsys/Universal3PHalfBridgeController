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
*   This file contains the Ethernet open interface function.
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>

#include "enet.h"
#include "enetprv.h"

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_open
*  Returned Value : ENET_OK or error code
*  Comments       :
*        Registers a protocol type on an Ethernet channel.
*
*END*-----------------------------------------------------------------*/

uint32_t ENET_open
   (
      /* [IN] the Ethernet state structure */
      _enet_handle      handle,

      /* [IN] the protocol */
      uint16_t           type,

      /* [IN] the callback function */
      void (_CODE_PTR_  service)(PCB_PTR, void *),

      /* [IN] private data for the callback */
      void             *prv
   )
{ 
   ENET_CONTEXT_STRUCT_PTR enet_ptr = (ENET_CONTEXT_STRUCT_PTR)handle;
   ENET_ECB_STRUCT_PTR     ecb_ptr, * search_ptr;

   /*
   ** This function can be called from any context, and it needs mutual
   ** exclusion with itself, ENET_close(), and ENET_ISR().
   */
   ENET_lock_context(enet_ptr);

   // Search for an existing entry for type
   for (search_ptr = (ENET_ECB_STRUCT_PTR *)&enet_ptr->ECB_HEAD; *search_ptr; search_ptr = &(*search_ptr)->NEXT) {
      if ((*search_ptr)->TYPE == type) {
         /* Found an existing entry */
         ENET_unlock_context(enet_ptr);
         return ENETERR_OPEN_PROT;
      } 
   } 

   // No existing entry found -- create a new one
   ecb_ptr = (ENET_ECB_STRUCT_PTR)_mem_alloc_system_zero(sizeof(ENET_ECB_STRUCT));
   if (!ecb_ptr) {
      ENET_unlock_context(enet_ptr);
      return ENETERR_ALLOC_ECB;
   } 
   _mem_set_type(ecb_ptr, MEM_TYPE_IO_ENET_ECB);
   
   ecb_ptr->TYPE     = type;
   ecb_ptr->SERVICE  = service;
   ecb_ptr->MCB_HEAD = NULL;
   ecb_ptr->PRIVATE  = prv;
   ecb_ptr->NEXT     = NULL;
   *search_ptr       = ecb_ptr;

   ENET_unlock_context(enet_ptr);
   return ENET_OK;

}


/* EOF */
