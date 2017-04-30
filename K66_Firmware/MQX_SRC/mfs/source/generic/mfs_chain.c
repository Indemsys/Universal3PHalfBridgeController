/*HEADER**********************************************************************
*
* Copyright 2014 Freescale Semiconductor, Inc.
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
*   This file contains functions for FAT chain abstraction
*
*END************************************************************************/

#include "mfs.h"
#include "mfs_prv.h"


/*!
 * \brief Resets the chain data
 *
 * Resets the chain invalidating any cached information except of the head cluster value
 * This function is used internally in MFS_chain_init
 *
 * \param[in]  drive_ptr  Pointer to MFS drive context data
 * \param[in]  chain      Pointer to chain structure
 *
 * \return MFS_NO_ERROR
 *
 * \see MFS_chain_init
 */
static _mfs_error MFS_chain_reset(MFS_DRIVE_STRUCT_PTR drive_ptr, FAT_CHAIN *chain)
{
    chain->SEGMENT_POS = 0;
    chain->SEGMENT_LAST_CLUSTER = chain->HEAD_CLUSTER;

    if (chain->HEAD_CLUSTER)
    {
        chain->SEGMENT_SECTOR_NUM = CLUSTER_TO_SECTOR(drive_ptr, chain->HEAD_CLUSTER);
        chain->SEGMENT_SECTOR_COUNT = drive_ptr->SECTORS_PER_CLUSTER;
    }
    else
    {
        chain->SEGMENT_SECTOR_NUM = 0;
        chain->SEGMENT_SECTOR_COUNT = 0;
    }

    return MFS_NO_ERROR;
}


/*!
 * \brief Initializes chain structure
 *
 * Initializes FAT chain abstraction structure associating it with particular existing FAT chain specified by its head cluster
 *
 * \param[in]  drive_ptr     Pointer to MFS drive context data
 * \param[in]  chain         Pointer to chain structure
 * \param[in]  head_cluster  Head cluster specifying real chain to be associated with the abstraction structure
 *
 * \return MFS_NO_ERROR
 */
_mfs_error MFS_chain_init(MFS_DRIVE_STRUCT_PTR drive_ptr, FAT_CHAIN *chain, uint32_t head_cluster)
{
    chain->HEAD_CLUSTER = head_cluster;
    return MFS_chain_reset(drive_ptr, chain);
}


/*!
 * \brief Initializes chain structure
 *
 * Initializes FAT chain abstraction structure so that it represents fixed area defined by starting sector and sector count
 *
 * \param[in]  drive_ptr     Pointer to MFS drive context data
 * \param[in]  chain         Pointer to chain structure
 * \param[in]  head_cluster  Head cluster specifying real chain to be associated with the abstraction structure
 *
 * \return MFS_NO_ERROR
 */
_mfs_error MFS_chain_forge(MFS_DRIVE_STRUCT_PTR drive_ptr, FAT_CHAIN *chain, uint32_t sector_num, uint32_t sector_count)
{
    chain->HEAD_CLUSTER = 0;

    chain->SEGMENT_POS = 0;
    chain->SEGMENT_LAST_CLUSTER = 0;

    chain->SEGMENT_SECTOR_NUM = sector_num;
    chain->SEGMENT_SECTOR_COUNT = sector_count;

    return MFS_NO_ERROR;
}


#if !MFSCFG_READ_ONLY

/*!
 * \brief Creates new FAT chain
 *
 * Creates new FAT chain and associates chain structure with it
 *
 * \param[in]  drive_ptr         Pointer to MFS drive context data
 * \param[in]  chain             Pointer to chain structure
 * \param[out] head_cluster_ptr  Head cluster of newly created FAT chain is returned via this parameter
 *
 * \return MFS_NO_ERROR or an error code
 */
_mfs_error MFS_chain_create(MFS_DRIVE_STRUCT_PTR drive_ptr, FAT_CHAIN *chain, uint32_t *head_cluster_ptr)
{
    _mfs_error error_code = MFS_NO_ERROR;
    uint32_t head_cluster;

    head_cluster = MFS_Find_unused_cluster_from(drive_ptr, CLUSTER_MIN_GOOD);
    if (head_cluster == CLUSTER_INVALID)
    {
        error_code = MFS_DISK_FULL;
    }
    else
    {
        error_code = MFS_Put_fat(drive_ptr, head_cluster, CLUSTER_EOF);
    }

    if (error_code == MFS_NO_ERROR)
    {
        error_code = MFS_chain_init(drive_ptr, chain, head_cluster);
        if (head_cluster_ptr)
        {
            *head_cluster_ptr = head_cluster;
        }
    }

    return error_code;
}

#endif


/*!
 * \brief Translates position in chain to sector address
 *
 * Translates logical position in the chain to sector number.
 * Number of consecutive sectors is also returned to allow for multi-sector operations.
 *
 * \param[in]  drive_ptr     Pointer to MFS drive context data
 * \param[in]  chain         Pointer to chain structure
 * \param[in]  pos           Logical position within the chain
 * \param[in]  extend_to     The chain may be automatically extended up to this size if desired position is behind end of the chain
 * \param[out] sector        Sector containing the desired location is returned via pointer
 * \param[out] sector_count  Number of consecutive sectors of the chain is returned via pointer
 *
 * \return MFS_NO_ERROR or an error code
 */
_mfs_error MFS_chain_locate(MFS_DRIVE_STRUCT_PTR drive_ptr, FAT_CHAIN *chain, uint32_t pos, uint32_t extend_to, uint32_t *sector, uint32_t *sector_count)
{
    _mfs_error error_code = MFS_NO_ERROR;

    if (pos < chain->SEGMENT_POS)
    {
        /* Reset the chain to start scanning from its beginning */
        MFS_chain_reset(drive_ptr, chain);
    }

    /* Scan the chain until EOF is reached or desired location falls into the current segment */
    while (pos >= chain->SEGMENT_POS + (chain->SEGMENT_SECTOR_COUNT << drive_ptr->SECTOR_POWER))
    {
        uint32_t next_cluster;

        /* Check if the last cluster value is valid (chain structure is associated with a real FAT chain) */
        if (chain->SEGMENT_LAST_CLUSTER < CLUSTER_MIN_GOOD)
        {
            error_code = MFS_EOF;
            break;
        }

        /* Get next link of the chain from FAT */
        error_code = MFS_get_cluster_from_fat(drive_ptr, chain->SEGMENT_LAST_CLUSTER, &next_cluster);
        if (error_code != MFS_NO_ERROR)
        {
            break;
        }

        /* Check for EOF marker */
        if (next_cluster >= CLUSTER_MIN_LAST)
        {
#if MFSCFG_READ_ONLY
            error_code = MFS_EOF;
#else
            /* If the extend_to size is beyond the chain end, it is allowed to extend it */
            if (extend_to > chain->SEGMENT_POS + (chain->SEGMENT_SECTOR_COUNT << drive_ptr->SECTOR_POWER))
            {
                next_cluster = MFS_Find_unused_cluster_from(drive_ptr, chain->SEGMENT_LAST_CLUSTER + 1);
                if (next_cluster == CLUSTER_INVALID)
                {
                    error_code = MFS_DISK_FULL;
                }
                else
                {
                    if (pos == extend_to)
                    {
                        /* This is to cover special use case = force extend the directory chain clearing the newly added cluster */
                        error_code = MFS_Clear_cluster(drive_ptr, next_cluster);
                    }
                    if (error_code == MFS_NO_ERROR)
                    {
                        error_code = MFS_Put_fat(drive_ptr, next_cluster, CLUSTER_EOF);
                        if (error_code == MFS_NO_ERROR)
                        {
                            error_code = MFS_Put_fat(drive_ptr, chain->SEGMENT_LAST_CLUSTER, next_cluster);
                        }
                    }
                }
            }
            else
            {
                error_code = MFS_EOF;
            }
#endif
        }
        else if ((next_cluster < CLUSTER_MIN_GOOD) || (next_cluster > CLUSTER_MAX_GOOD))
        {
            error_code = MFS_LOST_CHAIN;
        }

        if (error_code != MFS_NO_ERROR)
        {
            break;
        }

        if (next_cluster == chain->SEGMENT_LAST_CLUSTER + 1)
        {
            chain->SEGMENT_SECTOR_COUNT += drive_ptr->SECTORS_PER_CLUSTER;
        }
        else
        {
            chain->SEGMENT_POS += chain->SEGMENT_SECTOR_COUNT << drive_ptr->SECTOR_POWER;
            chain->SEGMENT_SECTOR_NUM = CLUSTER_TO_SECTOR(drive_ptr, next_cluster);
            chain->SEGMENT_SECTOR_COUNT = drive_ptr->SECTORS_PER_CLUSTER;
        }
        chain->SEGMENT_LAST_CLUSTER = next_cluster;
    }

    if (error_code == MFS_NO_ERROR && sector)
    {
        /* The requested position is for sure inside the segment */
        uint32_t rel_sector = (pos - chain->SEGMENT_POS) >> drive_ptr->SECTOR_POWER;

        *sector = chain->SEGMENT_SECTOR_NUM + rel_sector;
        if (sector_count)
        {
            *sector_count = chain->SEGMENT_SECTOR_COUNT - rel_sector;
        }
    }

    return error_code;
}


/*!
 * \brief Translates position in chain to sector address
 *
 * Extends the chain to given length clearing the newly added clusters.
 * Clusters which are already part of the chain are not touched.
 * If the requested length is less or equal to the current length then success is reported without performing any operation.
 *
 * \param[in]  drive_ptr     Pointer to MFS drive context data
 * \param[in]  chain         Pointer to chain structure
 * \param[in]  extend_to     The chain may be automatically extended up to this size if desired position is behind end of the chain
 *
 * \return MFS_NO_ERROR or an error code
 */
_mfs_error MFS_chain_extend_clear(MFS_DRIVE_STRUCT_PTR drive_ptr, FAT_CHAIN *chain, uint32_t extend_to)
{
    return MFS_chain_locate(drive_ptr, chain, extend_to, extend_to, NULL, NULL);
}
