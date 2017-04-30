
/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
* Copyright 2004-2008 Embedded Access Inc.
* Copyright 1989-2008 ARC International
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
*   This file contains the idle task.
*
*
*END************************************************************************/

#include "mqx_inc.h"

#if MQX_USE_IDLE_TASK

/*!
 * \brief This function is the code for the idle task.
 *
 * Idle Task is a MQX task that runs when all application tasks are blocked.
 * \n This function implements a simple counter.
 * \n Counter can be read from a debugger and idle CPU time can be calculated.
 *
 * \param[in] parameter Parameter passed to the task when created.
 */
void _mqx_idle_task
(
 uint32_t parameter
)
{ /* Body */
  volatile KERNEL_DATA_STRUCT_PTR kernel_data;

  _GET_KERNEL_DATA(kernel_data);

  while (1)
  {
  #if !defined(MQX_ENABLE_IDLE_LOOP) || MQX_ENABLE_IDLE_LOOP
    if (++kernel_data->IDLE_LOOP.IDLE_LOOP1 == 0)
    {
      if (++kernel_data->IDLE_LOOP.IDLE_LOOP2 == 0)
      {
        if (++kernel_data->IDLE_LOOP.IDLE_LOOP3 == 0)
        {
          ++kernel_data->IDLE_LOOP.IDLE_LOOP4;
        } /* Endif */
      } /* Endif */
    } /* Endif */
  #endif
  #if MQX_ENABLE_LOW_POWER
    if (parameter)
    {
      _ASM_SLEEP(&parameter);
    }
  #endif

  } /* Endwhile */

} /* Endbody */
#endif /* MQX_USE_IDLE_TASK */

/* EOF */

