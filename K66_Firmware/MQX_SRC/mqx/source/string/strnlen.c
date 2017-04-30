
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
*   This file contains the function for calculating the length of a 
*   string, length limited.
*
*
*END************************************************************************/

#include "mqx_inc.h"
#include "mqx_str.h"

/*!
 * \brief Gets the length of the length-limited string.
 * 
 * \param[in] string_ptr The address of the string.
 * \param[in] max_len    Maximum number of characters in the string.
 * 
 * \return The number of characters in the string.
 * 
 * \see _str_mqx_uint_to_hex_string 
 */ 
_mqx_uint _strnlen
   (
      register char  *string_ptr,
      register _mqx_uint max_len
   )
{ /* Body */
   register _mqx_uint i = 0;

   
   while (*string_ptr++ && max_len--) {
      ++i;
   } /* Endwhile */

   return(i);
   
} /* Endbody */

/* EOF */
