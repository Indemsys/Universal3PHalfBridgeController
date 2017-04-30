
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
*   watchdog component.
*
*
*END************************************************************************/
#ifndef __watchdog_h__
#define __watchdog_h__ 1

#include <mqx_cnfg.h>
#if (! MQX_USE_SW_WATCHDOGS) && (! defined (MQX_DISABLE_CONFIG_CHECK))
#error WATCHDOG component is currently disabled in MQX kernel. Please set MQX_USE_SW_WATCHDOGS to 1 in user_config.h and recompile kernel.
#endif

/*--------------------------------------------------------------------------*/
/*                        CONSTANT DEFINITIONS                              */

/* Error codes */

#define WATCHDOG_INVALID_ERROR_FUNCTION   (WATCHDOG_ERROR_BASE|0x01)
#define WATCHDOG_INVALID_INTERRUPT_VECTOR (WATCHDOG_ERROR_BASE|0x02)

/*--------------------------------------------------------------------------*/
/*
 *                    TYPEDEFS FOR _CODE_PTR_ FUNCTIONS
 */
typedef void (_CODE_PTR_  WATCHDOG_ERROR_FPTR)(void *);


/*--------------------------------------------------------------------------*/
/*                        DATA STRUCTURE DEFINITIONS                        */

/*
 *  external declarations for the interface procedures
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TAD_COMPILE__
extern _mqx_uint _watchdog_create_component(_mqx_uint, WATCHDOG_ERROR_FPTR);
extern bool   _watchdog_stop(void);
extern bool   _watchdog_start(uint32_t);
extern bool   _watchdog_start_ticks(MQX_TICK_STRUCT_PTR);
extern _mqx_uint _watchdog_test(void **, void **);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __watchdog_h__ */
/* EOF */
