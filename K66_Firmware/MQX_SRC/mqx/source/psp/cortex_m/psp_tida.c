
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
*   This file contains the functions for converting ticks to days
*
*
*END************************************************************************/

#include "mqx_inc.h"


#if MQX_HAS_TICK

/*!
 * \brief This function converts ticks into days
 * 
 * \param tick_ptr 
 * \param overflow_ptr 
 *
 * \return uint32_t - number of days
 */
uint32_t _psp_ticks_to_days
   (
      PSP_TICK_STRUCT_PTR tick_ptr,
      bool        *overflow_ptr
   )
{ /* Body */
   uint64_t                tmp;
   KERNEL_DATA_STRUCT_PTR kernel_data;

   _GET_KERNEL_DATA(kernel_data);

   tmp = tick_ptr->TICKS[0];

   if ((tmp != MAX_UINT_64) && 
      (tick_ptr->HW_TICKS[0] > (kernel_data->HW_TICKS_PER_TICK/2)))
   {
      tmp++;
   } /* Endif */

   tmp = (tmp / kernel_data->TICKS_PER_SECOND) /
      (SECS_IN_MINUTE * MINUTES_IN_HOUR * HOURS_IN_DAY);

   *overflow_ptr = (bool)(tmp > MAX_UINT_32);

   return (uint32_t)tmp;

} /* Endbody */

#endif

/* EOF */
