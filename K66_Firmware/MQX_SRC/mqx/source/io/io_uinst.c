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
*   This file contains the function for un-installing a dynamic device
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
* Function Name    : _io_dev_uninstall
* Returned Value   : _mqx_int a task error code or MQX_OK
* Comments         :
*    Un-Install a device dynamically.
*
*END*----------------------------------------------------------------------*/

_mqx_int _io_dev_uninstall
   (
      /* [IN] A string that identifies the device for fopen */
      char           *identifier
   )
{ /* Body */
   KERNEL_DATA_STRUCT_PTR kernel_data;
   IO_DEVICE_STRUCT_PTR   dev_ptr;
   _mqx_int               result = IO_OK;

   _GET_KERNEL_DATA(kernel_data);

   /* Find the device */
   _lwsem_wait((LWSEM_STRUCT_PTR)&kernel_data->IO_LWSEM);
   dev_ptr = (IO_DEVICE_STRUCT_PTR)((void *)kernel_data->IO_DEVICES.NEXT);
   while (dev_ptr != (void *)&kernel_data->IO_DEVICES.NEXT) {
      if (!strncmp(identifier, dev_ptr->IDENTIFIER, IO_MAXIMUM_NAME_LENGTH)) {
         /* Found it */
         if (dev_ptr->IO_UNINSTALL != NULL) {
            result = (*dev_ptr->IO_UNINSTALL)(dev_ptr);
            if (result == IO_OK) {
               _QUEUE_REMOVE(&kernel_data->IO_DEVICES, dev_ptr);
               _mem_free(dev_ptr);
            } /* Endif */
         } /* Endif */
         _lwsem_post((LWSEM_STRUCT_PTR)&kernel_data->IO_LWSEM);
         return(result);
      } /* Endif */
      dev_ptr = (IO_DEVICE_STRUCT_PTR)((void *)dev_ptr->QUEUE_ELEMENT.NEXT);
   } /* Endwhile */
   _lwsem_post((LWSEM_STRUCT_PTR)&kernel_data->IO_LWSEM);

   return(IO_DEVICE_DOES_NOT_EXIST);

} /* Endbody */

#endif // MQX_USE_IO_OLD
