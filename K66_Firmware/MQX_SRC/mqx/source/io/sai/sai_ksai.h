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
*   This file contains the definitions of constants and structures
*   required for the SAI driver
*
*
*END************************************************************************/

#ifndef __SAI_KSAI_H__
#define __SAI_KSAI_H__

#include "sai_audio.h"

/*
#if !PSP_MQX_CPU_IS_VYBRID
#include <cm_kinetis.h>
#else 
#include <cm_vybrid.h>
#endif
*/
/*
** Constants
*/

/* Maximum value  that can be stored in 16 bit long variable */
#define BIT16_MAX 32767

/*
** Types declaration
*/ 
#ifdef __cplusplus
extern "C" 
{
#endif

typedef struct ki2s_init_struct
{
    /* Selected SAI HW module */
    uint8_t HW_CHANNEL;
        
    /* The SAI TX channel to initialize */
    uint8_t TX_CHANNEL;
    
    /* The SAI RX channel to initialize */
    uint8_t RX_CHANNEL;
    
    /* Clock setup: sync-async; bitclock: normal-swapped */
    uint8_t CLOCK_MODE;

    /* Default operating mode */
    uint8_t MODE;

    /* I2S master clock source*/
    uint8_t CLOCK_SOURCE;

    /* Interrupt level to use */
    _mqx_uint LEVEL;

    /* Audio buffer block number */
    uint32_t BUFFER_BLOCK;

    /* Audio buffer size */
    uint32_t BUFFER_SIZE;
    
    /* Internal master clock source */
    CM_CLOCK_SOURCE MCLK_SRC;
    
    /* I/O data format */
    AUDIO_DATA_FORMAT const * IO_FORMAT;

} KSAI_INIT_STRUCT, *KSAI_INIT_STRUCT_PTR;

typedef const KSAI_INIT_STRUCT * KSAI_INIT_STRUCT_CPTR;

/*
** Global functions
*/
_mqx_int _ksai_dma_init(void *, uint8_t);
_mqx_int _ksai_dma_deinit(void *, uint8_t);    
_mqx_int _ksai_dma_ioctl(void *, _mqx_int, void *);


void * _bsp_get_sai_base_address(uint8_t);
uint32_t _bsp_get_sai_tx_vector(uint8_t);
uint32_t _bsp_get_sai_rx_vector(uint8_t);
uint32_t _bsp_get_sai_tx_dma_source(uint8_t);
uint32_t _bsp_get_sai_rx_dma_source(uint8_t);
uint8_t _bsp_get_sai_dma_channel_mask(uint8_t);
    
#ifdef __cplusplus
}
#endif

#endif /* __SAI_KSAI_H__ */

/* EOF */
