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
*   This file contains adaptation layer for FIO subsystem.
*
*
*END************************************************************************/

#include "mfs.h"
#include "mfs_prv.h"


struct init_t
{
    MQX_FILE_PTR dev_fd; /*[IN] the device on which to install MFS */
    uint32_t part_num;
};

static int _io_mfs_init(void *init_data, void **dev_context);


static _mqx_int _io_mfs_open(MQX_FILE_PTR fd_ptr, char *, char *flags_str);
static _mqx_int _io_mfs_close(MQX_FILE_PTR fd_ptr);
static _mqx_int _io_mfs_read(MQX_FILE_PTR file_ptr, char *data_ptr, int32_t num);
static _mqx_int _io_mfs_write(MQX_FILE_PTR file_ptr, char *data_ptr, int32_t num);
static _file_offset _io_mfs_lseek(MQX_FILE_PTR file_ptr, _file_offset offset, _mqx_uint mode);
static _mqx_int _io_mfs_ioctl(MQX_FILE_PTR file_ptr, _mqx_uint cmd, void *param_ptr);
static _mqx_int _io_mfs_deinit(struct io_device_struct *dev_context);

static const IO_DRVIF_STRUCT _io_mfs_drvif = {
    _io_mfs_open,
    _io_mfs_close,
    _io_mfs_read,
    _io_mfs_write,
    _io_mfs_lseek,
    _io_mfs_ioctl,
    _io_mfs_deinit
};


/*!
 * \brief Initialize the MSDOS File System.
 *
 * \param[in] dev_fd The device on which to install MFS.
 * \param[in] identifier The name that should be given to mfs (ex: "C:", "MFS1:", etc..).
 * \param[in] partition_num The partition number to install MFS on. 0 for no partitions.
 *
 * \return int Error code.
 */
int _io_mfs_install(
    MQX_FILE_PTR dev_fd,
    char *identifier,
    uint32_t partition_num)
{
    int error_code;
    void *handle = NULL;

    struct init_t mfs_init = {
        .dev_fd = dev_fd,
        .part_num = partition_num,
    };

    error_code = _io_mfs_init(&mfs_init, &handle);
    if (error_code != MFS_NO_ERROR)
    {
        return error_code;
    }

    return _io_dev_install_drvif(identifier, &_io_mfs_drvif, handle);
}


/*!
 * \brief Initialize the MSDOS File System.
 *
 * \param init_data
 * \param dev_context
 *
 * \return int Error code.
 */
static int _io_mfs_init(
    void *init_data,
    void **dev_context)
{
    MFS_DRIVE_STRUCT_PTR drive_ptr;
    struct init_t *mfs_init = (struct init_t *)init_data;
    _mfs_error error_code;

    error_code = MFS_Create_drive(&drive_ptr);
    if (error_code)
    {
        return error_code;
    }

    /* Fill in storage device handle */
    drive_ptr->DEV_FILE_PTR = mfs_init->dev_fd;
    drive_ptr->DRV_NUM = mfs_init->part_num;

    /* Inform Kernel that MFS is installed */
    _mqx_set_io_component_handle(IO_MFS_COMPONENT, (void *)MFS_VERSION);

    *dev_context = (void *)drive_ptr;

    return 0;
}


/*!
 * \brief Initialize the MSDOS File System.
 *
 * \param[in] identifier The name that should be given to mfs (ex: "C:", "MFS1:", etc..).
 *
 * \return int Error code.
 */
int _io_mfs_uninstall(
    char *identifier)
{
    return _io_dev_uninstall(identifier);
}


/*!
 * \brief Uninstalls the MSDOS File System and frees all memory allocated to it.
 *
 * \param[in] dev_context The identifier of the device.
 *
 * \return _mqx_int Error code.
 */
static _mqx_int _io_mfs_deinit(
    struct io_device_struct *dev_context)
{
    MFS_DRIVE_STRUCT_PTR drive_ptr = (MFS_DRIVE_STRUCT_PTR)dev_context->DRIVER_INIT_PTR;
    return MFS_Destroy_drive(drive_ptr);
}


/*!
 * \brief Opens and initializes MFS driver.
 *
 * \param[in] fd_ptr The file descriptor being opened.
 * \param[in] open_name_ptr The remaining portion of the name of the device.
 * \param[in] flags_str The flags to specify file type:
 *                                            w  write
 *                                            r  read
 *                                            a append
 *                                            x for temp file
 *                                            n for new file
 *                                            etc...
 *
 * \return _mqx_int MQX_OK or an error.
 */
static _mqx_int _io_mfs_open(
    MQX_FILE_PTR fd_ptr,
    char *open_name_ptr,
    char *flags_str)
{
    MFS_DRIVE_STRUCT_PTR drive_ptr;
    MFS_HANDLE_PTR handle = NULL;
    uint32_t fsflags;
    uint32_t error_code = MFS_NO_ERROR;

    drive_ptr = (MFS_DRIVE_STRUCT_PTR)fd_ptr->DEV_PTR->DRIVER_INIT_PTR;

    if (!drive_ptr->DEV_OPEN)
    {
        /* This means we are opening MFS and not a file . */
        error_code = MFS_Open_Device(drive_ptr);
        if (error_code == MFS_NOT_A_DOS_DISK)
        {
            /*
            ** This error is OK. It just means the disk probably has to be
            ** formatted
            */
            fd_ptr->ERROR = error_code;
            error_code = MFS_NO_ERROR;
        }
        if (error_code == MFS_NO_ERROR)
        {
            _io_register_file_system(fd_ptr, (char *)open_name_ptr);
            drive_ptr->DEV_OPEN = TRUE;
        }

        /* Reset file struct members */
        fd_ptr->LOCATION = 0;
        fd_ptr->SIZE = 0;

        return error_code;
    }

    else
    {
        /* Check for filename */
        open_name_ptr = MFS_Parse_Out_Device_Name((char *)open_name_ptr);
        if (*open_name_ptr == '\0')
        {
            return MFS_PATH_NOT_FOUND;
        }

        /*
        ** When opening a file, only read-only attributes are set. The other
        ** attributes (such as hidden, system, etc..) must be set afterwards
        ** with an ioctl call.
        */

        switch (*flags_str)
        {
            case 'r':
                fsflags = MFS_O_RDONLY;
                break;

#if !MFSCFG_READ_ONLY
            case 'w':
                fsflags = MFS_O_WRONLY | MFS_O_CREAT | MFS_O_TRUNC;
                break;

            case 'a':
                fsflags = MFS_O_WRONLY | MFS_O_CREAT | MFS_O_APPEND;
                break;

            case 'n':
                fsflags = MFS_O_WRONLY | MFS_O_CREAT | MFS_O_EXCL;
                break;
#endif  //!MFSCFG_READ_ONLY

            default:
                return MFS_INVALID_PARAMETER;
        }
        flags_str++;

        /* Walk through the rest of the mode string */
        while (*flags_str)
        {
            if (*flags_str == '+')
            {
                /* Extend access mode to read/write */
                fsflags = (fsflags & ~MFS_O_ACCMODE) | MFS_O_RDWR;
            }
            else if ((*flags_str == 'x') && (fsflags & MFS_O_CREAT))
            {
                /* Exclusive creation, C2011 extension */
                fsflags |= MFS_O_EXCL;
            }
            flags_str++;
        }

        /* Reset file struct members */
        fd_ptr->LOCATION = 0;
        fd_ptr->SIZE = 0;

        /* Open or create the file */
        handle = MFS_Open_file(drive_ptr, (char *)open_name_ptr, fsflags, &error_code);

        if (error_code != MFS_NO_ERROR)
        {
            /* There is an error, free the file handle if any. This is to cover potential corner case which should not happen */
            if (handle != NULL)
            {
                MFS_Close_file(drive_ptr, handle);
                handle = NULL;
            }
        }
        else if (handle == NULL)
        {
            /* There is no error but the file handle is not NULL. This is to cover potential corner case which should not happen */
            error_code = MFS_INVALID_HANDLE;
        }
        else
        {
            /* There is no error and the file handle is valid, update fd before returning. */
            fd_ptr->DEV_DATA_PTR = (void *)handle;
            fd_ptr->SIZE = handle->DIR_ENTRY->FILE_SIZE;
        }
    }

    return error_code;
}


/*!
 * \brief Closes MFS driver.
 *
 * \param[in] fd_ptr The file handle for the device being closed.
 *
 * \return _mqx_int Error code.
 */
static _mqx_int _io_mfs_close(
    MQX_FILE_PTR fd_ptr)
{
    MFS_HANDLE_PTR handle = (MFS_HANDLE_PTR)fd_ptr->DEV_DATA_PTR;
    MFS_DRIVE_STRUCT_PTR drive_ptr = (MFS_DRIVE_STRUCT_PTR)fd_ptr->DEV_PTR->DRIVER_INIT_PTR;
    uint32_t result = MQX_OK;

    // unregister file system from handle table
    _io_unregister_file_system(fd_ptr);

    if (handle == NULL)
    {
        /* We are closing the mfs_fd_ptr, and not a normal file */
        result = MFS_Close_Device(drive_ptr);
        drive_ptr->DEV_OPEN = FALSE;
    }
    else
    {
        /* We are closing a normal file */
        result = MFS_Close_file(drive_ptr, handle);
    }

    return (result);
}


/*!
 * \brief Reads data from MFS driver.
 *
 * \param[in] file_ptr The stream to perform the operation on.
 * \param[in] data_ptr The data location to read to.
 * \param[in] num The number of bytes to read.
 *
 * \return _mqx_int Number of characters read.
 */
static _mqx_int _io_mfs_read(
    MQX_FILE_PTR file_ptr,
    char *data_ptr,
    int32_t num)
{
    MFS_HANDLE_PTR handle = (MFS_HANDLE_PTR)file_ptr->DEV_DATA_PTR;
    MFS_DRIVE_STRUCT_PTR drive_ptr = (MFS_DRIVE_STRUCT_PTR)file_ptr->DEV_PTR->DRIVER_INIT_PTR;
    int32_t result;
    _mfs_error errcode;

    result = MFS_Read(drive_ptr, handle, num, data_ptr, &errcode);

    file_ptr->ERROR = errcode;
    file_ptr->LOCATION = handle->LOCATION;
    /* Check for EOF. The MFS EOF must be translated to the standard EOF */
    if (errcode == MFS_EOF)
    {
        file_ptr->FLAGS |= IO_FLAG_AT_EOF;
    }

    return (errcode == 0 || errcode == MFS_EOF) ? result : IO_ERROR;
}


/*!
 * \brief Writes data to the fdv_ram device.
 *
 * \param[in] file_ptr The stream to perform the operation on.
 * \param[in] data_ptr The data location to write to.
 * \param[in] num The number of bytes to write.
 *
 * \return _mqx_int Number of characters written.
 */
static _mqx_int _io_mfs_write(
    MQX_FILE_PTR file_ptr,
    char *data_ptr,
    int32_t num)
{
#if !MFSCFG_READ_ONLY

    MFS_HANDLE_PTR handle = (MFS_HANDLE_PTR)file_ptr->DEV_DATA_PTR;
    MFS_DRIVE_STRUCT_PTR drive_ptr = (MFS_DRIVE_STRUCT_PTR)file_ptr->DEV_PTR->DRIVER_INIT_PTR;
    int32_t result = 0;
    _mfs_error errcode;

    if (data_ptr == NULL)
    {
        if (num == 0)
        {
            errcode = MFS_Flush_Device(drive_ptr, handle);
        }
        else
        {
            return MFS_ERROR;
        }
    }
    else
    {
        result = MFS_Write(drive_ptr, handle, num, (char *)data_ptr, &errcode);
    }

    file_ptr->ERROR = errcode;

    /* Update location and file size in the FIO structure - legacy compatibility */
    file_ptr->LOCATION = handle->LOCATION;
    file_ptr->SIZE = handle->DIR_ENTRY->FILE_SIZE;

    /* Check for EOF. The MFS EOF must be translated to the standard EOF */
    if (result == 0 && errcode == MFS_EOF)
    {
        file_ptr->FLAGS |= IO_FLAG_AT_EOF;
    }

    return (errcode == 0 || errcode == MFS_EOF) ? result : IO_ERROR;

#else  //MFSCFG_READ_ONLY

    file_ptr->ERROR = MFS_OPERATION_NOT_ALLOWED;
    return IO_ERROR;

#endif  //MFSCFG_READ_ONLY
}


/*!
 * \brief The returned value is offset after the seek operation or -1  in case of an error.
 *
 * \param[in] file_ptr The stream to perform the operation on.
 * \param[in] offset Relative or absolute file offset.
 * \param[in] mode Seek mode.
 *
 * \return _file_offset
 */
static _file_offset _io_mfs_lseek(
    MQX_FILE_PTR file_ptr,
    _file_offset offset,
    _mqx_uint mode)
{
    MFS_HANDLE_PTR handle = (MFS_HANDLE_PTR)file_ptr->DEV_DATA_PTR;
    _file_offset location;

    switch (mode)
    {
        case IO_SEEK_SET:
            location = offset;
            break;
        case IO_SEEK_CUR:
            location = handle->LOCATION + offset;
            break;
        case IO_SEEK_END:
            location = handle->DIR_ENTRY->FILE_SIZE + offset;
            break;
        default:
            location = -1; /* Set location to invalid value */
            break;
    }

    /* Check validity of location */
    if (location < 0 || location > MFS_MAX_FILESIZE)
    {
        location = -1; /* -1 value indicates an error to the caller */
    }
    else
    {
        /* Store valid location */
        handle->LOCATION = location;
    }

    return location;
}


/*!
 * \brief The returned value an MQX error code.
 *
 * \param[in] file_ptr The stream to perform the operation on.
 * \param[in] cmd The ioctl command.
 * \param[in] param_ptr The ioctl parameters.
 *
 * \return int32_t
 */
static _mqx_int _io_mfs_ioctl(
    MQX_FILE_PTR file_ptr,
    _mqx_uint cmd,
    void *param_ptr)
{
    MFS_HANDLE_PTR handle = (MFS_HANDLE_PTR)file_ptr->DEV_DATA_PTR;
    MFS_DRIVE_STRUCT_PTR drive_ptr = (MFS_DRIVE_STRUCT_PTR)file_ptr->DEV_PTR->DRIVER_INIT_PTR;

    return MFS_ioctl(drive_ptr, handle, cmd, param_ptr);
}
