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
*   This file contains the definition for the lwadc devices
*
*
*
*END************************************************************************/

#include "mqx.h"
#include "bsp.h"
#include "bsp_prv.h"

const LWADC_INIT_STRUCT lwadc0_init = {
    /* The number of ADC peripheral, use adc_t enum from PSP */
    0,
    /* The clock source, selects the best from BUSCLK and BUSCLK/2 */
    LWADC_CLK_BUSCLK_ANY,
    /* The clock divisor for ADC. use the fastest one */
    LWADC_DIV_ANY,
    /* ADC high speed control, see ADC_HSC enum */
    LWADC_HSC_NORMAL,
    /* ADC low power control, see ADC_LPC enum */
    LWADC_LPC_NORMAL,
    /* The calibration data pointer */
    NULL,
    /* ADC interrupt vector */
    INT_ADC0,
    /* ADC interrupt vector */
    BSP_ADC0_VECTOR_PRIORITY,
    
    BSP_ADC_VREF_DEFAULT
};

const LWADC_INIT_STRUCT lwadc1_init = {
    /* The number of ADC peripheral, use adc_t enum from PSP */
    1,
    /* The clock source, selects the best from BUSCLK and BUSCLK/2 */
    LWADC_CLK_BUSCLK_ANY,
    /* The clock divisor for ADC. use the fastest one */
    LWADC_DIV_ANY,
    /* ADC high speed control, see ADC_HSC enum */
    LWADC_HSC_NORMAL,
    /* ADC low power control, see ADC_LPC enum */
    LWADC_LPC_NORMAL,
    /* The calibration data pointer */
    NULL,
    /* ADC interrupt vector */
    INT_ADC1,
    /* ADC interrupt vector */
    BSP_ADC1_VECTOR_PRIORITY,
    
    BSP_ADC_VREF_DEFAULT
};

/* EOF */
