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
*   This file contains the MACNET shutdown functions.
*
*
*END************************************************************************/

#include "mqx.h"
#include "bsp.h"
#include "enet.h"
#include "enetprv.h"
#include "macnet_prv.h"          

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_shutdown
*  Returned Value : ENET_OK or error code
*  Comments       :
*        Stops the chip.
*
*END*-----------------------------------------------------------------*/

uint32_t MACNET_shutdown
   (
      ENET_CONTEXT_STRUCT_PTR  enet_ptr
         /* [IN] the Ethernet state structure */
   )
{ /* Body */
   MACNET_CONTEXT_STRUCT_PTR   macnet_context_ptr = (MACNET_CONTEXT_STRUCT_PTR) enet_ptr->MAC_CONTEXT_PTR;
   ENET_MemMapPtr              macnet_ptr= macnet_context_ptr->MACNET_ADDRESS;

   if (macnet_ptr == NULL) return ENETERR_INVALID_DEVICE;

   /* Stop the chip */
   macnet_ptr->ECR = ENET_ECR_RESET_MASK;

   /* Disable all MACNET interrupts */
   macnet_ptr->EIMR = 0;

   /* clear any pending interrpts */
   macnet_ptr->EIR = ENET_EIR_ALL_PENDING;

   /* Stop the MACNET from interrupting the core */
    MACNET_mask_interrupts( enet_ptr );

   #if BSPCFG_ENET_RESTORE
      /* Restore old ISRs */
      MACNET_uninstall_all_isrs( enet_ptr );
   #endif

    /*
    ** Make sure all PCBs are free
    */
    if (macnet_context_ptr->ActiveRxBDs != macnet_context_ptr->NumRxBDs) {
       return ENETERR_FREE_PCB;
    } 

   MACNET_free_context(macnet_context_ptr);
   return ENET_OK;

} 


/* EOF */
