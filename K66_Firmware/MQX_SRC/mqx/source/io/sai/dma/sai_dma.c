/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
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
*   This include file is used to provide information needed by
*   applications using the SAI I/O functions.
*
*
*END************************************************************************/

#include <mqx.h>
#include <fio.h>
#include <io.h>
#include "sai.h"
#include "sai_dma_prv.h"

/*FUNCTION****************************************************************
*
* Function Name    : _io_sai_dma_install
* Returned Value   : MQX error code
* Comments         :
*    Install the SAI device.
*
*END**********************************************************************/

_mqx_uint _io_sai_dma_install
(
    /* [IN] The initialization structure for the device */
    SAI_INIT_STRUCT_CPTR init_ptr
)
{ /* Body */
    IO_SAI_DEVICE_STRUCT_PTR io_dev_ptr = NULL;    

    io_dev_ptr = (IO_SAI_DEVICE_STRUCT_PTR) _mem_alloc_system_zero(sizeof (IO_SAI_DEVICE_STRUCT));
    
    if (io_dev_ptr == NULL)
    {
        return MQX_OUT_OF_MEMORY;
    }

    io_dev_ptr->DEV_INIT            = init_ptr->INIT;
    io_dev_ptr->DEV_DEINIT          = init_ptr->DEINIT;
    io_dev_ptr->DEV_IOCTL           = init_ptr->IOCTL;
    io_dev_ptr->DEV_INIT_DATA_PTR   = init_ptr->INIT_DATA_PTR;
    
    _lwsem_create(&io_dev_ptr->LWSEM, 1);
    
    return (_io_dev_install_ext(
        init_ptr->ID_PTR,
        _io_sai_dma_open,
        _io_sai_dma_close,
        NULL,
        NULL,
        _io_sai_dma_ioctl,
        _io_sai_dma_uninstall,
        (void *)io_dev_ptr));

} /* Endbody */

/*FUNCTION****************************************************************
*
* Function Name    : _io_sai_dma_uninstall
* Returned Value   : MQX error code
* Comments         :
*    UnInstall interrupt SAI device.
*
*END**********************************************************************/

_mqx_int _io_sai_dma_uninstall
(
    /* [IN] The IO device structure for the device */
    IO_DEVICE_STRUCT_PTR    io_dev_ptr
)
{ /* Body */
    IO_SAI_DEVICE_STRUCT_PTR io_sai_dev_ptr = io_dev_ptr->DRIVER_INIT_PTR;

    if ((io_sai_dev_ptr->READ_COUNT == 0) && (io_sai_dev_ptr->WRITE_COUNT == 0))
    {
        _lwsem_destroy(&io_sai_dev_ptr->LWSEM);
        _mem_free (io_sai_dev_ptr);
        io_dev_ptr->DRIVER_INIT_PTR = NULL;
        return IO_OK;
    } else {
        return IO_ERROR_DEVICE_BUSY;
    }
} /* Endbody */

/*FUNCTION****************************************************************
*
* Function Name    : _io_sai_dma_open
* Returned Value   : MQX error code
* Comments         :
*    This routine initializes the SAI I/O channel. It acquires
*    memory, then stores information into it about the channel.
*    It then calls the hardware interface function to initialize the channel.
*
*END**********************************************************************/

_mqx_int _io_sai_dma_open
(
    /* [IN] the file handle for the device being opened */
    MQX_FILE_PTR    fd_ptr,

    /* [IN] the remaining portion of the name of the device */
    char        *open_name_ptr,

    /* [IN] the flags to be used during operation */
    char        *flags
)
{ /* Body */
    IO_DEVICE_STRUCT_PTR        io_dev_ptr;
    IO_SAI_DEVICE_STRUCT_PTR    io_sai_dev_ptr;
    _mqx_int result = MQX_OK;
    uint32_t i = 0;
    bool read_open = FALSE;
    bool write_open = FALSE;
    
    io_dev_ptr = (IO_DEVICE_STRUCT_PTR)fd_ptr->DEV_PTR;
    io_sai_dev_ptr = (void *)(io_dev_ptr->DRIVER_INIT_PTR);

 
    io_sai_dev_ptr->FLAGS = 0;
    
    if (flags == NULL)
    {
        flags = "rw";
    }
    while (flags[i] != '\0')
    {
        if (flags[i] == 'r') // open device for reading
        {
            read_open = TRUE;
        }
        if (flags[i++] == 'w') // open device for writing
        {
            write_open = TRUE;
        }
    }
    /* If we want to open device for reading */
    if (read_open)
    {
        _int_disable ();
        if (io_sai_dev_ptr->READ_COUNT)
        {
            /* Device is already opened for reading */
            _int_enable ();
            I2S_LOG("\n  _sai_open: Error - Rx already opened!. \n");
            return MQX_IO_OPERATION_NOT_AVAILABLE;
        }
        else
        {
             io_sai_dev_ptr->READ_COUNT = 1;
             _int_enable ();
        }
        io_sai_dev_ptr->FLAGS |= I2S_IO_READ;
        result = (*io_sai_dev_ptr->DEV_INIT)(io_sai_dev_ptr, I2S_IO_READ);
        if (result != MQX_OK)
        {
            _int_disable();
            io_sai_dev_ptr->READ_COUNT = 0;
            _int_enable();
            I2S_LOG("\n  _sai_open: Error - SAI Rx open failed!. \n");
            return result;
        }
    }
    /* If we want to open device for writing */
    if (write_open)
    {
        _int_disable ();
        if (io_sai_dev_ptr->WRITE_COUNT)
        {
            /* Device is already opened for reading */
            _int_enable ();
            I2S_LOG("\n  _sai_open: Error - Tx already opened!. \n");
            return MQX_IO_OPERATION_NOT_AVAILABLE;
        }
        else
        {
            io_sai_dev_ptr->WRITE_COUNT = 1;
            _int_enable();
        }
        io_sai_dev_ptr->FLAGS |= I2S_IO_WRITE;
        result = (*io_sai_dev_ptr->DEV_INIT)(io_sai_dev_ptr, I2S_IO_WRITE);
        if (result != MQX_OK)
        {
            _int_disable();
            io_sai_dev_ptr->WRITE_COUNT = 0;
            _int_enable();
            I2S_LOG("\n  _sai_open: Error - SAI Tx open failed!. \n");
            return result;
        }
    }

    fd_ptr->FLAGS         = (_mqx_uint)flags;
    return result;
} /* Endbody */

/*FUNCTION****************************************************************
*
* Function Name    : _io_sai_int_close
* Returned Value   : MQX error code
* Comments         :
*    This routine closes the SAI I/O channel.
*
*END**********************************************************************/

_mqx_int _io_sai_dma_close
(
    /* [IN] the file handle for the device being closed */
    MQX_FILE_PTR    fd_ptr
)
{ /* Body */
    IO_DEVICE_STRUCT_PTR        io_dev_ptr;
    IO_SAI_DEVICE_STRUCT_PTR    io_sai_dev_ptr;
    _mqx_int result = MQX_OK;

    io_dev_ptr     = (IO_DEVICE_STRUCT_PTR)fd_ptr->DEV_PTR;
    io_sai_dev_ptr = (void *)io_dev_ptr->DRIVER_INIT_PTR;

    _int_disable();
    if (io_sai_dev_ptr->READ_COUNT > 0)
    {
        io_sai_dev_ptr->READ_COUNT = 0;
        if (io_sai_dev_ptr->DEV_DEINIT)
        {
            result = (*io_sai_dev_ptr->DEV_DEINIT)(io_sai_dev_ptr, I2S_IO_READ);
        }        
    }
    if (io_sai_dev_ptr->WRITE_COUNT > 0) 
    {
        io_sai_dev_ptr->WRITE_COUNT = 0;
        if (io_sai_dev_ptr->DEV_DEINIT)
        {
            result = (*io_sai_dev_ptr->DEV_DEINIT)(io_sai_dev_ptr, I2S_IO_WRITE);
        }        
    }
    _int_enable();

    return result;
} /* Endbody */

/*FUNCTION*****************************************************************
*
* Function Name    : _io_sai_dma_ioctl
* Returned Value   : MQX error code
* Comments         :
*    Returns result of SAI ioctl operation.
*
*END*********************************************************************/

_mqx_int _io_sai_dma_ioctl
(
    /* [IN] the file handle for the device */
    MQX_FILE_PTR    fd_ptr,

    /* [IN] the ioctl command */
    _mqx_uint       cmd,

    /* [IN] the ioctl parameters */
    void           *input_param_ptr
)
{ /* Body */
    IO_DEVICE_STRUCT_PTR        io_dev_ptr;
    IO_SAI_DEVICE_STRUCT_PTR    io_sai_dev_ptr;
    _mqx_int                    result = MQX_OK;
    _mqx_uint_ptr               param_ptr = (_mqx_uint_ptr)input_param_ptr;

    io_dev_ptr     = (IO_DEVICE_STRUCT_PTR)fd_ptr->DEV_PTR;
    io_sai_dev_ptr = (void *)io_dev_ptr->DRIVER_INIT_PTR;

    switch (cmd) {
        case IO_IOCTL_DEVICE_IDENTIFY :
            /* return the device identify */
            param_ptr[0] = 0;
            param_ptr[1] = 0;
            param_ptr[2] = IO_DEV_ATTR_INTERRUPT | IO_DEV_ATTR_READ | IO_DEV_ATTR_WRITE;
            result = MQX_OK;
            break;
        default:
            if (io_sai_dev_ptr->DEV_IOCTL != NULL)
            {
                result = (*io_sai_dev_ptr->DEV_IOCTL)(io_sai_dev_ptr->DEV_INFO_PTR, cmd, param_ptr);
            }
    }
    return result;
} /* Endbody */

/* EOF */
