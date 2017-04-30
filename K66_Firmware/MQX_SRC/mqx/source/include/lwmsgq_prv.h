
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
*   This file contains private definitions for use with
*   light weight message queues
*
*
*END************************************************************************/

#ifndef __lwmsgq_prv_h__
#define __lwmsgq_prv_h__ 1

/*--------------------------------------------------------------------------*/
/*
 *                            MACRO DEFINITIONS
 */

#define LWMSGQ_VALID        (_mqx_uint)(0x6C776D73) /* "lwms" */

#define LWMSGQ_READ_BLOCKED  (0x30 | IS_BLOCKED | TD_IS_ON_AUX_QUEUE)
#define LWMSGQ_WRITE_BLOCKED (0x32 | IS_BLOCKED | TD_IS_ON_AUX_QUEUE)

/* special task info to inform that the lwmsgq was deinited */
#define LWMSGQ_DEINIT_INFO  (_mqx_uint)-1

/*--------------------------------------------------------------------------*/
/*
 * FUNCTION PROTOTYPES
 */

#ifdef __cplusplus
extern "C" {
#endif

_mqx_uint _lwmsgq_init_internal(void *, _mqx_uint, _mqx_uint, bool);

#ifdef __cplusplus
}
#endif

#endif /* __lwmsgq_prv_h__ */
/* EOF */
