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

#include "mqx_inc.h"
#include "fio.h"
#include "fio_prv.h"
#include "io.h"
#include "io_prv.h"
#ifdef NULL
#undef NULL
#endif
#include <string.h>
#ifndef NULL
#define NULL ((void *)0)
#endif

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _io_dev_install
* Returned Value   : _mqx_uint a task error code or MQX_OK
* Comments         :
*    Install a device dynamically, so tasks can fopen to it.
*
*END*----------------------------------------------------------------------*/

_mqx_int _io_dev_install
   (
      /* [IN] A string that identifies the device for fopen */
      char      *identifier,

      /* [IN] The I/O open function */
      IO_OPEN_FPTR  io_open,

      /* [IN] The I/O close function */
      IO_CLOSE_FPTR io_close,

      /* [IN] The I/O read function */
      IO_READ_FPTR  io_read,

      /* [IN] The I/O write function */
      IO_WRITE_FPTR io_write,

      /* [IN] The I/O ioctl function */
      IO_IOCTL_FPTR io_ioctl,

      /* [IN] The I/O initialization data */
      void         *io_init_data_ptr
   )
{
    IO_DRVIF_STRUCT drvif;

    drvif.IO_OPEN      = io_open;
    drvif.IO_CLOSE     = io_close;
    drvif.IO_READ      = io_read;
    drvif.IO_WRITE     = io_write;
    drvif.IO_IOCTL     = io_ioctl;
    drvif.IO_UNINSTALL = NULL;
    drvif.IO_LSEEK     = NULL;

    return _io_dev_install_drvif(identifier, &drvif, io_init_data_ptr);
}

#endif // MQX_USE_IO_OLD
