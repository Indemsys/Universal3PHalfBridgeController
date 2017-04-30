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
*   This file contains the Ethernet receive
*   support function.
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>

#include "enet.h"
#include "enetprv.h"

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_find_receiver
*  Returned Value : ECB for received packet, or NULL.
*  Comments       :
*        Finds an application for a received packet.
*
*END*-----------------------------------------------------------------*/

ENET_ECB_STRUCT_PTR ENET_find_receiver
   (
      /* [IN] the Ethernet state structure */
      ENET_CONTEXT_STRUCT_PTR  enet_ptr,

      /* [IN] the received packet */
      ENET_HEADER_PTR      packet_ptr,
      
      uint32_t          *length_ptr
   )
{ 
   ENET_ECB_STRUCT_PTR  ecb_ptr;
   ENET_MCB_STRUCT_PTR  mcb_ptr;
   unsigned char            *type_ptr;
   uint32_t              hdrlen;
   _enet_address        dest;
   uint16_t              type;


   hdrlen = sizeof(ENET_HEADER);
   type_ptr = packet_ptr->TYPE;
   type = mqx_ntohs(type_ptr);

   if (type == ENETPROT_8021Q) {
      ENET_8021QTAG_HEADER_PTR tag_ptr = (ENET_8021QTAG_HEADER_PTR)(type_ptr+2);
      hdrlen += sizeof(ENET_8021QTAG_HEADER);
      type_ptr = tag_ptr->TYPE;
      type = mqx_ntohs(type_ptr);
   } 

   if (type <= ENETPROT_LENGTH_TYPE_BOUNDARY) {               
      ENET_8022_HEADER_PTR llc_ptr = (ENET_8022_HEADER_PTR)(type_ptr+2);
      if ((mqx_ntohc(llc_ptr->DSAP) != 0xAA)
       || (mqx_ntohc(llc_ptr->SSAP) != 0xAA)) {
         return NULL;
      } 
      if (*length_ptr < hdrlen + type) {
         return NULL;
      } 
      *length_ptr = hdrlen + type;
      type_ptr = llc_ptr->TYPE;
      type = mqx_ntohs(type_ptr);
   } 

   for (ecb_ptr = enet_ptr->ECB_HEAD; ecb_ptr; ecb_ptr = ecb_ptr->NEXT) {
      if (ecb_ptr->TYPE == type) {

         ntohe(packet_ptr->DEST, dest);
         if ((dest[0] & 1) && !((dest[0] == 0xFF)
                             && (dest[1] == 0xFF)
                             && (dest[2] == 0xFF)
                             && (dest[3] == 0xFF)
                             && (dest[4] == 0xFF)
                             && (dest[5] == 0xFF))) {

            /*
            ** The destination is a multicast address.
            ** Check the joined mulicast groups.
            */
            for (mcb_ptr = ecb_ptr->MCB_HEAD; mcb_ptr; mcb_ptr = mcb_ptr->NEXT) {
               if ((dest[0] == mcb_ptr->GROUP[0])
                && (dest[1] == mcb_ptr->GROUP[1])
                && (dest[2] == mcb_ptr->GROUP[2])
                && (dest[3] == mcb_ptr->GROUP[3])
                && (dest[4] == mcb_ptr->GROUP[4])
                && (dest[5] == mcb_ptr->GROUP[5])) {
                  break;
               } 
            } 

            if (!mcb_ptr) {
               /*
               ** We received a packet multicasted to a group we
               ** haven't joined.  Break out of the big for loop
               ** and discard the packet.  We don't continue the
               ** big for loop because there is only one ECB per
               ** type and we already found it.
               */
               ecb_ptr = NULL;
            } 
         } 

         break;
      } 
   } 

   return ecb_ptr;

} 


/* EOF */
