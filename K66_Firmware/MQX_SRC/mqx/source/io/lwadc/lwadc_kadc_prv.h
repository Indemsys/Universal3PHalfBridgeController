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
*   CPU specific LWADC driver header file
*
*
*END************************************************************************/

#ifndef __lwadc_kadc_prv_h__
#define __lwadc_kadc_prv_h__

#ifdef __cplusplus
extern "C" {
#endif
#define ADC_MAX_FREQUENCY 5000000 /* 5 MHz for ADC clock source is maximum */

#ifndef ADC_CHANNELS_PER_ADC
    #define ADC_CHANNELS_PER_ADC (2)  /* one ADC can handle 2 channels */
#endif
#define ADC_MAX_HW_CHANNELS  ADC_CHANNELS_PER_ADC /* there is no sense to have more channels allocated */

#ifndef ADC_MAX_MODULES
    #error Define number of ADC peripheral modules on chip in BSP (<MQX>/source/bsp/<board_name>/<board_name>.h)
#endif

#ifndef ADC_MAX_SW_CHANNELS
    #define ADC_MAX_CHANNELS ADC_MAX_HW_CHANNELS
#else
    #if (ADC_MAX_HW_CHANNELS > ADC_MAX_SW_CHANNELS)
        #define ADC_MAX_CHANNELS ADC_MAX_SW_CHANNELS
    #else
        #define ADC_MAX_CHANNELS ADC_MAX_HW_CHANNELS
    #endif
#endif

#define LWADC_CFG2_ADLSTS_20             ADC_CFG2_ADLSTS(0)
#define LWADC_CFG2_ADLSTS_12             ADC_CFG2_ADLSTS(1)
#define LWADC_CFG2_ADLSTS_6              ADC_CFG2_ADLSTS(2)
#define LWADC_CFG2_ADLSTS_2              ADC_CFG2_ADLSTS(3)
#define LWADC_CFG2_ADLSTS_DEFAULT       (ADC_CFG2_ADLSTS_20)

#define LWADC_SC3_AVGS_4                 ADC_SC3_AVGS(0x00)
#define LWADC_SC3_AVGS_8                 ADC_SC3_AVGS(0x01)
#define LWADC_SC3_AVGS_16                ADC_SC3_AVGS(0x02)
#define LWADC_SC3_AVGS_32                ADC_SC3_AVGS(0x03)
#define LWADC_SC3_AVGE                   ADC_SC3_AVGS(0x04)
#define LWADC_SC3_ADCO                   ADC_SC3_AVGS(0x08)
#define LWADC_SC3_CALF                   ADC_SC3_AVGS(0x40)
#define LWADC_SC3_CAL                    ADC_SC3_AVGS(0x80)

#define LWADC_SC2_REFSEL_VREF            ADC_SC2_REFSEL(0x00)
#define LWADC_SC2_REFSEL_VALT            ADC_SC2_REFSEL(0x01)
#define LWADC_SC2_REFSEL_VBG             ADC_SC2_REFSEL(0x02)

#define LWADC_SC1_ADCH_DISABLED          ADC_SC1_ADCH(0x1F)

#define LWADC_CFG1_ADIV_1                ADC_CFG1_ADIV(0x00)
#define LWADC_CFG1_ADIV_2                ADC_CFG1_ADIV(0x01)
#define LWADC_CFG1_ADIV_4                ADC_CFG1_ADIV(0x02)
#define LWADC_CFG1_ADIV_8                ADC_CFG1_ADIV(0x03)

#define LWADC_CFG1_ADICLK_BUSCLK         ADC_CFG1_ADICLK(0x00)
#define LWADC_CFG1_ADICLK_BUSCLK2        ADC_CFG1_ADICLK(0x01)
#define LWADC_CFG1_ADICLK_ALTCLK         ADC_CFG1_ADICLK(0x02)
#define LWADC_CFG1_ADICLK_ADACK          ADC_CFG1_ADICLK(0x03)

extern void    *_bsp_get_lwadc_base_address(_mqx_uint);

#ifdef __cplusplus
}
#endif

#endif

/* EOF */
