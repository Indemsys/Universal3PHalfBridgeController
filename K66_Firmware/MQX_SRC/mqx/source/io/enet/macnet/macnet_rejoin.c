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
*   This file contains the MACNET multicasting
*   interface functions.
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
*  Function Name  : MACNET_rejoin
*  Returned Value : void
*  Comments       :
*        Rejoins all joined multicast groups.  Called by ENET_close
*        and ENET_leave.
*
*END*-----------------------------------------------------------------*/

uint32_t MACNET_rejoin
   (
      ENET_CONTEXT_STRUCT_PTR  enet_ptr
         /* [IN] the Ethernet state structure */
   )
{ 
   MACNET_CONTEXT_STRUCT_PTR  macnet_context_ptr = (MACNET_CONTEXT_STRUCT_PTR) enet_ptr->MAC_CONTEXT_PTR;
   ENET_MemMapPtr             macnet_ptr= macnet_context_ptr->MACNET_ADDRESS;
   ENET_ECB_STRUCT_PTR        ecb_ptr;
   ENET_MCB_STRUCT_PTR        mcb_ptr;
   uint32_t                    ialr = 0, iaur = 0;

   if (macnet_ptr == NULL)  return ENETERR_INVALID_DEVICE;

   /*
   ** Add the remaining multicast groups to the group address filter
   */
   for (ecb_ptr = enet_ptr->ECB_HEAD;
        ecb_ptr;
        ecb_ptr = ecb_ptr->NEXT) {

      for (mcb_ptr = ecb_ptr->MCB_HEAD;
           mcb_ptr;
           mcb_ptr = mcb_ptr->NEXT) {

         uint32_t crc = mcb_ptr->HASH;

         if (crc < 32)  {
            ialr |= 1 << (crc & 0x1F);
         } else {
            iaur |= 1 << (crc & 0x1F);
         } 
      } 
   } 

   macnet_ptr->GALR = ialr;
   macnet_ptr->GAUR = iaur;

   return ENET_OK;
} 

/* EOF */
