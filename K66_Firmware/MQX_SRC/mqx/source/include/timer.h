
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
*   This include file is used to define constants and data types for the
*   timer component.
*
*
*END************************************************************************/
#ifndef __timer_h__
#define __timer_h__ 1

#include <mqx_cnfg.h>
#if (! MQX_USE_TIMER) && (! defined (MQX_DISABLE_CONFIG_CHECK))
#error TIMER component is currently disabled in MQX kernel. Please set MQX_USE_TIMER to 1 in user_config.h and recompile kernel.
#endif

/*--------------------------------------------------------------------------*/
/*                        CONSTANT DEFINITIONS                              */

/* 
 * This mode tells the timer to use the elapsed time when calculating time 
 * (_time_get_elapsed)
 */
#define TIMER_ELAPSED_TIME_MODE       (1)

/* 
 * This mode tells the timer to use the actual time when calculating time
 * (_time_get)  Note that the time returned by _time_get can be modified
 * by _time_set.
 */
#define TIMER_KERNEL_TIME_MODE        (2)

/* The error return from the timer start functions */
#define TIMER_NULL_ID                 ((_timer_id)0)

/* The default parameters for the timer_create_component function */
#define TIMER_DEFAULT_TASK_PRIORITY   (1)
#define TIMER_DEFAULT_STACK_SIZE      (800)

/*--------------------------------------------------------------------------*/
/*                     DATA STRUCTURE DEFINITIONS                           */

typedef _mqx_uint  _timer_id;

/*--------------------------------------------------------------------------*/
/*
 *                    TYPEDEFS FOR _CODE_PTR_ FUNCTIONS
 */
typedef void (_CODE_PTR_  TIMER_NOTIFICATION_TIME_FPTR)( _timer_id, void *, uint32_t, uint32_t);
typedef void (_CODE_PTR_  TIMER_NOTIFICATION_TICK_FPTR)( _timer_id, void *, MQX_TICK_STRUCT_PTR);

/*--------------------------------------------------------------------------*/
/*                       EXTERNAL DECLARATIONS                              */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TAD_COMPILE__
extern _mqx_uint  _timer_cancel(_timer_id);
extern _mqx_uint  _timer_create_component(_mqx_uint, _mqx_uint);
extern _timer_id  _timer_start_oneshot_after(
   TIMER_NOTIFICATION_TIME_FPTR, void *, _mqx_uint, uint32_t);
extern _timer_id  _timer_start_oneshot_at( 
   TIMER_NOTIFICATION_TIME_FPTR, void *, _mqx_uint, TIME_STRUCT_PTR);        
extern _timer_id  _timer_start_periodic_every( 
   TIMER_NOTIFICATION_TIME_FPTR, void *, _mqx_uint, uint32_t);        
extern _timer_id  _timer_start_periodic_at( 
   TIMER_NOTIFICATION_TIME_FPTR, void *, _mqx_uint, TIME_STRUCT_PTR, uint32_t);
extern _timer_id  _timer_start_oneshot_after_ticks(
   TIMER_NOTIFICATION_TICK_FPTR, void *, _mqx_uint, MQX_TICK_STRUCT_PTR);
extern _timer_id  _timer_start_oneshot_at_ticks( 
   TIMER_NOTIFICATION_TICK_FPTR, void *, _mqx_uint, MQX_TICK_STRUCT_PTR);        
extern _timer_id  _timer_start_periodic_every_ticks( 
   TIMER_NOTIFICATION_TICK_FPTR, void *, _mqx_uint, MQX_TICK_STRUCT_PTR);        
extern _timer_id  _timer_start_periodic_at_ticks( 
   TIMER_NOTIFICATION_TICK_FPTR, void *, _mqx_uint,
   MQX_TICK_STRUCT_PTR, MQX_TICK_STRUCT_PTR);
extern _mqx_uint  _timer_test(void **);
#endif

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
