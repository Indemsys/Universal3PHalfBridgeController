
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
*   This file contains the function sscanf.
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
 * \brief  This function performs similarly to the 'C' sscanf function.
 * 
 * \param[in] str_ptr    The string to scan from.
 * \param[in] format_ptr The format string to scan with.
 * 
 * \return Number of input items converted and assigned. 
 */ 
_mqx_int _io_sscanf
   (
      char  *str_ptr,
      const char  *format_ptr, 
      ...
   )
{ /* Body */
   va_list ap;
   _mqx_int result;
   
   va_start(ap, format_ptr);
   result = _io_scanline(str_ptr, (char *)format_ptr, ap);
   va_end(ap);
   return result;

} /* Endbody */

#endif // MQX_USE_IO_OLD
