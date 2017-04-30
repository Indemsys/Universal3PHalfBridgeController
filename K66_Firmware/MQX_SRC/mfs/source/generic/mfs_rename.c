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
*   This file contains the file specific interface functions
*   of MFS which need a filename as an input parameter.
*
*
*END************************************************************************/

#include <string.h>

#include "mfs.h"
#include "mfs_prv.h"


#if !MFSCFG_READ_ONLY


/*!
 * \brief Rename an existing file.
 *
 * \param drive_ptr
 * \param[in] old_pathname Directory and file name of file to be renamed.
 * \param[in] new_pathname Directory and file name to be given to the file.
 *
 * \return _mfs_error
 */
_mfs_error MFS_Rename_file(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    char *old_pathname,
    char *new_pathname)
{
    _mfs_error error_code;

    DIR_ENTRY_DISK_PTR source_entry, dest_entry;
    DIR_ENTRY_DISK temp_dir_entry;

    char *dir_name;
    char *old_filename;
    char *new_filename;

    char *old_pathname_tmp;
    char *new_pathname_tmp;

    uint32_t source_dir_cluster; /* 1st cluster of the source dir      */
    uint32_t dest_dir_cluster; /* 1st cluster of the destination dir */

    uint32_t source_entry_cluster;
    uint32_t dest_entry_cluster;

    uint32_t source_entry_index;
    uint32_t dest_entry_index;

    uint32_t source_prev_cluster = CLUSTER_INVALID;
    uint32_t dest_prev_cluster = CLUSTER_INVALID;

    if (old_pathname == NULL || *old_pathname == '\0' || new_pathname == NULL || *new_pathname == '\0')
    {
        return MFS_INVALID_PARAMETER;
    }

    old_pathname_tmp = old_pathname;
    new_pathname_tmp = new_pathname;

    while (*old_pathname_tmp == *new_pathname_tmp)
    {
        if (*old_pathname_tmp == '\0')
        {
            /* Old and new pathnames exacly match - nothing to be done */
            return MFS_NO_ERROR;
        }
        old_pathname_tmp++;
        new_pathname_tmp++;
    }

    if ((*old_pathname_tmp == '\0') && ((*new_pathname_tmp == '\\') || (*new_pathname_tmp == '/')))
    {
        /* Old pathname is path prefix for new pathname */
        return MFS_INVALID_PARAMETER;
    }

    error_code = MFS_lock_and_enter(drive_ptr, MFS_ENTER_READWRITE);
    if (error_code != MFS_NO_ERROR)
    {
        return error_code;
    }

    error_code = MFS_alloc_path(&dir_name);
    if (error_code != MFS_NO_ERROR)
    {
        MFS_leave_and_unlock(drive_ptr, 0);
        return error_code;
    }

    error_code = MFS_alloc_2paths(&new_filename, &old_filename);
    if (error_code != MFS_NO_ERROR)
    {
        MFS_free_path(dir_name);
        MFS_leave_and_unlock(drive_ptr, 0);
        return error_code;
    }

    /*
    ** Find the destination directory
    */
    error_code = MFS_Parse_pathname(dir_name, new_filename, new_pathname);
    if (error_code != MFS_NO_ERROR)
    {
        MFS_free_path(dir_name);
        MFS_free_path(new_filename);
        MFS_free_path(old_filename);
        MFS_leave_and_unlock(drive_ptr, 0);
        return error_code;
    }

    dest_dir_cluster = MFS_Find_directory(drive_ptr, dir_name, drive_ptr->CUR_DIR_CLUSTER);
    if (dest_dir_cluster == CLUSTER_INVALID)
    {
        error_code = MFS_PATH_NOT_FOUND;
        goto error_unlock_and_free_paths;
    }

    /*
    ** Find the source directory
    */
    MFS_Parse_pathname(dir_name, old_filename, old_pathname);
    source_dir_cluster = MFS_Find_directory(drive_ptr, dir_name, drive_ptr->CUR_DIR_CLUSTER);
    if (source_dir_cluster == CLUSTER_INVALID)
    {
        error_code = MFS_PATH_NOT_FOUND;
        goto error_unlock_and_free_paths;
    }

    /*
    ** Check new filename for validity
    */
    if (!MFS_is_valid_lfn(new_filename))
    {
        error_code = MFS_INVALID_PARAMETER;
        goto error_unlock_and_free_paths;
    }

    /*
    ** Make sure that the new filename is unused so far
    */
    dest_entry_index = 0;
    dest_entry_cluster = dest_dir_cluster;
    dest_entry = MFS_Find_directory_entry(drive_ptr, new_filename, &dest_entry_cluster, &dest_entry_index, &dest_prev_cluster, MFS_ATTR_ANY, &error_code);
    if (dest_entry != NULL || error_code)
    {
        if (!error_code)
            error_code = MFS_FILE_EXISTS;
        goto error_unlock_and_free_paths;
    }

    /*
    ** Find the source file
    */
    source_entry_index = 0;
    source_entry_cluster = source_dir_cluster;
    source_entry = MFS_Find_directory_entry(drive_ptr, old_filename, &source_entry_cluster, &source_entry_index, &source_prev_cluster, MFS_ATTR_ANY, &error_code);
    if (source_entry == NULL || error_code)
    {
        if (!error_code)
            error_code = MFS_FILE_NOT_FOUND;
    }
    else if (mqx_dtohc(source_entry->ATTRIBUTE) & MFS_ATTR_VOLUME_NAME)
    {
        /* Volume label is a special entry which cannot be altered this way */
        error_code = MFS_WRITE_FAULT;
    }
    else if (MFS_Find_handle(drive_ptr, source_entry_cluster, source_entry_index) != NULL)
    {
        /* File is currently open */
        error_code = MFS_SHARING_VIOLATION;
    }
    else
    {
        _mem_copy(source_entry, &temp_dir_entry, sizeof(DIR_ENTRY_DISK));
        if (source_dir_cluster != dest_dir_cluster)
        {
            /*
            ** Moving the file into different directory
            */
            dest_entry_index = 0;
            dest_entry_cluster = dest_dir_cluster;
            dest_entry = MFS_Create_directory_entry(drive_ptr, new_filename, mqx_dtohc(temp_dir_entry.ATTRIBUTE), &dest_entry_cluster, &dest_entry_index, &error_code);

            if (error_code == MFS_NO_ERROR)
            {
                /* Copy information from source entry, skipping the filename */
                _mem_copy(((uint8_t *)&temp_dir_entry) + 11, ((uint8_t *)dest_entry) + 11, sizeof(DIR_ENTRY_DISK) - 11);

                source_entry_index = 0;
                source_entry_cluster = source_dir_cluster;
                source_entry = MFS_Find_directory_entry(drive_ptr, old_filename, &source_entry_cluster, &source_entry_index, &source_prev_cluster, MFS_ATTR_ANY, &error_code);
                if (error_code)
                {
                    goto error_unlock_and_free_paths;
                }

                *source_entry->NAME = MFS_DEL_FILE;
                drive_ptr->DIR_SECTOR_DIRTY = true;

                error_code = MFS_remove_lfn_entries(drive_ptr, source_entry_cluster, source_entry_index, source_prev_cluster);
            }
        }
        else
        {
            /*
            ** Renaming the file in the same directory
            */
            *source_entry->NAME = MFS_DEL_FILE;
            drive_ptr->DIR_SECTOR_DIRTY = true;

            error_code = MFS_remove_lfn_entries(drive_ptr, source_entry_cluster, source_entry_index, source_prev_cluster);
            if (error_code)
            {
                goto error_unlock_and_free_paths;
            }
            dest_entry_index = 0;
            dest_entry_cluster = dest_dir_cluster;
            dest_entry = MFS_Create_directory_entry(drive_ptr, new_filename, mqx_dtohc(temp_dir_entry.ATTRIBUTE), &dest_entry_cluster, &dest_entry_index, &error_code);

            if (error_code == MFS_NO_ERROR)
            {
                /* Copy information from source entry, skipping the filename */
                _mem_copy(((uint8_t *)&temp_dir_entry) + 11, ((uint8_t *)dest_entry) + 11, sizeof(DIR_ENTRY_DISK) - 11);
            }
        }
    }

error_unlock_and_free_paths:
    MFS_free_path(dir_name);
    MFS_free_path(new_filename);
    MFS_free_path(old_filename);

    MFS_leave_and_unlock(drive_ptr, 0);
    return error_code;
}

#endif
