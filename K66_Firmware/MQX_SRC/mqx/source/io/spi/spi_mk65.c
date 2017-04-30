/*HEADER**********************************************************************
*
* Copyright 2012 Freescale Semiconductor, Inc.
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
*   This file contains board-specific SPI initialization functions.
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>
#include "spi_dspi.h"


static const void *dspi_address[] =
{
    (void *)SPI0_BASE_PTR,
    (void *)SPI1_BASE_PTR,
    (void *)SPI2_BASE_PTR
};


static const uint32_t /*PSP_INTERRUPT_TABLE_INDEX*/ dspi_vectors[][1] =
{
    { INT_SPI0 },
    { INT_SPI1 },
    { INT_SPI2 },
};


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_get_dspi_base_address
* Returned Value   : Address upon success, NULL upon failure
* Comments         :
*    This function returns the base register address of the corresponding SPI
*    module.
*
*END*----------------------------------------------------------------------*/

void *_bsp_get_dspi_base_address(uint8_t dev_num)
{
    if (dev_num < ELEMENTS_OF(dspi_address)) {
        return (void *)dspi_address[dev_num];
    }
    return NULL;
}


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_get_dspi_vectors
* Returned Value   : Number of vectors associated with the peripheral
* Comments         :
*    This function returns desired interrupt vector table indices for specified
*    SPI module.
*
*END*----------------------------------------------------------------------*/

uint32_t _bsp_get_dspi_vectors(uint32_t dev_num, const uint32_t  **vectors_ptr)
{
    if (dev_num < ELEMENTS_OF(dspi_vectors)) {
        *vectors_ptr = dspi_vectors[dev_num];
        return ELEMENTS_OF(dspi_vectors[0]);
    }
    return 0;
}
