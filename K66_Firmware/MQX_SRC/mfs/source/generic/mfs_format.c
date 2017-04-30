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
*   This file contains drive specific interface functions related to
*   formating the drive.
*
*
*END************************************************************************/

#include "mfs.h"
#include "mfs_prv.h"

#if !MFSCFG_READ_ONLY
#ifdef MFSCFG_ENABLE_FORMAT


/*
** Number of data clusters has to be less than 4085 for FAT12 and less than 65525 for FAT16.
** Sector count boundaries are chosen to meet this constraint avoiding corner cases.
*/

struct
{
    uint32_t SECTOR_BOUND;
    uint32_t FAT_TYPE;
    uint32_t ROOTDIR_SIZE;
    uint32_t CLUSTER_SIZE;
} static const format_table[] = {
    /* sector count boundary ~ approximate capacity for 512B sectors */
    { 128, MFS_FAT12, 2, 1 }, /* minimal root directory for small ramdisks */
    { 2048, MFS_FAT12, 8, 1 },
    { 4096, MFS_FAT12, 14, 1 },
    { 8192, MFS_FAT12, 32, 2 },
    { 16384, MFS_FAT12, 32, 4 },
    { 32640, MFS_FAT12, 32, 8 }, /* 2**15-128 ~ up to  16 MB */
    { 262080, MFS_FAT16, 32, 4 }, /* 2**18-64  ~ up to 128 MB */
    { 524160, MFS_FAT16, 32, 8 }, /* 2**19-128 ~ up to 256 MB */
    { 1048320, MFS_FAT16, 32, 16 }, /* 2**20-256 ~ up to 512 MB */
    { 2096640, MFS_FAT16, 32, 32 }, /* 2**21-512 ~ up to   1 GB */
    { 16777216, MFS_FAT32, 0, 8 },
    { 33554432, MFS_FAT32, 0, 16 },
    { 67108864, MFS_FAT32, 0, 32 },
    { 0, MFS_FAT32, 0, 64 } /* any larger media */
};


/*!
 * \brief Perform a high-level DOS format.
 *
 * \param[in] format_ptr Information about the disk to format.
 *
 * \return _mfs_error Error code.
 */
_mfs_error MFS_Format(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    MFS_FORMAT_DATA_PTR format_ptr)
{
    FILESYSTEM_INFO_DISK_PTR fsinfo_ptr;
    uint8_t *boot_sector;
    _mfs_error error_code;
    uint32_t fat_type;
    uint32_t i;
    uint32_t temp;
    uint32_t reserved_sectors;
    uint32_t available_sectors;
    uint32_t fat_data_sectors;
    uint32_t fat_size;
    uint32_t rootdir_size;
    uint32_t sys_size;
    int32_t fat_count;
    unsigned char cluster_size;
    char version_str[6];

    if (MFS_is_read_only(drive_ptr))
    {
        return MFS_DISK_IS_WRITE_PROTECTED;
    }

    if (format_ptr->NUMBER_OF_SECTORS <= format_ptr->RESERVED_SECTORS)
    {
        return MFS_INVALID_PARAMETER;
    }

    error_code = MFS_lock(drive_ptr);
    if (error_code != MFS_NO_ERROR)
    {
        return error_code;
    }

    if (!_queue_is_empty(&drive_ptr->HANDLE_LIST))
    {
        MFS_unlock(drive_ptr);
        return MFS_SHARING_VIOLATION;
    }

    boot_sector = (uint8_t *)MFS_mem_alloc_system_align(drive_ptr->SECTOR_SIZE, drive_ptr->ALIGNMENT);
    if (boot_sector == NULL)
    {
        MFS_unlock(drive_ptr);
        return MFS_INSUFFICIENT_MEMORY;
    }
    _mem_set_type(boot_sector, MEM_TYPE_MFS_DATA_SECTOR);

    /*
    ** We are reformatting the disk, so mark it as not accessible.
    ** ie: it's not a dos disk during the format.
    */
    drive_ptr->DOS_DISK = false;

    i = 0;
    while (0 != format_table[i].SECTOR_BOUND)
    {
        if (format_ptr->NUMBER_OF_SECTORS - format_ptr->RESERVED_SECTORS < format_table[i].SECTOR_BOUND)
        {
            break;
        }
        i++;
    }

    fat_type = format_table[i].FAT_TYPE;
    rootdir_size = format_table[i].ROOTDIR_SIZE;
    cluster_size = format_table[i].CLUSTER_SIZE;

    /* Default value for reserved sectors: For FAT12, FAT16 it should be 1, for FAT32 it is usualy 32 */
    if (0 == format_ptr->RESERVED_SECTORS)
    {
        reserved_sectors = (fat_type == MFS_FAT32) ? 32 : 1;
    }
    else
    {
        reserved_sectors = format_ptr->RESERVED_SECTORS;
    }

    /* Number of sectors excluding reserved ones */
    available_sectors = format_ptr->NUMBER_OF_SECTORS - reserved_sectors;

    /*
    ** Check minimal number reserverd sectors for FAT32
    */
    if ((fat_type == MFS_FAT32) && (reserved_sectors < 32))
    {
        MFS_mem_free(boot_sector);
        MFS_unlock(drive_ptr);
        return (MFS_INVALID_PARAMETER);
    }

    /*
    ** Make sure the format parameters are big enough to initialize file system
    */
    if (available_sectors <= rootdir_size)
    {
        MFS_mem_free(boot_sector);
        MFS_unlock(drive_ptr);
        return (MFS_INVALID_PARAMETER);
    }


    drive_ptr->MEGA_SECTORS = format_ptr->NUMBER_OF_SECTORS;


    /*
    ** The formula is:
    **
    **                       / cluster_size * sector_size     \
    **   fat_data_sectors = (  -------------------------- + 2  ) * fat_size
    **                       \         (1.5 or 2)             /
    **
    */
    if (fat_type == MFS_FAT12)
    {
        fat_data_sectors = available_sectors - rootdir_size;
        temp = 2 * format_ptr->BYTES_PER_SECTOR * cluster_size + 6;
        fat_size = 1 + (3 * fat_data_sectors - 1) / temp;
    }
    else if (fat_type == MFS_FAT16)
    {
        fat_data_sectors = available_sectors - rootdir_size;
        temp = format_ptr->BYTES_PER_SECTOR * cluster_size / 2 + 2;
        fat_size = 1 + (fat_data_sectors - 1) / temp;
    }
    else
    {
        temp = (256 * cluster_size + MFSCFG_NUM_OF_FATS) / 2;
        fat_size = (available_sectors + temp - 1) / temp;
    }

    _mem_zero(boot_sector, format_ptr->BYTES_PER_SECTOR);
    sys_size = rootdir_size + MFSCFG_NUM_OF_FATS * fat_size + reserved_sectors;
    for (i = reserved_sectors + 1; i < sys_size; i++)
    {
        error_code = MFS_Write_device_sector(drive_ptr, i, (char *)boot_sector);
        if (error_code)
        {
            MFS_mem_free(boot_sector);
            MFS_unlock(drive_ptr);
            return (error_code);
        }
    }


    /* If we have a fat32 drive, we must clear the root dir cluster */
    if (fat_type == MFS_FAT32)
    {
        for (i = 0; i < cluster_size && !error_code; i++)
        {
            error_code = MFS_Write_device_sector(drive_ptr, sys_size + i, (char *)boot_sector);
        }
        if (error_code)
        {
            MFS_mem_free(boot_sector);
            MFS_unlock(drive_ptr);
            return (error_code);
        }
    }


    /* now write the first sector of each FAT */
    boot_sector[0] = format_ptr->MEDIA_DESCRIPTOR;
    boot_sector[1] = 0xFF;
    boot_sector[2] = 0xFF;

    if (fat_type == MFS_FAT16)
    {
        boot_sector[3] = 0xFF;
    }

    if (fat_type == MFS_FAT32)
    {
        boot_sector[3] = 0x0F;
        boot_sector[4] = 0xFF;
        boot_sector[5] = 0xFF;
        boot_sector[6] = 0xFF;
        boot_sector[7] = 0x0F;

        /* First cluster of root entry */
        boot_sector[8] = 0xFF;
        boot_sector[9] = 0xFF;
        boot_sector[10] = 0xFF;
        boot_sector[11] = 0x0F;
    }

    fat_count = MFSCFG_NUM_OF_FATS;
    while (--fat_count >= 0)
    {
        error_code = MFS_Write_device_sector(drive_ptr, reserved_sectors + (uint32_t)fat_count * fat_size, (char *)boot_sector);

        if (error_code)
        {
            MFS_mem_free(boot_sector);
            MFS_unlock(drive_ptr);
            return (error_code);
        }
    } /* EndWhile */

    boot_sector[0] = 0xEB; /* jmp boot_sector[62] */
    boot_sector[1] = 0x3C;
    boot_sector[2] = 0x90; /* nop */

    /* Write MFS and version number as OEM name */
    sprintf(version_str, "%x", MFS_VERSION);
    _mem_copy("MFS", boot_sector + 3, 3);
    _mem_copy(version_str, boot_sector + 6, 5);

    boot_sector[11] = (unsigned char)((format_ptr->BYTES_PER_SECTOR) & 0xFF);
    boot_sector[12] = (unsigned char)((format_ptr->BYTES_PER_SECTOR >> 8) & 0xFF);

    boot_sector[13] = cluster_size;

    boot_sector[14] = (unsigned char)((reserved_sectors)&0xFF);
    boot_sector[15] = (unsigned char)((reserved_sectors >> 8) & 0xFF);

    boot_sector[16] = MFSCFG_NUM_OF_FATS; /* number of FATs */

    rootdir_size *= format_ptr->BYTES_PER_SECTOR >> 5;
    boot_sector[17] = (unsigned char)((rootdir_size)&0xFF);
    boot_sector[18] = (unsigned char)((rootdir_size >> 8) & 0xFF);

    if ((format_ptr->NUMBER_OF_SECTORS + format_ptr->HIDDEN_SECTORS) <= MAX_UINT_16)
    {
        boot_sector[19] = (unsigned char)((format_ptr->NUMBER_OF_SECTORS) & 0xFF);
        boot_sector[20] = (unsigned char)((format_ptr->NUMBER_OF_SECTORS >> 8) & 0xFF);
    }
    else
    {
        /* boot_sector[19..20] should already be 0 */
        boot_sector[32] = (unsigned char)((format_ptr->NUMBER_OF_SECTORS) & 0xFF);
        boot_sector[33] = (unsigned char)((format_ptr->NUMBER_OF_SECTORS >> 8) & 0xFF);
        boot_sector[34] = (unsigned char)((format_ptr->NUMBER_OF_SECTORS >> 16) & 0xFF);
        boot_sector[35] = (unsigned char)((format_ptr->NUMBER_OF_SECTORS >> 24) & 0xFF);
    }

    boot_sector[21] = format_ptr->MEDIA_DESCRIPTOR;

    if (fat_type != MFS_FAT32)
    {
        boot_sector[22] = (unsigned char)((fat_size)&0xFF);
        boot_sector[23] = (unsigned char)((fat_size >> 8) & 0xFF);
    }
    else
    {
        boot_sector[36] = (unsigned char)((fat_size)&0xFF);
        boot_sector[37] = (unsigned char)((fat_size >> 8) & 0xFF);
        boot_sector[38] = (unsigned char)((fat_size >> 16) & 0xFF);
        boot_sector[39] = (unsigned char)((fat_size >> 24) & 0xFF);
    }

    boot_sector[24] = (unsigned char)((format_ptr->SECTORS_PER_TRACK) & 0xFF);
    boot_sector[25] = (unsigned char)((format_ptr->SECTORS_PER_TRACK >> 8) & 0xFF);

    boot_sector[26] = (unsigned char)((format_ptr->NUMBER_OF_HEADS) & 0xFF);
    boot_sector[27] = (unsigned char)((format_ptr->NUMBER_OF_HEADS >> 8) & 0xFF);

    boot_sector[28] = (unsigned char)((format_ptr->HIDDEN_SECTORS) & 0xFF);
    boot_sector[29] = (unsigned char)((format_ptr->HIDDEN_SECTORS >> 8) & 0xFF);
    boot_sector[30] = (unsigned char)((format_ptr->HIDDEN_SECTORS >> 16) & 0xFF);
    boot_sector[31] = (unsigned char)((format_ptr->HIDDEN_SECTORS >> 24) & 0xFF);

    if (fat_type == MFS_FAT32)
    {
        /* fields 40 and 41 are for FAT mirroring */
        boot_sector[40] = 0x00;
        boot_sector[41] = 0x00;

        boot_sector[42] = (unsigned char)((MFS_FAT32_VER)&0xFF); /* Minor FAT 32 rev */
        boot_sector[43] = (unsigned char)((MFS_FAT32_VER >> 8) & 0xFF); /* Major rev  */

        /* Root Cluster for FAT32... cluster 2 is the standard */
        boot_sector[44] = 0x02;
        boot_sector[45] = 0x00;
        boot_sector[46] = 0x00;
        boot_sector[47] = 0x00;

        /* BPB_FSINFO struct...  The standard is sector 1 */
        boot_sector[48] = (unsigned char)((FSINFO_SECTOR)&0xFF);
        boot_sector[49] = (unsigned char)((FSINFO_SECTOR >> 8) & 0xFF);

        /* Back up boot sector... The standard is sector 6  */
        boot_sector[50] = (unsigned char)((BKBOOT_SECTOR)&0xFF);
        boot_sector[51] = (unsigned char)((BKBOOT_SECTOR >> 8) & 0xFF);

        /* Fields 52 to 63 are reserved. They are already zeroed */

        boot_sector[64] = format_ptr->PHYSICAL_DRIVE;
        boot_sector[66] = 0x29; /* Extended boot signature */

        /* Volume label and ID */
        _mem_copy("NO NAME    FAT32   ", boot_sector + 71, 19);
    }
    else
    {

        boot_sector[36] = format_ptr->PHYSICAL_DRIVE;

        boot_sector[38] = 0x29; /* Extended boot signature */

        if (fat_type == MFS_FAT12)
        {
            /* Volume label and ID */
            _mem_copy("NO NAME    FAT12   ", boot_sector + 43, 19);
        }
        else if (fat_type == MFS_FAT16)
        {
            /* Volume label and ID */
            _mem_copy("NO NAME    FAT16   ", boot_sector + 43, 19);
        }

        boot_sector[62] = 0x33; /* xor ax, ax */
        boot_sector[63] = 0xC0;
        boot_sector[64] = 0x8E; /* mov ss, ax */
        boot_sector[65] = 0xD0;
        boot_sector[66] = 0xBC; /* mov sp, 7C00 */
        boot_sector[67] = 0x00;
        boot_sector[68] = 0x7C;
        boot_sector[69] = 0xFC; /* cld */
        boot_sector[70] = 0xE8; /* call boot_sector[118] */
        boot_sector[71] = 45;
        boot_sector[72] = 0;

        _mem_copy("\r\nNon-System disk\r\nPress any key to reboot\r\n", boot_sector + 73, 45);

        boot_sector[118] = 0x5E; /* pop si */
        boot_sector[119] = 0xEB; /* jmp boot_sector[123] */
        boot_sector[120] = 0x02;
        boot_sector[121] = 0xCD; /* int 10 */
        boot_sector[122] = 0x10;
        boot_sector[123] = 0xB4; /* mov ah, 0E */
        boot_sector[124] = 0x0E;
        boot_sector[125] = 0xBB; /* mov bx, 0007 */
        boot_sector[126] = 0x07;
        boot_sector[127] = 0x00;
        boot_sector[128] = 0x2E; /* cs:lodsb */
        boot_sector[129] = 0xAC;
        boot_sector[130] = 0x84; /* test al, al */
        boot_sector[131] = 0xC0;
        boot_sector[132] = 0x75; /* jnz boot_sector[121] */
        boot_sector[133] = 0xF3;
        boot_sector[134] = 0x98; /* cbw */
        boot_sector[135] = 0xCD; /* int 16 */
        boot_sector[136] = 0x16;
        boot_sector[137] = 0xCD; /* int 19 */
        boot_sector[138] = 0x19;
        boot_sector[139] = 0xEB; /* jmp boot_sector[62] */
        boot_sector[140] = 0xB1;
    }
    boot_sector[format_ptr->BYTES_PER_SECTOR - 2] = 0x55;
    boot_sector[format_ptr->BYTES_PER_SECTOR - 1] = 0xAA;

    error_code = MFS_Write_device_sector(drive_ptr, BOOT_SECTOR, (char *)boot_sector);

    if ((error_code == MFS_NO_ERROR) && (fat_type == MFS_FAT32))
    {
        error_code = MFS_Write_device_sector(drive_ptr, BKBOOT_SECTOR, (char *)boot_sector);

        if (error_code == MFS_NO_ERROR)
        {
            fsinfo_ptr = (FILESYSTEM_INFO_DISK_PTR)boot_sector;
            mqx_htodl(fsinfo_ptr->LEAD_SIG, FSI_LEADSIG);
            mqx_htodl(fsinfo_ptr->STRUCT_SIG, FSI_STRUCTSIG);
            mqx_htodl(fsinfo_ptr->FREE_COUNT, 0xFFFFFFFF); /* compute it */
            mqx_htodl(fsinfo_ptr->NEXT_FREE, 0xFFFFFFFF); /* compute it */
            mqx_htodl(fsinfo_ptr->TRAIL_SIG, FSI_TRAILSIG);

            error_code = MFS_Write_device_sector(drive_ptr, FSINFO_SECTOR, (char *)fsinfo_ptr);
        }
    }

    MFS_mem_free(boot_sector);

    if (error_code == MFS_NO_ERROR)
    {
        error_code = MFS_Mount_drive_internal(drive_ptr);
    }

    /* Calculate the free space on disk */
    if (error_code == MFS_NO_ERROR)
    {
        error_code = MFS_Get_disk_free_space_internal(drive_ptr, NULL);
    }

    MFS_unlock(drive_ptr);
    return (error_code);
}

/*!
 * \brief Perform a high-level DOS format with default values.
 *
 * \param drive_ptr
 *
 * \return _mfs_error Error code.
 */
_mfs_error MFS_Default_Format(
    MFS_DRIVE_STRUCT_PTR drive_ptr)
{
    MFS_FORMAT_DATA format_data;
    uint32_t sector_size;
    uint32_t num_sectors;
    _mfs_error error_code;

    if (MFS_is_read_only(drive_ptr))
    {
        return MFS_DISK_IS_WRITE_PROTECTED;
    }

    /* lock the drive */
    error_code = MFS_lock(drive_ptr);
    if (error_code != MFS_NO_ERROR)
    {
        return error_code;
    }

    /* sector size is obtained during open operation and stored in the drive structure */
    sector_size = drive_ptr->SECTOR_SIZE;

    /* get the number of sectors */
    error_code = ioctl(drive_ptr->DEV_FILE_PTR, IO_IOCTL_GET_NUM_SECTORS, &num_sectors);
#if !MQX_USE_IO_OLD
    if (error_code == -1 && errno == NIO_ENOTSUP)
    {
        num_sectors = lseek(drive_ptr->DEV_FILE_PTR, 0, SEEK_END) / sector_size;
        error_code = MFS_NO_ERROR;
    }
    if (error_code == -1)
    {
        error_code = errno;
    }
#endif

    /* unlock the drive */
    MFS_unlock(drive_ptr);

    if (error_code == MFS_NO_ERROR)
    {
        /* fill the MFS_FORMAT_DATA structure. This values are used in format command */
        /* this is the default settings */
        format_data.PHYSICAL_DRIVE = 0x80;
        format_data.MEDIA_DESCRIPTOR = 0xf8;
        format_data.BYTES_PER_SECTOR = sector_size;
        format_data.SECTORS_PER_TRACK = 0x00;
        format_data.NUMBER_OF_HEADS = 0x00;
        format_data.NUMBER_OF_SECTORS = num_sectors;
        format_data.HIDDEN_SECTORS = 0;
        format_data.RESERVED_SECTORS = 0;

        /* format drive */
        error_code = MFS_Format(drive_ptr, &format_data);
    }

    return error_code;
}


/*!
 * \brief Perform a sector test.
 *
 * \param drive_ptr
 * \param[in,out] count_ptr Number of bad clusters detected.
 *
 * \return _mfs_error Error code.
 */
_mfs_error MFS_Test_unused_clusters(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    uint32_t *count_ptr)
{
    unsigned char *cluster_buffer;
    uint32_t cluster_status;
    uint32_t cluster_num;
    uint32_t sector_num;
    _mfs_error error_code;
    uint32_t io_error;

    error_code = MFS_lock_and_enter(drive_ptr, MFS_ENTER_READWRITE);
    if (error_code != MFS_NO_ERROR)
    {
        return error_code;
    }

    cluster_buffer = MFS_mem_alloc_system_align(drive_ptr->CLUSTER_SIZE_BYTES, drive_ptr->ALIGNMENT);

    if (cluster_buffer == NULL)
    {
        error_code = MFS_INSUFFICIENT_MEMORY;
        MFS_unlock(drive_ptr);
        return error_code;
    }
    _mem_zero(cluster_buffer, drive_ptr->CLUSTER_SIZE_BYTES);
    _mem_set_type(cluster_buffer, MEM_TYPE_MFS_CLUSTER);

    *count_ptr = 0;

    cluster_num = CLUSTER_MIN_GOOD;
    sector_num = drive_ptr->DATA_START_SECTOR;

    while (cluster_num <= drive_ptr->LAST_CLUSTER && error_code == MFS_NO_ERROR)
    {
        /*
        ** Test all UNUSED clusters (after a format, all clusters are free)
        */

        error_code = MFS_get_cluster_from_fat(drive_ptr, cluster_num, &cluster_status);

        if (error_code == MFS_NO_ERROR && cluster_status == CLUSTER_UNUSED)
        {
            /* Check the cluster by performing write with no retries */
            io_error = MFS_Write_device_sectors(drive_ptr, sector_num, drive_ptr->SECTORS_PER_CLUSTER, 0, (char *)cluster_buffer, NULL);

            switch (io_error)
            {
                case MFS_WRITE_FAULT:
                case MFS_READ_FAULT:
                case MFS_SECTOR_NOT_FOUND:
                    error_code = MFS_Put_fat(drive_ptr, cluster_num, CLUSTER_BAD);
                    (*count_ptr)++;
                    break;
                case MFS_DISK_IS_WRITE_PROTECTED:
                    error_code = MFS_DISK_IS_WRITE_PROTECTED;
                    break;
            }
        }

        cluster_num++;
        sector_num += drive_ptr->SECTORS_PER_CLUSTER;
    }

    _mem_free(cluster_buffer);
    MFS_unlock(drive_ptr);

    return error_code;
}


#endif  //MFSCFG_ENABLE_FORMAT
#endif  //!MFSCFG_READ_ONLY
