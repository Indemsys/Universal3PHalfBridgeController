
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
 * \brief Normalizes ticks and partial ticks in a tick structure
 * 
 * \param[in,out] tick_ptr Tick structure to be normalized
 */
void _psp_normalize_ticks
   (
       /* [IN/OUT] Tick structure to be normalized */
       PSP_TICK_STRUCT_PTR tick_ptr
   )
{ /* Body */
   KERNEL_DATA_STRUCT_PTR  kernel_data;
   register uint32_t        ticks_per_tick;

   _GET_KERNEL_DATA(kernel_data);

   ticks_per_tick = kernel_data->HW_TICKS_PER_TICK;

   if (tick_ptr->HW_TICKS[0] >= ticks_per_tick) {
      register uint32_t t = tick_ptr->HW_TICKS[0] / ticks_per_tick;
      tick_ptr->TICKS[0] += t;
      tick_ptr->HW_TICKS[0] -= t * ticks_per_tick;
   } /* Endif */

} /* Endbody */
#endif

/* EOF */
