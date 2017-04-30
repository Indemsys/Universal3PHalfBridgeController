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
* See license agreement file for full license terms including other
* restrictions.
*****************************************************************************
*
* Comments:
*
*   This file contains an implementation of an
*   FTP server.
*
*
*END************************************************************************/

#include <string.h>
#include <rtcs.h>
#include "rtcs_util.h"

#include "ftpsrv_prv.h"
#include "ftpsrv_msg.h"
#include <mfs.h>
const FTPSRV_COMMAND_STRUCT ftpsrv_commands[] =
{
    {"ABOR",  ftpsrv_abor,  !FTPSRVCFG_IS_ANONYMOUS},
    {"APPE",  ftpsrv_appe,  !FTPSRVCFG_IS_ANONYMOUS},
    {"CWD",   ftpsrv_cwd,   !FTPSRVCFG_IS_ANONYMOUS},
    {"CDUP",  ftpsrv_cdup,  !FTPSRVCFG_IS_ANONYMOUS},
    {"DELE",  ftpsrv_dele,  !FTPSRVCFG_IS_ANONYMOUS},
    {"EPSV",  ftpsrv_epsv,  !FTPSRVCFG_IS_ANONYMOUS},
    {"EPRT",  ftpsrv_eprt,  !FTPSRVCFG_IS_ANONYMOUS},
    {"FEAT",  ftpsrv_feat,  0},
    {"HELP",  ftpsrv_help,  0},
    {"LIST",  ftpsrv_list,  !FTPSRVCFG_IS_ANONYMOUS},
    {"MKDIR", ftpsrv_mkdir, !FTPSRVCFG_IS_ANONYMOUS},
    {"MKD",   ftpsrv_mkdir, !FTPSRVCFG_IS_ANONYMOUS},
    {"NLST",  ftpsrv_list,  !FTPSRVCFG_IS_ANONYMOUS},
    {"NOOP",  ftpsrv_noop,  0},
    {"PASS",  ftpsrv_pass,  0},
    {"PASV",  ftpsrv_pasv,  !FTPSRVCFG_IS_ANONYMOUS},
    {"PORT",  ftpsrv_port,  !FTPSRVCFG_IS_ANONYMOUS},
    {"PWD",   ftpsrv_pwd,   !FTPSRVCFG_IS_ANONYMOUS},
    {"QUIT",  ftpsrv_quit,  0},
    {"RMDIR", ftpsrv_rmdir, !FTPSRVCFG_IS_ANONYMOUS},
    {"RMD",   ftpsrv_rmdir, !FTPSRVCFG_IS_ANONYMOUS},
    {"RETR",  ftpsrv_retr,  !FTPSRVCFG_IS_ANONYMOUS},
    {"RNFR",  ftpsrv_rnfr,  !FTPSRVCFG_IS_ANONYMOUS},
    {"RNTO",  ftpsrv_rnto,  !FTPSRVCFG_IS_ANONYMOUS},
    {"SITE",  ftpsrv_site,  0},
    {"SIZE",  ftpsrv_size,  !FTPSRVCFG_IS_ANONYMOUS},
    /*{"STAT",  ftpsrv_stat,  !FTPSRVCFG_IS_ANONYMOUS},*/
    {"STOR",  ftpsrv_stor,  !FTPSRVCFG_IS_ANONYMOUS},
    {"SYST",  ftpsrv_syst,  0},
    {"TYPE",  ftpsrv_type,  !FTPSRVCFG_IS_ANONYMOUS},
    {"USER",  ftpsrv_user,  0},
    {"XCUP",  ftpsrv_cdup,  !FTPSRVCFG_IS_ANONYMOUS},
    {"XCWD",  ftpsrv_cwd,   !FTPSRVCFG_IS_ANONYMOUS},
    {"XMKD",  ftpsrv_mkdir, !FTPSRVCFG_IS_ANONYMOUS},
    {"XPWD",  ftpsrv_pwd,   !FTPSRVCFG_IS_ANONYMOUS},
    {"XRMD",  ftpsrv_rmdir, !FTPSRVCFG_IS_ANONYMOUS},
    {NULL,    ftpsrv_unrecognized, 0}
};

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   ftpsrv_cwd
* Returned Value   :  int32_t error code
* Comments  :  mount a filesystem on a device.
*
* Usage:  cwd <directory>
*
*END*---------------------------------------------------------------------*/

int32_t ftpsrv_cwd(FTPSRV_SESSION_STRUCT* session)
{
    char*     path;
    char*     new_path;
    char*     full_path;
    int32_t   error;
    uint32_t  wrong_path;

    if (session->cmd_arg == NULL)
    {
        session->message = (char*) ftpsrvmsg_badsyntax;
        return(FTPSRV_ERROR);
    }

    rtcs_url_decode(session->cmd_arg);
    path = rtcs_path_strip_delimiters(session->cmd_arg);
        
    /* Get absolute directory path in filesystem */
    full_path = ftpsrv_get_full_path(session, path, &wrong_path);
    if (full_path == NULL)
    {
        if (wrong_path)
        {
            session->message = (char*) ftpsrvmsg_cd_error;
        }
        else
        {
            session->message = (char*) ftpsrvmsg_no_memory;
        }
        return(FTPSRV_ERROR);
    }

    /* Try to open directory only for reading so we know if directory exists */
    error = ioctl(session->fs_ptr, IO_IOCTL_CHECK_DIR_EXIST, (void*) full_path);   
    if (error != MFS_NO_ERROR)
    {
        session->message = (char*) ftpsrvmsg_cd_error;
        _mem_free(full_path);
        return(FTPSRV_ERROR);
    }

    /* Get path relative to FTP root so we can set it as new current directory */
    new_path = ftpsrv_get_relative_path(session, full_path);
    _mem_free(full_path);
    if (!new_path)
    {
        session->message = (char*) ftpsrvmsg_no_memory;
        return(FTPSRV_ERROR);  
    }

    /* If everything went right, set path we chdir'd to as session current path */ 
    if (session->cur_dir)
    {
        _mem_free(session->cur_dir);
    }

    session->cur_dir = new_path;
    return (ftpsrv_pwd(session));
}


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   ftpsrv_cdup
* Returned Value   :  int32_t error code
* Comments  :  change to parent directory.
*
* Usage:  cdup
*
*END*---------------------------------------------------------------------*/

int32_t ftpsrv_cdup(FTPSRV_SESSION_STRUCT* session)
{
    char arg[4];

    arg[0] = '.';
    arg[1] = '.';
    arg[2] = '\\';
    arg[3] = '\0';
    session->cmd_arg = arg;
    return(ftpsrv_cwd(session));
}


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   ftpsrv_feat
* Returned Value   :  int32_t error code
* Comments  :  Returns supported feature.
*
* Usage:  feat
*
*END*---------------------------------------------------------------------*/

int32_t ftpsrv_feat(FTPSRV_SESSION_STRUCT* session)
{ 
    session->message = "211-Features:\r\n SIZE\r\n211 End\r\n";
    return FTPSRV_OK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   ftpsrv_help
* Returned Value   :  int32_t error code
* Comments  :  
*
* Usage:  
*
*END*---------------------------------------------------------------------*/

int32_t ftpsrv_help(FTPSRV_SESSION_STRUCT* session)
{
    const FTPSRV_COMMAND_STRUCT* cmd_ptr = ftpsrv_commands;
    uint32_t                     length = 0;
    uint32_t                     n = 1;
    uint32_t                     space = FTPSRV_BUF_SIZE;
    uint32_t                     max_cmd_length = ftpsrv_max_cmd_length();
    char*                        separator;
    char*                        buffer = session->buffer;

    ftpsrv_send_msg(session, ftpsrvmsg_help_start);

    while(cmd_ptr->command != NULL)
    {
        /* After every fifth command print a new line. Commands are separated by spaces */
        separator = (n % 4) ? " " : "\r\n";
        n++;
        length += snprintf(buffer+length, space, "%s%s", cmd_ptr->command, separator);
        space = FTPSRV_BUF_SIZE-length;
        
        /* If there is not enough space in the buffer flush it to client and repeat printing */
        if (space < (max_cmd_length + sizeof("\r\n")))
        {
            send(session->control_sock, buffer, FTPSRV_BUF_SIZE-space, 0);
            _mem_zero(buffer, length);
            space = FTPSRV_BUF_SIZE;
            length = 0;
        }

        cmd_ptr++;
    }

    if (n)
    {
        space -= snprintf(buffer+length, space, "\r\n");
    }
    
    /* Send command list from the buffer and set message to HELP end text */
    send(session->control_sock, buffer, FTPSRV_BUF_SIZE-space, 0);

    session->message = (char*) ftpsrvmsg_help_end;
    return(FTPSRV_OK);
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   ftpsrv_list
* Returned Value   :  int32_t error code
* Comments  :  
*
* Usage:  
*
*END*---------------------------------------------------------------------*/
   
int32_t ftpsrv_list(FTPSRV_SESSION_STRUCT* session)
{
    int32_t  length;
    char*    path;
    void*    dir_ptr;
    uint32_t sock;
    char*    dir_param;
    char*    full_path;
    char*    temp;
    uint32_t path_length;
    uint32_t wrong_path;

    if (session->cmd_arg == NULL)
    {
        path = "";
    }
    else
    {
        rtcs_url_decode(session->cmd_arg);
        path = rtcs_path_strip_delimiters(session->cmd_arg);
    }

    /* Translate relative path to absolute. */
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
    path_length = strlen(full_path);

    /* Allocate space for path + appendix, copy full path and add appendix to it. */
    /* This is required because MFS cannot list directories, only files. */
    temp = RTCS_mem_alloc_zero(path_length+sizeof(FTPSRV_PATH_APPENDIX)); 
    _mem_copy(full_path, temp, path_length);
    _mem_copy(FTPSRV_PATH_APPENDIX, temp+path_length, sizeof(FTPSRV_PATH_APPENDIX)-1);
    _mem_free(full_path);
    full_path = temp;

    /* Open directory. Unix format for LIST command, simple file list for NLIST. */
    if (!strcmp(session->command, "LIST"))
    {
        /* Unix */
        dir_param = "u*";
    }
    else
    {
        /* File list */
        dir_param = "f*";
    }

    /* Open directory, get list, cleanup and return. */
    dir_ptr = _io_mfs_dir_open(session->fs_ptr, full_path, dir_param);
    if (dir_ptr == NULL)
    {
        session->message = (char*) ftpsrvmsg_no_file;
        return(FTPSRV_ERROR);
    } 

    /* Send initialization message */
    ftpsrv_send_msg(session, ftpsrvmsg_opening_datacon);
    
    /* Open data connection */
    sock = ftpsrv_open_data_connection(session);
    if (sock == RTCS_SOCKET_ERROR)
    {
        session->message = (char*) ftpsrvmsg_locerr;
        _mem_free(full_path);
        _io_mfs_dir_close(dir_ptr);
        return(FTPSRV_ERROR);
    }

    /* Send data (directory listing). */
    while ((length = _io_mfs_dir_read(dir_ptr, session->buffer, FTPSRV_BUF_SIZE)) > 0)
    {
        /* If we are in root do not list "one level up" nor "current dir" link */
        if ((strstr(session->buffer, " .. ") || strstr(session->buffer, " . ")) && !strcmp(session->cur_dir, "\\"))
        {
            _mem_zero(session->buffer, length);
        }
        else
        {
            if (send(sock, session->buffer, length, 0) != length)
            { 
                ftpsrv_send_msg(session, ftpsrvmsg_writefail);
                break;
            }
        }
    }
    /* Cleanup */
    closesocket(sock);
    _io_mfs_dir_close(dir_ptr);
    _mem_free(full_path);

    session->message = (char*) ftpsrvmsg_trans_complete;
    return FTPSRV_OK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   ftpsrv_noop
* Returned Value   :  int32_t error code
* Comments  :  
*
* Usage:  
*
*END*---------------------------------------------------------------------*/
   
int32_t ftpsrv_noop(FTPSRV_SESSION_STRUCT* session)
{
   session->message = (char*) ftpsrvmsg_ok;
   return FTPSRV_OK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   ftpsrv_pass
* Returned Value   :  int32_t error code
* Comments  :  .
*
*
*END*---------------------------------------------------------------------*/

int32_t ftpsrv_pass(FTPSRV_SESSION_STRUCT* session)
{
    int32_t  retval = FTPSRV_ERROR;
    FTPSRV_AUTH_STRUCT* input;

    input = &session->auth_input;
    input->pass = session->cmd_arg;

    if (input->pass != NULL)
    {
        /* Validate username and password */
        if (ftpsrv_check_authtbl(session))
        {
            session->auth_state = FTPSRV_LOGGED_IN;
            session->message = (char*) ftpsrvmsg_pass;
        }
        else
        {
            session->auth_state = FTPSRV_LOGGED_OUT;
            session->message = (char*) ftpsrvmsg_authfail;
        }
        retval = FTPSRV_OK;
    }
    else
    {
        session->message = (char*) ftpsrvmsg_badsyntax;
        retval = FTPSRV_ERROR;
    }
    return(retval);
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   ftpsrv_pwd
* Returned Value   :  int32_t error code
* Comments  :  .
*
*
*END*---------------------------------------------------------------------*/

int32_t ftpsrv_pwd(FTPSRV_SESSION_STRUCT* session)
{
    snprintf(session->buffer, FTPSRV_BUF_SIZE, "257 \"%s\" is the current directory\r\n", session->cur_dir);
    session->message = session->buffer;
    return FTPSRV_OK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   ftpsrv_quit
* Returned Value   :  int32_t error code
* Comments  :  
*
* Usage:  
*
*END*---------------------------------------------------------------------*/
   
int32_t ftpsrv_quit(FTPSRV_SESSION_STRUCT* session)
{
    session->message = (char*) ftpsrvmsg_goodbye;
    ftpsrv_abort(session->data_sock);
    session->connected = FALSE;
    return(FTPSRV_OK);
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   ftpsrv_site
* Returned Value   :  int32_t error code
* Comments  :  
*
* Usage:  
*
*END*---------------------------------------------------------------------*/
   
int32_t ftpsrv_site(FTPSRV_SESSION_STRUCT* session)
{
   session->message = (char*) ftpsrvmsg_site_info;
   return FTPSRV_OK;
}

int32_t ftpsrv_stat(FTPSRV_SESSION_STRUCT* session)
{
    session->message = "";
    return FTPSRV_OK;
}
  
/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   ftpsrv_syst
* Returned Value   :  int32_t error code
* Comments  :  
*
* Usage:  
*
*END*---------------------------------------------------------------------*/
   
int32_t ftpsrv_syst(FTPSRV_SESSION_STRUCT* session)
{
   session->message = (char*) ftpsrvmsg_syst;
   return FTPSRV_OK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   ftpsrv_type
* Returned Value   :  int32_t error code
* Comments  :  
*
* Usage:  
*
*END*---------------------------------------------------------------------*/
   
int32_t ftpsrv_type(FTPSRV_SESSION_STRUCT* session)
{
    if (session->cmd_arg == NULL)
    {
        session->message = (char*) ftpsrvmsg_badsyntax;
        return(FTPSRV_ERROR);
    }

    switch (*session->cmd_arg)
    {
        case 'a':
        case 'A':
            session->message = (char*) ftpsrvmsg_type_ascii;
            break;
        case 'b':
        case 'B': 
        case 'i':
        case 'I':
            session->message = (char*) ftpsrvmsg_type_image;
            break;
        default:
            session->message = (char*) ftpsrvmsg_type_unknown;
            return(FTPSRV_ERROR);
    } 
    return FTPSRV_OK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   ftpsrv_user
* Returned Value   :  int32_t error code
* Comments  :  
*
* Usage:  
*
*END*---------------------------------------------------------------------*/
   
int32_t ftpsrv_user(FTPSRV_SESSION_STRUCT* session)
{
    char*    temp;
    uint32_t length;
    int32_t  retval = FTPSRV_ERROR;
    FTPSRV_AUTH_STRUCT* input = &session->auth_input;
    
    if (session->cmd_arg == NULL)
    {
        session->message = (char*) ftpsrvmsg_badsyntax;
        return(FTPSRV_ERROR);
    }

    length = strlen(session->cmd_arg) + 1;
    
    if ((input->uid) && (strlen(input->uid) >= length))
    {
        temp = input->uid;
    }
    else
    {
        if (input->uid != NULL)
        {
            _mem_free(input->uid);
            input->uid = NULL;
        }
        temp = RTCS_mem_alloc_zero(sizeof(char)*length);
    }

    if (temp != NULL)
    {
        _mem_copy(session->cmd_arg, temp, length - 1);
        input->uid = temp;
        session->message = (char*) ftpsrvmsg_need_password;
        session->auth_state = FTPSRV_USER;
        retval = FTPSRV_OK;
    }
    
    if (retval != FTPSRV_OK)
    {
        session->message = (char*) ftpsrvmsg_no_memory;
    }

    return(retval);
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : ftpsrv_unrecognized
* Returned Value   : int32_t error code
*
*END*---------------------------------------------------------------------*/

int32_t ftpsrv_unrecognized(FTPSRV_SESSION_STRUCT* session)
{
   session->message = (char*) ftpsrvmsg_unrecognized;
   return(FTPSRV_ERROR);
}
