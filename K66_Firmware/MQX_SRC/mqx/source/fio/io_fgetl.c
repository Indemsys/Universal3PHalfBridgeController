
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
*   This file contains the function for reading an input line.
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
 * \brief Returns the number of characters read into the input line.
 *
 * The terminating line feed is stripped.
 *
 * \param[in]     file_ptr   The stream to read the characters from.
 * \param[in,out] str_ptr    Where to store the input characters.
 * \param[in]     max_length The maximum number of characters to store.
 *
 * \return Number of characters read.
 * \return IO_EOF
 */
_mqx_int _io_fgetline
   (
      MQX_FILE_PTR file_ptr,
      char    *str_ptr,
      _mqx_int     max_length
   )
{ /* Body */
   _mqx_int  c;
   _mqx_int  i;
   _mqx_uint flags;

#if MQX_CHECK_ERRORS
   if (file_ptr == NULL) {
      *str_ptr = '\0';
      return(IO_EOF);
   } /* Endif */
#endif

   if (max_length) {
      max_length--;  /* Need to leave 1 space for the null termination */
   } else {
      max_length = MAX_MQX_INT;  /* Effectively infinite length */
   } /* Endif */

   c = _io_fgetc(file_ptr);
   if (c == IO_EOF) {
      *str_ptr = '\0';
      return(IO_EOF);
   } /* Endif */
   flags = file_ptr->FLAGS;
   i = 0;
   while ( (! ((c == '\n') || (c == '\r'))) && (i < max_length) ) {
      if ((flags & IO_FLAG_TEXT) && (c == '\b')) {
         if ( i ) {
            *--str_ptr = ' ';
            --i;
         } /* Endif */
      } else {
         *str_ptr++ = (char)c;
         ++i;
      } /* Endif */
      c = _io_fgetc(file_ptr);
      if (c == IO_EOF) {
         *str_ptr = '\0'; /* null terminate the string before returning */
         return(IO_EOF);
      } /* Endif */
   } /* Endwhile */
   if (i >= max_length) {
      _io_fungetc((_mqx_int)c, file_ptr);
   } else {
      if (('\r' == c) && (i < max_length)) {
         c = _io_fgetc(file_ptr);
         if (c == IO_EOF) {
            *str_ptr = '\0'; /* null terminate the string before returning */
            return(IO_EOF);
         } /* Endif */
         if (c == '\n'){
            /* Strip end of line character */
         } else {
           /* Unget valid char for next read */
            _io_fungetc((_mqx_int)c, file_ptr);
         }
      }
   } /* Endif */

   *str_ptr = '\0';

   return (i);

} /* Endbody */

#endif // MQX_USE_IO_OLD
