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
*   This file contains the functions for a partition manager device that is
*   is built to work under MFS1.40. These functions can also create or
*   remove partitions from a disk.
*
*
*END************************************************************************/

#include "mfs.h"
#include "mfs_prv.h"
#include "part_mgr.h"


static int32_t _io_part_mgr_uninstall_internal(IO_DEVICE_STRUCT_PTR dev_ptr);


/*!
 * \brief Installs the partition manager device.
 *
 * \param[in] dev_fd Handle of the device on which to install the partition manager.
 * \param[in] identifier The new name of the partition manager device.
 * \param[in] sector_size The size of sectors in bytes on the lower level device.
 *
 * \return int32_t Error code.
 */
int32_t _io_part_mgr_install(
    MQX_FILE_PTR dev_fd,
    char *identifier,
    uint32_t sector_size)
{
    PART_MGR_STRUCT_PTR pm_struct_ptr;
    int32_t error_code;

    pm_struct_ptr = MFS_mem_alloc_system_zero(sizeof(PART_MGR_STRUCT));
    if (pm_struct_ptr == NULL)
    {
        return (PMGR_INSUF_MEMORY);
    }

    _mem_set_type(pm_struct_ptr, MEM_TYPE_PART_MGR_STRUCT);

/*
 ** Create partition manager lock
 */
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
                error_code = _mutex_init(&pm_struct_ptr->MUTEX, &mutex_attr);
            }

            _mutatr_destroy(&mutex_attr);
        }
    }
#else
    error_code = _lwsem_create(&pm_struct_ptr->LWSEM, 1);
#endif

    if (error_code)
    {
        MFS_mem_free(pm_struct_ptr);
        return error_code;
    }

    /* Store the handle of the device in the lower layer */
    pm_struct_ptr->DEV_FILE_PTR = dev_fd;
    pm_struct_ptr->INSTANCES = 0;

    error_code = _io_dev_install_ext(
        identifier,
        _io_part_mgr_open,
        _io_part_mgr_close,
        _io_part_mgr_read,
        _io_part_mgr_write,
        (_mqx_int(_CODE_PTR_)(MQX_FILE_PTR, _mqx_uint, void *))_io_part_mgr_ioctl,
        _io_part_mgr_uninstall_internal,
        (void *)pm_struct_ptr);

    if (error_code)
    {
#if MFSCFG_USE_MUTEX
        _mutex_destroy(&pm_struct_ptr->MUTEX);
#else
        _lwsem_destroy(&pm_struct_ptr->LWSEM);
#endif
        MFS_mem_free(pm_struct_ptr);
    }

    return error_code;
}


/*!
 * \brief Uninstalls the partition manager device.
 *
 * \param[in] dev_ptr The identifier of the device.
 *
 * \return int32_t Error code.
 */
static int32_t _io_part_mgr_uninstall_internal(
    IO_DEVICE_STRUCT_PTR dev_ptr)
{
    PART_MGR_STRUCT_PTR pm_struct_ptr;

    pm_struct_ptr = (PART_MGR_STRUCT_PTR)dev_ptr->DRIVER_INIT_PTR;

    PMGR_LOCK(pm_struct_ptr);

    if (pm_struct_ptr->INSTANCES)
    {
        PMGR_UNLOCK(pm_struct_ptr);
        return IO_ERROR_DEVICE_BUSY;
    }

#if MFSCFG_USE_MUTEX
    _mutex_destroy(&pm_struct_ptr->MUTEX);
#else
    _lwsem_destroy(&pm_struct_ptr->LWSEM);
#endif

    MFS_mem_free(pm_struct_ptr);

    return IO_OK;
}

#if MFSCFG_DEPRICATED_UNINSTALL
/*!
 * \brief Uninstalls the partition manager device.
 *
 * \param[in] identifier The identifier string of the device.
 *
 * \return int32_t Error code.
 */
int32_t _io_part_mgr_uninstall(
    char *identifier)
{
    return _io_dev_uninstall(identifier);
}
#endif


/*!
 * \brief Does nothing.
 * All the initialization is done in the io_part_mgr_install function.
 *
 * \param[in] fd_ptr The file pointer of the partition manager to open.
 * \param[in] open_name_ptr The name of the device that is being opened.
 * \param[in] open_mode_ptr The flags that determine the open mode.
 *
 * \retval MQX_OK.
 */
int32_t _io_part_mgr_open(
    MQX_FILE_PTR fd_ptr,
    char *open_name_ptr,
    char *open_mode_ptr)
{
    PART_MGR_STRUCT_PTR pm_struct_ptr;
    PMGR_PART_INFO_STRUCT_PTR part_info_ptr;
    MQX_FILE_PTR dev_fd;

    int32_t error;
    int32_t part_num;

    char *file_name_ptr;

    file_name_ptr = open_name_ptr;
    while (*file_name_ptr && *file_name_ptr++ != ':')
    {
    }

    if (file_name_ptr[0])
    {
        part_num = file_name_ptr[0] - '0';
        if (file_name_ptr[1] != '\0' || part_num < 0 || part_num > PMGR_MAX_PARTITIONS)
            return PMGR_INVALID_PARTITION;
    }
    else
    {
        part_num = 0;
    }

    pm_struct_ptr = (PART_MGR_STRUCT_PTR)fd_ptr->DEV_PTR->DRIVER_INIT_PTR;
    dev_fd = pm_struct_ptr->DEV_FILE_PTR;

    PMGR_LOCK(pm_struct_ptr);

    /* Validate device (check block mode and size) if no other instances exist so far */
    if (pm_struct_ptr->INSTANCES == 0)
    {
        error = _mfs_validate_device(dev_fd, &pm_struct_ptr->DEV_SECTOR_SIZE, &pm_struct_ptr->BLOCK_MODE);
        if (error)
        {
            PMGR_UNLOCK(pm_struct_ptr);
            return error;
        }

        pm_struct_ptr->DEV_NUM_SECTORS = 0;
        ioctl(dev_fd, IO_IOCTL_GET_NUM_SECTORS, &(pm_struct_ptr->DEV_NUM_SECTORS));

        pm_struct_ptr->ALIGNMENT = 1; /* Default alignment is one byte (which means no special requirement) */
        ioctl(dev_fd, IO_IOCTL_GET_REQ_ALIGNMENT, &(pm_struct_ptr->ALIGNMENT));
    }

    part_info_ptr = MFS_mem_alloc_system_zero(sizeof(PMGR_PART_INFO_STRUCT));
    if (part_info_ptr == NULL)
    {
        PMGR_UNLOCK(pm_struct_ptr);
        return PMGR_INSUF_MEMORY;
    }
    _mem_set_type(part_info_ptr, MEM_TYPE_PART_INFO_STRUCT);

    /* Check whether to open partition or operate on whole device (part_num==0) */
    if (part_num)
    {
        part_info_ptr->SLOT = part_num;
        if (_pmgr_get_part_info(pm_struct_ptr, part_info_ptr))
        {

            MFS_mem_free(part_info_ptr);
            PMGR_UNLOCK(pm_struct_ptr);
            return PMGR_INVALID_PARTITION;
        }
    }
    else
    {
        /* Use size of device as length of the partition */
        part_info_ptr->LENGTH = pm_struct_ptr->DEV_NUM_SECTORS;
    }

    fd_ptr->DEV_DATA_PTR = part_info_ptr;
    pm_struct_ptr->INSTANCES++;

    PMGR_UNLOCK(pm_struct_ptr);
    return IO_OK;
}


/*!
 * \brief Frees the memory allocated to hold the partition info.
 *
 * \param[in] fd_ptr The file pointer of the partition manager to close.
 *
 * \retval MQX_OK
 */
int32_t _io_part_mgr_close(
    MQX_FILE_PTR fd_ptr)
{
    PART_MGR_STRUCT_PTR pm_struct_ptr;

    pm_struct_ptr = (PART_MGR_STRUCT_PTR)fd_ptr->DEV_PTR->DRIVER_INIT_PTR;

    PMGR_LOCK(pm_struct_ptr);

    pm_struct_ptr->INSTANCES--;
    MFS_mem_free(fd_ptr->DEV_DATA_PTR);

    PMGR_UNLOCK(pm_struct_ptr);

    return IO_OK;
}


/*!
 * \brief Calls the read function of the next layer.
 *
 * \param[in] fd_ptr The file pointer of the partition manager to read from.
 * \param[in] data_ptr The data location to read to.
 * \param[in] num The number of bytes to read.
 *
 * \return int32_t The number of bytes read from device.
 */
int32_t _io_part_mgr_read(
    MQX_FILE_PTR fd_ptr,
    char *data_ptr,
    int32_t num)
{
    PART_MGR_STRUCT_PTR pm_struct_ptr;
    PMGR_PART_INFO_STRUCT_PTR part_info_ptr;
    uint32_t location;
    uint32_t part_start;
    uint32_t part_end;
    int32_t result;

    pm_struct_ptr = (PART_MGR_STRUCT_PTR)fd_ptr->DEV_PTR->DRIVER_INIT_PTR;
    part_info_ptr = (PMGR_PART_INFO_STRUCT_PTR)fd_ptr->DEV_DATA_PTR;

    /* If a partition is selected, also its start has to be defined */
    if (part_info_ptr->SLOT && part_info_ptr->START_SECTOR == 0)
    {
        fd_ptr->ERROR = PMGR_INVALID_PARTITION;
        return IO_ERROR;
    }

    part_start = part_info_ptr->START_SECTOR;
    part_end = part_info_ptr->LENGTH ? part_start + part_info_ptr->LENGTH : 0;

    /* If not operating on a block device, multiply values by sector size */
    if (!(pm_struct_ptr->BLOCK_MODE))
    {
        part_start *= pm_struct_ptr->DEV_SECTOR_SIZE;
        part_end *= pm_struct_ptr->DEV_SECTOR_SIZE;
    }

    /* Calculate seek location for undelying device */
    location = part_start + fd_ptr->LOCATION;
    if (part_end)
    {
        /* Check whether lhe location points into the partition area */
        if (location >= part_end)
        {
            fd_ptr->ERROR = IO_ERROR_SEEK;
            return IO_ERROR;
        }
        /* Check whether the transfer would overflow the partition area and shorten it if necessary */
        if (location + num > part_end)
        {
            num = part_end - location;
        }
    }

    /* Lock to prevent concurrent access to underlying device */
    PMGR_LOCK(pm_struct_ptr);

    /* Perform seek and data transfer */
    result = fseek(pm_struct_ptr->DEV_FILE_PTR, location, IO_SEEK_SET);
    if (result == MQX_OK)
    {
        result = read(((PART_MGR_STRUCT_PTR)fd_ptr->DEV_PTR->DRIVER_INIT_PTR)->DEV_FILE_PTR, data_ptr, num);
    }
    fd_ptr->ERROR = pm_struct_ptr->DEV_FILE_PTR->ERROR;

    /* Unlock */
    PMGR_UNLOCK(pm_struct_ptr);

    return result;
}


/*!
 * \brief Calls the write function of the next layer.
 *
 * \param[in] fd_ptr The file pointer of the partition manager to read from.
 * \param[in] data_ptr The data location to read from.
 * \param[in] num The number of bytes to write.
 *
 * \return int32_t The number of bytes writen to device.
 */
int32_t _io_part_mgr_write(
    MQX_FILE_PTR fd_ptr,
    char *data_ptr,
    int32_t num)
{
    PART_MGR_STRUCT_PTR pm_struct_ptr;
    PMGR_PART_INFO_STRUCT_PTR part_info_ptr;
    uint32_t location;
    uint32_t part_start;
    uint32_t part_end;
    int32_t result;

    pm_struct_ptr = (PART_MGR_STRUCT_PTR)fd_ptr->DEV_PTR->DRIVER_INIT_PTR;
    part_info_ptr = (PMGR_PART_INFO_STRUCT_PTR)fd_ptr->DEV_DATA_PTR;

    /* If a partition is selected, also its start has to be defined */
    if (part_info_ptr->SLOT && part_info_ptr->START_SECTOR == 0)
    {
        fd_ptr->ERROR = PMGR_INVALID_PARTITION;
        return IO_ERROR;
    }

    part_start = part_info_ptr->START_SECTOR;
    part_end = part_info_ptr->LENGTH ? part_start + part_info_ptr->LENGTH : 0;

    /* If not operating on a block device, multiply values by sector size */
    if (!(pm_struct_ptr->BLOCK_MODE))
    {
        part_start *= pm_struct_ptr->DEV_SECTOR_SIZE;
        part_end *= pm_struct_ptr->DEV_SECTOR_SIZE;
    }

    /* Calculate seek location for undelying device */
    location = fd_ptr->LOCATION + part_start;
    if (part_info_ptr->LENGTH)
    {
        /* Check whether lhe location points into the partition area */
        if (location >= part_end)
        {
            fd_ptr->ERROR = IO_ERROR_SEEK;
            return IO_ERROR;
        }
        /* Check whether the transfer would overflow the partition area and shorten it if necessary */
        if (location + num > part_end)
        {
            num = part_end - location;
        }
    }

    /* Lock to prevent concurrent access to underlying device */
    PMGR_LOCK(pm_struct_ptr);

    /* Perform seek and data transfer */
    result = fseek(pm_struct_ptr->DEV_FILE_PTR, location, IO_SEEK_SET);
    if (result == MQX_OK)
    {
        result = write(((PART_MGR_STRUCT_PTR)fd_ptr->DEV_PTR->DRIVER_INIT_PTR)->DEV_FILE_PTR, data_ptr, num);
    }
    fd_ptr->ERROR = pm_struct_ptr->DEV_FILE_PTR->ERROR;

    /* Unlock */
    PMGR_UNLOCK(pm_struct_ptr);

    return result;
}


/*!
 * \brief Executes ioctl commands.
 *
 * \param[in] fd_ptr The stream to perform the operation on.
 * \param[in] cmd The ioctl command.
 * \param[in] param_ptr The ioctl parameters.
 *
 * \return int32_t Int32_t or error code.
 */
int32_t _io_part_mgr_ioctl(
    MQX_FILE_PTR fd_ptr,
    uint32_t cmd,
    uint32_t *param_ptr)
{
    PART_MGR_STRUCT_PTR pm_struct_ptr;
    PMGR_PART_INFO_STRUCT_PTR part_info_ptr;
    uint32_t result = MQX_OK;

    pm_struct_ptr = (PART_MGR_STRUCT_PTR)fd_ptr->DEV_PTR->DRIVER_INIT_PTR;
    part_info_ptr = (PMGR_PART_INFO_STRUCT_PTR)fd_ptr->DEV_DATA_PTR;

    switch (cmd)
    {

        case IO_IOCTL_GET_BLOCK_SIZE:
            *param_ptr = pm_struct_ptr->DEV_SECTOR_SIZE;
            break;

        case IO_IOCTL_GET_NUM_SECTORS:
            *param_ptr = part_info_ptr->LENGTH;
            break;

        case IO_IOCTL_VAL_PART:
            if (param_ptr == NULL)
            {
                /* Validate current partition */
                if (part_info_ptr->START_SECTOR == 0 || part_info_ptr->LENGTH == 0)
                    result = PMGR_INVALID_PARTITION;
                else
                    result = _pmgr_check_part_type(part_info_ptr->TYPE);
            }
            else if (part_info_ptr->SLOT)
            {
                /* Available only if operating on whole device */
                result = MQX_IO_OPERATION_NOT_AVAILABLE;
                break;
            }
            else
            {
                PMGR_PART_INFO_STRUCT part_info;
                _mem_zero(&part_info, sizeof(PMGR_PART_INFO_STRUCT));
                /* In case partition number is 0 the _pmgr_get_part_info function performs just MBR validation */
                part_info.SLOT = *param_ptr;
                PMGR_LOCK(pm_struct_ptr);
                result = _pmgr_get_part_info(pm_struct_ptr, &part_info);
                PMGR_UNLOCK(pm_struct_ptr);
                if (!result && part_info.SLOT)
                {
                    result = _pmgr_check_part_type(part_info.TYPE);
                }
            }
            break;

        case IO_IOCTL_SEL_PART:
            if (*param_ptr && part_info_ptr->SLOT)
            {
                /*
                 * Available only if operating on whole device.
                 * This prevents selecting partition multiple times through a single handle,
                 * e.g. if the handle is "recycled" for mutliple MFS instances, which is wrong.
                 * To select a different partition, partition 0 (whole device) has to be selected first
                 */
                result = MQX_IO_OPERATION_NOT_AVAILABLE;
                break;
            }
            _mem_zero(part_info_ptr, sizeof(PMGR_PART_INFO_STRUCT));
            if (*param_ptr == 0)
            {
                /* Use size of device as length of the partition */
                part_info_ptr->LENGTH = pm_struct_ptr->DEV_NUM_SECTORS;
            }
            else
            {
                part_info_ptr->SLOT = *param_ptr;
                PMGR_LOCK(pm_struct_ptr);
                result = _pmgr_get_part_info(pm_struct_ptr, part_info_ptr);
                PMGR_UNLOCK(pm_struct_ptr);
            }
            /* Reset location to the beginning of the partition/device */
            fd_ptr->LOCATION = 0;
            break;

        case IO_IOCTL_GET_PARTITION:
            if (((PMGR_PART_INFO_STRUCT_PTR)param_ptr)->SLOT == 0)
            {
                /* Get info about current partition */
                _mem_copy(part_info_ptr, param_ptr, sizeof(PMGR_PART_INFO_STRUCT));
                break;
            }
            if (part_info_ptr->SLOT)
            {
                /* Available only if operating on whole device */
                result = MQX_IO_OPERATION_NOT_AVAILABLE;
                break;
            }
            PMGR_LOCK(pm_struct_ptr);
            result = _pmgr_get_part_info(pm_struct_ptr, (PMGR_PART_INFO_STRUCT_PTR)param_ptr);
            PMGR_UNLOCK(pm_struct_ptr);
            break;

        case IO_IOCTL_SET_PARTITION:
            if (part_info_ptr->SLOT)
            {
                /* Available only if operating on whole device */
                result = MQX_IO_OPERATION_NOT_AVAILABLE;
                break;
            }
            PMGR_LOCK(pm_struct_ptr);
            if (pm_struct_ptr->INSTANCES > 1)
            {
                /* Available if this is the only instance to avoid inconsistencies */
                result = IO_ERROR_DEVICE_BUSY;
            }
            else
            {
                result = _pmgr_set_part_info(pm_struct_ptr, (PMGR_PART_INFO_STRUCT_PTR)param_ptr);
            }
            PMGR_UNLOCK(pm_struct_ptr);
            break;

        case IO_IOCTL_CLEAR_PARTITION:
            if (part_info_ptr->SLOT)
            {
                /* Available only if operating on whole device */
                result = MQX_IO_OPERATION_NOT_AVAILABLE;
                break;
            }
            PMGR_LOCK(pm_struct_ptr);
            if (pm_struct_ptr->INSTANCES > 1)
            {
                /* Available if this is the only instance to avoid inconsistencies */
                result = IO_ERROR_DEVICE_BUSY;
            }
            else
            {
                result = _pmgr_clear_part_info(pm_struct_ptr, *param_ptr);
            }
            PMGR_UNLOCK(pm_struct_ptr);
            break;

        case IO_IOCTL_USE_PARTITION:
            /*
             * Force switching to area defined by submitted PMGR_PART_INFO_STRUCT,
             * without storing anything to the partition table on the device.
             * This may be useful to divide the device to areas defined elsewhere.
             */
            if (part_info_ptr->SLOT)
            {
                /* Available only if operating on whole device */
                result = MQX_IO_OPERATION_NOT_AVAILABLE;
                break;
            }
            _mem_copy(param_ptr, part_info_ptr, sizeof(PMGR_PART_INFO_STRUCT));
            /* Reset location to the beginning of the partition/device */
            fd_ptr->LOCATION = 0;

            break;

        default:
            /* Pass IOCTL command to lower layer */
            result = ioctl(pm_struct_ptr->DEV_FILE_PTR, cmd, param_ptr);
            break;
    }

    fd_ptr->ERROR = pm_struct_ptr->DEV_FILE_PTR->ERROR;

    return result;
}


/*!
 * \brief Checks partition type, returns success if it matches one of the FAT types.
 *
 * \param[in] part_type
 *
 * \return int32_t Error code.
 */
int32_t _pmgr_check_part_type(
    uint32_t part_type)
{
    switch (part_type)
    {
        case PMGR_PARTITION_NOT_USED:
            return PMGR_INVALID_PARTITION;

        case PMGR_PARTITION_FAT_12_BIT:
        case PMGR_PARTITION_FAT_16_BIT:
        case PMGR_PARTITION_HUGE:
        case PMGR_PARTITION_HUGE_LBA:
        case PMGR_PARTITION_FAT32:
        case PMGR_PARTITION_FAT32_LBA:
            break;

        default:
            return PMGR_UNKNOWN_PARTITION;
    }
    return MQX_OK;
}


/*!
 * \brief Validates MBR, checks signature and sanity of the partition table.
 *
 * \param mbr_ptr
 *
 * \return int32_t Error code.
 */
int32_t _pmgr_validate_mbr(
    char *mbr_ptr)
{
    int i, j;
    PMGR_PART_ENTRY_STRUCT_PTR disk_entry;

    uint32_t part_start[PMGR_MAX_PARTITIONS];
    uint32_t part_length[PMGR_MAX_PARTITIONS];

    /* Check partition table signature bytes */
    if (((uint8_t *)mbr_ptr)[510] != 0x55 || ((uint8_t *)mbr_ptr)[511] != 0xAA)
        return PMGR_INVALID_PARTTABLE;

    disk_entry = (PMGR_PART_ENTRY_STRUCT_PTR)(((uint8_t *)mbr_ptr) + PMGR_PARTITION_TABLE_OFFSET);

    for (i = 0; i < PMGR_MAX_PARTITIONS; i++)
    {
        part_start[i] = mqx_dtohl(disk_entry[i].START_SECTOR);
        part_length[i] = mqx_dtohl(disk_entry[i].LENGTH);

        /* If type is zero, the whole partition entry should be unused (zero) */
        if ((disk_entry[i].TYPE == 0) && (part_length[i] != 0 || part_start[i] != 0))
            return PMGR_INVALID_PARTTABLE;

        /* If type is non-zero, the partition start and length shall contain meaningfull values */
        if ((disk_entry[i].TYPE != 0) && (part_length[i] == 0 || part_start[i] == 0))
            return PMGR_INVALID_PARTTABLE;

        if (part_length[i] == 0)
            continue; /* No overlap check necessary, go for next record */

        /* Check for overlaps */
        for (j = 0; j < i; j++)
        {
            if (part_length[j] == 0)
                continue; /* No overlap check necessary, go for next record */

            /*
             * Two partitions do NOT overlap if: start of one is behind end of the other OR vice versa.
             * The condition below is negation of this sentence.
             */
            if ((part_start[i] < part_start[j] + part_length[j]) && (part_start[i] + part_length[i] > part_start[j]))
                return PMGR_INVALID_PARTTABLE;
        }
    }

    return MQX_OK;
}


/*!
 * \brief Read the partition table entry from disk.
 *
 * \param[in] pm_struct_ptr The partition manager information.
 * \param[in,out] host_entry The entry to read from disk.
 *
 * \return int32_t Error code.
 */
int32_t _pmgr_get_part_info(
    PART_MGR_STRUCT_PTR pm_struct_ptr,
    PMGR_PART_INFO_STRUCT_PTR host_entry)
{
    int32_t error_code = MQX_OK;
    char *mbr_ptr;
    int32_t size;
    PMGR_PART_ENTRY_STRUCT_PTR disk_entry;

    mbr_ptr = MFS_mem_alloc_system_align(pm_struct_ptr->DEV_SECTOR_SIZE, pm_struct_ptr->ALIGNMENT);
    if (mbr_ptr == NULL)
    {
        return MQX_OUT_OF_MEMORY;
    }
    _mem_set_type(mbr_ptr, MEM_TYPE_PART_MGR_SECTOR);

    size = pm_struct_ptr->BLOCK_MODE ? 1 : pm_struct_ptr->DEV_SECTOR_SIZE;

    fseek(pm_struct_ptr->DEV_FILE_PTR, 0, IO_SEEK_SET);
    if (read(pm_struct_ptr->DEV_FILE_PTR, mbr_ptr, size) != size)
    {
        error_code = pm_struct_ptr->DEV_FILE_PTR->ERROR;
    }
    else
    {
        error_code = _pmgr_validate_mbr(mbr_ptr);
    }

    if (!error_code && host_entry && host_entry->SLOT)
    {
        if (host_entry->SLOT > PMGR_MAX_PARTITIONS)
        {
            error_code = PMGR_INVALID_PARTITION;
        }
        else
        {
            disk_entry = (PMGR_PART_ENTRY_STRUCT_PTR)(mbr_ptr + PMGR_PARTITION_TABLE_OFFSET);
            _pmgr_disk_to_host(disk_entry + host_entry->SLOT - 1, host_entry);
            if (host_entry->TYPE == 0 || host_entry->START_SECTOR == 0 || host_entry->LENGTH == 0)
            {
                error_code = PMGR_INVALID_PARTITION;
            }
        }
    }

    MFS_mem_free(mbr_ptr);
    return error_code;
}


/*!
 * \brief Will overwrite the partition table entry.
 *
 * Will overwrite the partition table entry with the new information
 * received (if opcode == 0). If the opcode is a partition number, this
 * function will clear that partition. Will return error code upon error.
 *
 * \param[in] pm_struct_ptr The partition manager information.
 * \param[in] host_entry The entry to write to disk.
 *
 * \return int32_t Error code.
 */
int32_t _pmgr_set_part_info(
    PART_MGR_STRUCT_PTR pm_struct_ptr,
    PMGR_PART_INFO_STRUCT_PTR host_entry)
{
    int32_t error_code = MQX_OK;
    char *mbr_ptr;
    int32_t size;
    PMGR_PART_ENTRY_STRUCT_PTR disk_entry;

    if (!host_entry || host_entry->SLOT == 0 || host_entry->SLOT > PMGR_MAX_PARTITIONS)
    {
        return PMGR_INVALID_PARTITION;
    }

    mbr_ptr = MFS_mem_alloc_system_align(pm_struct_ptr->DEV_SECTOR_SIZE, pm_struct_ptr->ALIGNMENT);
    if (mbr_ptr == NULL)
    {
        return MQX_OUT_OF_MEMORY;
    }
    _mem_set_type(mbr_ptr, MEM_TYPE_PART_MGR_SECTOR);

    size = pm_struct_ptr->BLOCK_MODE ? 1 : pm_struct_ptr->DEV_SECTOR_SIZE;

    fseek(pm_struct_ptr->DEV_FILE_PTR, 0, IO_SEEK_SET);
    if (read(pm_struct_ptr->DEV_FILE_PTR, mbr_ptr, size) != size)
    {
        error_code = pm_struct_ptr->DEV_FILE_PTR->ERROR;
    }
    else
    {
        error_code = _pmgr_validate_mbr(mbr_ptr);
    }

    if (!error_code)
    {
        disk_entry = (PMGR_PART_ENTRY_STRUCT_PTR)(mbr_ptr + PMGR_PARTITION_TABLE_OFFSET);
        _pmgr_host_to_disk(host_entry, disk_entry + host_entry->SLOT - 1);
        error_code = _pmgr_validate_mbr(mbr_ptr); /* Check sanity of modified MBR */
        if (!error_code)
        {
            fseek(pm_struct_ptr->DEV_FILE_PTR, 0, IO_SEEK_SET);
            if (write(pm_struct_ptr->DEV_FILE_PTR, mbr_ptr, size) != size)
            {
                error_code = pm_struct_ptr->DEV_FILE_PTR->ERROR;
            }
        }
    }

    MFS_mem_free(mbr_ptr);
    return error_code;
}


/*!
 * \brief Clears partition information for given partition.
 *
 * \param[in] pm_struct_ptr The partition manager information.
 * \param[in] part_num The partition to clear.
 *
 * \return int32_t Error code.
 */
int32_t _pmgr_clear_part_info(
    PART_MGR_STRUCT_PTR pm_struct_ptr,
    unsigned char part_num)
{
    int32_t error_code = MQX_OK;
    char *mbr_ptr;
    int32_t size;
    PMGR_PART_ENTRY_STRUCT_PTR disk_entry;

    if (part_num > PMGR_MAX_PARTITIONS)
    {
        return PMGR_INVALID_PARTITION;
    }

    mbr_ptr = MFS_mem_alloc_system_align(pm_struct_ptr->DEV_SECTOR_SIZE, pm_struct_ptr->ALIGNMENT);
    if (mbr_ptr == NULL)
    {
        return MQX_OUT_OF_MEMORY;
    }
    _mem_set_type(mbr_ptr, MEM_TYPE_PART_MGR_SECTOR);

    size = pm_struct_ptr->BLOCK_MODE ? 1 : pm_struct_ptr->DEV_SECTOR_SIZE;

    fseek(pm_struct_ptr->DEV_FILE_PTR, 0, IO_SEEK_SET);
    if (read(pm_struct_ptr->DEV_FILE_PTR, mbr_ptr, size) != size)
    {
        error_code = pm_struct_ptr->DEV_FILE_PTR->ERROR;
    }
    else
    {
        error_code = _pmgr_validate_mbr(mbr_ptr);
    }

    if (error_code == PMGR_INVALID_PARTITION && part_num == 0)
    {
        /* Create blank MBR */
        _mem_zero(mbr_ptr, pm_struct_ptr->DEV_SECTOR_SIZE);
        mbr_ptr[510] = 0x55;
        mbr_ptr[511] = 0xAA;
        error_code = 0;
    }
    else if (!error_code)
    {
        disk_entry = (PMGR_PART_ENTRY_STRUCT_PTR)(mbr_ptr + PMGR_PARTITION_TABLE_OFFSET);
        if (!part_num)
        {
            _mem_zero(disk_entry, PMGR_MAX_PARTITIONS * sizeof(PMGR_PART_ENTRY_STRUCT));
        }
        else
        {
            _mem_zero(disk_entry + part_num - 1, sizeof(PMGR_PART_ENTRY_STRUCT));
        }
    }

    if (!error_code)
    {
        fseek(pm_struct_ptr->DEV_FILE_PTR, 0, IO_SEEK_SET);
        if (write(pm_struct_ptr->DEV_FILE_PTR, mbr_ptr, size) != size)
        {
            error_code = pm_struct_ptr->DEV_FILE_PTR->ERROR;
        }
    }

    MFS_mem_free(mbr_ptr);
    return error_code;
}


/*!
 * \brief Will copy a partition table entry from its disk format to a host format.
 *
 * \param[in] disk_entry Ptr to a partition table entry in disk format.
 * \param[out] part_entry Ptr to a partition table entry in a more usable format.
 *
 * \return void
 */
void _pmgr_disk_to_host(
    PMGR_PART_ENTRY_STRUCT_PTR disk_entry,
    PMGR_PART_INFO_STRUCT_PTR part_entry)
{
    part_entry->TYPE = disk_entry->TYPE;
    part_entry->ACTIVE_FLAG = disk_entry->ACTIVE_FLAG;
    part_entry->START_SECTOR = mqx_dtohl(disk_entry->START_SECTOR);
    part_entry->LENGTH = mqx_dtohl(disk_entry->LENGTH);
    part_entry->HEADS = 0;
    part_entry->SECTORS = 0;
    part_entry->CYLINDERS = 0;
}


/*!
 * \brief Will copy a partition table entry from its host format to the disk format.
 *
 * \param[in] part_entry Ptr to a partition table entry in a usable format.
 * \param[out] disk_entry Ptr to a partition table entry in disk format.
 *
 * \return void
 */
void _pmgr_host_to_disk(
    PMGR_PART_INFO_STRUCT_PTR part_entry,
    PMGR_PART_ENTRY_STRUCT_PTR disk_entry)
{
    uint32_t temp, cyl, head, sec, hds_cyl, sct_trk, lba;

    disk_entry->ACTIVE_FLAG = 0;
    disk_entry->TYPE = part_entry->TYPE;
    mqx_htodl(disk_entry->START_SECTOR, part_entry->START_SECTOR);
    mqx_htodl(disk_entry->LENGTH, part_entry->LENGTH);

    /* Check if CHS is present and valid */
    if (!part_entry->CYLINDERS || !part_entry->HEADS || !part_entry->SECTORS || part_entry->CYLINDERS > 1024 || part_entry->SECTORS > 63)
    {
        disk_entry->START_HEAD = 0;
        disk_entry->START_SECT_CYL = 0;
        disk_entry->START_CYLINDER = 0;
        disk_entry->ENDING_HEAD = 0;
        disk_entry->ENDING_SECT_CYL = 0;
        disk_entry->ENDING_CYLINDER = 0;
    }
    else
    {
        /* setup info used for calculations */
        sct_trk = part_entry->SECTORS;
        hds_cyl = part_entry->HEADS;

        /* Calculate starting CHS */
        lba = part_entry->START_SECTOR;
        cyl = lba / (hds_cyl * sct_trk);
        temp = lba % (hds_cyl * sct_trk);
        head = temp / sct_trk;
        sec = temp % sct_trk + 1;

        if (cyl > 1023)
        {
            /* Indicate invalid CHS entry */
            disk_entry->START_HEAD = 254;
            disk_entry->START_SECT_CYL = 0xFF;
            disk_entry->START_CYLINDER = 0xFF;
        }
        else
        {
            disk_entry->START_HEAD = (unsigned char)head;
            disk_entry->START_SECT_CYL = (unsigned char)((sec & 0x3f) + ((cyl & 0x0300) >> 2));
            disk_entry->START_CYLINDER = (unsigned char)(cyl & 0x0FF);
        }

        /* Calculate the ending CHS */
        lba = part_entry->LENGTH + part_entry->START_SECTOR;
        cyl = lba / (hds_cyl * sct_trk);
        temp = lba % (hds_cyl * sct_trk);
        head = temp / sct_trk;
        sec = temp % sct_trk + 1;

        if (cyl > 1023)
        {
            /* Indicate invalid CHS entry */
            disk_entry->ENDING_HEAD = 254;
            disk_entry->ENDING_SECT_CYL = 0xFF;
            disk_entry->ENDING_CYLINDER = 0xFF;
        }
        else
        {
            disk_entry->ENDING_HEAD = (unsigned char)head;
            disk_entry->ENDING_SECT_CYL = (unsigned char)((sec & 0x3f) + ((cyl & 0x0300) >> 2));
            disk_entry->ENDING_CYLINDER = (unsigned char)(cyl & 0x0FF);
        }
    }
}
