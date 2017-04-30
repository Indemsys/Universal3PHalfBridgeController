
/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
* Copyright 2004-2008 Embedded Access Inc.
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
*   Header file for serial version of Task Aware Debugging.
*
*
*END************************************************************************/
#ifndef _tad_h_
#define _tad_h_

/*----------------------------------------------------------------------*/
/*
 *                    FUNCTION PROTOTYPES
 */

#ifdef __cplusplus
extern "C" {
#endif

extern void _tad_lightweight_memory_blocks (void);
extern void _tad_stack_usage (void);

#ifdef __cplusplus
}
#endif

#endif /*_tad_h_ */
