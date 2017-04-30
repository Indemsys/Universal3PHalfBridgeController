#ifndef __part_mgr_h__
#define __part_mgr_h__
/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
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
*   This file contains internal data structures and constants
*   used to manage partitions.
*
*
*END************************************************************************/

#include "mfs_cnfg.h"

#if MFSCFG_USE_MUTEX
#include <mutex.h>
#define PMGR_LOCK(context) _mutex_lock(&(context)->MUTEX);
#define PMGR_UNLOCK(context) _mutex_unlock(&(context)->MUTEX);
#else
#define PMGR_LOCK(context) _lwsem_wait(&(context)->LWSEM);
#define PMGR_UNLOCK(context) _lwsem_post(&(context)->LWSEM);
#endif


/*
**  Defines constant for partiton
*/
#define  PMGR_VALID_STATE              1
#define  PMGR_INVALID_STATE            0

#define  PMGR_PARTITION_TABLE_OFFSET   0x01BE
#define  PMGR_MAX_PARTITIONS           0x04
#define  PMGR_ENTRY_SIZE               0x10

/* The 2 extra bytes store a validation signature */
#define  PMGR_PART_TABLE_SIZE          PMGR_ENTRY_SIZE * PMGR_MAX_PARTITIONS + 2


/*
** Partition types
*/
#define  PMGR_PARTITION_NOT_USED        0x00
#define  PMGR_PARTITION_FAT_12_BIT      0x01
#define  PMGR_PARTITION_FAT_16_BIT      0x04
#define  PMGR_PARTITION_HUGE            0x06
#define  PMGR_PARTITION_FAT32           0x0B
#define  PMGR_PARTITION_FAT32_LBA       0x0C
#define  PMGR_PARTITION_HUGE_LBA        0x0E


/* Structure of a partition entry on the disk */
typedef struct pmgr_part_entry_struct
{
   /* Active flag (0 = not bootable, 0x80 = bootable) */
   unsigned char     ACTIVE_FLAG;

   /* Starting head */
   unsigned char     START_HEAD;

   /* Starting sector/cylinder */
   unsigned char     START_SECT_CYL;

   /* Starting cylinder */
   unsigned char     START_CYLINDER;

   /* Partition type (0 not used, 1 FAT 12 bit, 4 FAT 16 bit, 5 extended,
   **                 6 huge - DOS 4.0+, 0x0B FAT32, 0x0C FAT32 LBA,
   **                 0x0E FAT16 LBA)
   */
   unsigned char     TYPE;

   /* Ending head */
   unsigned char     ENDING_HEAD;

   /* Ending sector/cylinder */
   unsigned char     ENDING_SECT_CYL;

   /* Ending cylinder */
   unsigned char     ENDING_CYLINDER;

   /* Starting sector for partition, relative to beginning of disk */
   unsigned char     START_SECTOR[4];

   /* Partition length in sectors */
   unsigned char     LENGTH[4];

} PMGR_PART_ENTRY_STRUCT, * PMGR_PART_ENTRY_STRUCT_PTR;


/* Structure passed to/from ioctl call MFS_IOCTL_GET_PARTITION and MFS_IOCTL_SET_PARTITION */
typedef struct pmgr_part_info_struct
{
   /* Partition slot (1 to 4) */
   unsigned char     SLOT;

   /* Partition type */
   unsigned char     TYPE;

   /* Active flag (0 = not bootable, 0x80 = bootable) */
   unsigned char     ACTIVE_FLAG;

   /* Padding */
   uint8_t    RESERVED;

   /* Starting sector for partition, relative to beginning of disk */
   uint32_t   START_SECTOR;

   /* Partition length in sectors */
   uint32_t   LENGTH;

   /* Heads per Cylinder */
   unsigned char     HEADS;

   /* Sectors per head */
   unsigned char     SECTORS;

   /* Cylinders on the device */
   uint16_t   CYLINDERS;

   int ERROR;

   long LOCATION;
} PMGR_PART_INFO_STRUCT, * PMGR_PART_INFO_STRUCT_PTR;


typedef struct part_mgr_struct
{
    MQX_FILE_PTR                     DEV_FILE_PTR;
#if MFSCFG_USE_MUTEX
    MUTEX_STRUCT                     MUTEX;
#else
    LWSEM_STRUCT                     LWSEM;
#endif
    uint32_t                         DEV_SECTOR_SIZE;
    uint32_t                         DEV_NUM_SECTORS;
    uint32_t                         ALIGNMENT;
    bool                             BLOCK_MODE;
    uint32_t                         INSTANCES;
} PART_MGR_STRUCT, * PART_MGR_STRUCT_PTR;

#ifdef __cplusplus
extern "C" {
#endif

int32_t _io_part_mgr_install(MQX_FILE_PTR dev_fd, char *identifier, uint32_t sector_size);
int32_t _io_part_mgr_open(MQX_FILE_PTR fd_ptr, char *open_name_ptr, char *flags_str);
int32_t _io_part_mgr_close(MQX_FILE_PTR fd_ptr);
int32_t _io_part_mgr_read(MQX_FILE_PTR file_ptr, char *data_ptr, int32_t num);
int32_t _io_part_mgr_write(MQX_FILE_PTR file_ptr, char *data_ptr, int32_t num);
int32_t _io_part_mgr_ioctl(MQX_FILE_PTR file_ptr, uint32_t cmd, uint32_t *param_ptr);
int32_t _io_part_mgr_uninstall(char *identifier);

int32_t _pmgr_check_part_type(uint32_t part_type);
int32_t _pmgr_validate_mbr(char *mbr_ptr);
int32_t _pmgr_get_part_info(PART_MGR_STRUCT_PTR part_mgr_ptr, PMGR_PART_INFO_STRUCT_PTR host_entry);
int32_t _pmgr_set_part_info(PART_MGR_STRUCT_PTR part_mgr_ptr, PMGR_PART_INFO_STRUCT_PTR host_entry);
int32_t _pmgr_clear_part_info(PART_MGR_STRUCT_PTR part_mgr_ptr, unsigned char part_num);
void _pmgr_disk_to_host(PMGR_PART_ENTRY_STRUCT_PTR disk_entry, PMGR_PART_INFO_STRUCT_PTR part_entry);
void _pmgr_host_to_disk(PMGR_PART_INFO_STRUCT_PTR part_entry, PMGR_PART_ENTRY_STRUCT_PTR disk_entry);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
