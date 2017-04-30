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
*   This file contains functions that search for specific directory entries
*   on the disk.
*
*
*END************************************************************************/

#include <string.h>

#include "mfs.h"
#include "mfs_prv.h"


/*!
 * \brief
 *
 * This function is implemented to mimic former directory sector buffer by means of new sector cache.
 * It is a replacement for former MFS_Read_directory_sector:
 * Reads ONE sector in the sector_buffer.
 * This function cannot read any sectors before the root directory.
 * The semaphore is assumed to be obtained.
 *
 * \param drive_ptr
 * \param[in] cluster Number of the cluster containing the sector to be read
 * \param[in] sector Index of the sector within the cluster.
 * \param error_ptr MFS pointer
 *
 * \return void * MFS pointer.
 */
void *MFS_Read_directory_sector(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    uint32_t cluster,
    uint16_t sector,
    _mfs_error_ptr error_ptr)
{
    uint32_t abs_sector;
    int error_code = MFS_NO_ERROR;

    if (cluster == 0)
    {
        /* Root directory is handled specially */
        abs_sector = drive_ptr->ROOT_START_SECTOR + sector;
        if (abs_sector >= drive_ptr->DATA_START_SECTOR)
        {
            abs_sector = MFS_INVALID_SECTOR;
        }
    }
    else
    {
        abs_sector = CLUSTER_TO_SECTOR(drive_ptr, cluster) + sector;
        if (abs_sector > drive_ptr->MEGA_SECTORS)
        {
            abs_sector = MFS_INVALID_SECTOR;
        }
    }

    /* There is only negligible penalty if the same sector is remapped, so simply unmap and map again without comparing the sector number */
    error_code = MFS_unmap_directory_sector(drive_ptr);

    if ((error_code == MFS_NO_ERROR) && (abs_sector != MFS_INVALID_SECTOR))
    {
        error_code = MFS_sector_map(drive_ptr, abs_sector, (void **)&(drive_ptr->DIR_SECTOR_PTR), MFS_MAP_MODE_MODIFY, 0);

        if (error_code == MFS_NO_ERROR)
        {
            drive_ptr->DIR_SECTOR_NUMBER = abs_sector;
        }
    }

    /* Fill in error code */
    if (error_ptr)
    {
        *error_ptr = error_code;
    }

    /* Return pointer to the buffer */
    return (drive_ptr->DIR_SECTOR_PTR);
}


/*!
 * \brief
 *
 * This function unmaps currently mapped directory sector.
 * It is part of emulation of former directory sector buffer.
 * The function is exported to allow for proper unmapping the sector upon drive flush or unlock.
 *
 * \param drive_ptr
 *
 * \return int Error code.
 */
int MFS_unmap_directory_sector(
    MFS_DRIVE_STRUCT_PTR drive_ptr)
{
    int error_code = MFS_NO_ERROR;

    /* If a valid sector is currently mapped unmap it first */
    if (drive_ptr->DIR_SECTOR_NUMBER != MFS_INVALID_SECTOR)
    {
        error_code = MFS_sector_unmap(drive_ptr, drive_ptr->DIR_SECTOR_NUMBER, drive_ptr->DIR_SECTOR_DIRTY);
    }

    /* Reset dir sector metadata */
    drive_ptr->DIR_SECTOR_NUMBER = MFS_INVALID_SECTOR;
    drive_ptr->DIR_SECTOR_PTR = NULL;
    drive_ptr->DIR_SECTOR_DIRTY = false;

    return error_code;
}


/*!
 * \brief
 *
 * \param[in] dattr The attribute of the existing file on disk.
 * \param[in] wattr The attribute we are trying to find out if it matches.
 *
 * \return bool True if the attributes match.
 */
bool MFS_Attribute_match(
    unsigned char dattr,
    unsigned char wattr)
{
    bool match;

    if (wattr & MFS_ATTR_ANY)
    {
        match = true;
    }
    else if (wattr == MFS_ATTR_LFN)
    {
        if (dattr == MFS_ATTR_LFN)
        {
            match = true;
        }
        else
        {
            match = false;
        }
    }
    else if (wattr == MFS_SEARCH_VOLUME)
    {
        if (dattr == MFS_SEARCH_VOLUME)
        {
            match = true;
        }
        else
        {
            match = false;
        }
    }
    else
    {
        if (wattr & ATTR_EXCLUSIVE)
        {
            if (dattr == (wattr & (~ATTR_EXCLUSIVE)))
            {
                match = true;
            }
            else
            {
                match = false;
            }
        }
        else
        {
            /* MFS_SEARCH_NORMAL returns all non-hidden, non-system files or directories */
            if (wattr == MFS_SEARCH_NORMAL)
            {
                if (dattr & (MFS_ATTR_HIDDEN_FILE | MFS_ATTR_SYSTEM_FILE))
                {
                    match = false;
                }
                else
                {
                    match = true;
                }
            }
            /* MFS_SEARCH_SUBDIR returns all non-hidden, non-system directories */
            else if (wattr == MFS_SEARCH_SUBDIR)
            {
                if ((dattr & (MFS_ATTR_DIR_NAME | MFS_ATTR_HIDDEN_FILE | MFS_ATTR_SYSTEM_FILE)) == MFS_ATTR_DIR_NAME)
                {
                    match = true;
                }
                else
                {
                    match = false;
                }
            }
            /* otherwise wattr must be subset of dattr */
            else
            {
                if ((dattr | (wattr & (MFS_ATTR_READ_ONLY | MFS_ATTR_HIDDEN_FILE | MFS_ATTR_SYSTEM_FILE | MFS_ATTR_DIR_NAME | MFS_ATTR_ARCHIVE))) == dattr)
                {
                    match = true;
                }
                else
                {
                    match = false;
                }
            }
        }
    }

    return (match);
}


/*!
 * \brief
 *
 * Search the directory starting at start_cluster for an entry
 * with the name of filename satisfying the attributes specified.
 * IF file_ptr = NULL, it returns the first entry at clust/index.
 * If file_ptr = "", it returns the next empty or deleted entry.
 *
 * \param[in] drive_ptr Drive context.
 * \param[in] file_ptr Specific name to search for, if NULL then first dir entry.
 * \param[in,out] start_cluster_ptr Start searching in this cluster (in).
 *                  Set to the cluster number in which search was satisfied (out).
 * \param[in,out] dir_index_ptr Start searching at this directory entry (in).
 *                  If entry is found the index of the next entry is returned (out).
 * \param[in,out] prev_cluster_ptr Set to the cluster number previous to start_cluster (in).
 *                  Set to the cluster number previous to *dir_cluster_ptr (out).
 * \param[in] attribute Search attribute, as per Find_first_file.
 * \param error_ptr The pointer carries the error information.
 *
 * \return DIR_ENTRY_DISK_PTR Pointer to a directory entry.
 */
DIR_ENTRY_DISK_PTR MFS_Find_directory_entry(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    char *file_ptr,
    uint32_t *start_cluster_ptr,
    uint32_t *dir_index_ptr,
    uint32_t *prev_cluster_ptr,
    unsigned char attribute,
    _mfs_error_ptr error_ptr)
{
    DIR_ENTRY_DISK_PTR dir_entry_ptr = NULL;
    char *lfn_ptr = NULL;
    bool found;
    bool lfn_flag;
    bool maybe = false;
    bool found_lfn = false;
    uint32_t current_cluster, current_index;
    _mfs_error error_code;
    uint32_t lfn_len = 0;
    uint16_t entries_per_sector;
    uint16_t k, i;
    char lfn_entry[40]; /* LFN entry contains up to 13 UTF16 characters, up to 40 bytes including null term are necessary when encoded using UTF8 */
    uint8_t fs_file[SFILENAME_SIZE];
    uint32_t prev_cluster = *prev_cluster_ptr;
    uint32_t dirname = 0;


    error_code = MFS_NO_ERROR;

    entries_per_sector = drive_ptr->ENTRIES_PER_SECTOR;
    current_cluster = *start_cluster_ptr;
    current_index = *dir_index_ptr;

    found = false;
    lfn_flag = false;

    /* If the name is a LFN, it will be treated differently */
    if (file_ptr && *file_ptr && !MFS_Dirname_valid(file_ptr))
    {
        lfn_flag = true;
        lfn_len = _strnlen(file_ptr, PATHNAME_SIZE);
        /* Set pointer just behind the end of the filename */
        lfn_ptr = file_ptr + lfn_len;
        maybe = true;
        found_lfn = false;
        /* We also need special treatement for a directory name */
        if (attribute == (ATTR_EXCLUSIVE | MFS_ATTR_DIR_NAME))
        {
            attribute = MFS_ATTR_LFN;
            dirname = 1;
        }
    }
    else if (file_ptr && *file_ptr)
    {
        MFS_Expand_dotfile(file_ptr, fs_file);
    }
    /*
    ** Search in all clusters within this directory
    */

    do
    {
        /*
        ** Search in all sectors within this cluster
        */
        for (k = (uint16_t)INDEX_TO_SECTOR(drive_ptr, current_index);
             ((current_cluster == 0) || (k < drive_ptr->SECTORS_PER_CLUSTER)) && found == false && current_cluster != CLUSTER_EOF;
             k++)
        {
            dir_entry_ptr = MFS_Read_directory_sector(drive_ptr, current_cluster, k, &error_code);
            if (dir_entry_ptr == NULL)
            {
                break;
            }

            /*
            ** Search in all entries within this sector
            */
            for (i = (uint16_t)INDEX_WITHIN_SECTOR(drive_ptr, current_index), dir_entry_ptr += i; i < entries_per_sector && found == false; i++)
            {
                if (*dir_entry_ptr->NAME == '\0') /* if NEVER USED entry */
                {
                    if (file_ptr) /* If not NULL                    */
                    {
                        if (!*file_ptr) /* but ""                         */
                        {
                            /* we found it */
                            found = true;
                            break;
                        }
                    }
                    return (NULL); /* Anyway, stop here-never used entry */
                    /* if the entry is relevant at all    */
                }
                else if ((unsigned char)*dir_entry_ptr->NAME == MFS_DEL_FILE)
                {
                    /* If DELETED */
                    if (file_ptr) /* If not NULL   */
                    {
                        if (!*file_ptr) /* but ""  */
                        {
                            found = true; /* we found it  */
                            break;
                        }
                    }
                }
                else /* If REGULAR ENTRY */
                {
                    if (file_ptr == NULL || *file_ptr)
                    {
                        if (MFS_Attribute_match(mqx_dtohc(dir_entry_ptr->ATTRIBUTE),
                                                attribute) == true)
                        {
                            if (!file_ptr)
                            {
                                found = true;
                                break;
                            }
                            else if (lfn_flag) /* Searching for a long name */
                            {
                                if (found_lfn)
                                {
                                    found = true;
                                    break;
                                }
                                if (!MFS_Attribute_match(mqx_dtohc(dir_entry_ptr->ATTRIBUTE),
                                                         MFS_ATTR_LFN))
                                {
                                    maybe = true; /* Reset maybe */
                                    dir_entry_ptr++;
                                    current_index++;
                                    continue; /* Not an LFN entry, skip it */
                                }
                                if (!maybe)
                                {
                                    dir_entry_ptr++;
                                    current_index++;
                                    continue;
                                }
                                int chunk_len;
                                chunk_len = MFS_lfn_extract((MFS_LNAME_ENTRY_PTR)dir_entry_ptr, lfn_entry);
                                if (lfn_ptr >= (file_ptr + chunk_len))
                                {
                                    lfn_ptr -= chunk_len;
                                    if (strncmp(lfn_entry, lfn_ptr, chunk_len))
                                    {
                                        lfn_ptr = file_ptr + lfn_len; /* reset ptr */
                                        maybe = false;
                                        dir_entry_ptr++;
                                        current_index++;
                                        continue; /* Strings don't match */
                                    }

                                    if (lfn_ptr == file_ptr)
                                    {
                                        found_lfn = true;
                                        if (dirname)
                                        {
                                            attribute = ATTR_EXCLUSIVE | MFS_ATTR_DIR_NAME;
                                        }
                                    }
                                }
                                else
                                {
                                    maybe = false;
                                    lfn_ptr = file_ptr + lfn_len; /* reset ptr */
                                }
                            }
                            else /* Searching for a short name */
                            {
                                found = !memcmp(fs_file, dir_entry_ptr->NAME, 11);
                            }
                        }
                        else
                        {
                            if (lfn_flag)
                            {
                                attribute = MFS_ATTR_LFN;
                                lfn_ptr = file_ptr + lfn_len; /* reset ptr */
                                maybe = true;
                                found_lfn = false;
                            }
                        }
                    }
                }

                if (found == false)
                {
                    dir_entry_ptr++;
                    current_index++;
                }
            }
        }

        if (found == false)
        {
            error_code = MFS_Increment_dir_index(drive_ptr, &current_cluster, &current_index, &prev_cluster);
            if (error_code)
            {
                break;
            }
        }

    } while (found == false && dir_entry_ptr && current_cluster != CLUSTER_EOF && current_cluster != CLUSTER_INVALID);

    if (found == false)
    {
        dir_entry_ptr = NULL;
    }
    else
    {
        *start_cluster_ptr = current_cluster;
        *prev_cluster_ptr = prev_cluster;
        *dir_index_ptr = current_index;
    }

    if (error_ptr)
    {
        *error_ptr = error_code;
    }

    return (dir_entry_ptr);
}


/*!
 * \brief
 *
 * Search through the file structure to find the specified
 * directory.  Will search either from the current directory or from the
 * root directory.  Returns the starting cluster number of the specified
 * directory.  If the specified directory cannot be found, then
 * CLUSTER_INVALID  is returned.
 *
 * \param[in] drive_ptr Drive context.
 * \param[in] path_ptr Specific directory to search for.
 * \param[in] first_cluster Start searching in this cluster, used for a relative search.
 *
 * \return uint32_t starting_cluster of the specified directory.
 */
uint32_t MFS_Find_directory(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    char *path_ptr,
    uint32_t first_cluster)
{
    DIR_ENTRY_DISK_PTR dir_entry_ptr;
    char *directory;
    uint32_t current_cluster;
    uint32_t dir_index;
    _mfs_error error_code;
    bool flag;
    uint32_t prev_cluster = CLUSTER_INVALID;
    unsigned char attribute;


    if (MFS_alloc_path(&directory) != MFS_NO_ERROR)
    {
        return (CLUSTER_INVALID);
    }

    if (*path_ptr == '\\' || *path_ptr == '/')
    {
        current_cluster = drive_ptr->ROOT_CLUSTER;
        path_ptr++;
    }
    else
    {
        current_cluster = first_cluster;
    }

    while (path_ptr)
    {
        if (*path_ptr)
        {
            path_ptr = MFS_Parse_next_filename(path_ptr, directory);

            if (!MFS_lfn_dirname_valid(directory))
            {
                current_cluster = CLUSTER_INVALID;
                break;
            }

            flag = false;
            if (current_cluster == 0)
                flag = true;
            if (drive_ptr->FAT_TYPE == MFS_FAT32)
            {
                if (current_cluster == drive_ptr->ROOT_CLUSTER)
                {
                    flag = true;
                }
            }

            if (flag)
            {
                /*
                ** Special treatment for '.' and '..' in root directory
                */
                dir_index = MFS_Is_dot_directory(directory);
                if (dir_index == 1)
                {
                    /* Return the value of the root cluster */
                    MFS_free_path(directory);
                    return drive_ptr->ROOT_CLUSTER;
                }
                else if (dir_index == 2)
                {
                    MFS_free_path(directory);
                    return (CLUSTER_INVALID);
                }
            }

            dir_index = 0;

            dir_entry_ptr = MFS_Find_directory_entry(drive_ptr, directory, &current_cluster, &dir_index, &prev_cluster, (~ATTR_EXCLUSIVE), &error_code);

            if (dir_entry_ptr == NULL)
            {
                current_cluster = CLUSTER_INVALID;
                break;
            }
            else
            {
                attribute = mqx_dtohc(dir_entry_ptr->ATTRIBUTE);

                if (attribute & MFS_ATTR_DIR_NAME)
                {
                    current_cluster = clustoh(drive_ptr, dir_entry_ptr->HFIRST_CLUSTER, dir_entry_ptr->LFIRST_CLUSTER);
                    if (current_cluster == 0 && drive_ptr->FAT_TYPE == MFS_FAT32)
                    {
                        current_cluster = drive_ptr->ROOT_CLUSTER;
                    }
                }
                else
                {
                    // Found an entry, but it is not a directory
                    MFS_free_path(directory);
                    return (CLUSTER_INVALID);
                }
            }
        }
        else
        {
            break;
        }
    }
    MFS_free_path(directory);

    return (current_cluster);
}
