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
*   This file contains the function for returning the current I/O handler
*   in the kernel data structure, or in a task.
*
*
*END************************************************************************/

#include "mqx_cnfg.h"
#if MQX_USE_IO_OLD

#include "mqx_inc.h"
#include "fio.h"
#include "io.h"

/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : _io_get_handle
* Returned Value   : pointer
* Comments         : 
*
*    This function returns the address of a default standard I/O FILE.
*    If an incorrect type is given, or the file_ptr has not been specified,
*    the function will return NULL.
* 
*END*----------------------------------------------------------------------*/

void *_io_get_handle
   (
      /* [IN] which I/O handle to return */
      _mqx_uint stdio_type
   )
{ /* Body */
   register KERNEL_DATA_STRUCT_PTR  kernel_data;

   _GET_KERNEL_DATA(kernel_data);
   switch ( (uint16_t)stdio_type ) {

      case IO_PROC_STDIN:
         return kernel_data->PROCESSOR_STDIN;

      case IO_PROC_STDOUT:
         return kernel_data->PROCESSOR_STDOUT;

      case IO_PROC_STDERR:
         return kernel_data->PROCESSOR_STDERR;

      case IO_STDIN:
         return kernel_data->ACTIVE_PTR->STDIN_STREAM;

      case IO_STDOUT:
         return kernel_data->ACTIVE_PTR->STDOUT_STREAM;

      case IO_STDERR:
         return kernel_data->ACTIVE_PTR->STDERR_STREAM;

      default:
         return (void *) NULL;

   } /* Endswitch */

} /* Endbody */

#endif // MQX_USE_IO_OLD
