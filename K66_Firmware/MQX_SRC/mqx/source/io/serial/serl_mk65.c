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
*   This file contains board-specific serial initialization functions.
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>
#include "serl_kuart.h"

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_get_serial_base_address
* Returned Value   : Address upon success, NULL upon failure
* Comments         :
*    This function returns the base register address of the corresponding UART
*
*END*----------------------------------------------------------------------*/
void *_bsp_get_serial_base_address(uint8_t dev_num)
{
    void   *addr;
    switch(dev_num)
    {
        case 0:
            addr = (void *)UART0_BASE_PTR;
            break;
        case 1:
            addr = (void *)UART1_BASE_PTR;
            break;
        case 2:
            addr = (void *)UART2_BASE_PTR;
            break;
        case 3:
            addr = (void *)UART3_BASE_PTR;
            break;
        case 4:
            addr = (void *)UART4_BASE_PTR;
            break;
        default:
            addr = 0;
    }
    return addr;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_get_serial_error_int_number
* Returned Value   : number upon success, 0 upon failure
* Comments         :
*    This function returns the error interrupt number of the corresponding UART
*
*END*----------------------------------------------------------------------*/
uint32_t _bsp_get_serial_error_int_num(uint8_t dev_num)
{
    uint32_t int_num;
    switch(dev_num)
    {
        case 0:
            int_num = INT_UART0_ERR;
            break;
        case 1:
            int_num = INT_UART1_ERR;
            break;
        case 2:
            int_num = INT_UART2_ERR;
            break;
        case 3:
            int_num = INT_UART3_ERR;
            break;
        case 4:
            int_num = INT_UART4_ERR;
            break;
        default:
            int_num = 0;
    }
    return int_num;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_get_serial_tx_dma_number
* Returned Value   : number upon success, 0 upon failure
* Comments         :
*    This function returns the dma request source number of the corresponding UART
*
*END*----------------------------------------------------------------------*/
uint32_t _bsp_get_serial_tx_dma_request(uint8_t dev_num)
{
    uint32_t dma_num;
    switch(dev_num)
    {
        case 0:
            dma_num = 3;
            break;
        case 1:
            dma_num = 5;
            break;
        case 2:
            dma_num = 7;
            break;
        case 3:
            dma_num = 9;
            break;
        case 4:
            dma_num = 11;
            break; 
        default:
            dma_num = 0xFF;
    }
    return dma_num;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_get_serial_rx_dma_number
* Returned Value   : number upon success, 0 upon failure
* Comments         :
*    This function returns the dma request source number of the corresponding UART
*
*END*----------------------------------------------------------------------*/
uint32_t _bsp_get_serial_rx_dma_request(uint8_t dev_num)
{
    uint32_t dma_num;
    switch(dev_num)
    {
        case 0:
            dma_num = 2;
            break;
        case 1:
            dma_num = 4;
            break;
        case 2:
            dma_num = 6;
            break;
        case 3:
            dma_num = 8;
            break;
        case 4:
            dma_num = 10;
            break; 
        default:
            dma_num = 0xFF;
    }
    return dma_num;
}
