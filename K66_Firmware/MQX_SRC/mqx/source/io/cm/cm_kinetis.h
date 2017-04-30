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
*   Clock manager Kinetis specific definitions.
*
*
*END************************************************************************/

#ifndef __cm_kinetis_h__
    #define __cm_kinetis_h__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PE_LDD_VERSION

typedef struct  {
    uint32_t cpu_core_clk_hz;            /* Core clock frequency in clock configuration */
    uint32_t cpu_bus_clk_hz;             /* Bus clock frequency in clock configuration */
#ifdef BSP_FLEXBUS_CLOCK                 /* Some platforms do not support Flexbus */
    uint32_t cpu_flexbus_clk_hz;         /* Flexbus clock frequency in clock configuration */
#endif
    uint32_t cpu_flash_clk_hz;           /* FLASH clock frequency in clock configuration */
    uint32_t cpu_usb_clk_hz;             /* USB clock frequency in clock configuration */
    uint32_t cpu_pll_fll_clk_hz;         /* PLL/FLL clock frequency in clock configuration */
    uint32_t cpu_mcgir_clk_hz;           /* MCG internal reference clock frequency in clock configuration */
    uint32_t cpu_oscer_clk_hz;           /* System OSC external reference clock frequency in clock configuration */
    uint32_t cpu_erclk32k_clk_hz;        /* External reference clock 32k frequency in clock configuration */
    uint32_t cpu_mcgff_clk_hz;           /* MCG fixed frequency clock */
} TCpuClockConfiguration;

#endif  /* PE_LDD_VERSION */

typedef enum  {
    CM_CLOCK_SOURCE_CORE = 0,
    CM_CLOCK_SOURCE_BUS,
#ifdef BSP_FLEXBUS_CLOCK                 /* Some platforms do not support Flexbus */
    CM_CLOCK_SOURCE_FLEXBUS,
#endif
    CM_CLOCK_SOURCE_FLASH,
    CM_CLOCK_SOURCE_USB,
    CM_CLOCK_SOURCE_PLLFLL,
    CM_CLOCK_SOURCE_MCGIR,
    CM_CLOCK_SOURCE_OSCER,
    CM_CLOCK_SOURCE_ERCLK32K,
    CM_CLOCK_SOURCE_MCGFF,
    CM_CLOCK_SOURCE_LPO,
    CM_CLOCK_SOURCES,            /* Number of clock sources available */
    CM_CLOCK_SOURCE_SYSTEM = CM_CLOCK_SOURCE_CORE
} CM_CLOCK_SOURCE;

#ifdef __cplusplus
}
#endif


#endif /* __cm_kinetis_h__ */
