
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
*   semaphore component.
*
*
*END************************************************************************/
#ifndef __sem_h__
#define __sem_h__ 1

#include <mqx_cnfg.h>
#if (! MQX_USE_SEMAPHORES) && (! defined (MQX_DISABLE_CONFIG_CHECK))
#error SEMAPHORE component is currently disabled in MQX kernel. Please set MQX_USE_SEMAPHORES to 1 in user_config.h and recompile kernel.
#endif

/*--------------------------------------------------------------------------*/
/*                        CONSTANT DEFINITIONS                              */

/* ERROR messages */

#define SEM_MULTI_PROCESSOR_NOT_AVAILABLE   (SEM_ERROR_BASE|0x00)
#define SEM_SEMAPHORE_DELETED               (SEM_ERROR_BASE|0x01)
#define SEM_SEMAPHORE_NOT_DELETED           (SEM_ERROR_BASE|0x02)
#define SEM_INVALID_SEMAPHORE_HANDLE        (SEM_ERROR_BASE|0x03)
#define SEM_CANNOT_POST                     (SEM_ERROR_BASE|0x04)
#define SEM_CANNOT_GET_SEMAPHORE            (SEM_ERROR_BASE|0x05)
#define SEM_INVALID_SEMAPHORE_COUNT         (SEM_ERROR_BASE|0x06)
#define SEM_WAIT_TIMEOUT                    (SEM_ERROR_BASE|0x07)
#define SEM_SEMAPHORE_EXISTS                (SEM_ERROR_BASE|0x08)
#define SEM_SEMAPHORE_TABLE_FULL            (SEM_ERROR_BASE|0x09)
#define SEM_SEMAPHORE_NOT_FOUND             (SEM_ERROR_BASE|0x0A)
#define SEM_INVALID_POLICY                  (SEM_ERROR_BASE|0x0B)
#define SEM_INVALID_SEMAPHORE               (SEM_ERROR_BASE|0x0C)
#define SEM_INCORRECT_INITIAL_COUNT         (SEM_ERROR_BASE|0x0D)

/* bit flag parameters for _sem_create */

#define SEM_PRIORITY_QUEUEING               (1)
#define SEM_PRIORITY_INHERITANCE            (2)
#define SEM_STRICT                          (4)

/* Default component creation parameters */
#define SEM_DEFAULT_INITIAL_NUMBER          (8)
#define SEM_DEFAULT_GROW_NUMBER             (8)
#define SEM_DEFAULT_MAXIMUM_NUMBER          (0) /* Unlimited */

/*--------------------------------------------------------------------------*/
/*                           EXTERNAL DECLARATIONS                          */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TAD_COMPILE__
extern _mqx_uint _sem_close(void *);
extern _mqx_uint _sem_create(char *, _mqx_uint, _mqx_uint);
extern _mqx_uint _sem_create_fast(_mqx_uint, _mqx_uint, _mqx_uint);
extern _mqx_uint _sem_create_component(_mqx_uint, _mqx_uint, _mqx_uint);
extern _mqx_uint _sem_destroy(char *, bool);
extern _mqx_uint _sem_destroy_fast(_mqx_uint, bool);
extern _mqx_uint _sem_get_wait_count(void *);
extern _mqx_uint _sem_open(char *, void **);
extern _mqx_uint _sem_open_fast(_mqx_uint, void **);
extern _mqx_uint _sem_post(void *);
extern _mqx_uint _sem_test(void **);
extern _mqx_uint _sem_get_value(void *);
extern _mqx_uint _sem_wait(void *, uint32_t);
extern _mqx_uint _sem_wait_for(void *, MQX_TICK_STRUCT_PTR);
extern _mqx_uint _sem_wait_ticks(void *, _mqx_uint);
extern _mqx_uint _sem_wait_until(void *, MQX_TICK_STRUCT_PTR);
#endif

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
