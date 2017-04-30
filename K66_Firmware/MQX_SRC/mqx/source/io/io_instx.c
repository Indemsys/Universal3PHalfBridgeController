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
*   This file contains the function for installing a dynamic device
*   driver.
*
*
*END************************************************************************/

#include "mqx_cnfg.h"
#if MQX_USE_IO_OLD

#include <string.h>
#include "mqx_inc.h"
#include "fio.h"
#include "fio_prv.h"
#include "io.h"
#include "io_prv.h"

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _io_dev_install_drvif
* Returned Value   : _mqx_uint a task error code or MQX_OK
* Comments         :
*    Install a device dynamically, so tasks can fopen to it.
*    Entry points of the driver are specified by driver interface structure.
*
*END*----------------------------------------------------------------------*/

_mqx_int _io_dev_install_drvif
   (
      /* [IN] A string that identifies the device for fopen */
      char              *identifier,

      /* [IN] Pointer to driver interface structure (driver entry points) */
      IO_DRVIF_STRUCT const *drvif_ptr,

      /* [IN] The I/O initialization data */
      void              *io_init_data_ptr
   )
{
    KERNEL_DATA_STRUCT_PTR kernel_data;
    IO_DEVICE_STRUCT_PTR   dev_ptr;
#if MQX_CHECK_ERRORS
    _mqx_uint              i;
    _mqx_uint              found = 0;
#endif

    _GET_KERNEL_DATA(kernel_data);

#if MQX_CHECK_ERRORS
    if ((drvif_ptr == NULL) || (drvif_ptr->IO_OPEN == NULL) || (drvif_ptr->IO_OPEN == NULL)){
        return(MQX_INVALID_PARAMETER);
    }

    if (identifier == NULL){
        return(MQX_INVALID_PARAMETER);
    }

   /* Search for delimiter */
    for (i = 0; i < IO_MAXIMUM_NAME_LENGTH; i++) {
        if (identifier[i] == IO_DEV_DELIMITER) {
            found++;
        } else if (identifier[i] == '\0') {
            break;
        }
    }

    /*
    ** Return an error if more than 1 delimiter found, no delimiter was found
    ** or the identifier was composed of a single delimiter only.
    */
    if ((found != 1) || (i == 1)) {
        return(MQX_INVALID_PARAMETER);
    } /* Endif */
#endif

    _lwsem_wait((LWSEM_STRUCT_PTR)&kernel_data->IO_LWSEM);

#if MQX_CHECK_ERRORS
    /* Check to see if device already installed */
    dev_ptr = (IO_DEVICE_STRUCT_PTR)((void *)kernel_data->IO_DEVICES.NEXT);
    while (dev_ptr != (void *)&kernel_data->IO_DEVICES.NEXT) {
        if (!strncmp(identifier, dev_ptr->IDENTIFIER, IO_MAXIMUM_NAME_LENGTH)) {
            _lwsem_post((LWSEM_STRUCT_PTR)&kernel_data->IO_LWSEM);
            return(IO_DEVICE_EXISTS);
        } /* Endif */
        dev_ptr = (IO_DEVICE_STRUCT_PTR)((void *)dev_ptr->QUEUE_ELEMENT.NEXT);
    } /* Endwhile */
#endif

    dev_ptr = (IO_DEVICE_STRUCT_PTR)_mem_alloc_system_zero((_mem_size)sizeof(IO_DEVICE_STRUCT));
#if MQX_CHECK_MEMORY_ALLOCATION_ERRORS
    if (dev_ptr == NULL) {
        _lwsem_post((LWSEM_STRUCT_PTR)&kernel_data->IO_LWSEM);
        return(MQX_OUT_OF_MEMORY);
    }/* Endif */
#endif
    _mem_set_type(dev_ptr, MEM_TYPE_IO_DEVICE);

    dev_ptr->IDENTIFIER      = identifier;
    dev_ptr->IO_OPEN         = drvif_ptr->IO_OPEN;
    dev_ptr->IO_CLOSE        = drvif_ptr->IO_CLOSE;
    dev_ptr->IO_READ         = drvif_ptr->IO_READ;
    dev_ptr->IO_WRITE        = drvif_ptr->IO_WRITE;
    dev_ptr->IO_LSEEK        = drvif_ptr->IO_LSEEK;
    dev_ptr->IO_IOCTL        = drvif_ptr->IO_IOCTL;
    dev_ptr->IO_UNINSTALL    = drvif_ptr->IO_UNINSTALL;
    dev_ptr->DRIVER_INIT_PTR = io_init_data_ptr;

    _QUEUE_ENQUEUE(&kernel_data->IO_DEVICES, dev_ptr);

    _lwsem_post((LWSEM_STRUCT_PTR)&kernel_data->IO_LWSEM);
    return MQX_OK;
}


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _io_dev_install_ext
* Returned Value   : _mqx_uint a task error code or MQX_OK
* Comments         :
*    Install a device dynamically, so tasks can fopen to it. Different from
* _io_dev_install since this function also installs an uninstall function.
*
*END*----------------------------------------------------------------------*/

_mqx_int _io_dev_install_ext
   (
      /* [IN] A string that identifies the device for fopen */
      char             *identifier,

      /* [IN] The I/O open function */
      IO_OPEN_FPTR      io_open,

      /* [IN] The I/O close function */
      IO_CLOSE_FPTR     io_close,

      /* [IN] The I/O read function */
      IO_READ_FPTR      io_read,

      /* [IN] The I/O write function */
      IO_WRITE_FPTR     io_write,

      /* [IN] The I/O ioctl function */
      IO_IOCTL_FPTR     io_ioctl,

      /* [IN] The I/O un-install function */
      IO_UNINSTALL_FPTR io_uninstall,

      /* [IN] The I/O initialization data */
      void             *io_init_data_ptr
   )
{
    IO_DRVIF_STRUCT drvif;

    drvif.IO_OPEN      = io_open;
    drvif.IO_CLOSE     = io_close;
    drvif.IO_READ      = io_read;
    drvif.IO_WRITE     = io_write;
    drvif.IO_IOCTL     = io_ioctl;
    drvif.IO_UNINSTALL = io_uninstall;
    drvif.IO_LSEEK     = NULL;

    return _io_dev_install_drvif(identifier, &drvif, io_init_data_ptr);
}

#endif // MQX_USE_IO_OLD
