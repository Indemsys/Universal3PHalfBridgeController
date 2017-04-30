/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
* Copyright 2011 Embedded Access Inc.
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
*   This file contains the ADC driver CPU specific functions
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>
#include <lwadc_kadc_prv.h>

/*FUNCTION*-------------------------------------------------------------------
 *
 * Function Name    : _lwadc_restart
 * Returned Value   :
 * Comments         :
 * This function restarts the ADC after a parameter change.
 *
 *END*----------------------------------------------------------------------*/

static ADC_CONTEXT adc_context[ADC_NUM_DEVICES] = {0};
uint32_t adc_max_frq;

static void _lwadc_restart(LWADC_STRUCT_PTR lwadc_ptr)
{
    ADC_MemMapPtr   adc_ptr;

    adc_ptr = lwadc_ptr->context_ptr->adc_ptr;
    if (!adc_ptr) {
        return;
    }
    adc_ptr->SC1[0] |= ADC_SC1_AIEN_MASK;

    return;
}

/*FUNCTION*-------------------------------------------------------------------
 *
 * Function Name    : _lwadc_init
 * Returned Value   : TRUE for success, FALSE for failure
 * Comments         :
 *    This function initializes the ADC device with global parameters
 *    (conversion frequency, resolution, number format). It does  not start
 *    any conversions.
 *
 *END*----------------------------------------------------------------------*/

bool _lwadc_init( const LWADC_INIT_STRUCT *  init_ptr)
{
    ADC_MemMapPtr           adc_ptr;
    uint32_t                mcr = LWADC_SC3_AVGS_32 | ADC_SC3_AVGE_MASK | ADC_SC3_ADCO_MASK;
    ADC_CONTEXT_PTR         context_ptr;
    uint8_t                 adc_max_frq_index;

    /* This table is made from datasheet values for ADC module in Kinetis */
    const static uint32_t adc_max_frq_table[] = {
        2500000,  /* 2.5 MHz  for low power,    normal speed, 16b resolution */
        5000000,  /* 5.0 MHz  for low power,    normal speed, lower resolution */
        5000000,  /* 5.0 MHz  for low power,    high speed,   16b resolution */
        8000000,  /* 8.0 MHz  for low power,    high speed,   lower resolution */
        8000000,  /* 8.0 MHz  for normal power, normal speed, 16b resolution */
        12000000, /* 12.0 MHz for normal power, normal speed, lower resolution */
        12000000, /* 12.0 MHz for normal power, high speed,   16b resolution */
        18000000, /* 18.0 MHz for normal power, high speed,   lower resolution */
    };
    #if PSP_HAS_DEVICE_PROTECTION
    if (!_bsp_adc_enable_access(init_ptr->ADC_NUMBER)) {
        return FALSE;
    }
    #endif

    /* Verify initialization of the ADC */
    if ((adc_context[init_ptr->ADC_NUMBER].init_ptr)
        && (adc_context[init_ptr->ADC_NUMBER].adc_ptr)) {
        return FALSE;
    }

    adc_ptr = _bsp_get_lwadc_base_address(init_ptr->ADC_NUMBER);
    if (adc_ptr == NULL) {
         return FALSE;
    }

    _bsp_adc_io_init(init_ptr->ADC_NUMBER);

    context_ptr = &adc_context[init_ptr->ADC_NUMBER];
    context_ptr->adc_ptr = adc_ptr;
    context_ptr->channels = 0;
    context_ptr->default_numerator   = init_ptr->REFERENCE;
    context_ptr->default_denominator = LWADC_RESOLUTION_DEFAULT;

    /* First, use 3 bit mask to create index to the max. frq. table */
    adc_max_frq_index = 0;
    switch (context_ptr->default_denominator) {
        case LWADC_RESOLUTION_16BIT:
            break;
        case LWADC_RESOLUTION_12BIT:
        case LWADC_RESOLUTION_10BIT:
        case LWADC_RESOLUTION_8BIT:
            adc_max_frq_index |= 0x01;
            break;
        default:
            return FALSE; /* bad resolution set */
    }
    if (init_ptr->SPEED == LWADC_HSC_HIGH)
        adc_max_frq_index |= 0x02;
    if (init_ptr->POWER == LWADC_LPC_NORMAL)
        adc_max_frq_index |= 0x04;
    adc_max_frq = adc_max_frq_table[adc_max_frq_index];

    /* set AD clock to be as fast as possible */
     switch (init_ptr->CLOCK_SOURCE) {
         case LWADC_CLK_BUSCLK_ANY:
             if (init_ptr->CLOCK_DIV == LWADC_DIV_ANY) {
                 if(BSP_BUS_CLOCK <= adc_max_frq)
                     adc_ptr->CFG1 = LWADC_CFG1_ADIV_1 | LWADC_CFG1_ADICLK_BUSCLK;
                 else if ((BSP_BUS_CLOCK / 2) <= adc_max_frq)
                     adc_ptr->CFG1 = LWADC_CFG1_ADIV_2 | LWADC_CFG1_ADICLK_BUSCLK;
                 else if ((BSP_BUS_CLOCK / 4) <= adc_max_frq)
                     adc_ptr->CFG1 = LWADC_CFG1_ADIV_4 | LWADC_CFG1_ADICLK_BUSCLK;
                 else if ((BSP_BUS_CLOCK / 8) <= adc_max_frq)
                     adc_ptr->CFG1 = LWADC_CFG1_ADIV_8 | LWADC_CFG1_ADICLK_BUSCLK;
                 else if ((BSP_BUS_CLOCK / 16) <= adc_max_frq)
                     adc_ptr->CFG1 = LWADC_CFG1_ADIV_8 | LWADC_CFG1_ADICLK_BUSCLK2;
                 else
                     return FALSE; /* cannot set ADC clock to be less than adc_max_frq */
             }
             else {
                 if ((BSP_BUS_CLOCK / (1 << init_ptr->CLOCK_DIV)) <= adc_max_frq)
                     adc_ptr->CFG1 = ADC_CFG1_ADIV(init_ptr->CLOCK_DIV) | LWADC_CFG1_ADICLK_BUSCLK;
                 else if ((BSP_BUS_CLOCK / 2 / (1 << init_ptr->CLOCK_DIV)) <= adc_max_frq)
                     adc_ptr->CFG1 = ADC_CFG1_ADIV(init_ptr->CLOCK_DIV) | LWADC_CFG1_ADICLK_BUSCLK2;
                 else
                     return FALSE; /* such clock combination is too fast for ADC */
             }
             break;

         case LWADC_CLK_BUSCLK:
             if (init_ptr->CLOCK_DIV == LWADC_DIV_ANY) {
                 if (BSP_BUS_CLOCK <= adc_max_frq)
                     adc_ptr->CFG1 = LWADC_CFG1_ADIV_1 | LWADC_CFG1_ADICLK_BUSCLK;
                 else if ((BSP_BUS_CLOCK / 2) <= adc_max_frq)
                     adc_ptr->CFG1 = LWADC_CFG1_ADIV_2 | LWADC_CFG1_ADICLK_BUSCLK;
                 else if ((BSP_BUS_CLOCK / 4) <= adc_max_frq)
                     adc_ptr->CFG1 = LWADC_CFG1_ADIV_4 | LWADC_CFG1_ADICLK_BUSCLK;
                 else if ((BSP_BUS_CLOCK / 8) <= adc_max_frq)
                     adc_ptr->CFG1 = LWADC_CFG1_ADIV_8 | LWADC_CFG1_ADICLK_BUSCLK;
                 else
                     return FALSE; /* cannot set ADC clock to be less than ADC max. frequency */
             }
             else if ((BSP_BUS_CLOCK / (1 << init_ptr->CLOCK_DIV)) <= adc_max_frq)
                 adc_ptr->CFG1 = ADC_CFG1_ADIV(init_ptr->CLOCK_DIV) | LWADC_CFG1_ADICLK_BUSCLK;
             else
                 return FALSE; /* such clock combination is too fast for ADC */
             break;

         case LWADC_CLK_BUSCLK2:
             if (init_ptr->CLOCK_DIV == LWADC_DIV_ANY) {
                 if ((BSP_BUS_CLOCK / 16) <= adc_max_frq)
                     adc_ptr->CFG1 = LWADC_CFG1_ADIV_8 | LWADC_CFG1_ADICLK_BUSCLK2;
                 if ((BSP_BUS_CLOCK / 16) <= adc_max_frq)
                     return FALSE; /* cannot set ADC clock to be less than ADC max. frequency */
             }
             else if ((BSP_BUS_CLOCK / 2 / (1 << init_ptr->CLOCK_DIV)) <= adc_max_frq)
                 adc_ptr->CFG1 = ADC_CFG1_ADIV(init_ptr->CLOCK_DIV) | LWADC_CFG1_ADICLK_BUSCLK2;
             else
                 return FALSE; /* such clock combination is too fast for ADC */
             break;

         case LWADC_CLK_ALTERNATE:
             if (init_ptr->CLOCK_DIV == LWADC_DIV_ANY)
                 return FALSE; /* alternate clock + any division is not supported now */
             adc_ptr->CFG1 = ADC_CFG1_ADIV(init_ptr->CLOCK_DIV) | LWADC_CFG1_ADICLK_ALTCLK;
             break;

         case LWADC_CLK_ASYNC:
             if (init_ptr->CLOCK_DIV == LWADC_DIV_ANY)
                 return FALSE; /* async clock + any division is not supported now */
             adc_ptr->CFG1 = ADC_CFG1_ADIV(init_ptr->CLOCK_DIV) | LWADC_CFG1_ADICLK_ADACK;
             break;

         default:
             return FALSE; /* invalid clock source */

     }
    adc_ptr->CFG1 |= ADC_CFG1_ADLPC_MASK;
    adc_ptr->CFG1 |= ADC_CFG1_MODE(0x3);

    adc_ptr->SC2 &= ~ADC_SC2_ADTRG_MASK; /* set SW trigger */
    adc_ptr->SC3 |= mcr;
    context_ptr->init_ptr = init_ptr;
    context_ptr->mcr = mcr;
    return TRUE;
}

/*FUNCTION*-------------------------------------------------------------------
 *
 * Function Name    : _lwadc_init_input
 * Returned Value   : TRUE for success, FALSE for failure
 * Comments         :
 * This function prepares and initialize the LWADC_STRUCT
 * with all data it will need later for quick control of the input.
 * The structure initialized here is used in all subsequent calls to any
 * LWADC driver and is uniquely identifying the input.
 * This function calls a BSP supplied function to enable the ADC input pin.
 * It adds the input to the list of channels being converted, and place the
 * ADC in continuous conversion mode if not already in this mode.
 *
 *END*----------------------------------------------------------------------*/

bool  _lwadc_init_input( LWADC_STRUCT_PTR lwadc_ptr, uint32_t input )
{
    ADC_MemMapPtr     adc_ptr;
    uint32_t          device, channel;
    uint32_t          source_reg, mux;

    device  = (((ADC_GET_MODULE(input)) >> ADC_SOURCE_MODULE_SHIFT) - 1);
    channel = ADC_GET_CHANNEL(input);
    mux     = ((ADC_GET_MUXSEL(input)) >> ADC_SOURCE_MUXSEL_SHIFT);
    adc_ptr = _bsp_get_lwadc_base_address(device);
    if (NULL == adc_ptr){
        return FALSE;
    }
    source_reg = channel;
    /* Used as a validity check in other _lwadc functions */
    lwadc_ptr->context_ptr = NULL;

    /* Make sure channel & device is in range */
    if ( (device >= ADC_NUM_DEVICES) || (channel > ADC_HW_CHANNELS) ) {
        return FALSE;
    }
#if ADC_HAS_DIFFERENTIAL_CHANNELS
    if (ADC_GET_DIFF(input))
        source_reg |= ADC_SC1_DIFF_MASK;
#endif /* ADC_HAS_DIFFERENTIAL_CHANNELS */

    /* Enable ADC I/O Pin */
    if ( _bsp_adc_channel_io_init(input) != IO_OK) {
        return FALSE;
    }
    adc_ptr->SC1[0] = source_reg;

    if (mux == 1) {
        /* channel A selected */
        adc_ptr->CFG2 &= ~ADC_CFG2_MUXSEL_MASK;
    }
    else if (mux == 2) {
        /* channel B selected */
        adc_ptr->CFG2 |= ADC_CFG2_MUXSEL_MASK;
    }
    /* set channel specific parameters */
    lwadc_ptr->input   = input;
    lwadc_ptr->context_ptr = &adc_context[device];
    lwadc_ptr->context_ptr->adc_ptr = adc_ptr;
    lwadc_ptr->numerator   = lwadc_ptr->context_ptr->default_numerator;
    lwadc_ptr->denominator = lwadc_ptr->context_ptr->default_denominator;

    /* Set channel to convert */
    _int_disable();
    lwadc_ptr->context_ptr->channels = channel;
    _lwadc_restart(lwadc_ptr);
    _int_enable();
    return TRUE;
}

/*FUNCTION*-------------------------------------------------------------------
 *
 * Function Name    : _lwadc_read_raw
 * Returned Value   : TRUE for success, FALSE for failure
 * Comments         :
 *    This function Read the current value of the ADC input and return
 *    the result without applying any scaling.
 *
 *END*----------------------------------------------------------------------*/


bool _lwadc_read_raw( LWADC_STRUCT_PTR lwadc_ptr, LWADC_VALUE_PTR outSample)
{
    ADC_MemMapPtr   adc_ptr;
    LWADC_VALUE     sample;

    if (lwadc_ptr->context_ptr == NULL) {
        return FALSE;
    }

    adc_ptr = lwadc_ptr->context_ptr->adc_ptr;
    if (NULL == adc_ptr){
        return FALSE;
    }

    if ((adc_ptr->SC3 & ADC_SC3_ADCO_MASK) == 0) {
        /* ADC is not in continuous mode. */
        adc_ptr->SC3 |= ADC_SC3_ADCO_MASK;
    }

    sample = adc_ptr->R[0];

    *outSample = sample & ADC_CDR_CDATA_MASK;

    /* Clear conversion complete */
    adc_ptr->SC1[0] &= ~ADC_SC1_AIEN_MASK;
    return TRUE;
}

/*FUNCTION*-------------------------------------------------------------------
 *
 * Function Name    : _lwadc_read
 * Returned Value   : TRUE for success, FALSE for failure
 * Comments         :
 *    This function Read the current value of the ADC input, applies any
 *    scaling, and return the result.
 *
 *END*----------------------------------------------------------------------*/

bool _lwadc_read( LWADC_STRUCT_PTR lwadc_ptr, LWADC_VALUE_PTR outSample)
{
    LWADC_VALUE rawSample;

    if (lwadc_ptr->context_ptr == NULL) {
        return FALSE;
    }

    if (_lwadc_read_raw(lwadc_ptr,&rawSample)) {
        *outSample = ((rawSample*lwadc_ptr->numerator) + (lwadc_ptr->denominator>>1))/ lwadc_ptr->denominator;
        return TRUE;
    }

    *outSample = 0;
    return FALSE;
}

/*FUNCTION*-------------------------------------------------------------------
 *
 * Function Name    : _lwadc_wait_next
 * Returned Value   : TRUE for success, FALSE for failure
 * Comments         :
 *    This function Waits for a new value to be available on the specified ADC input.
 *
 *END*----------------------------------------------------------------------*/

bool _lwadc_wait_next( LWADC_STRUCT_PTR lwadc_ptr )
{
    ADC_MemMapPtr   adc_ptr;
    uint32_t        channel;
    uint32_t        input;

    if (lwadc_ptr->context_ptr == NULL) {
        return FALSE;
    }

    input = lwadc_ptr->input;
    adc_ptr = lwadc_ptr->context_ptr->adc_ptr;
    if (NULL == adc_ptr){
        return FALSE;
    }
    if (lwadc_ptr->context_ptr == NULL) {
        return FALSE;
    }

    channel = ADC_GET_CHANNEL(input);
    if ((adc_ptr->SC3 & ADC_SC3_ADCO_MASK) == 0) {
        /* ADC is not in continuous mode. */
        adc_ptr->SC3 |= ADC_SC3_ADCO_MASK;
    }
#if ADC_HAS_DIFFERENTIAL_CHANNELS
    if (ADC_GET_DIFF(input))
        channel |= ADC_SC1_DIFF_MASK;
#endif /* ADC_HAS_DIFFERENTIAL_CHANNELS */

    /* Select channel for converting */
    adc_ptr->SC1[0] = channel;
    adc_ptr->SC1[0] |= ADC_SC1_AIEN_MASK;
    while((adc_ptr->SC1[0] & ADC_SC1_COCO_MASK) == 0){}
    return TRUE;
}

/*FUNCTION*-------------------------------------------------------------------
 *
 * Function Name    : _lwadc_read_average
 * Returned Value   : TRUE for success, FALSE for failure
 * Comments         :
 *    This function Read the current value of the ADC input, applies any
 *    scaling, and return the result.
 *
 *END*----------------------------------------------------------------------*/

bool _lwadc_read_average( LWADC_STRUCT_PTR lwadc_ptr, uint32_t num_samples, LWADC_VALUE_PTR outSample)
{
    LWADC_VALUE rawSample,sum = 0;
    uint32_t     i;

    if (num_samples > (MAX_UINT_32/LWADC_RESOLUTION_DEFAULT)) {
        return FALSE;
    }

    for (i=0;i<num_samples;i++) {
        if (!_lwadc_wait_next(lwadc_ptr)) {
            return FALSE;
        }

        if (!_lwadc_read_raw(lwadc_ptr,&rawSample)) {
            return FALSE;
        }
        sum += rawSample;
    }

    *outSample = (((sum/num_samples)*lwadc_ptr->numerator) + (lwadc_ptr->denominator>>1))/ lwadc_ptr->denominator;

    return TRUE;
}


/*FUNCTION*-------------------------------------------------------------------
 *
 * Function Name    : _lwadc_set_attribute
 * Returned Value   : TRUE for success, FALSE for failure
 * Comments         :
 * This function sets attributes for the specified ADC input. Attributes
 * could include single/differential mode, reference, scaling numerator or
 * denominator, etc.
 *
 *END*----------------------------------------------------------------------*/

bool _lwadc_set_attribute( LWADC_STRUCT_PTR lwadc_ptr, LWADC_ATTRIBUTE attribute,  uint32_t value)
{
    ADC_MemMapPtr adc_ptr;

    if (lwadc_ptr->context_ptr == NULL) {
        return FALSE;
    }

    adc_ptr = lwadc_ptr->context_ptr->adc_ptr;
    if (NULL == adc_ptr){
        return FALSE;
    }
    switch (attribute) {
        case LWADC_NUMERATOR:
            lwadc_ptr->numerator = value;
            break;

        case LWADC_DENOMINATOR:
            lwadc_ptr->denominator = value;
            break;

        case LWADC_DEFAULT_NUMERATOR:
            lwadc_ptr->context_ptr->default_numerator = value;
            break;

        case LWADC_DEFAULT_DENOMINATOR:
            lwadc_ptr->context_ptr->default_denominator = value;
            break;

        case LWADC_POWER_DOWN:
            if (value) {
                adc_ptr->CFG1 |= ADC_CFG1_ADLPC_MASK;
            } else {
                _int_disable();
                _lwadc_restart(lwadc_ptr);
                _int_enable();
            }
            break;

        case LWADC_FORMAT:
            break;

        case LWADC_FREQUENCY:
            break;

        case LWADC_DIVIDER:
            /* We support a divider, as long as it is 1. */
            return (value == 1)?TRUE:FALSE;

        case LWADC_DIFFERENTIAL:
            /* We don't support differential, so return FALSE if asked for it, TRUE otherwise */
            return !value;

        case LWADC_INPUT_CONVERSION_ENABLE:
            break;

        default:
            return FALSE;
    }
    return TRUE;
}

/*FUNCTION*-------------------------------------------------------------------
 *
 * Function Name    : _lwadc_get_attribute
 * Returned Value   : TRUE for success, FALSE for failure
 * Comments         :
 *    This function This function gets attributes for the specified ADC input,
 *    or for the ADC module as a whole. Attributes could include
 *    single/differential mode, reference, scaling numerator or denominator, etc.
 *
 *END*----------------------------------------------------------------------*/

bool _lwadc_get_attribute( LWADC_STRUCT_PTR lwadc_ptr, LWADC_ATTRIBUTE attribute,  uint32_t *value_ptr)
{
    ADC_MemMapPtr adc_ptr;
    if ((lwadc_ptr->context_ptr == NULL) || (value_ptr == NULL)) {
        return FALSE;
    }

    adc_ptr = lwadc_ptr->context_ptr->adc_ptr;
    if (NULL == adc_ptr){
        return FALSE;
    }
    switch (attribute) {
        case LWADC_NUMERATOR:
            *value_ptr = lwadc_ptr->numerator;
            break;

        case LWADC_DENOMINATOR:
            *value_ptr = lwadc_ptr->denominator;
            break;

        case LWADC_DEFAULT_NUMERATOR:
            *value_ptr = lwadc_ptr->context_ptr->default_numerator;
            break;

        case LWADC_DEFAULT_DENOMINATOR:
            *value_ptr = lwadc_ptr->context_ptr->default_denominator;
            break;

        case LWADC_POWER_DOWN:
            *value_ptr = (adc_ptr->CFG1&ADC_CFG1_ADLPC_MASK)?TRUE:FALSE;
            break;

        case LWADC_RESOLUTION:
            *value_ptr = LWADC_RESOLUTION_DEFAULT;
            break;

        case LWADC_REFERENCE:
            *value_ptr = lwadc_ptr->context_ptr->init_ptr->REFERENCE;
            break;

        case LWADC_FREQUENCY:
            *value_ptr = adc_max_frq;
            break;

        case LWADC_DIVIDER:
            *value_ptr = 1;
            break;

        case LWADC_DIFFERENTIAL:
            *value_ptr = FALSE;
            break;

        case LWADC_FORMAT:
            *value_ptr = FALSE;
            break;

        case LWADC_INPUT_CONVERSION_ENABLE:
            *value_ptr = FALSE;
            break;

        default:
            return FALSE;
    }
    return TRUE;
}

/* EOF */
