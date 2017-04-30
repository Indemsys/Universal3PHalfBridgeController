#ifndef __tfs_h__
#define __tfs_h__
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
*   This file contains the structure definitions and constants for a
*   user who will be compiling programs that will use the Embedded MS-DOS
*   File System (TFS)
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>
#include <ioctl.h>

/*
** Defines specific to TFS
*/

#define TFS_VERSION 1

#define TFS_FLAG_INDEX  1
#define TFS_FLAG_AUTH   2


/*
** error codes
*/
#define TFS_NO_ERROR                          FS_NO_ERROR
#define TFS_INVALID_FUNCTION_CODE             FS_INVALID_FUNCTION_CODE
#define TFS_FILE_NOT_FOUND                    FS_FILE_NOT_FOUND
#define TFS_PATH_NOT_FOUND                    FS_PATH_NOT_FOUND
#define TFS_ACCESS_DENIED                     FS_ACCESS_DENIED
#define TFS_INVALID_HANDLE                    FS_INVALID_HANDLE
#define TFS_INSUFFICIENT_MEMORY               FS_INSUFFICIENT_MEMORY
#define TFS_INVALID_MEMORY_BLOCK_ADDRESS      FS_INVALID_MEMORY_BLOCK_ADDRESS
#define TFS_ATTEMPT_TO_REMOVE_CURRENT_DIR     FS_ATTEMPT_TO_REMOVE_CURRENT_DIR
#define TFS_DISK_IS_WRITE_PROTECTED           FS_DISK_IS_WRITE_PROTECTED
#define TFS_BAD_DISK_UNIT                     FS_BAD_DISK_UNIT
#define TFS_INVALID_LENGTH_IN_DISK_OPERATION  FS_INVALID_LENGTH_IN_DISK_OPERATION
#define TFS_NOT_A_DOS_DISK                    FS_NOT_A_DOS_DISK
#define TFS_SECTOR_NOT_FOUND                  FS_SECTOR_NOT_FOUND 
#define TFS_WRITE_FAULT                       FS_WRITE_FAULT
#define TFS_READ_FAULT                        FS_READ_FAULT
#define TFS_SHARING_VIOLATION                 FS_SHARING_VIOLATION
#define TFS_FILE_EXISTS                       FS_FILE_EXISTS
#define TFS_ALREADY_ASSIGNED                  FS_ALREADY_ASSIGNED
#define TFS_INVALID_PARAMETER                 FS_INVALID_PARAMETER
#define TFS_DISK_FULL                         FS_DISK_FULL 
#define TFS_ROOT_DIR_FULL                     FS_ROOT_DIR_FULL 
#define TFS_EOF                               FS_EOF
#define TFS_CANNOT_CREATE_DIRECTORY           FS_CANNOT_CREATE_DIRECTORY
#define TFS_NOT_INITIALIZED                   FS_NOT_INITIALIZED
#define TFS_OPERATION_NOT_ALLOWED             FS_OPERATION_NOT_ALLOWED
#define TFS_ERROR_INVALID_DRIVE_HANDLE        FS_ERROR_INVALID_DRIVE_HANDLE
#define TFS_ERROR_INVALID_FILE_HANDLE         FS_ERROR_INVALID_FILE_HANDLE
#define TFS_ERROR_UNKNOWN_FS_VERSION          FS_ERROR_UNKNOWN_FS_VERSION
#define TFS_LOST_CHAIN                        FS_LOST_CHAIN
#define TFS_INVALID_DEVICE                    FS_INVALID_DEVICE
#define TFS_INVALID_CLUSTER_NUMBER            FS_INVALID_CLUSTER_NUMBER
#define TFS_FAILED_TO_DELETE_LFN              FS_FAILED_TO_DELETE_LFN
#define TFS_BAD_LFN_ENTRY                     FS_BAD_LFN_ENTRY

/*
** Extra IO_IOCTL codes
*/
#define IO_IOCTL_TFS_GET_NAME                     _IO(IO_TYPE_TFS,0x01)
#define IO_IOCTL_TFS_GET_ATTRIBUTES               _IO(IO_TYPE_TFS,0x02)
#define IO_IOCTL_TFS_GET_LENGTH                   _IO(IO_TYPE_TFS,0x03)
#define IO_IOCTL_TFS_GET_LAST_ERROR               _IO(IO_TYPE_TFS,0x04)
#define IO_IOCTL_TFS_GET_CURRENT_DATA_PTR         _IO(IO_TYPE_TFS,0x05)


/*
** TFS directory entry information
*/
typedef struct tfs_dir_entry
{
    char    *NAME;
    uint32_t     FLAGS;
    unsigned char   *DATA;
    uint32_t     SIZE;
} TFS_DIR_ENTRY, * TFS_DIR_ENTRY_PTR;


/*
** extern statements for TFS
*/

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t _io_tfs_install (char *, const TFS_DIR_ENTRY* );
extern int32_t _io_tfs_uninstall (IO_DEVICE_STRUCT_PTR);
extern int32_t _io_tfs_open (MQX_FILE_PTR, char *, char *);
extern int32_t _io_tfs_close (MQX_FILE_PTR);
extern int32_t _io_tfs_read (MQX_FILE_PTR, char *, int32_t);
extern int32_t _io_tfs_write (MQX_FILE_PTR, char *, int32_t);
extern _mqx_int _io_tfs_ioctl (MQX_FILE_PTR, _mqx_uint, void *);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */

