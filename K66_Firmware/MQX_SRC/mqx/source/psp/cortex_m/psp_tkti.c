
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
*   This file contains the functions for converting between ticks and
*   the TIME_STRUCT format.
*
*
*END************************************************************************/

#include "mqx_inc.h"


/*!
 * \brief This function converts ticks into the time struct format
 * 
 * \param[in] tick_ptr Pointer to the tick struct to store the results in
 * \param[out] time_ptr Pointer to the time struct to convert
 *
 * \return bool - Returns FALSE is overflow occurs
 */
bool _psp_ticks_to_time
   (
      /* [IN] Pointer to the tick struct to store the results in */
      PSP_TICK_STRUCT_PTR tick_ptr,

      /* [OUT] Pointer to the time struct to convert */
      TIME_STRUCT_PTR     time_ptr
   )
{
   uint64_t                tmp;
   uint32_t                tps;
   KERNEL_DATA_STRUCT_PTR kernel_data;

   _GET_KERNEL_DATA(kernel_data);

   tps = kernel_data->TICKS_PER_SECOND;

   /* Saturate if ticks go out of range of time struct */
   if ( (tick_ptr->TICKS[0] / tps) > MAX_UINT_32) {
      time_ptr->SECONDS      = MAX_UINT_32;
      time_ptr->MILLISECONDS = 999;
      return FALSE;
   } /* Endif */

   /* Recompute TICKS to milliseconds, together with HW_TICKS */
   tmp = (tick_ptr->TICKS[0] * 1000) + (tick_ptr->HW_TICKS[0] * 1000 / kernel_data->HW_TICKS_PER_TICK);
   tmp = tmp / tps;
 
   /* Compute seconds and remainder from milliseconds value */
   time_ptr->SECONDS      = tmp / 1000;
   time_ptr->MILLISECONDS = tmp - time_ptr->SECONDS * 1000;

   return TRUE;

}
