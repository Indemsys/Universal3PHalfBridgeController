
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
*   This file contains the function for adding two tick structs
*
*
*END************************************************************************/

#include "mqx_inc.h"

#if MQX_HAS_TICK

/*!
 * \brief Adds two PSP tick structure together, including hardware ticks
 * 
 * \param[in] a_ptr The two structures to add - both must be normalized
 * \param[in] b_ptr 
 * \param[out] r_ptr The result of the addition
 */
void _psp_add_ticks
   (
       /* [IN] The two structures to add - both must be normalized */
       PSP_TICK_STRUCT_PTR a_ptr,
       PSP_TICK_STRUCT_PTR b_ptr,

       /* [OUT] The result of the addition */
       PSP_TICK_STRUCT_PTR r_ptr
   )
{ /* Body */
   register uint32_t       a_hw_ticks;
   register uint32_t       b_hw_ticks;
   register uint32_t       hwtpt;

   r_ptr->TICKS[0] = a_ptr->TICKS[0] + b_ptr->TICKS[0];

   a_hw_ticks  = a_ptr->HW_TICKS[0];
   b_hw_ticks  = b_ptr->HW_TICKS[0];

   hwtpt = _mqx_kernel_data->HW_TICKS_PER_TICK;

   if ( a_hw_ticks >= (hwtpt - b_hw_ticks)) {
      r_ptr->TICKS[0]++;
      r_ptr->HW_TICKS[0] = a_hw_ticks + (b_hw_ticks - hwtpt);
   } else {
      r_ptr->HW_TICKS[0] = a_hw_ticks + b_hw_ticks;
   } /* Endif */

} /* Endbody */

#endif

/* EOF */

