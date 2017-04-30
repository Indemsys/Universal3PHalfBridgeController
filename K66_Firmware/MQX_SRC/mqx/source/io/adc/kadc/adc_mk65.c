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
*   CPU specific ADC driver header file
*
*
*END************************************************************************/

#include "mqx.h"
#include "bsp.h"

static const void *adc_address[] = {
    (void *)ADC0_BASE_PTR,
    (void *)ADC1_BASE_PTR
};

/*FUNCTION**********************************************************************
*
* Function Name    : _bsp_get_adc_base_address
* Returned Value   : pointer to base of ADC registers
* Comments         :
*    This function returns base address of ADC related register space.
*
*END***************************************************************************/
void *_bsp_get_adc_base_address(uint32_t device_number)
{
    /* Check if device number is correct */
    if (device_number < ELEMENTS_OF(adc_address)) {
        return (void *)adc_address[device_number];
    }
    return NULL;
}
