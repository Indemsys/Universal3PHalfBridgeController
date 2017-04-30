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
 * \brief Get the date and time of last update of a specific file.
 *
 * \param drive_ptr
 * \param handle
 * \param[in,out] date_ptr File date is written to this address.
 * \param[in,out] time_ptr File time is written to this address.
 *
 * \return _mfs_error (MFS_NO_ERROR or MFS_INVALID_FILE_HANDLE)
 */
_mfs_error MFS_Get_date_time(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    MFS_HANDLE_PTR handle,
    uint16_t *date_ptr,
    uint16_t *time_ptr)
{

    _mqx_int error_code;

    error_code = MFS_lock_and_enter(drive_ptr, 0);
    if (error_code != MFS_NO_ERROR)
    {
        return error_code;
    }

    /*
    ** Check the date and time ptrs; if any is NULL, don't write to it.
    */
    if (date_ptr)
    {
        *date_ptr = handle->DIR_ENTRY->WRITE_DATE;
    }
    if (time_ptr)
    {
        *time_ptr = handle->DIR_ENTRY->WRITE_TIME;
    }

    MFS_leave_and_unlock(drive_ptr, 0);
    return error_code;
}


#if !MFSCFG_READ_ONLY

/*!
 * \brief Set the date and time of last update of a specific file.
 *
 * \param drive_ptr
 * \param handle
 * \param[in] date_ptr File date to be written into this file's entry.
 * \param[in] time_ptr File time to be written into this file's entry.
 *
 * \return _mfs_error
 */
_mfs_error MFS_Set_date_time(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    MFS_HANDLE_PTR handle,
    uint16_t *date_ptr,
    uint16_t *time_ptr)
{
    _mfs_error error_code;

    error_code = MFS_lock_and_enter(drive_ptr, MFS_ENTER_READWRITE);
    if (error_code != MFS_NO_ERROR)
    {
        return error_code;
    }

    /* update values in the directory entry and mark it dirty */
    handle->DIR_ENTRY->WRITE_DATE = *date_ptr;
    handle->DIR_ENTRY->WRITE_TIME = *time_ptr;
    handle->DIR_ENTRY->DIRTY = 1;

    /* we do not want to set date and time in close, so set touched to 0 */
    handle->TOUCHED = 0;

    MFS_leave_and_unlock(drive_ptr, 0);
    return error_code;
}

#endif
