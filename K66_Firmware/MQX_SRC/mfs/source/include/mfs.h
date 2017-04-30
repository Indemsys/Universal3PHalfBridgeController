#ifndef __mfs_h__
#define __mfs_h__
/*HEADER**********************************************************************
*
* Copyright 2008-2014 Freescale Semiconductor, Inc.
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
*   This file contains the structure definitions and constants for a
*   user who will be compiling programs that will use the Embedded MS-DOS
*   File System (MFS)
*
*
*END************************************************************************/

#include <mqx.h>

#if MQX_USE_IO_OLD
#include <bsp.h>
#include <io.h>
#include <ioctl.h>
#include <fio.h>
#define MFS_FD_TYPE MQX_FILE_PTR

#else //NIO
#include <nio.h>
#define MFS_FD_TYPE int
#endif

#include <stdint.h>
#include <stdbool.h>

#include "mfs_rev.h"
#include "mfs_cnfg.h"


/*
** Defines specific to MFS
*/

#define MFS_VERSION             (0x00030000 | MFS_REVISION)

/* search attributes  */
#define  MFS_SEARCH_NORMAL      0x00
#define  MFS_SEARCH_READ_ONLY   0x01
#define  MFS_SEARCH_HIDDEN      0x02
#define  MFS_SEARCH_SYSTEM      0x04
#define  MFS_SEARCH_VOLUME      0x08
#define  MFS_SEARCH_SUBDIR      0x10
#define  MFS_SEARCH_ARCHIVE     0x20
#define  MFS_SEARCH_EXCLUSIVE   0x40
#define  MFS_SEARCH_ANY         0x80
#define  MFS_SEARCH_LFN        0x100

/* file open access modes */
#define  MFS_ACCESS_READ_ONLY   0x00
#define  MFS_ACCESS_WRITE_ONLY  0x01
#define  MFS_ACCESS_READ_WRITE  0x02

/* file entry attributes */
#define  MFS_ATTR_READ_ONLY     0x01
#define  MFS_ATTR_HIDDEN_FILE   0x02
#define  MFS_ATTR_SYSTEM_FILE   0x04
#define  MFS_ATTR_VOLUME_NAME   0x08
#define  MFS_ATTR_DIR_NAME      0x10
#define  MFS_ATTR_ARCHIVE       0x20

/* mask values for the date and time as recorded in the
** file entries
*/
#define  MFS_MASK_DAY           0x001F
#define  MFS_MASK_MONTH         0x01E0
#define  MFS_MASK_YEAR          0xFE00
#define  MFS_MASK_SECONDS       0x001F
#define  MFS_MASK_MINUTES       0x07E0
#define  MFS_MASK_HOURS         0xF800

/*
** shift for the above date and time masks
*/
#define  MFS_SHIFT_DAY          0
#define  MFS_SHIFT_MONTH        5
#define  MFS_SHIFT_YEAR         9
#define  MFS_SHIFT_SECONDS      0
#define  MFS_SHIFT_MINUTES      5
#define  MFS_SHIFT_HOURS        11

/*
** control for the write cache - deprecated
*/
#define  MFS_WRCACHE_OFF        0
#define  MFS_WRCACHE_ON         1
#define  MFS_WRCACHE_FLUSH      2

#define  MFS_DEFAULT_SECTOR_SIZE       512

#define MFS_FAT12             0x01
#define MFS_FAT16             0x06
#define MFS_FAT32             0x0C

/*
** FILENAME LENGTH CONSTANTS
*/
#define PATHNAME_SIZE         260
#define FILENAME_SIZE         255
#define SFILENAME_SIZE        12

/*
** error codes
*/
#define MFS_NO_ERROR                          FS_NO_ERROR
#define MFS_INVALID_FUNCTION_CODE             FS_INVALID_FUNCTION_CODE
#define MFS_FILE_NOT_FOUND                    FS_FILE_NOT_FOUND
#define MFS_PATH_NOT_FOUND                    FS_PATH_NOT_FOUND
#define MFS_ACCESS_DENIED                     FS_ACCESS_DENIED
#define MFS_INVALID_HANDLE                    FS_INVALID_HANDLE
#define MFS_INSUFFICIENT_MEMORY               FS_INSUFFICIENT_MEMORY
#define MFS_INVALID_MEMORY_BLOCK_ADDRESS      FS_INVALID_MEMORY_BLOCK_ADDRESS
#define MFS_ATTEMPT_TO_REMOVE_CURRENT_DIR     FS_ATTEMPT_TO_REMOVE_CURRENT_DIR
#define MFS_DISK_IS_WRITE_PROTECTED           FS_DISK_IS_WRITE_PROTECTED
#define MFS_BAD_DISK_UNIT                     FS_BAD_DISK_UNIT
#define MFS_INVALID_LENGTH_IN_DISK_OPERATION  FS_INVALID_LENGTH_IN_DISK_OPERATION
#define MFS_NOT_A_DOS_DISK                    FS_NOT_A_DOS_DISK
#define MFS_SECTOR_NOT_FOUND                  FS_SECTOR_NOT_FOUND
#define MFS_WRITE_FAULT                       FS_WRITE_FAULT
#define MFS_READ_FAULT                        FS_READ_FAULT
#define MFS_SHARING_VIOLATION                 FS_SHARING_VIOLATION
#define MFS_FILE_EXISTS                       FS_FILE_EXISTS
#define MFS_ALREADY_ASSIGNED                  FS_ALREADY_ASSIGNED
#define MFS_INVALID_PARAMETER                 FS_INVALID_PARAMETER
#define MFS_DISK_FULL                         FS_DISK_FULL
#define MFS_ROOT_DIR_FULL                     FS_ROOT_DIR_FULL
#define MFS_EOF                               FS_EOF
#define MFS_CANNOT_CREATE_DIRECTORY           FS_CANNOT_CREATE_DIRECTORY
#define MFS_NOT_INITIALIZED                   FS_NOT_INITIALIZED
#define MFS_OPERATION_NOT_ALLOWED             FS_OPERATION_NOT_ALLOWED
#define MFS_ERROR_INVALID_DRIVE_HANDLE        FS_ERROR_INVALID_DRIVE_HANDLE
#define MFS_ERROR_INVALID_FILE_HANDLE         FS_ERROR_INVALID_FILE_HANDLE
#define MFS_ERROR_UNKNOWN_FS_VERSION          FS_ERROR_UNKNOWN_FS_VERSION
#define MFS_LOST_CHAIN                        FS_LOST_CHAIN
#define MFS_INVALID_DEVICE                    FS_INVALID_DEVICE
#define MFS_INVALID_CLUSTER_NUMBER            FS_INVALID_CLUSTER_NUMBER
#define MFS_FAILED_TO_DELETE_LFN              FS_FAILED_TO_DELETE_LFN
#define MFS_BAD_LFN_ENTRY                     FS_BAD_LFN_ENTRY

#define MFS_IO_OPERATION_NOT_AVAILABLE        MQX_IO_OPERATION_NOT_AVAILABLE
#define MFS_OUT_OF_MEMORY                     MQX_OUT_OF_MEMORY
#define MFS_INVALID_POINTER                   MQX_INVALID_POINTER

#define MFS_ERROR                             (-1)
#define MFS_ERROR_SEEK                        MFS_ERROR
#define MFS_ERROR_DEVICE_BUSY                 MFS_ERROR
#define MFS_OK                                (0)

#define PMGR_INVALID_PARTITION                FS_PMGR_INVALID_PARTITION
#define PMGR_INSUF_MEMORY                     FS_PMGR_INSUF_MEMORY
#define PMGR_UNKNOWN_PARTITION                FS_PMGR_UNKNOWN_PARTITION
#define PMGR_INVALID_PARTTABLE                FS_PMGR_INVALID_PARTTABLE



/*
** Extra MFS_IOCTL codes
*/
#define MFS_IOCTL_BASE 0x100
#define IO_IOCTL_DELETE_FILE                  (MFS_IOCTL_BASE + 0x02)
#define IO_IOCTL_BAD_CLUSTERS                 (MFS_IOCTL_BASE + 0x03)
#define IO_IOCTL_FREE_SPACE                   (MFS_IOCTL_BASE + 0x04)
#define IO_IOCTL_TEST_UNUSED_CLUSTERS         (MFS_IOCTL_BASE + 0x05)
#define IO_IOCTL_CREATE_SUBDIR                (MFS_IOCTL_BASE + 0x06)
#define IO_IOCTL_REMOVE_SUBDIR                (MFS_IOCTL_BASE + 0x07)
#define IO_IOCTL_GET_CURRENT_DIR              (MFS_IOCTL_BASE + 0x08)
#define IO_IOCTL_CHANGE_CURRENT_DIR           (MFS_IOCTL_BASE + 0x09)
#define IO_IOCTL_WRITE_CACHE_ON               (MFS_IOCTL_BASE + 0x0A)
#define IO_IOCTL_WRITE_CACHE_OFF              (MFS_IOCTL_BASE + 0x0B)
#define IO_IOCTL_FORMAT                       (MFS_IOCTL_BASE + 0x0C)
#define IO_IOCTL_FORMAT_TEST                  (MFS_IOCTL_BASE + 0x0D)
#define IO_IOCTL_FIND_FIRST_FILE              (MFS_IOCTL_BASE + 0x0E)
#define IO_IOCTL_FIND_NEXT_FILE               (MFS_IOCTL_BASE + 0x0F)
#define IO_IOCTL_RENAME_FILE                  (MFS_IOCTL_BASE + 0x10)
#define IO_IOCTL_GET_FILE_ATTR                (MFS_IOCTL_BASE + 0x11)
#define IO_IOCTL_SET_FILE_ATTR                (MFS_IOCTL_BASE + 0x12)
#define IO_IOCTL_GET_DATE_TIME                (MFS_IOCTL_BASE + 0x13)
#define IO_IOCTL_SET_DATE_TIME                (MFS_IOCTL_BASE + 0x14)
#define IO_IOCTL_FLUSH_FAT                    (MFS_IOCTL_BASE + 0x15)
#define IO_IOCTL_FAT_CACHE_ON                 (MFS_IOCTL_BASE + 0x16)
#define IO_IOCTL_FAT_CACHE_OFF                (MFS_IOCTL_BASE + 0x17)
#define IO_IOCTL_SET_VOLUME                   (MFS_IOCTL_BASE + 0x18)
#define IO_IOCTL_GET_VOLUME                   (MFS_IOCTL_BASE + 0x19)
#define IO_IOCTL_GET_LFN                      (MFS_IOCTL_BASE + 0x1A)
#define IO_IOCTL_FREE_CLUSTERS                (MFS_IOCTL_BASE + 0x1B)
#define IO_IOCTL_GET_CLUSTER_SIZE             (MFS_IOCTL_BASE + 0x1C)
#define IO_IOCTL_LAST_CLUSTER                 (MFS_IOCTL_BASE + 0x20)
#define IO_IOCTL_SEL_PART                     (MFS_IOCTL_BASE + 0x21)
#define IO_IOCTL_USE_PARTITION                (MFS_IOCTL_BASE + 0x22)
#define IO_IOCTL_VAL_PART                     (MFS_IOCTL_BASE + 0x23)
#define IO_IOCTL_SET_PARTITION                (MFS_IOCTL_BASE + 0x24)
#define IO_IOCTL_GET_PARTITION                (MFS_IOCTL_BASE + 0x25)
#define IO_IOCTL_CLEAR_PARTITION              (MFS_IOCTL_BASE + 0x26)
#define IO_IOCTL_GET_DEVICE_HANDLE            (MFS_IOCTL_BASE + 0x27)
#define IO_IOCTL_GET_FAT_CACHE_MODE           (MFS_IOCTL_BASE + 0x28)
#define IO_IOCTL_SET_FAT_CACHE_MODE           (MFS_IOCTL_BASE + 0x29)
#define IO_IOCTL_GET_WRITE_CACHE_MODE         (MFS_IOCTL_BASE + 0x30)
#define IO_IOCTL_SET_WRITE_CACHE_MODE         (MFS_IOCTL_BASE + 0x31)
#define IO_IOCTL_DEFAULT_FORMAT               (MFS_IOCTL_BASE + 0x32)
#define IO_IOCTL_VERIFY_WRITES                (MFS_IOCTL_BASE + 0x33)
#define IO_IOCTL_CHECK_DIR_EXIST              (MFS_IOCTL_BASE + 0x34)
#define IO_IOCTL_CHECK_FORMATTED              (MFS_IOCTL_BASE + 0x35)

/* Legacy, not implemented */
#define IO_IOCTL_GET_DRIVE_PARAMS             (MFS_IOCTL_BASE + 0x37)

/* Element defines for ID array */
#define MFS_IOCTL_ID_PHY_ELEMENT          (0)
#define MFS_IOCTL_ID_LOG_ELEMENT          (1)
#define MFS_IOCTL_ID_ATTR_ELEMENT         (2)

/*
Following IOCTL codes are obsolete.
No device locking is necessary anymore as partition manager may be open multiple times to allow for concurrent access to multiple partitions.
#define MFS_IOCTL_DEV_LOCK
#define MFS_IOCTL_DEV_UNLOCK
*/


#define MFS_MEM_TYPE_BASE                     ( (IO_MFS_COMPONENT) << (MEM_TYPE_COMPONENT_SHIFT))
#define MEM_TYPE_MFS_OTHER                    (MFS_MEM_TYPE_BASE+0)
#define MEM_TYPE_MFS_DRIVE_STRUCT             (MFS_MEM_TYPE_BASE+1)
#define MEM_TYPE_MFS_DIR_STRUCT               (MFS_MEM_TYPE_BASE+2)
#define MEM_TYPE_MFS_DATA_SECTOR              (MFS_MEM_TYPE_BASE+3)
#define MEM_TYPE_MFS_DIRECTORY_SECTOR         (MFS_MEM_TYPE_BASE+4)
#define MEM_TYPE_MFS_FAT_BUFFER               (MFS_MEM_TYPE_BASE+5)
#define MEM_TYPE_MFS_FILESYSTEM_INFO_DISK     (MFS_MEM_TYPE_BASE+6)
#define MEM_TYPE_MFS_CLUSTER                  (MFS_MEM_TYPE_BASE+7)
#define MEM_TYPE_MFS_PATHNAME                 (MFS_MEM_TYPE_BASE+8)
#define MEM_TYPE_PART_MGR_STRUCT              (MFS_MEM_TYPE_BASE+9)
#define MEM_TYPE_PART_MGR_SECTOR              (MFS_MEM_TYPE_BASE+10)
#define MEM_TYPE_PART_INFO_STRUCT             (MFS_MEM_TYPE_BASE+11)


/*
** Data Structures specific to MFS
*/
typedef uint32_t _mfs_error;
typedef uint32_t *_mfs_error_ptr;

typedef enum {
   MFS_WRITE_THROUGH_CACHE=0,    // No write caching (only read caching)
   MFS_MIXED_MODE_CACHE=1,       // Write Caching allowed on file write only
   MFS_WRITE_BACK_CACHE=2        // Write Caching fully enabled
} _mfs_cache_policy;

/*
** Information required for high-level format
*/
typedef struct mfs_format_data
{
   unsigned char    PHYSICAL_DRIVE;
   unsigned char    MEDIA_DESCRIPTOR;
   uint16_t  BYTES_PER_SECTOR;
   uint16_t  SECTORS_PER_TRACK;
   uint16_t  NUMBER_OF_HEADS;
   uint32_t  NUMBER_OF_SECTORS;
   uint32_t  HIDDEN_SECTORS;
   uint16_t  RESERVED_SECTORS;
} MFS_FORMAT_DATA, * MFS_FORMAT_DATA_PTR;

/*
** Format information for ioctl calls involving format operations
*/
typedef struct mfs_ioctl_format
{
   MFS_FORMAT_DATA_PTR     FORMAT_PTR;
   uint32_t               *COUNT_PTR;      /* To count the bad clusters */
} MFS_IOCTL_FORMAT_PARAM, * MFS_IOCTL_FORMAT_PARAM_PTR;


/* FAT chain abstraction structure */
typedef struct fat_chain
{
    uint32_t HEAD_CLUSTER;
    uint32_t SEGMENT_POS;
    uint32_t SEGMENT_SECTOR_NUM;
    uint32_t SEGMENT_SECTOR_COUNT;
    uint32_t SEGMENT_LAST_CLUSTER;
} FAT_CHAIN;


typedef struct mfs_internal_search
{
   char      *WILDCARD;   
   uint8_t    ATTR_ONE_MASK;
   uint8_t    ATTR_ZERO_MASK;
   FAT_CHAIN  DIR_CHAIN;   
   uint32_t   DIR_CHAIN_LOC;   
} MFS_INTERNAL_SEARCH, * MFS_INTERNAL_SEARCH_PTR;

/*
** Search data block, used for find_first and find_next
*/
typedef struct mfs_search_data
{
   void                *DRIVE_PTR;
   MFS_INTERNAL_SEARCH  INTERNAL_SEARCH_DATA;
   uint32_t             ATTRIBUTE;
   uint16_t             TIME;
   uint16_t             DATE;
   uint32_t             FILE_SIZE;
   char                 NAME[24];  // max. length of SFN encoded using UTF-8
   char                *LFN_BUF;
   int                  LFN_BUF_LEN;
} MFS_SEARCH_DATA, * MFS_SEARCH_DATA_PTR;


/*
** Search information for ioctl find file first search calls
*/
typedef struct mfs_search_param
{
   uint32_t             ATTRIBUTE;
   char                *WILDCARD;
   char                *LFN_BUF;
   uint32_t             LFN_BUF_LEN;
   MFS_SEARCH_DATA_PTR  SEARCH_DATA_PTR;
} MFS_SEARCH_PARAM, * MFS_SEARCH_PARAM_PTR;


/*
** Structure used to pass parameters to get_lfn ioctl call
*/
typedef struct mfs_get_lfn_struct
{
   char                *PATHNAME;
   char                *LONG_FILENAME;
} MFS_GET_LFN_STRUCT, * MFS_GET_LFN_STRUCT_PTR;


/*
** Pathname information for the rename_file ioctl call
*/
typedef struct mfs_rename_param
{
   char                *OLD_PATHNAME;
   char                *NEW_PATHNAME;
} MFS_RENAME_PARAM, * MFS_RENAME_PARAM_PTR;


/*
** Paramater information for get and set file attribute ioctl calls
*/
typedef struct mfs_file_attr_param
{
   char                *PATHNAME;
   unsigned char       *ATTRIBUTE_PTR;
} MFS_FILE_ATTR_PARAM, * MFS_FILE_ATTR_PARAM_PTR;


/*
** Parameter information for get/set date time ioctl calls
*/
typedef struct mfs_date_time_param
{
   uint16_t             *DATE_PTR;
   uint16_t             *TIME_PTR;
} MFS_DATE_TIME_PARAM, * MFS_DATE_TIME_PARAM_PTR;


/*
** extern statements for MFS
*/

extern _mem_pool_id _MFS_pool_id;
extern uint32_t _MFS_handle_pool_init;
extern uint32_t _MFS_handle_pool_grow;
extern uint32_t _MFS_handle_pool_max;

#ifdef __cplusplus
extern "C" {
#endif

int       _io_mfs_install(MFS_FD_TYPE dev_fd, char *identifier, uint32_t part_num);
int       _io_mfs_uninstall(char *identifier);

void      *_io_mfs_dir_open(MFS_FD_TYPE fs_ptr, char *wildcard_ptr, char *mode_ptr);
int32_t   _io_mfs_dir_read(void *dir, char *buffer, uint32_t size);
int32_t   _io_mfs_dir_close(void *dir);

char      *MFS_Error_text(uint32_t error_code);

_mfs_error MFS_alloc_path(char ** path_ptr_ptr);
_mfs_error MFS_alloc_2paths(char ** path1_ptr_ptr,char ** path2_ptr_ptr);
bool       MFS_free_path(char *path_ptr);

#ifdef __cplusplus
}
#endif

#if (MFSCFG_NUM_OF_FATS == 0)
#error "Illegal number of FATS"
#endif

#undef MFS_FD_TYPE

#endif
/* EOF */
