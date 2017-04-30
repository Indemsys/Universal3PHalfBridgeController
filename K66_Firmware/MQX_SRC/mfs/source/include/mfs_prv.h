#ifndef __mfs_prv_h__
#define __mfs_prv_h__
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
*   This file contains the structure definitions and constants
*   that are internal to the Embedded MS-DOS File System (MFS)
*
*
*END************************************************************************/

#if MQX_USE_IO_OLD
#include <io_prv.h>
#define MFS_FD_TYPE MQX_FILE_PTR
#define MFS_FD_VALID(x) ((x) != NULL)

#else //NIO
#include <stdio.h>
#include <fcntl.h>
#include <ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <fs_supp.h>
#define MFS_FD_TYPE int
#define MFS_FD_VALID(x) ((x) >= 0)
#endif

#include <stdint.h>
#include <stdbool.h>

#include "mfs_cnfg.h"

#if MFSCFG_USE_MUTEX
#include <mutex.h>
#endif

/*
** Defines specific to MFS
*/
#define MFS_EXTRACT_UTF8      0

#define BOOT_SECTOR           0
#define FSINFO_SECTOR         1     /* FAT32 only */
#define BKBOOT_SECTOR         6     /* FAT32 only */

#define MFS_DOS30_JMP         0xEB /* 2 byte jmp 80x86 opcode */
#define MFS_DOS30_B           0xE9 /* 2 byte b 80x86 opcode */
#define MFS_DEL_FILE          0xE5 /* DOS byte used for a deleted file */

/* FAT 32 Filesystem info signatures */
#define FSI_LEADSIG           0x41615252UL
#define FSI_STRUCTSIG         0x61417272UL
#define FSI_TRAILSIG          0xAA550000UL
#define FSI_UNKNOWN           0xFFFFFFFFUL

/* This is the vesion of tfe FAT32 driver */
#define MFS_FAT32_VER         0x0000


#define MFS_ATTR_LFN_MASK     0x3F
#define MFS_ATTR_LFN          0x0F

#define MFS_LFN_END           0x40
#define MFS_LFN_ORD           0x3F

#define ATTR_EXCLUSIVE        0x40
#define MFS_ATTR_ANY          0x80


/* Defines for FAT table */
#define CLUSTER_INVALID       0x00000001UL
#define CLUSTER_UNUSED        0x00000000UL

#define CLUSTER_MIN_GOOD      0x00000002UL
#define CLUSTER_MAX_GOOD      0x0FFFFFEFUL

#define CLUSTER_MIN_RESERVED  0x0FFFFFF0UL
#define CLUSTER_MAX_RESERVED  0x0FFFFFF6UL

#define CLUSTER_BAD           0x0FFFFFF7UL

#define CLUSTER_MIN_LAST      0x0FFFFFF8UL
#define CLUSTER_EOF           0x0FFFFFFFUL

#define CLUSTER_MAXVAL_12     0x00000FEFUL
#define CLUSTER_EXTEND_12     0x0FFFF000UL

#define CLUSTER_MAXVAL_16     0x0000FFEFUL
#define CLUSTER_EXTEND_16     0x0FFF0000UL


/* Sector cache flags and modes */
#define MFS_CACHE_FLAG_READ   0x01UL
#define MFS_CACHE_FLAG_WRITE  0x02UL
#define MFS_CACHE_FLAG_DIRTY  0x04UL
#define MFS_CACHE_FLAG_FAT    0x08UL

#define MFS_MAP_MODE_READONLY  (MFS_CACHE_FLAG_READ)
#define MFS_MAP_MODE_MODIFY    (MFS_CACHE_FLAG_READ | MFS_CACHE_FLAG_WRITE)
#define MFS_MAP_MODE_OVERWRITE (MFS_CACHE_FLAG_WRITE)
#define MFS_MAP_MODE_MASK      (MFS_CACHE_FLAG_READ | MFS_CACHE_FLAG_WRITE)


/* API entry mode flags (MFS_enter function) */
#define MFS_ENTER_READONLY     0x00
#define MFS_ENTER_READWRITE    0x01
#define MFS_ENTER_RAW          0x02


#define MFS_INVALID_SECTOR 0xFFFFFFFF

/* Maximum file size is 4GB-1 */
#define MFS_MAX_FILESIZE 0xFFFFFFFFUL

/* File status flags */
#if MQX_USE_IO_OLD
#define MFS_O_ACCMODE         0x03
#define MFS_O_RDONLY          0x00
#define MFS_O_WRONLY          0x01
#define MFS_O_RDWR            0x02
#define MFS_O_CREAT           0x40
#define MFS_O_EXCL            0x80
#define MFS_O_TRUNC          0x200
#define MFS_O_APPEND         0x400
#define MFS_O_SYNC          0x1000

#else //MQX_USE_IO_OLD
#define MFS_O_ACCMODE  O_ACCMODE
#define MFS_O_RDONLY   O_RDONLY
#define MFS_O_WRONLY   O_WRONLY
#define MFS_O_RDWR     O_RDWR
#define MFS_O_CREAT    O_CREAT
#define MFS_O_EXCL     O_EXCL
#define MFS_O_TRUNC    O_TRUNC
#define MFS_O_APPEND   O_APPEND
#define MFS_O_SYNC     O_SYNC
#endif

/*
**  File system data structures as stored on the media
*/

/* Structure of the BIOS Parameter Block maintained in the boot sector as */
typedef struct bios_param_struct_disk
{
    uint8_t  JMPBOOT[3];
    uint8_t  OEMNAME[8];
    uint8_t  SECTOR_SIZE[2];
    uint8_t  SECTORS_PER_CLUSTER[1];
    uint8_t  RESERVED_SECTORS[2];
    uint8_t  NUMBER_OF_FAT[1];
    uint8_t  ROOT_ENTRIES[2];
    uint8_t  NUMBER_SECTORS[2];
    uint8_t  MEDIA_TYPE[1];
    uint8_t  SECTORS_PER_FAT[2];
    uint8_t  SECTORS_PER_TRACK[2];
    uint8_t  NUM_HEADS[2];
    uint8_t  HIDDEN_SECTORS[4];
    uint8_t  MEGA_SECTORS[4];
} BIOS_PARAM_STRUCT_DISK, * BIOS_PARAM_STRUCT_DISK_PTR;


/* Structure of the BIOS Parameter Block maintained in the boot sector (FAT32 extension) */
typedef struct bios_param32_struct_disk
{
    uint8_t  FAT_SIZE[4];
    uint8_t  EXT_FLAGS[2];
    uint8_t  FS_VER[2];
    uint8_t  ROOT_CLUSTER[4];
    uint8_t  FS_INFO[2];
    uint8_t  BK_BOOT_SEC[2];
    uint8_t  RESERVED[12];
} BIOS_PARAM32_STRUCT_DISK, * BIOS_PARAM32_STRUCT_DISK_PTR;


/* The file system info struct. Special sector, for FAT32 only */
typedef struct filesystem_info_disk
{
    uint8_t  LEAD_SIG[4];
    uint8_t  RESERVED1[480];
    uint8_t  STRUCT_SIG[4];
    uint8_t  FREE_COUNT[4];
    uint8_t  NEXT_FREE[4];
    uint8_t  RESERVED2[12];
    uint8_t  TRAIL_SIG[4];
} FILESYSTEM_INFO_DISK, * FILESYSTEM_INFO_DISK_PTR;


/* Structure of entries in the file system directory */
typedef struct dir_entry_disk
{
    uint8_t  NAME[8];
    uint8_t  TYPE[3];  // unused in the code
    uint8_t  ATTRIBUTE[1];
    uint8_t  RESERVED[8];
    uint8_t  HFIRST_CLUSTER[2];
    uint8_t  TIME[2];
    uint8_t  DATE[2];
    uint8_t  LFIRST_CLUSTER[2];
    uint8_t  FILE_SIZE[4];
} DIR_ENTRY_DISK, * DIR_ENTRY_DISK_PTR;


/* Structure of a long file name slot */
typedef struct mfs_lname_entry
{
    uint8_t  ID;
    uint8_t  NAME0_4[10];
    uint8_t  ATTR;
    uint8_t  RESERVE;
    uint8_t  ALIAS_CHECKSUM;
    uint8_t  NAME5_10[12];
    uint8_t  START[2];
    uint8_t  NAME11_12[4];
} MFS_LNAME_ENTRY, * MFS_LNAME_ENTRY_PTR;



/*
**  MFS Internal data structures
*/

/* Sector cache data structures */
typedef struct sector_cache_record
{
    struct sector_cache_record *LRU_NEXT;
    uint32_t SECTOR_NUM;
    uint32_t USAGE_COUNTER;
    uint32_t FLAGS;
    uint32_t TAG;
    void *BUFFER;
} SECTOR_CACHE_RECORD;

typedef struct sector_cache
{
    int BUFFERS;
    void *BUFFER_AREA;
    SECTOR_CACHE_RECORD *RECORDS;
    SECTOR_CACHE_RECORD *RECENT;
} SECTOR_CACHE;


/* Directory entry abstraction */
typedef struct dir_entry
{
    uint32_t ENTRY_SECTOR;  /* sector of media containing the entry */
    uint32_t ENTRY_INDEX;   /* index of directory entry withing the sector */
    uint32_t HEAD_CLUSTER;  /* head cluster of the chain associated with this directory entry */
    uint32_t FILE_SIZE;     /* size of the file in bytes */
    uint16_t WRITE_DATE;    /* write date in disk format (but host endianess) */
    uint16_t WRITE_TIME;    /* write time in disk format (but host endianess) */
    uint32_t DIRTY;         /* this flag is set to indicate necessity to update the entry on the media */
    uint32_t REFCNT;        /* reference counter, number of handles accessing this directory entry */
    uint32_t VSTAMP;        /* validity stamp, the value changes if an update needs to be propagated to all open handles */
} DIR_ENTRY, *DIR_ENTRY_PTR;


/* File handle - MFS context data for open file */
typedef struct mfs_handle
{
    QUEUE_ELEMENT_STRUCT HEADER_PTR;

    uint32_t      LOCATION;
    uint32_t      FSFLAGS;  /* file status flags */
    uint32_t      TOUCHED;  /* write performed, indicates necessity to update write time of the file, not to be confused with dir entry dirty flag */
    FAT_CHAIN     CHAIN;
    DIR_ENTRY    *DIR_ENTRY;
    uint32_t      VSTAMP;   /* validity stamp to keep handle up to date with directory entry shared with other handles */
} MFS_HANDLE, *MFS_HANDLE_PTR;


/* Drive structure - MFS context data for mounted file system */
typedef struct mfs_drive_struct
{
    uint32_t                   DRV_NUM;

    bool                       DEV_OPEN;
    bool                       DOS_DISK;
    bool                       BLOCK_MODE;
    bool                       READ_ONLY;

    uint32_t                   ALIGNMENT;
    uint32_t                   ALIGNMENT_MASK;

    uint16_t                   SECTOR_SIZE;
    uint16_t                   SECTOR_POWER;

    uint32_t                   SECTORS_PER_CLUSTER;

    uint16_t                   CLUSTER_POWER_SECTORS;
    uint16_t                   CLUSTER_POWER_BYTES;
    uint32_t                   CLUSTER_SIZE_BYTES;

    uint32_t                   SECTORS_PER_FAT;
    uint32_t                   NUMBER_OF_FAT;

    uint16_t                   ENTRIES_PER_SECTOR;

    uint16_t                   ROOT_ENTRIES;
    uint32_t                   ROOT_START_SECTOR;
    uint32_t                   ROOT_CLUSTER;
    FAT_CHAIN                  ROOT_CHAIN;

    uint32_t                   FAT_START_SECTOR;
    uint32_t                   DATA_START_SECTOR;

    uint32_t                   FS_INFO;
    uint32_t                   MEGA_SECTORS;

    QUEUE_STRUCT               HANDLE_LIST;

    uint32_t                   FAT_TYPE;
    uint32_t                   LAST_CLUSTER;

    uint32_t                   CUR_DIR_CLUSTER;
    FAT_CHAIN                  CUR_DIR_CHAIN_PREALLOC;
    FAT_CHAIN                 *CUR_DIR_CHAIN_PTR;
    char                       CURRENT_DIR[PATHNAME_SIZE + 1];

#if MFSCFG_USE_MUTEX
    MUTEX_STRUCT               MUTEX;
#else
    LWSEM_STRUCT               LWSEM;
#endif

    MFS_FD_TYPE                DEV_FILE_PTR;

    bool                       DIR_SECTOR_DIRTY;
    uint32_t                   DIR_SECTOR_NUMBER;
    char                       *DIR_SECTOR_PTR;

    /* Count of free clusters. 0xFFFFFFFF means unknown. Must be recalculated */
    uint32_t                   FREE_COUNT;
    /* Is set to the last allocated cluster. If = 0xFFFFFFF start search at 2 */
    uint32_t                   NEXT_FREE_CLUSTER;

    SECTOR_CACHE               SECTOR_CACHE;
    _mfs_cache_policy          WRITE_CACHE_POLICY;

} MFS_DRIVE_STRUCT, *MFS_DRIVE_STRUCT_PTR;



/*
** MFS Macro code
*/

#ifndef min
    #define min(a,b) (((a)<(b))?(a):(b))
#endif

#define MFS_set_error_and_return(error_ptr, error, retval) { if (error_ptr != NULL) { *error_ptr = error;} return(retval ); }
#define MFS_LOG(x)  //(x);

/*
** The offset within the sector is obtained by masking off the MSB's
*/
#define OFFSET_WITHIN_SECTOR(drive_ptr, x)  ((x) & ((drive_ptr)->SECTOR_SIZE - 1))

/*
**  Returns the # of the first sector in the cluster.
*/
#define CLUSTER_TO_SECTOR(drive_ptr, x) (((x - CLUSTER_MIN_GOOD) << (drive_ptr)->CLUSTER_POWER_SECTORS) + (drive_ptr)->DATA_START_SECTOR)

/*
** INDEX_TO_SECTOR converts an entry_index within the cluster or directory
** to the sector # in which that entry is found.
*/
#define INDEX_TO_SECTOR(drive_ptr, x)  ((x) / (drive_ptr)->ENTRIES_PER_SECTOR)

/*
** INDEX_WITHIN_SECTOR converts a directory index to the index within the sector
*/
#define INDEX_WITHIN_SECTOR(drive_ptr, x) ((x) & ((drive_ptr)->ENTRIES_PER_SECTOR-1))


/* Used to fix bug in date calculation */
#define NORMALIZE_DATE(c_ptr) \
   if ((c_ptr)->YEAR < 1980) (c_ptr)->YEAR = 1980


/* Special Macros for reading and writing the FAT */

#define htof0(p,x)    ((p)[0] = (uint8_t)((x) & 0xFF), \
                       (p)[1] = (uint8_t)(((p)[1] & 0xF0) | (((x) >> 8) & 0x0F)) \
                      )

#define htof1(p,x)    ((p)[0] = (uint8_t)(((p)[0] & 0x0F) | (((x) << 4) & 0xF0)), \
                       (p)[1] = (uint8_t)(((x) >> 4) & 0xFF) \
                      )

#define ftoh0(p)      (((uint32_t)((p)[0]       )       & 0x000000FFL) | \
                       ((uint32_t)((p)[1] & 0x0F) <<  8 & 0x0000FF00L))

#define ftoh1(p)      (((uint32_t)((p)[0] & 0xF0) >>  4 & 0x000000FFL) | \
                       ((uint32_t)((p)[1]       ) <<  4 & 0x00000FF0L))

#define htof32(p,x)   ((p)[3] = ((p)[3] & 0xF0) | (((x) >> 24) & 0x0F), \
                       (p)[2] = ((x) >> 16) & 0xFF, \
                       (p)[1] = ((x) >>  8) & 0xFF, \
                       (p)[0] = ((x)      ) & 0xFF)


/*
** Macros for getting/setting cluster, using a separate high and low word
*/
#define clustoh(drive_ptr, high, low) \
    ( \
    (((uint32_t)(low)[0])       )  | \
    (((uint32_t)(low)[1])  <<  8)  | \
    ((drive_ptr)->FAT_TYPE != MFS_FAT32 ? 0 : \
    (((uint32_t)(high)[0]) << 16)  | \
    (((uint32_t)(high)[1]) << 24)  & 0x0FFFFFFF) \
    )

#define clustod(high, low, x) \
    ( \
    (low)[0]  = (uint8_t) ((x)      ) , \
    (low)[1]  = (uint8_t) ((x) >>  8) , \
    (high)[0] = (uint8_t) ((x) >> 16) , \
    (high)[1] = (uint8_t) ((x) >> 24) \
    )


#define PACK_TIME(x)  ((x.HOUR << MFS_SHIFT_HOURS) |\
                       (x.MINUTE << MFS_SHIFT_MINUTES) |\
                       (x.SECOND >>1))

#define PACK_DATE(x) (((x.YEAR -1980)<< MFS_SHIFT_YEAR) |\
                       (x.MONTH << MFS_SHIFT_MONTH) |\
                       (x.DAY << MFS_SHIFT_DAY))

#define CAPITALIZE(c)  ( ((c)>='a'&&(c)<='z') ? (c)&~0x20 : (c) )


#define IS_DELIMITER(c) (((c) == '\\') || ((c) == '/'))

/*
** extern statements for MFS
*/
#ifdef __cplusplus
extern "C" {
#endif

#ifndef _mqx_str_h_
    extern uint32_t _strnlen(char *, uint32_t);
#endif

_mfs_error        _mfs_validate_device(MFS_FD_TYPE, uint32_t *, bool *);
_mfs_error        MFS_Bad_clusters(MFS_DRIVE_STRUCT_PTR, uint32_t *bad_clusters);
uint32_t          MFS_Change_current_dir(MFS_DRIVE_STRUCT_PTR, char *);
uint32_t          MFS_Check_dir_exist(MFS_DRIVE_STRUCT_PTR, char *);
uint32_t          MFS_Clear_cluster(MFS_DRIVE_STRUCT_PTR, uint32_t);
int32_t           MFS_Close_Device(MFS_DRIVE_STRUCT_PTR);
_mfs_error        MFS_Close_file(MFS_DRIVE_STRUCT_PTR, MFS_HANDLE_PTR);
DIR_ENTRY_DISK_PTR MFS_Create_directory_entry(MFS_DRIVE_STRUCT_PTR, char *, char, uint32_t *, uint32_t *, uint32_t *);
DIR_ENTRY_DISK_PTR MFS_Create_entry_slave(MFS_DRIVE_STRUCT_PTR, unsigned char, char *, uint32_t *, uint32_t *, _mfs_error_ptr);
void              *MFS_Create_file(MFS_DRIVE_STRUCT_PTR, unsigned char, char *, uint32_t *);
_mfs_error        MFS_Create_subdir(MFS_DRIVE_STRUCT_PTR, char *);
void              MFS_Decrement_free_clusters(MFS_DRIVE_STRUCT_PTR);
_mfs_error        MFS_Default_Format(MFS_DRIVE_STRUCT_PTR);
uint32_t          MFS_Delete_file(MFS_DRIVE_STRUCT_PTR, MFS_HANDLE_PTR, char *);
bool              MFS_Dirname_valid(char *);
void              MFS_Expand_dotfile(char *sfn, uint8_t *sfn_disk);
_mfs_error        MFS_Extend_chain(MFS_DRIVE_STRUCT_PTR, uint32_t, uint32_t, uint32_t *);
bool              MFS_Filename_valid(char *);
uint32_t          MFS_Find_directory(MFS_DRIVE_STRUCT_PTR, char *, uint32_t);
DIR_ENTRY_DISK_PTR MFS_Find_directory_entry(MFS_DRIVE_STRUCT_PTR, char *, uint32_t *, uint32_t *, uint32_t *, unsigned char, uint32_t *);
uint32_t          MFS_Find_unused_cluster_from(MFS_DRIVE_STRUCT_PTR, uint32_t);
int32_t           MFS_Flush_Device(MFS_DRIVE_STRUCT_PTR, MFS_HANDLE_PTR);
uint32_t          MFS_Format(MFS_DRIVE_STRUCT_PTR, MFS_FORMAT_DATA_PTR);
_mfs_error        MFS_get_cluster_from_fat(MFS_DRIVE_STRUCT_PTR, uint32_t, uint32_t *);
uint32_t          MFS_Get_current_dir(MFS_DRIVE_STRUCT_PTR, char *);
uint32_t          MFS_Get_date_time(MFS_DRIVE_STRUCT_PTR, MFS_HANDLE_PTR, uint16_t *, uint16_t *);
_mfs_error        MFS_Get_disk_free_space(MFS_DRIVE_STRUCT_PTR drive_ptr, uint32_t *clusters_free_ptr, uint64_t *bytes_free_ptr);
_mfs_error        MFS_Get_disk_free_space_internal(MFS_DRIVE_STRUCT_PTR, uint32_t *free_clusters);
uint32_t          MFS_Get_file_attributes(MFS_DRIVE_STRUCT_PTR, MFS_HANDLE_PTR, char *, unsigned char *);
uint32_t          MFS_get_prev_cluster(MFS_DRIVE_STRUCT_PTR, uint32_t *, uint32_t);
uint32_t          MFS_Get_volume(MFS_DRIVE_STRUCT_PTR, char *);
uint32_t          MFS_Increment_dir_index(MFS_DRIVE_STRUCT_PTR, uint32_t *, uint32_t *, uint32_t *);
void              MFS_Increment_free_clusters(MFS_DRIVE_STRUCT_PTR);
_mfs_error        MFS_increment_lfn(char *);
uint16_t          MFS_Is_dot_directory(char *);
bool              MFS_is_read_only(MFS_DRIVE_STRUCT_PTR);
bool              MFS_is_valid_lfn(char *);
_mfs_error        MFS_Last_cluster(MFS_DRIVE_STRUCT_PTR, uint32_t *last_cluster);
unsigned char     MFS_lfn_checksum(uint8_t *sfn_disk);
bool              MFS_lfn_dirname_valid(char *);
int               MFS_lfn_extract(MFS_LNAME_ENTRY_PTR, char *);
uint32_t          MFS_lfn_name_to_entry(char *, MFS_LNAME_ENTRY_PTR);
int32_t           MFS_Open_Device(MFS_DRIVE_STRUCT_PTR);
MFS_HANDLE_PTR    MFS_Open_file(MFS_DRIVE_STRUCT_PTR, char *, uint32_t, _mfs_error *);
char              *MFS_Parse_next_filename(char *, char *);
char              *MFS_Parse_Out_Device_Name(char *);
uint32_t          MFS_Parse_pathname(char *, char *, char *);
uint32_t          MFS_Put_fat(MFS_DRIVE_STRUCT_PTR, uint32_t, uint32_t);
_mfs_error        MFS_Read_device_sectors(MFS_DRIVE_STRUCT_PTR, uint32_t, uint32_t, uint32_t, char *, uint32_t *);
void              *MFS_Read_directory_sector(MFS_DRIVE_STRUCT_PTR, uint32_t, uint16_t, uint32_t *);
_mfs_error        MFS_Release_chain(MFS_DRIVE_STRUCT_PTR, uint32_t);
uint32_t          MFS_remove_lfn_entries(MFS_DRIVE_STRUCT_PTR, uint32_t, uint32_t, uint32_t);
uint32_t          MFS_Remove_subdir(MFS_DRIVE_STRUCT_PTR, char *);
uint32_t          MFS_Rename_file(MFS_DRIVE_STRUCT_PTR, char *, char *);
uint32_t          MFS_Set_date_time(MFS_DRIVE_STRUCT_PTR, MFS_HANDLE_PTR, uint16_t *, uint16_t *);
uint32_t          MFS_Set_file_attributes(MFS_DRIVE_STRUCT_PTR, MFS_HANDLE_PTR, char *, unsigned char);
uint32_t          MFS_Set_volume(MFS_DRIVE_STRUCT_PTR, char *);
uint32_t          MFS_Test_unused_clusters(MFS_DRIVE_STRUCT_PTR, uint32_t *);
extern _mfs_error MFS_Write_device_sectors(MFS_DRIVE_STRUCT_PTR, uint32_t, uint32_t, uint32_t, char *, uint32_t *);
extern _mfs_error MFS_Write_device_sector(MFS_DRIVE_STRUCT_PTR, uint32_t, char *);

uint32_t  MFS_Read(MFS_DRIVE_STRUCT_PTR, MFS_HANDLE_PTR, uint32_t, char *, uint32_t *);
uint32_t  MFS_Write(MFS_DRIVE_STRUCT_PTR, MFS_HANDLE_PTR, uint32_t, char *, uint32_t *);

void      *MFS_mem_alloc(_mem_size size);
void      *MFS_mem_alloc_zero(_mem_size size);
void      *MFS_mem_alloc_system(_mem_size size);
void      *MFS_mem_alloc_system_zero(_mem_size size);
void      *MFS_mem_alloc_system_align(_mem_size size, _mem_size align);
_mfs_error MFS_mem_free(void *mem_ptr);

int MFS_sector_cache_alloc(MFS_DRIVE_STRUCT_PTR drive_ptr, int buffers);
int MFS_sector_cache_free(MFS_DRIVE_STRUCT_PTR drive_ptr);

int MFS_sector_cache_flush(MFS_DRIVE_STRUCT_PTR drive_ptr, uint32_t first_sector, uint32_t sectors);
int MFS_sector_cache_flush_tag(MFS_DRIVE_STRUCT_PTR drive_ptr, uint32_t tag);
int MFS_sector_cache_invalidate(MFS_DRIVE_STRUCT_PTR drive_ptr, uint32_t first_sector, uint32_t sectors);
int MFS_sector_cache_invalidate_tag(MFS_DRIVE_STRUCT_PTR drive_ptr, uint32_t tag);

int MFS_sector_map(MFS_DRIVE_STRUCT_PTR drive_ptr, uint32_t sector_num, void **buf_ptr, int flags, uint32_t tag);
int MFS_sector_unmap(MFS_DRIVE_STRUCT_PTR drive_ptr, uint32_t sector_num, bool updated);

int MFS_unmap_directory_sector(MFS_DRIVE_STRUCT_PTR  drive_ptr);

_mfs_error MFS_chain_init(MFS_DRIVE_STRUCT_PTR drive_ptr, FAT_CHAIN *chain, uint32_t head_cluster);
_mfs_error MFS_chain_forge(MFS_DRIVE_STRUCT_PTR drive_ptr, FAT_CHAIN *chain, uint32_t sector_num, uint32_t sector_count);
_mfs_error MFS_chain_create(MFS_DRIVE_STRUCT_PTR drive_ptr, FAT_CHAIN *chain, uint32_t *head_cluster_ptr);
_mfs_error MFS_chain_locate(MFS_DRIVE_STRUCT_PTR drive_ptr, FAT_CHAIN *chain, uint32_t pos, uint32_t extend_to, uint32_t *sector, uint32_t *sector_count);

void MFS_dir_entry_from_disk(MFS_DRIVE_STRUCT_PTR drive_ptr, DIR_ENTRY *dir_entry, DIR_ENTRY_DISK *dir_entry_disk);
void MFS_dir_entry_to_disk(MFS_DRIVE_STRUCT_PTR drive_ptr, DIR_ENTRY *dir_entry, DIR_ENTRY_DISK *dir_entry_disk);

_mfs_error MFS_dir_entry_fetch(MFS_DRIVE_STRUCT_PTR drive_ptr, struct dir_entry *dir_entry, uint32_t sector_num, uint32_t sector_offset);
_mfs_error MFS_dir_entry_store(MFS_DRIVE_STRUCT_PTR drive_ptr, struct dir_entry *dir_entry);
_mfs_error MFS_dir_entry_sync(MFS_DRIVE_STRUCT_PTR drive_ptr, struct dir_entry *dir_entry);

_mfs_error MFS_update_entry(MFS_DRIVE_STRUCT_PTR, MFS_HANDLE_PTR);
_mfs_error MFS_update_entries(MFS_DRIVE_STRUCT_PTR);

void MFS_sfn_from_disk(uint8_t *disk_sfn, char *utf8_sfn);

MFS_HANDLE_PTR MFS_Create_handle(MFS_DRIVE_STRUCT_PTR drive_ptr, MFS_HANDLE_PTR assoc_with);
void           MFS_Destroy_handle(MFS_DRIVE_STRUCT_PTR drive_ptr, MFS_HANDLE_PTR handle);
MFS_HANDLE_PTR MFS_Find_handle(MFS_DRIVE_STRUCT_PTR drive_ptr, uint32_t dir_cluster, uint32_t dir_index);
MFS_HANDLE_PTR MFS_Find_handle_new(MFS_DRIVE_STRUCT_PTR drive_ptr, uint32_t entry_sector, uint32_t entry_index);

int MFS_ioctl(MFS_DRIVE_STRUCT_PTR drive_ptr, MFS_HANDLE_PTR handle, unsigned long int cmd, ...);
int MFS_vioctl(MFS_DRIVE_STRUCT_PTR drive_ptr, MFS_HANDLE_PTR handle, unsigned long int cmd, va_list ap);

uint32_t MFS_Mount_drive_internal(MFS_DRIVE_STRUCT_PTR);
int      MFS_Unmount_drive_internal(MFS_DRIVE_STRUCT_PTR drive_ptr);

_mfs_error MFS_lock(MFS_DRIVE_STRUCT_PTR drive_ptr);
_mfs_error MFS_unlock(MFS_DRIVE_STRUCT_PTR drive_ptr);

_mfs_error MFS_enter(MFS_DRIVE_STRUCT_PTR drive_ptr, int mode);
_mfs_error MFS_leave(MFS_DRIVE_STRUCT_PTR drive_ptr, int flags);

_mfs_error MFS_lock_and_enter(MFS_DRIVE_STRUCT_PTR drive_ptr, int mode);
_mfs_error MFS_leave_and_unlock(MFS_DRIVE_STRUCT_PTR drive_ptr, int flags);

_mfs_error MFS_Create_drive(MFS_DRIVE_STRUCT_PTR *drive_ptr_ptr);
_mfs_error MFS_Destroy_drive(MFS_DRIVE_STRUCT_PTR drive_ptr);

_mfs_error MFS_get_dir_chain(MFS_DRIVE_STRUCT_PTR drive_ptr, char *path, FAT_CHAIN *dir_chain, FAT_CHAIN **dir_chain_ptr, char **entry_name);
_mfs_error MFS_scan_dir_chain(MFS_DRIVE_STRUCT_PTR drive_ptr, FAT_CHAIN *dir_chain, char *name, DIR_ENTRY_DISK *entry_copy, uint32_t *entry_sector, uint32_t *entry_index, uint32_t *entry_loc);
_mfs_error MFS_read_dir_chain(MFS_DRIVE_STRUCT_PTR drive_ptr, FAT_CHAIN *dir_chain, uint32_t *dir_loc_ptr, char *lfn_buf, int lfn_buf_len, DIR_ENTRY_DISK *entry_copy, uint32_t *entry_sector, uint32_t *entry_index, uint32_t *entry_loc);
_mfs_error MFS_locate_dir_entry(MFS_DRIVE_STRUCT_PTR drive_ptr, char *pathname, DIR_ENTRY_DISK *entry_copy, uint32_t *entry_sector, uint32_t *entry_index);
_mfs_error MFS_free_dir_entry(MFS_DRIVE_STRUCT_PTR  drive_ptr, FAT_CHAIN *dir_chain, uint32_t location);
_mfs_error MFS_check_dir_empty(MFS_DRIVE_STRUCT_PTR  drive_ptr, FAT_CHAIN *dir_chain);

int MFS_next_path_component(char **parse_pos, char **path_component);
int MFS_path_component_len(char *pathcomp, int *unicode_chars, int *utf16_words);

int MFS_lfn_utf16_len(MFS_LNAME_ENTRY *lfn_entry);
_mfs_error MFS_lfn_to_sfn(char *lfn, char *sfn);
_mfs_error MFS_lfn_to_sfn_disk(char *lfn, int lfn_len, uint8_t sfn_disk[11], bool *fits_sfn, bool *needs_lfn);
int MFS_lfn_alias_index(uint8_t sfn_template[11], uint8_t sfn[11]);
bool MFS_lfn_entry_match_r(char *utf8_str, char **utf8_pos, MFS_LNAME_ENTRY *lfn_entry, uint32_t *surrogate_buf);
int MFS_lfn_entry_extract_r(MFS_LNAME_ENTRY *lfn_entry, char **utf8_pos, char *utf8_end, uint32_t *surrogate_buf);
_mfs_error MFS_lfn_chain_store(MFS_DRIVE_STRUCT_PTR drive_ptr, FAT_CHAIN *dir_chain, uint32_t location, char *lfn, int lfn_chksum);
_mfs_error MFS_get_lfn(MFS_DRIVE_STRUCT_PTR drive_ptr, char *filepath, char *lfn);

_mfs_error MFS_find_first_file(MFS_DRIVE_STRUCT_PTR drive_ptr, MFS_SEARCH_PARAM_PTR sp_ptr);
_mfs_error MFS_find_next_file(MFS_DRIVE_STRUCT_PTR drive_ptr, MFS_SEARCH_DATA_PTR sd_ptr);

int32_t utf8_decode(char **decode_pos, char *decode_boundary);
int32_t utf8_decode_r(char *decode_str, char **decode_pos);
int utf8_encode(uint32_t codepoint, char **encode_pos, char *encode_boundary);
int utf8_encode_r(uint32_t codepoint, char **encode_pos, char *encode_boundary);

void mem_reverse(char *first, char *last);


int ilog2(uint32_t x);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
