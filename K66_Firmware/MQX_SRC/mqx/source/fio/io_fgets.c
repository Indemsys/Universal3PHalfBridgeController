
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
*   This file contains the function for getting a string.
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
 * \brief Reads the specified string
 * 
 * This function reads at most the next size-1 characters into the array 
 * tty_line_ptr, stopping if a newline is encountered; The newline is included 
 * in the array, which is terminated by '\0'.
 * 
 * \param[in,out] tty_line_ptr Where to store the input string.
 * \param[in]     size         The maximum length to store.
 * \param[in]     file_ptr     The stream to read from.
 * 
 * \return Pointer to the character array.
 * \return NULL  (End of file or error.) 
 */ 
char  *_io_fgets
   (
      char   *tty_line_ptr,
      _mqx_int    size,
      MQX_FILE_PTR file_ptr
   )
{ /* Body */
   _mqx_int result;

#if MQX_CHECK_ERRORS
   if (file_ptr == NULL) {
      return(NULL);
   } /* Endif */
#endif

   /* Null terminate the buffer so we can tell if _io_fgetline() actually
      read any data... */

   tty_line_ptr[0] = '\0';

   /* Attempt to read a line of text from file_ptr */

   result = _io_fgetline(file_ptr, tty_line_ptr, size);

   /* If _io_fgetline() returned IO_EOF *and* no data was
      read into the tty_line_ptr buffer, return NULL... */

   if ((result == IO_EOF) && (tty_line_ptr[0] == '\0')) {
      return(NULL);
   } /* Endif */

   /* Otherwise, at least 1 byte was read so return the buffer pointer */

   return tty_line_ptr;

} /* Endbody */

#endif // MQX_USE_IO_OLD
