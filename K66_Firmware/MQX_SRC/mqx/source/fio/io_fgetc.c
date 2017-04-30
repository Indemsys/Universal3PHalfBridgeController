
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
*   This file contains the functions for reading a character.
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
 * \brief This function returns the next character as an unsigned char (converted 
 * to an int) or IO_EOF if end of file or error occurs.
 *  
 * \param[in] file_ptr The stream to read the character from.
 * 
 * \return Converted character.
 * \return IO_EOF  
 */ 
_mqx_int _io_fgetc
   (
      MQX_FILE_PTR file_ptr
   )
{ /* Body */
   IO_DEVICE_STRUCT_PTR   dev_ptr;
   char                   tmp;
   _mqx_int               ungot;

#if MQX_CHECK_ERRORS
   if (file_ptr == NULL) {
      return(IO_EOF);
   } /* Endif */
#endif

   if ( file_ptr->HAVE_UNGOT_CHARACTER ) {
      ungot = file_ptr->UNGOT_CHARACTER;
      file_ptr->HAVE_UNGOT_CHARACTER = FALSE;
      return ungot;
   } /* Endif */

   dev_ptr = file_ptr->DEV_PTR;
#if MQX_CHECK_ERRORS
   if (dev_ptr->IO_READ == NULL) {
      file_ptr->ERROR = MQX_IO_OPERATION_NOT_AVAILABLE;
      return(IO_EOF);
   } /* Endif */
#endif

   if ((*dev_ptr->IO_READ)(file_ptr, &tmp, 1) == 1) {
      return (_mqx_int)(unsigned char)tmp;
   } else {
      return(IO_EOF);
   } /* Endif */

} /* Endbody */

#endif // MQX_USE_IO_OLD
