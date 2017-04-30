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


#if !MFSCFG_READ_ONLY

/*!
 * \brief Delete a specific file.
 *
 * \param[in] drive_ptr
 * \param[in] handle
 * \param[in] pathname Directory and file name of the file to delete.
 *
 * \return _mfs_error
 */
_mfs_error MFS_Delete_file(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    MFS_HANDLE_PTR handle,
    char *pathname)
{
    _mfs_error error_code;
    _mfs_error error_code_tmp;

    FAT_CHAIN dir_chain;

    char *entry_name;
    DIR_ENTRY_DISK entry_copy;
    uint32_t entry_sector;
    uint32_t entry_index;
    uint32_t entry_loc;

    uint32_t first_cluster;

    if ((pathname == NULL) || (*pathname == '\0'))
    {
        return MFS_INVALID_PARAMETER;
    }

    error_code = MFS_lock_and_enter(drive_ptr, MFS_ENTER_READWRITE);
    if (error_code != MFS_NO_ERROR)
    {
        return error_code;
    }

    /* Find the directory in which the file shall be located */
    error_code = MFS_get_dir_chain(drive_ptr, pathname, &dir_chain, NULL, &entry_name);
    if (error_code == MFS_NO_ERROR)
    {
        /* Lookup entry  with the requested name in the directory */
        error_code = MFS_scan_dir_chain(drive_ptr, &dir_chain, entry_name, &entry_copy, &entry_sector, &entry_index, &entry_loc);
        if (error_code == MFS_NO_ERROR)
        {
            if (mqx_dtohc(entry_copy.ATTRIBUTE) & (MFS_ATTR_DIR_NAME | MFS_ATTR_VOLUME_NAME))
            {
                /* Not a file */
                error_code = MFS_ACCESS_DENIED;
            }
            else if (MFS_Find_handle_new(drive_ptr, entry_sector, entry_index) != NULL)
            {
                /* File is currently open */
                error_code = MFS_SHARING_VIOLATION;
            }
            else
            {
                first_cluster = clustoh(drive_ptr, entry_copy.HFIRST_CLUSTER, entry_copy.LFIRST_CLUSTER);
                error_code = MFS_Release_chain(drive_ptr, first_cluster);
                if (error_code == MFS_NO_ERROR || error_code == MFS_LOST_CHAIN)
                {
                    error_code_tmp = MFS_free_dir_entry(drive_ptr, &dir_chain, entry_loc);
                    if (error_code == MFS_NO_ERROR)
                    {
                        error_code = error_code_tmp;
                    }
                }
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
