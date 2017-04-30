
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
*   name component.
*
*
*END************************************************************************/
#ifndef __name_h__
#define __name_h__ 1

#include <mqx_cnfg.h>
#if (! MQX_USE_NAME) && (! defined (MQX_DISABLE_CONFIG_CHECK))
#error NAME component is currently disabled in MQX kernel. Please set MQX_USE_NAME to 1 in user_config.h and recompile kernel.
#endif

/*--------------------------------------------------------------------------*/
/*                        CONSTANT DEFINITIONS                              */

/* Error codes */

#define NAME_TABLE_FULL             (NAME_ERROR_BASE|0x00)
#define NAME_EXISTS                 (NAME_ERROR_BASE|0x01)
#define NAME_NOT_FOUND              (NAME_ERROR_BASE|0x02)
#define NAME_TOO_LONG               (NAME_ERROR_BASE|0x03)
#define NAME_TOO_SHORT              (NAME_ERROR_BASE|0x04)

/* The maximum name size for a name component name */
#define NAME_MAX_NAME_SIZE          (32)

/* Default component creation parameters */
#define NAME_DEFAULT_INITIAL_NUMBER (8)
#define NAME_DEFAULT_GROW_NUMBER    (8)
#define NAME_DEFAULT_MAXIMUM_NUMBER (0) /* Unlimited */

/*--------------------------------------------------------------------------*/
/*                           EXTERNAL DECLARATIONS                          */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TAD_COMPILE__
extern _mqx_uint _name_create_component(_mqx_uint, _mqx_uint, _mqx_uint);
extern _mqx_uint _name_add(char *, _mqx_max_type);
extern _mqx_uint _name_delete(char *);
extern _mqx_uint _name_find(char *, _mqx_max_type_ptr);
extern _mqx_uint _name_find_by_number(_mqx_max_type, char *);
extern _mqx_uint _name_test(void **, void **);
#endif

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
