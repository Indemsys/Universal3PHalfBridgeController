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
*   support functions.
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>
#include "enet.h"
#include "enetprv.h"
#include "macnet_prv.h"          

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_join
*  Returned Value : void
*  Comments       :
*        Joins a multicast group on an Ethernet channel.
*
*END*-----------------------------------------------------------------*/

uint32_t MACNET_join
   (
     ENET_CONTEXT_STRUCT_PTR  enet_ptr,
         /* [IN] the Ethernet state structure */
      ENET_MCB_STRUCT_PTR     mcb_ptr
         /* [IN] the multicast control block */
   )
{ 
   MACNET_CONTEXT_STRUCT_PTR  macnet_context_ptr = (MACNET_CONTEXT_STRUCT_PTR) enet_ptr->MAC_CONTEXT_PTR;
   ENET_MemMapPtr             macnet_ptr         = macnet_context_ptr->MACNET_ADDRESS;
   uint32_t                    i,j,crc            = 0xFFFFFFFFL;
   
   if (macnet_ptr == NULL) return ENETERR_INVALID_DEVICE;

   /* Compute the CRC-32 polynomial on the multicast group */
   for (i=0; i<6; i++) {
      unsigned char c = mcb_ptr->GROUP[i];
      for (j=0; j<8; j++) {
         if ((c ^ crc) & 1) {
            crc >>= 1;
            c >>= 1;
            crc ^= 0xEDB88320L;
         } else {
            crc >>= 1;
            c >>= 1;
         }  
      } 
   }  

   /* Set the appropriate bit in the hash table */
   crc >>= 26;
   crc &= 0x3F;
   mcb_ptr->HASH = crc;

   if (crc < 32) {
      macnet_ptr->GALR |= 0x1 << (crc & 0x1F);
   } else {
      macnet_ptr->GAUR |= 0x1 << (crc & 0x1F);
   }  

   return ENET_OK;
} 

/* EOF */
