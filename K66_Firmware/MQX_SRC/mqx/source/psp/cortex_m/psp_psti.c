
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
*   This file contains the function for converting picoseconds to a ticks
*
*
*END************************************************************************/

#include "mqx_inc.h"
#if MQX_HAS_TICK

/*!
 * \brief This function converts picoseconds into ticks. Note, there is no way to 
 *  represent MAX_UINT_16 picoseconds in terms of ticks.
 * 
 * \param[in] psecs The number of picoseconds to convert
 * \param[out] tick_ptr Pointer to tick structure where the result will be stored
 */
void _psp_psecs_to_ticks
   (
       /* [IN] The number of picoseconds to convert */
       _mqx_uint           psecs,

       /* [OUT] Pointer to tick structure where the result will be stored */
       PSP_TICK_STRUCT_PTR tick_ptr
   )
{ /* Body */
   PSP_128_BIT_UNION      tmp;
   KERNEL_DATA_STRUCT_PTR kernel_data;

   _GET_KERNEL_DATA(kernel_data);

#if PSP_ENDIAN == MQX_LITTLE_ENDIAN
   tmp.LLW[1] = 0;
   tmp.LLW[0] = (uint64_t)psecs * kernel_data->TICKS_PER_SECOND;
   tick_ptr->TICKS[0] = tmp.LLW[0] / (1000ULL * 1000 * 1000 * 1000);

   /* Calculate the remaining picoticks */
   tmp.LLW[0] %= (1000ULL * 1000 * 1000 * 1000);
#else
   tmp.LLW[0] = 0;
   tmp.LLW[1] = (uint64_t)psecs * kernel_data->TICKS_PER_SECOND;
   tick_ptr->TICKS[0] = tmp.LLW[1] / (1000ULL * 1000 * 1000 * 1000);

   /* Calculate the remaining picoticks */
   tmp.LLW[1] %= (1000ULL * 1000 * 1000 * 1000);
#endif

   /* Convert to hardware ticks */

   _psp_mul_128_by_32(&tmp, kernel_data->HW_TICKS_PER_TICK, &tmp);
   
   _psp_div_128_by_32(&tmp, 1000UL * 1000 * 1000, &tmp);
   _psp_div_128_by_32(&tmp, 1000, &tmp);

#if PSP_ENDIAN == MQX_LITTLE_ENDIAN
   tick_ptr->HW_TICKS[0] = tmp.LW[0];
#else
   tick_ptr->HW_TICKS[0] = tmp.LW[3];
#endif

} /* Endbody */
#endif

/* EOF */
