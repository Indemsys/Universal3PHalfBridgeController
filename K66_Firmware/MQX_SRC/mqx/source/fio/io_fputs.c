
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
*   This file contains the functions for printing a string
*
*
*END************************************************************************/
#include "mqx_cnfg.h"
#if MQX_USE_IO_OLD

#include <string.h>
#include "mqx.h"
#include "fio.h"
#include "fio_prv.h"
#include "io.h"
#include "io_prv.h"

/*!
 * \brief Writes a sting.
 * 
 * \param[in] string_ptr The string to print out.
 * \param[in] file_ptr   The stream upon which to print out the string.
 * 
 * \return Written string.
 * \return IO_EOF (Failure.)    
 */ 
_mqx_int _io_fputs
   (
      const char   *string_ptr,
      MQX_FILE_PTR      file_ptr
   )
{ /* Body */
   _mqx_int result;

#if MQX_CHECK_ERRORS
   if (file_ptr == NULL) {
      return(IO_EOF);
   } /* Endif */
#endif

   result = _io_write(file_ptr, (char *)string_ptr, strlen(string_ptr));

   return(result);

} /* Endbody */

#endif // MQX_USE_IO_OLD
