
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
*   This file contains the functions for vprintf, vfprintf and vsprintf.
*   These functions are equivalent to the corresponding printf functions,
*   except that the variable argument list is replaced by one argument,
*   which has been initialized by the va_start macro.
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
 * \brief This function is equivalent to the corresponding printf function, except 
 * that the variable argument list is replaced by one argument, which has been 
 * initialized by the va_start macro.
 * 
 * \param[in] fmt_ptr The format string.
 * \param[in] arg     The arguments.
 * 
 * \return Number of characters
 * \return IO_EOF (End Of File found.)   
 */ 
_mqx_int _io_vprintf
   (
      const char  *fmt_ptr, 
      va_list          arg
   )
{ /* Body */
   _mqx_int result;
   
   result = _io_doprint(stdout, _io_fputc, -1, (char *)fmt_ptr, arg);

   return result;

} /* Endbody */

/*!
 * \brief This function is equivalent to the corresponding printf function, except 
 * that the variable argument list is replaced by one argument, which has been 
 * initialized by the va_start macro.
 * 
 * \param[in] file_ptr The stream to print upon.
 * \param[in] fmt_ptr  The format string to use for printing.
 * \param[in] arg      The argument list to print.
 * 
 * \return Number of characters
 * \return IO_EOF (End Of File found.)   
 */ 
_mqx_int _io_vfprintf
   (
      MQX_FILE_PTR     file_ptr,
      const char  *fmt_ptr,
      va_list          arg
   )
{ /* Body */
   _mqx_int result;
   
   result = 0;
   if ( file_ptr ) {
      result = _io_doprint(file_ptr, _io_fputc, -1, (char *)fmt_ptr, arg);
   } /* Endif */
   return result;

} /* Endbody */

/*!
 * \brief This function is quivalent to the corresponding printf function, except 
 * that the variable argument list is replaced by one argument, which has been 
 * initialized by the va_start macro.
 * 
 * \param[in] str_ptr The string to print into.
 * \param[in] fmt_ptr The format string.
 * \param[in] arg     The arguments.
 * 
 * \return Number of characters
 * \return IO_EOF (End Of File found.)   
 */ 
_mqx_int _io_vsprintf
   ( 
      char         *str_ptr,
      const char   *fmt_ptr,
      va_list           arg
   )
{ /* Body */
   _mqx_int result;
   
   result = _io_doprint((MQX_FILE_PTR)((void *)&str_ptr), _io_sputc, -1, (char *)fmt_ptr, arg);
   *str_ptr = '\0';
   return result;

} /* Endbody */

/*!
 * \brief This function is equivalent to the corresponding printf function, except 
 * that the variable argument list is replaced by one argument, which has been 
 * initialized by the va_start macro.
 * 
 * \param[in] str_ptr The string to print into.
 * \param[in] max_count The maximal size of string.
 * \param[in] fmt_ptr   The format specifier.
 * \param[in] arg       The arguments.
 * 
 * \return Number of characters
 * \return IO_EOF (End Of File found.)  
 */ 
_mqx_int _io_vsnprintf
   (
      char           *str_ptr,
      _mqx_int            max_count,
      const char     *fmt_ptr,
      va_list             arg
   )
{
    _mqx_int result;

    result = _io_doprint((MQX_FILE_PTR)((void *)&str_ptr), _io_sputc, max_count, (char *)fmt_ptr, arg);
    
    if (0 != max_count)
    {
        *str_ptr = '\0';
    }
    
    return result;

}

#endif // MQX_USE_IO_OLD
