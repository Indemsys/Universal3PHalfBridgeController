
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
*   This file contains the function _io_ioctl.
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
 * \brief Performs specified io_ctl command.
 * 
 * \param[in] file_ptr  The stream to perform the operation on.
 * \param[in] cmd       The ioctl command.
 * \param[in] param_ptr The ioctl parameters.
 * 
 * \return MQX_OK (Succes.)
 * \return IO_EOF
 * \return MQX error code.  
 */ 
_mqx_int _io_ioctl
   ( 
      MQX_FILE_PTR file_ptr,
      _mqx_uint   cmd,
      void       *param_ptr
   )
{ /* Body */
   IO_DEVICE_STRUCT_PTR   dev_ptr;
   _mqx_uint_ptr          tmp_ptr;
   _mqx_uint              result = MQX_OK;

#if MQX_CHECK_ERRORS
   if (file_ptr == NULL) {
      return(IO_EOF);
   } /* Endif */
#endif

   tmp_ptr = (_mqx_uint_ptr)param_ptr;

   switch (cmd) {
      case IO_IOCTL_GET_FLAGS:
         *tmp_ptr = file_ptr->FLAGS;
         break;
      case IO_IOCTL_SET_FLAGS:
         file_ptr->FLAGS = *tmp_ptr;
         break;
      default:
         dev_ptr = file_ptr->DEV_PTR;
         if (dev_ptr->IO_IOCTL != NULL) {
            result = (*dev_ptr->IO_IOCTL)(file_ptr, cmd, param_ptr);
         }
         else
            result = IO_ERROR_INVALID_IOCTL_CMD;
         
         break;
   } /* Endswitch */

   return(result);

} /* Endbody */


#endif // MQX_USE_IO_OLD
