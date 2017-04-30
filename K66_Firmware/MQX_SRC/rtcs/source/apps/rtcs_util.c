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
*   This file contains various RTCS utility functions (URI encoding, path 
*   cleanup etc.)
*
*
*END************************************************************************/

#include "rtcs.h"
#include "rtcs_util.h"
#include <stdlib.h>
#include <ctype.h>

/*
 * Normalize filesystem path.
 */
void rtcs_path_normalize(char* path)
{
    char        *bs = path;      /* backslash location. */
    char        *prev_bs = path; /* previous backslash location. */
    bool        init = true;
    uint32_t    offset = 0;

    RTCS_ASSERT(path != NULL);
    /* Replace path segments for listing one level up */
    while((bs = strchr(prev_bs+offset, '\\')) != NULL)
    {
        if (init)
        {
            init = false;
            prev_bs = bs;
        }
        /* Encountered "\.." sequence. */
        if ((bs[1] == '.') && (bs[2] == '.'))
        {
            uint32_t index;
            
            index = (bs[3] == '\0') ? 1 : 0;
            memmove(prev_bs+index, bs+3, strlen(bs+3)+1);
            bs = prev_bs;
            offset = 0;
        }
        /* Encountered "..\" sequence. */
        else if ((bs[-1] == '.') && (bs[-2] == '.') && (bs[1] == '\0'))
        {
            memmove(prev_bs, bs, strlen(bs)+1);
            bs = prev_bs;
            offset = 0;
        }
        /* Encountered "\\" sequence. */
        else if (bs[1] == '\\')
        {
            memmove(bs, bs+1, strlen(bs)+1);
            offset = 0;
        }
        else
        {
            offset = 1;
        }
        prev_bs = bs;
    }
}

/*
 * Remove trailing and ending backslash.
 */
char* rtcs_path_strip_delimiters(char* path)
{
   char*   ptr;
   uint32_t len;

   RTCS_ASSERT(path != NULL);
   ptr = path;
   len = strlen(ptr);

    if (len > 2)
    {
        if ((ptr[0] == '/') && (ptr[1] == '\\') && (ptr[2] == '/'))
        {
            ptr +=2;
            len -=2;
        }
        if (ptr[len-1] == '/')
        {
            ptr[len-1] = '\0';   
        }
    }   
    return ptr;
}

/*
 * Decode URL encoded string.
 */
void rtcs_url_decode(char* url)
{
    char* src = url;
    char* dst = url;

    RTCS_ASSERT(url != NULL);

    while(*src != '\0')
    {
        if ((*src == '%') && (isxdigit((int) *(src+1))) && (isxdigit((int) *(src+2))))
        {
            *src = *(src+1);
            *(src+1) = *(src+2);
            *(src+2) = '\0';
            *dst++ = strtol(src, NULL, 16);
            src += 3;
        }
        else
        {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

/*
 * URL cleanup (remove invalid path segments - /./ and /../)
 */
void rtcs_url_cleanup(char* url)
{
    char* src = url;
    char* dst = url;

    RTCS_ASSERT(url != NULL);

    while(*src != '\0')
    {
        if ((src[0] == '/') && (src[1] == '.'))
        {
            if (src[2] ==  '/')
            {
                src += 2;
            }
            else if ((src[2] == '.') && (src[3] == '/'))
            {
                src += 3;
            }
        }
        *dst++ = *src++;
    }
    *dst = '\0';
}

/*
 * Join root directory and relative path.
 */
char* rtcs_path_create(const char *root, char *filename)
{
    char       *path;
    char *tmp;
    uint32_t   root_length;
    uint32_t   filename_length;
    uint32_t   path_length;

    RTCS_ASSERT(root != NULL);
    RTCS_ASSERT(filename != NULL);
    
    root_length = strlen(root);
    filename_length = strlen(filename);
    /* 
     * Length is worst case - +1 for terminating zero and +1 for potential 
     * missing backslash
     */
    path_length = root_length + filename_length + 2;
    tmp = filename;
    
    /* Correct path slashes */
    while (*tmp != '\0')
    {
        if (*tmp == '/')
        {
            *tmp = '\\';
        }
        tmp++;
    }
    
    path = (char *) RTCS_mem_alloc_zero(path_length);
    if (path != NULL)
    {
        _mem_copy(root, path, root_length);
        if ((root[root_length-1] != '\\') && (filename[0] != '\\'))
        {
            path[root_length] = '\\';
            root_length++;
        }
        _mem_copy(filename, path+root_length, filename_length);
    }
    return(path);
}
