
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
*   This file contains the function io_fflush.
*
*
*END************************************************************************/
#include "mqx_cnfg.h"
#if MQX_USE_IO_OLD

#include "mqx.h"
#include "fio.h"
#include "fio_prv.h"
#include "io.h"
#include "io_prv.h"

/*!
 * \brief This function causes any buffered but unwritten data to be written.
 * 
 * \param[in] file_ptr The stream whose status is desired.
 * 
 * \return MQX_OK
 * \return IO_EOF (Failure.) 
 */ 
_mqx_int _io_fflush
   (
      MQX_FILE_PTR file_ptr
   )
{ /* Body */
   IO_DEVICE_STRUCT_PTR   dev_ptr;
   _mqx_int               res = MQX_OK;

#if MQX_CHECK_ERRORS
   if (file_ptr == NULL) {
      return(IO_EOF);
   } /* Endif */
#endif

   dev_ptr = file_ptr->DEV_PTR;
   if (dev_ptr->IO_IOCTL != NULL) {   
      res = ((*dev_ptr->IO_IOCTL)(file_ptr,
         IO_IOCTL_FLUSH_OUTPUT, NULL));
   } /* Endif */

   /* Handle ungetc buffer in the same way as fseek */ 
   if (file_ptr->HAVE_UNGOT_CHARACTER) {
       _io_fseek(file_ptr, 0, IO_SEEK_CUR);
   }
   return res;

} /* Endbody */

#endif // MQX_USE_IO_OLD
