#ifndef _apcflshpr_h_
#define _apcflshpr_h_
/*HEADER**********************************************************************
*
* Copyright 2011 Freescale Semiconductor, Inc.
* Copyright 2004-2008 Embedded Access Inc.
* Copyright 1989-2008 ARC International
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
*   definitions private to the advanced PC Card flash drivers
*
*
*END************************************************************************/

#include "bsp.h"
#include "ata.h"

/* Default values used unless definition is provided by apccard layer */
#ifndef APCCARD_BUS_WIDTH
#define APCCARD_BUS_WIDTH     1
#endif

#ifndef APCCARD_ADDR_SHIFT
#define APCCARD_ADDR_SHIFT    0
#endif

/* Register offsets taking address bus shift into account */
#define PCFLASH_REG_BASE            (ATA_REG_BASE << APCCARD_ADDR_SHIFT)
#define PCFLASH_REG_ERROR           (ATA_ERROR << APCCARD_ADDR_SHIFT)
#define PCFLASH_REG_FEATURES        (ATA_FEATURES << APCCARD_ADDR_SHIFT)
#define PCFLASH_REG_SECTOR_COUNT    (ATA_SECTOR_COUNT << APCCARD_ADDR_SHIFT)
#define PCFLASH_REG_SECTOR          (ATA_SECTOR << APCCARD_ADDR_SHIFT)
#define PCFLASH_REG_CYLINDER_LOW    (ATA_CYLINDER_LOW << APCCARD_ADDR_SHIFT)
#define PCFLASH_REG_CYLINDER_HIGH   (ATA_CYLINDER_HIGH << APCCARD_ADDR_SHIFT)
#define PCFLASH_REG_CARD_DRIVE_HEAD (ATA_CARD_DRIVE_HEAD << APCCARD_ADDR_SHIFT)
#define PCFLASH_REG_STATUS          (ATA_STATUS << APCCARD_ADDR_SHIFT)
#define PCFLASH_REG_COMMAND         (ATA_COMMAND << APCCARD_ADDR_SHIFT)
#define PCFLASH_REG_DUP_RDE_DATA    (ATA_DUP_RDE_DATA << APCCARD_ADDR_SHIFT)
#define PCFLASH_REG_DUP_RDO_DATA    (ATA_DUP_RDO_DATA << APCCARD_ADDR_SHIFT)
#define PCFLASH_REG_ALT_STATUS_CMD  (ATA_ALT_STATUS_CMD << APCCARD_ADDR_SHIFT)
#define PCFLASH_REG_DEV_CNTRL       (ATA_DEV_CNTRL << APCCARD_ADDR_SHIFT)
#define PCFLASH_REG_DATA_BUF        (ATA_DATA_BUF << APCCARD_ADDR_SHIFT)


#define PCFLASH_REG16_SECTOR        (ATA_REG16_SECTOR << APCCARD_ADDR_SHIFT)
#define PCFLASH_REG16_CYLINDER      (ATA_REG16_CYLINDER << APCCARD_ADDR_SHIFT)
#define PCFLASH_REG16_DHC           (ATA_REG16_DHC << APCCARD_ADDR_SHIFT)
#define PCFLASH_REG16_STATUS        (ATA_REG16_STATUS << APCCARD_ADDR_SHIFT)
#define PCFLASH_REG16_DATA          (ATA_REG16_DATA << APCCARD_ADDR_SHIFT)
#define PCFLASH_REG16_ERROR         (ATA_REG16_ERROR << APCCARD_ADDR_SHIFT)
#define PCFLASH_REG16_DEV_CNTRL     (ATA_REG16_DEV_CNTRL << APCCARD_ADDR_SHIFT)


/*----------------------------------------------------------------------*/
/*
**                          CONSTANT DEFINITIONS
*/

/* Error codes from lower layers */
#define PCFLASH_NO_ERROR             (0)
#define PCFLASH_ERROR_NO_CARD        (PCFLASH_ERROR_BASE | 0x01)
#define PCFLASH_INVALID_SECTOR       (PCFLASH_ERROR_BASE | 0x02)
#define PCFLASH_INVALID_CARD         (PCFLASH_ERROR_BASE | 0x03)
#define PCFLASH_INVALID_DRIVE        (PCFLASH_ERROR_BASE | 0x04)
#define PCFLASH_INVALID_VOLTAGE      (PCFLASH_ERROR_BASE | 0x05)
#define PCFLASH_INVALID_HANDLE       (PCFLASH_ERROR_BASE | 0x06)
#define PCFLASH_HARDWARE_ERROR       (PCFLASH_ERROR_BASE | 0x07)
#define PCFLASH_READ_ERROR           (PCFLASH_ERROR_BASE | 0x08)
#define PCFLASH_WRITE_ERROR          (PCFLASH_ERROR_BASE | 0x09)

/*----------------------------------------------------------------------*/
/*
**                    DATATYPE DEFINITIONS
*/


/*
** PC_FLASH_INFO_STRUCT
** Run time state information for each PC Card Flash device
*/
typedef struct io_pcflash_info_struct
{
   /* Handle for PCCard calls */
   MQX_FILE_PTR  PCCARD_STREAM;

   /* Drive number to associate with this slot */
   uint32_t       DRIVE;

   /* Sector size in bytes */
   uint32_t       SECTOR_SIZE;

   /* The total number of sectors in the device */
   uint32_t       NUM_SECTORS;

   /* Total size of flash card in bytes */
   uint32_t       SIZE;

   /* The number of heads as reported by the ATA ident command */
   uint32_t       NUMBER_OF_HEADS;

   /* The number of tracks as reported by the ATA ident command */
   uint32_t       NUMBER_OF_TRACKS;

   /* The number of sectos per cylinder as reported by the ATA ident command */
   uint32_t       SECTORS_PER_TRACK;

   /* ATA Register location */
   unsigned char     *ATA_REG_PTR;

   /* ATA Data Register location */
   uint16_t   *ATA_DATA_PTR;

   /* Light weight semaphore struct */
   LWSEM_STRUCT  LWSEM;

   /* The address of temp buffer */
   unsigned char     *TEMP_BUFF_PTR;

   /* The current error code for the device */
   uint32_t       ERROR_CODE;

   /* Indicates if the device is running in block mode or character mode */
   bool       BLOCK_MODE;


} IO_PCFLASH_STRUCT, * IO_PCFLASH_STRUCT_PTR;

#ifdef __cplusplus
extern "C" {
#endif

extern _mqx_int _io_apcflash_open(FILE_DEVICE_STRUCT_PTR, char *, char *);
extern _mqx_int _io_apcflash_close(FILE_DEVICE_STRUCT_PTR);
extern _mqx_int _io_apcflash_read (FILE_DEVICE_STRUCT_PTR, char *, _mqx_int);
extern _mqx_int _io_apcflash_write(FILE_DEVICE_STRUCT_PTR, char *, _mqx_int);
extern _mqx_int _io_apcflash_ioctl(FILE_DEVICE_STRUCT_PTR, _mqx_uint, void *);

extern uint32_t _io_apcflash_read_sector(IO_PCFLASH_STRUCT_PTR, uint32_t,
   unsigned char *);
extern int32_t  _io_apcflash_read_partial_sector(IO_PCFLASH_STRUCT_PTR, uint32_t,
   uint32_t, uint32_t, unsigned char *);
extern uint32_t _io_apcflash_write_sector(IO_PCFLASH_STRUCT_PTR, uint32_t,
   unsigned char *);
extern int32_t  _io_apcflash_write_partial_sector(IO_PCFLASH_STRUCT_PTR,
   uint32_t, uint32_t, uint32_t, unsigned char *);
extern uint32_t _io_apcflash_identify_device(IO_PCFLASH_STRUCT_PTR, unsigned char *);
extern uint32_t _io_apcflash_reset(IO_PCFLASH_STRUCT_PTR);
extern bool _io_apcflash_status_timeout(IO_PCFLASH_STRUCT_PTR, uint32_t, unsigned char);
/* Start CR 812 */
extern _mqx_int _io_apcflash_read_write_blocks(FILE_DEVICE_STRUCT_PTR,
   IO_PCFLASH_STRUCT_PTR, char *, _mqx_int, bool);
/* End   CR 812 */

#ifdef __cplusplus
}
#endif

#endif
