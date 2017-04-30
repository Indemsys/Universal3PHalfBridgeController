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
*   This file contains of sector caching for MFS
*
*END************************************************************************/

#include "mfs.h"
#include "mfs_prv.h"


/*!
 * \brief Initializes sector cache data structures
 *
 * The function initializes sector cache data structures.
 * It is not exported from the module, it is executed as part of sector cache allocation (MFS_sector_cache_alloc).
 *
 * \param[in] drive_ptr  Pointer to MFS drive context data
 *
 * \return MFS_NO_ERROR, at present no other error state is reported.
 *
 * \see MFS_sector_cache_alloc
 */
static int MFS_sector_cache_init(MFS_DRIVE_STRUCT_PTR drive_ptr)
{
    SECTOR_CACHE *cache;

    SECTOR_CACHE_RECORD *cache_record;
    SECTOR_CACHE_RECORD **cache_record_ptr;

    int sector_size;
    int buffer_offset;

    int i;

    cache = &drive_ptr->SECTOR_CACHE;
    sector_size = drive_ptr->SECTOR_SIZE;

    cache_record_ptr = &cache->RECENT; /* store pointer to the first element in the linked list here */
    buffer_offset = 0;
    for (i = 0; i < cache->BUFFERS; i++)
    {
        cache_record = &cache->RECORDS[i];

        cache_record->SECTOR_NUM = MFS_INVALID_SECTOR;
        cache_record->USAGE_COUNTER = 0;
        cache_record->FLAGS = 0;
        cache_record->BUFFER = (uint8_t *)(cache->BUFFER_AREA) + buffer_offset;

        *cache_record_ptr = cache_record;
        cache_record_ptr = &cache_record->LRU_NEXT;

        buffer_offset += sector_size;
    }
    *cache_record_ptr = NULL;

    return MFS_NO_ERROR;
}


/*!
 * \brief Allocates and initializes sector cache
 *
 * The function allocates sector cache for the drive with specified number of sector buffers.
 *
 * \param[in] drive_ptr  Pointer to MFS drive context data
 * \param[in] buffers    Number of sector buffers to be allocated
 *
 * \return MFS_NO_ERROR
 * \return MFS_INSUFFICIENT_MEMORY
 *
 * \see MFS_sector_cache_free
 */
int MFS_sector_cache_alloc(MFS_DRIVE_STRUCT_PTR drive_ptr, int buffers)
{
    SECTOR_CACHE *cache;

    int sector_size;
    int error_code;

    /* First of all free the cache if it is already allocated */
    error_code = MFS_sector_cache_free(drive_ptr);
    if (error_code != MFS_NO_ERROR)
    {
        return error_code;
    }

    cache = &drive_ptr->SECTOR_CACHE;
    sector_size = drive_ptr->SECTOR_SIZE;

    cache->BUFFER_AREA = MFS_mem_alloc_system_zero(buffers * sector_size);
    if (cache->BUFFER_AREA == NULL)
    {
        return MFS_INSUFFICIENT_MEMORY;
    }
    _mem_set_type(cache->BUFFER_AREA, MEM_TYPE_MFS_DATA_SECTOR);

    cache->RECORDS = MFS_mem_alloc_system_zero(buffers * sizeof(SECTOR_CACHE_RECORD));
    if (cache->RECORDS == NULL)
    {
        MFS_mem_free(cache->BUFFER_AREA);
        cache->BUFFER_AREA = NULL;
        return MFS_INSUFFICIENT_MEMORY;
    }
    _mem_set_type(cache->RECORDS, MEM_TYPE_MFS_OTHER);

    cache->BUFFERS = buffers;

    return MFS_sector_cache_init(drive_ptr);
}


/*!
 * \brief Deallocates sector cache
 *
 * The function deallocates sector cache of given drive.
 * The sector cache shall be explicitly flushed before calling MFS_sector_cache_free (if desired).
 * Data in sector buffers which are not flushed before calling MFS_sector_cache_free is silently discarded.
 *
 * \param[in] drive_ptr  Pointer to MFS drive context data
 *
 * \return MFS_NO_ERROR or error code returned by memory allocator
 *
 * \see MFS_sector_cache_alloc
 */
int MFS_sector_cache_free(MFS_DRIVE_STRUCT_PTR drive_ptr)
{
    SECTOR_CACHE *cache;

    int error_code;
    int result;

    cache = &drive_ptr->SECTOR_CACHE;
    result = MFS_NO_ERROR;

    if (cache->BUFFER_AREA != NULL)
    {
        error_code = MFS_mem_free(cache->BUFFER_AREA);
        if (result == MFS_NO_ERROR)
        {
            result = error_code;
        }
        cache->BUFFER_AREA = NULL;
    }

    if (cache->RECORDS != NULL)
    {
        error_code = MFS_mem_free(cache->RECORDS);
        if (result == MFS_NO_ERROR)
        {
            result = error_code;
        }
        cache->RECORDS = NULL;
    }

    cache->BUFFERS = 0;
    cache->RECENT = NULL;

    return result;
}


#if !MFSCFG_READ_ONLY
/*!
 * \brief Writes a sector of FAT table to the storage device
 *
 * The function writes data from buffer to given sector and if the sector number is within FAT table
 * it ensure that all copies of FAT table are kept in sync (writes the same data to all sector mirroring it).
 *
 * \param[in] drive_ptr   Pointer to MFS drive context data
 * \param[in] sector_num  Address of the sector
 * \param[in] buffer      Pointer to buffer containing data to be written
 *
 * \return MFS_NO_ERROR or an error code returned by underlying write
 *
 */
static int MFS_write_fat_sector(MFS_DRIVE_STRUCT_PTR drive_ptr, uint32_t sector_num, void *buffer)
{
    int error_code;
    uint32_t fat_size;
    uint32_t fat_copies;

    fat_size = drive_ptr->SECTORS_PER_FAT;
    fat_copies = drive_ptr->NUMBER_OF_FAT;

    if (sector_num < drive_ptr->FAT_START_SECTOR || sector_num >= drive_ptr->FAT_START_SECTOR + fat_copies * fat_size)
    {
        /* Not a FAT sector, safety fallback to single sector write */
        fat_copies = 1;
    }

    /* Seek to first FAT copy */
    while (sector_num >= drive_ptr->FAT_START_SECTOR + fat_size)
    {
        sector_num -= fat_size;
    }

    /* Write the sector to all FAT copies */
    while (fat_copies--)
    {
        error_code = MFS_Write_device_sectors(drive_ptr, sector_num, 1, MFSCFG_MAX_WRITE_RETRIES, buffer, NULL);
        if (error_code != MFS_NO_ERROR)
        {
            return error_code;
        }
        sector_num += fat_size;
    }

    return MFS_NO_ERROR;
}
#endif


/*!
 * \brief Flushes single cache record
 *
 * The function ensures that data of the given cache record is written back to the drive.
 *
 * \param[in] drive_ptr     Pointer to MFS drive context data
 * \param[in] cache_record  Pointer to cache record
 *
 * \return MFS_NO_ERROR or an error code returned by underlying write
 *
 */
static int MFS_sector_cache_flush_record(MFS_DRIVE_STRUCT_PTR drive_ptr, SECTOR_CACHE_RECORD *cache_record)
{
#if !MFSCFG_READ_ONLY
    int error_code;

    if ((cache_record->SECTOR_NUM == MFS_INVALID_SECTOR) || ((cache_record->FLAGS & MFS_CACHE_FLAG_DIRTY) == 0))
    {
        /* Sector cache is clean, just return */
        return MFS_NO_ERROR;
    }

    if (cache_record->FLAGS & MFS_CACHE_FLAG_FAT)
    {
        /* Special handling of FAT sectors - mirroring */
        error_code = MFS_write_fat_sector(drive_ptr, cache_record->SECTOR_NUM, cache_record->BUFFER);
    }
    else
    {
        /* Write sector to the device */
        error_code = MFS_Write_device_sectors(drive_ptr, cache_record->SECTOR_NUM, 1, MFSCFG_MAX_WRITE_RETRIES, cache_record->BUFFER, NULL);
    }

    if (error_code == MFS_NO_ERROR)
    {
        /* Sector written to storage device, mark it clean */
        cache_record->FLAGS &= ~MFS_CACHE_FLAG_DIRTY;
    }

    return error_code;
#else
    return MFS_NO_ERROR;
#endif
}


/*!
 * \brief Flushes range of sectors
 *
 * The function ensures that buffers of all sectors withing given range are written back to the drive.
 * If the sector range is specified as 0,0 then whole sector cache is flushed.
 *
 * \param[in] drive_ptr     Pointer to MFS drive context data
 * \param[in] first_sector  Defines first sector of the range
 * \param[in] sectors       Defines number of sectors spanning the range
 *
 * \return MFS_NO_ERROR or an error code returned by underlying write
 *
 */
int MFS_sector_cache_flush(MFS_DRIVE_STRUCT_PTR drive_ptr, uint32_t first_sector, uint32_t sectors)
{
#if !MFSCFG_READ_ONLY
    SECTOR_CACHE *cache;
    SECTOR_CACHE_RECORD *cache_record;

    int error_code;
    int result = MFS_NO_ERROR;

    uint32_t last_sector;

    cache = &drive_ptr->SECTOR_CACHE;

    /* As a side effect, following statement ensures that sector range specified as 0,0 defaults to whole drive */
    last_sector = first_sector + sectors - 1;

    cache_record = cache->RECENT;
    while (cache_record)
    {
        if ((cache_record->SECTOR_NUM >= first_sector) && (cache_record->SECTOR_NUM < last_sector))
        {
            error_code = MFS_sector_cache_flush_record(drive_ptr, cache_record);
            if (result == MFS_NO_ERROR)
            {
                /* Report first error_code code which occurs */
                result = error_code;
            }
        }
        cache_record = cache_record->LRU_NEXT;
    }

    return result;
#else
    return MFS_NO_ERROR;
#endif
}


/*!
 * \brief Flushes cache records with given tag
 *
 * The function ensures that buffers with given tag are are written back to the drive.
 * If the sector range is specified as 0,0 then whole sector cache is flushed.
 *
 * \param[in] drive_ptr     Pointer to MFS drive context data
 * \param[in] tag           Specifies tag for selecting cache records
 *
 * \return MFS_NO_ERROR or an error code returned by underlying write
 *
 */
int MFS_sector_cache_flush_tag(MFS_DRIVE_STRUCT_PTR drive_ptr, uint32_t tag)
{
#if !MFSCFG_READ_ONLY
    SECTOR_CACHE *cache;
    SECTOR_CACHE_RECORD *cache_record;

    int error_code;
    int result = MFS_NO_ERROR;

    cache = &drive_ptr->SECTOR_CACHE;

    cache_record = cache->RECENT;
    while (cache_record)
    {
        if (cache_record->TAG == tag)
        {
            error_code = MFS_sector_cache_flush_record(drive_ptr, cache_record);
            if (result == MFS_NO_ERROR)
            {
                /* Report first error_code code which occurs */
                result = error_code;
            }
        }
        cache_record = cache_record->LRU_NEXT;
    }

    return result;
#else
    return MFS_NO_ERROR;
#endif
}


/*!
 * \brief Invalidates single cache record
 *
 * The function invalidates given cache record, discarding any changes in its buffer.
 * If the cache record is in use, contents its buffer is replaced by data read from the storage device.
 * If the cache record is not currently being used then it is marked as free.
 *
 * \param[in] drive_ptr     Pointer to MFS drive context data
 * \param[in] cache_record  Pointer to cache record
 *
 * \return MFS_NO_ERROR or an error code returned by underlying read
 *
 */
static int MFS_sector_cache_invalidate_record(MFS_DRIVE_STRUCT_PTR drive_ptr, SECTOR_CACHE_RECORD *cache_record)
{
    int error_code = MFS_NO_ERROR;

    if (cache_record->USAGE_COUNTER == 0)
    {
        cache_record->SECTOR_NUM = MFS_INVALID_SECTOR;
        cache_record->FLAGS = 0;
    }
    else
    {
        cache_record->FLAGS &= ~MFS_CACHE_FLAG_DIRTY;
        error_code = MFS_Read_device_sectors(drive_ptr, cache_record->SECTOR_NUM, 1, MFSCFG_MAX_READ_RETRIES, cache_record->BUFFER, NULL);
    }

    return error_code;
}


/*!
 * \brief Invalidates range of sectors
 *
 * The function invalidates cache records corresponding to given sector range, discarding any changes in their buffers.
 * If a cache record is in use, content of its is replaced by data read from the storage device.
 * If a cache record is not currently being used then it is marked as free.
 *
 * \param[in] drive_ptr     Pointer to MFS drive context data
 * \param[in] first_sector  Defines first sector of the range
 * \param[in] sectors       Defines number of sectors spanning the range
 *
 * \return MFS_NO_ERROR or an error code returned by underlying read
 *
 */
int MFS_sector_cache_invalidate(MFS_DRIVE_STRUCT_PTR drive_ptr, uint32_t first_sector, uint32_t sectors)
{
    SECTOR_CACHE *cache;
    SECTOR_CACHE_RECORD *cache_record;

    int error_code;
    int result = MFS_NO_ERROR;

    uint32_t last_sector;

    cache = &drive_ptr->SECTOR_CACHE;

    /* As a side effect, following statement ensures that sector range specified as 0,0 defaults to whole drive */
    last_sector = first_sector + sectors - 1;

    cache_record = cache->RECENT;
    while (cache_record)
    {
        if ((cache_record->SECTOR_NUM >= first_sector) && (cache_record->SECTOR_NUM < last_sector))
        {
            error_code = MFS_sector_cache_invalidate_record(drive_ptr, cache_record);
            if (result == MFS_NO_ERROR)
            {
                /* Report first error_code code which occurs */
                result = error_code;
            }
        }
        cache_record = cache_record->LRU_NEXT;
    }

    return result;
}


/*!
 * \brief Invalidates range of sectors
 *
 * The function invalidates cache records corresponding to given sector range, discarding any changes in their buffers.
 * If a cache record is in use, content of its is replaced by data read from the storage device.
 * If a cache record is not currently being used then it is marked as free.
 *
 * \param[in] drive_ptr     Pointer to MFS drive context data
 * \param[in] tag           Specifies tag for selecting cache records
 *
 * \return MFS_NO_ERROR or an error code returned by underlying read
 *
 */
int MFS_sector_cache_invalidate_tag(MFS_DRIVE_STRUCT_PTR drive_ptr, uint32_t tag)
{
    SECTOR_CACHE *cache;
    SECTOR_CACHE_RECORD *cache_record;

    int error_code;
    int result = MFS_NO_ERROR;

    cache = &drive_ptr->SECTOR_CACHE;

    cache_record = cache->RECENT;
    while (cache_record)
    {
        if (cache_record->TAG == tag)
        {
            error_code = MFS_sector_cache_invalidate_record(drive_ptr, cache_record);
            if (result == MFS_NO_ERROR)
            {
                /* Report first error_code code which occurs */
                result = error_code;
            }
        }
        cache_record = cache_record->LRU_NEXT;
    }

    return result;
}


/*!
 * \brief Maps sector to memory
 *
 * The function attempts to map given sector to memory.
 * If the mapping is successful then pointer to sector content is returned.
 * A tag may be specified to later reference the related sector cache record.
 * Once the sector content is not needed anymore the sector has to be released from the cache using MFS_sector_unmap.
 * The same sector may be mapped more than once at a time. An internal reference counter is used to keep track of active mappings.
 *
 * \param[in]  drive_ptr     Pointer to MFS drive context data
 * \param[in]  sector_num    Sector to be mapped
 * \param[out] buf_ptr       Pointer to memory mapped content is returned via this parameter
 * \param[in]  flags         Specifies access mode and optional flags to control behavior of the cache record
 * \param[in]  tag           Specifies tag to associate with the cache record
 *
 * \return MFS_NO_ERROR or an error code
 *
 * \see MFS_sector_unmap
 */
int MFS_sector_map(MFS_DRIVE_STRUCT_PTR drive_ptr, uint32_t sector_num, void **buf_ptr, int flags, uint32_t tag)
{
    SECTOR_CACHE *cache;

    SECTOR_CACHE_RECORD *cache_record;
    SECTOR_CACHE_RECORD **cache_record_ptr; /* pointer to current record is stored here */

    SECTOR_CACHE_RECORD *best = NULL; /* the best candidate found so far */
    SECTOR_CACHE_RECORD **best_ptr = NULL; /* pointer to the best candidate is stored here */
    uint32_t best_score = 0;

    uint32_t fat_buffers;
    uint32_t fat_buffers_prefer;

    uint32_t ordinal;
    uint32_t score;

    int error_code;

    if (buf_ptr)
    {
        *buf_ptr = NULL;
    }

    /* Parameter sanity check */
    if ((flags & (MFS_CACHE_FLAG_READ | MFS_CACHE_FLAG_WRITE)) == 0)
    {
        /* Sector access mode has to be specified */
        return MFS_INVALID_PARAMETER;
    }

    /* Check access permissions */
    if ((flags & MFS_CACHE_FLAG_WRITE) && MFS_is_read_only(drive_ptr))
    {
        return MFS_DISK_IS_WRITE_PROTECTED;
    }

    cache = &drive_ptr->SECTOR_CACHE;

    cache_record_ptr = &cache->RECENT;
    cache_record = cache->RECENT;
    ordinal = 0;

    /* prefer to keep FAT sectors in the cache up to the half of its size */
    fat_buffers_prefer = (cache->BUFFERS + 1) / 2;
    fat_buffers = 0;

    /* Look up sector cache whether the sector is already mapped and find candidate to be replaced */
    while (cache_record)
    {

        if (cache_record->SECTOR_NUM == sector_num)
        {
            /* Match, the sector is already buffered, reuse it */
            best = cache_record;
            best_ptr = cache_record_ptr;
            break;
        }

        ordinal++;

        if (cache_record->FLAGS & MFS_CACHE_FLAG_FAT)
        {
            fat_buffers++;
        }

        /* Only unused cache record may be swapped out */
        if (cache_record->USAGE_COUNTER == 0)
        {

            if (cache_record->SECTOR_NUM == MFS_INVALID_SECTOR)
            {
                score = UINT32_MAX;
            }
            else if ((cache_record->FLAGS & MFS_CACHE_FLAG_FAT) && (fat_buffers <= fat_buffers_prefer))
            {
                score = 0;
            }
            else
            {
                score = ordinal;
            }

            /* Check if this is a better candidate */
            if (score >= best_score)
            {
                best = cache_record;
                best_ptr = cache_record_ptr;
                best_score = score;
            }
        }

        /* Advance to the next element of the linked list and remember where the pointer to it is stored */
        cache_record_ptr = &cache_record->LRU_NEXT;
        cache_record = cache_record->LRU_NEXT;
    }

    if (best == NULL)
    {
        return MFS_INSUFFICIENT_MEMORY;
    }

    cache_record = best;
    cache_record_ptr = best_ptr;

    if (cache_record->SECTOR_NUM != sector_num)
    {

        error_code = MFS_sector_cache_flush_record(drive_ptr, cache_record);
        if (error_code != MFS_NO_ERROR)
        {
            return error_code;
        }

        /* Reset cache metadata */
        cache_record->SECTOR_NUM = MFS_INVALID_SECTOR;
        cache_record->FLAGS = 0;
        cache_record->TAG = 0;

        if (flags & MFS_CACHE_FLAG_READ)
        {
            /* Fetch sector from storage device */
            error_code = MFS_Read_device_sectors(drive_ptr, sector_num, 1, MFSCFG_MAX_READ_RETRIES, cache_record->BUFFER, NULL);
            if (error_code != MFS_NO_ERROR)
            {
                return error_code;
            }
        }
    }

    if ((flags & MFS_CACHE_FLAG_READ) == 0)
    {
        /* Clear sector cache, not really necessary, wipe out data in the buffer for safety */
        _mem_zero(cache_record->BUFFER, drive_ptr->SECTOR_SIZE);
        cache_record->FLAGS |= MFS_CACHE_FLAG_DIRTY;
    }

    /* Update cache metadata */
    cache_record->SECTOR_NUM = sector_num;
    cache_record->FLAGS |= flags;
    cache_record->TAG = tag;
    cache_record->USAGE_COUNTER++;

    /* Reorder the list to place this (most recent) record first */
    *cache_record_ptr = cache_record->LRU_NEXT;
    cache_record->LRU_NEXT = cache->RECENT;
    cache->RECENT = cache_record;

    /* Return pointer to the buffer */
    if (buf_ptr)
    {
        *buf_ptr = cache_record->BUFFER;
    }

    return MFS_NO_ERROR;
}


/*!
 * \brief Releases active memory mapping
 *
 * The function releases active mapping of the given sector.
 * The pointer to buffer previously obtained by MFS_sector_map should be treated as invalid after the function returns.
 * If the sector was mapped for updating or overwriting the caller may indicate that content of the buffer has been updated
 * and thus shall be eventually written back to storage device.
 *
 * \param[in]  drive_ptr     Pointer to MFS drive context data
 * \param[in]  sector_num    Sector number mapping of which is to be released
 * \param[in]  updated       Indicates that content of the buffer has been updated
 *
 * \return MFS_NO_ERROR or an error code
 *
 * \see MFS_sector_map
 */
int MFS_sector_unmap(MFS_DRIVE_STRUCT_PTR drive_ptr, uint32_t sector_num, bool updated)
{
    SECTOR_CACHE *cache;
    SECTOR_CACHE_RECORD *cache_record;
    uint32_t flags;

    cache = &drive_ptr->SECTOR_CACHE;

    /* Find record related to the given sector number */
    cache_record = cache->RECENT;
    while (cache_record)
    {
        if (cache_record->SECTOR_NUM == sector_num)
            break;
        cache_record = cache_record->LRU_NEXT;
    }

    if (cache_record == NULL)
    {
        return MFS_SECTOR_NOT_FOUND;
    }

    if (cache_record->USAGE_COUNTER == 0)
    {
        return MFS_OPERATION_NOT_ALLOWED;
    }

    flags = cache_record->FLAGS;

    cache_record->USAGE_COUNTER--;
    if (cache_record->USAGE_COUNTER == 0)
    {
        /* If there is no mapping left clear access flags */
        cache_record->FLAGS &= ~MFS_MAP_MODE_MASK;
    }

    if (updated)
    {
        if ((flags & MFS_CACHE_FLAG_WRITE) == 0)
        {
            /* The sector data might be altered, possibly invalidate the record */
            return MFS_WRITE_FAULT;
        }
        cache_record->FLAGS |= MFS_CACHE_FLAG_DIRTY;
    }

    return MFS_NO_ERROR;
}
