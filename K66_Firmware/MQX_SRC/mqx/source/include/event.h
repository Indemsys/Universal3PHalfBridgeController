
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
*   event component.
*
*
*END************************************************************************/
#ifndef __event_h__
#define __event_h__ 1

#include <mqx_cnfg.h>
#if (! MQX_USE_EVENTS) && (! defined (MQX_DISABLE_CONFIG_CHECK))
#error EVENT component is currently disabled in MQX kernel. Please set MQX_USE_EVENTS to 1 in user_config.h and recompile kernel.
#endif

/*--------------------------------------------------------------------------*/
/*                        CONSTANT DEFINITIONS                              */

/* ERROR messages */

#define EVENT_MULTI_PROCESSOR_NOT_AVAILABLE     (EVENT_ERROR_BASE|0x00)
#define EVENT_DELETED                           (EVENT_ERROR_BASE|0x01)
#define EVENT_NOT_DELETED                       (EVENT_ERROR_BASE|0x02)
#define EVENT_INVALID_EVENT_HANDLE              (EVENT_ERROR_BASE|0x03)
#define EVENT_CANNOT_SET                        (EVENT_ERROR_BASE|0x04)
#define EVENT_CANNOT_GET_EVENT                  (EVENT_ERROR_BASE|0x05)
#define EVENT_INVALID_EVENT_COUNT               (EVENT_ERROR_BASE|0x06)
#define EVENT_WAIT_TIMEOUT                      (EVENT_ERROR_BASE|0x07)
#define EVENT_EXISTS                            (EVENT_ERROR_BASE|0x08)
#define EVENT_TABLE_FULL                        (EVENT_ERROR_BASE|0x09)
#define EVENT_NOT_FOUND                         (EVENT_ERROR_BASE|0x0A)
#define EVENT_INVALID_EVENT                     (EVENT_ERROR_BASE|0x0B)
#define EVENT_CANNOT_WAIT_ON_REMOTE_EVENT       (EVENT_ERROR_BASE|0x0C)

/* Default component creation parameters */
#define EVENT_DEFAULT_INITIAL_NUMBER            (8)
#define EVENT_DEFAULT_GROW_NUMBER               (8)
#define EVENT_DEFAULT_MAXIMUM_NUMBER            (0) /* Unlimited */

/*--------------------------------------------------------------------------*/
/*                           EXTERNAL DECLARATIONS                          */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TAD_COMPILE__
extern _mqx_uint _event_clear(void *, _mqx_uint);
extern _mqx_uint _event_close(void *);
extern _mqx_uint _event_create_component(_mqx_uint, _mqx_uint, _mqx_uint);
extern _mqx_uint _event_create_fast(_mqx_uint);
extern _mqx_uint _event_create_fast_auto_clear(_mqx_uint);
extern _mqx_uint _event_create(char *);
extern _mqx_uint _event_create_auto_clear(char *);
extern _mqx_uint _event_destroy(char *);
extern _mqx_uint _event_destroy_fast(_mqx_uint);
extern _mqx_uint _event_get_value(void *, _mqx_uint_ptr);
extern _mqx_uint _event_get_wait_count(void *);
extern _mqx_uint _event_open(char *, void **);
extern _mqx_uint _event_open_fast(_mqx_uint, void **);
extern _mqx_uint _event_set(void *, _mqx_uint);
extern _mqx_uint _event_test(void **);
extern _mqx_uint _event_wait_all(void *, _mqx_uint, uint32_t);
extern _mqx_uint _event_wait_all_for(void *, _mqx_uint, MQX_TICK_STRUCT_PTR);
extern _mqx_uint _event_wait_all_ticks(void *, _mqx_uint, _mqx_uint);
extern _mqx_uint _event_wait_all_until(void *, _mqx_uint, MQX_TICK_STRUCT_PTR);
extern _mqx_uint _event_wait_any(void *, _mqx_uint, uint32_t);
extern _mqx_uint _event_wait_any_for(void *, _mqx_uint, MQX_TICK_STRUCT_PTR);
extern _mqx_uint _event_wait_any_ticks(void *, _mqx_uint, _mqx_uint);
extern _mqx_uint _event_wait_any_until(void *, _mqx_uint, MQX_TICK_STRUCT_PTR);
#endif

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
