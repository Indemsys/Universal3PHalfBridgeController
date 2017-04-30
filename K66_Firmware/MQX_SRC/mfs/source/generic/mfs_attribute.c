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
*   This file contains the file specific interface functions
*   of MFS which need a filename as an input parameter.
*
*
*END************************************************************************/

#include "mfs.h"
#include "mfs_prv.h"


/*!
 * \brief Get the attribute byte of the specified file.
 *
 * \param[in] drive_ptr
 * \param[in] handle
 * \param[in] pathname
 * \param[in] attribute_ptr
 *
 * \return _mfs_error
 */
_mfs_error MFS_Get_file_attributes(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    MFS_HANDLE_PTR handle,
    char *pathname,
    unsigned char *attribute_ptr)
{
    _mfs_error error_code;
    unsigned char attribute;
    DIR_ENTRY_DISK dir_entry;

    if ((pathname == NULL) || (attribute_ptr == NULL) || (*pathname == '\0'))
    {
        return MFS_INVALID_PARAMETER;
    }

    error_code = MFS_lock_and_enter(drive_ptr, 0);
    if (error_code != MFS_NO_ERROR)
    {
        return (error_code);
    }

    error_code = MFS_locate_dir_entry(drive_ptr, pathname, &dir_entry, NULL, NULL);
    if (error_code == MFS_NO_ERROR)
    {
        attribute = mqx_dtohc(dir_entry.ATTRIBUTE);
        *attribute_ptr = attribute;
    }

    MFS_leave_and_unlock(drive_ptr, 0);

    return (error_code);
}


#if !MFSCFG_READ_ONLY

/*!
 * \brief Change the attribute of the specified file.
 *
 * \param[in] drive_ptr
 * \param[in] handle
 * \param[in] pathname Pathname of the specific file
 * \param[in] attribute Attribute of file
 *
 * \return _mfs_error
 */
_mfs_error MFS_Set_file_attributes(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    MFS_HANDLE_PTR handle,
    char *pathname,
    unsigned char attribute)
{
    _mfs_error error_code;
    _mfs_error error_code_tmp;
    DIR_ENTRY_DISK *dir_entry_ptr;
    uint32_t entry_sector;
    uint32_t entry_index;
    unsigned char orig_attr;
    bool updated = false;

    if ((pathname == NULL) || (*pathname == '\0'))
    {
        return MFS_INVALID_PARAMETER;
    }

    error_code = MFS_lock_and_enter(drive_ptr, MFS_ENTER_READWRITE);
    if (error_code != MFS_NO_ERROR)
    {
        return error_code;
    }

    error_code = MFS_locate_dir_entry(drive_ptr, pathname, NULL, &entry_sector, &entry_index);
    if (error_code == MFS_NO_ERROR)
    {
        error_code = MFS_sector_map(drive_ptr, entry_sector, (void **)&dir_entry_ptr, MFS_MAP_MODE_MODIFY, 0);
        if (error_code == MFS_NO_ERROR)
        {
            dir_entry_ptr += entry_index;
            orig_attr = mqx_dtohc(dir_entry_ptr->ATTRIBUTE);

            if (orig_attr & MFS_ATTR_VOLUME_NAME)
            {
                /* do not allow change to entries with volume name attribute */
                error_code = MFS_ACCESS_DENIED;
            }
            else
            {
                attribute &= MFS_ATTR_ARCHIVE | MFS_ATTR_READ_ONLY | MFS_ATTR_HIDDEN_FILE | MFS_ATTR_SYSTEM_FILE;
                attribute |= orig_attr & MFS_ATTR_DIR_NAME; /* preserve original dir attribute */
                if (orig_attr != attribute)
                {
                    mqx_htodc(dir_entry_ptr->ATTRIBUTE, attribute);
                    updated = true;
                }
            }

            error_code_tmp = MFS_sector_unmap(drive_ptr, entry_sector, updated);
            if (error_code == MFS_NO_ERROR)
            {
                error_code = error_code_tmp;
            }
        }
    }

    error_code_tmp = MFS_leave_and_unlock(drive_ptr, 0);
    if (error_code == MFS_NO_ERROR)
    {
        error_code = error_code_tmp;
    }

    return error_code;
}

#endif
