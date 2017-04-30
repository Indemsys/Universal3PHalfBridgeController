/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
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
*   This file contains board-specific LPT initialization functions.
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>
#include "timer_qpit.h"

PSP_INTERRUPT_TABLE_INDEX _qpit_vectors[] =
{
	INT_PIT0,  
	INT_PIT1,
	INT_PIT2,
	INT_PIT3
};

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_get_qpit_base_address
* Returned Value   : none
* Comments         :
*    Get base address of qpit
*
*END*------------------------------------------------------------------------*/

VQPIT_REG_STRUCT_PTR _bsp_get_qpit_base_address
    (
        /* [IN] PIT index */
    	uint32_t timer  	
    )
{
    if (timer != 0) 
    	return NULL;
    
    return (VQPIT_REG_STRUCT_PTR)PIT_BASE_PTR;	 
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_get_qpit_vector
* Returned Value   : Vector number upon success, 0 upon failure
* Comments         :
*    This function returns desired interrupt vector number for specified PIT
*
*END*----------------------------------------------------------------------*/

PSP_INTERRUPT_TABLE_INDEX _bsp_get_qpit_vector
    (
        uint32_t timer,
        uint32_t channel
    )
{
	if (timer != 0)
		return INT_Initial_Stack_Pointer; //we return in fact 0 to inform that error occured
	
    return (PSP_INTERRUPT_TABLE_INDEX) _qpit_vectors[channel];  
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_qpit_clk_en
* Returned Value   : Address upon success, NULL upon failure
* Comments         :
*    This function enables clock to corresponding MTIM
*
*END*----------------------------------------------------------------------*/
void _bsp_qpit_clk_en (uint32_t timer)
{
    switch (timer) {
        case 0:
        	SIM_SCGC6 |= SIM_SCGC6_PIT_MASK; // Enable PIT Module Clock
          break;
    }
}

