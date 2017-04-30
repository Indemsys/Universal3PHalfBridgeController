/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
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
*
*   MQX configuration set: for small-RAM devices
*
*
*END************************************************************************/

#ifndef __small_ram_config_h__
#define __small_ram_config_h__

/* Disable heavy weight components */

#ifndef MQX_USE_IPC
#define MQX_USE_IPC                         0
#endif

#ifndef MQX_USE_LOGS         
#define MQX_USE_LOGS                        0
#endif

#ifndef MQX_USE_SEMAPHORES   
#define MQX_USE_SEMAPHORES                  0
#endif

#ifndef MQX_USE_SW_WATCHDOGS 
#define MQX_USE_SW_WATCHDOGS                0
#endif

#ifndef MQX_USE_TIMER        
#define MQX_USE_TIMER                       0
#endif

/* Other configuration */

#ifndef MQX_DEFAULT_TIME_SLICE_IN_TICKS
#define MQX_DEFAULT_TIME_SLICE_IN_TICKS     1
#endif

#ifndef MQX_LWLOG_TIME_STAMP_IN_TICKS   
#define MQX_LWLOG_TIME_STAMP_IN_TICKS       1
#endif

#ifndef MQX_TIMER_USES_TICKS_ONLY       
#define MQX_TIMER_USES_TICKS_ONLY           1
#endif

#ifndef MQX_EXTRA_TASK_STACK_ENABLE     
#define MQX_EXTRA_TASK_STACK_ENABLE         0
#endif

#ifndef MQX_IS_MULTI_PROCESSOR          
#define MQX_IS_MULTI_PROCESSOR              0
#endif

#ifndef MQX_MUTEX_HAS_POLLING           
#define MQX_MUTEX_HAS_POLLING               0
#endif

#ifndef MQX_USE_INLINE_MACROS             
#define MQX_USE_INLINE_MACROS               0
#endif

#ifndef MQX_USE_LWMEM_ALLOCATOR           
#define MQX_USE_LWMEM_ALLOCATOR             1
#endif

#ifndef MQX_ROM_VECTORS
#define MQX_ROM_VECTORS                     1
#endif

#ifndef MQX_USE_IDLE_TASK
#define MQX_USE_IDLE_TASK                   0
#endif

#ifndef MQX_HAS_TIME_SLICE
#define MQX_HAS_TIME_SLICE                  0
#endif


#ifndef MQX_KERNEL_LOGGING
#define MQX_KERNEL_LOGGING                  0
#endif

#ifndef MQX_SPARSE_ISR_TABLE
#define MQX_SPARSE_ISR_TABLE                1

#ifndef MQX_SPARSE_ISR_SHIFT
#define MQX_SPARSE_ISR_SHIFT                3
#endif

#endif /* MQX_SPARSE_ISR_TABLE */

#ifndef MQX_USE_IO_OLD
#define MQX_USE_IO_OLD  1
#endif

#ifndef MQX_EXIT_ENABLED
#define MQX_EXIT_ENABLED                    0
#endif

/*
** MFS settings
*/

#ifndef MFSCFG_MINIMUM_FOOTPRINT
#define MFSCFG_MINIMUM_FOOTPRINT            1
#endif

#endif
/* EOF */
