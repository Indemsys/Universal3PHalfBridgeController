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
*   This file contains the file specific interface functions
*   of the MFS.
*
*
*END************************************************************************/

#include "mfs.h"
#include "mfs_prv.h"


#if !MFSCFG_READ_ONLY


/*!
 * \brief Write a specific number of bytes to an open file.
 *
 * \param[in] drive_ptr The drive on which to operate.
 * \param[in] handle File handle upon which the action is to be taken.
 * \param[in] num_bytes Number of bytes to be written.
 * \param[in,out] buffer_address Bytes are written from this buffer.
 * \param[in,out] error_ptr Error code is written to this address.
 *
 * \return uint32_t Number of bytes writen
 */
uint32_t MFS_Write(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    MFS_HANDLE_PTR handle,
    uint32_t num_bytes,
    char *buffer_address,
    _mfs_error_ptr error_ptr)
{
    _mfs_error error_code;
    _mfs_error error_temp;

    uint32_t sector_number;
    uint32_t sector_offset;

    uint32_t whole_sectors;
    uint32_t cont_sectors;
    uint32_t proc_sectors;

    uint32_t location;
    uint32_t file_size;
    uint32_t extend_to;

    uint32_t num_zeros;
    uint32_t zeros_written;
    uint32_t bytes_written;
    uint32_t chunk_size;

    uint8_t *data_sector;

    if (num_bytes == 0)
    {
        MFS_set_error_and_return(error_ptr, MFS_NO_ERROR, 0);
    }

    if (buffer_address == NULL)
    {
        MFS_set_error_and_return(error_ptr, MFS_INVALID_PARAMETER, 0);
    }

    error_code = MFS_lock_and_enter(drive_ptr, MFS_ENTER_READWRITE);
    if (error_code != MFS_NO_ERROR)
    {
        MFS_set_error_and_return(error_ptr, error_code, 0);
    }

    if ((handle->FSFLAGS & MFS_O_ACCMODE) == MFS_O_RDONLY)
    {
        MFS_leave_and_unlock(drive_ptr, 0);
        MFS_set_error_and_return(error_ptr, MFS_ACCESS_DENIED, 0);
    }

    /* Check whether the handle is in sync with the shared directory entry */
    if (handle->VSTAMP != handle->DIR_ENTRY->VSTAMP)
    {
        error_code = MFS_chain_init(drive_ptr, &handle->CHAIN, handle->DIR_ENTRY->HEAD_CLUSTER);
        if (error_code != MFS_NO_ERROR)
        {
            MFS_leave_and_unlock(drive_ptr, 0);
            MFS_set_error_and_return(error_ptr, error_code, 0);
        }
        handle->VSTAMP = handle->DIR_ENTRY->VSTAMP;
    }

    /* If there is no chain associated with the file it is necessary to create one */
    if (handle->CHAIN.HEAD_CLUSTER == 0)
    {
        error_code = MFS_chain_create(drive_ptr, &handle->CHAIN, NULL);
        if (error_code == MFS_NO_ERROR)
        {
            handle->DIR_ENTRY->HEAD_CLUSTER = handle->CHAIN.HEAD_CLUSTER;
            handle->DIR_ENTRY->DIRTY = 1;
            handle->DIR_ENTRY->VSTAMP++;
            handle->VSTAMP = handle->DIR_ENTRY->VSTAMP;
            handle->TOUCHED = 1;
        }
        else
        {
            MFS_leave_and_unlock(drive_ptr, 0);
            MFS_set_error_and_return(error_ptr, error_code, 0);
        }
    }

    file_size = handle->DIR_ENTRY->FILE_SIZE;

    /* Set location (local variable) to the actual location to be written */
    if (handle->FSFLAGS & MFS_O_APPEND)
    {
        /* set location to the end of file just before writing in append mode */
        location = handle->LOCATION = file_size;
    }
    else if (handle->LOCATION > file_size)
    {
        /* location shall never point beyond end of file, event if handle->LOCATION does */
        location = file_size;
    }
    else
    {
        location = handle->LOCATION;
    }

    /* Check the file size limit and adjust amount of data to be written accordingly */
    if (num_bytes > MFS_MAX_FILESIZE - handle->LOCATION)
    {
        num_bytes = MFS_MAX_FILESIZE - handle->LOCATION;
    }

    /* Calculate possible gap to fill in by zeros if writing behind the end of file */
    num_zeros = handle->LOCATION - location;

    /* Calculate the boundary up to which the chain may be extended */
    extend_to = handle->LOCATION + num_bytes;

    zeros_written = 0;
    bytes_written = 0;

    /* Write zeros to fill in gap if LOCATION points behind the end of file */
    while (zeros_written < num_zeros)
    {
        sector_offset = OFFSET_WITHIN_SECTOR(drive_ptr, location);
        chunk_size = min(num_zeros - zeros_written, drive_ptr->SECTOR_SIZE - sector_offset);

        error_code = MFS_chain_locate(drive_ptr, &handle->CHAIN, location, extend_to, &sector_number, &cont_sectors);
        if (error_code != MFS_NO_ERROR)
            break;

        /* If offset is non-zero, then reading the data is required, otherwise the whole sector is going to be overwritten */
        error_code = MFS_sector_map(drive_ptr, sector_number, (void **)&data_sector, sector_offset ? MFS_MAP_MODE_MODIFY : MFS_MAP_MODE_OVERWRITE, handle->DIR_ENTRY->HEAD_CLUSTER);
        if (error_code != MFS_NO_ERROR)
            break;

        /* Zero the buffer  */
        _mem_zero(data_sector + sector_offset, chunk_size);

        error_code = MFS_sector_unmap(drive_ptr, sector_number, 1);
        if (error_code != MFS_NO_ERROR)
            break;

        zeros_written += chunk_size;
        location += chunk_size;
    }

    sector_offset = OFFSET_WITHIN_SECTOR(drive_ptr, location);

    /* Write partial sector if sector_offset is non-zero */
    if (num_bytes && (sector_offset != 0) && (error_code == MFS_NO_ERROR))
    {
        error_code = MFS_chain_locate(drive_ptr, &handle->CHAIN, location, extend_to, &sector_number, &cont_sectors);
        if (error_code == MFS_NO_ERROR)
        {
            /* Offset is always non-zero here, read-modify-write is required */
            error_code = MFS_sector_map(drive_ptr, sector_number, (void **)&data_sector, MFS_MAP_MODE_MODIFY, handle->DIR_ENTRY->HEAD_CLUSTER);
            if (error_code == MFS_NO_ERROR)
            {
                /* The requested length of data may span the sector to it's end  */
                chunk_size = min(num_bytes, drive_ptr->SECTOR_SIZE - sector_offset);
                _mem_copy(buffer_address, data_sector + sector_offset, chunk_size);

                error_code = MFS_sector_unmap(drive_ptr, sector_number, 1);
                if (error_code == MFS_NO_ERROR)
                {
                    bytes_written += chunk_size;
                    location += chunk_size;
                }
            }
        }
    }

    /* If application buffer is properly aligned write whole sectors directly from the application buffer (zero copy) */
    if (((((uint32_t)buffer_address + bytes_written) & drive_ptr->ALIGNMENT_MASK) == 0) && (error_code == MFS_NO_ERROR))
    {
        whole_sectors = (num_bytes - bytes_written) >> drive_ptr->SECTOR_POWER;
        while (whole_sectors > 0)
        {
            error_code = MFS_chain_locate(drive_ptr, &handle->CHAIN, location, extend_to, &sector_number, &cont_sectors);
            if (error_code != MFS_NO_ERROR)
                break;

            cont_sectors = (cont_sectors > whole_sectors) ? whole_sectors : cont_sectors;
            error_code = MFS_Write_device_sectors(drive_ptr, sector_number, cont_sectors, MFSCFG_MAX_WRITE_RETRIES, buffer_address + bytes_written, &proc_sectors);

            chunk_size = proc_sectors << drive_ptr->SECTOR_POWER;
            bytes_written += chunk_size;
            location += chunk_size;
            whole_sectors -= proc_sectors;

            if (error_code != MFS_NO_ERROR)
                break;

            /* The zero copy operation bypasses the sector cache, this is to keep the cache coherent */
            error_code = MFS_sector_cache_invalidate(drive_ptr, sector_number, cont_sectors);
            if (error_code != MFS_NO_ERROR)
                break;
        }
    }

    /* write the rest of the data. This handles writing from misaligned application buffer and the tail (last incomplete sector) */
    while ((bytes_written < num_bytes) && (error_code == MFS_NO_ERROR))
    {
        error_code = MFS_chain_locate(drive_ptr, &handle->CHAIN, location, extend_to, &sector_number, &cont_sectors);
        if (error_code == MFS_NO_ERROR)
        {
            uint32_t map_flags;
            chunk_size = min(num_bytes - bytes_written, drive_ptr->SECTOR_SIZE);
            if ((location + chunk_size) >= file_size || chunk_size >= drive_ptr->SECTOR_SIZE)
            {
                map_flags = MFS_MAP_MODE_OVERWRITE;
            }
            else
            {
                map_flags = MFS_MAP_MODE_MODIFY;
            }
            error_code = MFS_sector_map(drive_ptr, sector_number, (void **)&data_sector, map_flags, handle->DIR_ENTRY->HEAD_CLUSTER);
            if (error_code == MQX_OK)
            {
                _mem_copy(buffer_address + bytes_written, data_sector, chunk_size);
                bytes_written += chunk_size;
                location += chunk_size;
                error_code = MFS_sector_unmap(drive_ptr, sector_number, 1);
            }
        }
    }

    if (zeros_written || bytes_written)
    {
        handle->TOUCHED = 1;
    }

    if (location > file_size)
    {
        handle->DIR_ENTRY->FILE_SIZE = location;
        handle->DIR_ENTRY->DIRTY = 1;
        if ((drive_ptr->WRITE_CACHE_POLICY == MFS_WRITE_THROUGH_CACHE) && (error_code == MFS_NO_ERROR))
        {
            error_code = MFS_update_entry(drive_ptr, handle);
        }
    }

    /* LOCATION moves forward only by actual data written, the zero fill does not count */
    handle->LOCATION += bytes_written;

    error_temp = MFS_leave_and_unlock(drive_ptr, 0);
    if (error_code == MFS_NO_ERROR)
    {
        error_code = error_temp;
    }

    MFS_set_error_and_return(error_ptr, error_code, bytes_written);
}

#endif
