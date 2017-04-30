/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
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
*   User configuration for MQX components
*
*
*END************************************************************************/

#ifndef __user_config_h__
#define __user_config_h__



/* mandatory CPU identification */
#define MQX_CPU                 PSP_CPU_MK65F180M

/* MGCT: <generated_code> */
#define BSPCFG_ENABLE_TTYA       0
#define BSPCFG_ENABLE_ITTYA      0
#define BSPCFG_ENABLE_TTYB       0
#define BSPCFG_ENABLE_ITTYB      0
#define BSPCFG_ENABLE_TTYC       0
#define BSPCFG_ENABLE_ITTYC      0
#define BSPCFG_ENABLE_TTYD       0
#define BSPCFG_ENABLE_ITTYD      0
#define BSPCFG_ENABLE_TTYE       0
#define BSPCFG_ENABLE_ITTYE      1
#define BSPCFG_ENABLE_TTYF       0
#define BSPCFG_ENABLE_ITTYF      0
#define BSPCFG_ENABLE_I2C0       0
#define BSPCFG_ENABLE_II2C0      0
#define BSPCFG_ENABLE_I2C1       0
#define BSPCFG_ENABLE_II2C1      0
#define BSPCFG_ENABLE_SPI0       0
#define BSPCFG_ENABLE_SPI1       0
#define BSPCFG_ENABLE_SPI2       0
#define BSPCFG_ENABLE_RTCDEV     1
#define BSPCFG_ENABLE_PCFLASH    0
#define BSPCFG_ENABLE_ADC0       0
#define BSPCFG_ENABLE_ADC1       0
#define BSPCFG_ENABLE_ADC2       0
#define BSPCFG_ENABLE_ADC3       0
#define BSPCFG_ENABLE_FLASHX     0
#define BSPCFG_ENABLE_ESDHC      1
#define BSPCFG_ENABLE_IODEBUG    0
#define BSPCFG_ENABLE_NANDFLASH  0
#define BSPCFG_ENABLE_SAI        0

#define MQX_USE_MEM              1
#define MQX_USE_UNCACHED_MEM     0
#define MQX_USE_LWMEM_ALLOCATOR  1 // Установка в 0 вызывает ошибки выделения памяти при работе приложения

#define BSPCFG_HAS_SRAM_POOL     1
#define BSPCFG_ENET_SRAM_BUF     0

#define MQX_USE_IDLE_TASK               1
#define MQX_ENABLE_LOW_POWER            0
#define MQXCFG_ENABLE_FP                1
#define MQX_INCLUDE_FLOATING_POINT_IO   1

#define RTCSCFG_ENABLE_ICMP      0
#define RTCSCFG_ENABLE_UDP       0
#define RTCSCFG_ENABLE_TCP       0
#define RTCSCFG_ENABLE_STATS     0
#define RTCSCFG_ENABLE_GATEWAYS  0
#define FTPDCFG_USES_MFS         0
#define RTCSCFG_ENABLE_SNMP      0

#define TELNETDCFG_NOWAIT        FALSE
#define BSPCFG_ENABLE_ENET_STATS 0

#define MQX_TASK_DESTRUCTION     1

#define MQX_USE_TIMER            1

/* MGCT: </generated_code> */

#define ENETCFG_SUPPORT_PTP      0

/*
** include common settings
*/
#define MQX_USE_IO_OLD          1

#define MQX_USE_IPC             1

/*
// Убираем все проверки чтобы ускорить процессы RTOS
#define MQX_CHECK_VALIDITY      0
#define MQX_MONITOR_STACK       0
#define MQX_CHECK_ERRORS        0
#define MQX_CHECK_MEMORY_ALLOCATION_ERRORS  0


//
#define MQX_KERNEL_LOGGING      0
#define MQX_USE_INLINE_MACROS   0
*/


/* use the rest of defaults from small-RAM-device profile */
#include "maximum_config.h"

/* and enable verification checks in kernel */
#include "verif_enabled_config.h"

#endif /* __user_config_h__ */
