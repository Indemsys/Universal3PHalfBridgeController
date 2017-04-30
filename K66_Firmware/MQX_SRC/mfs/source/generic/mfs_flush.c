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
*   This file contains implementation of flush function
*
*
*END************************************************************************/

#include "mfs.h"
#include "mfs_prv.h"


#if !MFSCFG_READ_ONLY


/*!
 * \brief Writes caches and other related data structures to the media.
 *
 * \param drive_ptr
 * \param handle
 *
 * \return int32_t MQX_OK or an error
 */
int32_t MFS_Flush_Device(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    MFS_HANDLE_PTR handle)
{
    _mfs_error error_code;
    _mfs_error error_temp;

    error_code = MFS_lock_and_enter(drive_ptr, MFS_ENTER_READWRITE);
    if (error_code == MFS_NO_ERROR)
    {
        if (handle != NULL)
        {
            /* flushing particular file - update directory record */
            error_code = MFS_update_entry(drive_ptr, handle);

            if (handle->DIR_ENTRY->HEAD_CLUSTER)
            {
                /* flush data sectors of the particular file */
                error_temp = MFS_sector_cache_flush_tag(drive_ptr, handle->DIR_ENTRY->HEAD_CLUSTER);
                error_code = (error_code == MFS_NO_ERROR) ? error_temp : error_code;
            }

            /* flush all metadata (untagged sectors) */
            error_temp = MFS_sector_cache_flush_tag(drive_ptr, 0);
            error_code = (error_code == MFS_NO_ERROR) ? error_temp : error_code;
        }
        else
        {
            /* flushing whole device - update directory entries of all open files */
            error_code = MFS_update_entries(drive_ptr);
            /* flush whole sector cache */
            error_temp = MFS_sector_cache_flush(drive_ptr, 0, 0);
            error_code = (error_code == MFS_NO_ERROR) ? error_temp : error_code;
        }

        error_temp = MFS_leave_and_unlock(drive_ptr, 0);
        error_code = (error_code == MFS_NO_ERROR) ? error_temp : error_code;
    }

    return error_code;
}

#endif
