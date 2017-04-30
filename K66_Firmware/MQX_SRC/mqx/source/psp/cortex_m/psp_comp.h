/*HEADER**********************************************************************
*
* Copyright 2010 Freescale Semiconductor, Inc.
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
*   This file determines which compiler is running, then includes
*   the compiler specific header file
*
*
*END************************************************************************/

#ifndef __psp_comp_h__
#define __psp_comp_h__

/* Include compiler specific header file */
#ifndef __ASM__
#include "psp_cpudef.h"
#include "comp.h"

#define PSP_TASK_PARAM(stack_start_ptr) (stack_start_ptr->PARAMETER)

#endif /* __ASM__ */

#endif /* __psp_comp_h__ */
/* EOF */
