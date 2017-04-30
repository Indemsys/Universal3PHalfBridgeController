
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
*   This include file is used to define constants and data types private
*   to the event component.
*
*
*END************************************************************************/

#ifndef __lwevent_prv_h__
#define __lwevent_prv_h__ 1

/*--------------------------------------------------------------------------*/
/*                        CONSTANT DEFINITIONS                              */

/* Used to mark a block of memory as belonging to an event group */
#define LWEVENT_VALID                  ((_mqx_uint)(0x6C65766E))  /* "levn" */

/*--------------------------------------------------------------------------*/
/*                         ANSI C PROTOTYPES                                */
#ifdef __cplusplus
extern "C" {
#endif

_mqx_uint _lwevent_create_internal  (LWEVENT_STRUCT_PTR, _mqx_uint, bool);
_mqx_uint _lwevent_destroy_internal (LWEVENT_STRUCT_PTR, bool);
_mqx_uint _lwevent_wait_internal    (LWEVENT_STRUCT_PTR, _mqx_uint, bool, MQX_TICK_STRUCT_PTR, bool);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
