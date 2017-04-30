/*HEADER**********************************************************************
*
* Copyright 2015 Freescale Semiconductor, Inc.
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
*   This file contains routines for handling directories using FAT chain abstraction
*
*END************************************************************************/

#include "mfs.h"
#include "mfs_prv.h"


/*!
 * \brief Scan single directory for entry with given name
 *
 * \param[in] drive_ptr Drive context.
 * \param[in] dir_chain FAT chain of directory to search.
 * \param[in] name Specific name to search for, terminated by \0 or path delimiter
 * \param[out] entry_copy  The pointer to store the entry copy to.
 * \param[out] entry_sector The sector containing the main (SFN) entry.
 * \param[out] entry_index Index of the main (SFN) entry within the sector.
 * \param[out] entry_loc Location of the entry slot in the chain (points to LFN entry, if any).
 *
 * \return _mfs_error
 */
_mfs_error MFS_scan_dir_chain(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    FAT_CHAIN *dir_chain,
    char *name,
    DIR_ENTRY_DISK *entry_copy,
    uint32_t *entry_sector,
    uint32_t *entry_index,
    uint32_t *entry_loc)
{
    _mfs_error error_code;

    uint32_t location;
    uint32_t sector_num;
    uint32_t sector_count;
    uint32_t index;

    DIR_ENTRY_DISK *dir_entry_ptr;

    bool finished;
    bool found;

    int alias_index;
    int alias_max;
    uint32_t alias_used;

    uint8_t name_8dot3[11];
    int name_len;
    char *name_end;
    int name_len_utf16;
    bool name_fits_sfn;
    bool name_needs_lfn;

    /* LFN validation state variables */
    uint32_t lfn_loc;  // location of the LFN being scanned in the chain
    int lfn_ord;  // ordinal of last entry of the LFN being scanned
    int lfn_chksum;  // alias checksum of the LFN being scanned, -1 when no LFN is being processed

    /* LFN matching state variables */
    int lfn_match_state;
    char *lfn_match_pos;
    uint32_t lfn_surrogate_buf;

    /* free slot scanning - used to get location for non-existing entry in one go during */
    uint32_t free_slots_loc;
    int free_slots_found;
    int free_slots_required;


    /* parameter sanity check */
    if ((name == NULL) || (*name == '\0'))
    {
        return MFS_INVALID_PARAMETER;
    }

    /* calculate name length in UTF-16 words; this also checks UTF-8 validity of the string */
    name_len = MFS_path_component_len(name, NULL, &name_len_utf16);
    if (name_len <= 0)
    {
        return MFS_INVALID_PARAMETER;
    }
    name_end = name + name_len - 1;

    error_code = MFS_lfn_to_sfn_disk(name, name_len, name_8dot3, &name_fits_sfn, &name_needs_lfn);
    if (error_code != MFS_NO_ERROR)
    {
        return error_code;
    }

    free_slots_required = name_needs_lfn ? ((name_len_utf16 + 12) / 13 + 1) : 1;
    free_slots_loc = 0;
    free_slots_found = 0;

    alias_used = 0;
    alias_max = 0;

    location = 0;
    lfn_chksum = -1;  // no valid LFN found so far

    sector_count = 0;

    found = false;
    finished = false;

    while (!finished)
    {
        if (sector_count > 0)
        {
            /* there is an unprocessed sector located during previous iteration, use it */
            sector_num++;
        }
        else
        {
            error_code = MFS_chain_locate(drive_ptr, dir_chain, location, 0, &sector_num, &sector_count);
            if (error_code != MFS_NO_ERROR)
            {
                break;
            }
        }
        sector_count--; //consume one sector

        error_code = MFS_sector_map(drive_ptr, sector_num, (void **)&dir_entry_ptr, MFS_MAP_MODE_READONLY, 0);
        if (error_code != MFS_NO_ERROR)
        {
            break;
        }

        /* process all entry slots in the sector, location is always aligned to sector, thus index starts with 0 */
        for (index = 0; index < drive_ptr->ENTRIES_PER_SECTOR; index++)
        {
            if (dir_entry_ptr->NAME[0] == 0)
            {
                /* end of directory, exit the loop */
                finished = true;
                break;
            }
            if (dir_entry_ptr->NAME[0] == MFS_DEL_FILE)
            {
                /* deleted entry*/
                lfn_chksum = -1; /* reset LFN processing */
                if (location == free_slots_loc + (free_slots_found * sizeof(DIR_ENTRY_DISK)))
                {
                    /* the slot being processed is adjacent with previously found free slots */
                    free_slots_found++;
                }
                else if (free_slots_found < free_slots_required)
                {
                    /* the number of adjacent free slots is not sufficient, restart scanning for free slots */
                    free_slots_loc = location;
                    free_slots_found = 1;
                }
            }
            else if ((dir_entry_ptr->ATTRIBUTE[0] & MFS_ATTR_LFN_MASK) == MFS_ATTR_LFN)
            {
                /* LFN entry processing */
                MFS_LNAME_ENTRY *lfn_entry_ptr = (MFS_LNAME_ENTRY *)dir_entry_ptr;
                if (lfn_entry_ptr->ID & MFS_LFN_END)
                {
                    /* first (tail) LFN entry, init LFN validation state machine */
                    lfn_loc = location;
                    lfn_ord = lfn_entry_ptr->ID & MFS_LFN_ORD;
                    lfn_chksum = lfn_entry_ptr->ALIAS_CHECKSUM;
                    /* init LFN matching state machine */
                    lfn_match_state = 0;
                    if (name_len_utf16 == MFS_lfn_utf16_len(lfn_entry_ptr))
                    {
                        lfn_match_pos = name_end;
                        lfn_surrogate_buf = 0;
                        lfn_match_state = MFS_lfn_entry_match_r(name, &lfn_match_pos, lfn_entry_ptr, &lfn_surrogate_buf);
                    }
                }
                else if ((lfn_chksum != lfn_entry_ptr->ALIAS_CHECKSUM) || (--lfn_ord != lfn_entry_ptr->ID))
                {
                    /* the entry does not belong to the same LFN as previous entry */
                    lfn_chksum = -1;
                }
                else if (lfn_match_state == 1)
                {
                    /* the previous LFN entries match so far, check this one and update the matching state */
                    lfn_match_state = MFS_lfn_entry_match_r(name, &lfn_match_pos, lfn_entry_ptr, &lfn_surrogate_buf);
                }
            }
            else
            {
                if (0 == (dir_entry_ptr->ATTRIBUTE[0] & MFS_ATTR_VOLUME_NAME)) {
                /* main (SFN) entry, check if there was valid and corresponding LFN found */
                if ((lfn_chksum >= 0) && (lfn_ord == 1) && (lfn_chksum == MFS_lfn_checksum(dir_entry_ptr->NAME)))
                {
                    /* check if there was a match in LFN */
                    if (lfn_match_state && (lfn_match_pos < name) && (lfn_surrogate_buf == 0))
                    {
                        /* LFN matches */
                        found = true;
                        break;
                    }
                }

                if (name_fits_sfn)
                {
                    if (!memcmp(name_8dot3, dir_entry_ptr->NAME, 11))
                    {
                        /* SFN matches */
                        found = true;
                        break;
                    }
                }
                else
                {
                    alias_index = MFS_lfn_alias_index(name_8dot3, dir_entry_ptr->NAME);
                    if (alias_index > 0 && alias_index <= 32)
                    {
                        alias_used |= (1 << (alias_index - 1));
                    }
                    if (alias_index > alias_max)
                    {
                        alias_max = alias_index;
                    }
                }

                }
                lfn_chksum = -1; /* ensure that the LFN (if any) is no longer considered valid for subsequent SFNs */
            }

            dir_entry_ptr++;
            location += sizeof(DIR_ENTRY_DISK);
        }

        if (found)
        {
            /* process the entry before unmapping the directory sector */
            if (entry_copy)
            {
                _mem_copy(dir_entry_ptr, entry_copy, sizeof(DIR_ENTRY_DISK));
            }
            if (entry_loc)
            {
                /* location of the LFN (if any), otherwise location of the main (SFN entry) */
                *entry_loc = (lfn_chksum >= 0) ? lfn_loc : location;
            }
            if (entry_sector)
            {
                *entry_sector = sector_num;
            }
            if (entry_index)
            {
                *entry_index = index;
            }
            finished = true;
        }

        error_code = MFS_sector_unmap(drive_ptr, sector_num, 0);
        if (error_code != MFS_NO_ERROR)
        {
            break;
        }
    }

    /* check error states (file not found, etc.) */
    if (!found && (error_code == MFS_NO_ERROR || error_code == MFS_EOF))
    {
        if (entry_loc)
        {
            /* entry not found, set entry_loc to place where it may be created, if desired */
            *entry_loc = (free_slots_found >= free_slots_required) ? free_slots_loc : location;
        }
        error_code = MFS_FILE_NOT_FOUND;
    }

    return error_code;
}


/*!
 * \brief Retrieve directory entries one by one
 *
 * \param[in] drive_ptr Drive context.
 * \param[in] dir_chain FAT chain of directory to search.
 * \param[in,out] dir_loc_ptr Keeps current location in the chain.
 * \param[in] lfn_buf Buffer for store LFN.
 * \param[in] lfn_buf_len  Length of string pointed to by name parameter.
 * \param[out] entry_copy The pointer to store the entry copy to.
 * \param[out] entry_sector The sector containing the main (SFN) entry.
 * \param[out] entry_index Index of the main (SFN) entry within the sector.
 * \param[out] entry_loc Location of the entry slot in the chain (points to LFN entry, if any).
 *
 * \return _mfs_error
 */
_mfs_error MFS_read_dir_chain(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    FAT_CHAIN *dir_chain,
    uint32_t *dir_loc_ptr,
    char *lfn_buf,
    int lfn_buf_len,
    DIR_ENTRY_DISK *entry_copy,
    uint32_t *entry_sector,
    uint32_t *entry_index,
    uint32_t *entry_loc)
{
    _mfs_error error_code;

    uint32_t sector_num;
    uint32_t sector_count;
    uint32_t index;
    uint32_t first_index;

    DIR_ENTRY_DISK *dir_entry_ptr;

    bool finished;
    bool found;
    uint32_t location;

    /* LFN extraction state variables */
    uint32_t lfn_loc;  // location of the LFN being scanned in the chain
    int lfn_ord;  // ordinal of last entry of the LFN being scanned
    int lfn_chksum;  // alias checksum of the LFN being scanned, -1 when no LFN is being processed
    uint32_t lfn_surrogate_buf;  // buffer to hold unprocessed half of a surrogate pair
    char *lfn_pos;  // current position on the LFN buffer
    char *lfn_boundary;  // LFN buffer boundary

    location = *dir_loc_ptr;
    first_index = (location % drive_ptr->SECTOR_SIZE) / sizeof(DIR_ENTRY_DISK);

    lfn_chksum = -1;  // no valid LFN found so far
    lfn_boundary = (lfn_buf && lfn_buf_len) ? (lfn_buf + lfn_buf_len) : NULL;  // initialize end of LFN buffer
    if (lfn_buf)
    {
        *lfn_buf = '\0';  // pre-fill first position of the buffer with null terminator
    }

    sector_count = 0;
    finished = false;
    found = false;

    while (!finished)
    {
        if (sector_count > 0)
        {
            /* there is an unprocessed sector located during previous iteration, use it */
            sector_num++;
        }
        else
        {
            error_code = MFS_chain_locate(drive_ptr, dir_chain, location, 0, &sector_num, &sector_count);
            if (error_code != MFS_NO_ERROR)
            {
                break;
            }
        }
        sector_count--; //consume one sector

        error_code = MFS_sector_map(drive_ptr, sector_num, (void **)&dir_entry_ptr, MFS_MAP_MODE_READONLY, 0);
        if (error_code != MFS_NO_ERROR)
        {
            break;
        }
        dir_entry_ptr += first_index;

        /* process all entry slots in the sector */
        for (index = first_index; index < drive_ptr->ENTRIES_PER_SECTOR; index++)
        {
            if (dir_entry_ptr->NAME[0] == 0)
            {
                /* end of directory, exit the loop */
                finished = true;
                break;
            }
            if (dir_entry_ptr->NAME[0] == MFS_DEL_FILE)
            {
                /* deleted entry*/
                lfn_chksum = -1; /* reset LFN processing */
            }
            else if ((dir_entry_ptr->ATTRIBUTE[0] & MFS_ATTR_LFN_MASK) == MFS_ATTR_LFN)
            {
                /* LFN entry processing */
                MFS_LNAME_ENTRY *lfn_entry_ptr = (MFS_LNAME_ENTRY *)dir_entry_ptr;
                if (lfn_entry_ptr->ID & MFS_LFN_END)
                {
                    /* first (tail) LFN entry, init LFN validation state machine */
                    lfn_loc = location;
                    lfn_ord = lfn_entry_ptr->ID & MFS_LFN_ORD;
                    lfn_chksum = lfn_entry_ptr->ALIAS_CHECKSUM;
                    lfn_surrogate_buf = 0;
                    lfn_pos = lfn_buf + (lfn_buf != NULL);  // set position to second position of the buffer
                }
                else if ((lfn_chksum != lfn_entry_ptr->ALIAS_CHECKSUM) || (--lfn_ord != lfn_entry_ptr->ID))
                {
                    /* the entry does not belong to the same LFN as previous entry */
                    lfn_chksum = -1;
                }
                if (lfn_buf && lfn_chksum >= 0)
                {
                    /* extract part of LFN */
                    if (MFS_lfn_entry_extract_r(lfn_entry_ptr, &lfn_pos, lfn_boundary, &lfn_surrogate_buf) < 0)
                    {
                        lfn_chksum = -1;
                    }
                }
            }
            else
            {
                /* main (SFN) entry, check if there was valid and corresponding LFN found */
                if ((lfn_chksum >= 0) && (lfn_ord == 1) && (lfn_chksum == MFS_lfn_checksum(dir_entry_ptr->NAME)))
                {
                    /* reverse LFN buffer */
                    if (lfn_buf)
                    {
                        mem_reverse(lfn_buf, lfn_pos - 1);
                    }
                }
                else
                {
                    /* invalid or no LFN found */
                    lfn_chksum = -1;
                    if (lfn_boundary - lfn_buf >= 24)
                    {
                        /* extract SFN to LFN buffer */
                        MFS_sfn_from_disk(dir_entry_ptr->NAME, lfn_buf);
                    }
                }
                if (entry_copy)
                {
                    _mem_copy(dir_entry_ptr, entry_copy, sizeof(DIR_ENTRY_DISK));
                }
                if (entry_loc)
                {
                    /* location of the LFN (if any), otherwise location of the main (SFN entry) */
                    *entry_loc = (lfn_chksum >= 0) ? lfn_loc : location;
                }
                if (entry_sector)
                {
                    *entry_sector = sector_num;
                }
                if (entry_index)
                {
                    *entry_index = index;
                }

                finished = true;
                found = true;
                location += sizeof(DIR_ENTRY_DISK);  // advance location to next entry
                break;
            }

            dir_entry_ptr++;
            location += sizeof(DIR_ENTRY_DISK);
        }
        first_index = 0;

        error_code = MFS_sector_unmap(drive_ptr, sector_num, 0);
        if (error_code != MFS_NO_ERROR)
        {
            break;
        }
    }

    if (error_code == MFS_NO_ERROR && !found)
    {
        error_code = MFS_EOF;
    }

    *dir_loc_ptr = location;
    return error_code;
}


/*!
 * \brief Resolves path up to the last component and returns the corresponding directory chain.
 *
 * \param[in] drive_ptr Drive context.
 * \param[in] path Path to search for.
 * \param[out] dir_chain Pre-allocated memory to place the copy of the directory chain.
 * \param[out] dir_chain_ptr Optional - this may point to a (cached) directory chain allocated elsewhere - future expansion.
 * \param[out] entry_name Optional - pointer to the last component of the path, if any.
 *
 * \return _mfs_error
 */
_mfs_error MFS_get_dir_chain(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    char *path,
    FAT_CHAIN *dir_chain,
    FAT_CHAIN **dir_chain_ptr,
    char **entry_name)
{
    _mfs_error error_code;
    DIR_ENTRY_DISK dir_entry;
    FAT_CHAIN *active_chain;
    char *parse_pos;
    char *path_component;
    int path_comp_len;
    uint32_t dir_head_cluster;

    if (dir_chain == NULL)
    {
        return MFS_INVALID_PARAMETER;
    }

    error_code = MFS_NO_ERROR;

    parse_pos = MFS_Parse_Out_Device_Name(path);
    path_component = parse_pos;

    if (IS_DELIMITER(*parse_pos))
    {
        active_chain = &drive_ptr->ROOT_CHAIN;
    }
    else
    {
        active_chain = drive_ptr->CUR_DIR_CHAIN_PTR;
    }

    while (error_code == MFS_NO_ERROR)
    {
        /* get next path component */
        path_comp_len = MFS_next_path_component(&parse_pos, &path_component);

        /* check if the parsing is at the end of the string (the path component just found is the last one) */
        if (*parse_pos == '\0')
        {
            /* exit the loop if either: caller provided a pointer to obtain the entry name (last path component which may not represent a directory) or there is no path component to be processed at  all*/
            if (entry_name || path_comp_len == 0)
            {
                break;
            }
        }

        /* skip "this directory" (single dot) designator */
        if (path_comp_len == 1 && *path_component == '.')
        {
            continue;
        }

        /* lookup path component in the active directory chain */
        error_code = MFS_scan_dir_chain(drive_ptr, active_chain, path_component, &dir_entry, NULL, NULL, NULL);
        if (error_code != MFS_NO_ERROR)
        {
            if (error_code == MFS_FILE_NOT_FOUND)
            {
                /* entry for the path component was not found - translate this to "path not found error" */
                error_code = MFS_PATH_NOT_FOUND;
            }
            break;
        }

        /* check if the entry belongs to a directory */
        if ((dir_entry.ATTRIBUTE[0] & MFS_ATTR_DIR_NAME) == 0)
        {
            error_code = MFS_PATH_NOT_FOUND;
            break;
        }

        /* get the chain of the referenced directory and jump to it */
        dir_head_cluster = clustoh(drive_ptr, dir_entry.HFIRST_CLUSTER, dir_entry.LFIRST_CLUSTER);
        if (dir_head_cluster == 0)
        {
            active_chain = &drive_ptr->ROOT_CHAIN;
        }
        else if (dir_head_cluster == drive_ptr->CUR_DIR_CHAIN_PTR->HEAD_CLUSTER)
        {
            active_chain = drive_ptr->CUR_DIR_CHAIN_PTR;
        }
        else
        {
            error_code = MFS_chain_init(drive_ptr, dir_chain, dir_head_cluster);
            active_chain = dir_chain;
        }
    }

    if (entry_name)
    {
        *entry_name = path_component;
    }

    if (dir_chain != active_chain)
    {
        /* return copy of the directory chain */
        *dir_chain = *active_chain;
    }

    if (dir_chain_ptr)
    {
        /* return reference to the directory chain as well */
        *dir_chain_ptr = active_chain;
    }

    return error_code;
}


/*!
 * \brief Looks up directory entry for given path and returns is copy and location on the disk.
 *
 * \param[in] drive_ptr Drive context.
 * \param[in] pathname Path to the entry.
 * \param[out] entry_copy The pointer to store the entry copy to.
 * \param[out] entry_sector The sector containing the main (SFN) entry.
 * \param[out] entry_index  Index of the main (SFN) entry within the sector.
 *
 * \return _mfs_error
 */
_mfs_error MFS_locate_dir_entry(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    char *pathname,
    DIR_ENTRY_DISK *entry_copy,
    uint32_t *entry_sector,
    uint32_t *entry_index)
{
    _mfs_error error_code;
    FAT_CHAIN dir_chain;
    char *entry_name;

    /* Find the directory in which the file shall be located */
    error_code = MFS_get_dir_chain(drive_ptr, pathname, &dir_chain, NULL, &entry_name);
    if (error_code == MFS_NO_ERROR)
    {
        /* Lookup entry  with the requested name in the directory */
        error_code = MFS_scan_dir_chain(drive_ptr, &dir_chain, entry_name, entry_copy, entry_sector, entry_index, NULL);
    }

    return error_code;
}


/*!
 * \brief
 *
 * Marks directory entry including LFN slots as deleted. Location has to point to the first (tail)
 * LFN associated with the entry to be deleted.
 *
 * \param[in] drive_ptr Drive context.
 * \param[in] dir_chain FAT chain of the directory to operate on.
 * \param[in] location Location of the entry/entries which shall be marked as free.
 *
 * \return _mfs_error
 */
_mfs_error MFS_free_dir_entry(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    FAT_CHAIN *dir_chain,
    uint32_t location)
{
    _mfs_error error_code;

    uint32_t sector_num;
    uint32_t sector_count;
    bool sector_dirty;
    uint32_t index;
    uint32_t first_index;

    DIR_ENTRY_DISK *dir_entry_ptr;

    bool finished;

    sector_count = 0;
    finished = false;

    first_index = (location % drive_ptr->SECTOR_SIZE) / sizeof(DIR_ENTRY_DISK);

    while (!finished)
    {
        if (sector_count > 0)
        {
            /* there is an unprocessed sector located during previous iteration, use it */
            sector_num++;
        }
        else
        {
            error_code = MFS_chain_locate(drive_ptr, dir_chain, location, 0, &sector_num, &sector_count);
            if (error_code != MFS_NO_ERROR)
            {
                break;
            }
        }
        sector_count--;

        error_code = MFS_sector_map(drive_ptr, sector_num, (void **)&dir_entry_ptr, MFS_MAP_MODE_MODIFY, 0);
        if (error_code != MFS_NO_ERROR)
        {
            break;
        }
        sector_dirty = false;
        dir_entry_ptr += first_index;

        /* process all entry slots in the sector */
        for (index = first_index; index < drive_ptr->ENTRIES_PER_SECTOR; index++)
        {
            if ((dir_entry_ptr->NAME[0] == 0) || (dir_entry_ptr->NAME[0] == MFS_DEL_FILE))
            {
                /* free slot or end of directory reached, exit the loop */
                finished = true;
                break;
            }
            else
            {
                dir_entry_ptr->NAME[0] = MFS_DEL_FILE;
                sector_dirty = true;
                if ((dir_entry_ptr->ATTRIBUTE[0] & MFS_ATTR_LFN_MASK) != MFS_ATTR_LFN)
                {
                    /* SFN entry reached, this is the last one to be deleted */
                    finished = true;
                    break;
                }
            }

            dir_entry_ptr++;
            location += sizeof(DIR_ENTRY_DISK);
        }
        first_index = 0;

        error_code = MFS_sector_unmap(drive_ptr, sector_num, sector_dirty);
        if (error_code != MFS_NO_ERROR)
        {
            break;
        }
    }

    if (error_code == MFS_EOF)
    {
        error_code = MFS_NO_ERROR;
    }

    return error_code;
}


/*!
 * \brief Checks whether the directory represented by given chain is empty.
 *
 * \param[in] drive_ptr Drive context.
 * \param[in] dir_chain FAT chain of directory to search.
 *
 * \return _mfs_error
 */
_mfs_error MFS_check_dir_empty(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    FAT_CHAIN *dir_chain)
{
    _mfs_error error_code;

    uint32_t location;
    uint32_t sector_num;
    uint32_t sector_count;
    uint32_t index;

    DIR_ENTRY_DISK *dir_entry_ptr;

    bool finished;
    bool found;

    location = 0;
    sector_count = 0;

    found = false;
    finished = false;

    while (!finished)
    {
        if (sector_count > 0)
        {
            /* there is an unprocessed sector located during previous iteration, use it */
            sector_num++;
        }
        else
        {
            error_code = MFS_chain_locate(drive_ptr, dir_chain, location, 0, &sector_num, &sector_count);
            if (error_code != MFS_NO_ERROR)
            {
                break;
            }
        }
        sector_count--;

        error_code = MFS_sector_map(drive_ptr, sector_num, (void **)&dir_entry_ptr, MFS_MAP_MODE_READONLY, 0);
        if (error_code != MFS_NO_ERROR)
        {
            break;
        }

        /* process all entry slots in the sector, location is always aligned to sector, thus index starts with 0 */
        for (index = 0; index < drive_ptr->ENTRIES_PER_SECTOR; index++)
        {
            if (dir_entry_ptr->NAME[0] == 0)
            {
                /* end of directory, exit the loop */
                finished = true;
                break;
            }
            if ((dir_entry_ptr->NAME[0] != MFS_DEL_FILE) && ((dir_entry_ptr->ATTRIBUTE[0] & MFS_ATTR_LFN_MASK) != MFS_ATTR_LFN))
            {
                if (!(dir_entry_ptr->NAME[0] == '.' && (dir_entry_ptr->NAME[1] == ' ' || (dir_entry_ptr->NAME[1] == '.' && dir_entry_ptr->NAME[2] == ' '))))
                {
                    /* the entry which is neither "." or ".." */
                    found = true;
                    break;
                }
            }

            dir_entry_ptr++;
            location += sizeof(DIR_ENTRY_DISK);
        }

        if (found)
        {
            finished = true;
        }

        error_code = MFS_sector_unmap(drive_ptr, sector_num, 0);
        if (error_code != MFS_NO_ERROR)
        {
            break;
        }
    }

    /* If there was no fatal error so far set the error_code to indicate file existence */
    if (error_code == MFS_NO_ERROR || error_code == MFS_EOF)
    {
        error_code = found ? MFS_FILE_EXISTS : MFS_NO_ERROR;
    }

    return error_code;
}
