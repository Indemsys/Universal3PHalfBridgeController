
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
*   This file contains the function for converting minutes to ticks
*
*
*END************************************************************************/

#include "mqx_inc.h"
#if MQX_HAS_TICK

/*!
 * \brief This function converts minutes into ticks
 * 
 * \param[in] minutes The number of minutes to convert
 * \param[out] tick_ptr Pointer to tick structure where the result will be stored
 */
void _psp_minutes_to_ticks
   (
       /* [IN] The number of minutes to convert */
       _mqx_uint           minutes,

       /* [OUT] Pointer to tick structure where the result will be stored */
       PSP_TICK_STRUCT_PTR tick_ptr
   )
{ /* body */
   KERNEL_DATA_STRUCT_PTR kernel_data;

   _GET_KERNEL_DATA(kernel_data);

   tick_ptr->HW_TICKS[0] = 0;
   tick_ptr->TICKS[0]    = (uint64_t)minutes * kernel_data->TICKS_PER_SECOND 
       * 60;
      
} /* Endbody */
#endif

/* EOF */
