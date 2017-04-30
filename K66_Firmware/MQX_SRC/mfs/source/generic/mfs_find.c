/*HEADER**********************************************************************
*
* Copyright 2015 Freescale Semiconductor, Inc.
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
*   This file contains functions necessary for find first/next directory search API
*
*END************************************************************************/

#include "mfs.h"
#include "mfs_prv.h"


/*!
 * \brief
 *
 * \param[in] wildcard pattern to match against
 * \param[in] name to match
 * \param[in] allowed recursions depth if wildcard contains more than one '*'
 * \param[in] allowed recursions depth if wildcard contains more than one '*'
 * \param[in] flags internally passed during recursion, shall be 0 in the initial call
 *
 * \return result of matching as bool value
 */
static bool MFS_wildcard_match_internal(
    char *wildcard,
    char *name,
    int recursions,
    int flags)
{
    int32_t wildcard_wchar;
    int32_t name_wchar;

    while (*wildcard)
    {
        if (*wildcard == '*')
        {
            /* Discard '*' character */
            wildcard++;
            if (*wildcard == '\0')
            {
                /* Optimization: this '*' is the last (greedy) wildcard, it matches against all the rest of the name */
                return true;
            }

            while (*name)
            {
                /* Try to match rest using recursion */
                if (recursions > 0 && MFS_wildcard_match_internal(wildcard, name, recursions - 1, flags))
                {
                    return true;
                }
                /* There is no success, consume one character of the name and retry */
                name_wchar = utf8_decode(&name, NULL);
                if (name_wchar < 0)
                {
                    /* invalid UTF8 string */
                    return false;
                }
                if (name_wchar == '.')
                {
                    flags = 1;  // indicate dot found
                }
            }
        }
        else
        {
            /* Is there anything to match at all? */
            if (*name == '\0')
            {
                /* End of filename may match a dot if no other dot was found.
                   This is for compatibility so that patterns like "anything.*" match also filenames without an extension
                   and patterns like "*." files exclusively without an extension */
                if (*wildcard == '.' && !flags)
                {
                    wildcard++;
                    flags = 1;  // indicate dot matched
                    continue;
                }
                return false;
            }

            name_wchar = utf8_decode(&name, NULL);
            if (name_wchar < 0)
            {
                /* invalid UTF8 string */
                return false;
            }
            if (name_wchar == '.')
            {
                flags = 1;  // indicate dot found
            }

            /* If there is '?' in the wildcard it matches any valid character, just continue */
            if (*wildcard == '?')
            {
                wildcard++;
                continue;
            }

            wildcard_wchar = utf8_decode(&wildcard, NULL);
            if (wildcard_wchar < 0)
            {
                /* invalid UTF8 string */
                return false;
            }

            /* Check case insensitive match */
            if (CAPITALIZE(wildcard_wchar) != CAPITALIZE(name_wchar))
            {
                /* mismatch */
                return false;
            }
        }
    }

    /* End of the wildcard reached, the name matches if it was whole consumed */
    return (*name == '\0');
}


/*!
 * \brief
 *
 * \param[in] wildcard pattern to match against
 * \param[in] name to match
 *
 * \return result of matching as bool value
 */
static bool MFS_wildcard_match(
    char *wildcard,
    char *name)
{
    return MFS_wildcard_match_internal(wildcard, name, 3, 0);
}


/*!
 * \brief
 *
 * \param drive_ptr
 * \param[in] attribute Type of file to find, Search attributes.
 * \param[in] pathname Optionally the directory and filename to search for.
 * \param[in] sd_ptr Address of search data block into which the results of the search are put.
 *
 * \return _mfs_error Error code.
 */
_mfs_error MFS_find_init(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    MFS_SEARCH_PARAM_PTR sp_ptr,
    MFS_SEARCH_DATA_PTR sd_ptr)
{
    _mfs_error error_code;

    MFS_INTERNAL_SEARCH *isd_ptr;
    char *pathname;
    char *wildcard;

    _mem_zero(sd_ptr, sizeof(MFS_SEARCH_DATA));

    sd_ptr->DRIVE_PTR = drive_ptr;
    isd_ptr = &sd_ptr->INTERNAL_SEARCH_DATA;
    pathname = sp_ptr->WILDCARD ? sp_ptr->WILDCARD : "";

    error_code = MFS_get_dir_chain(drive_ptr, pathname, &isd_ptr->DIR_CHAIN, NULL, &wildcard);
    if (error_code != MFS_NO_ERROR)
    {
        return error_code;
    }

    /* If direct LFN extraction is requested, initialize pointer to the LFN buffer */
    if ((sp_ptr->ATTRIBUTE & MFS_SEARCH_LFN) && sp_ptr->LFN_BUF)
    {
        sd_ptr->LFN_BUF = sp_ptr->LFN_BUF;
        sd_ptr->LFN_BUF_LEN = sp_ptr->LFN_BUF_LEN;
    }

    /* Initialize internal search data */
    isd_ptr->DIR_CHAIN_LOC = 0;
    isd_ptr->WILDCARD = wildcard;


    if (sp_ptr->ATTRIBUTE & MFS_SEARCH_ANY)
    {
        isd_ptr->ATTR_ONE_MASK = 0;
        isd_ptr->ATTR_ZERO_MASK = 0;
    }
    else if (sp_ptr->ATTRIBUTE & MFS_SEARCH_EXCLUSIVE)
    {
        /* require exact match */
        isd_ptr->ATTR_ONE_MASK = sp_ptr->ATTRIBUTE & 0x3F;
        isd_ptr->ATTR_ZERO_MASK = ~isd_ptr->ATTR_ONE_MASK;
    }
    else if (sp_ptr->ATTRIBUTE & MFS_SEARCH_SUBDIR)
    {
        /* non-system, non-hidden subdirs*/
        isd_ptr->ATTR_ONE_MASK = MFS_ATTR_DIR_NAME;
        isd_ptr->ATTR_ZERO_MASK = MFS_ATTR_HIDDEN_FILE | MFS_ATTR_SYSTEM_FILE;
    }
    else if ((sp_ptr->ATTRIBUTE & 0x3F) == MFS_SEARCH_NORMAL)
    {
        /* no attributes explicitly specified, search for non-system, non-hidden */
        isd_ptr->ATTR_ONE_MASK = 0;
        isd_ptr->ATTR_ZERO_MASK = MFS_ATTR_HIDDEN_FILE | MFS_ATTR_SYSTEM_FILE;
    }
    else
    {
        /* require all bit specified in search attributes to be set */
        isd_ptr->ATTR_ONE_MASK = sp_ptr->ATTRIBUTE & 0x3F;
        isd_ptr->ATTR_ZERO_MASK = 0;
    }

    return error_code;
}


/*!
 * \brief
 *
 * \param drive_ptr
 * \param[in,out] sd_ptr Address of search data block indicating the current criteria and the results
 *                       of the last search results of this search are placed in this data block
 *
 * \return _mfs_error Error code.
 */
_mfs_error MFS_find_next_internal(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    MFS_SEARCH_DATA_PTR sd_ptr)
{
    _mfs_error error_code;
    MFS_INTERNAL_SEARCH *isd_ptr;
    DIR_ENTRY_DISK dir_entry;

    char *lfn_buf;
    int lfn_buf_len;

    /* if there is no buffer for long filename then allocate a temporary one */
    if (sd_ptr->LFN_BUF == NULL)
    {
        lfn_buf_len = FILENAME_SIZE;
        lfn_buf = MFS_mem_alloc_system(lfn_buf_len);
        if (lfn_buf == NULL)
        {
            return MFS_INSUFFICIENT_MEMORY;
        }
    }
    else
    {
        lfn_buf_len = sd_ptr->LFN_BUF_LEN;
        lfn_buf = sd_ptr->LFN_BUF;
    }

    isd_ptr = &sd_ptr->INTERNAL_SEARCH_DATA;

    while (1)
    {
        error_code = MFS_read_dir_chain(drive_ptr, &isd_ptr->DIR_CHAIN, &isd_ptr->DIR_CHAIN_LOC, lfn_buf, lfn_buf_len, &dir_entry, NULL, NULL, NULL);
        if (error_code != MFS_NO_ERROR)
        {
            break;
        }

        /* Check attributes and if there is mismatch got for next iteration rightaway */
        if ((mqx_dtohc(dir_entry.ATTRIBUTE) & isd_ptr->ATTR_ZERO_MASK) != 0)
        {
            /* an attribute bit required to be zero is set */
            continue;
        }
        if ((dir_entry.ATTRIBUTE[0] & isd_ptr->ATTR_ONE_MASK) != isd_ptr->ATTR_ONE_MASK)
        {
            /* an attribute bit required to be one is clear */
            continue;
        }

        /* Extract short filename */
        MFS_sfn_from_disk(dir_entry.NAME, sd_ptr->NAME);

        /* Do not perform any matching if there is no wildcard string (instant match) */
        if (*(isd_ptr->WILDCARD) == '\0')
        {
            break;
        }

        /* Match short filename against the wildcard */
        if (MFS_wildcard_match(isd_ptr->WILDCARD, sd_ptr->NAME))
        {
            break;
        }

        /* Match long filename against the wildcard */
        if (MFS_wildcard_match(isd_ptr->WILDCARD, lfn_buf))
        {
            break;
        }
    }

    if (error_code == MFS_NO_ERROR)
    {
        sd_ptr->ATTRIBUTE = mqx_dtohc(dir_entry.ATTRIBUTE);
        sd_ptr->TIME = mqx_dtohs(dir_entry.TIME);
        sd_ptr->DATE = mqx_dtohs(dir_entry.DATE);
        sd_ptr->FILE_SIZE = mqx_dtohl(dir_entry.FILE_SIZE);
    }
    else if (error_code == MFS_EOF)
    {
        error_code = MFS_FILE_NOT_FOUND;
    }

    if (sd_ptr->LFN_BUF == NULL)
    {
        MFS_mem_free(lfn_buf);
    }

    return error_code;
}


/*!
 * \brief Find the first file that matches the input criteria.
 *
 * \param drive_ptr
 * \param[in] attribute Type of file to find, Search attributes.
 * \param[in] pathname Optionally the directory and filename to search for.
 * \param[in] sd_ptr Address of search data block into which the results of the search are put.
 *
 * \return _mfs_error Error code.
 */
_mfs_error MFS_find_first_file(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    MFS_SEARCH_PARAM_PTR sp_ptr)
{
    _mfs_error error_code;
    _mfs_error error_code_tmp;

    MFS_SEARCH_DATA *sd_ptr;

    if (sp_ptr == NULL)
    {
        return MFS_INVALID_PARAMETER;
    }

    if (sp_ptr->SEARCH_DATA_PTR == NULL)
    {
        return MFS_INVALID_MEMORY_BLOCK_ADDRESS;
    }

    sd_ptr = sp_ptr->SEARCH_DATA_PTR;

    error_code = MFS_lock_and_enter(drive_ptr, 0);
    if (error_code != MFS_NO_ERROR)
    {
        return error_code;
    }

    error_code = MFS_find_init(drive_ptr, sp_ptr, sd_ptr);

    if (error_code == MFS_NO_ERROR)
    {
        error_code = MFS_find_next_internal(drive_ptr, sd_ptr);
    }

    error_code_tmp = MFS_leave_and_unlock(drive_ptr, 0);
    if (error_code == MFS_NO_ERROR)
    {
        error_code = error_code_tmp;
    }

    return error_code;
}


/*!
 * \brief Find the next file that matches the input criteria.
 *
 * Must follow a Find_first_file and the search data block from the search
 * must be supplied.
 *
 * \param drive_ptr
 * \param[in,out] sd_ptr Address of search data block indicating the current criteria and the results of
 *                             the last search results of this search are placed in this data block
 *
 * \return _mfs_error Error code.
 */
_mfs_error MFS_find_next_file(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    MFS_SEARCH_DATA_PTR sd_ptr)
{
    _mfs_error error_code;
    _mfs_error error_code_tmp;

    error_code = MFS_lock_and_enter(drive_ptr, 0);
    if (error_code != MFS_NO_ERROR)
    {
        return error_code;
    }

    error_code = MFS_find_next_internal(drive_ptr, sd_ptr);

    error_code_tmp = MFS_leave_and_unlock(drive_ptr, 0);
    if (error_code == MFS_NO_ERROR)
    {
        error_code = error_code_tmp;
    }

    return error_code;
}
