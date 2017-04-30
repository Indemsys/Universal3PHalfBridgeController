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
*   This file contains functions related to using long filenames of up to
*   255 characters.
*
*
*END************************************************************************/

#include "mfs.h"
#include "mfs_prv.h"


/* Offsets of UTF-16 words in LFN entry */
static const int lfn_wchar_offset[] = { /* NAME0_4 */ 1, 3, 5, 7, 9, /* NAME 5_10 */ 14, 16, 18, 20, 22, 24, /* NAME11_12 */ 28, 30 };


/*!
 * \brief
 *
 * \param lfn_entry
 *
 * \return int
 */
int MFS_lfn_utf16_len(MFS_LNAME_ENTRY *lfn_entry)
{
    int padding = 0;

    if (lfn_entry->ID & MFS_LFN_END)
    {
        /* this entry contains end of the filename, skip padding FFs and 0 */
        uint8_t *wchar_ptr;
        int32_t wchar;
        int i;

        for (i = 13; i; i--)
        {
            wchar_ptr = ((uint8_t *)lfn_entry) + lfn_wchar_offset[i - 1];
            wchar = wchar_ptr[0] | (wchar_ptr[1] << 8);

            if ((wchar != 0xFFFF) && (wchar != 0x0000))
            {
                /* valid character, leave for further processing */
                break;
            }
        }
        padding = 13 - i;
    }

    return (lfn_entry->ID & MFS_LFN_ORD) * 13 - padding;
}


/*!
 * \brief Match single LFN entry against given UTF-8 string starting from given position backward
 *
 * \param utf8_str
 * \param utf8_pos
 * \param lfn_entry
 * \param surrogate_buf
 *
 * \return bool
 */
bool MFS_lfn_entry_match_r(char *utf8_str, char **utf8_pos, MFS_LNAME_ENTRY *lfn_entry, uint32_t *surrogate_buf)
{
    uint8_t *wchar_ptr;

    int32_t wchar_a;
    int32_t wchar_b;

    /* LFN record contains up to 13 UTF-16 words */
    int i = 13;

    if (lfn_entry->ID & MFS_LFN_END)
    {
        /* this entry contains end of the filename, skip padding FFs and 0 */
        for (; i; i--)
        {
            wchar_ptr = ((uint8_t *)lfn_entry) + lfn_wchar_offset[i - 1];
            wchar_a = wchar_ptr[0] | (wchar_ptr[1] << 8);

            if ((wchar_a != 0xFFFF) && (wchar_a != 0x0000))
            {
                /* valid character, leave for further processing */
                break;
            }
        }

        if (i == 0)
        {
            /* the entry does not contain any valid character, consider it non-matching */
            return false;
        }
    }

    /* compare valid characters */
    for (; i; i--)
    {
        wchar_ptr = ((uint8_t *)lfn_entry) + lfn_wchar_offset[i - 1];
        wchar_a = wchar_ptr[0] | (wchar_ptr[1] << 8);

        if ((wchar_a & 0xDC00) == 0xDC00)
        {
            /* low surrogate */
            if (*surrogate_buf != 0)
            {
                /* malformed UTF-16 encoding */
                return false;
            }
            *surrogate_buf = wchar_a;
            continue;
        }
        else if ((wchar_a & 0xDC00) == 0xD800)
        {
            /* high surrogate */
            if (*surrogate_buf == 0)
            {
                /* malformed UTF-16 encoding */
                return false;
            }
            /* merge surrogates */
            wchar_a = (wchar_a & 0x3FF) || ((*surrogate_buf & 0x3FF) << 10);
            *surrogate_buf = 0;
        }

        wchar_b = utf8_decode_r(utf8_str, utf8_pos);

        wchar_a = CAPITALIZE(wchar_a);
        wchar_b = CAPITALIZE(wchar_b);

        if (wchar_a != wchar_b)
        {
            return false;
        }
    }

    return true;
}


/*!
 * \brief
 *
 * Will take a long filename (actual file name or directory name
 * and will parse it into the corresponding 8.3 name. It is assumed that
 * there is space allocated for the short name.
 *
 * \param[in] l_fname The long filename.
 * \param[out] s_fname The short filename.
 *
 * \return _mfs_error
 */
int static MFS_lfn_to_sfn_tr(char **lfn_pos, char *lfn_boundary, char **sfn_pos, int sfn_len_limit, bool *fits_sfn, bool *needs_lfn)
{
    int32_t codepoint;
    int c;
    int sfn_len;

    sfn_len = 0;
    while ((sfn_len < sfn_len_limit) && (*lfn_pos < lfn_boundary))
    {
        codepoint = utf8_decode(lfn_pos, lfn_boundary);
        if (codepoint <= 0)
        {
            return -1;
        }

        switch (codepoint)
        {
            case ' ':
            case '.':
                *fits_sfn = false;
                *needs_lfn = true;
                /* do not copy dots and spaces */
                continue;

            /* characters not allowed in any name */
            case '\\':
            case '\"':
            case '*':
            case '?':
            case '<':
            case '>':
            case '|':
            case '/':
            case ':':
                return -1;

            /* characters not allowed in 8.3 names */
            case '+':
            case ',':
            case ';':
            case '=':
            case '[':
            case ']':
                *fits_sfn = false;
                *needs_lfn = true;
                /* translate to underscore */
                c = '_';
                break;

            default:
                if (codepoint > 0xFF)
                {
                    *fits_sfn = false;
                    *needs_lfn = true;
                    /* translate to underscore */
                    c = '_';
                }
                else
                {
                    c = CAPITALIZE(codepoint);
                    if (c != codepoint)
                    {
                        /* LFN is required to represent lowercase characters, but their capitalized form still fits SFN */
                        *needs_lfn = true;
                    }
                }
                break;
        }

        **sfn_pos = (char)c;
        (*sfn_pos)++;
        sfn_len++;
    }

    /* check if whole input name was consumed to create SFN */
    if (*lfn_pos != lfn_boundary)
    {
        *fits_sfn = false;
        *needs_lfn = true;
    }

    return sfn_len;
}


/*!
 * \brief
 *
 * Will take a long filename (actual file name or directory name
 * and will parse it into the corresponding 8.3 name. It is assumed that
 * there is space allocated for the short name.
 *
 * \param[in] lfn The long filename.
 * \param[out] sfn The short filename.
 *
 * \return _mfs_error
 */
_mfs_error MFS_lfn_to_sfn(char *lfn, char *sfn)
{
    char *parse_pos;
    char *dest_pos;

    char *dot_pos;
    char *name_boundary;
    char *ext_pos;
    char *ext_boundary;

    /* initially presume that the name is SFN compatible */
    bool fits_sfn_local = true;
    bool needs_lfn_local = false;

    /* split to filename and extension */
    parse_pos = lfn;

    dot_pos = NULL;
    ext_pos = NULL;
    name_boundary = NULL;

    /* skip leading dots */
    while (*parse_pos == '.')
    {
        parse_pos++;
    }

    while (*parse_pos != '\0')
    {
        if (*parse_pos == '.')
        {
            dot_pos = parse_pos;
        }
        else
        {
            if (dot_pos != NULL)
            {
                /* found non-dot character following a dot */
                name_boundary = dot_pos;  // set name boundary to the dot position
                ext_pos = parse_pos;  // and remember current position as start of the extension
            }
            dot_pos = NULL;
        }
        parse_pos++;
    }

    if (ext_pos != NULL)
    {
        /* valid extension found, set ext boundary; no need to deal with name boundary as it is for sure already set */
        ext_boundary = parse_pos;
    }
    else
    {
        /* set name boundary */
        name_boundary = parse_pos;
    }


    dest_pos = sfn;
    parse_pos = lfn;
    if (MFS_lfn_to_sfn_tr(&parse_pos, name_boundary, &dest_pos, 6, &fits_sfn_local, &needs_lfn_local) < 0)
    {
        return MFS_INVALID_PARAMETER;
    }

    *(dest_pos)++ = '~';
    *(dest_pos)++ = '1';

    if (ext_pos)
    {
        *(dest_pos)++ = '.';
        parse_pos = ext_pos;
        if (MFS_lfn_to_sfn_tr(&parse_pos, ext_boundary, &dest_pos, 3, &fits_sfn_local, &needs_lfn_local) < 0)
        {
            return MFS_INVALID_PARAMETER;
        }
    }

    *dest_pos = '\0';

    return MFS_NO_ERROR;
}


/*!
 * \brief
 *
 * Will take a long filename (actual file name or directory name
 * and will parse it into the corresponding 8.3 name. It is assumed that
 * there is space allocated for the short name.
 *
 * \param[in] lfn The long filename.
 * \param[out] sfn The short filename.
 *
 * \return _mfs_error
 */
_mfs_error MFS_lfn_to_sfn_disk(char *lfn, int lfn_len, uint8_t sfn_disk[11], bool *fits_sfn, bool *needs_lfn)
{
    char *parse_pos;
    char *dest_pos;
    char *dot_pos;

    char *parse_boundary;
    char *name_boundary;
    char *ext_pos;

    int i;

    bool fits_sfn_local;
    bool needs_lfn_local;

    if (fits_sfn == NULL)
        fits_sfn = &fits_sfn_local;

    if (needs_lfn == NULL)
        needs_lfn = &needs_lfn_local;

    /* initially presume that the name is SFN compatible */
    *fits_sfn = true;
    *needs_lfn = false;

    /* clear SFN buffer */
    for (i = 0; i < 11; i++)
    {
        sfn_disk[i] = ' ';
    }

    /* set parse boundary */
    parse_boundary = lfn + lfn_len;

    /* skip leading dots */
    parse_pos = lfn;
    while ((parse_pos < parse_boundary) && (*parse_pos == '.'))
    {
        parse_pos++;
    }

    /* check for '.' and '..' which require special handling */
    if ((parse_pos == parse_boundary) && (lfn_len <= 2))
    {
        for (i = 0; i < lfn_len; i++)
        {
            sfn_disk[i] = '.';
        }
        return MFS_NO_ERROR;
    }

    /* split to filename and extension */
    dot_pos = NULL;
    ext_pos = NULL;
    name_boundary = parse_boundary;

    while (parse_pos < parse_boundary)
    {
        if (*parse_pos == '.')
        {
            dot_pos = parse_pos;
        }
        else
        {
            if (dot_pos != NULL)
            {
                /* found non-dot character following a dot */
                name_boundary = dot_pos;  // set name boundary to the dot position
                ext_pos = parse_pos;  // and remember current position as start of the extension
            }
            dot_pos = NULL;
        }
        parse_pos++;
    }

    /* store filename */
    dest_pos = (char *)sfn_disk;
    parse_pos = lfn;
    if (MFS_lfn_to_sfn_tr(&parse_pos, name_boundary, &dest_pos, 8, fits_sfn, needs_lfn) < 0)
    {
        return MFS_INVALID_PARAMETER;
    }

    /* store extension, if any */
    if (ext_pos)
    {
        dest_pos = (char *)(sfn_disk + 8);
        parse_pos = ext_pos;
        if (MFS_lfn_to_sfn_tr(&parse_pos, parse_boundary, &dest_pos, 3, fits_sfn, needs_lfn) < 0)
        {
            return MFS_INVALID_PARAMETER;
        }
    }

    return MFS_NO_ERROR;
}


/*!
 * \brief
 *
 * Takes a SFN template and compares it with actual SFN.
 * If the SFN matches the template then index following the tilde character is returned, otherwise the return value is zero.
 *
 * \param[in] SFN template
 * \param[in] SFN to match
 *
 * \return index of the LFN alias (numeric part of SFN which follows the tilde character, converted to int)
 */
int MFS_lfn_alias_index(uint8_t sfn_template[11], uint8_t sfn[11])
{
    int i;
    int tilde_pos;
    int index;

    /* compare extension */
    for (i = 8; i < 11; i++)
    {
        if (sfn_template[i] != sfn[i])
        {
            return 0;
        }
    }

    /* the first character must always match */
    if (sfn_template[0] != sfn[0])
    {
        return 0;
    }

    /* look for '~' character from 2nd to 7th position of sfn */
    for (i = 1; i < 7; i++)
    {
        if (sfn[i] == '~')
        {
            tilde_pos = i;
            break;
        }
        if (sfn_template[i] != sfn[i])
        {
            return 0;
        }
    }

    if (i >= 7)
    {
        /* no tilde found in the sfn */
        return 0;
    }

    i++;
    index = 0;

    for (; i < 8; i++)
    {
        index *= 10;
        if (sfn[i] >= '0' && sfn[i] <= '9')
        {
            index += (sfn[i] - '0');
        }
        else if (sfn[i] == ' ')
        {
            break;
        }
        else
        {
            return 0;
        }
    }

    if (i < 8)
    {
        /* check if the rest of the name is filled with spaces */
        for (; i < 8; i++)
        {
            if (sfn[i] != ' ')
            {
                return 0;
            }
        }

        /* the indexed name does not span whole 8 characters, match only if template length corresponds with this */
        if (sfn_template[tilde_pos] != '~' && sfn_template[tilde_pos] != ' ')
        {
            return 0;
        }
    }

    return index;
}


/*!
 * \brief
 *
 * Will take a long filename (actual file name or directory name)
 * and will validate it by checking for length and for invalid characters.
 *
 * \param[in] l_fname The long filename.
 *
 * \return bool
 */
bool MFS_is_valid_lfn(
    char *l_fname)
{
    register uint32_t len;
    register uint32_t i;
    register char c;
#if defined(__GNUC__)
    // FIXME: workaround for gcc release version of qspi flash xip images, to
    // make sure the address of the string is 4-byte aligned. Otherwise, it
    // might be put a non-4-byte aligned address, then the access will abort.
    static const char illegal[] __attribute__((aligned(0x4))) = "*?<>|\":/\\";
#else
    const char illegal[] = "*?<>|\":/\\";
#endif
    bool ok_char_flag;

    ok_char_flag = false;
    len = 0;

    /* process characters until null term found */
    while ('\0' != (c = *l_fname++))
    {
        /* check maximum lfn length */
        if (++len > FILENAME_SIZE)
        {
            return (false);
        }

        /* dot and space are allowed but at least one different character is necessary to form a valid lfn */
        if (c == '.' || c == ' ')
        {
            continue;
        }

        /* check c against list of illegal characters */
        for (i = 0; illegal[i]; i++)
        {
            if (c == illegal[i])
            {
                return (false);
            }
        }

        /* valid character found */
        ok_char_flag = true;
    }

    return (ok_char_flag);
}


/*!
 * \brief
 *
 *  Allows . and ..
 *
 * \param[in] filename Filename to validate.
 *
 * \return bool True if valid LFN directory name.
 */
bool MFS_lfn_dirname_valid(
    char *filename)
{

    if (filename[0] == '.')
    {
        if (filename[1] == '\0')
        {
            return true;  // '.'
        }
        if (filename[1] == '.' && filename[2] == '\0')
        {
            return true;  // '..'
        }
    }

    return MFS_is_valid_lfn(filename);
}


/*!
 * \brief
 *
 * Takes a LFN directory entry, extracts the 13 UTF16 code units
 * from the entry and stores it stripping the upper byte.
 * This conversion is compatible with ASCII characters
 * but looses information if the filename contains characters above 0xFF.
 *
 * \param[in] lname_entry Pointer to the entry to read UNICODE name from.
 * \param[out] filename Buffer to store the 13 chars in.
 *
 * \return int
 */
int MFS_lfn_extract(
    MFS_LNAME_ENTRY_PTR lname_entry,
    char *filename)
{
    int i;

    char *output;
    int output_len;

    unsigned char *utf16_chunk;
    uint16_t utf16_unit;

    output = filename;
    output_len = 0;

    utf16_chunk = lname_entry->NAME0_4;
    for (i = 0; i < 10; i += 2)
    {
        utf16_unit = utf16_chunk[0] | (utf16_chunk[1] << 8);
        utf16_chunk += 2;
        if (utf16_unit == 0 || utf16_unit == 0xFFFF)
        {
            return output_len;
        }
        *output++ = utf16_unit & 0xFF;
        output_len++;
    }

    utf16_chunk = lname_entry->NAME5_10;
    for (i = 0; i < 12; i += 2)
    {
        utf16_unit = utf16_chunk[0] | (utf16_chunk[1] << 8);
        utf16_chunk += 2;
        if (utf16_unit == 0 || utf16_unit == 0xFFFF)
        {
            return output_len;
        }
        *output++ = utf16_unit & 0xFF;
        output_len++;
    }

    utf16_chunk = lname_entry->NAME11_12;
    for (i = 0; i < 4; i += 2)
    {
        utf16_unit = utf16_chunk[0] | (utf16_chunk[1] << 8);
        utf16_chunk += 2;
        if (utf16_unit == 0 || utf16_unit == 0xFFFF)
        {
            return output_len;
        }
        *output++ = utf16_unit & 0xFF;
        output_len++;
    }

    return output_len;
}


/*!
 * \brief
 *
 * Takes a compressed short filename (that represents a lfn) and
 * increments the number after the ~
 *
 * \param[in,out] filename A short compressed filename (a valid one).
 *
 * \return void
 */
_mfs_error MFS_increment_lfn(
    char *filename)
{
    int32_t i;
    uint32_t num;
    uint32_t power;

    for (i = 0; (i < 8) && (filename[i] != '.'); i++)
        ;

    if (i == 0)
    {
        return MFS_INVALID_PARAMETER;
    }

    /* Find out where the ~ starts and get the number after the ~ */
    for (i--, power = 1, num = 0; i >= 0; i--)
    {
        if (filename[i] == '~')
        {
            break;
        }
        if ((filename[i] < '0') || (filename[i] > '9'))
        {
            return MFS_INVALID_PARAMETER;
        }
        /* Translate the ASCII value to an int value */
        num += power * (uint32_t)(filename[i] - '0');
        power *= 10;
    }

    num++;

    if (num >= power)
    {
        if (i <= 2)
        {
            return MFS_FILE_EXISTS;  // cannot create unique alias
        }
        filename[--i] = '~';
        power *= 10;
    }

    /* Write the new number */
    for (power /= 10; power; power /= 10)
    {
        /* Translate the int value into an ASCII value */
        filename[++i] = (unsigned char)(num / power) + '0';
        num %= power;
    }

    return MFS_NO_ERROR;
}

#if !MFSCFG_READ_ONLY
/*!
 * \brief
 *
 * Removes the extra entries (corresponding to the dir_entry_ptr
 * containing the LFN). Can be used on a normal dir entry, but does
 * nothing in this case.
 *
 * \param[in] drive_ptr Drive on which to operate.
 * \param[in] parent_cluster The cluster in which the entry lies.
 * \param[in] dir_index The index of where in the cluster the entry lies.
 * \param[in] prev_cluster The cluster previous to the cluster in which the entry lies.
 *
 * \return _mfs_error
 */
_mfs_error MFS_remove_lfn_entries(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    uint32_t parent_cluster,
    uint32_t dir_index,
    uint32_t prev_cluster)
{
    MFS_LNAME_ENTRY_PTR lname_entry = NULL;
    bool last_entry_flag = false;
    uint32_t current_index;
    _mfs_error error_code = MFS_NO_ERROR;
    uint32_t temp_prev_cluster;
    bool went_back = false;
    bool start_deletion = false;

    /* As long as we haven't found the EOF flag in the LFN name */
    while (last_entry_flag == false)
    {
        /* Check to see if the LFN spans to different sector */
        if (dir_index == 0)
        {
            /* Check to see if we are in root directory */
            if (parent_cluster == 0)
            {
                break;
            }
            else if (drive_ptr->FAT_TYPE == MFS_FAT32)
            {
                if (drive_ptr->ROOT_CLUSTER == parent_cluster)
                {
                    break;
                }
            }
            if (!went_back)
            {
                error_code = MFS_get_prev_cluster(drive_ptr, &parent_cluster, prev_cluster);
                went_back = true;
            }
            else
            {
                if (start_deletion)
                {
                    error_code = MFS_FAILED_TO_DELETE_LFN;
                }
                break;
            }
            dir_index = drive_ptr->ENTRIES_PER_SECTOR << drive_ptr->CLUSTER_POWER_SECTORS;
        }

        current_index = dir_index;

        if (!error_code)
        {
            dir_index--;
            lname_entry = (MFS_LNAME_ENTRY_PTR)MFS_Find_directory_entry(drive_ptr,
                                                                        NULL,
                                                                        &parent_cluster,
                                                                        &dir_index,
                                                                        &temp_prev_cluster,
                                                                        MFS_ATTR_LFN,
                                                                        &error_code);
        }

        /* If a LFN, mark entry as deleted */
        if ((lname_entry != NULL) && !error_code && (dir_index < current_index))
        {
            if (lname_entry->ID & MFS_LFN_END)
            {
                last_entry_flag = true;
            }

            lname_entry->ID = MFS_DEL_FILE;
            drive_ptr->DIR_SECTOR_DIRTY = true;

            start_deletion = true;
        }
        else
        {
            break;
        }
    }

    return (error_code);
}


/*!
 * \brief  Takes a string of up to 13 characters and writes them to a LFN directory entry.
 *
 * \param[in] filename A string up to 13 chars long, to be put in the entry.
 * \param[out] entry_ptr The LNAME entry in which the string is places.
 *
 * \return _mfs_error
 */
_mfs_error MFS_lfn_name_to_entry(
    char *filename,
    MFS_LNAME_ENTRY_PTR entry_ptr)
{
    uint32_t i;
    bool end_of_name = false;

    if (entry_ptr == NULL)
    {
        return (MFS_CANNOT_CREATE_DIRECTORY);
    }


    /* The entry contains UNICODE chars... Which is why i = i +2 */
    /* Copy chars 0 to 4 */
    for (i = 0; i <= 9; i = i + 2)
    {
        if (*filename)
        {
            mqx_htods((entry_ptr->NAME0_4 + i), (uint16_t)*filename);
            filename++;
        }
        else if (!end_of_name)
        {
            mqx_htods((entry_ptr->NAME0_4 + i), 0);
            end_of_name = true;
        }
        else
        {
            mqx_htods((entry_ptr->NAME0_4 + i), 0xFFFF);
        }
    }

    /* Copy chars 5 to 10 */
    for (i = 0; i <= 11; i = i + 2)
    {
        if (*filename)
        {
            mqx_htods((entry_ptr->NAME5_10 + i), (uint16_t)*filename);
            filename++;
        }
        else if (!end_of_name)
        {
            mqx_htods((entry_ptr->NAME5_10 + i), 0);
            end_of_name = true;
        }
        else
        {
            mqx_htods((entry_ptr->NAME5_10 + i), 0xFFFF);
        }
    }

    /* Copy chars 11 to 12 */
    for (i = 0; i <= 3; i = i + 2)
    {
        if (*filename)
        {
            mqx_htods((entry_ptr->NAME11_12 + i), (uint16_t)*filename);
            filename++;
        }
        else if (!end_of_name)
        {
            mqx_htods((entry_ptr->NAME11_12 + i), 0);
            end_of_name = true;
        }
        else
        {
            mqx_htods((entry_ptr->NAME11_12 + i), 0xFFFF);
        }
    }


    return (MFS_NO_ERROR);
}

#endif


/*!
 * \brief
 *
 * Takes a valid expanded filename of 11 characters (doesn't need
 * to be terminated by a NULL char) and calculates the LFN checksum
 * value for it.
 *
 * \param[in] filename_ptr Pointer to filename on which to calculate checksum.
 *
 * \return unsigned char Checksum
 */
unsigned char MFS_lfn_checksum(
    uint8_t *sfn_disk)
{
    unsigned char sum;
    uint32_t i;

    for (sum = 0, i = 0; i < 11; i++)
    {
        sum = (((sum & 1) << 7) | ((sum & 0xfe) >> 1)) + *sfn_disk++;
    }

    return (sum);
}


/*!
 * \brief Finds the previous cluster in the cluster chain for any given cluster.
 *
 * \param[in] drive_ptr The drive on which to operate.
 * \param[in,out] cluster_ptr Cluster to perform search with / previous cluster.
 * \param[in] previous_cluster The previous cluster.
 *
 * \return _mfs_error
 */
_mfs_error MFS_get_prev_cluster(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    uint32_t *cluster_ptr,
    uint32_t previous_cluster)
{
    uint32_t i;
    _mfs_error error_code = MFS_NO_ERROR;
    uint32_t next_cluster;

    /* Check to see if we are at the very first cluster */
    if (drive_ptr->FAT_TYPE == MFS_FAT32)
    {
        if (*cluster_ptr == drive_ptr->ROOT_CLUSTER)
        {
            return MFS_INVALID_CLUSTER_NUMBER;
        }
    }
    else if (*cluster_ptr == 0)
    {
        return MFS_INVALID_CLUSTER_NUMBER;
    }

    // If we have been given a previous cluster
    if (previous_cluster != CLUSTER_INVALID)
    {
        // Make sure it is before the current cluster
        MFS_get_cluster_from_fat(drive_ptr, previous_cluster, &next_cluster);
        if (*cluster_ptr == next_cluster)
        {
            // It is, so use it
            *cluster_ptr = previous_cluster;
            return MFS_NO_ERROR;
        }
    }

    if (drive_ptr->FAT_TYPE == MFS_FAT32)
    {
        return MFS_INVALID_CLUSTER_NUMBER;
    }

    /* Search the rest of the fat for the wanted cluster */
    for (i = CLUSTER_MIN_GOOD; i <= drive_ptr->LAST_CLUSTER &&
                                   !error_code;
         i++)
    {
        MFS_get_cluster_from_fat(drive_ptr, i, &next_cluster);
        if (*cluster_ptr == next_cluster)
        {
            *cluster_ptr = i;
            break;
        }
    }

    if (i > drive_ptr->LAST_CLUSTER)
    {
        error_code = MFS_INVALID_CLUSTER_NUMBER;
    }

    return (error_code);
}


/*!
 * \brief Extracts single LFN entry and stores it from the beginning of the buffer in reverse order (mirrored), returns number of buffer bytes taken
 *
 * \param[in] lfn_entry
 * \param[in/out] utf8_pos
 * \param[in] utf8_boundary
 * \param[in/out] surrogate_buf
 *
 * \return int
 */
int MFS_lfn_entry_extract_r(MFS_LNAME_ENTRY *lfn_entry, char **utf8_pos, char *utf8_boundary, uint32_t *surrogate_buf)
{
    uint8_t *wchar_ptr;
    int32_t wchar;

    int len = 0;
    int tmp_len;

    /* LFN record contains up to 13 UTF-16 words */
    int i = 13;

    if (lfn_entry->ID & MFS_LFN_END)
    {
        /* this entry contains end of the filename, skip padding FFs and 0 */
        for (; i; i--)
        {
            wchar_ptr = ((uint8_t *)lfn_entry) + lfn_wchar_offset[i - 1];
            wchar = wchar_ptr[0] | (wchar_ptr[1] << 8);

            if ((wchar != 0xFFFF) && (wchar != 0x0000))
            {
                /* valid character, leave for further processing */
                break;
            }
        }
    }

    /* extract valid characters */
    for (; i; i--)
    {
        wchar_ptr = ((uint8_t *)lfn_entry) + lfn_wchar_offset[i - 1];
        wchar = wchar_ptr[0] | (wchar_ptr[1] << 8);

        if ((wchar & 0xDC00) == 0xDC00)
        {
            /* low surrogate */
            if (*surrogate_buf != 0)
            {
                /* malformed UTF-16 encoding */
                return -1;
            }
            *surrogate_buf = wchar;
            continue;
        }
        else if ((wchar & 0xDC00) == 0xD800)
        {
            /* high surrogate */
            if (*surrogate_buf == 0)
            {
                /* malformed UTF-16 encoding */
                return -1;
            }
            wchar = (wchar & 0x3FF) || ((*surrogate_buf & 0x3FF) << 10);
            *surrogate_buf = 0;
        }

        tmp_len = utf8_encode_r(wchar, utf8_pos, utf8_boundary);
        if (tmp_len > 0)
        {
            len += tmp_len;
        }
        else
        {
            return -1;
        }
    }

    return len;
}


/*!
 * \brief Reads UTF8 string in reverse order (from the end) and stores as much fits into the LFN entry. The function assumes that the input sting always contains enough characters (underflow is not handled).
 *
 * \param [out] lfn_entry
 * \param utf8_pos[in/out] pointer to next character of the name to be stored
 * \param ord_ptr[in/out] ordinal number of the entry (automatically adjusted)
 * \param chksum[in] alias checksum
 * \param padding[in] number of padding words
 *
 * \return int
 */
_mfs_error MFS_lfn_entry_store_r(MFS_LNAME_ENTRY *lfn_entry, char **utf8_pos, int *ord_ptr, int chksum, int padding, uint32_t *surrogate_buf)
{
    uint8_t *wchar_ptr;
    int32_t wchar;

    if ((*ord_ptr & MFS_LFN_ORD) == 0)
    {
        return MFS_INVALID_PARAMETER;
    }

    if ((*ord_ptr & MFS_LFN_END) == 0)
    {
        padding = 0;  // padding is used only for the last (tail) entry
    }
    else if (padding > 12)
    {
        return MFS_INVALID_PARAMETER;
    }

    /* LFN record contains up to 13 UTF-16 words */
    int i = 13;

    /* store padding, if any */
    while (padding > 0)
    {
        wchar_ptr = ((uint8_t *)lfn_entry) + lfn_wchar_offset[i - 1];
        /*
        LFN specification: LFN is terminated by null character but only if there is space for it in the tail entry.
        Implementation: The last stored padding character is used as the optional null terminator.
        */
        wchar = ((padding == 1) ? 0x00 : 0xFF);
        wchar_ptr[0] = wchar_ptr[1] = wchar;
        i--;
    }

    /* store the name re-encoding it from UTF-8 to UTF-16 */
    for (; i; i--)
    {
        wchar_ptr = ((uint8_t *)lfn_entry) + lfn_wchar_offset[i - 1];

        if (*surrogate_buf != 0)
        {
            /* get high surrogate from the surrogate buf */
            wchar = (*surrogate_buf);
            *surrogate_buf = 0;
        }
        else
        {
            wchar = utf8_decode_r(NULL, utf8_pos);

            if (wchar <= 0)
            {
                /* invalid string: this shall not happen, this function assumes that the string was checked for validity by the caller */
                return MFS_INVALID_PARAMETER;
            }

            if (wchar > 0xFFFF)
            {
                /* split codepoint to surrogates */
                *surrogate_buf = (wchar >> 10) | 0xD800;  // keep high surrogate between iterations/calls
                wchar = (wchar & 0x3FF) | 0xDC00;
            }
        }

        wchar_ptr[0] = wchar & 0xFF;
        wchar_ptr[1] = wchar >> 8;
    }

    lfn_entry->ALIAS_CHECKSUM = chksum;
    lfn_entry->ID = *ord_ptr;
    *ord_ptr = (*ord_ptr & MFS_LFN_ORD) - 1;  // decrement ordinal number;

    return MFS_NO_ERROR;
}


/*!
 * \brief
 *
 * \param[in] drive_ptr The drive on which to operate.
 * \param dir_chain
 * \param location
 * \param[out] lfn_buf The long file name.
 * \param[in] lfn_buf_len Length of the buffer for LFN.
 *
 * \return _mfs_error
 */
_mfs_error MFS_lfn_chain_extract(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    FAT_CHAIN *dir_chain,
    uint32_t location,
    char *lfn_buf,
    int lfn_buf_len)
{
    _mfs_error error_code;
    _mfs_error error_code_tmp;

    uint32_t sector_num;
    uint32_t sector_count;

    uint32_t index;
    uint32_t first_index;

    DIR_ENTRY_DISK *dir_entry_ptr;

    int lfn_ord;  // ordinal of last entry of the LFN being scanned
    int lfn_chksum;  // alias checksum of the LFN being scanned, -1 when no LFN is being processed

    char *lfn_pos;
    char *lfn_buf_end;

    uint32_t surrogate_buf;

    bool finished;

    sector_count = 0;
    finished = false;

    lfn_buf_end = lfn_buf_len ? lfn_buf + lfn_buf_len : NULL;
    lfn_pos = lfn_buf;
    *lfn_pos++ = '\0';  // this will become the null terminator for the final string

    lfn_ord = 0;
    lfn_chksum = -1;
    surrogate_buf = 0;

    first_index = (location % drive_ptr->SECTOR_SIZE) / sizeof(DIR_ENTRY_DISK);

    while (!finished)
    {
        if (sector_count > 0)
        {
            /* there is an unprocessed sector located during previous iteration, use it */
            sector_num++;
            sector_count--;
        }
        else
        {
            error_code = MFS_chain_locate(drive_ptr, dir_chain, location, 0, &sector_num, &sector_count);
            if (error_code != MFS_NO_ERROR)
            {
                break;
            }
        }

        error_code = MFS_sector_map(drive_ptr, sector_num, (void **)&dir_entry_ptr, MFS_MAP_MODE_READONLY, 0);
        if (error_code != MFS_NO_ERROR)
        {
            break;
        }
        dir_entry_ptr += first_index;

        /* process the entry slots within the sector */
        for (index = first_index; index < drive_ptr->ENTRIES_PER_SECTOR; index++)
        {
            if ((dir_entry_ptr->NAME[0] == 0) || (dir_entry_ptr->NAME[0] == MFS_DEL_FILE) || ((dir_entry_ptr->ATTRIBUTE[0] & MFS_ATTR_LFN_MASK) != MFS_ATTR_LFN))
            {
                /* non-LFN reached, exit the loop */
                error_code = MFS_FILE_NOT_FOUND;
                finished = true;
                break;
            }

            MFS_LNAME_ENTRY *lfn_entry_ptr = (MFS_LNAME_ENTRY *)dir_entry_ptr;
            if (lfn_entry_ptr->ID & MFS_LFN_END)
            {
                /* this is the tail LFN entry, check if it is expected */
                if (lfn_ord != 0)
                {
                    /* unexpected LFN record */
                    error_code = MFS_FILE_NOT_FOUND;
                    finished = true;
                    break;
                }
                lfn_ord = lfn_entry_ptr->ID & MFS_LFN_ORD;
                lfn_chksum = lfn_entry_ptr->ALIAS_CHECKSUM;
            }
            else if ((lfn_chksum != lfn_entry_ptr->ALIAS_CHECKSUM) || (--lfn_ord != lfn_entry_ptr->ID))
            {
                /* unexpected LFN record */
                error_code = MFS_FILE_NOT_FOUND;
                finished = true;
                break;
            }

            /* extract single LFN entry */
            if (MFS_lfn_entry_extract_r(lfn_entry_ptr, &lfn_pos, lfn_buf_end, &surrogate_buf) < 0)
            {
                /* malformed LFN record */
                error_code = MFS_FILE_NOT_FOUND;
                finished = true;
                break;
            }

            if (lfn_ord == 1)
            {
                /* last LFN slot processed */
                mem_reverse(lfn_buf, lfn_pos - 1);
                finished = true;
                break;
            }

            dir_entry_ptr++;
            location += sizeof(DIR_ENTRY_DISK);
        }
        first_index = 0;

        error_code_tmp = MFS_sector_unmap(drive_ptr, sector_num, 0);
        if (error_code == MFS_NO_ERROR)
        {
            error_code = error_code_tmp;
        }

        if (error_code != MFS_NO_ERROR)
        {
            break;
        }
    }

    return error_code;
}


/*!
 * \brief
 *
 * \param[in] drive_ptr The drive on which to operate.
 * \param[in] dir_chain
 * \param[in] location
 * \param[in] lfn The long file name to store.
 * \param[in] lfn_chksum Checksum of the SFN entry to associate the LFN entries with.
 *
 * \return _mfs_error
 */
_mfs_error MFS_lfn_chain_store(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    FAT_CHAIN *dir_chain,
    uint32_t location,
    char *lfn,
    int lfn_chksum)
{
    _mfs_error error_code;
    _mfs_error error_code_tmp;

    uint32_t sector_num;
    uint32_t sector_count;

    uint32_t index;
    uint32_t first_index;

    MFS_LNAME_ENTRY *lfn_entry_ptr;

    int lfn_len;
    int lfn_len_utf16;

    int lfn_entries;
    int lfn_padding;
    int lfn_ord;  // ordinal of the entry being stored

    char *lfn_pos;

    uint32_t surrogate_buf;

    bool finished;

    /* calculate name length in UTF-16 words; this also checks UTF-8 validity of the string */
    lfn_len = MFS_path_component_len(lfn, NULL, &lfn_len_utf16);
    if (lfn_len <= 0 || lfn_len > (13 * MFS_LFN_ORD))
    {
        return MFS_INVALID_PARAMETER;
    }

    lfn_entries = (lfn_len_utf16 + 12) / 13;
    lfn_padding = (lfn_entries * 13) - lfn_len_utf16;
    lfn_ord = lfn_entries | MFS_LFN_END;
    lfn_pos = lfn + lfn_len - 1;
    surrogate_buf = 0;

    first_index = (location % drive_ptr->SECTOR_SIZE) / sizeof(DIR_ENTRY_DISK);

    sector_count = 0;
    finished = false;

    while (!finished)
    {
        if (sector_count > 0)
        {
            /* there is an unprocessed sector located during previous iteration, use it */
            sector_num++;
        }
        else
        {
            error_code = MFS_chain_locate(drive_ptr, dir_chain, location, 0, &sector_num, &sector_count);
            if (error_code != MFS_NO_ERROR)
            {
                break;
            }
        }
        sector_count--;

        error_code = MFS_sector_map(drive_ptr, sector_num, (void **)&lfn_entry_ptr, MFS_MAP_MODE_MODIFY, 0);
        if (error_code != MFS_NO_ERROR)
        {
            break;
        }
        lfn_entry_ptr += first_index;

        /* process all entry slots in the sector */
        for (index = first_index; index < drive_ptr->ENTRIES_PER_SECTOR; index++)
        {
            error_code = MFS_lfn_entry_store_r(lfn_entry_ptr, &lfn_pos, &lfn_ord, lfn_chksum, lfn_padding, &surrogate_buf);
            if (error_code != MFS_NO_ERROR)
            {
                break;
            }
        }
        first_index = 0;

        error_code_tmp = MFS_sector_unmap(drive_ptr, sector_num, 1);
        if (error_code == MFS_NO_ERROR)
        {
            error_code = error_code_tmp;
        }

        if (error_code != MFS_NO_ERROR)
        {
            break;
        }
    }

    return error_code;
}


/*!
 * \brief
 *
 * Will take an a pathname that points to a normal 8.3 entry.
 * If this entry has a LFN, the LFN will be placed in the lfn buffer.
 * Assumes the drive is locked.
 *
 * \param[in] drive_ptr The drive on which to operate.
 * \param[in] filepath The filepath to the 8.3 filename.
 * \param[out] lfn The long file name, if it exists.
 *
 * \return _mfs_error
 */
_mfs_error MFS_get_lfn(
    MFS_DRIVE_STRUCT_PTR drive_ptr,
    char *filepath,
    char *lfn)
{
    _mfs_error error_code;
    FAT_CHAIN dir_chain;

    char *entry_name;
    uint32_t entry_loc;

    /* Find the directory in which the file shall be located */
    error_code = MFS_get_dir_chain(drive_ptr, filepath, &dir_chain, NULL, &entry_name);
    if (error_code == MFS_NO_ERROR)
    {
        /* Lookup entry  with the requested name in the directory */
        error_code = MFS_scan_dir_chain(drive_ptr, &dir_chain, entry_name, NULL, NULL, NULL, &entry_loc);
    }

    if (error_code == MFS_NO_ERROR)
    {
        /* Extract LFN */
        error_code = MFS_lfn_chain_extract(drive_ptr, &dir_chain, entry_loc, lfn, 0);
    }

    return error_code;
}
