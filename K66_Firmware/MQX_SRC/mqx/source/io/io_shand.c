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
*   This file contains the function for setting the default I/O stream.
*   in the kernel data structure, or for a task.
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
* Function Name    : _io_set_handle
* Returned Value   : MQX_FILE_PTR (BUT as a void *)
* Comments         : 
*    This function changes the address of a default I/O handle, and returns
*    the previous one.  If an incorrect type is given, or the I/O handle was
*    uninitialized, 0 is returned.
*
*END*----------------------------------------------------------------------*/

void *_io_set_handle
   (
      /* [IN] which I/O handle to modify */
      _mqx_uint stdio_type,

      /* [IN] the new I/O handle */    
      void   *new_file_ptr
   )
{ /* Body */
   register KERNEL_DATA_STRUCT_PTR  kernel_data;
   register TD_STRUCT_PTR           active_ptr;
   register void                   *old_file_ptr;
   
   _GET_KERNEL_DATA(kernel_data);
   _KLOGE3(KLOG_io_set_handle, stdio_type, new_file_ptr);
   
   switch ( (uint16_t)stdio_type ) {

      case IO_PROC_STDIN:
         old_file_ptr = kernel_data->PROCESSOR_STDIN;
         kernel_data->PROCESSOR_STDIN = new_file_ptr;
         break;

      case IO_PROC_STDOUT:
         old_file_ptr = kernel_data->PROCESSOR_STDOUT;
         kernel_data->PROCESSOR_STDOUT = new_file_ptr;
         break;

      case IO_PROC_STDERR:
         old_file_ptr = kernel_data->PROCESSOR_STDERR;
         kernel_data->PROCESSOR_STDERR = new_file_ptr;
         break;

      case IO_STDIN:
         active_ptr = kernel_data->ACTIVE_PTR;
         old_file_ptr = active_ptr->STDIN_STREAM;
         active_ptr->STDIN_STREAM = new_file_ptr;
         break;

      case IO_STDOUT:
         active_ptr = kernel_data->ACTIVE_PTR;
         old_file_ptr = active_ptr->STDOUT_STREAM;
         active_ptr->STDOUT_STREAM = new_file_ptr;
         break;

      case IO_STDERR:
         active_ptr = kernel_data->ACTIVE_PTR;
         old_file_ptr = active_ptr->STDERR_STREAM;
         active_ptr->STDERR_STREAM = new_file_ptr;
         break;

      default:
         old_file_ptr = NULL;

   } /* Endswitch */

   _KLOGX2(KLOG_io_set_handle, old_file_ptr);
   return (old_file_ptr);

} /* Endbody */

#endif // MQX_USE_IO_OLD
