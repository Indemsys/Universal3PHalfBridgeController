
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
*   This file contains the function for sprintf and snprintf.
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
 * \brief Performs similarly to the sprintf function found in 'C'.
 * 
 * The returned number of characters does not include the terminating '\0'
 * 
 * \param[in] str_ptr The string to print into.
 * \param[in] fmt_ptr The format specifier.
 * 
 * \return Number of characters
 * \return IO_EOF (End Of File found.)   
 */ 
_mqx_int _io_sprintf
   (
      char         *str_ptr,
      const char   *fmt_ptr,
      ...
   )
{ /* Body */
   _mqx_int result;
   va_list ap;
   
   va_start(ap, fmt_ptr);
   result = _io_doprint((MQX_FILE_PTR)((void *)&str_ptr), _io_sputc, -1, (char *)fmt_ptr, ap);
   *str_ptr = '\0';
   va_end(ap);
   return result;

} /* Endbody */

/*!
 * \brief This function performs similarly to the sprintf function found in 'C'.
 * 
 * The returned number of characters does not include the terminating '\0'.
 * 
 * \param[in] str_ptr   The string to print into.
 * \param[in] max_count The maximal size of string.
 * \param[in] fmt_ptr   The format specifier.
 * 
 * \return Number of characters
 * \return IO_EOF (End Of File found.) 
 */ 
_mqx_int _io_snprintf
   (
      char           *str_ptr,
      _mqx_int      max_count,
      const char     *fmt_ptr,
      ...
   )
{
    _mqx_int result;

    va_list ap;
    va_start(ap, fmt_ptr);
    result = _io_doprint((MQX_FILE_PTR)((void *)&str_ptr), _io_sputc, max_count, (char *)fmt_ptr, ap);
    va_end(ap);
    if (0 != max_count)
    {
        *str_ptr = '\0';
    }
   
    return result;

}

/*!
 * \brief Erites the character into the string located by the string pointer and
 * updates the string pointer.
 * 
 * \param[in]      c                The character to put into the string.
 * \param[in, out] input_string_ptr This is an updated pointer to a string pointer.
 * 
 * \return Character written into string. 
 */ 
_mqx_int _io_sputc
   (
      _mqx_int     c,
      MQX_FILE_PTR input_string_ptr
   )
{ /* Body */
   char           **string_ptr = (char  **)((void *)input_string_ptr);

   *(*string_ptr)++ = (char)c;
   return c;

} /* Endbody */

#endif // MQX_USE_IO_OLD
