/*HEADER**********************************************************************
*
* Copyright 2011 Freescale Semiconductor, Inc.
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
*   This file contains Low Power Manager initialization information.
*
*
*END************************************************************************/


#include "mqx.h"
#include "bsp.h"


#if MQX_ENABLE_LOW_POWER


#ifndef PE_LDD_VERSION


const LPM_CPU_OPERATION_MODE LPM_CPU_OPERATION_MODES[LPM_OPERATION_MODES] = 
{
    // LPM_OPERATION_MODE_RUN
    {
        LPM_CPU_POWER_MODE_RUN,                     // Index of predefined mode
        0,                                          // Additional mode flags
        0,                                          // Mode wake up events from pins 0..3
        0,                                          // Mode wake up events from pins 4..7
        0,                                          // Mode wake up events from pins 8..11
        0,                                          // Mode wake up events from pins 12..15
        0                                           // Mode wake up events from internal input sources
    },
    // LPM_OPERATION_MODE_WAIT
    {
        LPM_CPU_POWER_MODE_VLPR,                    // Index of predefined mode
        0,                                          // Additional mode flags
        0,                                          // Mode wake up events from pins 0..3
        0,                                          // Mode wake up events from pins 4..7
        0,                                          // Mode wake up events from pins 8..11
        0,                                          // Mode wake up events from pins 12..15
        0                                           // Mode wake up events from internal input sources
    },
    // LPM_OPERATION_MODE_SLEEP
    {
        LPM_CPU_POWER_MODE_WAIT,                    // Index of predefined mode
        LPM_CPU_POWER_MODE_FLAG_SLEEP_ON_EXIT,      // Additional mode flags
        0,                                          // Mode wake up events from pins 0..3
        0,                                          // Mode wake up events from pins 4..7
        0,                                          // Mode wake up events from pins 8..11
        0,                                          // Mode wake up events from pins 12..15
        0                                           // Mode wake up events from internal input sources
    },
    // LPM_OPERATION_MODE_STOP
    {
        LPM_CPU_POWER_MODE_LLS,                     // Index of predefined mode
        0,                                          // Additional mode flags
        0,                                          // Mode wake up events from pins 0..3
        0,                                          // Mode wake up events from pins 4..7
        0,                                          // Mode wake up events from pins 8..11
        0,                                          // Mode wake up events from pins 12..15
        LLWU_ME_WUME0_MASK                          // Mode wake up events from internal input sources - LPT
    },
    // LPM_OPERATION_MODE_HSRUN
    {
        LPM_CPU_POWER_MODE_HSRUN,                     // Index of predefined mode
        0,                                          // Additional mode flags
        0,                                          // Mode wake up events from pins 0..3
        0,                                          // Mode wake up events from pins 4..7
        0,                                          // Mode wake up events from pins 8..11
        0,                                          // Mode wake up events from pins 12..15
        0                                           // Mode wake up events from internal input sources - LPT
    }
};


#else


const LPM_CPU_OPERATION_MODE LPM_CPU_OPERATION_MODES[1] = {0};


#endif


#endif


/* EOF */
