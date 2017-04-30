
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
*   This file contains the function scanf.
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
 * \brief This function performs similarly to the scanf function found in 'C'.
 * 
 * \param[in] fmt_ptr The format string to scan with.
 * 
 * \return The number of input items converted and assigned.
 * \return IO_EOF (Failure.) 
 */ 
_mqx_int _io_scanf
   ( 
      const char  *fmt_ptr, 
      ...
   )
{ /* Body */
   char    temp_buf[IO_MAXLINE];
   va_list ap;
   _mqx_int result;
   
   va_start(ap, fmt_ptr);
   temp_buf[0] = '\0';
   if (_io_fgetline(stdin, temp_buf, IO_MAXLINE) == IO_EOF) {
      return(IO_EOF);
   } /* Endif */
   result = _io_scanline(temp_buf, (char *)fmt_ptr, ap);
   va_end(ap);
   return result;

} /* Endbody */

#endif // MQX_USE_IO_OLD
