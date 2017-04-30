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
*   This file contains functions related to using file handles.
*
*
*END************************************************************************/

#include "mfs.h"
#include "mfs_prv.h"


/*!
 * \brief Creates a new file handle and adds it the to queue of open handles.
 *
 * \param[in] drive_ptr The drive on which to operate.
 * \param[in] assoc_with Pointer to existing handle to associate with (representing the same file).
 *
 * \return MFS_HANDLE_PTR A file handle.
 */
MFS_HANDLE_PTR MFS_Create_handle(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    MFS_HANDLE_PTR assoc_with)
{
    MFS_HANDLE_PTR handle_ptr;

    handle_ptr = MFS_mem_alloc_system_zero(sizeof(MFS_HANDLE));

    if (handle_ptr != NULL)
    {
        _mem_set_type(handle_ptr, MEM_TYPE_MFS_OTHER);

        if (assoc_with != NULL)
        {
            handle_ptr->DIR_ENTRY = assoc_with->DIR_ENTRY;
        }
        else
        {
            handle_ptr->DIR_ENTRY = (DIR_ENTRY *)MFS_mem_alloc_system_zero(sizeof(DIR_ENTRY));
            _mem_set_type(handle_ptr->DIR_ENTRY, MEM_TYPE_MFS_OTHER);
        }

        if (handle_ptr->DIR_ENTRY != NULL)
        {
            handle_ptr->DIR_ENTRY->REFCNT++;
            _queue_enqueue(&drive_ptr->HANDLE_LIST, (QUEUE_ELEMENT_STRUCT_PTR)handle_ptr);
        }
        else
        {
            MFS_mem_free(handle_ptr);
            handle_ptr = NULL;
        }
    }

    return handle_ptr;
}


/*!
 * \brief Disposes handle structure performing necessary cleanup
 *
 * \param[in] drive_ptr The drive on which to operate.
 * \param[in] handle_ptr Handle to be released.
 *
 * \return void
 */
void MFS_Destroy_handle(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    MFS_HANDLE_PTR handle_ptr)
{
    if (handle_ptr != NULL)
    {
        /* Unlink handle from HANDLE_LIST */
        _queue_unlink(&drive_ptr->HANDLE_LIST, (QUEUE_ELEMENT_STRUCT_PTR)handle_ptr);

        /* If this handle is the last one referring to the related directory entry then dispose it */
        if ((handle_ptr->DIR_ENTRY != NULL) && (--(handle_ptr->DIR_ENTRY->REFCNT) == 0))
        {
            MFS_mem_free(handle_ptr->DIR_ENTRY);
        }

        /* Release memory */
        MFS_mem_free(handle_ptr);
    }
}


/*!
 * \brief
 *
 * \param[in] drive_ptr The drive on which to operate.
 * \param[in] entry_sector
 * \param[in] entry_index
 *
 * \return MFS_HANDLE_PTR
 */
MFS_HANDLE_PTR MFS_Find_handle_new(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    uint32_t entry_sector,
    uint32_t entry_index)
{
    MFS_HANDLE_PTR handle;

    handle = (MFS_HANDLE_PTR)_queue_head(&(drive_ptr->HANDLE_LIST));
    while (handle)
    {
        if ((handle->DIR_ENTRY->ENTRY_SECTOR == entry_sector) && (handle->DIR_ENTRY->ENTRY_INDEX == entry_index))
        {
            break;
        }
        handle = (MFS_HANDLE_PTR)_queue_next(&drive_ptr->HANDLE_LIST, (QUEUE_ELEMENT_STRUCT_PTR)handle);
    }

    return handle;
}


/*!
 * \brief
 *
 * \param[in] drive_ptr The drive on which to operate.
 * \param[in] dir_cluster
 * \param[in] dir_index
 *
 * \return MFS_HANDLE_PTR
 */
MFS_HANDLE_PTR MFS_Find_handle(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    uint32_t dir_cluster,
    uint32_t dir_index)
{
    uint32_t entry_sector;
    uint32_t entry_index;

    entry_sector = dir_cluster ? CLUSTER_TO_SECTOR(drive_ptr, dir_cluster) : drive_ptr->ROOT_START_SECTOR;
    entry_sector += INDEX_TO_SECTOR(drive_ptr, dir_index);
    entry_index = INDEX_WITHIN_SECTOR(drive_ptr, dir_index);

    return MFS_Find_handle_new(drive_ptr, entry_sector, entry_index);
}
