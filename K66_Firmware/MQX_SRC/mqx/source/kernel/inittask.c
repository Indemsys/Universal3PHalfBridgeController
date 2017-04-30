
/*HEADER**********************************************************************
*
* Copyright 2013 Freescale Semiconductor, Inc.
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
*   This file contains the init task. 
*   
*
*END************************************************************************/

#include "mqx_inc.h"

#if !MQX_USE_IO_OLD
#include "nio.h"
#endif

_WEAK_FUNCTION(int _bsp_post_init(void)) {
    /* This function is called with disabled preemption!!!
    ** Any task blocking function can switch to another task and that can cause that the init
    ** task will not be finished (destroyed) and its resources (stack, ...) will remain allocated.
    */
    return 0;
}

/** Default Init task, when BSP do not override it.
 *
 * Init task is MQX internal task which runs at begining, inmediately after
 * scheduler starts. When it finishes, release used memory and newer run again.
 *
 * \param[in] parameter Parameter passed to the task when created.
 */
void _mqx_init_task(uint32_t parameter) {
    KERNEL_DATA_STRUCT_PTR kernel_data;
    TASK_TEMPLATE_STRUCT_PTR template_ptr;
    TD_STRUCT_PTR td_ptr;

    // initialize IO subsystem
#if !MQX_USE_IO_OLD
    _nio_init(20);
#endif

    // call install function - install BSP specific drivers (this function is implemented in BSP)
    _bsp_init();

    _task_stop_preemption();

    // set all task with flag MQX_AUTO_START_TASK to ready state
    _GET_KERNEL_DATA(kernel_data);
    _lwsem_wait((LWSEM_STRUCT_PTR)&kernel_data->TASK_CREATE_LWSEM);

    // check here for auto-create tasks, and create them here
    template_ptr = kernel_data->INIT.TASK_TEMPLATE_LIST;
    while (template_ptr->TASK_TEMPLATE_INDEX) {
        if (template_ptr->TASK_ATTRIBUTES & MQX_AUTO_START_TASK) {
            td_ptr = _task_init_internal(template_ptr, kernel_data->ACTIVE_PTR->TASK_ID, template_ptr->CREATION_PARAMETER, FALSE, NULL, 0);
#if MQX_CHECK_MEMORY_ALLOCATION_ERRORS
            if (td_ptr == NULL) {
                _mqx_exit(MQX_OUT_OF_MEMORY);
            }
#endif
            _task_ready_internal(td_ptr);
        }
        ++template_ptr;
    }

    _lwsem_post((LWSEM_STRUCT_PTR)&kernel_data->TASK_CREATE_LWSEM);

    _bsp_post_init();
}
