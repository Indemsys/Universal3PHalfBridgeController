/*HEADER**********************************************************************
*
* Copyright 2013,2014 Freescale Semiconductor, Inc.
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
*   This file contains FTP server commands for operations with files and
*   directories.
*
*
*END************************************************************************/

#include <string.h>
#include "rtcs_util.h"
#include "ftpsrv_prv.h"
#include "ftpsrv_msg.h"
#include <mfs.h>

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   ftpsrv_dele
* Returned Value   :  int32_t error code
* Comments  :  mount a filesystem on a device.
*
* Usage:  dele <file>
*
*END*---------------------------------------------------------------------*/

int32_t ftpsrv_dele(FTPSRV_SESSION_STRUCT* session)
{ 
    int         error;
    int32_t     retval;
    char*       filename;
    char*       full_path;
    uint32_t    wrong_path;
    
    if (session->cmd_arg == NULL)
    {
        session->message = (char*) ftpsrvmsg_badsyntax;
        return(FTPSRV_ERROR);
    }

    /* Prepare filename */
    rtcs_url_decode(session->cmd_arg);
    filename = rtcs_path_strip_delimiters(session->cmd_arg);
    full_path = ftpsrv_get_full_path(session, filename, &wrong_path);
    if (full_path == NULL)
    {
        if (wrong_path)
        {
            session->message = (char*) ftpsrvmsg_no_file;
        }
        else
        {
            session->message = (char*) ftpsrvmsg_no_memory;
        }
        return(FTPSRV_ERROR);
    }

    /* Remove file from filesystem */
    error = ioctl(session->fs_ptr, IO_IOCTL_DELETE_FILE, (void*) full_path);   
    if (error != 0)
    {
        _mem_free(full_path);
        session->message = (char*) ftpsrvmsg_delete_error;
        retval = FTPSRV_ERROR;
    }
    else
    {
        char* buffer = session->buffer;
        uint32_t length = strlen(filename);

        /* Prepare response message */
        memmove(buffer+5, filename, length);
        _mem_copy("257 \"", buffer, 5);
        _mem_copy("\" deleted.\r\n", buffer+5+length, 13);
        _mem_free(full_path);
        session->message = session->buffer;
        retval = FTPSRV_OK;
    }
    
    return(retval);
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   ftpsrv_mkdir
* Returned Value   :  int32_t error code
* Comments  :  
*
* Usage:  mkdir <directory>
*
*END*---------------------------------------------------------------------*/

int32_t ftpsrv_mkdir(FTPSRV_SESSION_STRUCT* session)
{
    int32_t             error;
    char*               path = NULL;
    char*               full_path;
    char*               dir_name = session->cmd_arg;
    uint32_t            length;
    char*               buffer = session->buffer;
    MFS_FILE_ATTR_PARAM param;
    unsigned char       attr;
    uint32_t            wrong_path;

    if (session->cmd_arg == NULL)
    {
        session->message = (char*) ftpsrvmsg_badsyntax;
        return(FTPSRV_ERROR);
    }

    rtcs_url_decode(dir_name);
    path = rtcs_path_strip_delimiters(dir_name);
    full_path = ftpsrv_get_full_path(session, path, &wrong_path);
    if (full_path == NULL)
    {
        if (wrong_path)
        {
            session->message = (char*) ftpsrvmsg_no_file;
        }
        else
        {
            session->message = (char*) ftpsrvmsg_no_memory;
        }
        return(FTPSRV_ERROR);
    }

    param.PATHNAME = full_path;
    param.ATTRIBUTE_PTR = &attr;

    /* Check if directory exists */
    error = ioctl(session->fs_ptr, IO_IOCTL_GET_FILE_ATTR, (void*) &param);
    if (error == MFS_NO_ERROR)
    {
        _mem_free(full_path);
        session->message = (char*) ftpsrvmsg_mkdir_error;
        return(FTPSRV_ERROR);
    }

    error = ioctl(session->fs_ptr, IO_IOCTL_CREATE_SUBDIR, (void*) full_path);
    _mem_free(full_path);
    if(error != MFS_NO_ERROR)
    {
        session->message = (char*) ftpsrvmsg_no_fs;
        return(FTPSRV_ERROR);
    }

    length = strlen(dir_name);
    /* Prepare response message */
    memmove(buffer+5, dir_name, length);
    _mem_copy("257 \"", buffer, 5);
    _mem_copy("\" directory created.\r\n", buffer+5+length, 22);

    session->message = buffer;
    return FTPSRV_OK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   ftpsrv_rmdir
* Returned Value   :  int32_t error code
* Comments  :  mount a filesystem on a device.
*
* Usage:  rmdir <directory>
*
*END*---------------------------------------------------------------------*/

int32_t ftpsrv_rmdir(FTPSRV_SESSION_STRUCT* session)
{
    int32_t  error;
    char*    dir_name;
    char*    full_path;
    uint32_t length;
    char*    buffer = session->buffer;
    uint32_t wrong_path;

    if (session->cmd_arg == NULL)
    {
        session->message = (char*) ftpsrvmsg_badsyntax;
        return(FTPSRV_ERROR);
    }

    rtcs_url_decode(session->cmd_arg);
    dir_name = rtcs_path_strip_delimiters(session->cmd_arg);
    full_path = ftpsrv_get_full_path(session, dir_name, &wrong_path);
    if (full_path == NULL)
    {
        if (wrong_path)
        {
            session->message = (char*) ftpsrvmsg_no_file;
        }
        else
        {
            session->message = (char*) ftpsrvmsg_no_memory;
        }
        return(FTPSRV_ERROR);
    }

    error = ioctl(session->fs_ptr, IO_IOCTL_REMOVE_SUBDIR, (void*) full_path);
    _mem_free(full_path);
    if(error)
    {
        session->message = (char*) ftpsrvmsg_rmdir_error;
        return(FTPSRV_ERROR);
    } 

    length = strlen(dir_name);
    /* Prepare response message */
    memmove(buffer+5, dir_name, length);
    _mem_copy("257 \"", buffer, 5);
    _mem_copy("\" directory removed.\r\n", buffer+5+length, 22);

    session->message = buffer;
    return FTPSRV_OK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   ftpsrv_rnfr
* Returned Value   :  int32_t error code
* Comments  :  specify rename from file.
*
* Usage:  rnfr <file>
*
*END*---------------------------------------------------------------------*/

int32_t ftpsrv_rnfr(FTPSRV_SESSION_STRUCT* session)
{
  
    int32_t              error;
    char*                path;
    char*                full_path;
    MFS_FILE_ATTR_PARAM  param;
    unsigned char        attr;
    uint32_t             wrong_path;

    rtcs_url_decode(session->cmd_arg);
    path = rtcs_path_strip_delimiters(session->cmd_arg);

    full_path = ftpsrv_get_full_path(session, path, &wrong_path);
    if (full_path == NULL)
    {
        if (wrong_path)
        {
            session->message = (char*) ftpsrvmsg_no_file;
        }
        else
        {
            session->message = (char*) ftpsrvmsg_no_memory;
        }
        return(FTPSRV_ERROR);
    }

    /* Free old filename if there is some */
    if (session->rnfr_path)
    {
        _mem_free(session->rnfr_path);
        session->rnfr_path = NULL;
    }

    session->rnfr_path = full_path;
    param.PATHNAME = session->rnfr_path;
    param.ATTRIBUTE_PTR = &attr;

    /* Check if file exists */
    error = ioctl(session->fs_ptr, IO_IOCTL_GET_FILE_ATTR, (void*) &param);
    if(error)
    {
        session->message = (char*) ftpsrvmsg_no_file;
        return(FTPSRV_ERROR);
    } 

    session->message = (char*) ftpsrvmsg_ready_for_dest;
    return(FTPSRV_OK);
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   ftpsrv_rnto
* Returned Value   :  int32_t error code
* Comments  :  specify rename to file.
*
* Usage:  rnto <file>
*
*END*---------------------------------------------------------------------*/

int32_t ftpsrv_rnto(FTPSRV_SESSION_STRUCT* session)
{
  
    int32_t          error;
    int32_t          retval;
    char*            path;
    char*            full_path = NULL;
    MFS_RENAME_PARAM param;  
    uint32_t         length;
    char*            filename;
    char*            old_filename;
    uint32_t         wrong_path;

    if (session->rnfr_path == NULL)
    {
        session->message = (char*) ftpsrvmsg_no_filename;
        return(FTPSRV_ERROR);
    }

    rtcs_url_decode(session->cmd_arg);
    path = rtcs_path_strip_delimiters(session->cmd_arg);
    
    /* Copy filename to local variable */
    length = strlen(path);
    filename = RTCS_mem_alloc_zero(length+1);
    if (filename == NULL)
    {
        session->message = (char*) ftpsrvmsg_no_memory;
        retval = FTPSRV_ERROR;
        goto ERROR;
    }
    _mem_copy(session->cmd_arg, filename, length);

    /* Get full file path */
    full_path = ftpsrv_get_full_path(session, path, &wrong_path);
    if (full_path == NULL)
    {
        if (wrong_path)
        {
            session->message = (char*) ftpsrvmsg_no_file;
        }
        else
        {
            session->message = (char*) ftpsrvmsg_no_memory;
            retval = FTPSRV_ERROR;
            goto ERROR;
        }
        return(FTPSRV_ERROR);
    }
    /* Rename file */
    param.OLD_PATHNAME = session->rnfr_path;
    param.NEW_PATHNAME = full_path;
    error = ioctl(session->fs_ptr, IO_IOCTL_RENAME_FILE, (void*) &param);
    
    /* Get old filename so response message can be printed */
    old_filename = strrchr(session->rnfr_path, '\\');
    if (old_filename == NULL)
    {
        session->message = (char*) ftpsrvmsg_syntax_error;
        retval = FTPSRV_ERROR;
        goto ERROR;
    }
    old_filename++;

    /* Print response message */
    if(error)
    {
        snprintf(session->buffer, FTPSRV_BUF_SIZE, "550 unable to rename \"%s\" to \"%s\".\r\n", old_filename, filename);
        retval = FTPSRV_ERROR;
    }
    else
    {
        snprintf(session->buffer, FTPSRV_BUF_SIZE, "250 \"%s\" renamed to \"%s\".\r\n", old_filename, filename);
        retval = FTPSRV_OK;
    } 

    ERROR:
    if (filename != NULL)
    {
        _mem_free(filename);
    }
    if (session->rnfr_path != NULL)
    {
        _mem_free(session->rnfr_path);
        session->rnfr_path = NULL;
    }
    if (full_path != NULL)
    {
        _mem_free(full_path);
    }
    if (session->message == NULL)
    {
        session->message = session->buffer;
    }

    return (retval);
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   ftpsrv_size
* Returned Value   :  int32_t error code
* Comments  :  
*
* Usage:  
*
*END*---------------------------------------------------------------------*/
   
int32_t ftpsrv_size(FTPSRV_SESSION_STRUCT* session)
{
    char*             full_path;    
    char*             path;
#if MQX_USE_IO_OLD
    int32_t           size = 0;
    MQX_FILE*         fp;
#else
    long int          size = 0;
    FILE *            fp;
#define IO_ERROR      -1
#endif
    uint32_t          wrong_path;

    if (session->cmd_arg == NULL)
    {
        session->message = (char*) ftpsrvmsg_badsyntax;
        return(FTPSRV_ERROR);
    }

    rtcs_url_decode(session->cmd_arg);
    path = rtcs_path_strip_delimiters(session->cmd_arg);
    full_path = ftpsrv_get_full_path(session, path, &wrong_path);
    if (full_path == NULL)
    {
        if (wrong_path)
        {
            session->message = (char*) ftpsrvmsg_no_file;
        }
        else
        {
            session->message = (char*) ftpsrvmsg_no_memory;
        }
        return(FTPSRV_ERROR);
    }

    fp = fopen(full_path,"r");
    _mem_free(full_path);
    if (fp == NULL)
    {
        session->message = (char*) ftpsrvmsg_no_file;
        return(FTPSRV_ERROR);
    } 

    #if MQX_USE_IO_OLD
    fseek(fp, 0, IO_SEEK_END);
    #else
    fseek(fp, 0, SEEK_END);
    #endif
    size = ftell(fp);
    fclose(fp);

    if (size == IO_ERROR)
    {
        session->message = (char*) ftpsrvmsg_no_file;
        return(FTPSRV_ERROR);
    } 

    snprintf(session->buffer, FTPSRV_BUF_SIZE, "213 %d\r\n", size);
    session->message = session->buffer;
    return FTPSRV_OK;
}
