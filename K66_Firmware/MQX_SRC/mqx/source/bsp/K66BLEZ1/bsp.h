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
*   This file includes all include files specific to this target system.
*
*
*END************************************************************************/

#ifndef __bsp_h__
#define __bsp_h__   1

#include <psp.h>

/* Processor Expert files */
#include <PE_LDD.h>
#ifdef PE_LDD_VERSION
#include <Events.h>
#endif

#include <bsp_rev.h>
#include <K66BLEZ1.h>

/* Clock manager */
#include <cm_kinetis.h>
#include <bsp_cm.h>
#include <cm.h>
/* Low power manager*/
#include <init_lpm.h>
#include <lpm_kinetis.h>
#include <lpm.h>

#include <fio.h>
#include <io.h>
#include <serial.h>
#include <serl_kuart.h>
#include <spi.h>
#include <spi_dspi.h>
#include <spi_dspi_dma.h>
#include <i2c.h>
#include <i2c_ki2c.h>
#include <io_mem.h>
#include <io_null.h>
#include <enet.h>
#include <macnet_mk65.h>
#include <lwgpio_kgpio.h>
#include <lwgpio.h>
#include <io_gpio.h>
#include <esdhc.h>
#include <sdcard.h>
#include <sdcard_spi.h>
#include <sdcard_esdhc.h>
#include <iodebug.h>
#include <apccard.h>
#include <pccardflexbus.h>
#include <pcflash.h>
#include <adc_mk65.h>
#include <adc.h>
#include <adc_kadc.h>
#include <lwadc_kadc.h>
#include <kinetis_fcan.h>
/* #include <kflexcan.h> */
#include <fsl_flexcan_driver.h>
#include <fsl_flexcan_hal.h>
#include <fsl_flexcan_int.h>
#include <flashx.h>
#include <flash_ftfe.h>
#include <flash_mk65.h>
#include <hwtimer.h>
#include <hwtimer_lpt.h>
#include <hwtimer_pit.h>
#include <hwtimer_systick.h>
#include <krtc.h>
#include <rtc.h>
#include <usb_dcd.h>
#include <usb_dcd_kn.h>
#include <usb_dcd_kn_prv.h>
#include <tchres.h>

#include <timer_qpit.h>
#include <usb_bsp.h>
#include <sai.h>
#include <sai_ksai.h>
#include <dma.h>


#ifdef __cplusplus
extern "C" {
#endif

_mqx_int _bsp_adc_io_init(_mqx_uint adc_num);
_mqx_int _bsp_adc_channel_io_init(uint16_t source);
_mqx_int _bsp_dspi_io_init(uint32_t dev_num);
_mqx_int _bsp_flexcan_io_init (uint8_t);
_mqx_int _bsp_i2c_io_init( uint32_t );
_mqx_int _bsp_gpio_io_init( void );
_mqx_int _bsp_esdhc_io_init (uint8_t, uint16_t);
_mqx_int _bsp_rtc_io_init( void );
_mqx_int _bsp_serial_io_init(uint8_t dev_num,  uint8_t flags);
_mqx_int _bsp_serial_rts_init( uint32_t );
_mqx_int _bsp_usb_host_io_init(struct usb_host_if_struct *);
_mqx_int _bsp_usb_dev_io_init(struct usb_dev_if_struct *);
_mqx_int _bsp_enet_io_init(_mqx_uint);
bool  _bsp_get_mac_address(uint32_t,uint32_t,_enet_address);
_mqx_int _bsp_ftfx_io_init(_mqx_uint);
_mqx_int _bsp_sai_io_init(uint32_t);
_mqx_int _bsp_serial_irda_tx_init(uint32_t, bool);
_mqx_int _bsp_serial_irda_rx_init(uint32_t, bool);
_mqx_int _bsp_crc_io_init(void);
_mqx_int _bsp_sdramc_io_init(void);

extern const SDCARD_INIT_STRUCT _bsp_sdcard0_init;
extern const LPM_CPU_OPERATION_MODE LPM_CPU_OPERATION_MODES[];
extern const LWADC_INIT_STRUCT lwadc0_init;
extern const LWADC_INIT_STRUCT lwadc1_init;

#define _bsp_int_init(num, prior, subprior, enable)     _nvic_int_init(num, prior, enable)
#define _bsp_int_enable(num)                            _nvic_int_enable(num)
#define _bsp_int_disable(num)                           _nvic_int_disable(num)




extern int RTT_terminal_printf(const char *sFormat, ...);

#ifdef __cplusplus
}
#endif

#endif  /* __bsp_h__ */
/* EOF */
