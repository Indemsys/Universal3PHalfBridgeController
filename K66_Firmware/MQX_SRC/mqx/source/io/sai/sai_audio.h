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

#ifndef __SAI_AUDIO_H__
#define __SAI_AUDIO_H__
#include <mqx.h>
/*--------------------------------------------------------------------------*/
/*
**                            CONSTANT DEFINITIONS
*/

/*
** IOCTL calls specific to audio devices
*/
#define IO_TYPE_AUDIO                           0x16
#define IO_IOCTL_AUDIO_SET_TX_DATA_FORMAT _IO   (IO_TYPE_AUDIO, 0x01)
#define IO_IOCTL_AUDIO_SET_RX_DATA_FORMAT _IO   (IO_TYPE_AUDIO, 0x02)
#define IO_IOCTL_AUDIO_GET_TX_DATA_FORMAT _IO   (IO_TYPE_AUDIO, 0x03)
#define IO_IOCTL_AUDIO_GET_RX_DATA_FORMAT _IO   (IO_TYPE_AUDIO, 0x04)


/*
** Audio deformat defaults
*/
#define AUDIO_DEFAULT_ENDIAN        AUDIO_BIG_ENDIAN
#define AUDIO_DEFAULT_ALIGNMENT     AUDIO_ALIGNMENT_RIGHT
#define AUDIO_DEFAULT_BITS          AUDIO_BIT_SIZE_MAX

/*
** Audio data aligment
*/
#define AUDIO_ALIGNMENT_RIGHT   0x00
#define AUDIO_ALIGNMENT_LEFT    0x01

/*
** Audio data endianity
*/
#define AUDIO_BIG_ENDIAN    MQX_BIG_ENDIAN
#define AUDIO_LITTLE_ENDIAN MQX_LITTLE_ENDIAN

/*
** Data bit size limits
*/
#define AUDIO_BIT_SIZE_MIN  8
#define AUDIO_BIT_SIZE_MAX  32

/*
** Audio error codes
*/
#define AUDIO_ERROR_INVALID_IO_FORMAT   (AUDIO_ERROR_BASE  | 0x01)

/*--------------------------------------------------------------------------*/
/*
**                    DATATYPE DECLARATIONS
*/

typedef struct audio_data_format
{
    /* Endian of input data */
    uint8_t ENDIAN;

    /* Aligment of input data */
    uint8_t ALIGNMENT;

    /* Bit size of input data */
    uint8_t BITS;

    /* Sample size in bytes */
    uint8_t SIZE;

    /* Number of channels */
    uint8_t CHANNELS;

    /* Sample rate */
    uint32_t SAMPLE_RATE;

} AUDIO_DATA_FORMAT, * AUDIO_DATA_FORMAT_PTR;

#endif /* __SAI_AUDIO_H__ */

/* EOF */
