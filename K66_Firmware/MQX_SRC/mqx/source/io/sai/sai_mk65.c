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
*   This file contains board-specific SAI initialization functions.
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>
#include "sai.h"
#include "sai_ksai.h"
#include "sai_ksai_prv.h"

/*FUNCTION****************************************************************
*
* Function Name    : _bsp_get_sai_base_address
* Returned Value   : address if successful, NULL otherwise
* Comments         :
*    This function returns the base register address of the corresponding SAI device.
*
*END*********************************************************************/

void *_bsp_get_sai_base_address
(
    uint8_t dev_num
)
{
    void   *addr;

    switch (dev_num)
    {
        case 0:
            addr = (void *)I2S0_BASE_PTR;
            break;
        default:
            addr = NULL;
            break;
    }
    
    return addr;
}

/*FUNCTION****************************************************************
*
* Function Name    : _bsp_get_sai_tx_vector
* Returned Value   : vector number if successful, 0 otherwise
* Comments         :
*    This function returns desired interrupt tx vector number for specified SAI device.
*
*END*********************************************************************/

uint32_t _bsp_get_sai_tx_vector
(
    uint8_t dev_num
)
{
    uint32_t vector;
    
    switch (dev_num)
    {
        case 0:
            vector = INT_I2S0_Tx;
            break;
        default:
            vector = 0;
            break;
    }
    
    return vector;
}

/*FUNCTION****************************************************************
*
* Function Name    : _bsp_get_sai_rx_vector
* Returned Value   : vector number if successful, 0 otherwise
* Comments         :
*    This function returns desired interrupt rx vector number for specified SAI device.
*
*END*********************************************************************/

uint32_t _bsp_get_sai_rx_vector
(
    uint8_t dev_num
)
{
    uint32_t vector;
    
    switch (dev_num)
    {
        case 0:
            vector = INT_I2S0_Rx;
            break;
        default:
            vector = 0;
            break;
    }
    
    return vector;
}

/*FUNCTION****************************************************************
*
* Function Name    : _bsp_get_sai_tx_dma_source
* Returned Value   : dma request source number if successful, 0 otherwise
* Comments         :
*    This function returns dma request source number for specified SAI device.
*
*END*********************************************************************/
uint32_t _bsp_get_sai_tx_dma_source(uint8_t dev_num)
{
    uint32_t source = 0;
    switch (dev_num)
    {
        case 0:
            source = 13;
            break;
        default:
            source = 0;
            break;
    }
    return source;
}

/*FUNCTION****************************************************************
*
* Function Name    : _bsp_get_sai_rx_dma_source
* Returned Value   : dma request source number if successful, 0 otherwise
* Comments         :
*    This function returns dma request source number for specified SAI device.
*
*END*********************************************************************/
uint32_t _bsp_get_sai_rx_dma_source(uint8_t dev_num)
{
    uint32_t source = 0;
    switch (dev_num)
    {
        case 0:
            source = 12;
            break;
        default:
            source = 0;
            break;
    }
    return source;
}

uint8_t _bsp_get_sai_dma_channel_mask(uint8_t dev_num)
{
    uint32_t mask = 0;
    switch (dev_num)
    {
        case 0:
            mask = 1;
            break;
        default:
            mask = 0;
            break;
    }
    return mask;

}


/* EOF */
