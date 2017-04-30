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
*   This file contains flash boot code to initialize chip selects,
*   disable the watchdog timer and initialize the PLL.
*
*
*END************************************************************************/

#include "mqx.h"
#include "bsp.h"
#include "bsp_prv.h"

extern int Init_MK66FN2M0VLQ18_K66BLEZ1(void);

/*FUNCTION*---------------------------------------------------------------------
*
* Function Name    : _bsp_watchdog_disable
* Returned Value   : void
* Comments         :
*   Disable watchdog timer
*
*END*-------------------------------------------------------------------------*/

void _bsp_watchdog_disable(void)
{
    WDOG_MemMapPtr reg = WDOG_BASE_PTR;

    /* NOTE: DO NOT SINGLE STEP THROUGH THIS FUNCTION!!!
    * There are timing requirements for the execution of the unlock. If
    * you single step through the code you will cause the CPU to reset.
    */

    /* unlock watchdog */
    reg->UNLOCK = 0xc520;
    reg->UNLOCK = 0xd928;

    /* disable watchdog */
    reg->STCTRLH &= ~(WDOG_STCTRLH_WDOGEN_MASK);
}





/*FUNCTION*---------------------------------------------------------------------
*
* Function Name    : init_hardware
* Returned Value   : void
* Comments         :
*   Initialize Kinetis device.
*
*   .........................................................................................................................................
*   Процедура вызывается по цепочке: вектор сброса -> __boot -> toolchain_startup -> __iar_program_start -> __low_level_init -> init_hardware
*   .........................................................................................................................................
*
*END*-------------------------------------------------------------------------*/

void init_hardware(void)
{

#if PE_LDD_VERSION
    /*  Watch Dog disabled by CPU bean (need to setup in CPU Inspector) */
    __pe_initialize_hardware();
#else
    Init_MK66FN2M0VLQ18_K66BLEZ1();
#endif
}
