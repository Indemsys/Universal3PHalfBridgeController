
/*HEADER**********************************************************************
*
* Copyright 2010 Freescale Semiconductor, Inc.
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
*   This file contains the function for printing the ticks portion
*   of the PSP_TICK_STRUCT
*
*
*END************************************************************************/
#include "mqx_inc.h"
#if MQX_USE_IO_OLD
#include "fio.h"
#else
#include <stdio.h>
#endif

/*!
 * \brief Prints ticks in hex notation
 * 
 * \param tick_ptr 
 */
void _psp_print_ticks
   (
      PSP_TICK_STRUCT_PTR tick_ptr
   )
{ /* Body */
   PSP_64_BIT_UNION  tmp;
   int32_t            i;

   printf("0x");
   tmp.LLW = tick_ptr->TICKS[0];
#if PSP_ENDIAN == MQX_LITTLE_ENDIAN
   for (i = 7; i >= 0; i--) {
      printf("%X", tmp.B[i]);
   } /* Endfor */
#else
   for (i = 0; i <= 7; i++) {
      printf("%X", tmp.B[i]);
   } /* Endfor */
#endif
   printf(":%04lX", tick_ptr->HW_TICKS[0]);

} /* Endbody */

