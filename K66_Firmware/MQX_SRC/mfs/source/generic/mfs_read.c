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


/*!
 * \brief Read a specific number of bytes from an open file.
 *
 * \param[in] drive_ptr The drive on which to operate.
 * \param[in] handle File handle upon which the action is to be taken.
 * \param[in] num_bytes Number of bytes to be read.
 * \param[in,out] buffer_address Bytes are read into this buffer.
 * \param[in,out] error_ptr Error code is written to this address.
 *
 * \return uint32_t Number of bytes read.
 */
uint32_t MFS_Read(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    MFS_HANDLE_PTR handle,
    uint32_t num_bytes,
    char *buffer_address,
    _mfs_error_ptr error_ptr)
{
    _mfs_error error_code;
    _mfs_error error_temp;
    bool eof_reached;

    uint32_t sector_number;
    uint32_t sector_offset;

    uint32_t whole_sectors;
    uint32_t cont_sectors;
    uint32_t proc_sectors;

    uint32_t location;
    uint32_t file_size;
    uint32_t bytes_left_in_file;

    uint32_t bytes_read;
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

    error_code = MFS_lock_and_enter(drive_ptr, 0);
    if (error_code != MFS_NO_ERROR)
    {
        MFS_set_error_and_return(error_ptr, error_code, 0);
    }

    if ((handle->FSFLAGS & MFS_O_ACCMODE) == MFS_O_WRONLY)
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

    bytes_read = 0;
    eof_reached = false;
    location = handle->LOCATION;

    /*
    ** Can't read past file size
    */
    file_size = handle->DIR_ENTRY->FILE_SIZE;
    if (location > file_size)
    {
        location = file_size;
    }
    bytes_left_in_file = file_size - location;
    if (num_bytes > bytes_left_in_file)
    {
        eof_reached = true;
        num_bytes = bytes_left_in_file;
    }


    if (bytes_left_in_file)
    {
        /*
        ** Read the number of bytes from the current file
        ** position to the end of the current cluster
        */

        sector_offset = OFFSET_WITHIN_SECTOR(drive_ptr, location);

        /* Read partial sector if sector_offet is non-zero */
        if (sector_offset != 0)
        {
            error_code = MFS_chain_locate(drive_ptr, &handle->CHAIN, location, 0, &sector_number, &cont_sectors);
            if (error_code == MFS_NO_ERROR)
            {
                error_code = MFS_sector_map(drive_ptr, sector_number, (void **)&data_sector, MFS_MAP_MODE_READONLY, handle->DIR_ENTRY->HEAD_CLUSTER);
                if (error_code == MFS_NO_ERROR)
                {
                    /* The requested lenght of data may span the sector to it's end  */
                    chunk_size = min(num_bytes, drive_ptr->SECTOR_SIZE - sector_offset);
                    _mem_copy(data_sector + sector_offset, buffer_address, chunk_size);
                    bytes_read += chunk_size;
                    location += chunk_size;
                    error_code = MFS_sector_unmap(drive_ptr, sector_number, 0);
                }
            }
        }

        /* If application buffer is properly aligned read whole sectors directly to the application buffer (zero copy) */
        if (((((uint32_t)buffer_address + bytes_read) & drive_ptr->ALIGNMENT_MASK) == 0) && (error_code == MFS_NO_ERROR))
        {
            whole_sectors = (num_bytes - bytes_read) >> drive_ptr->SECTOR_POWER;
            while (whole_sectors > 0)
            {
                error_code = MFS_chain_locate(drive_ptr, &handle->CHAIN, location, 0, &sector_number, &cont_sectors);
                if (error_code != MFS_NO_ERROR)
                    break;

                cont_sectors = (cont_sectors > whole_sectors) ? whole_sectors : cont_sectors;

                /* The zero copy operation bypasses the sector cache, this is to keep the cache coherent */
                error_code = MFS_sector_cache_flush(drive_ptr, sector_number, cont_sectors);
                if (error_code != MFS_NO_ERROR)
                    break;

                error_code = MFS_Read_device_sectors(drive_ptr, sector_number, cont_sectors, MFSCFG_MAX_READ_RETRIES, buffer_address + bytes_read, &proc_sectors);

                chunk_size = proc_sectors << drive_ptr->SECTOR_POWER;
                bytes_read += chunk_size;
                location += chunk_size;
                whole_sectors -= proc_sectors;

                if (error_code != MFS_NO_ERROR)
                    break;
            }
        }

        /* Read the rest of the data. This handles reading to misaligned application buffer and the tail (last incomplete sector) */
        while ((bytes_read < num_bytes) && (error_code == MFS_NO_ERROR))
        {
            error_code = MFS_chain_locate(drive_ptr, &handle->CHAIN, location, 0, &sector_number, &cont_sectors);
            if (error_code == MFS_NO_ERROR)
            {
                error_code = MFS_sector_map(drive_ptr, sector_number, (void **)&data_sector, MFS_MAP_MODE_READONLY, handle->DIR_ENTRY->HEAD_CLUSTER);
                if (error_code == MQX_OK)
                {
                    chunk_size = min(num_bytes - bytes_read, drive_ptr->SECTOR_SIZE);
                    _mem_copy(data_sector, buffer_address + bytes_read, chunk_size);
                    bytes_read += chunk_size;
                    location += chunk_size;
                    error_code = MFS_sector_unmap(drive_ptr, sector_number, 0);
                }
            }
        }

        handle->LOCATION = location;
    }

    error_temp = MFS_leave_and_unlock(drive_ptr, 0);
    if (error_code == MFS_NO_ERROR)
    {
        error_code = error_temp;
    }

    if ((error_code == MFS_NO_ERROR) && eof_reached)
    {
        error_code = MFS_EOF;
    }

    MFS_set_error_and_return(error_ptr, error_code, bytes_read);
}
