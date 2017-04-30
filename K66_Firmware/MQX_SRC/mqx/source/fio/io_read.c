
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
*   Contains the function read.
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

/*!
 * \brief Reads form the specified stream.
 * 
 * \param[in] file_ptr The stream to perform the operation on.
 * \param[in] data_ptr The data location to read to.
 * \param[in] num      The number of bytes to read.
 * 
 * \return Number of characters read.
 * \return IO_ERROR   
 */ 
_mqx_int _io_read
   ( 
      MQX_FILE_PTR file_ptr,
      void        *data_ptr,
      _mqx_int     num      
   )
{ /* Body */
   IO_DEVICE_STRUCT_PTR   dev_ptr;
   char               *data = (char *) data_ptr;
   _mqx_int               res = 0;

#if MQX_CHECK_ERRORS
   if (file_ptr == NULL) {
      return(IO_ERROR);
   } /* Endif */
#endif

   if (!file_ptr->HAVE_UNGOT_CHARACTER || num > 1) {

        dev_ptr = file_ptr->DEV_PTR;
#if MQX_CHECK_ERRORS
        if (dev_ptr->IO_READ == NULL) {
            file_ptr->ERROR = MQX_IO_OPERATION_NOT_AVAILABLE;
            return (IO_ERROR);
        } /* Endif */
#endif

        if (file_ptr->HAVE_UNGOT_CHARACTER) {
            res = (*dev_ptr->IO_READ)(file_ptr, data+1, num-1);
        }
        else {
            res = (*dev_ptr->IO_READ)(file_ptr, data, num);
        }
        if (res<0) return res;
   }
      
   if ( file_ptr->HAVE_UNGOT_CHARACTER ) {
       if (num <= 0) return 0;
       *data = (char)file_ptr->UNGOT_CHARACTER;
       file_ptr->HAVE_UNGOT_CHARACTER = FALSE;
       res++;
   } /* Endif */

   return res;
} /* Endbody */

#endif // MQX_USE_IO_OLD
