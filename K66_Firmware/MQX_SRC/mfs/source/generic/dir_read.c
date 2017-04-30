/*HEADER**********************************************************************
*
* Copyright 2008-2015 Freescale Semiconductor, Inc.
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

// Needs to be integrated into fopen (dirname, "d+mode_ptr"); etc


#define FS_DIR_MODE_UNIX 1
#define FS_DIR_MODE_MSDOS 2
#define FS_DIR_MODE_FILENAME 3

typedef struct
{
    MFS_FD_TYPE FS_PTR;
    MFS_SEARCH_DATA SEARCH_DATA;
    MFS_SEARCH_PARAM SEARCH_PARAM;
    char LFN[FILENAME_SIZE];
    uint32_t MODE;
} DIR_STRUCT, *DIR_STRUCT_PTR;


/*!
 * \brief
 *
 * \param[in] fs_ptr
 * \param[in] wildcard_ptr
 * \param[in] mode_ptr
 *
 * \return void *
 */
void *_io_mfs_dir_open(MFS_FD_TYPE fs_ptr, char *wildcard_ptr, char *mode_ptr)
{
    DIR_STRUCT_PTR dir_ptr;
    uint32_t attr = 0;
    int i;

    if (!MFS_FD_VALID(fs_ptr))
        return NULL;

    dir_ptr = (DIR_STRUCT_PTR)MFS_mem_alloc_zero(sizeof(DIR_STRUCT));
    if (dir_ptr == NULL)
        return NULL;

    _mem_set_type(dir_ptr, MEM_TYPE_MFS_DIR_STRUCT);

    dir_ptr->SEARCH_PARAM.WILDCARD = wildcard_ptr;
    dir_ptr->SEARCH_PARAM.LFN_BUF = dir_ptr->LFN;
    dir_ptr->SEARCH_PARAM.LFN_BUF_LEN = sizeof(dir_ptr->LFN);

    dir_ptr->FS_PTR = fs_ptr;
    dir_ptr->MODE = FS_DIR_MODE_UNIX;

    if (mode_ptr != NULL)
    {
        for (i = 0; mode_ptr[i]; i++)
        {
            switch (mode_ptr[i])
            {
                case '*':
                    attr = MFS_ATTR_ANY;
                    break;

                case 'r':
                    attr |= MFS_ATTR_READ_ONLY;
                    break;

                case 's':
                    attr |= MFS_ATTR_SYSTEM_FILE;
                    break;

                case 'h':
                    attr |= MFS_ATTR_HIDDEN_FILE;
                    break;

                case 'd':
                    attr |= MFS_ATTR_DIR_NAME;
                    break;

                case 'v':
                    attr |= MFS_ATTR_VOLUME_NAME;
                    break;

                case 'a':
                    attr |= MFS_ATTR_ARCHIVE;
                    break;

                case 'u':
                    dir_ptr->MODE = FS_DIR_MODE_UNIX;
                    break;

                case 'm':
                    dir_ptr->MODE = FS_DIR_MODE_MSDOS;
                    break;

                case 'f':
                    dir_ptr->MODE = FS_DIR_MODE_FILENAME;
                    break;

                default:
                    break;
            }
        }
    }
    dir_ptr->SEARCH_PARAM.ATTRIBUTE = attr | MFS_SEARCH_LFN;  // directly retrieve long filenames

    return (void *)dir_ptr;
}


static const char *const months[] =
    {
      "???", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };


/*!
 * \brief
 *
 * \param[in] dir
 * \param[in] buffer
 * \param[in] size
 *
 * \return int32_t
 */
int32_t _io_mfs_dir_read(void *dir, char *buffer, uint32_t size)
{
    DIR_STRUCT_PTR dir_ptr = (DIR_STRUCT_PTR)dir;
    int error_code;
    uint32_t month;
    uint32_t year;
    uint32_t ofs;
    uint32_t len = 0;

    if (dir_ptr == NULL)
    {
        return -1;
    }

    if (dir_ptr->SEARCH_PARAM.SEARCH_DATA_PTR == NULL)
    {
        dir_ptr->SEARCH_PARAM.SEARCH_DATA_PTR = &dir_ptr->SEARCH_DATA;
        error_code = ioctl(dir_ptr->FS_PTR, IO_IOCTL_FIND_FIRST_FILE, (void *)&dir_ptr->SEARCH_PARAM);
    }
    else
    {
        error_code = ioctl(dir_ptr->FS_PTR, IO_IOCTL_FIND_NEXT_FILE, (void *)&dir_ptr->SEARCH_DATA);
    }

    if (error_code != MFS_OK)
    {
        return -1;
    }

    /* attributes test */
    if (dir_ptr->MODE == FS_DIR_MODE_MSDOS)
    {
        ofs = len < size ? len : size;
        len = snprintf(buffer + ofs, size - ofs, "%-12.12s %8lu %02lu-%02lu-%04lu %02lu:%02lu:%02lu %c%c%c%c%c%c ", dir_ptr->SEARCH_DATA.NAME, dir_ptr->SEARCH_DATA.FILE_SIZE,

                       (uint32_t)(dir_ptr->SEARCH_DATA.DATE & MFS_MASK_MONTH) >> MFS_SHIFT_MONTH,
                       (uint32_t)(dir_ptr->SEARCH_DATA.DATE & MFS_MASK_DAY) >> MFS_SHIFT_DAY,
                       (uint32_t)((dir_ptr->SEARCH_DATA.DATE & MFS_MASK_YEAR) >> MFS_SHIFT_YEAR) + 1980,

                       (uint32_t)(dir_ptr->SEARCH_DATA.TIME & MFS_MASK_HOURS) >> MFS_SHIFT_HOURS,
                       (uint32_t)(dir_ptr->SEARCH_DATA.TIME & MFS_MASK_MINUTES) >> MFS_SHIFT_MINUTES,
                       (uint32_t)(dir_ptr->SEARCH_DATA.TIME & MFS_MASK_SECONDS) << 1,

                       (dir_ptr->SEARCH_DATA.ATTRIBUTE & MFS_ATTR_READ_ONLY) ? 'R' : ' ',
                       (dir_ptr->SEARCH_DATA.ATTRIBUTE & MFS_ATTR_HIDDEN_FILE) ? 'H' : ' ',
                       (dir_ptr->SEARCH_DATA.ATTRIBUTE & MFS_ATTR_SYSTEM_FILE) ? 'S' : ' ',
                       (dir_ptr->SEARCH_DATA.ATTRIBUTE & MFS_ATTR_VOLUME_NAME) ? 'V' : ' ',
                       (dir_ptr->SEARCH_DATA.ATTRIBUTE & MFS_ATTR_DIR_NAME) ? 'D' : ' ',
                       (dir_ptr->SEARCH_DATA.ATTRIBUTE & MFS_ATTR_ARCHIVE) ? 'A' : ' ');
    }
    else if (dir_ptr->MODE == FS_DIR_MODE_UNIX)
    {
        ofs = len < size ? len : size;
        len += snprintf(buffer + ofs, size - ofs, "%cr%c-r%c-r%c-   1 ", (dir_ptr->SEARCH_DATA.ATTRIBUTE & MFS_ATTR_DIR_NAME) ? 'd' : '-',

                        (dir_ptr->SEARCH_DATA.ATTRIBUTE & MFS_ATTR_READ_ONLY) ? '-' : 'w',
                        (dir_ptr->SEARCH_DATA.ATTRIBUTE & MFS_ATTR_READ_ONLY) ? '-' : 'w',
                        (dir_ptr->SEARCH_DATA.ATTRIBUTE & MFS_ATTR_READ_ONLY) ? '-' : 'w');

        ofs = len < size ? len : size;
        len += snprintf(buffer + ofs, size - ofs, "ftp      ftp  ");

        ofs = len < size ? len : size;
        len += snprintf(buffer + ofs, size - ofs, "  %10lu ", dir_ptr->SEARCH_DATA.FILE_SIZE);

        month = (uint32_t)(dir_ptr->SEARCH_DATA.DATE & MFS_MASK_MONTH) >> MFS_SHIFT_MONTH;
        if (month > 12)
            month = 0;

        ofs = len < size ? len : size;
        len += snprintf(buffer + ofs, size - ofs, "%3s %02d ", months[month], (uint32_t)(dir_ptr->SEARCH_DATA.DATE & MFS_MASK_DAY));

        year = (uint32_t)((dir_ptr->SEARCH_DATA.DATE & MFS_MASK_YEAR) >> MFS_SHIFT_YEAR) + 1980;
        ofs = len < size ? len : size;
        len += snprintf(buffer + ofs, size - ofs, " %4d ", year);
    }

    if (error_code == MFS_OK)
    {
        ofs = len < size ? len : size;
        len += snprintf(buffer + ofs, size - ofs, "%s\r\n", dir_ptr->LFN);
    }
    else
    {
        ofs = len < size ? len : size;
        len += snprintf(buffer + ofs, size - ofs, "%s\r\n", dir_ptr->SEARCH_DATA.NAME);
    }

    /* Terminate the string for sure */
    if (size > 0)
        buffer[size - 1] = '\0';

    return len;
}

/*!
 * \brief
 *
 * \param[in] dir
 *
 * \return int32_t
 */
int32_t _io_mfs_dir_close(void *dir)
{
    if (dir == NULL)
    {
        return -1;
    }

    MFS_mem_free(dir);
    return MFS_OK;
}
