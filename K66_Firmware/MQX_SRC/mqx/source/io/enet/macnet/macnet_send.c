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
*   This file contains the MACNET send
*   support function.
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
*  Function Name  : MACNET_send
*  Returned Value : ENET_OK or error code
*  Comments       :
*        Sends a packet.
*
*END*-----------------------------------------------------------------*/

uint32_t MACNET_send
   (
      ENET_CONTEXT_STRUCT_PTR  enet_ptr,
         /* [IN] the Ethernet state structure */
      PCB_PTR              packet,
         /* [IN] the packet to send */
      uint32_t              size,
         /* [IN] total size of the packet */
      uint32_t              frags,
         /* [IN] total fragments in the packet */
      uint32_t              flags
         /* [IN] optional flags, zero = default */
   )
{ 
   MACNET_CONTEXT_STRUCT_PTR   macnet_context_ptr = (MACNET_CONTEXT_STRUCT_PTR) enet_ptr->MAC_CONTEXT_PTR;
   ENET_MemMapPtr              macnet_ptr= macnet_context_ptr->MACNET_ADDRESS;
   PCB_FRAGMENT_PTR            frag_ptr;
   VENET_BD_STRUCT_PTR         tx_bd_ptr;
   uint32_t                     len,totlen,frag;
   unsigned char                   *txmem;
   bool                     aligned;
   uint32_t                     err = ENET_OK;
#if ENETCFG_SUPPORT_PTP
    MACNET_PTP_TS_DATA          tmp_tx_time;
    bool                    wait_for_ts=FALSE;
    uint32_t                     counter=0,control;
#endif

   if (macnet_ptr == NULL) 
      return ENETERR_INVALID_DEVICE;
   
   MACNET_int_disable();
   
   /*
   ** Make sure there aren't too many fragments.  (We really should check
   ** this every time through the previous loop, but it is extremely
   ** unlikely that the fragment counter overflowed -- there would have
   ** to be over 2^32 fragments.)
   */
   if (macnet_context_ptr->AvailableTxBDs < 1) {
      ENET_INC_STATS(COMMON.ST_TX_MISSED);
   
      err = ENETERR_SEND_FULL;      
      goto END;
   } 

   #if BSPCFG_ENABLE_ENET_HISTOGRAM
   {
      uint32_t index = size>> ENET_HISTOGRAM_SHIFT;
      
      if (index < ENET_HISTOGRAM_ENTRIES) {
         ENET_INC_STATS(TX_HISTOGRAM[index]);
      }
   }
   #endif
   
   aligned = TRUE;
   for (frag_ptr = packet->FRAG; frag_ptr->LENGTH; frag_ptr++) {
      if (MACNET_TX_ALIGN((uint32_t)frag_ptr->FRAGMENT)!= (uint32_t)frag_ptr->FRAGMENT)
         aligned = FALSE;
   } 
   if (aligned) {
      ENET_INC_STATS(TX_ALL_ALIGNED);
   }
   
   /*
   ** Enqueue the packet on the transmit ring.  Don't set the ready
   ** bit in the first descriptor until all descriptors are enqueued.
   */
   tx_bd_ptr = &macnet_context_ptr->MACNET_TX_RING_PTR[macnet_context_ptr->NextTxBD];

   frag_ptr = packet->FRAG;
   frag = (uint32_t) frag_ptr->FRAGMENT;
   if (frags > 1 || (MACNET_TX_ALIGN(frag)!= frag)) {
      // Packet is fragmented and/or it is misaligned, needs to be copied
      txmem = NULL;
      // See if it fits in a small buffer
      if (size <= MACNET_SMALL_PACKET_SIZE) {
         // it does
         txmem = ENET_Dequeue_Buffer((void **) &macnet_context_ptr->SMALL_BUFFERS);
      }
      // If it didn't fit, or the small alloc failed, try for a large buffer
      if (txmem) {
         // signal buffer is to be deallocated.
         macnet_context_ptr->FREE_TX_SMALL |= (1<<macnet_context_ptr->NextTxBD);
         ENET_INC_STATS(ST_TX_COPY_SMALL);

      } else { 
         if (size <=  macnet_context_ptr->AlignedTxBufferSize) {

            txmem = ENET_Dequeue_Buffer((void **) &macnet_context_ptr->TX_BUFFERS);
         }
         if (txmem) {
            // signal buffer is to be deallocated.
            macnet_context_ptr->FREE_TX |= (1<<macnet_context_ptr->NextTxBD);
         } else {
            ENET_INC_STATS(COMMON.ST_TX_MISSED);
            
            err = ENETERR_NO_TX_BUFFER;      
            goto END;
         }
         
      }
      totlen = 0;
      
      for (len = frag_ptr->LENGTH; len != 0; len = frag_ptr->LENGTH) {
         _mem_copy(frag_ptr->FRAGMENT, txmem + totlen, len);
         totlen += len;
         frag_ptr++;
      } 

   } else {
      // Packet is not fragmented and it is not misaligned
      totlen = frag_ptr->LENGTH;
      txmem  = frag_ptr->FRAGMENT;
      ENET_INC_STATS(TX_ALIGNED);
   } 

   // Flush the buffer from cache
   _DCACHE_FLUSH_MBYTES(txmem, totlen);  

   // Invalidate the bd from cache
   _DCACHE_INVALIDATE_MBYTES((void *)tx_bd_ptr, sizeof(ENET_BD_STRUCT));  

   // set up the tx bd
   tx_bd_ptr->CONTROL &= HOST_TO_BE_SHORT_CONST(ENET_BD_ETHER_TX_WRAP);
   tx_bd_ptr->BUFFER = (unsigned char *)HOST_TO_BE_LONG((uint32_t)txmem);
   tx_bd_ptr->LENGTH = HOST_TO_BE_SHORT(totlen);
   tx_bd_ptr->CONTROL |= HOST_TO_BE_SHORT_CONST(ENET_BD_ETHER_TX_LAST | ENET_BD_ETHER_TX_SEND_CRC | ENET_BD_ETHER_TX_READY);
   
   tx_bd_ptr->CONTROL_EXT0 |= HOST_TO_BE_SHORT_CONST(ENET_BD_EXT0_ETHER_TX_GENERATE_INTR);
     
   
#if ENETCFG_SUPPORT_PTP
    if (macnet_context_ptr->PTP_PRIV->PTIMER_PRESENT) 
    {
        wait_for_ts = MACNET_ptp_parse(txmem,&tmp_tx_time);
        if(wait_for_ts){
            tx_bd_ptr->CONTROL_EXT0 |= HOST_TO_BE_SHORT_CONST(ENET_BD_EXT0_ETHER_TX_TIMESTAMP);
        }
    }
#endif /* ENETCFG_SUPPORT_PTP */

   // Flush the tx bd from cache
   _DCACHE_FLUSH_MBYTES((void *)tx_bd_ptr, sizeof(ENET_BD_STRUCT));  

   macnet_context_ptr->TxPCBS_PTR[macnet_context_ptr->NextTxBD] = packet;
   
   macnet_context_ptr->AvailableTxBDs--;
   BD_INC(macnet_context_ptr->NextTxBD,macnet_context_ptr->NumTxBDs);

   macnet_ptr->TDAR = ENET_TDAR_TDAR_MASK;
   
END:
   MACNET_int_enable();
      
#if ENETCFG_SUPPORT_PTP
   if(wait_for_ts)
   {

        /* Wait for the TS_AVAIL flag indicating that TX timestamping was done
           and TX timestamp is available in BD */
        while ((!(macnet_ptr->EIR & ENET_EIR_TS_AVAIL_MASK)) && counter < 1000 ) {counter++;}
      
        err = MACNET_ptp_store_txstamp(enet_ptr);
        /* Clear the TS flag from the BD */
        tx_bd_ptr->CONTROL_EXT0 &= HOST_TO_BE_SHORT_CONST(~ENET_BD_EXT0_ETHER_TX_TIMESTAMP);
        /* Only store the tx timestamp when we get the valid timestamp*/
        if(err == ENET_OK){      
            tmp_tx_time.TS.NSEC = macnet_context_ptr->PTP_PRIV->TXSTAMP.NSEC;
            tmp_tx_time.TS.SEC  = macnet_context_ptr->PTP_PRIV->TXSTAMP.SEC;
            MACNET_ptp_insert(&(macnet_context_ptr->PTP_PRIV->TX_TIME), &tmp_tx_time);
        }
        wait_for_ts = FALSE;   
   }

#endif /* ENETCFG_SUPPORT_PTP */
   return err;
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_process_tx_bds
*  Returned Value : void
*  Comments       :
*        Processes transmitted packets.
*
*END*-----------------------------------------------------------------*/

static void MACNET_process_tx_bds
   (
         /* [IN] the Ethernet state structure */
      ENET_CONTEXT_STRUCT_PTR  enet_ptr
   )
{ 
   MACNET_CONTEXT_STRUCT_PTR   macnet_context_ptr = (MACNET_CONTEXT_STRUCT_PTR) enet_ptr->MAC_CONTEXT_PTR;
   uint16_t                     tx_status;
   

   /* Dequeue all transmitted frames */
   while (macnet_context_ptr->AvailableTxBDs < macnet_context_ptr->NumTxBDs) {
      VENET_BD_STRUCT_PTR bd_ptr = &macnet_context_ptr->MACNET_TX_RING_PTR[macnet_context_ptr->LastTxBD];

      _DCACHE_INVALIDATE_MBYTES((void *)bd_ptr, sizeof(ENET_BD_STRUCT));  

      if (bd_ptr->CONTROL & HOST_TO_BE_SHORT_CONST(ENET_BD_ETHER_TX_READY)) {
         break;
      } /* Endif */

      macnet_context_ptr->TxErrors |= SHORT_BE_TO_HOST(bd_ptr->CONTROL);
      tx_status = macnet_context_ptr->TxErrors;

      /* Record statistics for each frame (not each buffer) */
      if (tx_status & ENET_BD_ETHER_TX_LAST) {
         PCB_PTR pcb_ptr;
         macnet_context_ptr->TxErrors = 0;

         ENET_INC_STATS(COMMON.ST_TX_TOTAL);

         pcb_ptr = macnet_context_ptr->TxPCBS_PTR[macnet_context_ptr->LastTxBD];
         PCB_free(pcb_ptr);
      } 

      if (macnet_context_ptr->FREE_TX_SMALL & (1<<macnet_context_ptr->LastTxBD)) {
         ENET_Enqueue_Buffer((void **) &macnet_context_ptr->SMALL_BUFFERS, (void *)LONG_BE_TO_HOST((uint32_t)(bd_ptr->BUFFER)));
         macnet_context_ptr->FREE_TX_SMALL &= ~(1<<macnet_context_ptr->LastTxBD);
      } else if (macnet_context_ptr->FREE_TX & (1<<macnet_context_ptr->LastTxBD)) {
         ENET_Enqueue_Buffer((void **) &macnet_context_ptr->TX_BUFFERS, (void *)LONG_BE_TO_HOST((uint32_t)(bd_ptr->BUFFER)));
         macnet_context_ptr->FREE_TX &= ~(1<<macnet_context_ptr->LastTxBD);
      }      
      BD_INC(macnet_context_ptr->LastTxBD,macnet_context_ptr->NumTxBDs);
      macnet_context_ptr->AvailableTxBDs++;

   } 
} 


/*NOTIFIER*-------------------------------------------------------------
*
*  Function Name  : MACNET_TX_ISR
*  Returned Value : void
*  Comments       :
*        Interrupt service routine for Tx.
*
*END*-----------------------------------------------------------------*/

void MACNET_TX_ISR 
   (
         /* [IN] the Ethernet state structure */
      void    *enet
   )
{    
   ENET_CONTEXT_STRUCT_PTR    enet_ptr           = (ENET_CONTEXT_STRUCT_PTR)enet;
   MACNET_CONTEXT_STRUCT_PTR  macnet_context_ptr = (MACNET_CONTEXT_STRUCT_PTR) enet_ptr->MAC_CONTEXT_PTR;
   ENET_MemMapPtr             macnet_ptr         = macnet_context_ptr->MACNET_ADDRESS;
   uint32_t                    events;

   if (macnet_ptr == NULL) return;
   
   events = macnet_ptr->EIR;
   while (events & (ENET_EIR_TXB_MASK | ENET_EIR_TXF_MASK)) {

      /* clear the interrupt */
      macnet_ptr->EIR = (ENET_EIR_TXF_MASK | ENET_EIR_TXB_MASK);
      MACNET_process_tx_bds(enet_ptr);

      events = macnet_ptr->EIR;
   } 
    
   /* Needed for Kinetis: otherwise last packet in ring buffer may be not sent.*/
   macnet_ptr->TDAR = ENET_TDAR_TDAR_MASK;
} 




/* EOF */
