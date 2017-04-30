
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
*   This file includes the private definitions for the I/O subsystem.
*
*
*END************************************************************************/
#ifndef __io_prv_h__
#define __io_prv_h__

#if MQX_USE_IO_OLD

/*--------------------------------------------------------------------------*/
/*
 *                            CONSTANT DEFINITIONS
 */


/* Flag meanings */

/* Is the stream at EOF? */
#define IO_FLAG_TEXT        (4)
#define IO_FLAG_AT_EOF      (8)

/* Maximum name check length */
#define IO_MAXIMUM_NAME_LENGTH (1024)

/*--------------------------------------------------------------------------*/
/*
 *                            DATATYPE DECLARATIONS
 */

/*
 * FILE DEVICE STRUCTURE
 *
 * This structure is used by the current I/O Subsystem to store
 * state information.
 * Use the same structure as the formatted I/O.
 */
typedef MQX_FILE FILE_DEVICE_STRUCT, * FILE_DEVICE_STRUCT_PTR;

/*--------------------------------------------------------------------------*/
/*
 *                            FUNCTION PROTOTYPES
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif // MQX_USE_IO_OLD

#endif
/* EOF */
