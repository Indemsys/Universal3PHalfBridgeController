/*HEADER**********************************************************************
*
* Copyright 2008-2015 Freescale Semiconductor, Inc.
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
*   This file contains functions for opening and closing file or device
*
*
*END************************************************************************/

#include "mfs.h"
#include "mfs_prv.h"

/*!
 * \brief Open a an existing file.
 *
 * \param drive_ptr
 * \param[in] pathname Directory and filename of the file to be opened.
 * \param[in] fsflags Type of access required: read, write or read/write.
 * \param[in,out] error_ptr Error code is written to this address.
 *
 * \return MFS_HANDLE_PTR A pointer to an MFS_HANDLE.
 */
MFS_HANDLE_PTR MFS_Open_file(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    char *pathname,
    uint32_t fsflags,
    _mfs_error *error_ptr)
{
    MFS_HANDLE_PTR handle = NULL;
    bool write_access;
    _mfs_error error_code;

    if ((pathname == NULL) || (*pathname == '\0'))
    {
        error_code = MFS_INVALID_PARAMETER;
        if (error_ptr != NULL)
        {
            *error_ptr = error_code;
        }
        return NULL;
    }

    write_access = ((fsflags & MFS_O_ACCMODE) != MFS_O_RDONLY) || ((fsflags & MFS_O_CREAT) != 0);

    error_code = MFS_lock_and_enter(drive_ptr, write_access ? MFS_ENTER_READWRITE : MFS_ENTER_READONLY);
    if (error_code != MFS_NO_ERROR)
    {
        if (error_ptr != NULL)
        {
            *error_ptr = error_code;
        }
        return NULL;
    }

    /* File creation in exclusive mode is requested, force file creation by indicating "file not found" and skip searching for existing file */
    if ((fsflags & MFS_O_CREAT) && (fsflags & MFS_O_EXCL))
    {
        error_code = MFS_FILE_NOT_FOUND;
    }
    else
    {
        /* Try to open existing file */
        FAT_CHAIN dir_chain;
        DIR_ENTRY_DISK dir_entry;
        char *entry_name;
        uint32_t entry_sector;
        uint32_t entry_index;

        /* Find the directory in which the file shall be located */
        error_code = MFS_get_dir_chain(drive_ptr, pathname, &dir_chain, NULL, &entry_name);
        if (error_code == MFS_NO_ERROR)
        {
            /* Lookup entry  with the requested name in the directory */
            error_code = MFS_scan_dir_chain(drive_ptr, &dir_chain, entry_name, &dir_entry, &entry_sector, &entry_index, NULL);
            if (error_code == MFS_NO_ERROR)
            {
                /* Check if it is a regular file and verify permissions */
                if (dir_entry.ATTRIBUTE[0] & (MFS_ATTR_DIR_NAME | MFS_ATTR_VOLUME_NAME))
                {
                    error_code = MFS_ACCESS_DENIED;
                }
                else if ((dir_entry.ATTRIBUTE[0] & MFS_ATTR_READ_ONLY) && ((fsflags & MFS_O_ACCMODE) != MFS_O_RDONLY))
                {
                    error_code = MFS_ACCESS_DENIED;
                }
                else
                {
                    MFS_HANDLE_PTR existing_handle;

                    /* Check to see if the file is already opened */
                    existing_handle = MFS_Find_handle_new(drive_ptr, entry_sector, entry_index);

                    /* Create new handle possibly associating it with the existing one */
                    handle = MFS_Create_handle(drive_ptr, existing_handle);
                    if (handle == NULL)
                    {
                        error_code = MFS_INSUFFICIENT_MEMORY;
                    }

                    /* Fill in data in the directory entry, unless it was associated with an existing handle (i.e. already filled in) */
                    if (existing_handle == NULL)
                    {
                        MFS_dir_entry_from_disk(drive_ptr, handle->DIR_ENTRY, &dir_entry);
                        handle->DIR_ENTRY->ENTRY_SECTOR = entry_sector;
                        handle->DIR_ENTRY->ENTRY_INDEX = entry_index;
                        handle->DIR_ENTRY->DIRTY = 0;
                    }
                }
            }
        }
    }

#if !MFSCFG_READ_ONLY
    /* If the file was not found and there is request to create it */
    if ((error_code == MFS_FILE_NOT_FOUND) && (fsflags & MFS_O_CREAT))
    {
        /* Create new handle with blank directory entry structure first */
        handle = MFS_Create_handle(drive_ptr, NULL);
        if (handle == NULL)
        {
            error_code = MFS_INSUFFICIENT_MEMORY;
        }
        else
        {
            DIR_ENTRY_DISK_PTR dir_entry_ptr;
            uint32_t dir_cluster;
            uint32_t dir_index;

            /* Create on-disk directory entry, should the handle creation above fail the on-disk directory entry would not be created */
            dir_entry_ptr = MFS_Create_entry_slave(drive_ptr, MFS_ATTR_ARCHIVE, pathname, &dir_cluster, &dir_index, &error_code);
            if (error_code == MFS_NO_ERROR)
            {
                MFS_dir_entry_from_disk(drive_ptr, handle->DIR_ENTRY, dir_entry_ptr);
                handle->DIR_ENTRY->ENTRY_SECTOR = dir_cluster ? CLUSTER_TO_SECTOR(drive_ptr, dir_cluster) : drive_ptr->ROOT_START_SECTOR;
                handle->DIR_ENTRY->ENTRY_SECTOR += INDEX_TO_SECTOR(drive_ptr, dir_index);
                handle->DIR_ENTRY->ENTRY_INDEX = INDEX_WITHIN_SECTOR(drive_ptr, dir_index);
                handle->DIR_ENTRY->DIRTY = 0;
            }
        }
    }
#endif

    /* If there is no error at this point then either an existing file was found or a new one was successfully created */
    if (error_code == MFS_NO_ERROR)
    {
        handle->FSFLAGS = fsflags;

#if !MFSCFG_READ_ONLY
        /* Truncate the file if required */
        if (((fsflags & MFS_O_ACCMODE) != MFS_O_RDONLY) && (fsflags & MFS_O_TRUNC))
        {
            error_code = MFS_Release_chain(drive_ptr, handle->DIR_ENTRY->HEAD_CLUSTER);
            if (error_code == MFS_NO_ERROR)
            {
                handle->DIR_ENTRY->HEAD_CLUSTER = 0;
                handle->DIR_ENTRY->FILE_SIZE = 0;
                handle->DIR_ENTRY->VSTAMP++; /* Truncation needs to be propagated to all open handles */
                handle->TOUCHED = 1;
            }
        }
#endif

        if (error_code == MFS_NO_ERROR)
        {
            error_code = MFS_chain_init(drive_ptr, &handle->CHAIN, handle->DIR_ENTRY->HEAD_CLUSTER);
            handle->VSTAMP = handle->DIR_ENTRY->VSTAMP;
        }
    }

    if ((error_code != MFS_NO_ERROR) && (handle != NULL))
    {
        MFS_Destroy_handle(drive_ptr, handle);
        handle = NULL;
    }

    if (error_ptr)
    {
        *error_ptr = error_code;
    }

    MFS_leave_and_unlock(drive_ptr, 0);
    return handle;
}


/*!
 * \brief Close an existing file.
 *
 * \param drive_ptr
 * \param handle
 *
 * \return _mfs_error Error code.
 */
_mfs_error MFS_Close_file(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    MFS_HANDLE_PTR handle)
{
    _mfs_error error_code;

    error_code = MFS_lock_and_enter(drive_ptr, 0);
    if (error_code != MFS_NO_ERROR)
    {
        return error_code;
    }

#if !MFSCFG_READ_ONLY
    error_code = MFS_update_entry(drive_ptr, handle);

    if (handle->DIR_ENTRY->HEAD_CLUSTER)
    {
        /* flush data sectors of the particular file */
        _mfs_error error_temp;
        error_temp = MFS_sector_cache_flush_tag(drive_ptr, handle->DIR_ENTRY->HEAD_CLUSTER);
        error_code = (error_code == MFS_NO_ERROR) ? error_temp : error_code;
    }
#endif

    MFS_Destroy_handle(drive_ptr, handle);

    MFS_leave_and_unlock(drive_ptr, 0);
    return error_code;
}


/*!
 * \brief Opens and initializes MFS driver.
 *
 * \param drive_ptr
 *
 * \return int32_t MQX_OK or an error.
 */
int32_t MFS_Open_Device(
    MFS_DRIVE_STRUCT_PTR drive_ptr)
{
    MFS_FD_TYPE dev_fd;
    uint32_t sector_size;
    int32_t error_code;

    uint32_t temp[3];

    MFS_lock(drive_ptr);

    dev_fd = drive_ptr->DEV_FILE_PTR;

    /* Select partition, if desired */
    if (drive_ptr->DRV_NUM)
    {
        error_code = ioctl(dev_fd, IO_IOCTL_SEL_PART, &drive_ptr->DRV_NUM);
        if (error_code != MFS_NO_ERROR)
        {
            MFS_unlock(drive_ptr);
            return error_code;
        }
    }

    drive_ptr->ALIGNMENT = 1; /* Default alignment is one byte (which means no special requirement) */
    ioctl(dev_fd, IO_IOCTL_GET_REQ_ALIGNMENT, &drive_ptr->ALIGNMENT);
    /* Check if alignment is power of 2 */
    if (drive_ptr->ALIGNMENT & (drive_ptr->ALIGNMENT - 1))
    {
        MFS_unlock(drive_ptr);
        return FS_INVALID_DEVICE;
    }
    /* Calculate alignment mask */
    drive_ptr->ALIGNMENT_MASK = drive_ptr->ALIGNMENT - 1;

    /* Check if the device is suitable for MFS */
    error_code = _mfs_validate_device(dev_fd, &sector_size, &drive_ptr->BLOCK_MODE);
    if (error_code)
    {
        /* Device isn't valid */
        MFS_unlock(drive_ptr);
        return error_code;
    }
    drive_ptr->SECTOR_SIZE = sector_size;
    drive_ptr->SECTOR_POWER = ilog2(drive_ptr->SECTOR_SIZE);

    /* Reset dir sector metadata */
    drive_ptr->DIR_SECTOR_NUMBER = MFS_INVALID_SECTOR;
    drive_ptr->DIR_SECTOR_PTR = NULL;
    drive_ptr->DIR_SECTOR_DIRTY = false;

    /* Allocate sector cache */
    error_code = MFS_sector_cache_alloc(drive_ptr, MFSCFG_SECTOR_CACHE_SIZE);
    if (error_code != MFS_NO_ERROR)
    {
        MFS_unlock(drive_ptr);
        return error_code;
    }

    /* Caching policy */
    if ((ioctl(drive_ptr->DEV_FILE_PTR, IO_IOCTL_DEVICE_IDENTIFY, temp) != MQX_OK) || !(temp[IO_IOCTL_ID_ATTR_ELEMENT] & IO_DEV_ATTR_REMOVE))
    {
        /*
        ** Device either doesn't support the identify command or is not removable.
        ** Assume we can fully enable caching
        */
        drive_ptr->WRITE_CACHE_POLICY = MFS_WRITE_BACK_CACHE;
        drive_ptr->READ_ONLY = false;
    }
    else
    {
        /* Device is removable. Enable Mixed mode write cache (only cache during consecutive file writes) */
        drive_ptr->WRITE_CACHE_POLICY = MFS_MIXED_MODE_CACHE;
        drive_ptr->READ_ONLY = (temp[IO_IOCTL_ID_ATTR_ELEMENT] & IO_DEV_ATTR_WRITE) == 0;
    }

    _queue_init(&drive_ptr->HANDLE_LIST, 0);

    error_code = MFS_Mount_drive_internal(drive_ptr);
    if (error_code != MFS_NO_ERROR)
    {
        MFS_unlock(drive_ptr);
        return error_code;
    }

/* Calculate the free space on disk */
#if MFSCFG_CALCULATE_FREE_SPACE_ON_OPEN
    error_code = MFS_Get_disk_free_space_internal(drive_ptr, NULL);
#endif

    MFS_unlock(drive_ptr);
    return error_code;
}


/*!
 * \brief Closes MFS driver.
 *
 * \param drive_ptr
 *
 * \return int32_t MQX_OK or an error.
 */
int32_t MFS_Close_Device(
    MFS_DRIVE_STRUCT_PTR drive_ptr)
{
    int32_t result = MFS_NO_ERROR;

    MFS_lock(drive_ptr);

    if (_queue_is_empty(&drive_ptr->HANDLE_LIST))
    {
        result = MFS_Unmount_drive_internal(drive_ptr);
        if (result == MFS_NO_ERROR)
        {
            result = MFS_sector_cache_flush(drive_ptr, 0, 0);
            MFS_sector_cache_free(drive_ptr);
        }
    }
    else
    {
        result = MFS_SHARING_VIOLATION;
    }

    MFS_unlock(drive_ptr);

    return result;
}
