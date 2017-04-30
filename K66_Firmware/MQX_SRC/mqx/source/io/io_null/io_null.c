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
*   This file contains the nulldisk driver functions
*
*
*END************************************************************************/

#include "mqx.h"
#include "fio.h"
#include "fio_prv.h"
#include "io.h"
#include "io_null.h"
#include "io_prv.h"
#include "ionulprv.h"


/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : _io_null_install
* Returned Value   : _mqx_uint a task error code or MQX_OK
* Comments         :
*    Install a fdv_null driver.
*
*END*----------------------------------------------------------------------*/

_mqx_uint _io_null_install
    (
        /* [IN] A string that identifies the device for fopen */
        char            *identifier
    )
{
    _mqx_uint result;

    result = _io_dev_install(
        identifier,
        _io_null_open,
        _io_null_close,
        _io_null_read,
        _io_null_write,
        _io_null_ioctl,
        NULL
    ); 

    return result;
}


/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : _io_null_open
* Returned Value   : a null pointer
* Comments         : Opens and initializes fdv_null driver.
* 
*END*----------------------------------------------------------------------*/

_mqx_int _io_null_open
    (
        /* [IN] the file handle for the device being opened */
        MQX_FILE_PTR    fd_ptr,

        /* [IN] the remaining portion of the name of the device */
        char            *open_name_ptr,

        /* [IN] the flags to be used during operation:
        ** echo, translation, xon/xoff, encoded into a pointer.
        */
        char            *flags
    )
{
    /* Nothing to do */
    (void)fd_ptr; /* disable 'unused variable' warning */
    (void)open_name_ptr; /* disable 'unused variable' warning */
    (void)flags; /* disable 'unused variable' warning */
    return(MQX_OK);
}


/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : _io_null_close
* Returned Value   : ERROR CODE
* Comments         : Closes fdv_null driver
* 
*END*----------------------------------------------------------------------*/

_mqx_int _io_null_close
    (
        /* [IN] the file handle for the device being closed */
        MQX_FILE_PTR fd_ptr
    )
{
    /* Nothing to do */
    (void)fd_ptr;
    return(MQX_OK);
}


/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : _io_null_read
* Returned Value   : number of characters read
* Comments         : Reads data from fdv_ram driver
* 
*END*----------------------------------------------------------------------*/

_mqx_int _io_null_read
    (
        /* [IN] the file handle for the device */
        MQX_FILE_PTR    fd_ptr,

        /* [IN] where the characters are to be stored */
        char            *data_ptr,

        /* [IN] the number of characters to input */
        _mqx_int        num
    )
{
    (void)fd_ptr; /* disable 'unused variable' warning */
    (void)data_ptr; /* disable 'unused variable' warning */
    (void)num; /* disable 'unused variable' warning */
    return(0);
}


/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : _io_null_write
* Returned Value   : number of characters written
* Comments         : Writes data to the fdv_ram device
* 
*END*----------------------------------------------------------------------*/

_mqx_int _io_null_write
    (
        /* [IN] the file handle for the device */
        MQX_FILE_PTR    fd_ptr,

        /* [IN] where the characters are */
        char            *data_ptr,

        /* [IN] the number of characters to output */
        _mqx_int        num
    )
{
    (void)fd_ptr; /* disable 'unused variable' warning */
    (void)data_ptr; /* disable 'unused variable' warning */
    (void)num; /* disable 'unused variable' warning */
    return(num);
}


/*FUNCTION*****************************************************************
* 
* Function Name    : _io_null_ioctl
* Returned Value   : int32_t
* Comments         :
*    Returns result of ioctl operation.
*
*END*********************************************************************/

_mqx_int _io_null_ioctl
    (
        /* [IN] the file handle for the device */
        MQX_FILE_PTR    fd_ptr,

        /* [IN] the ioctl command */
        _mqx_uint       cmd,

        /* [IN] the ioctl parameters */
        void            *param_ptr
    )
{
    (void)fd_ptr; /* disable 'unused variable' warning */
    (void)cmd; /* disable 'unused variable' warning */
    (void)param_ptr; /* disable 'unused variable' warning */
    return IO_ERROR_INVALID_IOCTL_CMD;
}


