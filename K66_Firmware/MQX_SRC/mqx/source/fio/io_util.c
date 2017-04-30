
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
* See license agreement file for full license terms including other restrictions.
*****************************************************************************
*
* Comments:
*
*   Contains misc io functions.
*
*
*END************************************************************************/
#include "mqx_cnfg.h"
#if MQX_USE_IO_OLD

#include "mqx_inc.h"
#include "fio.h"
#include <ctype.h>
#include <string.h>


char *_parse_next_filename(char *src, int32_t *len, int32_t max_len);

static FS_TABLE_ENTRY _opened_fs_table[MAX_FS_INSTANCES];

/*!
 * \brief Converts specifed string to lower case.
 *
 * \param[in] arg Pointer to string to convert.
 *
 * \return TRUE or FALSE (Arg is NULL.)
 */
bool _io_strtolower( char *arg)
{
   uint32_t i=0;

   if (arg == NULL) return FALSE;
   while (arg[i])  {
      // Allows constant strings - as long as they are lowercase already
      if (isupper((int)arg[i])) {
        arg[i] = (char)tolower((int)arg[i]);
      }
      i++;
   }
   return TRUE;
}

/*!
 * \brief Compares two strings with different case size.
 *
 * \param[in] s1 Pointer to string 1.
 * \param[in] s2 Pointer to string 2.
 *
 * \return 0 End of string 1.
 * \return Difference between the two strings.
 */
_mqx_int _io_strcasecmp(const char *s1, const char *s2)
{
   while (tolower((int)*s1) == tolower((int)*s2))
   {
      if (*s1++ == '\0')
         return (0);
      s2++;
   }
   return (tolower((int)*s1) - tolower((int)*s2));
}

/*!
 * \brief Compares first n characters of two strings with different case size.
 *
 * \param[in] s1 Pointer to string 1.
 * \param[in] s2 Pointer to string 2.
 * \param[in] n  Number of characters to compare.
 *
 * \return 0 End of string 1.
 * \return Difference between the two strings.
 */
_mqx_int _io_strncasecmp(const char *s1, const char *s2, uint32_t n)
{
   if (n != 0)
   {
      do
      {
         if (tolower((int)*s1) != tolower((int)*s2))
            return (tolower((int)*s1) - tolower((int)*s2));
         if (*s1++ == '\0')
            break;
         s2++;
      } while (--n != 0);
   }
   return (0);
}
 /*!
  * \brief Finds file name stored in specified string.
  *
  * \param[in] arg Pointer to search file name in.
  *
  * \return Pointer to file name.
  * \return NULL (failure)
  */
char *_io_find_filename(char *arg)
{
   uint32_t i=0;

   if (arg==NULL) return NULL;

   while (isgraph((int)arg[i]) && ((int)arg[i] != ':')) i++;
   if (arg[i] == ':')  {
      return &arg[i+1];
   } else  if (arg[i] == '\0')  {
      return arg;
   } else  {
      return NULL;
   }
}

/*!
 * \brief Validates specified device.
 *
 * \param[in] arg Pointer to the device to validate.
 *
 * \return TRUE or FALSE (failure)
 */
bool _io_validate_device(char *arg)
{
   uint32_t i=0;

   while (isgraph((int)arg[i]) && ((int)arg[i] != ':')) i++;
   if (i && (arg[i]==':') && (arg[i+1]=='\0')) return TRUE;
   return FALSE;
}

/*!
 * \brief Creates prefixed file name.
 *
 * If no new name supplied, current device name will be used.
 *
 * \param[in] new_ptr Pointer to new file name.
 * \param[in] in_ptr  Pointer to string containing new file name.
 * \param[in] dev_ptr Pointer to current device name.
 */
void _io_create_prefixed_filename( char *new_ptr, char *in_ptr,  char *dev_ptr)
{
   char *file_ptr;

   new_ptr[0] = '\0';
   file_ptr = _io_find_filename(in_ptr);
   if (file_ptr == in_ptr)  {
      // No device name supplied, use current
      strcpy(new_ptr, dev_ptr);
   }
   strcat(new_ptr, in_ptr);
}

/*!
 * \brief Gets pointer on the row where fs_name is stored.
 *
 * \param[in] fs_name string that is being looked for.
 *
 * \return Pointer to the row where the fs_name is stored.
 * \return NULL (failure)
 */
MQX_FILE_PTR _io_get_fs_by_name(char *fs_name)
{
   int i;
   char *a;
   char *b;

   if ((fs_name == NULL) || (*fs_name == '\0'))
     return NULL;

   for(i=0; i<MAX_FS_INSTANCES; i++)
   {
      a = _opened_fs_table[i].FS_NAME;
      b = fs_name;

      // skip blank entries in the table
      if (*a == '\0')
      {
          continue;
      }
      
      // match with fs_name char by char
      while (*a && (*a == *b))
      {
         a++;
         b++;
      }

      // check whether name from the table is prefix of fs_name
      if (*a == '\0')
      {
         return _opened_fs_table[i].FS_PTR;
      }
   }

   // filesystem not found in table
   return NULL;
}
/*!
 * \brief Gets pointer on the row where first valid filesystem is stored.
 *
 * \return Pointer to the row where the first fs_name is stored.
 * \return NULL (failure)
 */
MQX_FILE_PTR _io_get_first_valid_fs()
{
   int i;
   // find first used field in table
   for(i=0;i<MAX_FS_INSTANCES;i++)
   {
      if (_opened_fs_table[i].FS_PTR != NULL)
      {
         return _opened_fs_table[i].FS_PTR;
      }
   }
   return NULL;
}

/*!
 * \brief Return TRUE or FALSE if filesystem is in list or not.
 *
 * \param[in] fs_ptr Pointer to file system structure.
 *
 * \return TRUE (File system is in table.) of FALSE (failure).
 */
bool _io_is_fs_valid(MQX_FILE_PTR fs_ptr)
{
   int i;

   /* First check that file pointer is not null */
   if (fs_ptr == NULL)
   {
      return FALSE;
   }

   /* find first used field in table */
   for(i=0;i<MAX_FS_INSTANCES;i++)
   {
      if (_opened_fs_table[i].FS_PTR == fs_ptr)
      {
         return TRUE;
      }
   }
   return FALSE;
}

/*!
 * \brief Checks whether file system is in list or not.
 *
 * \param[in] fs_ptr      Pointer to the file system.
 * \param[in] fs_name     File system name.
 * \param[in] fs_name_len File system name length.
 *
 * \return MQX_OK
 * \return MQX_INVALID_POINTER (Fd_ptr is NULL.)
 * \return MQX_INVALID_SIZE (Invalid file system name length.)
 */
int32_t _io_get_fs_name(MQX_FILE_PTR fs_ptr, char *fs_name, int32_t fs_name_len)
{
   int i;

    /* First check that input parameters are not not null */
   if (fs_ptr == NULL || fs_name == NULL)
   {
      return MQX_INVALID_POINTER;
   }

   /* find first used field in table */
   for(i=0;i<MAX_FS_INSTANCES;i++)
   {
      if (_opened_fs_table[i].FS_PTR == fs_ptr)
      {
         if(fs_name_len > strlen(_opened_fs_table[i].FS_NAME))
         {
           strcpy(fs_name,_opened_fs_table[i].FS_NAME);
           return MQX_OK;
         }
         else
         {
           return MQX_INVALID_SIZE;
         }
      }
   }
   return MQX_INVALID_POINTER;
}

/*!
 * \brief Store file system name and handle into _opened_fs_table.
 *
 * \param[in] fd_ptr   Pointer to file.
 * \param[in] name_ptr Pointer to file name.
 *
 * \return MQX_OK
 * \return MQX_INVALID_POINTER (Fd_ptr is NULL.)
 * \return MQX_OUT_OF_MEMORY (There is no empty slot.)
 */
uint32_t _io_register_file_system(MQX_FILE_PTR fd_ptr,char *name_ptr)
{
   int i;

   /* First check that file pointer is not null */
   if (fd_ptr == NULL || name_ptr == NULL)
   {
      return MQX_INVALID_POINTER;
   }


   /* find first empty field in table */
   for(i=0;i<MAX_FS_INSTANCES;i++)
   {
      if (_opened_fs_table[i].FS_PTR == NULL)
      {
        /* store filesystem pointer and name into the table */
        _opened_fs_table[i].FS_PTR = fd_ptr;
         strncpy(_opened_fs_table[i].FS_NAME, name_ptr, FS_MAX_DEVLEN);
         return MQX_OK;
      }
   }
   /* there is no empty slot, report error */
   return MQX_OUT_OF_MEMORY;
}

/*!
 * \brief Removes filesystem name and handle from _opened_fs_table.
 *
 * \param[in] fd_ptr Pointer to file.
 *
 * \return MQX_OK
 * \return MQX_INVALID_POINTER (Fd_ptr is NULL.)
 * \return MQX_INVALID_HANDLE (File system not found.)
 */
uint32_t _io_unregister_file_system(MQX_FILE_PTR fd_ptr)
{
   int i;

   /* First check that file pointer is not null */
   if (fd_ptr == NULL)
   {
      return MQX_INVALID_POINTER;
   }

   // find the corresponding entry in the table
   for(i=0; i<MAX_FS_INSTANCES; i++)
   {
      if (_opened_fs_table[i].FS_PTR == fd_ptr)
      {
         // store filesystem pointer and name into the table
         _opened_fs_table[i].FS_PTR = NULL;
         _opened_fs_table[i].FS_NAME[0] ='\0' ;
         // return MQX_OK
         return MQX_OK;
      }
   }

   // filesystem not found report error
   return MQX_INVALID_HANDLE;
}

/*!
 * \brief  Parses given path and returns device name or current device name
 * (if there was no device name in specified path).
 *
 * \param[out] out_dev        Pointer to place where device: will be stored.
 * \param[out] is_dev_in_path TRUE/FALSE - device name found in path
 * \param[in]  dev_len        Size of the output path.
 * \param[in]  input_path     Pointer to the parsed path.
 * \param[in]  cur_dev        Current device name.
 *
 * \return Length of device name (can be 0).
 * \return 0 (Input parameters are invalid)
 */
int32_t _io_get_dev_for_path(char *out_dev, bool *is_dev_in_path, int32_t dev_len, char *input_path, char *cur_dev)
{
   int i = 0;

   //parameter sanity check
   if (input_path == NULL) return 0;
   if (out_dev == NULL) return 0;
   if (dev_len <= 0) return 0;
   
   out_dev[0] = '\0';

   while (isgraph((int)input_path[i]) && (input_path[i] != ':'))
      i++;

   if (input_path[i] == ':')
   {
      // there is device name in input path
      if (is_dev_in_path != NULL)
      {
        *is_dev_in_path = TRUE;
      }
      if (dev_len <= ++i)
      {
        return 0;
      }
      strncpy(out_dev, input_path, i);
      out_dev[i] = '\0';
      return i;
   }
   else
   {
      // device name is NOT in input path
      if (is_dev_in_path != NULL)
      {
        *is_dev_in_path = FALSE;
      }
      if (cur_dev == NULL)
      {
         return 0;
      }
      i = strlen(cur_dev);
      if (i < dev_len)
      {
         strcpy(out_dev,cur_dev);
         return i;
      }
      else
      {
         return 0;
      }
   }
}


/*!
 * \brief Adds path to existing path converting directory delimiters and resolving relative directory references.
 *
 * \param[out] result    Pointer to place where resulting path will be stored. It has to be either an empty string or a result from previous call to this function.
 * \param[in]  len       Size of result.
 * \param[in]  path      If the path is absolute then contents of result is discarded prior processing. Slashes are converted to back-slashes. The path may contain '.' and '..'.
 *
 * \return MQX_INVALID_PARAMETER
 */
int32_t _io_path_add(char *result, int32_t len, char *path)
{
    int result_end;

    if (result == NULL) return MQX_INVALID_PARAMETER;
    if (path == NULL) return MQX_INVALID_PARAMETER;
    if (len <= 0) return MQX_INVALID_PARAMETER;

    if (*path == '\\' || *path == '/')
    {
        result_end = 0;
        result[result_end++] = '\\';
        if (result_end >= len) return MQX_INVALID_PARAMETER;
    }
    else
    {
        for (result_end=0; result[result_end]; result_end++)
        {
            if (result_end >= len) return MQX_INVALID_PARAMETER;
        }
    }

    while (*path)
    {
        /* skip consecutive delimiters in the in the source path */
        while (*path == '\\' || *path == '/')
        {
            path++;
        }

        /* check for '.' and '..' */
        if (path[0] == '.')
        {
            if ((path[1] == '\\') || (path[1] == '/') || (path[1]=='\0'))
            {
                /* single dot, just skip it */
                path++;
                continue;
            }
            else if ((path[1] == '.') && ((path[2] == '\\') || (path[2] == '/') || (path[2]=='\0')))
            {
                /* double dot, go up one directory level */
                if ((result_end > 0) && (result[result_end-1] == '\\'))
                {
                    /* remove trailing directory delimiter */
                    result_end--;
                }
                if (result_end == 0)
                {
                    /* underflow, relative path refers to non-existing upper directory level */
                    return MQX_INVALID_PARAMETER;
                }
                while (result_end > 0 && result[result_end-1] != '\\')
                {
                    /* trim the path until delimiter is reached */
                    result_end--;
                }
                path += 2;
                continue;
            }
        }

        /* check if there is a delimiter at the end of the result and eventually add it */
        if ((result_end > 0) && (result[result_end-1] != '\\'))
        {
            result[result_end++] = '\\';
            if (result_end >= len) return MQX_INVALID_PARAMETER;
        }

        /* copy source path until next delimiter or null-term */
        while (*path && *path != '\\' && *path != '/')
        {
            result[result_end++] = *path++;
            if (result_end >= len) return MQX_INVALID_PARAMETER;
        }
    }

    result[result_end] = '\0';
    return MQX_OK;
}


 /*!
 * \brief Converts relative to absolute path and extend it by the device name if
 * not present.
 *
 * \param[out] result    Pointer to place where absolute path will be stored.
 * \param[in]  curdir    Current directory name.
 * \param[in]  inputpath Pointer to parsed path.
 * \param[in]  len       Size of result.
 * \param[in]  cur_dev   Current device name.
 *
 * \return MQX_INVALID_PARAMETER
 */
int32_t _io_rel2abs(char *result, char *curdir, char *inputpath, int32_t len, char *cur_dev)
{
    int32_t devlen;
    int32_t error_code = MQX_OK;
    bool is_dev_in_path = FALSE;

    if (inputpath == NULL) return MQX_INVALID_PARAMETER;
    if (result == NULL) return MQX_INVALID_PARAMETER;

    devlen = _io_get_dev_for_path(result, &is_dev_in_path, len, inputpath, cur_dev);

    /* device name during path parsing will be skiped */
    result += devlen;
    if(is_dev_in_path == TRUE)
    {
        /* there was device name in input path -> skip it in parsing */
        inputpath += devlen;
    }

    /* the remaining length of the buffer */
    len -= devlen;
    if (len < 2)
    {
        return MQX_INVALID_PARAMETER;
    }

    /* start with root */
    result[0] = '\\';
    result[1] = '\0';

    /* if inputpath is a relative path, add current directory first */
    if ((*inputpath != '/') && (*inputpath != '\\'))
    {
        error_code = _io_path_add(result, len, curdir);
    }

    /* add inputpath to the result */
    if (error_code == MQX_OK)
    {
        error_code = _io_path_add(result, len, inputpath);
    }

    return error_code;
}


/*!
 * \brief Reads the first filename in source string, up to '\' ,'/' or '.'.
 *
 * \param[in]  src     The path name.
 * \param[out] len     Number of currently read characters.
 * \param[in]  max_len Maximal allowend length for filename.
 *
 * \return The read path name.
 * \return NULL (Len is larger then max_len.)
 */
char *_parse_next_filename
    (
    char   *src,
    int32_t *len,
    int32_t     max_len
    )
{
    *len = 0;

    while ( *src && *src != '/' && *src != '\\' )
    {
        src++;
        (*len)++;
        if ( *len > max_len )
        {
            *len = 0;
            return NULL;
        }
    }

    if ( *src == '\\' || *src == '/' )
    {
        src++;
    }

    return(src);
}

/*!
 * \brief Converts ASCII to string.
 *
 * \param[in] str Specified ASCII string.
 *
 * \return Conveted data.
 * \return 0 (failure)
 */
_mqx_int _io_atoi(const char *str) {
    volatile int value = 0;
    int sign = 1;
    while (*str == ' ') ++str; // skip leading whitespace

   if ( *str == '-' ) {
        sign = -1;
        str++;
   } else if ( *str == '+' ) {
        str++;
   } 

    while ( *str) {
        if ((*str >= '0') && (*str <= '9'))
            value = 10*value + (*str - '0');
        else
            break; // non-digit
        str++;
    }
    if (value < 0) return 0; // integer overflow
    return value*sign;
}


 /*
static MQX_FILE_PTR _io_default_filesystem = NULL;

void _io_set_default_filesystem(MQX_FILE_PTR fp)
{
   _io_default_filesystem = fp;
}

MQX_FILE_PTR _io_get_default_filesystem(void)
{
   return _io_default_filesystem;
}

static char *_io_default_filesystem_name = NULL;

void _io_set_default_filesystem_name(char *name)
{
   _io_default_filesystem_name = name;
}

char *_io_get_default_filesystem_name(void)
{
   return _io_default_filesystem_name;
}
*/

#endif // MQX_USE_IO_OLD
