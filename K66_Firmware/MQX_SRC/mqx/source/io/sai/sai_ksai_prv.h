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
*   required for the SAI/I2S driver
*
*
*END************************************************************************/
#ifndef __SAI_KSAI_PRV_H__
#define __SAI_KSAI_PRV_H__

#include "sai_ksai.h"
#include "sai_audio.h"

/*--------------------------------------------------------------------------*/
/*
**                            CONSTANT DEFINITIONS
*/

/* Limits for master clock divider */
#define FRACT_MAX 256
#define DIV_MAX 4096

/*
** SAI limits
*/

#define BCLK_DIV_MIN    1
#define BCLK_DIV_MAX    4096

/* Hardware FIFO size */
#if !PSP_MQX_CPU_IS_VYBRID
  #define SIZE_OF_FIFO    8
#else
  #define SIZE_OF_FIFO    32
#endif
/* Number of data channels on both RX and TX */
#if (!PSP_MQX_CPU_IS_VYBRID && !(MQX_CPU == PSP_CPU_MK22FN512))
  #define SAI_DATA_CHANNELS 2
#else
  #define SAI_DATA_CHANNELS 1
#endif

#define DEFAULT_BCLK_DIV 8

/* Define the max size of the buffer */
#if (MQX_CPU == PSP_CPU_MK22FN512)
#define SAI_MAX_BUFFER_SIZE 4096
#else
#define SAI_MAX_BUFFER_SIZE 8192
#endif



/*--------------------------------------------------------------------------*/
/*
**                    DATATYPE DECLARATIONS
*/

/*
** SAI software buffer structure
*/
typedef struct ksai_buffer
{    
    /* data for dma mode*/
    uint8_t *DMA_DATA;
    
    /* Buffer size */
    uint32_t SIZE;
    
    /* Free space in buffer */
    uint32_t SPACE;

    /*The low block address */
    uint8_t * START_PERIOD;

    /*The high block address*/
    uint8_t * END_PERIOD; 

    /* The period number */
    uint8_t PERIODS;

} KSAI_BUFFER, *KSAI_BUFFER_PTR;


typedef struct ksai_info_struct
{  
    /* Current initialized values */
    KSAI_INIT_STRUCT INIT;

    I2S_MemMapPtr SAI_PTR;
    
    /* Selected SAI HW module */
    uint8_t HW_CHANNEL;
    
    /* I2S TX data channel */
    uint8_t TX_CHANNEL;
    uint8_t RX_CHANNEL;
    
    /* Master clock source*/
    CM_CLOCK_SOURCE MCLK_SRC;
    
    /* 
    ** Oversampling clock frequency in Hz - only valid 
    ** when internal clock source is selected, ignored otherwise 
    */
    uint32_t MCLK_FREQ;   
    
    /* I2S module clock source (only affects SAI master mode) */
    uint8_t CLOCK_SOURCE;

    /* I2S mode of operation (master/slave)*/
    uint8_t MODE;

    /* I2S I/O mode (write/read) */
    uint8_t IO_MODE;

    /* Audio data input/output format */
    AUDIO_DATA_FORMAT TX_FORMAT;
    AUDIO_DATA_FORMAT RX_FORMAT;

    /* Pointer to the buffer to use for current data */
    KSAI_BUFFER TX_BUFFER;
    KSAI_BUFFER RX_BUFFER;

    /* Transmission statistics */
    I2S_STATISTICS_STRUCT TX_STATS;
    I2S_STATISTICS_STRUCT RX_STATS;

    /* First Input/Output operation */
    bool TX_FIRST_IO;
    
    /* First Input/Output operation */
    bool RX_FIRST_IO;

    /* TX callback fucntion */
    SAI_CALLBACK TX_CALLBACK;
    /* TX callback param */
    void * TX_CALLBACK_PARAM;

    /* RX callback function */
    SAI_CALLBACK RX_CALLBACK;
    /* Rx callback param */
    void * RX_CALLBACK_PARAM;

    /* Is DMA kick off */
    bool TX_DMA_KICKOFF;
    bool RX_DMA_KICKOFF;
    /* Clock setup: sync-async; bitclock: normal-swapped */
    uint8_t CLOCK_MODE;
    
    /*DMA settings*/
    DMA_TCD TX_TCD;
    DMA_TCD RX_TCD;

    DMA_CHANNEL_HANDLE TX_DCH;
    DMA_CHANNEL_HANDLE RX_DCH;

    uint32_t TX_TCD_SEQ;
    uint32_t RX_TCD_SEQ;	
} KSAI_INFO_STRUCT, *KSAI_INFO_STRUCT_PTR;

#endif /* __SAI_KSAI_PRV_H__ */

/* EOF */
