
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
*   This file contains the function for checking the status of input.
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
 * \brief Checks the HAVE_UNGOT_CHARACTER of the input_ucb and if none is 
 * available, call the device status function.
 * 
 * \param[in] file_ptr The stream whose status is desired.
 * 
 * \return TRUE (HAVE_UNGOT_CHARACTER is TRUE), FALSE (HAVE_UNGOT_CHARACTER is 
 * TRUE).
 * \return Value form device status function.  
 */ 
bool _io_fstatus
   (
      MQX_FILE_PTR file_ptr
   )
{ /* Body */
   IO_DEVICE_STRUCT_PTR   dev_ptr;
   bool                result;

#if MQX_CHECK_ERRORS
   if (file_ptr == NULL) {
      return (FALSE);
   } /* Endif */
#endif

   if ( file_ptr->HAVE_UNGOT_CHARACTER ) {
      return (TRUE);
   } else {
      dev_ptr = file_ptr->DEV_PTR;
      if (dev_ptr->IO_IOCTL != NULL) {   
         (*dev_ptr->IO_IOCTL)(file_ptr, IO_IOCTL_CHAR_AVAIL, &result);
         return(result);
      } /* Endif */
   } /* Endif */
   return (FALSE);

} /* Endbody */

#endif // MQX_USE_IO_OLD
