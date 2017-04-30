#ifndef __sdcard_prv_h__
#define __sdcard_prv_h__
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
*   The file contains functions prototype, defines, structure 
*   definitions private to the SD card.
*
*
*END************************************************************************/


#include "sdcard.h"


/*----------------------------------------------------------------------*/
/*
**                          CONSTANT DEFINITIONS
*/


/* Properties of device */
#define IO_SDCARD_ATTRIBS  (IO_DEV_ATTR_READ | IO_DEV_ATTR_REMOVE | IO_DEV_ATTR_SEEK | IO_DEV_ATTR_WRITE | IO_DEV_ATTR_BLOCK_MODE)


/*----------------------------------------------------------------------*/
/*
**                    DATATYPE DEFINITIONS
*/

/*
** IO_SDCARD STRUCT
**
** The address of this structure is used to maintain SD card specific 
** information.
*/
typedef struct io_sdcard_struct
{
   /* Init struct pointer */
   SDCARD_INIT_STRUCT_PTR INIT;

   /* The low level SD communication file descriptor */
   MQX_FILE_PTR           COM_DEVICE;

   /* Light weight semaphore struct */
   LWSEM_STRUCT           LWSEM;

   /* The low level response timeout >= 250 ms */
   uint32_t               TIMEOUT;

   /* The number of blocks for the device */
   uint32_t               NUM_BLOCKS;

   /* High capacity = block addressing */
   bool                   SDHC;

   /* Specification 2 or later card = different CSD register */
   bool                   VERSION2;

   /* Card address */
   uint32_t               ADDRESS;

   /* Required buffer alignment for read/write operations */
   uint32_t               ALIGNMENT;

   /* Card identification */
   uint8_t                CID[16];

} SDCARD_STRUCT, * SDCARD_STRUCT_PTR;


/* Internal functions to SD */
#ifdef __cplusplus
extern "C" {
#endif

extern _mqx_int _io_sdcard_open(MQX_FILE_PTR, char *, char *);
extern _mqx_int _io_sdcard_close(MQX_FILE_PTR);
extern _mqx_int _io_sdcard_write_blocks(MQX_FILE_PTR, char *, _mqx_int);
extern _mqx_int _io_sdcard_read_blocks (MQX_FILE_PTR, char *, _mqx_int);
extern _mqx_int _io_sdcard_ioctl(MQX_FILE_PTR, _mqx_uint, void *);

extern uint32_t  _io_sdcard_csd_capacity(uint8_t *);
extern uint32_t  _io_sdcard_csd_baudrate(uint8_t *);

#ifdef __cplusplus
}
#endif

#endif

/* EOF */
