#ifndef __psp_prv_h__
#define __psp_prv_h__
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
* See license agreement file for full license terms including other
* restrictions.
*****************************************************************************
*
* Comments:
*   This file contains psp private declarations for use when compiling
*   the kernel.
*
*
*END************************************************************************/

#include "mqx_prv.h"

/* This macro modifies the context of a blocked task so that the
** task will execute the provided function when it next runs
*/
#define _PSP_SET_PC_OF_BLOCKED_TASK(stack_ptr, func)    \
    ((PSP_BLOCKED_STACK_STRUCT_PTR)(stack_ptr))->PC = ((uint32_t)(func))

/* This macro modifies the interrupt priority of a blocked task so that the
** task will execute at the correct interrupt priority when it restarts
*/
#define _PSP_SET_SR_OF_BLOCKED_TASK(stack_ptr, sr_value)

/* This macro modifies the context of a task that has been interrupted
** so that the task will execute the provided function when the isr returns
*/
#define _PSP_SET_PC_OF_INTERRUPTED_TASK(stack_ptr, func)    \
    ((PSP_BASIC_INT_FRAME_STRUCT_PTR)(__get_PSP()))->PC = (uint32_t)(func)

/* This macro obtains the address of the kernel data structure */
#define _GET_KERNEL_DATA(x)     x = _mqx_kernel_data

#define _SET_KERNEL_DATA(x)     \
    _mqx_kernel_data = (struct kernel_data_struct *)(x)

#define _PSP_GET_CALLING_PC()   (0)

/*--------------------------------------------------------------------------*/
/*
**                PROTOTYPES OF PRIVATE PSP FUNCTIONS
*/

#ifdef __cplusplus
extern "C" {
#endif

/* ARM PSP specific prototypes */
void     _psp_exception_return(void *);
void     _psp_setup_int_mode_stack(void *, uint32_t, uint32_t);
uint32_t _psp_get_int_mode_stack(uint32_t);
uint32_t _psp_get_int_mode_lr(uint32_t);

bool     _psp_build_float_context(TD_STRUCT_PTR);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
