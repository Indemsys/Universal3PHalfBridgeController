
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
*   This file contains the functions for converting from time to ticks.
*
*
*END************************************************************************/

#include "mqx_inc.h"

/*!
 * \brief This function converts the time struct format into ticks
 * 
 * \param time_ptr 
 * \param tick_ptr 
 */
void _psp_time_to_ticks
   (
      TIME_STRUCT_PTR     time_ptr,
      PSP_TICK_STRUCT_PTR tick_ptr
   )
{ /* Body */
   PSP_TICK_STRUCT tick1;
   PSP_TICK_STRUCT tick2;

   _psp_seconds_to_ticks(time_ptr->SECONDS, &tick1);
   _psp_msecs_to_ticks(time_ptr->MILLISECONDS, &tick2);

   _psp_add_ticks(&tick1, &tick2, tick_ptr);

} /* Endbody */

/* EOF */
