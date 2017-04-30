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
*   This file contains the function for making sure the device on which
*   MFS is being run on is suitable
*
*
*END************************************************************************/

#include "mfs.h"
#include "mfs_prv.h"
#include "part_mgr.h"

/*!
 * \brief Checks the lower layer device for compatibility
 *
 * \param[in] dev_fd The file handle for the lower layer device.
 * \param[out] sector_size_ptr Pointer to where the sector size is to be stored.
 * \param[out] block_mode_ptr Pointer to where the block mode is to be stored.
 *
 * \return _mfs_error MFS_NO_ERROR or an error
 */
_mfs_error _mfs_validate_device(
    MFS_FD_TYPE dev_fd,
    uint32_t *sector_size_ptr,
    bool *block_mode_ptr)
{
    _mfs_error error_code;
    uint32_t sector_size = 0;
    bool block_mode = false;

#ifdef IO_DEV_ATTR_BLOCK_MODE
    uint32_t id_array[3];

    id_array[0] = id_array[1] = id_array[2] = 0;

    /*
    ** Issue the id command. Block mode drivers must support this command but
    ** other drivers can support it also
    */
    error_code = ioctl(dev_fd, IO_IOCTL_DEVICE_IDENTIFY, id_array);
    if (0 == error_code)
    {
        /*
        ** The identify command is supported.
        ** Check to see if it is a block mode
        */
        if (id_array[MFS_IOCTL_ID_ATTR_ELEMENT] & IO_DEV_ATTR_BLOCK_MODE)
        {
            block_mode = true;
        }
    }
#if MQX_USE_IO_OLD
    else if (error_code == IO_ERROR_INVALID_IOCTL_CMD)
#else
    else if ((errno == NIO_EINVAL) || (errno == NIO_ENOTSUP))
#endif
    {
        /*
        ** The ID command is not supported by the lower layer.
        ** It is not a block mode driver
        */
        block_mode = false;
        error_code = MFS_NO_ERROR;
    }
    else
    {
        return error_code;
    }
#endif

    error_code = ioctl(dev_fd, IO_IOCTL_GET_BLOCK_SIZE, &sector_size);
    if (0 == error_code)
    {
        /* Check to see if the sector or block size is suitable for FAT */
        if ((4096 % sector_size) != 0)
        {
            /*
            ** The block size isn't compatible.
            */
            error_code = MFS_INVALID_DEVICE;
        }
    }
#if MQX_USE_IO_OLD
    else if (error_code == IO_ERROR_INVALID_IOCTL_CMD)
#else
    else if ((errno == NIO_EINVAL) || (errno == NIO_ENOTSUP))
#endif
    {
        /*
        ** The command is not supported.
        ** This is OK as long as it isn't a block mode driver.
        */
        if (!block_mode)
        {
            /*
            ** It doesn't matter that we can't tell the actual block size. MFS
            ** will be doing byte accesses anyway since it isn't a block mode
            ** driver.
            */
            sector_size = MFS_DEFAULT_SECTOR_SIZE;
            error_code = MFS_NO_ERROR;
        }
    }

    if (sector_size_ptr)
        *sector_size_ptr = sector_size;

    if (block_mode_ptr)
        *block_mode_ptr = block_mode;

    return error_code;
}
