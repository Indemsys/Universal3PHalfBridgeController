/*HEADER**********************************************************************
*
* Copyright 2013 Freescale Semiconductor, Inc.
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
*   This file contains CPU specific eDMA functions.
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>
#include "edma.h"


static const uint32_t /*PSP_INTERRUPT_TABLE_INDEX*/ edma_done_vectors[][16] =
{
    { 
        INT_DMA0_DMA16,
        INT_DMA1_DMA17,
        INT_DMA2_DMA18,
        INT_DMA3_DMA19,
        INT_DMA4_DMA20,
        INT_DMA5_DMA21,
        INT_DMA6_DMA22,
        INT_DMA7_DMA23,
        INT_DMA8_DMA24,
        INT_DMA9_DMA25,
        INT_DMA10_DMA26,
        INT_DMA11_DMA27,
        INT_DMA12_DMA28,
        INT_DMA13_DMA29,
        INT_DMA14_DMA30,
        INT_DMA15_DMA31
    }
};



static const uint32_t /*PSP_INTERRUPT_TABLE_INDEX*/ edma_error_vectors[][1] =
{
    { 
        INT_DMA_Error
    }
};



/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_get_edma_done_vectors
* Returned Value   : Number of vectors associated with the peripheral
* Comments         :
*    This function returns desired interrupt vector table indices for specified module.
*
*END*----------------------------------------------------------------------*/

int _bsp_get_edma_done_vectors(uint32_t dev_num, const uint32_t  **vectors_ptr)
{
    if (dev_num < ELEMENTS_OF(edma_done_vectors)) {
        *vectors_ptr = edma_done_vectors[dev_num];
        return ELEMENTS_OF(edma_done_vectors[0]);
    }
    return 0;
}


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_get_edma_error_vector
* Returned Value   : Number of vectors associated with the peripheral
* Comments         :
*    This function returns desired interrupt vector table indices for specified module.
*
*END*----------------------------------------------------------------------*/

int _bsp_get_edma_error_vectors(uint32_t dev_num, const uint32_t  **vectors_ptr)
{
    if (dev_num < ELEMENTS_OF(edma_error_vectors)) {
        *vectors_ptr = edma_error_vectors[dev_num];
        return ELEMENTS_OF(edma_error_vectors[0]);
    }
    return 0;
}


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_edma_enable
* Returned Value   :
* Comments         :
*    The function performs necessary operations to enable eDMA module
*
*END*----------------------------------------------------------------------*/

int _bsp_edma_enable(uint32_t dev_num)
{
    /* Enable DMAMUX clock */
    SIM_SCGC6 |= SIM_SCGC6_DMAMUX_MASK;

    /* Enable DMA clock */
    SIM_SCGC7 |= SIM_SCGC7_DMA_MASK;

    return 0;
}
