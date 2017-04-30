
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
*   This file contains the function for subtracting two tick structs
*
*
*END************************************************************************/

#include "mqx_inc.h"
#if MQX_HAS_TICK

/*!
 * \brief r = a - b
 * 
 * \param[in] a_ptr The two structures to subtract - both must be normalized
 * \param[in] b_ptr 
 * \param[out] r_ptr The result of the subtraction
 */
void _psp_subtract_ticks
   (
       /* [IN] The two structures to subtract - both must be normalized */
       PSP_TICK_STRUCT_PTR a_ptr,
       PSP_TICK_STRUCT_PTR b_ptr,

       /* [OUT] The result of the subtraction */
       PSP_TICK_STRUCT_PTR r_ptr
   )
{ /* Body */
   register uint32_t       a_hw_ticks;
   register uint32_t       b_hw_ticks;

   r_ptr->TICKS[0] = a_ptr->TICKS[0] - b_ptr->TICKS[0];

   a_hw_ticks  = a_ptr->HW_TICKS[0];
   b_hw_ticks  = b_ptr->HW_TICKS[0];

   if ( a_hw_ticks < b_hw_ticks) {
      a_hw_ticks += _mqx_kernel_data->HW_TICKS_PER_TICK;
      r_ptr->TICKS[0]--;
   } /* Endif */

   r_ptr->HW_TICKS[0] = a_hw_ticks - b_hw_ticks;

} /* Endbody */


/*!
 * \brief r = a - b, clamp into range <-(MAX_INT_32 + 1), MAX_INT_32>
 * 
 * \param[in] a_ptr The two structures to subtract - both must be normalized
 * \param[in] b_ptr 
 * \param[out] o_ptr The result of the subtraction
 */
int32_t _psp_subtract_ticks_int32
   (
       /* [IN] The two structures to subtract - both must be normalized */
       PSP_TICK_STRUCT_PTR a_ptr,
       PSP_TICK_STRUCT_PTR b_ptr,

       /* [OUT] The result of the subtraction */
       bool        *o_ptr
   )
{ /* Body */
   register uint32_t       a_hw_ticks;
   register uint32_t       b_hw_ticks;
   PSP_TICK_STRUCT        r;

   r.TICKS[0] = a_ptr->TICKS[0] - b_ptr->TICKS[0];

   a_hw_ticks  = a_ptr->HW_TICKS[0];
   b_hw_ticks  = b_ptr->HW_TICKS[0];

   if ( a_hw_ticks < b_hw_ticks)
   {
      a_hw_ticks += _mqx_kernel_data->HW_TICKS_PER_TICK;
      r.TICKS[0]--;
   } /* Endif */

   /* exchange sign bits between the 32bit halves of 64bit signed difference */
#if PSP_ENDIAN == MQX_LITTLE_ENDIAN
   b_hw_ticks = ((uint32_t *)r.TICKS)[1];
#else
   b_hw_ticks = ((uint32_t *)r.TICKS)[0];
#endif
   a_hw_ticks = (b_hw_ticks & (MAX_UINT_32 ^ MAX_INT_32)) | (((uint32_t)(r.TICKS[0])) & MAX_INT_32);
   b_hw_ticks = (((uint32_t)(r.TICKS[0])) & (MAX_UINT_32 ^ MAX_INT_32)) | (b_hw_ticks & MAX_INT_32);
   if (o_ptr != NULL) *o_ptr = FALSE;

   /* a_hw_ticks contains 32bit signed result, b_hw_ticks must contain all zeroes or all ones according to sign */
   if ((a_hw_ticks >> 31) * MAX_UINT_32 != b_hw_ticks)
   {
      a_hw_ticks = MAX_INT_32 + (a_hw_ticks >> 31);
      if (o_ptr != NULL) *o_ptr = TRUE;
   }

   return (int32_t)a_hw_ticks;

} /* Endbody */
#endif

/* EOF */
