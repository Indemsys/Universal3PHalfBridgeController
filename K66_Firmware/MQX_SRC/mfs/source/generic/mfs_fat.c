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
*   This file contains the functions that are used to modify and update
*   the FAT table on the disk.
*
*
*END************************************************************************/

#include "mfs.h"
#include "mfs_prv.h"


/*!
 * \brief
 *
 * Subroutines called by MFS_get_cluster_from_fat to obtain value
 * from specific FAT type (FAT12)
 *
 * \param drive_ptr
 * \param[in] fat_entry FAT entry index.
 * \param[out] cluster_number_ptr Pointer to return entry data.
 *
 * \return static _mfs_error
 */
static _mfs_error MFS_get_fat12(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    uint32_t fat_entry,
    uint32_t *cluster_number_ptr)
{
    uint32_t sector;
    uint32_t offset;

    uint32_t cluster_number;

    void *sector_buffer;
    uint8_t fat_entry_buffer[2];

    int error;

    /* Calculate entry offset within FAT and relative sector number within FAT */
    offset = fat_entry + (fat_entry >> 1);
    sector = offset >> drive_ptr->SECTOR_POWER;

    /* Adjust offset to be relative within the sector and calculate absolute sector number */
    offset -= sector << drive_ptr->SECTOR_POWER;
    sector += drive_ptr->FAT_START_SECTOR;

    /* Map sector into memory */
    error = MFS_sector_map(drive_ptr, sector, &sector_buffer, MFS_MAP_MODE_READONLY | MFS_CACHE_FLAG_FAT, 0);
    if (error != MFS_NO_ERROR)
    {
        return error;
    }

    /* Extract first byte of entry to the entry buffer */
    fat_entry_buffer[0] = ((uint8_t *)sector_buffer)[offset++];

    if (offset >= drive_ptr->SECTOR_SIZE)
    {
        /* Sector boundary reached, release mapped sector */
        error = MFS_sector_unmap(drive_ptr, sector, 0);
        if (error != MFS_NO_ERROR)
        {
            return error;
        }
        /* Advance to following sector and map it to memory */
        sector++;
        offset = 0;
        error = MFS_sector_map(drive_ptr, sector, &sector_buffer, MFS_MAP_MODE_READONLY | MFS_CACHE_FLAG_FAT, 0);
        if (error != MFS_NO_ERROR)
        {
            return error;
        }
    }

    /* Extract second byte of entry to the entry buffer */
    fat_entry_buffer[1] = ((uint8_t *)sector_buffer)[offset++];

    if ((fat_entry & 1) == 0)
    {
        /*
        ** For an even cluster number (on byte boundary), the lower 12
        ** bits are the relevant ones.
        */
        cluster_number = ftoh0(fat_entry_buffer);
    }
    else
    {
        /*
        ** For an odd cluster number (not on byte boundary), the
        ** higher 12 bits are the relevant ones.
        */
        cluster_number = ftoh1(fat_entry_buffer);
    }

    /* Extract FAT entry */
    if (cluster_number > CLUSTER_MAXVAL_12)
    {
        cluster_number |= CLUSTER_EXTEND_12;
    }
    if (cluster_number >= CLUSTER_MIN_LAST)
    {
        cluster_number = CLUSTER_EOF;
    }

    *cluster_number_ptr = cluster_number;

    /* Release mapped sector and return */
    error = MFS_sector_unmap(drive_ptr, sector, 0);
    return error;
}


/*!
 * \brief
 *
 * Subroutines called by MFS_get_cluster_from_fat to obtain value
 * from specific FAT type (FAT16)
 *
 * \param drive_ptr
 * \param[in] fat_entry FAT entry index.
 * \param[out] cluster_number_ptr Pointer to return entry data.
 *
 * \return static _mfs_error
 */
static _mfs_error MFS_get_fat16(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    uint32_t fat_entry,
    uint32_t *cluster_number_ptr)
{
    uint32_t sector;
    uint32_t offset;

    uint32_t cluster_number;

    void *sector_buffer;
    uint8_t *fat_entry_ptr;

    int error;

    /* Calculate entry offset within FAT and relative sector number within FAT */
    offset = fat_entry << 1;
    sector = offset >> drive_ptr->SECTOR_POWER;

    /* Adjust offset to be relative within the sector and calculate absolute sector number */
    offset -= sector << drive_ptr->SECTOR_POWER;
    sector += drive_ptr->FAT_START_SECTOR;

    /* Map sector into memory */
    error = MFS_sector_map(drive_ptr, sector, &sector_buffer, MFS_MAP_MODE_READONLY | MFS_CACHE_FLAG_FAT, 0);
    if (error != MFS_NO_ERROR)
    {
        return error;
    }

    /* Calculate pointer to the FAT entry */
    fat_entry_ptr = (uint8_t *)sector_buffer + offset;

    /* Extract FAT entry */
    cluster_number = mqx_dtohs((unsigned char *)fat_entry_ptr);
    if (cluster_number > CLUSTER_MAXVAL_16)
    {
        cluster_number |= CLUSTER_EXTEND_16;
    }
    if (cluster_number >= CLUSTER_MIN_LAST)
    {
        cluster_number = CLUSTER_EOF;
    }

    *cluster_number_ptr = cluster_number;

    /* Release mapped sector and return */
    error = MFS_sector_unmap(drive_ptr, sector, 0);
    return error;
}


/*!
 * \brief
 *
 * Subroutines called by MFS_get_cluster_from_fat to obtain value
 * from specific FAT type (FAT32)
 *
 * \param drive_ptr
 * \param[in] fat_entry FAT entry index.
 * \param[out] cluster_number_ptr Pointer to return entry data.
 *
 * \return static _mfs_error
 */
static _mfs_error MFS_get_fat32(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    uint32_t fat_entry,
    uint32_t *cluster_number_ptr)
{
    uint32_t sector;
    uint32_t offset;

    uint32_t cluster_number;

    void *sector_buffer;
    uint8_t *fat_entry_ptr;

    int error;

    /* Calculate entry offset within FAT and relative sector number within FAT */
    offset = fat_entry << 2;
    sector = offset >> drive_ptr->SECTOR_POWER;

    /* Adjust offset to be relative within the sector and calculate absolute sector number */
    offset -= sector << drive_ptr->SECTOR_POWER;
    sector += drive_ptr->FAT_START_SECTOR;

    /* Map sector into memory */
    error = MFS_sector_map(drive_ptr, sector, &sector_buffer, MFS_MAP_MODE_READONLY | MFS_CACHE_FLAG_FAT, 0);
    if (error != MFS_NO_ERROR)
    {
        return error;
    }

    /* Calculate pointer to the FAT entry */
    fat_entry_ptr = (uint8_t *)sector_buffer + offset;

    /* Extract FAT entry */
    cluster_number = mqx_dtohl((unsigned char *)fat_entry_ptr) & 0x0FFFFFFF;
    if (cluster_number >= CLUSTER_MIN_LAST)
    {
        cluster_number = CLUSTER_EOF;
    }

    *cluster_number_ptr = cluster_number;

    /* Release mapped sector and return */
    error = MFS_sector_unmap(drive_ptr, sector, 0);
    return error;
}


/*!
 * \brief
 *
 * Get the cluster number from the FAT stored in the buffer pointed
 * to by the FAT buffer. Assumes FAT in sync and sem obtained.
 *
 * \param drive_ptr
 * \param[in] fat_entry FAT entry index.
 * \param[out] cluster_number_ptr Pointer to return entry data.
 *
 * \return _mfs_error
 */
_mfs_error MFS_get_cluster_from_fat(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    uint32_t fat_entry,
    uint32_t *cluster_number_ptr)
{
    if ((fat_entry > drive_ptr->LAST_CLUSTER) || (fat_entry < CLUSTER_MIN_GOOD))
    {
        return MFS_INVALID_CLUSTER_NUMBER;
    }

    switch (drive_ptr->FAT_TYPE)
    {
        case MFS_FAT12:
            return MFS_get_fat12(drive_ptr, fat_entry, cluster_number_ptr);

        case MFS_FAT16:
            return MFS_get_fat16(drive_ptr, fat_entry, cluster_number_ptr);

        default:
            return MFS_get_fat32(drive_ptr, fat_entry, cluster_number_ptr);
    }
}


#if !MFSCFG_READ_ONLY


/*!
 * \brief
 *
 * Subroutines called by MFS_Put_fat to update value
 * in specific FAT type (FAT12)
 *
 * \param drive_ptr
 * \param[in] fat_entry FAT entry index
 * \param[in] code Code to write into the FAT index.
 * \param[out] code_prev Code read from FAT prior modification.
 *
 * \return static _mfs_error
 */
static _mfs_error MFS_put_fat12(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    uint32_t fat_entry,
    uint32_t code,
    uint32_t *code_prev)
{
    uint32_t sector;
    uint32_t offset;

    void *sector_buffer;

    uint8_t fat_entry_buffer[2];
    uint8_t fat_entry_mask[2];
    uint8_t fat_entry_prev[2];

    int error;

    /* Calculate entry offset within FAT and relative sector number within FAT */
    offset = fat_entry + (fat_entry >> 1);
    sector = offset >> drive_ptr->SECTOR_POWER;

    /* Adjust offset to be relative within the sector and calculate absolute sector number */
    offset -= sector << drive_ptr->SECTOR_POWER;
    sector += drive_ptr->FAT_START_SECTOR;

    fat_entry_buffer[0] = fat_entry_buffer[1] = 0;
    fat_entry_mask[0] = fat_entry_mask[1] = 0;

    if ((fat_entry & 1) == 0)
    {
        /*
        ** For an even cluster number (on byte boundary), the lower 12
        ** bits are the relevant ones.
        */
        htof0(fat_entry_buffer, code);
        htof0(fat_entry_mask, 0xFFFFFFFF);
    }
    else
    {
        /*
        ** For an odd cluster number (not on byte boundary), the
        ** higher 12 bits are the relevant ones.
        */
        htof1(fat_entry_buffer, code);
        htof1(fat_entry_mask, 0xFFFFFFFF);
    }

    /* Map sector into memory */
    error = MFS_sector_map(drive_ptr, sector, &sector_buffer, MFS_MAP_MODE_MODIFY | MFS_CACHE_FLAG_FAT, 0);
    if (error != MFS_NO_ERROR)
    {
        return error;
    }

    /* Process byte of the entry */
    fat_entry_prev[0] = ((uint8_t *)sector_buffer)[offset];
    ((uint8_t *)sector_buffer)[offset] &= ~fat_entry_mask[0];
    ((uint8_t *)sector_buffer)[offset] |= fat_entry_buffer[0];
    offset++;

    if (offset >= drive_ptr->SECTOR_SIZE)
    {
        /* Sector boundary reached, release mapped sector */
        error = MFS_sector_unmap(drive_ptr, sector, 1);
        if (error != MFS_NO_ERROR)
        {
            return error;
        }
        /* Advance to following sector and map it to memory */
        sector++;
        offset = 0;
        error = MFS_sector_map(drive_ptr, sector, &sector_buffer, MFS_MAP_MODE_MODIFY | MFS_CACHE_FLAG_FAT, 0);
        if (error != MFS_NO_ERROR)
        {
            return error;
        }
    }

    /* Process second byte of the entry */
    fat_entry_prev[1] = ((uint8_t *)sector_buffer)[offset];
    ((uint8_t *)sector_buffer)[offset] &= ~fat_entry_mask[1];
    ((uint8_t *)sector_buffer)[offset] |= fat_entry_buffer[1];
    offset++;

    /* Extract previous FAT entry */
    if ((fat_entry & 1) == 0)
    {
        /*
        ** For an even cluster number (on byte boundary), the lower 12
        ** bits are the relevant ones.
        */
        *code_prev = ftoh0(fat_entry_prev);
    }
    else
    {
        /*
        ** For an odd cluster number (not on byte boundary), the
        ** higher 12 bits are the relevant ones.
        */
        *code_prev = ftoh1(fat_entry_prev);
    }

    /* Release mapped sector and return */
    error = MFS_sector_unmap(drive_ptr, sector, 1);
    return error;
}


/*!
 * \brief
 *
 * Subroutines called by MFS_Put_fat to update value
 * in specific FAT type (FAT16)
 *
 * \param drive_ptr
 * \param[in] fat_entry FAT entry index
 * \param[in] code Code to write into the FAT index.
 * \param[out] code_prev Code read from FAT prior modification.
 *
 * \return static _mfs_error
 */
static _mfs_error MFS_put_fat16(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    uint32_t fat_entry,
    uint32_t code,
    uint32_t *code_prev)
{
    uint32_t sector;
    uint32_t offset;

    void *sector_buffer;
    uint8_t *fat_entry_ptr;

    int error;

    /* Calculate entry offset within FAT and relative sector number within FAT */
    offset = fat_entry << 1;
    sector = offset >> drive_ptr->SECTOR_POWER;

    /* Adjust offset to be relative within the sector and calculate absolute sector number */
    offset -= sector << drive_ptr->SECTOR_POWER;
    sector += drive_ptr->FAT_START_SECTOR;

    /* Map sector into memory */
    error = MFS_sector_map(drive_ptr, sector, &sector_buffer, MFS_MAP_MODE_MODIFY | MFS_CACHE_FLAG_FAT, 0);
    if (error != MFS_NO_ERROR)
    {
        return error;
    }

    /* Calculate pointer to the FAT entry */
    fat_entry_ptr = (uint8_t *)sector_buffer + offset;

    /* Modify FAT entry */
    *code_prev = mqx_dtohs((unsigned char *)fat_entry_ptr);
    mqx_htods((unsigned char *)fat_entry_ptr, code);

    /* Release mapped sector marking it dirty and return */
    error = MFS_sector_unmap(drive_ptr, sector, 1);
    return error;
}


/*!
 * \brief
 *
 * Subroutines called by MFS_Put_fat to update value
 * in specific FAT type (FAT32)
 *
 * \param drive_ptr
 * \param[in] fat_entry FAT entry index
 * \param[in] code Code to write into the FAT index.
 * \param[out] code_prev_ptr Code read from FAT prior modification.
 *
 * \return static _mfs_error
 */
static _mfs_error MFS_put_fat32(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    uint32_t fat_entry,
    uint32_t code,
    uint32_t *code_prev_ptr)
{
    uint32_t sector;
    uint32_t offset;

    void *sector_buffer;
    uint8_t *fat_entry_ptr;

    uint32_t code_prev;

    int error;

    /* Calculate entry offset within FAT and relative sector number within FAT */
    offset = fat_entry << 2;
    sector = offset >> drive_ptr->SECTOR_POWER;

    /* Adjust offset to be relative within the sector and calculate absolute sector number */
    offset -= sector << drive_ptr->SECTOR_POWER;
    sector += drive_ptr->FAT_START_SECTOR;

    /* Map sector into memory */
    error = MFS_sector_map(drive_ptr, sector, &sector_buffer, MFS_MAP_MODE_MODIFY | MFS_CACHE_FLAG_FAT, 0);
    if (error != MFS_NO_ERROR)
    {
        return error;
    }

    /* Calculate pointer to the FAT entry */
    fat_entry_ptr = (uint8_t *)sector_buffer + offset;

    /* Extract previous valive from FAT */
    code_prev = mqx_dtohl((unsigned char *)fat_entry_ptr);
    *code_prev_ptr = code_prev & 0x0FFFFFFF;

    /* Update value in FAT preserving highest 4 bits */
    code &= (code & 0x0FFFFFFF) | (code_prev & 0xF0000000);
    htof32((unsigned char *)fat_entry_ptr, code);

    /* Release mapped sector marking it dirty and return */
    error = MFS_sector_unmap(drive_ptr, sector, 1);
    return error;
}


/*!
 * \brief Write a cluster number in the specified slot of the FAT
 *
 * Assume FAT in sync and locked semaphore.
 *
 * \param drive_ptr
 * \param[in] fat_entry FAT entry index
 * \param[in] code Code to write into the FAT index.
 *
 * \return _mfs_error
 */
_mfs_error MFS_Put_fat(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    uint32_t fat_entry,
    uint32_t code)
{
    int error;

    uint32_t code_prev;

    if ((fat_entry > drive_ptr->LAST_CLUSTER) || (fat_entry < CLUSTER_MIN_GOOD))
    {
        return MFS_INVALID_CLUSTER_NUMBER;
    }

    switch (drive_ptr->FAT_TYPE)
    {
        case MFS_FAT12:
            error = MFS_put_fat12(drive_ptr, fat_entry, code, &code_prev);
            break;

        case MFS_FAT16:
            error = MFS_put_fat16(drive_ptr, fat_entry, code, &code_prev);
            break;

        default:
            error = MFS_put_fat32(drive_ptr, fat_entry, code, &code_prev);
            break;
    }

    if (error != MFS_NO_ERROR)
    {
        return error;
    }

    /* Update free cluster counter */
    if (code == CLUSTER_UNUSED && code_prev != CLUSTER_UNUSED)
    {
        /* Previously used cluster was marked as free */
        MFS_Increment_free_clusters(drive_ptr);
    }
    else if (code != CLUSTER_UNUSED && code_prev == CLUSTER_UNUSED)
    {
        /* Previously unused cluster was marked as allocated */
        MFS_Decrement_free_clusters(drive_ptr);
    }

    return MFS_NO_ERROR;
}

#endif
