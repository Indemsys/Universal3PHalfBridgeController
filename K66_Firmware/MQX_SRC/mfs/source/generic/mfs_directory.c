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
*   This file contains the functions that are used to create, remove, change,
*   and search through directories.
*
*
*END************************************************************************/

#include <string.h>

#include "mfs.h"
#include "mfs_prv.h"


#if !MFSCFG_READ_ONLY

/*!
 * \brief Create a subdirectory.
 *
 * \param drive_ptr
 * \param[in] pathname Pathname of the directory to be created.
 *
 * \return _mfs_error
 */
_mfs_error MFS_Create_subdir(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    char *pathname)
{
    DIR_ENTRY_DISK_PTR dir_entry_ptr;
    uint32_t dir_cluster;
    uint32_t parent_cluster;
    uint32_t free_cluster;
    uint32_t dir_index;
    _mfs_error error_code;
    char *temp_dirname;
    char *temp_filename;

    if ((pathname == NULL) || (*pathname == '\0'))
    {
        return MFS_INVALID_PARAMETER;
    }

    error_code = MFS_lock_and_enter(drive_ptr, MFS_ENTER_READWRITE);
    if (error_code != MFS_NO_ERROR)
    {
        return error_code;
    }

    error_code = MFS_alloc_2paths(&temp_dirname, &temp_filename);
    if (error_code != MFS_NO_ERROR)
    {
        MFS_leave_and_unlock(drive_ptr, 0);
        return error_code;
    }

    error_code = MFS_Parse_pathname(temp_dirname, temp_filename, pathname);
    if (error_code != MFS_NO_ERROR)
    {
        MFS_free_path(temp_dirname);
        MFS_free_path(temp_filename);
        MFS_leave_and_unlock(drive_ptr, 0);
        return error_code;
    }

    dir_cluster = drive_ptr->CUR_DIR_CLUSTER;
    parent_cluster = MFS_Find_directory(drive_ptr, temp_dirname, dir_cluster);

    dir_cluster = parent_cluster;
    if (MFS_is_valid_lfn(temp_filename))
    {
        if (dir_cluster != CLUSTER_INVALID)
        {
            /*
            ** We'll obtain a cluster for the new directory first. If we
            ** cannot create the directory afterwards, it is easier to re-free
            ** the cluster than to remove the new entry.
            */
            free_cluster = MFS_Find_unused_cluster_from(drive_ptr, drive_ptr->NEXT_FREE_CLUSTER);
            if (free_cluster != CLUSTER_INVALID)
            {
                error_code = MFS_Clear_cluster(drive_ptr, free_cluster);
                if (error_code)
                {
                    MFS_free_path(temp_dirname);
                    MFS_free_path(temp_filename);
                    MFS_leave_and_unlock(drive_ptr, 0);
                    return error_code;
                }
                error_code = MFS_Put_fat(drive_ptr, free_cluster, CLUSTER_EOF);
                dir_entry_ptr = MFS_Create_directory_entry(drive_ptr, temp_filename, MFS_ATTR_DIR_NAME, &dir_cluster, &dir_index, &error_code);
                if (error_code == MFS_NO_ERROR)
                {
                    clustod(dir_entry_ptr->HFIRST_CLUSTER, dir_entry_ptr->LFIRST_CLUSTER, free_cluster);
                    drive_ptr->DIR_SECTOR_DIRTY = true;

                    /*
                    ** We shall now create the "." and ".." entries.
                    */
                    dir_cluster = free_cluster;
                    dir_entry_ptr = MFS_Create_directory_entry(drive_ptr, ".", MFS_ATTR_DIR_NAME, &dir_cluster, &dir_index, &error_code);
                    if (error_code == MFS_NO_ERROR)
                    {
                        clustod(dir_entry_ptr->HFIRST_CLUSTER, dir_entry_ptr->LFIRST_CLUSTER, free_cluster);
                        drive_ptr->DIR_SECTOR_DIRTY = true;
                        dir_entry_ptr = MFS_Create_directory_entry(drive_ptr, "..", MFS_ATTR_DIR_NAME, &dir_cluster, &dir_index, &error_code);

                        if (error_code == MFS_NO_ERROR)
                        {
                            if (drive_ptr->FAT_TYPE == MFS_FAT32)
                            {
                                if (drive_ptr->ROOT_CLUSTER == parent_cluster)
                                {
                                    /*
                                    ** Even though the FAT32 root sector can be
                                    ** anywhere, it is identified as 0 when referenced
                                    ** through a directory entry
                                    */
                                    parent_cluster = 0;
                                }
                            }
                            clustod(dir_entry_ptr->HFIRST_CLUSTER, dir_entry_ptr->LFIRST_CLUSTER, parent_cluster);
                            drive_ptr->DIR_SECTOR_DIRTY = true;
                        }
                    }
                }
                else
                {
                    MFS_Put_fat(drive_ptr, free_cluster, CLUSTER_UNUSED);
                }
            }
            else
            {
                error_code = MFS_DISK_FULL;
            }
        }
        else
        {
            error_code = MFS_PATH_NOT_FOUND;
        }
    }
    else if (MFS_lfn_dirname_valid(temp_filename))
    {
        if (dir_cluster)
        {
            error_code = MFS_FILE_EXISTS;
        }
        else
        {
            error_code = MFS_CANNOT_CREATE_DIRECTORY;
        }
    }
    else
    {
        error_code = MFS_INVALID_PARAMETER;
    }

    MFS_free_path(temp_dirname);
    MFS_free_path(temp_filename);

    MFS_leave_and_unlock(drive_ptr, 0);
    return error_code;
}


/*!
 * \brief If the specified subdirectory has no entries, then remove the directory.
 *
 * \param drive_ptr
 * \param[in] pathname Pathname of the directory to be removed.
 *
 * \return _mfs_error
 */
_mfs_error MFS_Remove_subdir(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    char *pathname)
{
    _mfs_error error_code;
    _mfs_error error_code_tmp;

    FAT_CHAIN dir_chain;
    FAT_CHAIN subdir_chain;

    char *entry_name;
    DIR_ENTRY_DISK entry_copy;
    uint32_t entry_sector;
    uint32_t entry_index;
    uint32_t entry_loc;

    uint32_t first_cluster;

    if ((pathname == NULL) || (*pathname == '\0'))
    {
        return MFS_INVALID_PARAMETER;
    }

    error_code = MFS_lock_and_enter(drive_ptr, MFS_ENTER_READWRITE);
    if (error_code != MFS_NO_ERROR)
    {
        return error_code;
    }

    /* Find the directory in which the file shall be located */
    error_code = MFS_get_dir_chain(drive_ptr, pathname, &dir_chain, NULL, &entry_name);
    if (error_code == MFS_NO_ERROR)
    {
        if (entry_name[0] == '.' && (entry_name[1] == '\0' || (entry_name[1] == '.' && entry_name[2] == '\0')))
        {
            /* It is forbidden to delete "." or ".." entry */
            error_code = MFS_INVALID_PARAMETER;
        }
        else
        {
            /* Lookup entry  with the requested name in the directory */
            error_code = MFS_scan_dir_chain(drive_ptr, &dir_chain, entry_name, &entry_copy, &entry_sector, &entry_index, &entry_loc);
            if (error_code == MFS_NO_ERROR)
            {
                first_cluster = clustoh(drive_ptr, entry_copy.HFIRST_CLUSTER, entry_copy.LFIRST_CLUSTER);
                if ((mqx_dtohc(entry_copy.ATTRIBUTE) & MFS_ATTR_DIR_NAME) == 0)
                {
                    /* Not a directory */
                    error_code = MFS_WRITE_FAULT;  // legacy compatibility (this is probably not the best choice of error code)
                }
                else if (first_cluster == drive_ptr->CUR_DIR_CLUSTER)
                {
                    /* Cannot remove current directory */
                    error_code = MFS_ATTEMPT_TO_REMOVE_CURRENT_DIR;
                }
                else
                {
                    /* The subdirectory to be remove shall be empty, verify it */
                    error_code = MFS_chain_init(drive_ptr, &subdir_chain, first_cluster);
                    if (error_code == MFS_NO_ERROR)
                    {
                        error_code = MFS_check_dir_empty(drive_ptr, &subdir_chain);
                        if (error_code == MFS_NO_ERROR)
                        {
                            error_code = MFS_Release_chain(drive_ptr, first_cluster);
                            if (error_code == MFS_NO_ERROR || error_code == MFS_LOST_CHAIN)
                            {
                                error_code_tmp = MFS_free_dir_entry(drive_ptr, &dir_chain, entry_loc);
                                if (error_code == MFS_NO_ERROR)
                                {
                                    error_code = error_code_tmp;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    error_code_tmp = MFS_leave_and_unlock(drive_ptr, 0);
    if (error_code == MFS_NO_ERROR)
    {
        error_code = error_code_tmp;
    }

    return error_code;
}

#endif


/*!
 * \brief Return the pathname of the current directory.
 *
 * \param drive_ptr
 * \param[in,out] buffer_address Address of the buffer where the pathname is to be written.
 *
 * \return _mfs_error
 */
_mfs_error MFS_Get_current_dir(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    char *buffer_address)
{
    _mfs_error error_code;

    if (buffer_address == NULL)
    {
        return MFS_INVALID_PARAMETER;
    }

    error_code = MFS_lock_and_enter(drive_ptr, 0);
    if (error_code != MFS_NO_ERROR)
    {
        return error_code;
    }

    strcpy(buffer_address, drive_ptr->CURRENT_DIR);

    MFS_leave_and_unlock(drive_ptr, 0);
    return error_code;
}


/*!
 * \brief Change the current directory.
 *
 * \param drive_ptr
 * \param[in] pathname Pathname of the directory to become the current dir.
 *
 * \return _mfs_error
 */
_mfs_error MFS_Change_current_dir(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    char *pathname)
{
    _mfs_error error_code;

    FAT_CHAIN dir_chain;
    FAT_CHAIN *dir_chain_ptr;

    if ((pathname == NULL) || (*pathname == '\0'))
    {
        return MFS_INVALID_PARAMETER;
    }

    pathname = MFS_Parse_Out_Device_Name(pathname);

    error_code = MFS_lock_and_enter(drive_ptr, 0);
    if (error_code != MFS_NO_ERROR)
    {
        return error_code;
    }

    error_code = MFS_get_dir_chain(drive_ptr, pathname, &dir_chain, &dir_chain_ptr, NULL);
    if (error_code == MFS_NO_ERROR)
    {
        if (dir_chain_ptr == &dir_chain)
        {
            /* if the dir_chain_ptr points to locally allocated chain then copy it to preallocated space in the drive context */
            drive_ptr->CUR_DIR_CHAIN_PREALLOC = dir_chain;
            drive_ptr->CUR_DIR_CHAIN_PTR = &drive_ptr->CUR_DIR_CHAIN_PREALLOC;
        }
        else
        {
            /* otherwise just set the current dir pointer in the drive context */
            drive_ptr->CUR_DIR_CHAIN_PTR = dir_chain_ptr;
        }

        /* in either case, set the current dir cluster and store current dir path (legacy) */
        drive_ptr->CUR_DIR_CLUSTER = drive_ptr->CUR_DIR_CHAIN_PTR->HEAD_CLUSTER;
        _io_path_add(drive_ptr->CURRENT_DIR, sizeof(drive_ptr->CURRENT_DIR), pathname);
    }

    MFS_leave_and_unlock(drive_ptr, 0);
    return error_code;
}


/*!
 * \brief Check if directory exist on device.
 *
 * \param drive_ptr
 * \param[in] pathname Pathname of the directory to become the current dir.
 *
 * \return _mfs_error
 */
_mfs_error MFS_Check_dir_exist(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    char *pathname)
{
    _mfs_error error_code;
    FAT_CHAIN dir_chain;

    // for empty string return error
    if ((pathname == NULL) || (*pathname == '\0'))
    {
        return MFS_INVALID_PARAMETER;
    }

    error_code = MFS_lock_and_enter(drive_ptr, 0);
    if (error_code != MFS_NO_ERROR)
    {
        return error_code;
    }

    error_code = MFS_get_dir_chain(drive_ptr, pathname, &dir_chain, NULL, NULL);

    MFS_leave_and_unlock(drive_ptr, 0);
    return error_code;
}


/*!
 * \brief Sets the drive volume. Assumes the drive is NOT locked.
 *
 * \param drive_ptr
 * \param[in] volume_name Name to use for the volune.
 *
 * \return _mfs_error
 */
#if !MFSCFG_READ_ONLY
_mfs_error MFS_Set_volume(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    char *volume_name)
{
    DIR_ENTRY_DISK_PTR dir_entry_ptr;
    _mfs_error error_code;
    uint32_t vol_cluster, vol_index;
    uint32_t prev_cluster = CLUSTER_INVALID;

    if (volume_name == NULL)
    {
        return (MFS_INVALID_PARAMETER);
    }

    if (*volume_name == '\0')
    {
        return (MFS_INVALID_PARAMETER);
    }

    error_code = MFS_lock_and_enter(drive_ptr, MFS_ENTER_READWRITE);

    if (!error_code)
    {
        vol_cluster = drive_ptr->ROOT_CLUSTER;
        vol_index = 0;
        dir_entry_ptr = MFS_Find_directory_entry(drive_ptr, NULL, &vol_cluster, &vol_index, &prev_cluster, MFS_ATTR_VOLUME_NAME, &error_code);

        if (dir_entry_ptr)
        {
            MFS_Expand_dotfile(volume_name, dir_entry_ptr->NAME);
            *dir_entry_ptr->ATTRIBUTE = MFS_ATTR_VOLUME_NAME;
            drive_ptr->DIR_SECTOR_DIRTY = true;
        }
        else if (!error_code)
        {
            vol_cluster = drive_ptr->ROOT_CLUSTER;
            vol_index = 0;
            dir_entry_ptr = MFS_Find_directory_entry(drive_ptr, "", &vol_cluster, &vol_index, &prev_cluster, MFS_ATTR_ANY, &error_code);
            if (dir_entry_ptr)
            {
                MFS_Expand_dotfile(volume_name, dir_entry_ptr->NAME);
                *dir_entry_ptr->ATTRIBUTE = MFS_ATTR_VOLUME_NAME;
                drive_ptr->DIR_SECTOR_DIRTY = true;
            }
        }
    }

    MFS_leave_and_unlock(drive_ptr, 0);
    return error_code;
}

#endif


/*!
 * \brief Gets the drive volume. Assumes the drive is NOT locked.
 *
 * \param drive_ptr
 * \param[out] volume_name Name of the volune.
 *
 * \return _mfs_error
 */
_mfs_error MFS_Get_volume(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    char *volume_name)
{
    DIR_ENTRY_DISK_PTR dir_entry_ptr;
    _mfs_error error_code;
    uint32_t vol_cluster, vol_index, i;
    uint32_t prev_cluster = CLUSTER_INVALID;

    if (volume_name == NULL)
    {
        return (MFS_INVALID_PARAMETER);
    }

    error_code = MFS_lock_and_enter(drive_ptr, 0);

    if (!error_code)
    {
        vol_cluster = drive_ptr->ROOT_CLUSTER;
        vol_index = 0;
        dir_entry_ptr = MFS_Find_directory_entry(drive_ptr, NULL, &vol_cluster, &vol_index, &prev_cluster, MFS_ATTR_VOLUME_NAME, &error_code);

        if (dir_entry_ptr)
        {
            for (i = 0; i < SFILENAME_SIZE - 1; i++)
            {
                *volume_name++ = (char)(dir_entry_ptr->NAME[i]);
            }
        }
        *volume_name = '\0';
    }

    MFS_leave_and_unlock(drive_ptr, 0);
    return error_code;
}

/* EOF */
