
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
*   This file contains the function for converting milliseconds to a ticks
*   ignoring the Hardware Ticks
*
*
*END************************************************************************/

#include "mqx_inc.h"

/*!
 * \brief This function converts milliseconds into ticks without HW ticks
 * 
 * \param[in] msecs The number of milliseconds to convert
 * \param[out] tick_ptr Pointer to tick structure where the result will be stored
 */
void _psp_msecs_to_ticks_quick
   (
       /* [IN] The number of milliseconds to convert */
       _mqx_uint           msecs,

       /* [OUT] Pointer to tick structure where the result will be stored */
       PSP_TICK_STRUCT_PTR tick_ptr
   )
{ /* Body */
   static uint32_t         ms_per_tick, heuristic = 0;
   KERNEL_DATA_STRUCT_PTR kernel_data;

   tick_ptr->HW_TICKS[0] = 0;

   if (heuristic == 1) {
      /* Perform fast calculation */
fast: tick_ptr->TICKS[0] = (uint64_t)(msecs / ms_per_tick);
      return;
   }/* Endif */

   _GET_KERNEL_DATA(kernel_data);

   if (heuristic == 2) {
slow: tick_ptr->TICKS[0] =
         ((uint64_t)msecs * kernel_data->TICKS_PER_SECOND) / 1000;
      return;
   }/* Endif */

   ms_per_tick = 1000 / kernel_data->TICKS_PER_SECOND;
   if ((ms_per_tick * kernel_data->TICKS_PER_SECOND) == 1000) {
      heuristic = 1;
      goto fast;
   }/* Endif */

   heuristic = 2;
   goto slow;

} /* Endbody */
/* EOF */
