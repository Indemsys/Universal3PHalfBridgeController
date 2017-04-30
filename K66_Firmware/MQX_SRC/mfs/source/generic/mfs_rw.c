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
*   This file contains the functions that are used to read and write to the
*   device under MFS.
*
*
*END************************************************************************/

#include "mfs.h"
#include "mfs_prv.h"

#if !MFSCFG_READ_ONLY


/*!
 * \brief Reads or writes consecutive sectors.
 *
 * \param drive_ptr
 * \param[in] sector_number First sector to read/write from/to file system medium.
 * \param[in] sector_count Number of sectors to read/write from/to file system medium.
 * \param[in] max_retries Number of entries of the same low level operation if it fails.
 * \param[in,out] buffer_ptr Address of where data is to be stored/written.
 * \param[out] processed Number of sector successfully processed.
 *
 * \return _mfs_error
 */
_mfs_error MFS_Write_device_sectors(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    uint32_t sector_number,
    uint32_t sector_count,
    uint32_t max_retries,
    char *buffer_ptr,
    uint32_t *processed)
{
    uint32_t attempts;
    int32_t num, expect_num, seek_loc, shifter;
    char *data_ptr;
    _mfs_error error;

    if (MFS_is_read_only(drive_ptr))
    {
        return MFS_DISK_IS_WRITE_PROTECTED;
    }

    error = MFS_NO_ERROR;

    MFS_LOG(printf("MFS_Write_device_sectors %d %d\n", sector_number, sector_count));

    if (sector_number > drive_ptr->MEGA_SECTORS)
    {
        return MFS_SECTOR_NOT_FOUND;
    }

    if (drive_ptr->BLOCK_MODE)
    {
        shifter = 0;
        seek_loc = sector_number;
        expect_num = sector_count;
    }
    else
    {
        shifter = drive_ptr->SECTOR_POWER;
        seek_loc = sector_number << shifter;
        expect_num = sector_count << shifter;
    }

#if MQX_USE_IO_OLD
    fseek(drive_ptr->DEV_FILE_PTR, seek_loc, IO_SEEK_SET);
#else
    lseek(drive_ptr->DEV_FILE_PTR, seek_loc, SEEK_SET);
//TODO: check errno lseek
#endif

    data_ptr = buffer_ptr;
    attempts = 0;
    while (expect_num > 0 && attempts <= max_retries)
    {
        num = write(drive_ptr->DEV_FILE_PTR, data_ptr, expect_num);
        if (num < 0)
        {
            error = MFS_WRITE_FAULT;
            break;
        }
        if (num > 0)
        {
            expect_num -= num;
            data_ptr += num << (drive_ptr->SECTOR_POWER - shifter);
            attempts = 0; /* there is a progress, reset attempts counter */
        }
        attempts++;
    }

    if (expect_num > 0)
    {
#if MQX_USE_IO_OLD
        error = drive_ptr->DEV_FILE_PTR->ERROR;
#else
        error = errno;
#endif
    }

    if (error == MFS_NO_ERROR && expect_num)
    {
        /* Ensure that error code is always set if less than requested data was written */
        error = MFS_WRITE_FAULT;
    }

    if (processed)
        *processed = ((sector_count << shifter) - expect_num) >> shifter;

    return error;
}


/*!
 * \brief Write one sector to the device.
 *
 * \param drive_ptr
 * \param[in] sector_number Sector number to read/write from/to file system medium.
 * \param[in,out] sector_ptr Address of where data is to be stored/written.
 *
 * \return _mfs_error
 */
_mfs_error MFS_Write_device_sector(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    uint32_t sector_number,
    char *sector_ptr)
{
    return MFS_Write_device_sectors(drive_ptr, sector_number, 1, MFSCFG_MAX_WRITE_RETRIES, sector_ptr, NULL);
}

#endif  //!MFSCFG_READ_ONLY


/*!
 * \brief Reads consecutive sectors into given buffer.
 *
 * \param drive_ptr
 * \param[in] sector_number First sector to read/write from/to file system medium.
 * \param[in] sector_count Number of sectors to read/write from/to file system medium.
 * \param[in] max_retries Number of retries of the same low level operation if it fails.
 * \param[in,out] buffer_ptr Address of where data is to be stored/written.
 * \param[out] processed Number of sector successfully processed.
 *
 * \return _mfs_error
 */
_mfs_error MFS_Read_device_sectors(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    uint32_t sector_number,
    uint32_t sector_count,
    uint32_t max_retries,
    char *buffer_ptr,
    uint32_t *processed)
{
    uint32_t attempts;
    int32_t num, expect_num, shifter, seek_loc;
    _mfs_error error;

    error = MFS_NO_ERROR;

    if (sector_number + sector_count - 1 > drive_ptr->MEGA_SECTORS)
    {
        return (MFS_SECTOR_NOT_FOUND);
    }

    MFS_LOG(printf("MFS_Read_device_sectors %d %d", sector_number, sector_count));

    if (drive_ptr->BLOCK_MODE)
    {
        shifter = 0;
        seek_loc = sector_number;
        expect_num = sector_count;
    }
    else
    {
        shifter = drive_ptr->SECTOR_POWER;
        seek_loc = sector_number << shifter;
        expect_num = sector_count << shifter;
    }

#if MQX_USE_IO_OLD
    fseek(drive_ptr->DEV_FILE_PTR, seek_loc, IO_SEEK_SET);
#else
    lseek(drive_ptr->DEV_FILE_PTR, seek_loc, SEEK_SET);
//TODO: check errno lseek
#endif

    attempts = 0;
    while (expect_num > 0 && attempts <= max_retries)
    {
        num = read(drive_ptr->DEV_FILE_PTR, buffer_ptr, expect_num);
        if (num < 0)
        {
            error = MFS_READ_FAULT;
            break;
        }
        else if (num > 0)
        {
            expect_num -= num;
            buffer_ptr += num << (drive_ptr->SECTOR_POWER - shifter);
            attempts = 0; /* there is a progress, reset attempts counter */
        }
        attempts++;
    }

    if (expect_num > 0)
    {
#if MQX_USE_IO_OLD
        error = drive_ptr->DEV_FILE_PTR->ERROR;
#else
        error = errno;
#endif
    }

    if (error == MFS_NO_ERROR && expect_num)
    {
        /* Ensure that error code is always set if less than requested data was read */
        error = MFS_READ_FAULT;
    }

    if (processed)
        *processed = ((sector_count << shifter) - expect_num) >> shifter;

    return error;
}
