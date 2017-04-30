
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
*   Contains the function fopen.
*
*
*END************************************************************************/

#include "mqx_cnfg.h"
#if MQX_USE_IO_OLD

#include <string.h>
#include "mqx_inc.h"
#include "fio.h"
#include "fio_prv.h"
#include "io_prv.h"

/*!
 * \brief The returned value is a pointer to an I/O model.
 * 
 * \param[in] open_type_ptr The name of the device to open.
 * \param[in] open_mode_ptr I/O initialization parameter to pass to the device 
 * initialization.
 * 
 * \return Pointer to MQX_FILE structure
 * \return NULL (Failure.)  
 */ 
MQX_FILE_PTR _io_fopen
   ( 
      const char  *open_type_ptr,
      const char  *open_mode_ptr
   )
{ /* Body */
   KERNEL_DATA_STRUCT_PTR kernel_data;
   MQX_FILE_PTR           file_ptr;
   IO_DEVICE_STRUCT_PTR   dev_ptr;
   char                  *dev_name_ptr;
   char                  *tmp_ptr;
   _mqx_int                result;

   _GET_KERNEL_DATA(kernel_data);

   _lwsem_wait((LWSEM_STRUCT_PTR)&kernel_data->IO_LWSEM);
   dev_ptr = (IO_DEVICE_STRUCT_PTR)((void *)kernel_data->IO_DEVICES.NEXT);
   while (dev_ptr != (void *)&kernel_data->IO_DEVICES.NEXT) {
      dev_name_ptr = dev_ptr->IDENTIFIER;
      tmp_ptr      = (char *)open_type_ptr;
      while (*tmp_ptr && *dev_name_ptr &&
         (*tmp_ptr == *dev_name_ptr))
      {
         ++tmp_ptr;
         ++dev_name_ptr;
      } /* Endwhile */
      if (*dev_name_ptr == '\0') {
         /* Match */
         break;
      } /* Endif */
      dev_ptr = (IO_DEVICE_STRUCT_PTR)((void *)dev_ptr->QUEUE_ELEMENT.NEXT);
   } /* Endwhile */
   
   _lwsem_post((LWSEM_STRUCT_PTR)&kernel_data->IO_LWSEM);
   
   if (dev_ptr == (void *)&kernel_data->IO_DEVICES.NEXT) {
      return(NULL);
   } /* Endif */

   file_ptr = (MQX_FILE_PTR)_mem_alloc_system_zero((_mem_size)sizeof(MQX_FILE));
#if MQX_CHECK_MEMORY_ALLOCATION_ERRORS
   if (file_ptr == NULL) {
      return(NULL);
   } /* Endif */
#endif
   _mem_set_type(file_ptr, MEM_TYPE_FILE_PTR);
   
   file_ptr->DEV_PTR = dev_ptr;
   if (dev_ptr->IO_OPEN != NULL) {
      result = (*dev_ptr->IO_OPEN)(file_ptr, (char *)open_type_ptr, (char *)open_mode_ptr);
      if (result != MQX_OK) {
         _task_set_error(result);
         _mem_free(file_ptr);
         return(NULL);
      } /* Endif */
   } /* Endif */

   return(file_ptr);

} /* Endbody */

#endif // MQX_USE_IO_OLD
