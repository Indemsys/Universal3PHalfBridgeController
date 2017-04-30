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
*   This include file is used to provide information needed by
*   applications using the SAI I/O functions.
*
*
*END************************************************************************/

/*
** TODO:
** 1. Extend driver API, so it supports clock hooking combinations RX0->TX0, 
**    TX0->RX1 etc and bit clock swapping between TX and RX.
** 2. Separate channel selection for TX and RX - Extend init and info struct.
**
*/

#ifndef __SAI_H__
#define __SAI_H__

#include <ioctl.h>

/*--------------------------------------------------------------------------*/
/*
**                            CONSTANT DEFINITIONS
*/

/* Enable floating point operations in I2S */
#ifndef I2S_USE_FLOAT
#define I2S_USE_FLOAT   0
#endif

#define I2S_CHAR_BIT    (0x08)

/*
** SAI Clock sources
*/
#define I2S_CLK_INT     (0x00)
#define I2S_CLK_EXT     (0x01)

/*
** SAI Bus Modes
*/
#define I2S_MODE_MASTER (0x01)
#define I2S_MODE_SLAVE  (0x02)

/*
** SAI I/O Mode
*/
#define I2S_IO_READ     (0x04)
#define I2S_IO_WRITE    (0x08)

/*
** SAI RX/TX clock modes
*/
#define I2S_TX_ASYNCHRONOUS (0x01)
#define I2S_TX_SYNCHRONOUS  (0x02)
#define I2S_RX_ASYNCHRONOUS (0x04)
#define I2S_RX_SYNCHRONOUS  (0x08)

#define I2S_TX_BCLK_NORMAL  (0x10)
#define I2S_TX_BCLK_SWAPPED (0x20)
#define I2S_RX_BCLK_NORMAL  (0x40)
#define I2S_RX_BCLK_SWAPPED (0x80)

#define I2S_TX_MASTER       (I2S_MODE_MASTER << 4)
#define I2S_TX_SLAVE        (I2S_MODE_SLAVE  << 4)

#define I2S_RX_MASTER       (I2S_MODE_MASTER)
#define I2S_RX_SLAVE        (I2S_MODE_SLAVE)

#define I2S_DEBUG       0

#if I2S_DEBUG
#define I2S_LOG(...)  \
        {   \
            printf("\nI2S_LOG: %s: %d", __FILE__, __LINE__);  \
            printf(__VA_ARGS__);    \
        }
#else
#define I2S_LOG(...) {}
#endif

/*
** SAI Error Codes
*/
#define I2S_OK                              (0x00)
#define I2S_ERROR_INVALID_PARAMETER         (I2S_ERROR_BASE | 0x01)
#define I2S_ERROR_CHANNEL_INVALID           (I2S_ERROR_BASE | 0x02)
#define I2S_ERROR_MODE_INVALID              (I2S_ERROR_BASE | 0x03)
#define I2S_ERROR_WORD_LENGTH_UNSUPPORTED   (I2S_ERROR_BASE | 0x04)
#define I2S_ERROR_CLK_INVALID               (I2S_ERROR_BASE | 0x05)
#define I2S_ERROR_DIVIDER_VALUE_INVALID     (I2S_ERROR_BASE | 0x06)
#define I2S_ERROR_FREQUENCY_INVALID         (I2S_ERROR_BASE | 0x07)
#define I2S_ERROR_BUFFER_SMALL              (I2S_ERROR_BASE | 0x08)
#define I2S_ERROR_DEVICE_BUSY               (I2S_ERROR_BASE | 0x09)
#define I2S_ERROR_PARAM_OUT_OF_RANGE        (I2S_ERROR_BASE | 0x0A)
#define I2S_ERROR_INVALID_BCLK              (I2S_ERROR_BASE | 0x0B)
#define I2S_ERROR_INVALID_TX_CHANNEL        (I2S_ERROR_BASE | 0x0C)
#define I2S_ERROR_INVALID_RX_CHANNEL        (I2S_ERROR_BASE | 0x0D)
#define I2S_ERROR_DMA_FAIL                 (I2S_ERROR_BASE | 0x0E)

/*
** IOCTL calls specific to I2S
*/
#define IO_TYPE_I2S                                 0x15
#define IO_IOCTL_I2S_SET_MODE_MASTER                _IO(IO_TYPE_I2S, 0x01)
#define IO_IOCTL_I2S_SET_MODE_SLAVE                 _IO(IO_TYPE_I2S, 0x02)
#define IO_IOCTL_I2S_SET_CLOCK_SOURCE_INT           _IO(IO_TYPE_I2S, 0x03)
#define IO_IOCTL_I2S_SET_CLOCK_SOURCE_EXT           _IO(IO_TYPE_I2S, 0x04)
#define IO_IOCTL_I2S_SET_MCLK_FREQ                  _IO(IO_TYPE_I2S, 0x05)
#define IO_IOCTL_I2S_GET_MODE                       _IO(IO_TYPE_I2S, 0x06)
#define IO_IOCTL_I2S_GET_CLOCK_SOURCE               _IO(IO_TYPE_I2S, 0x07)
#define IO_IOCTL_I2S_GET_MCLK_FREQ                  _IO(IO_TYPE_I2S, 0x08)
#define IO_IOCTL_I2S_GET_TX_STATISTICS                 _IO(IO_TYPE_I2S, 0x09)
#define IO_IOCTL_I2S_GET_RX_STATISTICS                 _IO(IO_TYPE_I2S, 0x0A)
#define IO_IOCTL_I2S_SET_TXFIFO_WATERMARK           _IO(IO_TYPE_I2S, 0x0B)
#define IO_IOCTL_I2S_SET_RXFIFO_WATERMARK           _IO(IO_TYPE_I2S, 0x0C)
#define IO_IOCTL_I2S_GET_TXFIFO_WATERMARK           _IO(IO_TYPE_I2S, 0x0D)
#define IO_IOCTL_I2S_GET_RXFIFO_WATERMARK           _IO(IO_TYPE_I2S, 0x0E)
#define IO_IOCTL_I2S_CLEAR_TX_STATISTICS               _IO(IO_TYPE_I2S, 0x0F)
#define IO_IOCTL_I2S_CLEAR_RX_STATISTICS               _IO(IO_TYPE_I2S, 0x10)
#define IO_IOCTL_I2S_SET_CLOCK_MODE                 _IO(IO_TYPE_I2S, 0x11)
#define IO_IOCTL_I2S_GET_CLOCK_MODE                 _IO(IO_TYPE_I2S, 0x12)
#define IO_IOCTL_I2S_UPDATE_TX_STATUS                     _IO(IO_TYPE_I2S, 0x13)
#define IO_IOCTL_I2S_UPDATE_RX_STATUS                     _IO(IO_TYPE_I2S, 0x14)
#define IO_IOCTL_I2S_WAIT_TX_EVENT                  _IO(IO_TYPE_I2S, 0x15)
#define IO_IOCTL_I2S_WAIT_RX_EVENT                  _IO(IO_TYPE_I2S, 0x16)
#define IO_IOCTL_I2S_CONFIG_SAI_TX_BUFFER              _IO(IO_TYPE_I2S, 0x17)
#define IO_IOCTL_I2S_CONFIG_SAI_RX_BUFFER              _IO(IO_TYPE_I2S, 0x18)
#define IO_IOCTL_I2S_START_TX                     _IO(IO_TYPE_I2S, 0x19)
#define IO_IOCTL_I2S_START_RX                     _IO(IO_TYPE_I2S, 0x1A)
#define IO_IOCTL_I2S_STOP_TX                      _IO(IO_TYPE_I2S, 0x1B)
#define IO_IOCTL_I2S_STOP_RX                      _IO(IO_TYPE_I2S, 0x1C)
#define IO_IOCTL_I2S_RESUME_TX                    _IO(IO_TYPE_I2S, 0x1D)
#define IO_IOCTL_I2S_RESUME_RX                    _IO(IO_TYPE_I2S, 0x1E)
#define IO_IOCTL_I2S_REG_TX_CALLBACK             _IO(IO_TYPE_I2S, 0x1F)
#define IO_IOCTL_I2S_REG_RX_CALLBACK             _IO(IO_TYPE_I2S, 0x20)
#define IO_IOCTL_I2S_GET_TX_OCCUPANCY         _IO(IO_TYPE_I2S, 0x21)
#define IO_IOCTL_I2S_GET_RX_OCCUPANCY         _IO(IO_TYPE_I2S, 0x22)

/* Callback function */
typedef enum
{
    SAI_TX_FIFO_ERROR = 0x1,
    SAI_TX_BUFFER_ERROR = 0x2,
    SAI_RX_FIFO_ERROR = 0x3,
    SAI_RX_BUFFER_ERROR = 0x4
} SAI_CALLBACK_EVENT;

typedef void (* SAI_CALLBACK)(void *param, SAI_CALLBACK_EVENT event);

typedef struct sai_callback_type
{
    SAI_CALLBACK callback;
    void *param;

} SAI_CALLBACK_TYPE;

/*--------------------------------------------------------------------------*/
/*
**                    DATATYPE DECLARATIONS
*/

/*
** I2S_STATISTICS_STRUCT
** SAI run time statistics structure
*/

typedef struct i2s_statistics_struct
{
    /* Number of SAI interrupts so far */
    uint32_t INTERRUPTS;	

    /* FIFO error */
    uint32_t FIFO_ERROR;

    /* FIFO sync error */
    uint32_t BUFFER_ERROR;

    /* Number of bytes currently read or wrote */
    uint32_t PACKETS_PROCESSED;

    /* Number of bytes currently buffered */
    uint32_t PACKETS_QUEUED;

    /* Number of bytes requested for reading or writing */
    uint32_t PACKETS_REQUESTED;

    /*Current buffer status*/
    uint8_t *IN_BUFFER;
    
    uint8_t *OUT_BUFFER;

    /*The size which can be written in, maybe this is no need*/
    uint32_t SIZE;

    /*The semaphore to tell the driver the buffer is filled*/	
    LWSEM_STRUCT BUFFER_EVENT;
 
} I2S_STATISTICS_STRUCT, *I2S_STATISTICS_STRUCT_PTR;

/*
 *The structure is used to re-configure the sai buffer for user applications
 * */
typedef struct sai_buffer_config
{
    /* The size of every buffer period */
    uint32_t PERIOD_SIZE;

    /* The period number */
    uint32_t PERIOD_NUMBER;
}SAI_BUFFER_CONFIG;

typedef struct sai_init_struct
{
    /* A void * to a string that identifies the device for fopen */
    char *               ID_PTR;

    _mqx_int (_CODE_PTR_    INIT)(void *, uint8_t);

    _mqx_int (_CODE_PTR_    DEINIT)(void *, uint8_t);

    /* The ioctl function to call */
    _mqx_int (_CODE_PTR_    IOCTL)(void *, _mqx_int, void *);
    
    /* The I/O channel initialization data */
    void *                 INIT_DATA_PTR;
} SAI_INIT_STRUCT, * SAI_INIT_STRUCT_PTR;

typedef const SAI_INIT_STRUCT * SAI_INIT_STRUCT_CPTR; 

/*
** FUNCTION PROTOTYPE
*/

#ifdef __cplusplus
extern "C" 
{
#endif

    _mqx_uint _io_sai_dma_install(SAI_INIT_STRUCT_CPTR);

#ifdef __cplusplus
}
#endif

#endif /* __SAI_H__ */

/* EOF */
