#ifndef __esdhc_prv_h__
#define __esdhc_prv_h__
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
*   This file contains definitions private to the ESDHC driver.
*
*
*END************************************************************************/

#include "esdhc.h"
#include "lwevent.h"


/*--------------------------------------------------------------------------*/
/*
**                    CONSTANT DEFINITIONS
*/

#define IO_ESDHC_ATTRIBS (IO_DEV_ATTR_READ | IO_DEV_ATTR_REMOVE | IO_DEV_ATTR_SEEK | IO_DEV_ATTR_WRITE | IO_DEV_ATTR_BLOCK_MODE)

#define ESDHC_PROCTL_EMODE_BIG                0x00
#define ESDHC_PROCTL_EMODE_LITTLE             0x02

#define ESDHC_PROCTL_DTW_1BIT                 0x00
#define ESDHC_PROCTL_DTW_4BIT                 0x01
#define ESDHC_PROCTL_DTW_8BIT                 0x10

#define ESDHC_ADMA2_FLAG_VALID                0x01
#define ESDHC_ADMA2_FLAG_END                  0x02
#define ESDHC_ADMA2_FLAG_INT                  0x04
#define ESDHC_ADMA2_FLAG_TRAN                 0x20
#define ESDHC_ADMA2_FLAG_LINK                 0x30

#define ESDHC_CMD_TICK_TIMEOUT                20  // 40ms?
#define ESDHC_CMD12_TICK_TIMEOUT              200 //500ms
#define ESDHC_TRANSFER_TIMEOUT_MS             750 //750ms

#define ESDHC_LWEVENT_CMD_DONE                0x00000001
#define ESDHC_LWEVENT_CMD_ERROR               0x00000002
#define ESDHC_LWEVENT_CMD_TIMEOUT             0x00000004
#define ESDHC_LWEVENT_TRANSFER_DONE           0x00000008
#define ESDHC_LWEVENT_TRANSFER_ERROR          0x00000010
#define ESDHC_LWEVENT_TRANSFER_TIMEOUT        0x00000020

#ifndef ESDHC_IS_HANDLING_CACHE
  #define ESDHC_IS_HANDLING_CACHE PSP_HAS_DATA_CACHE
#endif

#ifndef BSP_ESDHC_INT_LEVEL
  #define BSP_ESDHC_INT_LEVEL 4 // default value to achieve compatibility in older BSPs
#endif

#ifndef ESDHC_CARD_DETECTION_SUPPORT
  #define ESDHC_CARD_DETECTION_SUPPORT       (0)
#endif

#ifndef ESDHC_AUTO_CLK_GATING
  #define ESDHC_AUTO_CLK_GATING              (1)
#endif

/*--------------------------------------------------------------------------*/
/*
**                    DATATYPE DECLARATIONS
*/

/*
** ESDHC_DEVICE_STRUCT
*/

typedef struct esdhc_adma2_desc
{
    uint32_t LEN_ATTR;
    uint32_t DATA_ADDR;
} ESDHC_ADMA2_DESC;


typedef struct esdhc_adma2_data
{
    uint8_t HEAD_BUF[PSP_MEMORY_ALIGNMENT+1];
    uint8_t TAIL_BUF[PSP_MEMORY_ALIGNMENT+1];
    ESDHC_ADMA2_DESC DESC[3];
} ESDHC_ADMA2_DATA;


typedef struct esdhc_device_struct
{
    /* The current init values for this device */
    ESDHC_INIT_STRUCT_CPTR       INIT;

    /* The number of opened file descriptors */
    uint32_t                     COUNT;

    /* The actual card status */
    uint32_t                     CARD;

    /* Interrupt vector of the ESDHC controller */
    uint32_t                     VECTOR;

    /* ESDHC registers (base address) */
    ESDHC_REG_STRUCT_PTR         ESDHC_REG_PTR;

    /* The pointer to callback and data for IO card interrupt*/
    ESDHC_IO_INT_CALLBACK_STRUCT         IO_CALLBACK_STR;

#if ESDHC_CARD_DETECTION_SUPPORT
    /* The pointer to callback and data for card presence change*/
    ESDHC_CARD_PRESENCE_CALLBACK_STRUCT  CARD_PRESENCE_CALLBACK_STR;
#endif

    /* Buffers and descriptors for ADMA2 are dynamically allocated to be properly aligned */
    ESDHC_ADMA2_DATA             *ADMA2_DATA;

    /* The buffered command for data operations */
    ESDHC_COMMAND_STRUCT         BUFFERED_CMD;

    /* Semaphore signalled from ISR when to notify about job done */
    LWSEM_STRUCT                 EVENT_IO_FINISHED;

    /* LightWeight Events to manage the interrupt & DMA style of driver */
    LWEVENT_STRUCT               LWEVENT;

} ESDHC_DEVICE_STRUCT, * ESDHC_DEVICE_STRUCT_PTR;


/*--------------------------------------------------------------------------*/
/*
**                        FUNCTION PROTOTYPES
*/

#ifdef __cplusplus
extern "C" {
#endif

extern _mqx_int _esdhc_open (MQX_FILE_PTR, char *, char *);
extern _mqx_int _esdhc_close (MQX_FILE_PTR);
extern _mqx_int _esdhc_read (MQX_FILE_PTR, char *, int32_t);
extern _mqx_int _esdhc_write (MQX_FILE_PTR, char *, int32_t);
extern _mqx_int _esdhc_ioctl (MQX_FILE_PTR, uint32_t, void *);
extern _mqx_int _esdhc_uninstall (IO_DEVICE_STRUCT_PTR);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
