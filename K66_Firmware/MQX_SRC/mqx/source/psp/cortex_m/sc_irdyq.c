
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
*   This file contains psp functions for initializing the scheduler.n
*
*
*END************************************************************************/

#include "mqx_inc.h"

/*!
 * \brief This function sets up the kernel disable priority.
 */
void _psp_set_kernel_disable_level
    (
        void
    )
{
    KERNEL_DATA_STRUCT_PTR          kernel_data;
    MQX_INITIALIZATION_STRUCT_PTR   init_ptr;
    uint32_t temp;
    _mqx_int i;

    _GET_KERNEL_DATA(kernel_data);

    init_ptr = (MQX_INITIALIZATION_STRUCT_PTR)&kernel_data->INIT;

    /* Calculate the enable and disable interrupt values for the kernel. */
    temp = init_ptr->MQX_HARDWARE_INTERRUPT_LEVEL_MAX;
    if (temp > 7) {
        temp = 7;
        init_ptr->MQX_HARDWARE_INTERRUPT_LEVEL_MAX = 7;
    } else if (temp == 0) {
        temp = 1;
        init_ptr->MQX_HARDWARE_INTERRUPT_LEVEL_MAX = 1;
    }

    kernel_data->DISABLE_SR = CORTEX_PRIOR(temp);

    /* Set all (till now unused) interrupts level to the disable level */
    for (i = 0; i < sizeof(NVIC_BASE_PTR->IP) / sizeof(NVIC_BASE_PTR->IP[0]); i++)
        NVIC_BASE_PTR->IP[i] = CORTEX_PRIOR((1 << CORTEX_PRIOR_IMPL) - 2);
    /* Disable interrupts by default */
    {
        uint32_t * icer_ptr = (uint32_t *)&NVIC_BASE_PTR->ICER;

        for (i = 0; i < sizeof(NVIC_BASE_PTR->ICER) / sizeof(uint32_t); i++)    {
            /* Disable 32 interrupts in a row */
            *(icer_ptr + i) = 0xFFFFFFFF;
        }
    }
}


/*!
 * \brief This function sets up the kernel priority ready queues
 */
uint32_t _psp_init_readyqs
    (
        void
    )
{ /* Body */
   KERNEL_DATA_STRUCT_PTR kernel_data;
   READY_Q_STRUCT_PTR     q_ptr;
   uint32_t                priority_levels;
   uint32_t                n;

    _GET_KERNEL_DATA(kernel_data);
    kernel_data->READY_Q_LIST = (READY_Q_STRUCT_PTR) NULL;
   priority_levels = kernel_data->LOWEST_TASK_PRIORITY + 3; // IDLE TASK, INIT TASK

   q_ptr = (READY_Q_STRUCT_PTR)_mem_alloc_zero(sizeof(READY_Q_STRUCT) * priority_levels);
#if MQX_CHECK_MEMORY_ALLOCATION_ERRORS
   if ( q_ptr == NULL ) {
      return (MQX_OUT_OF_MEMORY);
   } /* Endif */
#endif
   _mem_set_type(q_ptr, MEM_TYPE_READYQ);

   n = priority_levels;
   while (n--) {
      q_ptr->HEAD_READY_Q  = (TD_STRUCT_PTR)q_ptr;
      q_ptr->TAIL_READY_Q  = (TD_STRUCT_PTR)q_ptr;
      q_ptr->PRIORITY      = (uint16_t)n;

      if (n + kernel_data->INIT.MQX_HARDWARE_INTERRUPT_LEVEL_MAX < (1 << CORTEX_PRIOR_IMPL))
        q_ptr->ENABLE_SR   = CORTEX_PRIOR(n + kernel_data->INIT.MQX_HARDWARE_INTERRUPT_LEVEL_MAX);
      else
        q_ptr->ENABLE_SR   = 0;

        q_ptr->NEXT_Q        = kernel_data->READY_Q_LIST;
        kernel_data->READY_Q_LIST = q_ptr++;
    }


    /*
    ** Set the current ready q (where the ready queue searches start) to
    ** the head of the list of ready queues.
    */
    kernel_data->CURRENT_READY_Q = kernel_data->READY_Q_LIST;

#if 0
    /* Initialize the ENABLE_SR fields in the ready queues */
    sr = 0;
    n  = priority_levels;
    q_ptr =  kernel_data->READY_Q_LIST;
    while (n--) {
        q_ptr->ENABLE_SR = CORTEX_PRIOR(sr);
        if (sr < kernel_data->INIT.MQX_HARDWARE_INTERRUPT_LEVEL_MAX) {
            sr++;
        }
        q_ptr = q_ptr->NEXT_Q;
    }
#endif

    return MQX_OK;

} /* Endbody */

/* EOF */
