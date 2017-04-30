
/*HEADER**********************************************************************
*
* Copyright 2010 Freescale Semiconductor, Inc.
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
* See license agreement file for full license terms including other restrictions.
*****************************************************************************
*
* Comments:
*
*   This file contains the function for initializing the handling of
*   interrupts.
*
*
*END************************************************************************/

#include "mqx_inc.h"

/*!
 * \brief This function initializes kernel interrupt tables.
 */
#if MQX_USE_INTERRUPTS

void _psp_int_install(void)
{
    KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);
    __set_MSP((uint32_t)kernel_data->INTERRUPT_STACK_PTR);

#if !MQX_ROM_VECTORS
    {
        extern uint32_t ram_vector[];

        int         i;
        uint32_t   *ptr = (uint32_t *)ram_vector;

        /* Initialize the hardware interrupt vector table
         * 0: Initial stack pointer
         * 1: Initial program counter
         */
        for (i = 16; i < PSP_MAXIMUM_INTERRUPT_VECTORS; i++) {
            ptr[i] = (uint32_t)_int_kernel_isr;
        }
    }
#endif
}

#endif /* MQX_USE_INTERRUPTS */
