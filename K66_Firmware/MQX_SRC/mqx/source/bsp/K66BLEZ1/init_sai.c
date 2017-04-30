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
*   This file contains board-specific SAI initialization functions.
*
*
*END************************************************************************/

#include "mqx.h"
#include "bsp.h"
#include "sai.h"
#include "sai_audio.h"
#include "sai_ksai.h"
#include <cm_kinetis.h>

AUDIO_DATA_FORMAT _bsp_audio_data_init = {
    AUDIO_LITTLE_ENDIAN,    /* Endian of input data */
    AUDIO_ALIGNMENT_LEFT,   /* Alignment of input data */
    16,                      /* Bit size of input data */
    2,                      /* Sample size in bytes */
    2,                      /* Number of channels */
	44100                   /* Sample rate */
};

KSAI_INIT_STRUCT _bsp_ksai_init = {
    0,                      /* Selected peripheral (HW channel) */
    0,                      /* TX channel */
    0,                      /* RX channel */
    I2S_TX_ASYNCHRONOUS |   /* TX is asynchronous */
    I2S_RX_SYNCHRONOUS  |   /* RX hooked on TX */
    I2S_TX_BCLK_NORMAL  |   /* Both TX and RX are clocked by the transmitter */
    I2S_RX_BCLK_NORMAL,     /* bit clock (SAI_TX_BCLK) */
    I2S_TX_MASTER |         /* SAI transmitter mode */
    I2S_RX_MASTER,          /* SAI receiver mode */
    I2S_CLK_INT,            /* Clock source */
    5,                      /* Interrupt priority */
	4,                      /* Buffer block number */
    512,                    /* Buffer size */
    CM_CLOCK_SOURCE_SYSTEM, /* Internal master clock source */
    &_bsp_audio_data_init   /* Audio init */
};

const SAI_INIT_STRUCT _bsp_sai_init = 
{
    "sai:",
    _ksai_dma_init,
    _ksai_dma_deinit,
    _ksai_dma_ioctl,
    &_bsp_ksai_init
};
