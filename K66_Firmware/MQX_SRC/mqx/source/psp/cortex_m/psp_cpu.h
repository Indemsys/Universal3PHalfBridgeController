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
*   for including processor specific information
*
*
*END************************************************************************/

#ifndef __psp_cpu_h__
    #define __psp_cpu_h__

#include "psp_cpudef.h"


/*
** The main requirement is to define target processor
*/
#ifndef MQX_CPU
    #error  You must define target processor in "user_config.h" (MQX_CPU)
#endif

#if PSP_MQX_CPU_IS_KINETIS
    #include <kinetis.h>
#elif PSP_MQX_CPU_IS_VYBRID
    #include <vybrid.h>
#else
    #error INCORRECT MQX_CPU SETTING
#endif

#include <cortex.h>

/* Needed for smarter _PSP_SET_CACR macro & backward compatibility */
#ifndef CACR_AUTO_CLEAR_BITS
    #define CACR_AUTO_CLEAR_BITS    0
#endif

#ifndef PSP_CACHE_SPLIT
    #define PSP_CACHE_SPLIT         0
#endif

#endif /* __psp_cpu_h__ */
