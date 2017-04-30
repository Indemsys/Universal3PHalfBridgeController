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
*   This file contains functions to lock and unlock the drive.
*
*
*END************************************************************************/

#include "mfs.h"
#include "mfs_prv.h"


/*!
 * \brief Finds the highest 1 bit in the argument using bisection
 *
 * \param x
 *
 * \return int Binary logarithm of the argument truncated to integer.
 */
int ilog2(uint32_t x)
{
    uint32_t x2;
    int n = 0;

    x2 = (x & 0xFFFF0000);
    if (x2 != 0)
    {
        x = x2;
        n += 16;
    }

    x2 = (x & 0xFF00FF00);
    if (x2 != 0)
    {
        x = x2;
        n += 8;
    }

    x2 = (x & 0xF0F0F0F0);
    if (x2 != 0)
    {
        x = x2;
        n += 4;
    }

    x2 = (x & 0xCCCCCCCC);
    if (x2 != 0)
    {
        x = x2;
        n += 2;
    }

    x2 = (x & 0xAAAAAAAA);
    if (x2 != 0)
    {
        x = x2;
        n += 1;
    }

    return n;
}


/*!
 * \brief Checks if the filesystem is isntalled as read-only.
 *
 * \param drive_ptr
 *
 * \return bool
 */
bool MFS_is_read_only(
    MFS_DRIVE_STRUCT_PTR drive_ptr)
{
#if MFSCFG_READ_ONLY
    /* Always return false if build without write support */
    return false;
#else
    return drive_ptr->READ_ONLY;
#endif
}


/*!
 * \brief Allocates and initializes drive context structure.
 *
 * \param drive_ptr_ptr
 *
 * \return _mfs_error
 */
_mfs_error MFS_Create_drive(
    MFS_DRIVE_STRUCT_PTR *drive_ptr_ptr)
{
    MFS_DRIVE_STRUCT_PTR drive_ptr;
    _mfs_error error_code = MFS_NO_ERROR;

    if (drive_ptr_ptr == NULL)
    {
        return MFS_INVALID_POINTER;
    }

    drive_ptr = MFS_mem_alloc_system_zero(sizeof(MFS_DRIVE_STRUCT));
    if (drive_ptr == NULL)
    {
        return MFS_INSUFFICIENT_MEMORY;
    }
    _mem_set_type(drive_ptr, MEM_TYPE_MFS_DRIVE_STRUCT);

#if MFSCFG_USE_MUTEX
    {
        MUTEX_ATTR_STRUCT mutex_attr;

        error_code = _mutatr_init(&mutex_attr);
        if (error_code == MFS_NO_ERROR)
        {
            error_code = _mutatr_set_sched_protocol(&mutex_attr, MUTEX_PRIO_INHERIT);
            if (error_code == MFS_NO_ERROR)
            {
                error_code = _mutatr_set_wait_protocol(&mutex_attr, MUTEX_PRIORITY_QUEUEING);
            }

            if (error_code == MFS_NO_ERROR)
            {
                error_code = _mutex_init(&drive_ptr->MUTEX, &mutex_attr);
            }

            _mutatr_destroy(&mutex_attr);
        }
    }
#else
    error_code = _lwsem_create(&drive_ptr->LWSEM, 1);
#endif

    if (error_code)
    {
        MFS_mem_free(drive_ptr);
    }

    *drive_ptr_ptr = drive_ptr;
    return error_code;
}


/*!
 * \brief Deallocates drive context structure.
 *
 * \param drive_ptr
 *
 * \return _mfs_error
 */
_mfs_error MFS_Destroy_drive(
    MFS_DRIVE_STRUCT_PTR drive_ptr)
{
    if (drive_ptr->DEV_OPEN)
    {
        return MFS_SHARING_VIOLATION;
    }

#if MFSCFG_USE_MUTEX
    _mutex_destroy(&drive_ptr->MUTEX);
#else
    _lwsem_destroy(&drive_ptr->LWSEM);
#endif

    MFS_mem_free(drive_ptr);

    return MFS_NO_ERROR;
}


/*!
 * \brief Obtains the MFS drive lock. Cannot be called from an ISR.
 *
 * \param drive_ptr
 *
 * \return _mfs_error
 */
_mfs_error MFS_lock(
    MFS_DRIVE_STRUCT_PTR drive_ptr)
{
#if MFSCFG_USE_MUTEX
    return _mutex_lock(&drive_ptr->MUTEX);
#else
    return _lwsem_wait(&drive_ptr->LWSEM);
#endif
}


/*!
 * \brief Releases the MFS drive lock.
 *
 * \param drive_ptr
 *
 * \return _mfs_error
 */
_mfs_error MFS_unlock(
    MFS_DRIVE_STRUCT_PTR drive_ptr)
{
#if MFSCFG_USE_MUTEX
    return _mutex_unlock(&drive_ptr->MUTEX);
#else
    return _lwsem_post(&drive_ptr->LWSEM);
#endif
}


/*!
 * \brief
 *
 * Verifies that the drive is mounted and possibly checks other preconditions.
 * The function assumes that the drive is locked.
 *
 * \param drive_ptr
 * \param mode
 *
 * \return _mfs_error
 */
_mfs_error MFS_enter(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    int mode)
{
    if (!drive_ptr->DOS_DISK)
    {
        return MFS_NOT_A_DOS_DISK;
    }

    if ((mode & MFS_ENTER_READWRITE) && MFS_is_read_only(drive_ptr))
    {
        return MFS_DISK_IS_WRITE_PROTECTED;
    }

    return MFS_NO_ERROR;
}


/*!
 * \brief
 *
 * Executes operations to be done before leaving MFS call.
 * The function assumes that the drive is locked.
 *
 * \param drive_ptr
 * \param flags
 *
 * \return _mfs_error
 */
_mfs_error MFS_leave(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    int flags)
{
    _mfs_error error_code = MFS_NO_ERROR;
    _mfs_error error_temp;

    /* unmap dictory sector when leaving MFS */
    error_temp = MFS_unmap_directory_sector(drive_ptr);

    switch (drive_ptr->WRITE_CACHE_POLICY)
    {
        case MFS_WRITE_THROUGH_CACHE:
            /* flush whole sector cache */
            error_code = MFS_sector_cache_flush(drive_ptr, 0, 0);
            break;

        case MFS_MIXED_MODE_CACHE:
            /* flush untagged cache records (metadata only, exclude data sectors) */
            error_code = MFS_sector_cache_flush_tag(drive_ptr, 0);
            break;

        case MFS_WRITE_BACK_CACHE:
            /* do not flush anything, flushing is done on request or upon unmount */
            break;

        default:
            break;
    }

    error_code = (error_code == MFS_NO_ERROR) ? error_temp : error_code;

    return error_code;
}


/*!
 * \brief
 *
 * Locks the drive and executes operations to be done upon entering MFS call.
 * Cannot be called from an ISR.
 *
 * \param drive_ptr
 * \param mode
 *
 * \return _mfs_error
 */
_mfs_error MFS_lock_and_enter(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    int mode)
{
    _mfs_error error_code;

    error_code = MFS_lock(drive_ptr);
    if (error_code != MFS_NO_ERROR)
    {
        return error_code;
    }

    error_code = MFS_enter(drive_ptr, mode);
    if (error_code != MFS_NO_ERROR)
    {
        MFS_unlock(drive_ptr);
    }

    return error_code;
}


/*!
 * \brief
 *
 * Executes operations to be done before leaving MFS call
 * and unconditionally unlocks the drive.
 *
 * \param drive_ptr
 * \param flags
 *
 * \return _mfs_error
 */
_mfs_error MFS_leave_and_unlock(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    int flags)
{
    _mfs_error error_code;

    error_code = MFS_leave(drive_ptr, flags);

    MFS_unlock(drive_ptr);

    return error_code;
}


/*!
 * \brief
 *
 * \param size
 *
 * \return void *
 */
void *MFS_mem_alloc(_mem_size size)
{
    if (_MFS_pool_id)
    {
        return _mem_alloc_from(_MFS_pool_id, size);
    }
    else
    {
        return _mem_alloc(size);
    }
}

/*!
 * \brief
 *
 * \param size
 *
 * \return void *
 */
void *MFS_mem_alloc_zero(_mem_size size)
{
    if (_MFS_pool_id)
    {
        return _mem_alloc_zero_from(_MFS_pool_id, size);
    }
    else
    {
        return _mem_alloc_zero(size);
    }
}

/*!
 * \brief
 *
 * \param size
 *
 * \return void *
 */
void *MFS_mem_alloc_system(_mem_size size)
{
    if (_MFS_pool_id)
    {
        return _mem_alloc_system_from(_MFS_pool_id, size);
    }
    else
    {
        return _mem_alloc_system(size);
    }
}

/*!
 * \brief
 *
 * \param size
 *
 * \return void *
 */
void *MFS_mem_alloc_system_zero(_mem_size size)
{
    if (_MFS_pool_id)
    {
        return _mem_alloc_system_zero_from(_MFS_pool_id, size);
    }
    else
    {
        return _mem_alloc_system_zero(size);
    }
}

/*!
 * \brief
 *
 * \param size
 * \param align
 *
 * \return void *
 */
void *MFS_mem_alloc_system_align(_mem_size size, _mem_size align)
{
    if (_MFS_pool_id)
    {
        return _mem_alloc_system_align_from(_MFS_pool_id, size, align);
    }
    else
    {
        return _mem_alloc_system_align(size, align);
    }
}

/*!
 * \brief
 *
 * \param mem_ptr
 *
 * \return _mfs_error
 */
_mfs_error MFS_mem_free(void *mem_ptr)
{
    return _mem_free(mem_ptr);
}


/*!
 * \brief
 *
 * \param path_ptr_ptr
 *
 * \return _mfs_error
 */
_mfs_error MFS_alloc_path(char **path_ptr_ptr)
{
    char *path_ptr;

    path_ptr = MFS_mem_alloc_system(PATHNAME_SIZE + 1);
    if (path_ptr == NULL)
    {
        return MFS_INSUFFICIENT_MEMORY;
    }
    _mem_set_type(path_ptr, MEM_TYPE_MFS_PATHNAME);
    *path_ptr_ptr = path_ptr;
    return MFS_NO_ERROR;
}


/*!
 * \brief
 *
 * \param path1_ptr_ptr
 * \param path2_ptr_ptr
 *
 * \return _mfs_error
 */
_mfs_error MFS_alloc_2paths(char **path1_ptr_ptr, char **path2_ptr_ptr)
{
    _mfs_error error_code;
    char *path1_ptr, *path2_ptr;

    error_code = MFS_alloc_path(&path1_ptr);
    if (error_code == MFS_NO_ERROR)
    {
        error_code = MFS_alloc_path(&path2_ptr);
        if (error_code == MFS_NO_ERROR)
        {
            *path1_ptr_ptr = path1_ptr;
            *path2_ptr_ptr = path2_ptr;
        }
        else
        {
            MFS_free_path(path1_ptr);
        }
    }

    return error_code;
}


/*!
 * \brief
 *
 * \param path_ptr
 *
 * \return bool
 */
bool MFS_free_path(char *path_ptr)
{
    if (path_ptr != NULL)
    {
        _mem_free(path_ptr);
        return true;
    }

    return false;
}


/*!
 * \brief Isolates the pathname and filename.
 *
 * \param name_ptr
 *
 * \return char * A pointer to character following the semi-colon (':').
 */
char *MFS_Parse_Out_Device_Name(
    char *name_ptr)
{
    char *start_path_ptr;

    start_path_ptr = name_ptr;

    /* Parse out the device name */
    while ((*start_path_ptr != ':') && (*start_path_ptr != '\0'))
    {
        start_path_ptr = start_path_ptr + 1;
    }

    if (*start_path_ptr == '\0')
    {
        start_path_ptr = name_ptr;
    }
    else
    {
        start_path_ptr++;
    }

    return (start_path_ptr);
}


/*!
 * \brief Takes a MFS error code and returns the error description ptr.
 *
 * \param error_code
 *
 * \return char *
 */
char *MFS_Error_text(
    _mfs_error error_code)
{
    switch (error_code)
    {
        case MFS_NO_ERROR:
            return ("MFS NO ERROR");
        case MFS_INVALID_FUNCTION_CODE:
            return ("MFS INVALID FUNCTION CODE");
        case MFS_FILE_NOT_FOUND:
            return ("MFS FILE NOT FOUND");
        case MFS_PATH_NOT_FOUND:
            return ("MFS PATH NOT FOUND");
        case MFS_ACCESS_DENIED:
            return ("MFS ACCESS DENIED");
        case MFS_INVALID_HANDLE:
            return ("MFS INVALID HANDLE");
        case MFS_INSUFFICIENT_MEMORY:
            return ("MFS INSUFFICIENT MEMORY");
        case MFS_INVALID_MEMORY_BLOCK_ADDRESS:
            return ("MFS INVALID MEMORY BLOCK ADDRESS");
        case MFS_ATTEMPT_TO_REMOVE_CURRENT_DIR:
            return ("MFS ATTEMPT TO REMOVE CURRENT DIR");
        case MFS_DISK_IS_WRITE_PROTECTED:
            return ("MFS DISK IS WRITE PROTECTED");
        case MFS_BAD_DISK_UNIT:
            return ("MFS BAD DISK UNIT");
        case MFS_INVALID_LENGTH_IN_DISK_OPERATION:
            return ("MFS INVALID LENGTH IN DISK OPERATION");
        case MFS_NOT_A_DOS_DISK:
            return ("MFS NOT A DOS DISK");
        case MFS_SECTOR_NOT_FOUND:
            return ("MFS SECTOR NOT FOUND");
        case MFS_WRITE_FAULT:
            return ("MFS WRITE FAULT");
        case MFS_READ_FAULT:
            return ("MFS READ FAULT");
        case MFS_SHARING_VIOLATION:
            return ("MFS SHARING VIOLATION");
        case MFS_FILE_EXISTS:
            return ("MFS FILE EXISTS");
        case MFS_ALREADY_ASSIGNED:
            return ("MFS ALREADY ASSIGNED");
        case MFS_INVALID_PARAMETER:
            return ("MFS INVALID PARAMETER");
        case MFS_DISK_FULL:
            return ("MFS DISK FULL");
        case MFS_ROOT_DIR_FULL:
            return ("MFS ROOT DIR FULL");
        case MFS_EOF:
            return ("MFS EOF");
        case MFS_CANNOT_CREATE_DIRECTORY:
            return ("MFS CANNOT CREATE DIRECTORY");
        case MFS_NOT_INITIALIZED:
            return ("MFS NOT INITIALIZED");
        case MFS_OPERATION_NOT_ALLOWED:
            return ("MFS OPERATION NOT ALLOWED");
        case MFS_ERROR_INVALID_DRIVE_HANDLE:
            return ("MFS INVALID DRIVE HANDLE");
        case MFS_ERROR_INVALID_FILE_HANDLE:
            return ("MFS INVALID FILE HANDLE");
        case MFS_ERROR_UNKNOWN_FS_VERSION:
            return ("MFS UNKNOWN FILESYSTEM VERSION");
        case MFS_LOST_CHAIN:
            return ("MFS LOST CHAIN");
        case MFS_INVALID_DEVICE:
            return ("MFS INVALID DEVICE");
        case MFS_INVALID_CLUSTER_NUMBER:
            return ("MFS INVALID CLUSTER NUMBER");
        case MFS_FAILED_TO_DELETE_LFN:
            return ("MFS FAILED TO DELETE LFN");
        case MFS_BAD_LFN_ENTRY:
            return ("MFS BAD LFN ENTRY");
        case PMGR_INVALID_PARTITION:
            return ("PMGR INVALID PARTITION");
        case PMGR_INSUF_MEMORY:
            return ("PMGR INSUF MEMORY");
        case PMGR_UNKNOWN_PARTITION:
            return ("PMGR UNKNOWN PARTITION TYPE");
        case PMGR_INVALID_PARTTABLE:
            return ("PMGR INVALID PARTITION TABLE");
        default:
            return ("UNKNOWN ERROR !!!");
    }
}
