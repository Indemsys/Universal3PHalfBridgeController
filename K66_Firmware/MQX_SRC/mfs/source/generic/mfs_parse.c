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
*   This file contains functions to parse and manipulate filenames.
*
*
*END************************************************************************/

#include <string.h>

#include "mfs.h"
#include "mfs_prv.h"


/*!
 * \brief Take a pathname and convert it into its directory name and filename components.
 *
 * \param[in,out] dirname_ptr Pointer to buffer where directory is to be written.
 * \param[in,out] filename_ptr Pointer to buffer where filename is to be written.
 * \param[in] pathname_ptr Pathname include '\', not including drive specification.
 *
 * \return _mfs_error Error code.
 */
_mfs_error MFS_Parse_pathname(
    char *dirname_ptr,
    char *filename_ptr,
    char *pathname_ptr)
{
    _mfs_error error_code;
    int len;
    char *src;
    char *dst;
    char *dst_end;
    char *dst_boundary;

    if (pathname_ptr == NULL)
    {
        return MFS_INVALID_PARAMETER;
    }

    error_code = MFS_NO_ERROR;

    src = pathname_ptr;
    dst = dirname_ptr;
    dst_boundary = dirname_ptr + PATHNAME_SIZE;

    if (*src == '\\' || *src == '/')
    {
        *dst++ = '\\';
        src++;
    }
    dst_end = dst;

    while (*src)
    {
        src = MFS_Parse_next_filename(src, filename_ptr);
        if (src == NULL)
        {
            /* if the next filename was not a valid filename */
            error_code = MFS_PATH_NOT_FOUND;
            break;
        }
        else if (*src)
        {
            if (MFS_lfn_dirname_valid(filename_ptr) == false)
            {
                error_code = MFS_PATH_NOT_FOUND;
                break;
            }

            len = strlen(filename_ptr);
            if (dst + len >= dst_boundary)
            {
                error_code = MFS_INSUFFICIENT_MEMORY;
                break;
            }

            strcpy(dst, filename_ptr);
            dst += len;
            dst_end = dst;
            *dst++ = '\\';  // this eventually gets overwritten by null terminator
        }
    }

    if (error_code != MFS_NO_ERROR)
    {
        *dirname_ptr = *filename_ptr = '\0';
    }
    else
    {
        *dst_end = '\0';
    }

    return error_code;
}


/*!
 * \brief Put a dot-format filename in non-dot format. Assumes a valid DOS filename.
 *
 * It maps non_dot_file[0] from 0xE5 to 0x05 if necessary.
 *
 * \param[in] dot_file Pointer to the file's dot_name.
 * \param[in] non_dot_file Pointer to the file's non-dot_name.
 *
 * \return void
 */
void MFS_Expand_dotfile(
    char *sfn,
    uint8_t sfn_disk[11])
{
    uint16_t k;
    unsigned char c;

    unsigned char *dot_file = (unsigned char *)sfn;
    uint8_t *non_dot_file = sfn_disk;

    if (*dot_file == '.')
    {
        *non_dot_file++ = *dot_file++;
        for (k = 0; k < 10; k++)
        {
            c = *dot_file;
            if (c && !IS_DELIMITER(c))
            {
                *non_dot_file++ = CAPITALIZE(c);
                dot_file++;
            }
            else
            {
                *non_dot_file++ = ' ';
            }
        }
        return;
    }

    for (k = 0; k < 8; k++)
    {
        c = *dot_file;
        if (c && c != '.' && !IS_DELIMITER(c))
        {
            *non_dot_file++ = CAPITALIZE(c);
            dot_file++;
        }
        else
        {
            *non_dot_file++ = ' ';
        }
    }

    if (*dot_file == '.')
    {
        dot_file++;
    }
    for (k = 0; k < 3; k++)
    {
        c = *dot_file;
        if (c && c != '.' && !IS_DELIMITER(c))
        {
            *non_dot_file++ = CAPITALIZE(c);
            dot_file++;
        }
        else
        {
            *non_dot_file++ = ' ';
        }
    }

    if (*sfn_disk == MFS_DEL_FILE)
    {
        *sfn_disk = 0x05;
    }
}


/*!
 * \brief Checks for length, invalid characters.
 *
 * \param[in] filename Filename to validate.
 *
 * \return bool True if valid DOS filename.
 */
bool MFS_Filename_valid(
    char *filename)
{
    char c;
    char *s;

    int name_len;
    int ext_len;
    int dot_found;

    if (filename == NULL)
    {
        return false;
    }

    s = filename;

    name_len = 0;
    ext_len = 0;
    dot_found = 0;

    for (c = *s; c != '\0'; c = *(++s))
    {

        if (c == '.')
        {
            if (dot_found)
            {
                /* dot may occur only once */
                return false;
            }

            if (name_len == 0)
            {
                /* blank filename not allowed */
                return false;
            }

            dot_found++;

            continue;
        }

        if (c == ' ' || c == '\"' || c == '*' || c == '+' ||
            c == ',' || c == '/' || c == ':' || c == ';' ||
            c == '<' || c == '=' || c == '>' || c == '?' ||
            c == '[' || c == '\\' || c == ']' || c == '|')
        {
            return false;
        }

        if (!dot_found)
        {
            if (++name_len > 8)
            {
                return false; /* name too long */
            }
        }
        else
        {
            if (++ext_len > 3)
            {
                return false; /* extension too long */
            }
        }
    }

    /* only non-blank filenames are valid */
    return (name_len > 0);
}


/*!
 * \brief  Allows . and ..
 *
 * \param[in] filename Filename to validate.
 *
 * \return bool True if valid DOS directory name.
 */
bool MFS_Dirname_valid(
    char *filename)
{

    if (*filename != '.')
    {
        return (MFS_Filename_valid(filename));
    }
    else
    {
        if ((*(filename + 1) == '\0') ||
            ((*(filename + 1) == '.') && !*(filename + 2)))
        {
            return (true);
        }
    }

    return (false);
}


/*!
 * \brief Reads the first filename in source string, up to '\' ,'/' or '.'
 *
 *  If the filename we're reading is invalid, return NULL
 *
 * \param[in] src The pathanme.
 * \param[in,out] out The filename just read in.
 *
 * \return char * Pointer to where it left off (non / or \ char)
 */
char *MFS_Parse_next_filename(
    char *src,
    char *out)
{
    uint32_t i;

    if (out == NULL)
    {
        return (NULL);
    }

    i = 0;

    while (*src && *src != '/' && *src != '\\')
    {
        *out++ = *src++;
        i++;
        if (i > FILENAME_SIZE)
        {
            return NULL;
        }
    }

    *out = '\0';
    if (*src == '\\' || *src == '/')
    {
        src++;
    }

    return (src);
}


/*!
 * \brief
 *
 * It checks if the filename is on of '.' or '..' and returns 1 or 2
 * respectively. Otherwise, it returns 0.
 *
 * \param[in] filename Pointer to the filename to be checked.
 *
 * \return uint16_t The number of dots in the filename.
 */
uint16_t MFS_Is_dot_directory(
    char *filename)
{
    if (*filename++ == '.')
    {
        if (*filename)
        {
            if (*filename++ == '.')
            {
                if (!*filename)
                {
                    return (2);
                }
            }
        }
        else
        {
            return (1);
        }
    }

    return (0);
}


/*!
 * \brief Finds next path component and returns its position and length in bytes.
 *
 * \param parse_pos
 * \param path_component
 *
 * \return length of the path component
 */
int MFS_next_path_component(char **parse_pos, char **path_component)
{
    int len = 0;

    /* skip initial path delimiters */
    while (IS_DELIMITER(**parse_pos))
    {
        (*parse_pos)++;
    }

    *path_component = *parse_pos;

    /* find next path delimiter or end of the string */
    while (**parse_pos && !IS_DELIMITER(**parse_pos))
    {
        (*parse_pos)++;
        len++;
    }

    return len;
}


/*!
 * \brief Calculates length of path component encoded in UTF-8 (counts up to first path delimiter found)
 *
 * \param[in] pathcomp
 * \param[out] unicode_chars number of Unicode characters in the string
 * \param[out] utf16_words number of UTF-16 words required to encode the path component
 *
 * \return raw length of the string in chars
 */
int MFS_path_component_len(char *pathcomp, int *unicode_chars, int *utf16_words)
{
    int32_t codepoint;
    int uchars = 0;
    int uwords = 0;
    char *pos = pathcomp;

    while (*pos && !IS_DELIMITER(*pos))
    {
        codepoint = utf8_decode(&pos, NULL);  // decoding advances pos
        if (codepoint < 0)
        {
            /* error during decoding - invalid UTF-8 string */
            uchars = 0;
            uwords = 0;
            pos = pathcomp;  // invalid string is reported as empty, this induces invalid parameter error in the upper layer
            break;
        }
        uchars++;
        /* increment uwords according by number of words required to encode the codepoint */
        uwords += 1 + (codepoint > 0xFFFF);
    };

    if (unicode_chars)
    {
        *unicode_chars = uchars;
    }

    if (utf16_words)
    {
        *utf16_words = uwords;
    }

    return pos - pathcomp;
}
