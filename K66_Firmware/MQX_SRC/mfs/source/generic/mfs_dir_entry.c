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
*   This file contains the functions that are used to manipulate FAT directory
*   entries.
*
*
*END************************************************************************/

#include <string.h>

#include "mfs.h"
#include "mfs_prv.h"


#if !MFSCFG_READ_ONLY


/*!
 * \brief
 *
 * Try to create a new file.  Search for an unused directory entry, setup the
 * entry and write back the directory.
 *
 * \param[in] drive_ptr The drive on which to operate.
 * \param[in] filename_ptr Pointer to the file's name.
 * \param[in] attribute Attribute to be assigned to the file.
 * \param[in,out] dir_cluster_ptr Indicates cluster in which the entry was put.
 * \param[in,out] dir_index_ptr Index of the location of new file within the directory.
 * \param[in,out] error_ptr error_return_address
 *
 * \return DIR_ENTRY_DISK_PTR Pointer to the directory entry.
 */
DIR_ENTRY_DISK_PTR MFS_Create_directory_entry(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    char *filename_ptr,
    char attribute,
    uint32_t *dir_cluster_ptr,
    uint32_t *dir_index_ptr,
    uint32_t *error_ptr)
{
    DIR_ENTRY_DISK_PTR dir_entry_ptr, dir_entry_ptr1;
    DIR_ENTRY_DISK_PTR temp_entry_ptr;
    TIME_STRUCT time;
    DATE_STRUCT clk_time;
    uint32_t dir_index, saved_index, temp_index;
    uint32_t entry_cluster, saved_cluster, temp_cluster;
    uint32_t needed_entries, i;
    int32_t length;
    bool found_all = false;
    char temp_name[SFILENAME_SIZE + 1];
    uint8_t short_name[SFILENAME_SIZE];
    unsigned char checksum = 0;
    uint32_t prev_cluster = CLUSTER_INVALID, saved_prev_cluster, temp_prev_cluster;

    dir_entry_ptr = NULL;
    entry_cluster = *dir_cluster_ptr;
    dir_index = 0;

    /*
    ** Check for duplicate file
    */
    if (filename_ptr == NULL)
    {
        *error_ptr = MFS_INVALID_PARAMETER;
    }
    else if (*filename_ptr == '\0')
    {
        *error_ptr = MFS_FILE_NOT_FOUND;
    }
    else if ((dir_entry_ptr = MFS_Find_directory_entry(drive_ptr, filename_ptr, &entry_cluster, &dir_index, &prev_cluster, MFS_ATTR_ANY, error_ptr)) != NULL)
    {
        *error_ptr = MFS_FILE_EXISTS;
    }
    else
    {
        /*
        ** Search for an empty slot
        */

        dir_index = 0;
        dir_entry_ptr = MFS_Find_directory_entry(drive_ptr, "", &entry_cluster, &dir_index, &prev_cluster, attribute, error_ptr);


        if (MFS_Dirname_valid(filename_ptr))
        {
            found_all = true;
        }

        if (dir_entry_ptr == NULL && !*error_ptr)
        {
            found_all = true;
        }

        /*
        ** Save it now because we might not go into the while loop
        ** If we don't go into while, we lose original values when these
        ** variables are restored after the while loop
        */
        saved_cluster = entry_cluster;
        saved_index = dir_index;

        while (!found_all)
        {
            /* Calculate the amount of extra entries needed for LFN's */
            saved_cluster = entry_cluster;
            saved_index = dir_index;

            for (needed_entries = (strlen(filename_ptr) + 12) / 13; needed_entries >= 1 && !found_all; needed_entries--)
            {
                *error_ptr = MFS_Increment_dir_index(drive_ptr, &entry_cluster, &dir_index, NULL);
                if (entry_cluster == CLUSTER_EOF && !*error_ptr)
                {
                    /* This means the LFN will span a cluster. */
                    found_all = true;
                    break;
                }
                temp_index = dir_index;
                temp_cluster = entry_cluster;
                temp_entry_ptr = MFS_Find_directory_entry(drive_ptr, "", &entry_cluster, &dir_index, &prev_cluster, MFS_ATTR_ANY, error_ptr);
                if (*error_ptr)
                {
                    return NULL;
                }
                if (!temp_entry_ptr)
                {
                    found_all = true;
                    dir_entry_ptr = NULL;
                    break;
                }
                else if (dir_index == temp_index && temp_cluster == entry_cluster)
                {
                    continue;
                }
                else
                {
                    break;
                }
            }

            if (needed_entries == 0)
            {
                found_all = true;
            }

            if (found_all)
            {
                dir_entry_ptr = MFS_Find_directory_entry(drive_ptr, "", &saved_cluster, &saved_index, &saved_prev_cluster, MFS_ATTR_ANY, error_ptr);
            }
        }

        entry_cluster = saved_cluster;
        dir_index = saved_index;


        if (dir_entry_ptr == NULL && !*error_ptr)
        {
            /*
            ** Empty spot not found... get a new cluster and use the first entry.
            */
            dir_index = 0;
            if (entry_cluster == 0)
            {
                *error_ptr = MFS_ROOT_DIR_FULL;
                return (NULL);
            }
            else
            {
                *error_ptr = MFS_Extend_chain(drive_ptr, entry_cluster, 1, &entry_cluster);
                if (*error_ptr == MFS_NO_ERROR)
                {
                    *error_ptr = MFS_Clear_cluster(drive_ptr, entry_cluster);
                    if (*error_ptr)
                    {
                        return (NULL);
                    }
                    dir_entry_ptr = MFS_Read_directory_sector(drive_ptr, entry_cluster, 0, error_ptr);
                    if (*error_ptr)
                    {
                        return (NULL);
                    }
                }
            }
        }
    }

    if (*error_ptr == MFS_NO_ERROR)
    {
        /* At this point we have one of the following cases
        **
        ** 1. We have a normal 8.3 filename and an empty slot for it
        ** 2. We have a LFN, and a slot which has enough consecutive free slots
        **    to store the LFN, followed by the actual entry
        */

        /* If the file is a normal 8.3 file */
        if (MFS_Dirname_valid(filename_ptr))
        {
            _mem_zero(dir_entry_ptr, sizeof(DIR_ENTRY_DISK));
            mqx_htodc(dir_entry_ptr->ATTRIBUTE, attribute);
            MFS_Expand_dotfile(filename_ptr, dir_entry_ptr->NAME);

            _time_get(&time);
            _time_to_date(&time, &clk_time);
            NORMALIZE_DATE(&clk_time);
            mqx_htods(dir_entry_ptr->TIME, PACK_TIME(clk_time));
            mqx_htods(dir_entry_ptr->DATE, PACK_DATE(clk_time));
            drive_ptr->DIR_SECTOR_DIRTY = true;
        }
        else
        {
            /* Case where the file is a LFN */
            length = strlen(filename_ptr);

            /* Get the 8.3 name and calculate the checksum */
            temp_index = 0;
            temp_cluster = *dir_cluster_ptr;
            *error_ptr = MFS_lfn_to_sfn(filename_ptr, temp_name);

            if (!*error_ptr)
            {
                do
                {
                    dir_entry_ptr1 = MFS_Find_directory_entry(drive_ptr, temp_name, &temp_cluster, &temp_index, &temp_prev_cluster, MFS_ATTR_ANY, error_ptr);
                    if (!*error_ptr && dir_entry_ptr1 != NULL)
                    {
                        *error_ptr = MFS_increment_lfn(temp_name);
                        temp_index = 0;
                    }
                } while (!*error_ptr && dir_entry_ptr1 != NULL);
                if (*error_ptr)
                {
                    return NULL;
                }

                dir_entry_ptr = MFS_Find_directory_entry(drive_ptr, "", &entry_cluster, &dir_index, &prev_cluster, attribute, error_ptr);

                /*
                ** The memory should be cleared after finding the directory entry.
                */
                _mem_zero(dir_entry_ptr, sizeof(DIR_ENTRY_DISK));

                /* Setup the final slot */
                ((MFS_LNAME_ENTRY_PTR)dir_entry_ptr)->ID |= MFS_LFN_END;
                drive_ptr->DIR_SECTOR_DIRTY = true;

                MFS_Expand_dotfile(temp_name, short_name);
                checksum = MFS_lfn_checksum(short_name);
            }

            while (length && !*error_ptr)
            {
                i = length - ((length - 1) % 13 + 1);
                *error_ptr = MFS_lfn_name_to_entry(filename_ptr + i, (MFS_LNAME_ENTRY_PTR)dir_entry_ptr);

                /* Set the entry number, the checksum value and the attribute */
                ((MFS_LNAME_ENTRY_PTR)dir_entry_ptr)->ID |= (length + 12) / 13;
                ((MFS_LNAME_ENTRY_PTR)dir_entry_ptr)->ALIAS_CHECKSUM = checksum;
                ((MFS_LNAME_ENTRY_PTR)dir_entry_ptr)->ATTR = MFS_ATTR_LFN;
                drive_ptr->DIR_SECTOR_DIRTY = true;

                length -= (length - i);
                if (length < 0)
                {
                    length = 0;
                }

                if (length && !*error_ptr)
                {

                    dir_entry_ptr = MFS_Find_directory_entry(drive_ptr, "", &entry_cluster, &dir_index, &prev_cluster, attribute, error_ptr);
                    if (dir_entry_ptr == NULL && !*error_ptr)
                    {
                        /* No empty spots... We need to get a new cluster */
                        dir_index = 0;
                        if (entry_cluster == 0)
                        {
                            *error_ptr = MFS_ROOT_DIR_FULL;
                            return (NULL);
                        }
                        else
                        {
                            *error_ptr = MFS_Extend_chain(drive_ptr, entry_cluster, 1, &entry_cluster);
                            if (*error_ptr == MFS_NO_ERROR)
                            {
                                *error_ptr = MFS_Clear_cluster(drive_ptr, entry_cluster);
                                if (*error_ptr)
                                {
                                    return (NULL);
                                }
                                dir_entry_ptr = MFS_Read_directory_sector(drive_ptr, entry_cluster, 0, error_ptr);
                                if (*error_ptr)
                                {
                                    return (NULL);
                                }
                            }
                        }
                    }
                    _mem_zero(dir_entry_ptr, sizeof(DIR_ENTRY_DISK));
                    drive_ptr->DIR_SECTOR_DIRTY = true;
                }
            }

            /* LFN is written, so write the actual entry */
            if (!*error_ptr)
            {
                dir_entry_ptr = MFS_Find_directory_entry(drive_ptr, "", &entry_cluster, &dir_index, &prev_cluster, attribute, error_ptr);
                if (dir_entry_ptr == NULL && !*error_ptr)
                {
                    dir_index = 0;
                    if (entry_cluster == 0)
                    {
                        *error_ptr = MFS_ROOT_DIR_FULL;
                        return (NULL);
                    }
                    else
                    {
                        *error_ptr = MFS_Extend_chain(drive_ptr, entry_cluster, 1, &entry_cluster);
                        if (*error_ptr == MFS_NO_ERROR)
                        {
                            *error_ptr = MFS_Clear_cluster(drive_ptr, entry_cluster);
                            if (*error_ptr)
                            {
                                return (NULL);
                            }
                            dir_entry_ptr = MFS_Read_directory_sector(drive_ptr, entry_cluster, 0, error_ptr);
                            if (*error_ptr)
                            {
                                return (NULL);
                            }
                        }
                    }
                }

                _mem_zero(dir_entry_ptr, sizeof(DIR_ENTRY_DISK));
                mqx_htodc(dir_entry_ptr->ATTRIBUTE, attribute);
                MFS_Expand_dotfile(temp_name, dir_entry_ptr->NAME);

                _time_get(&time);
                _time_to_date(&time, &clk_time);
                NORMALIZE_DATE(&clk_time);
                mqx_htods(dir_entry_ptr->TIME, PACK_TIME(clk_time));
                mqx_htods(dir_entry_ptr->DATE, PACK_DATE(clk_time));
                drive_ptr->DIR_SECTOR_DIRTY = true;
            }
        }

        if (*error_ptr)
        {
            return (NULL);
        }
    }

    *dir_cluster_ptr = entry_cluster;
    *dir_index_ptr = dir_index;
    return (dir_entry_ptr);
}


/*!
 * \brief Create a file.
 *
 * If a file exists, and the Overwrite flag is false, NULL is returned.
 * When an entry is found and overwritten, the error is set to
 * MFS_FILE_EXISTS, but the returned PTR points to a valid location.
 * If error is NO_ERROR, a new entry has been created.
 * The attr of the new entry has to be compatible with the old one,
 * or WRITE_FAULT is returned.
 * If the old one is Read only, ACCESS_DENIED is returned.
 * The semaphore is assumed locked and the fat up to date
 *
 * \param[in] drive_ptr The drive on which to operate.
 * \param[in] attr Attribute to be given to the new file.
 * \param[in] pathname Directory and file name to be given to the new file.
 * \param[out] cluster_ptr Indicates cluster in which the entry was put.
 * \param[out] index_ptr Index of the location of new file within the directory.
 * \param[in,out] error_ptr Error code is written to this address.
 *
 * \return DIR_ENTRY_DISK_PTR Pointer to the buffer on disk.
 */
DIR_ENTRY_DISK_PTR MFS_Create_entry_slave(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    unsigned char attr,
    char *pathname,
    uint32_t *cluster_ptr,
    uint32_t *index_ptr,
    _mfs_error_ptr error_ptr)
{
    DIR_ENTRY_DISK_PTR dir_entry_ptr;
    char *temp_dirname;
    char *temp_filename;
    uint32_t dir_cluster;
    uint32_t dir_index;
    uint32_t vol_cluster;
    uint32_t vol_index;
    uint32_t prev_cluster;
    _mfs_error error_code;

    dir_index = 0;
    dir_cluster = drive_ptr->CUR_DIR_CLUSTER;
    dir_entry_ptr = NULL;

    error_code = MFS_alloc_2paths(&temp_dirname, &temp_filename);
    if (error_code)
    {
        if (error_ptr)
        {
            *error_ptr = error_code;
        }
        return (NULL);
    }

    error_code = MFS_Parse_pathname(temp_dirname, temp_filename, pathname);
    if (error_code == MFS_NO_ERROR)
    {
        dir_cluster = MFS_Find_directory(drive_ptr, temp_dirname, dir_cluster);

        if (dir_cluster == CLUSTER_INVALID)
        {
            error_code = MFS_PATH_NOT_FOUND;
        }
        else
        {
            if (MFS_is_valid_lfn(temp_filename))
            {
                if (attr & MFS_ATTR_VOLUME_NAME)
                {
                    /*
                    ** If creating a volume label, it must be in the root
                    ** directory.
                    */
                    if (dir_cluster == 0)
                    {
                        vol_cluster = 0;
                        vol_index = 0;
                        dir_entry_ptr = MFS_Find_directory_entry(drive_ptr, NULL, &vol_cluster, &vol_index, &prev_cluster, MFS_ATTR_VOLUME_NAME, &error_code);
                        if (dir_entry_ptr)
                        {
                            error_code = MFS_ALREADY_ASSIGNED;
                        }
                    }
                    else
                    {
                        error_code = MFS_ACCESS_DENIED;
                    }
                }

                if (error_code == MFS_NO_ERROR)
                {
                    dir_entry_ptr = MFS_Create_directory_entry(drive_ptr, temp_filename, attr, &dir_cluster, &dir_index, &error_code);

                    /* if dir_entry_ptr == NULL
                    ** we couldn't create a new entry.
                    ** if not NULL, it may be a brand new entry (MFS_NO_ERROR) or
                    ** an old one (MFS_FILE_EXISTS)
                    */

                    if (error_code != MFS_NO_ERROR)
                    {
                        dir_entry_ptr = NULL;
                    }

                    *cluster_ptr = dir_cluster;
                    *index_ptr = dir_index;
                }
            }
            else
            {
                error_code = MFS_INVALID_PARAMETER;
                dir_entry_ptr = NULL;
            }
        }
    }

    if (error_ptr)
    {
        *error_ptr = error_code;
    }

    MFS_free_path(temp_filename);
    MFS_free_path(temp_dirname);

    return (dir_entry_ptr);
}

#endif


/*!
 * \brief
 *
 * \param[in] drive_ptr The drive on which to operate.
 * \param[in,out] cluster_ptr The initial/next cluster #.
 * \param[in,out] index_ptr The initial/next index.
 * \param[in,out] prev_cluster_ptr The prev cluster #.
 *
 * \return _mfs_error
 */
_mfs_error MFS_Increment_dir_index(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    uint32_t *cluster_ptr,
    uint32_t *index_ptr,
    uint32_t *prev_cluster_ptr)
{
    uint32_t index;
    uint32_t cluster;
    _mfs_error error_code;

    error_code = MFS_NO_ERROR;
    index = *index_ptr;
    cluster = *cluster_ptr;

    index++;

    if (!(index & (drive_ptr->ENTRIES_PER_SECTOR - 1)))
    {
        /*
        ** if the index count LSB's wrapped to 0 (new sector)
        */
        if (cluster != 0)
        {
            if (INDEX_TO_SECTOR(drive_ptr, index) >= drive_ptr->SECTORS_PER_CLUSTER)
            {
                /*
                ** If we are over the size of a cluster
                */

                error_code = MFS_get_cluster_from_fat(drive_ptr, cluster, &cluster);
                if (error_code == MFS_NO_ERROR)
                {
                    index = 0;
                }
            }
        }
        else
        {
            if (index >= drive_ptr->ROOT_ENTRIES)
            {
                index = 0;
                cluster = CLUSTER_INVALID;
            }
        }
    }

    if ((cluster != *cluster_ptr) && (cluster != CLUSTER_INVALID))
    {
        if (prev_cluster_ptr != NULL)
        {
            *prev_cluster_ptr = *cluster_ptr;
        }
    }

    *index_ptr = index;
    *cluster_ptr = cluster;

    return (error_code);
}


/*!
 * \brief Directory entry abstraction
 *
 * \param drive_ptr
 * \param dir_entry
 * \param dir_entry_disk
 *
 * \return void
 */
void MFS_dir_entry_from_disk(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    DIR_ENTRY *dir_entry,
    DIR_ENTRY_DISK *dir_entry_disk)
{
    dir_entry->FILE_SIZE = mqx_dtohl(dir_entry_disk->FILE_SIZE);
    dir_entry->HEAD_CLUSTER = clustoh(drive_ptr, dir_entry_disk->HFIRST_CLUSTER, dir_entry_disk->LFIRST_CLUSTER);

    dir_entry->WRITE_DATE = mqx_dtohs(dir_entry_disk->DATE);
    dir_entry->WRITE_TIME = mqx_dtohs(dir_entry_disk->TIME);
}


/*!
 * \brief
 *
 * \param drive_ptr
 * \param dir_entry
 * \param dir_entry_disk
 *
 * \return void
 */
void MFS_dir_entry_to_disk(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    DIR_ENTRY *dir_entry,
    DIR_ENTRY_DISK *dir_entry_disk)
{
    mqx_htodl(dir_entry_disk->FILE_SIZE, dir_entry->FILE_SIZE);
    clustod(dir_entry_disk->HFIRST_CLUSTER, dir_entry_disk->LFIRST_CLUSTER, dir_entry->HEAD_CLUSTER);

    mqx_htods(dir_entry_disk->TIME, dir_entry->WRITE_TIME);
    mqx_htods(dir_entry_disk->DATE, dir_entry->WRITE_DATE);
}


/*!
 * \brief
 *
 * \param[in] drive_ptr The drive on which to operate.
 * \param dir_entry
 * \param entry_sector
 * \param entry_index
 *
 * \return _mfs_error
 */
_mfs_error MFS_dir_entry_fetch(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    DIR_ENTRY *dir_entry,
    uint32_t entry_sector,
    uint32_t entry_index)
{
    _mqx_int error_code;
    void *buf_ptr;

    dir_entry->ENTRY_SECTOR = entry_sector;
    dir_entry->ENTRY_INDEX = entry_index;

    error_code = MFS_sector_map(drive_ptr, dir_entry->ENTRY_SECTOR, &buf_ptr, MFS_MAP_MODE_READONLY, 0);
    if (error_code != MFS_NO_ERROR)
    {
        return error_code;
    }

    dir_entry->DIRTY = 0;

    MFS_dir_entry_from_disk(drive_ptr, dir_entry, ((DIR_ENTRY_DISK *)buf_ptr) + dir_entry->ENTRY_INDEX);

    error_code = MFS_sector_unmap(drive_ptr, dir_entry->ENTRY_SECTOR, 0);

    return error_code;
}


/*!
 * \brief
 *
 * \param[in] drive_ptr The drive on which to operate.
 * \param dir_entry
 *
 * \return _mfs_error
 */
_mfs_error MFS_dir_entry_store(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    DIR_ENTRY *dir_entry)
{
    _mqx_int error_code;
    void *buf_ptr;

    error_code = MFS_sector_map(drive_ptr, dir_entry->ENTRY_SECTOR, &buf_ptr, MFS_MAP_MODE_MODIFY, 0);
    if (error_code != MFS_NO_ERROR)
    {
        return error_code;
    }

    MFS_dir_entry_to_disk(drive_ptr, dir_entry, ((DIR_ENTRY_DISK *)buf_ptr) + dir_entry->ENTRY_INDEX);

    error_code = MFS_sector_unmap(drive_ptr, dir_entry->ENTRY_SECTOR, 1);

    if (error_code == MFS_NO_ERROR)
    {
        dir_entry->DIRTY = 0;
    }

    return error_code;
}


/*!
 * \brief
 *
 * \param[in] drive_ptr The drive on which to operate.
 * \param dir_entry
 *
 * \return _mfs_error
 */
_mfs_error MFS_dir_entry_sync(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    DIR_ENTRY *dir_entry)
{
    if (dir_entry->DIRTY)
    {
        return MFS_dir_entry_store(drive_ptr, dir_entry);
    }

    return MFS_NO_ERROR;
}


/*!
 * \brief Updates write timestamp of the file, and syncs the directory entry.
 *
 * \param[in] drive_ptr The drive on which to operate.
 * \param[in] handle Update this file.
 *
 * \return _mfs_error
 */
_mfs_error MFS_update_entry(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    MFS_HANDLE_PTR handle)
{
    if (handle->TOUCHED)
    {
        TIME_STRUCT time;
        DATE_STRUCT date;

        _time_get(&time);
        _time_to_date(&time, &date);

        NORMALIZE_DATE(&date);
        handle->DIR_ENTRY->WRITE_TIME = PACK_TIME(date);
        handle->DIR_ENTRY->WRITE_DATE = PACK_DATE(date);

        handle->DIR_ENTRY->DIRTY = 1;

        handle->TOUCHED = 0;
    }

    return MFS_dir_entry_sync(drive_ptr, handle->DIR_ENTRY);
}


/*!
 * \brief Updates directory entries of all open files.
 *
 * \param[in] drive_ptr The drive on which to operate.
 *
 * \return _mfs_error
 */
_mfs_error MFS_update_entries(
    MFS_DRIVE_STRUCT_PTR drive_ptr)
{
    MFS_HANDLE_PTR handle;
    _mfs_error error_code = MFS_NO_ERROR;
    _mfs_error error_temp;

    handle = (MFS_HANDLE_PTR)_queue_head(&(drive_ptr->HANDLE_LIST));
    while (handle)
    {
        error_temp = MFS_update_entry(drive_ptr, handle);
        if (error_code == MFS_NO_ERROR)
        {
            error_code = error_temp;
        }
        handle = (MFS_HANDLE_PTR)_queue_next(&drive_ptr->HANDLE_LIST, (QUEUE_ELEMENT_STRUCT_PTR)handle);
    }

    return error_code;
}


/*!
 * \brief Extract SFN from disk format to 8.3 notation encoded in UTF-8
 *
 * \param[in] dirent_sfn
 * \param[out] utf8_sfn
 *
 * \return void
 */
void MFS_sfn_from_disk(uint8_t *disk_sfn, char *utf8_sfn)
{
    int i;
    char *utf8_end = utf8_sfn;

    for (i = 0; i < 8; i++)
    {
        utf8_encode(*disk_sfn, &utf8_sfn, NULL);
        if (*disk_sfn != ' ')
        {
            /* if a non-space character was encoded then move end pointer just behind it */
            utf8_end = utf8_sfn;
        }
        disk_sfn++;
    }

    *utf8_end = '.';  // place a dot behind the filename, note that the end pointer stays at the same place
    utf8_sfn = utf8_end + 1;  // start encoding the extension just behind the dot

    for (i = 0; i < 3; i++)
    {
        utf8_encode(*disk_sfn, &utf8_sfn, NULL);
        if (*disk_sfn != ' ')
        {
            utf8_end = utf8_sfn;
        }
        disk_sfn++;
    }

    *utf8_end = '\0';  // store null terminator, this eventually overwrites the dot if the extension contains just spaces
}
