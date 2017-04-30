/*HEADER**********************************************************************
*
* Copyright 2008, 2014 Freescale Semiconductor, Inc.
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
*   This file contains the Ethernet send
*   interface function.
*
*
*END************************************************************************/

#include <mqx_inc.h>
#include <bsp.h>

#include "enet.h"
#include "enetprv.h"

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_send
*  Returned Value : ENET_OK or error code
*  Comments       :
*        Sends a packet.
*
*END*-----------------------------------------------------------------*/

uint32_t ENET_send
   (
      /* [IN] the Ethernet state structure */
      _enet_handle   handle,

      /* [IN] the packet to send */
      PCB_PTR        packet,

      /* [IN] the protocol */
      uint16_t        type,

      /* [IN] the destination Ethernet address */
      _enet_address  dest,

      /* [IN] optional flags, zero = default */
      uint32_t        flags
   )
{ 
   ENET_CONTEXT_STRUCT_PTR  enet_ptr = (ENET_CONTEXT_STRUCT_PTR)handle;
   ENET_HEADER_PTR         packet_ptr;
   unsigned char               *type_ptr;
   PCB_FRAGMENT_PTR        frag_ptr;
   uint32_t                 swhdr, size, frags;
   uint32_t                 error;
   _KLOGM(KERNEL_DATA_STRUCT_PTR kernel_data);
   
   _KLOGM(_GET_KERNEL_DATA(kernel_data));
   _KLOGE6(KLOG_ENET_send, handle, packet, type, dest, flags);

   if (flags & ENET_OPT_8021QTAG) {
       swhdr = ENET_FRAMESIZE_HEAD_VLAN;
   } else {
       swhdr = ENET_FRAMESIZE_HEAD;
   } 

   /*
   ** Make sure the first fragment is long enough for the Ethernet
   ** frame header.  This isn't strictly necessary, but it's impractical
   ** to split a 14-26 byte header over multiple fragments.
   */
#if MQX_CHECK_ERRORS
   if (packet->FRAG[0].LENGTH < swhdr) {
      ENET_INC_STATS(COMMON.ST_TX_DISCARDED);
      error = ENETERR_SEND_SHORT;
      goto ERROR;
   }
#endif

   /*
   ** Make sure that no fragment exceeds a maximum packet length.
   ** We check every fragment because we want to prevent something
   ** like FRAG[0].LENGTH = 2000, FRAG[1].LENGTH = -1000.  This
   ** situation would not be detected if we only check the total
   ** length.
   */
   size = frags = 0;
   for (frag_ptr = packet->FRAG; frag_ptr->LENGTH; frag_ptr++) {
#if MQX_CHECK_ERRORS
      if (frag_ptr->LENGTH > enet_ptr->MaxTxFrameSize) {   
         ENET_INC_STATS(COMMON.ST_TX_DISCARDED);
         error = ENETERR_SEND_LONG;
         goto ERROR;
      } 
#endif
      size += frag_ptr->LENGTH;
      frags++;
   } 

   /*
   ** Make sure that the total sum of the fragments doesn't exceed
   ** a maximum packet length.
   */
#if MQX_CHECK_ERRORS
   if (size > enet_ptr->MaxTxFrameSize) {
      ENET_INC_STATS(COMMON.ST_TX_DISCARDED);
      error = ENETERR_SEND_LONG;
      goto ERROR;
   } 
#endif

   /*
   ** Everything checks out -- fill in the header.
   */
   packet_ptr = (ENET_HEADER_PTR)packet->FRAG[0].FRAGMENT;
   htone(packet_ptr->DEST, dest);
   htone(packet_ptr->SOURCE, enet_ptr->ADDRESS);
   type_ptr = packet_ptr->TYPE;

   if (flags & ENET_OPT_8021QTAG) {
      ENET_8021QTAG_HEADER_PTR tag_ptr = (ENET_8021QTAG_HEADER_PTR)(type_ptr+2);
      uint16_t tag;
      
      /* WIKI: Two bytes are used for the tag protocol identifier (TPID),
       * a 16-bit field set to a value of 0x8100 in order to identify the frame as an IEEE 802.1Q-tagged frame */  
      mqx_htons(type_ptr, ENETPROT_8021Q);
      /* Two bytes of tag control information (TCI). The TCI field is further divided into PCP, DEI, and VID*/
      tag = (ENET_GETOPT_8021QPRIO(flags) << 13) | ENET_GETOPT_8021QVID(flags);  
      mqx_htons(tag_ptr->TAG, tag);
      type_ptr = tag_ptr->TYPE;
   } 

   if (flags & ENET_OPT_8023) {
      ENET_8022_HEADER_PTR llc_ptr = (ENET_8022_HEADER_PTR)(type_ptr+2);
      (void)mqx_htons(type_ptr, size - swhdr);
      mqx_htonc(llc_ptr->DSAP, 0xAA);
      mqx_htonc(llc_ptr->SSAP, 0xAA);
      mqx_htonc(llc_ptr->COMMAND, 0x03);
      mqx_htonc(&llc_ptr->OUI[0], 0x00);
      mqx_htonc(&llc_ptr->OUI[1], 0x00);
      mqx_htonc(&llc_ptr->OUI[2], 0x00);
      type_ptr = llc_ptr->TYPE;
   } 

   mqx_htons(type_ptr, type);

   /*
   ** This function can be called from any context, and it needs mutual
   ** exclusion with itself, and with ENET_ISR().
   */
   ENET_lock_context(enet_ptr);
   while(ENETERR_SEND_FULL == enet_send(enet_ptr, packet, size, frags, flags, &error))
   {
     _sched_yield();
   }   
   ENET_unlock_context(enet_ptr);

   if (error) {
   ERROR:
      PCB_free(packet);
   }
   
   _KLOGX4(KLOG_ENET_send, handle, packet, error);
   
   return error;
}  



/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_send_raw
*  Returned Value : ENET_OK or error code
*  Comments       :
*        Sends a packet.
*
*END*-----------------------------------------------------------------*/

uint32_t ENET_send_raw
   (
      /* [IN] the Ethernet state structure */
      _enet_handle   handle,

      /* [IN] the packet to send */
      PCB_PTR        packet
   )
{ /* Body */
   ENET_CONTEXT_STRUCT_PTR    enet_ptr = (ENET_CONTEXT_STRUCT_PTR)handle;
   PCB_FRAGMENT_PTR           frag_ptr;
   uint32_t                    size, frags;
   uint32_t                    error;

   /*
   ** Make sure that no fragment exceeds a maximum packet length.
   ** We check every fragment because we want to prevent something
   ** like FRAG[0].LENGTH = 2000, FRAG[1].LENGTH = -1000.  This
   ** situation would not be detected if we only check the total
   ** length.
   */
   size = frags = 0;
   for (frag_ptr = packet->FRAG; frag_ptr->LENGTH; frag_ptr++) {
#if MQX_CHECK_ERRORS
      if (frag_ptr->LENGTH > enet_ptr->MaxTxFrameSize) {   
         ENET_INC_STATS(COMMON.ST_TX_DISCARDED);
         error = ENETERR_SEND_LONG;
         goto ERROR;
      } 
#endif
      size += frag_ptr->LENGTH;
      frags++;
   } 

   /*
   ** Make sure that the total sum of the fragments doesn't exceed
   ** a maximum packet length.
   */
#if MQX_CHECK_ERRORS
   if (size > enet_ptr->MaxTxFrameSize) {
      ENET_INC_STATS(COMMON.ST_TX_DISCARDED);
      error = ENETERR_SEND_LONG;
      goto ERROR;
   } 
#endif

   /*
   ** This function can be called from any context, and it needs mutual
   ** exclusion with itself, and with ENET_ISR().
   */
   ENET_lock_context(enet_ptr);
   error = (*enet_ptr->PARAM_PTR->ENET_IF->MAC_IF->SEND)(handle, packet, size, frags, 0);
   ENET_unlock_context(enet_ptr);

   if (error) {
   ERROR:
      PCB_free(packet);
   }  

   return error;

}  

/* EOF */
