
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
*   This file contains the function for obtaining the address of the
*   stack start structure for the task.
*
*
*END************************************************************************/

#include "mqx_inc.h"

/*!
 * \brief This function returns the pointer to the stack start structure on the stack.
 * 
 * \param[in] td_ptr the task descriptor whose stack start struct address is wanted
 */
PSP_STACK_START_STRUCT_PTR _psp_get_stack_start
   (
      /* [IN] the task descriptor whose stack start struct address is wanted */
      TD_STRUCT_PTR td_ptr
   )
{ /* Body */

   return (void *)((unsigned char *)td_ptr->STACK_BASE -  sizeof(PSP_STACK_START_STRUCT));

} /* Endbody */

/* EOF */
