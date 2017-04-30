
/*HEADER**********************************************************************
*
* Copyright 2010-2013 Freescale Semiconductor, Inc.
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

*
*END************************************************************************/

#include "asm_mac.h"
#include "mqx_cnfg.h"
#include "types.inc"
#include "psp_prv.inc"

#define __ASM__

#include "psp_cpu.h"

#undef __ASM__

    ASM_COMP_SPECIFIC_DIRECTIVES
    ASM_CODE_SECTION(.text)
    SET_FUNCTION_ALIGNMENT

    ASM_PUBLIC(__boot)

/*FUNCTION*-------------------------------------------------------------------

 Function Name    : __boot
 Returned Value   :
 Comments         : startup sequence

 END*-----------------------------------------------------------------------*/

ASM_EQUATE(NVIC_ICER0, 0xE000E180)
ASM_EQUATE(NVIC_ICPR0, 0xE000E280)


    ASM_PUBLIC_BEGIN(__boot)
    ASM_PUBLIC_FUNC(__boot)
ASM_LABEL(__boot)

#if MQX_AUX_CORE
        msr MSP, r0
        isb #15
#endif

        /* Disable interrupts and clear pending flags */
        ldr r0, =NVIC_ICER0
        ldr r1, =NVIC_ICPR0
        ldr r2, =0xFFFFFFFF
        mov r3, #8

ASM_LABEL(_boot_loop)
        cbz r3, _boot_loop_end
        str r2, [r0], #4        /* NVIC_ICERx - clear enable IRQ register */
        str r2, [r1], #4        /* NVIC_ICPRx - clear pending IRQ register */
        sub r3, r3, #1
        b _boot_loop

ASM_LABEL(_boot_loop_end)

        /* Prepare process stack pointer */
        mrs r0, MSP
        msr PSP, r0

        /* Switch to proccess stack (PSP) */
        mrs r0, CONTROL
        orr r0, r0, #2
        msr CONTROL, r0
        isb #15

#if MQXCFG_ENABLE_FP && PSP_HAS_FPU
        /* CPACR is located at address 0xE000ED88 */
        LDR.W   R0, =0xE000ED88
        /* Read CPACR */
        LDR     R1, [R0]
        /* Set bits 20-23 to enable CP10 and CP11 coprocessors */
        ORR     R1, R1, #(0xF << 20)
        /* Write back the modified value to the CPACR */
        STR     R1, [R0]
        /* turn off fpu register stacking in exception entry */

        ldr r0, =0xE000EF34     /* FPCCR */
        mov r1, #0
        str r1, [r0]
#endif

        /* Perform toolchain startup routines */
        ASM_EXTERN(toolchain_startup)
        b ASM_PREFIX(toolchain_startup)

    ASM_PUBLIC_END(__boot)

        ASM_ALIGN(4)
        ASM_END
