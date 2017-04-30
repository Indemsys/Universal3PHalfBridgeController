/*HEADER***********************************************************************
*
* Copyright 2013 Freescale Semiconductor, Inc.
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
*
*END************************************************************************/

#include <asm_mac.h>

#include "mqx_cnfg.h"
#include "types.inc"
#include "psp_prv.inc"

#define __ASM__
#include "psp_cpu.h"
#include "mqx_prv.h"
#undef __ASM__

                ASM_EXTERN(_mqx_kernel_data)
                ASM_CODE_SECTION(.text)

#if PSP_HAS_FPU
/*FUNCTION*******************************************************************
*
* Function Name    : _psp_push_fp_context
* Returned Value   : none
* Comments         : Save FPU and NEON registers to stack
*
*END************************************************************************/
                ASM_PUBLIC(_psp_push_fp_context)
                ASM_PUBLIC_BEGIN(_psp_push_fp_context)
                ASM_PUBLIC_FUNC(_psp_push_fp_context)
ASM_LABEL(_psp_push_fp_context)
#if (defined(__IASMARM__) || defined(__CC_ARM))
                str r1, [sp, #-136]			/* save r1 to possition in stack (1word reg (r1) + 1word reg (FPSCR) + 32word registers) */
#else
                ASM_CONST32(0x1c88f84d)
#endif

                /* save FPU, FPU status registers */
                vmrs r1, FPSCR
                //str r1, [sp, #-4]!                       /* FPSCR */
                push {r1}                       /* FPSCR */

                vstmdb sp!, {s0-s31}                      /* restore fpu registers */

#if (defined(__IASMARM__) || defined(__CC_ARM))
                ldr r1, [sp, #-4]                       /* restore changed r1 */
#else
                ASM_CONST32(0x1c04f85d)
#endif

                bx lr
                ASM_PUBLIC_END(_psp_push_fp_context)

/*FUNCTION*******************************************************************
*
* Function Name    : _psp_pop_fp_context
* Returned Value   : none
* Comments         : Restore FPU and NEON registers from stack
*
*END************************************************************************/
                ASM_PUBLIC(_psp_pop_fp_context)
                ASM_PUBLIC_BEGIN(_psp_pop_fp_context)
                ASM_PUBLIC_FUNC(_psp_pop_fp_context)
ASM_LABEL(_psp_pop_fp_context)
#if (defined(__IASMARM__) || defined(__CC_ARM))
                str r1, [sp, #-4]                       /* save r1 */
#else
                ASM_CONST32(0x1c04f84d)
#endif

                /* restore FPU, FPU status and NEON registers */
                vldm sp!, {s0-s31}                      /* restore fpu registers */

                //ldr r1, [sp], #4                        /* FPSCR */
                pop {r1}                        /* FPSCR */
                vmsr FPSCR, r1

#if (defined(__IASMARM__) || defined(__CC_ARM))
                ldr r1, [sp, #-136]                     /* restore r1 */        
#else
                ASM_CONST32(0x1c88f85d)
#endif

                bx lr
                ASM_PUBLIC_END(_psp_pop_fp_context)

#endif // PSP_HAS_FPU


/*FUNCTION*******************************************************************
*
* Function Name    : tlsf_clz
* Returned Value   : none
* Comments         : Enable the interrupts
*
*END************************************************************************/
                ASM_PUBLIC(tlsf_clz)
                ASM_PUBLIC_BEGIN(tlsf_clz)
                ASM_PUBLIC_FUNC(tlsf_clz)
ASM_LABEL(tlsf_clz)
                clz r0, r0
                bx lr             
                ASM_PUBLIC_END(tlsf_clz)
                ASM_END
