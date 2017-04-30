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
* See license agreement file for full license terms including other
* restrictions.
*****************************************************************************
*
* Comments:
*
*   This file contains the settings for ADC1 converter
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>

/*
** parameters of PDB initialization
*/
const KPDB_INIT_STRUCT _bsp_pdb_init = {
    /* PDB interrupt vector */
    INT_PDB0,
    /* PDB interrupt vector */
    BSP_PDB_VECTOR_PRIORITY,
     /* PDB interrupt vector */
};

/*
** parameters of ADC initialization
*/

const KADC_INIT_STRUCT _bsp_adc0_init = {
    /* The number of ADC peripheral, use adc_t enum from PSP */
    0,
    /* The clock source, selects the best from BUSCLK and BUSCLK/2 */
    ADC_CLK_BUSCLK_ANY,
    /* The clock divisor for ADC. use the fastest one */
    ADC_DIV_ANY,
    /* ADC high speed control, see ADC_HSC enum */
    ADC_HSC_NORMAL,
    /* ADC low power control, see ADC_LPC enum */
    ADC_LPC_NORMAL,
    /* The calibration data pointer */
    NULL,
    /* ADC interrupt vector */
    INT_ADC0,
    /* ADC interrupt vector */
    BSP_ADC0_VECTOR_PRIORITY,
    /* PDB init structure */
    &_bsp_pdb_init
};

/*
** parameters of ADC initialization
*/
const KADC_INIT_STRUCT _bsp_adc1_init = {
    /* The number of ADC peripheral, use adc_t enum from PSP */
    1,
    /* The clock source, selects the best from BUSCLK and BUSCLK/2 */
    ADC_CLK_BUSCLK_ANY,
    /* The clock divisor for ADC. use the fastest one */
    ADC_DIV_ANY,
    /* ADC high speed control, see ADC_HSC enum */
    ADC_HSC_NORMAL,
    /* ADC low power control, see ADC_LPC enum */
    ADC_LPC_NORMAL,
    /* The calibration data pointer */
    NULL,
    /* ADC interrupt vector */
    INT_ADC1,
    /* ADC interrupt vector */
    BSP_ADC1_VECTOR_PRIORITY,
    /* PDB init structure */
    &_bsp_pdb_init
};

/* EOF */
