
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
*
*   This file contains the function for returning the previous vector
*   table field in the kernel data structure.
*
*
*END************************************************************************/


#include "mqx_inc.h"

#if MQX_EXIT_ENABLED && MQX_USE_INTERRUPTS
/*!
 * \brief Gets the address of the interrupt vector table that MQX might have 
 * created when it started.
 * 
 * The function is useful if you are installing third-party debuggers or monitors.
 * 
 * \return Address of the interrupt vector table that MQX creates when it starts.
 * 
 * \see _int_get_vector_table
 * \see _int_set_vector_table
 */       
_psp_code_addr _int_get_previous_vector_table
   (
      void
   )
{ /* Body */
   register KERNEL_DATA_STRUCT_PTR  kernel_data;

   _GET_KERNEL_DATA(kernel_data);
   return( (_psp_code_addr)kernel_data->USERS_VBR );

} /* Endbody */
#endif

/* EOF */
