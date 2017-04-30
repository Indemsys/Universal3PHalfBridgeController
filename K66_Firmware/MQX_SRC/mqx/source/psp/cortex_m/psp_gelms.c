
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
*   This file contains the functions for obtaining the elapsed milliseconds
*
*
*END************************************************************************/

#include "mqx_inc.h"


/*!
 * \brief This function obtains current elapsed milliseconds
 *
 * \return uint32_t - number of milliseconds
 */
uint32_t _psp_get_elapsed_milliseconds
   (
      void
   )
{ /* Body */
   KERNEL_DATA_STRUCT_PTR kernel_data;
   uint64_t                ticks;
   uint32_t                ms_per_tick;
   uint32_t                ms;

   _GET_KERNEL_DATA(kernel_data);

   ticks = ((PSP_TICK_STRUCT_PTR)&kernel_data->TIME)->TICKS[0];
   ms_per_tick = 1000 / kernel_data->TICKS_PER_SECOND;
   if ((ms_per_tick * kernel_data->TICKS_PER_SECOND) == 1000) {
      ms = (uint32_t)(ticks * ms_per_tick);
   } else {
      ms = (uint32_t)((ticks * 1000) / kernel_data->TICKS_PER_SECOND);
   } /* Endif */
   return(ms);
   
} /* Endbody */

/* EOF */
