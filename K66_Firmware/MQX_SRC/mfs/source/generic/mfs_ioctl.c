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
*   This file contains implementation of IOCTL functions
*
*
*END************************************************************************/

#include "mfs.h"
#include "mfs_prv.h"


/*!
 * \brief
 *
 * \param[in] drive_ptr The drive on which to operate
 * \param[in] handle File handle upon which the action is to be taken
 * \param[in] cmd The ioctl command.
 * \param[in] ... The ioctl parameters.
 *
 * \return int The returned value is 0 or an MQX error code.
 */
int MFS_ioctl(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    MFS_HANDLE_PTR handle,
    unsigned long int cmd,
    ...)
{
    int result;
    va_list ap;

    va_start(ap, cmd);
    result = MFS_vioctl(drive_ptr, handle, cmd, ap);
    va_end(ap);

    return result;
}


/*!
 * \brief
 *
 * \param[in] drive_ptr The drive on which to operate
 * \param[in] handle File handle upon which the action is to be taken
 * \param[in] cmd The ioctl command.
 * \param[in] ap The ioctl parameters.
 *
 * \return int The returned value is 0 or an MQX error code.
 */
int MFS_vioctl(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    MFS_HANDLE_PTR handle,
    unsigned long int cmd,
    va_list ap)
{
    uint32_t *param_ptr;
    uint64_t *param64_ptr;
    _mfs_error result = MFS_OK;

    switch (cmd)
    {
        case IO_IOCTL_BAD_CLUSTERS:
            param_ptr = va_arg(ap, uint32_t *);
            result = MFS_Bad_clusters(drive_ptr, param_ptr);
            break;

        case IO_IOCTL_LAST_CLUSTER:
            param_ptr = va_arg(ap, uint32_t *);
            result = MFS_Last_cluster(drive_ptr, param_ptr);
            break;

        case IO_IOCTL_FREE_SPACE:
            param64_ptr = va_arg(ap, uint64_t *);
            if (param64_ptr)
            {
                result = MFS_Get_disk_free_space(drive_ptr, NULL, param64_ptr);
            }
            else
            {
                result = MFS_INVALID_POINTER;
            }
            break;

        case IO_IOCTL_FREE_CLUSTERS:
            param_ptr = va_arg(ap, uint32_t *);
            if (param_ptr)
            {
                result = MFS_Get_disk_free_space(drive_ptr, param_ptr, NULL);
            }
            else
            {
                result = MFS_INVALID_POINTER;
            }
            break;

        case IO_IOCTL_GET_CLUSTER_SIZE:
            MFS_lock(drive_ptr);
            param_ptr = va_arg(ap, uint32_t *);
            *param_ptr = (uint32_t)drive_ptr->CLUSTER_SIZE_BYTES;
            MFS_unlock(drive_ptr);
            break;

        case IO_IOCTL_GET_CURRENT_DIR:
            param_ptr = va_arg(ap, uint32_t *);
            result = MFS_Get_current_dir(drive_ptr, (char *)param_ptr);
            break;

        case IO_IOCTL_CHANGE_CURRENT_DIR:
            param_ptr = va_arg(ap, uint32_t *);
            param_ptr = (uint32_t *)MFS_Parse_Out_Device_Name((char *)param_ptr);
            result = MFS_Change_current_dir(drive_ptr, (char *)param_ptr);
            break;

        case IO_IOCTL_CHECK_DIR_EXIST:
            param_ptr = va_arg(ap, uint32_t *);
            param_ptr = (uint32_t *)MFS_Parse_Out_Device_Name((char *)param_ptr);
            result = MFS_Check_dir_exist(drive_ptr, (char *)param_ptr);
            break;

        case IO_IOCTL_FIND_FIRST_FILE:
            param_ptr = va_arg(ap, uint32_t *);
            result = MFS_find_first_file(drive_ptr, (MFS_SEARCH_PARAM_PTR)param_ptr);
            break;

        case IO_IOCTL_FIND_NEXT_FILE:
            param_ptr = va_arg(ap, uint32_t *);
            result = MFS_find_next_file(drive_ptr, (MFS_SEARCH_DATA_PTR)param_ptr);
            break;

        case IO_IOCTL_GET_FILE_ATTR:
            param_ptr = va_arg(ap, uint32_t *);
            result = MFS_Get_file_attributes(drive_ptr, handle, MFS_Parse_Out_Device_Name(((MFS_FILE_ATTR_PARAM_PTR)param_ptr)->PATHNAME), ((MFS_FILE_ATTR_PARAM_PTR)param_ptr)->ATTRIBUTE_PTR);
            break;

        case IO_IOCTL_GET_DATE_TIME:
            param_ptr = va_arg(ap, uint32_t *);
            result = MFS_Get_date_time(drive_ptr, handle, ((MFS_DATE_TIME_PARAM_PTR)param_ptr)->DATE_PTR, ((MFS_DATE_TIME_PARAM_PTR)param_ptr)->TIME_PTR);
            break;

        case IO_IOCTL_CHECK_FORMATTED:
            param_ptr = va_arg(ap, uint32_t *);
            if (param_ptr == NULL)
            {
                result = MFS_INVALID_POINTER;
            }
            else if (MFS_lock_and_enter(drive_ptr, 0) == MFS_OK)
            {
                *param_ptr = 1;
                MFS_leave_and_unlock(drive_ptr, 0);
            }
            else
            {
                *param_ptr = 0;
            }
            break;

        case IO_IOCTL_GET_LFN:
            param_ptr = va_arg(ap, uint32_t *);
            result = MFS_lock_and_enter(drive_ptr, 0);
            if (result == MFS_OK)
            {
                MFS_GET_LFN_STRUCT_PTR get_lfn_ptr = ((MFS_GET_LFN_STRUCT_PTR)param_ptr);
                char *pathname = MFS_Parse_Out_Device_Name(get_lfn_ptr->PATHNAME);
                result = MFS_get_lfn(drive_ptr, pathname, get_lfn_ptr->LONG_FILENAME);
                MFS_leave_and_unlock(drive_ptr, 0);
            }
            break;

        case IO_IOCTL_GET_VOLUME:
            param_ptr = va_arg(ap, uint32_t *);
            result = MFS_Get_volume(drive_ptr, (char *)param_ptr);
            break;

#if MFSCFG_READ_ONLY
        case IO_IOCTL_DELETE_FILE:
        case IO_IOCTL_CREATE_SUBDIR:
        case IO_IOCTL_REMOVE_SUBDIR:
        case IO_IOCTL_RENAME_FILE:
        case IO_IOCTL_SET_FILE_ATTR:
        case IO_IOCTL_FLUSH_OUTPUT:
        case IO_IOCTL_SET_DATE_TIME:
        case IO_IOCTL_SET_VOLUME:
        case IO_IOCTL_FORMAT:
        case IO_IOCTL_DEFAULT_FORMAT:
        case IO_IOCTL_FORMAT_TEST:
        case IO_IOCTL_WRITE_CACHE_ON:
        case IO_IOCTL_WRITE_CACHE_OFF:
        case IO_IOCTL_TEST_UNUSED_CLUSTERS:
        case IO_IOCTL_GET_WRITE_CACHE_MODE:
        case IO_IOCTL_SET_WRITE_CACHE_MODE:
        case IO_IOCTL_VERIFY_WRITES:
            result = MFS_OPERATION_NOT_ALLOWED;
            break;
#else  //MFSCFG_READ_ONLY

        case IO_IOCTL_DELETE_FILE:
            param_ptr = va_arg(ap, uint32_t *);
            param_ptr = (uint32_t *)MFS_Parse_Out_Device_Name((char *)param_ptr);
            result = MFS_Delete_file(drive_ptr, handle, (char *)param_ptr);
            break;

        case IO_IOCTL_CREATE_SUBDIR:
            param_ptr = va_arg(ap, uint32_t *);
            param_ptr = (uint32_t *)MFS_Parse_Out_Device_Name((char *)param_ptr);
            result = MFS_Create_subdir(drive_ptr, (char *)param_ptr);
            break;

        case IO_IOCTL_REMOVE_SUBDIR:
            param_ptr = va_arg(ap, uint32_t *);
            param_ptr = (uint32_t *)MFS_Parse_Out_Device_Name((char *)param_ptr);
            result = MFS_Remove_subdir(drive_ptr, (char *)param_ptr);
            break;

        case IO_IOCTL_RENAME_FILE:
            param_ptr = va_arg(ap, uint32_t *);
            result = MFS_Rename_file(drive_ptr,
                                     MFS_Parse_Out_Device_Name(((MFS_RENAME_PARAM_PTR)param_ptr)->OLD_PATHNAME),
                                     MFS_Parse_Out_Device_Name(((MFS_RENAME_PARAM_PTR)param_ptr)->NEW_PATHNAME));
            break;

        case IO_IOCTL_SET_FILE_ATTR:
            param_ptr = va_arg(ap, uint32_t *);
            result = MFS_Set_file_attributes(drive_ptr, handle, MFS_Parse_Out_Device_Name(((MFS_FILE_ATTR_PARAM_PTR)param_ptr)->PATHNAME), *((MFS_FILE_ATTR_PARAM_PTR)param_ptr)->ATTRIBUTE_PTR);
            break;

        case IO_IOCTL_SET_DATE_TIME:
            param_ptr = va_arg(ap, uint32_t *);
            result = MFS_Set_date_time(drive_ptr, handle, ((MFS_DATE_TIME_PARAM_PTR)param_ptr)->DATE_PTR, ((MFS_DATE_TIME_PARAM_PTR)param_ptr)->TIME_PTR);
            break;

        case IO_IOCTL_SET_VOLUME:
            param_ptr = va_arg(ap, uint32_t *);
            result = MFS_Set_volume(drive_ptr, (char *)param_ptr);
            break;

        case IO_IOCTL_WRITE_CACHE_ON:
            param_ptr = va_arg(ap, uint32_t *);
            MFS_lock(drive_ptr);
            drive_ptr->WRITE_CACHE_POLICY = MFS_WRITE_BACK_CACHE;
            MFS_unlock(drive_ptr);
            break;

        case IO_IOCTL_WRITE_CACHE_OFF:
            MFS_lock(drive_ptr);
            drive_ptr->WRITE_CACHE_POLICY = MFS_WRITE_THROUGH_CACHE;
            MFS_unlock(drive_ptr);
            break;

        case IO_IOCTL_FLUSH_OUTPUT:
            result = MFS_Flush_Device(drive_ptr, handle);
            break;

        case IO_IOCTL_SET_WRITE_CACHE_MODE:
            param_ptr = va_arg(ap, uint32_t *);
            if (param_ptr)
            {
                _mfs_cache_policy policy = *((_mfs_cache_policy *)param_ptr);
                if ((policy == MFS_WRITE_THROUGH_CACHE) || (policy == MFS_MIXED_MODE_CACHE) || (policy == MFS_WRITE_BACK_CACHE))
                {
                    MFS_lock(drive_ptr);
                    drive_ptr->WRITE_CACHE_POLICY = policy;
                    MFS_unlock(drive_ptr);
                }
                else
                {
                    result = MFS_INVALID_PARAMETER;
                }
            }
            else
            {
                result = MFS_INVALID_POINTER;
            }
            break;


        case IO_IOCTL_GET_WRITE_CACHE_MODE:
            param_ptr = va_arg(ap, uint32_t *);
            if (param_ptr)
            {
                *((_mfs_cache_policy *)param_ptr) = drive_ptr->WRITE_CACHE_POLICY;
            }
            else
            {
                result = MFS_INVALID_POINTER;
            }
            break;

        case IO_IOCTL_FAT_CACHE_ON:
        case IO_IOCTL_FAT_CACHE_OFF:
        case IO_IOCTL_FLUSH_FAT:
        case IO_IOCTL_SET_FAT_CACHE_MODE:
        case IO_IOCTL_GET_FAT_CACHE_MODE:
            /* Dedicated FAT cache is obsolete */
            result = MFS_INVALID_FUNCTION_CODE;
            break;

        case IO_IOCTL_TEST_UNUSED_CLUSTERS:
            param_ptr = va_arg(ap, uint32_t *);
            result = MFS_Test_unused_clusters(drive_ptr, param_ptr);
            break;

        case IO_IOCTL_VERIFY_WRITES:
            /* Obsolete, not supported */
            result = MFS_INVALID_FUNCTION_CODE;
            break;

#ifdef MFSCFG_ENABLE_FORMAT

        case IO_IOCTL_FORMAT:
            param_ptr = va_arg(ap, uint32_t *);
            result = MFS_Format(drive_ptr, ((MFS_IOCTL_FORMAT_PARAM_PTR)param_ptr)->FORMAT_PTR);
            break;

        case IO_IOCTL_DEFAULT_FORMAT:
            result = MFS_Default_Format(drive_ptr);
            break;

        case IO_IOCTL_FORMAT_TEST:
            param_ptr = va_arg(ap, uint32_t *);
            result = MFS_Format(drive_ptr, ((MFS_IOCTL_FORMAT_PARAM_PTR)param_ptr)->FORMAT_PTR);
            if (result == MFS_OK)
            {
                result = MFS_Test_unused_clusters(drive_ptr, ((MFS_IOCTL_FORMAT_PARAM_PTR)param_ptr)->COUNT_PTR);
            }
            break;

#else  //MFSCFG_ENABLE_FORMAT
        case IO_IOCTL_FORMAT:
        case IO_IOCTL_DEFAULT_FORMAT:
        case IO_IOCTL_FORMAT_TEST:
            result = MFS_OPERATION_NOT_ALLOWED;
            break;

#endif  //MFSCFG_ENABLE_FORMAT

#endif  //MFSCFG_READ_ONLY

        case IO_IOCTL_GET_DEVICE_HANDLE:
            param_ptr = va_arg(ap, uint32_t *);
            *param_ptr = (uint32_t)drive_ptr->DEV_FILE_PTR;
            break;

        default:
#if MQX_USE_IO_OLD
            result = IO_ERROR_INVALID_IOCTL_CMD;
#else
            result = NIO_EINVAL;
#endif
            break;
    }

    return ((int32_t)result);
}
