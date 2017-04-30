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
*   This file contains the functions that are used to mount/unmount MFS volume
*
*
*END************************************************************************/

#include "mfs.h"
#include "mfs_prv.h"
#include "part_mgr.h"


/*!
 * \brief Used to set the MFS drive parameters for a unit.
 *
 * This function assumes that the boot sector of the drive is stored in
 * the drive's sector buffer.  This function is called after MFS is
 * initialized, or after the drive has been formatted.
 *
 * NOTE: It is assumed that the drive is locked by the calling function.
 *
 * \param drive_ptr
 *
 * \return uint32_t Error code.
 */
uint32_t MFS_Mount_drive_internal(
    MFS_DRIVE_STRUCT_PTR drive_ptr)
{
    BIOS_PARAM_STRUCT_DISK_PTR bpb_ptr;
    BIOS_PARAM32_STRUCT_DISK_PTR bpb32_ptr;
    FILESYSTEM_INFO_DISK_PTR fsinfo_ptr;

    uint32_t reserved_sectors;
    uint32_t root_dir_sectors;
    uint32_t data_sectors;
    uint32_t cluster_count;

    uint32_t bpb_sector_size;
    uint32_t bpb_sector_mult;

    int error_code;
    int result = MFS_NO_ERROR;

    uint8_t *boot_sector;

    drive_ptr->DOS_DISK = false;

    error_code = MFS_sector_cache_invalidate(drive_ptr, 0, 0);
    if (error_code != MFS_NO_ERROR)
    {
        return error_code;
    }

    error_code = MFS_sector_map(drive_ptr, BOOT_SECTOR, (void **)&boot_sector, MFS_MAP_MODE_READONLY, 0);
    if (error_code != MFS_NO_ERROR)
    {
        return error_code;
    }

    /*
    ** Extract the drive parameters (BIOS Parameter Block) from the BOOT Record.
    */
    bpb_ptr = (BIOS_PARAM_STRUCT_DISK_PTR)boot_sector;
    bpb32_ptr = (BIOS_PARAM32_STRUCT_DISK_PTR)(boot_sector + sizeof(BIOS_PARAM_STRUCT_DISK));

    /*
    ** Next, check  to see that the BOOT record is that of a DOS disk.  If  not,
    ** the drive will have to be formatted by the upper layer before the drive
    ** can be 'mounted'.
    */
    if ((boot_sector[0] != MFS_DOS30_JMP) && (boot_sector[0] != MFS_DOS30_B))
    {
        result = MFS_NOT_A_DOS_DISK;
    }

    if (result == MFS_NO_ERROR)
    {
        /*
        ** Always use storage device sector size.
        ** If BPB sector size is larger, then recalculate other parameters accordingly.
        ** In any case, BPB sector size has to be multiple of device sector size, the code explicitly checks this.
        */
        bpb_sector_size = mqx_dtohs(bpb_ptr->SECTOR_SIZE);
        if (bpb_sector_size % drive_ptr->SECTOR_SIZE)
        {
            result = MFS_NOT_A_DOS_DISK;
        }
    }

    if (result == MFS_NO_ERROR)
    {
        /* Sector values from BPB are to be multiplied by this factor */
        bpb_sector_mult = bpb_sector_size / drive_ptr->SECTOR_SIZE;

        reserved_sectors = mqx_dtohs(bpb_ptr->RESERVED_SECTORS) * bpb_sector_mult;

        drive_ptr->SECTORS_PER_CLUSTER = mqx_dtohc(bpb_ptr->SECTORS_PER_CLUSTER) * bpb_sector_mult;
        drive_ptr->CLUSTER_POWER_SECTORS = ilog2(drive_ptr->SECTORS_PER_CLUSTER);
        drive_ptr->CLUSTER_POWER_BYTES = drive_ptr->SECTOR_POWER + drive_ptr->CLUSTER_POWER_SECTORS;
        drive_ptr->CLUSTER_SIZE_BYTES = drive_ptr->SECTOR_SIZE * drive_ptr->SECTORS_PER_CLUSTER;

        drive_ptr->NUMBER_OF_FAT = mqx_dtohc(bpb_ptr->NUMBER_OF_FAT);
        drive_ptr->ROOT_ENTRIES = mqx_dtohs(bpb_ptr->ROOT_ENTRIES);

        drive_ptr->SECTORS_PER_FAT = mqx_dtohs(bpb_ptr->SECTORS_PER_FAT);
        if (drive_ptr->SECTORS_PER_FAT == 0)
        {
            drive_ptr->SECTORS_PER_FAT = mqx_dtohl(bpb32_ptr->FAT_SIZE);
        }
        drive_ptr->SECTORS_PER_FAT *= bpb_sector_mult;

        drive_ptr->MEGA_SECTORS = mqx_dtohs(bpb_ptr->NUMBER_SECTORS);
        if (drive_ptr->MEGA_SECTORS == 0)
        {
            drive_ptr->MEGA_SECTORS = mqx_dtohl(bpb_ptr->MEGA_SECTORS);
        }
        drive_ptr->MEGA_SECTORS *= bpb_sector_mult;

        /* Determine FAT type by calculating the count of clusters on disk */
        drive_ptr->ENTRIES_PER_SECTOR = drive_ptr->SECTOR_SIZE / sizeof(DIR_ENTRY_DISK);
        root_dir_sectors = drive_ptr->ROOT_ENTRIES / drive_ptr->ENTRIES_PER_SECTOR;

        data_sectors = drive_ptr->MEGA_SECTORS - reserved_sectors - root_dir_sectors - (drive_ptr->NUMBER_OF_FAT * drive_ptr->SECTORS_PER_FAT);
        cluster_count = data_sectors / drive_ptr->SECTORS_PER_CLUSTER;

        /* Now we have cluster count, so we can determine FAT type */
        if (cluster_count < 4085)
        {
            drive_ptr->FAT_TYPE = MFS_FAT12;
        }
        else if (cluster_count < 65525)
        {
            drive_ptr->FAT_TYPE = MFS_FAT16;
        }
        else
        {
            drive_ptr->FAT_TYPE = MFS_FAT32;
        }

        drive_ptr->CLUSTER_SIZE_BYTES = drive_ptr->SECTOR_SIZE * drive_ptr->SECTORS_PER_CLUSTER;
        drive_ptr->CLUSTER_POWER_BYTES = drive_ptr->SECTOR_POWER + drive_ptr->CLUSTER_POWER_SECTORS;

        drive_ptr->FREE_COUNT = FSI_UNKNOWN; /* This is the unknown value */
        drive_ptr->NEXT_FREE_CLUSTER = FSI_UNKNOWN; /* MFS will calculate it later */

        drive_ptr->FAT_START_SECTOR = reserved_sectors;
        drive_ptr->DATA_START_SECTOR = drive_ptr->FAT_START_SECTOR + (drive_ptr->SECTORS_PER_FAT * drive_ptr->NUMBER_OF_FAT) + root_dir_sectors;

        if (drive_ptr->FAT_TYPE != MFS_FAT32)
        {
            /* FAT12 or FAT16 */
            drive_ptr->ROOT_START_SECTOR = drive_ptr->FAT_START_SECTOR + (drive_ptr->SECTORS_PER_FAT * drive_ptr->NUMBER_OF_FAT);
            drive_ptr->ROOT_CLUSTER = 0;
            MFS_chain_forge(drive_ptr, &drive_ptr->ROOT_CHAIN, drive_ptr->ROOT_START_SECTOR, root_dir_sectors);
        }
        else if (mqx_dtohs(bpb32_ptr->FS_VER) > MFS_FAT32_VER)
        {
            /* Unsupported FAT32 level */
            result = MFS_ERROR_UNKNOWN_FS_VERSION;
        }
        else
        {
            /* Supported FAT32 */
            drive_ptr->ROOT_CLUSTER = mqx_dtohl(bpb32_ptr->ROOT_CLUSTER);
            drive_ptr->ROOT_START_SECTOR = 0;
            MFS_chain_init(drive_ptr, &drive_ptr->ROOT_CHAIN, drive_ptr->ROOT_CLUSTER);

            drive_ptr->FS_INFO = mqx_dtohs(bpb32_ptr->FS_INFO);
        }
    }

    error_code = MFS_sector_unmap(drive_ptr, BOOT_SECTOR, 0);
    if (result == MFS_NO_ERROR)
    {
        result = error_code;
    }

    if (result != MFS_NO_ERROR)
    {
        return result;
    }

    if (drive_ptr->FAT_TYPE == MFS_FAT32)
    {

        /*
        ** Reset the FSInfo->Free_Count and the FSInfo->Next_Free to
        ** unknown (0xFFFFFFFF). MFS uses it's own internal version of these
        ** fields. If Windows uses the same disk, it will recalculate the
        ** correct fields the first time it mounts the drive.
        */

        error_code = MFS_sector_map(drive_ptr, drive_ptr->FS_INFO, (void **)&fsinfo_ptr, MFS_is_read_only(drive_ptr) ? MFS_MAP_MODE_READONLY : MFS_MAP_MODE_MODIFY, 0);
        if (error_code == MFS_NO_ERROR)
        {

            if ((mqx_dtohl(fsinfo_ptr->LEAD_SIG) == FSI_LEADSIG) && (mqx_dtohl(fsinfo_ptr->STRUCT_SIG) == FSI_STRUCTSIG) &&
                (mqx_dtohl(fsinfo_ptr->TRAIL_SIG) == FSI_TRAILSIG))
            {
                drive_ptr->FREE_COUNT = mqx_dtohl(fsinfo_ptr->FREE_COUNT);
                drive_ptr->NEXT_FREE_CLUSTER = mqx_dtohl(fsinfo_ptr->NEXT_FREE);
            }

            if (!MFS_is_read_only(drive_ptr))
            {
                mqx_htodl(fsinfo_ptr->LEAD_SIG, FSI_LEADSIG);
                mqx_htodl(fsinfo_ptr->STRUCT_SIG, FSI_STRUCTSIG);
                mqx_htodl(fsinfo_ptr->FREE_COUNT, FSI_UNKNOWN); /* compute it */
                mqx_htodl(fsinfo_ptr->NEXT_FREE, FSI_UNKNOWN); /* compute it */
                mqx_htodl(fsinfo_ptr->TRAIL_SIG, FSI_TRAILSIG);
            }

            error_code = MFS_sector_unmap(drive_ptr, drive_ptr->FS_INFO, !MFS_is_read_only(drive_ptr));
        }
        if (result == MFS_NO_ERROR)
        {
            result = error_code;
        }
    }

    drive_ptr->LAST_CLUSTER = (drive_ptr->MEGA_SECTORS - drive_ptr->DATA_START_SECTOR) / drive_ptr->SECTORS_PER_CLUSTER + 1;

    drive_ptr->CURRENT_DIR[0] = '\\'; /* Root dir */
    drive_ptr->CURRENT_DIR[1] = '\0';
    drive_ptr->CUR_DIR_CLUSTER = drive_ptr->ROOT_CLUSTER;
    drive_ptr->CUR_DIR_CHAIN_PTR = &drive_ptr->ROOT_CHAIN;

    if (result == MFS_NO_ERROR)
    {
        drive_ptr->DOS_DISK = true;
    }

    return result;
}


/*!
 * \brief Unmounts the filesystem.
 *
 * This function brings filesystem on drive to consistent state by flushing
 * all cached data and releases data structures.
 *
 * \param drive_ptr
 *
 * \return int Error code.
 */
int MFS_Unmount_drive_internal(
    MFS_DRIVE_STRUCT_PTR drive_ptr)
{
    int result = MFS_NO_ERROR;

#if !MFSCFG_READ_ONLY
    if (drive_ptr->FAT_TYPE == MFS_FAT32)
    {
        if (!MFS_is_read_only(drive_ptr))
        {
            FILESYSTEM_INFO_DISK_PTR fsinfo_ptr;
            result = MFS_sector_map(drive_ptr, drive_ptr->FS_INFO, (void **)&fsinfo_ptr, MFS_MAP_MODE_OVERWRITE, 0);
            if (result == MFS_NO_ERROR)
            {
                mqx_htodl(fsinfo_ptr->LEAD_SIG, FSI_LEADSIG);
                mqx_htodl(fsinfo_ptr->STRUCT_SIG, FSI_STRUCTSIG);
                mqx_htodl(fsinfo_ptr->FREE_COUNT, drive_ptr->FREE_COUNT);
                mqx_htodl(fsinfo_ptr->NEXT_FREE, drive_ptr->NEXT_FREE_CLUSTER);
                mqx_htodl(fsinfo_ptr->TRAIL_SIG, FSI_TRAILSIG);

                result = MFS_sector_unmap(drive_ptr, drive_ptr->FS_INFO, 1);
            }
        }
    }
#endif

    return result;
}
