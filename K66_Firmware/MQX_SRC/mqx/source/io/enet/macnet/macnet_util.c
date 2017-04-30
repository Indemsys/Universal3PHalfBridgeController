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
*   This file contains the MACNET util functions.
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
*  Function Name  : MACNET_install_isr
*  Returned Value : ENET_OK or error code
*  Comments       :
*         
*
*END*-----------------------------------------------------------------*/

bool MACNET_install_isr( 
   ENET_CONTEXT_STRUCT_PTR enet_ptr, 
   uint32_t                 int_num, 
   uint32_t                 int_index, 
   INT_ISR_FPTR            isr, 
   uint32_t                 level, 
   uint32_t                 sublevel  ) 
{
   uint32_t  vector = MACNET_get_vector(enet_ptr->PARAM_PTR->ENET_IF->MAC_NUMBER, int_index);
 
#if BSPCFG_ENET_RESTORE
   MACNET_CONTEXT_STRUCT_PTR    macnet_context_ptr = (MACNET_CONTEXT_STRUCT_PTR) enet_ptr->MAC_CONTEXT_PTR;


   /* Save old ISR and data */
   macnet_context_ptr->OLDISR_PTR[int_num]   = _int_get_isr(vector);
   macnet_context_ptr->OLDISR_DATA[int_num] = _int_get_isr_data(vector);
#endif

   if (_int_install_isr(vector, isr, (void *)enet_ptr)==NULL) {
      return FALSE;
   }

   /* Initialize interrupt priority and level */
   _bsp_int_init((PSP_INTERRUPT_TABLE_INDEX)vector, level, sublevel, TRUE);

   return TRUE;
}


#if PSP_MQX_CPU_IS_VYBRID
/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_RX_TX_ISR
*  Returned Value : none
*  Comments       : function for macnet interrupt servicing on Vybrid 
*                   platform
*
*END*-----------------------------------------------------------------*/
void MACNET_RX_TX_ISR 
   (
      /* [IN] the Ethernet state structure */
      void    *enet
   )
{
	ENET_CONTEXT_STRUCT_PTR    enet_ptr           = (ENET_CONTEXT_STRUCT_PTR)enet;
	MACNET_CONTEXT_STRUCT_PTR  macnet_context_ptr = (MACNET_CONTEXT_STRUCT_PTR) enet_ptr->MAC_CONTEXT_PTR;
	ENET_MemMapPtr             macnet_ptr         = macnet_context_ptr->MACNET_ADDRESS;
   /* If receive ISR flag is set and interrupt is unmasked */
	if (macnet_ptr->EIMR & (ENET_EIMR_RXB_MASK | ENET_EIMR_RXF_MASK))
	{
		MACNET_RX_ISR(enet);
	}
   /* If transmit ISR flag is set and interrupt is unmasked */
	if (macnet_ptr->EIMR & (ENET_EIMR_TXB_MASK | ENET_EIMR_TXF_MASK))
	{
		MACNET_TX_ISR(enet);
	}
	
#if ENETCFG_SUPPORT_PTP
    /*  If ts_timer ISR flag is set and interrupot is unmasked*/
    if(macnet_ptr->EIMR & ENET_EIMR_TS_TIMER_MASK)
    {
        MACNET_ptp_increment_seconds(enet);
    }
#endif
}

#endif // PSP_MQX_CPU_IS_VYBRID

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_install_isrs
*  Returned Value :  
*  Comments       :
*        
*
*END*-----------------------------------------------------------------*/

bool MACNET_install_isrs( ENET_CONTEXT_STRUCT_PTR enet_ptr, MACNET_INIT_STRUCT const * enet_init_ptr  ) 
{
   bool bOK;

#if PSP_MQX_CPU_IS_VYBRID
   bOK = MACNET_install_isr (enet_ptr, 0, ENET_INT_RX_TX, MACNET_RX_TX_ISR, enet_init_ptr->ETX_LEVEL, enet_init_ptr->ETX_SUBLEVEL);
#else
   bOK = MACNET_install_isr(enet_ptr, 0, ENET_INT_TX_INTB, MACNET_TX_ISR, enet_init_ptr->ETX_LEVEL,enet_init_ptr->ETX_SUBLEVEL  ) ;
   
   if (bOK) {
      bOK = MACNET_install_isr(enet_ptr, 1, ENET_INT_TX_INTF, MACNET_TX_ISR, enet_init_ptr->ETX_LEVEL,enet_init_ptr->ETX_SUBLEVEL  ) ;
   }
   if (bOK) {
      bOK = MACNET_install_isr(enet_ptr, 2, ENET_INT_RX_INTB, MACNET_RX_ISR, enet_init_ptr->ERX_LEVEL,enet_init_ptr->ERX_SUBLEVEL  ) ;
   }
   if (bOK) {
      bOK = MACNET_install_isr(enet_ptr, 3, ENET_INT_RX_INTF, MACNET_RX_ISR, enet_init_ptr->ERX_LEVEL,enet_init_ptr->ERX_SUBLEVEL  ) ;
   }
#if ENETCFG_SUPPORT_PTP
   /* Just one MACNET module handles 1588timer and incrementation of seconds */
   if (bOK && (enet_ptr->PARAM_PTR->OPTIONS & ENET_OPTION_PTP_MASTER_CLK)) {
      bOK = MACNET_ptp_install_ts_timer_isr(enet_ptr, enet_init_ptr ) ;
   }

#endif /* ENETCFG_SUPPORT_PTP */
#endif // PSP_MQX_CPU_IS_VYBRID

   return bOK;
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_mask_interrupts
*  Returned Value :  
*  Comments       :
*        
*
*END*-----------------------------------------------------------------*/

void MACNET_mask_interrupts( ENET_CONTEXT_STRUCT_PTR enet_ptr ) 
{
   uint32_t  i;
   
   for (i = 0; i < ENET_NUM_INTS; i++)
   {
      _bsp_int_disable((PSP_INTERRUPT_TABLE_INDEX)(MACNET_get_vector(enet_ptr->PARAM_PTR->ENET_IF->MAC_NUMBER,i)));
   }
}


#if BSPCFG_ENET_RESTORE

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_uninstall_isr
*  Returned Value :  
*  Comments       :
*        
*
*END*-----------------------------------------------------------------*/

static void MACNET_uninstall_isr( ENET_CONTEXT_STRUCT_PTR enet_ptr, uint32_t int_num, uint32_t int_index  ) 
{
   uint32_t  vector = MACNET_get_vector(enet_ptr->PARAM_PTR->ENET_IF->MAC_NUMBER, int_index);
   MACNET_CONTEXT_STRUCT_PTR    macnet_context_ptr = (MACNET_CONTEXT_STRUCT_PTR) enet_ptr->MAC_CONTEXT_PTR;
 
   _bsp_int_disable((PSP_INTERRUPT_TABLE_INDEX)vector);
   if (macnet_context_ptr->OLDISR_PTR[int_num]) 
   {
      _int_install_isr(vector, macnet_context_ptr->OLDISR_PTR[int_num], macnet_context_ptr->OLDISR_DATA[int_num]);
      macnet_context_ptr->OLDISR_PTR[int_num] = (INT_ISR_FPTR)NULL;
   }
}


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_uninstall_all_isrs
*  Returned Value :  
*  Comments       :
*        
*
*END*-----------------------------------------------------------------*/

void MACNET_uninstall_all_isrs (ENET_CONTEXT_STRUCT_PTR enet_ptr) 
{
#if PSP_MQX_CPU_IS_VYBRID
   MACNET_uninstall_isr(enet_ptr, 0, ENET_INT_RX_TX);
#else
   MACNET_uninstall_isr(enet_ptr, 0, ENET_INT_TX_INTB);
   MACNET_uninstall_isr(enet_ptr, 1, ENET_INT_TX_INTF);
   MACNET_uninstall_isr(enet_ptr, 2, ENET_INT_RX_INTB);
   MACNET_uninstall_isr(enet_ptr, 3, ENET_INT_RX_INTF);
#endif // PSP_MQX_CPU_IS_VYBRID

}
#endif


/*FUNCTION*-------------------------------------------------------------
* 
* Function Name    : MACNET_get_vector
* Returned Value   : MQX vector number for specified interrupt
* Comments         :
*    This function returns index into MQX interrupt vector table for
*    specified enet interrupt. If not known, returns 0.
*
*END*-----------------------------------------------------------------*/


uint32_t MACNET_get_vector 
(
    uint32_t device,
    uint32_t vector_index
)
{ 
    uint32_t index = 0;

    if ((device < MACNET_DEVICE_COUNT) && (vector_index < ENET_NUM_INTS)) {
      index = MACNET_vectors[device][vector_index];
    }

    return index;
} 


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_free_context
*  Returned Value :  
*  Comments       :
*        
*
*END*-----------------------------------------------------------------*/

void MACNET_free_context( MACNET_CONTEXT_STRUCT_PTR macnet_context_ptr ) 
{
   if (macnet_context_ptr) {
      if (macnet_context_ptr->UNALIGNED_BUFFERS) {
         _mem_free((void *)macnet_context_ptr->UNALIGNED_BUFFERS);
      }
      if (macnet_context_ptr->RX_PCB_BASE) {
         _mem_free((void *)macnet_context_ptr->RX_PCB_BASE);
      }
      if (macnet_context_ptr->TxPCBS_PTR) {
         _mem_free((void *)macnet_context_ptr->TxPCBS_PTR);
      }
      if (macnet_context_ptr->UNALIGNED_RING_PTR) {
         _mem_free((void *)macnet_context_ptr->UNALIGNED_RING_PTR);
      }
   
      _mem_free((void *)macnet_context_ptr);
   }
}


/* EOF */
