/*HEADER**********************************************************************
*
* Copyright 2011 Freescale Semiconductor, Inc.
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
*   This file contains the definitions of constants and structures
*   required for initialization of the card.
*
*
*END************************************************************************/
#ifndef _bsp_prv_h
#define _bsp_prv_h 1
#ifdef __cplusplus
extern "C" {
#endif

/*
**  FUNCTION PROTOTYPES
*/

extern void     __init(void);
extern uint32_t  _bsp_get_hwticks(void *);
extern void     _bsp_exit_handler(void);
extern void     _bsp_timer_isr(void *);
extern uint32_t  _rtc_int_install(INT_ISR_FPTR); 
extern void     _bsp_flexbus_pccard_setup (const uint32_t base_address);
extern void     _bsp_watchdog_disable(void);
extern void     _bsp_ddr2_setup (void);
extern void     systick_config(uint32_t bsp_system_clock, uint32_t bsp_alarm_frequency);


/*
**  STRUCTURE DEFINITIONS
*/
extern       HWTIMER systimer;
/* I/O initialization controlled by initialization structures for each channel */
extern const KUART_INIT_STRUCT _bsp_sci0_init;
extern const KUART_INIT_STRUCT _bsp_sci1_init;
extern const KUART_INIT_STRUCT _bsp_sci2_init;
extern const KUART_INIT_STRUCT _bsp_sci3_init;
extern const KUART_INIT_STRUCT _bsp_sci4_init;

extern const SPI_INIT_STRUCT _bsp_spi0_init;
extern const SPI_INIT_STRUCT _bsp_spi1_init;
extern const SPI_INIT_STRUCT _bsp_spi2_init;

extern const KI2C_INIT_STRUCT _bsp_i2c0_init;
extern const KI2C_INIT_STRUCT _bsp_i2c1_init;
extern const KI2C_INIT_STRUCT _bsp_i2c2_init;
extern const KI2C_INIT_STRUCT _bsp_i2c3_init;

extern const KADC_INIT_STRUCT _bsp_adc0_init;
extern const KADC_INIT_STRUCT _bsp_adc1_init;
extern const KADC_INIT_STRUCT _bsp_adc2_init;
extern const KADC_INIT_STRUCT _bsp_adc3_init;

extern const LWADC_INIT_STRUCT lwadc0_init;
extern const LWADC_INIT_STRUCT lwadc1_init;
extern const LWADC_INIT_STRUCT lwadc2_init;
extern const LWADC_INIT_STRUCT lwadc3_init;

extern const TCHRES_INIT_STRUCT _bsp_tchscr_resisitve_init;
extern const PCCARDFLEXBUS_INIT_STRUCT  _bsp_cfcard_init;
extern const IODEBUG_INIT_STRUCT _bsp_iodebug_init;

extern const KUSB_DCD_INIT_STRUCT _bsp_usb_dcd_init;
extern const ESDHC_INIT_STRUCT _bsp_esdhc0_init;
extern const FLASHX_FILE_BLOCK _bsp_flashx_file_blocks[];
extern const FLASHX_INIT_STRUCT _bsp_flashx_init;
extern const SAI_INIT_STRUCT _bsp_sai_init;

extern const DMA_DEVIF_LIST _bsp_dma_devif_list[];

#ifdef __cplusplus
}
#endif

#endif
/* EOF */

