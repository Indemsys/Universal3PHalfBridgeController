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
*   This file provides a generic header file for use by the mqx kernel
*   for including platform specific information and macros
*
*
*END************************************************************************/

#ifndef __psp_h__
#define __psp_h__

#include <mqx_cnfg.h>

#ifndef __ASM__

#include <psptypes.h>

#include <psp_time.h>
#include <psp_math.h>
#include <psp_comp.h>

#include <mqx_ioc.h>

#endif // __ASM__

#include <psp_cpu.h>



#endif
/* EOF */
