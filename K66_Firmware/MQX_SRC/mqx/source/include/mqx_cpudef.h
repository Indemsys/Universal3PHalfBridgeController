
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
* See license agreement file for full license terms including other restrictions.
*****************************************************************************
*
* Comments:
*
*
*
*END************************************************************************/

#ifndef __mqx_cpudef_h__
#define __mqx_cpudef_h__

/* generating macros */
#define PSP_CPU_ARCH_MASK           (0x0f)
#define PSP_CPU_ARCH_SHIFT          (12)
#define PSP_CPU_GROUP_MASK          (0xff)
#define PSP_CPU_GROUP_SHIFT         (4)
#define PSP_CPU_VAR_MASK            (0x0f)
#define PSP_CPU_VAR_SHIFT           (0)


#define PSP_CPU_ARCH(arch)          ((arch & PSP_CPU_ARCH_MASK)  << PSP_CPU_ARCH_SHIFT)
#define PSP_CPU_GROUP(grp)          ((grp & PSP_CPU_GROUP_MASK)  << PSP_CPU_GROUP_SHIFT)
#define PSP_CPU_VAR(var)            ((var & PSP_CPU_VAR_MASK)    << PSP_CPU_VAR_SHIFT)

#define PSP_GET_CPU_ARCH(num)       ((num >> PSP_CPU_ARCH_SHIFT)  & PSP_CPU_ARCH_MASK)
#define PSP_GET_CPU_GROUP(num)      ((num >> PSP_CPU_GROUP_SHIFT) & PSP_CPU_GROUP_MASK)
#define PSP_GET_CPU_VAR(num)        ((num >> PSP_CPU_VAR_SHIFT)   & PSP_CPU_VAR_MASK)

#define PSP_CPU_NUM(arch, grp, var) (PSP_CPU_ARCH(arch) | PSP_CPU_GROUP(grp) | PSP_CPU_VAR(var))

/* architecture defines */
#define PSP_CPU_ARCH_COLDFIRE           (1)
#define PSP_CPU_ARCH_ARM_CORTEX_M4      (2)
#define PSP_CPU_ARCH_PPC                (3)
#define PSP_CPU_ARCH_ARM_CORTEX_M0P     (4)
#define PSP_CPU_ARCH_ARM_CORTEX_A5      (5)
#define PSP_CPU_ARCH_ARM_CORTEX_A8      (6)
#define PSP_CPU_ARCH_ARM_OTHER          (10)

#define PSP_MQX_CPU_IS_PPC              ((PSP_GET_CPU_ARCH(MQX_CPU) == PSP_CPU_ARCH_PPC))
#define PSP_MQX_CPU_IS_COLDFIRE         ((PSP_GET_CPU_ARCH(MQX_CPU) == PSP_CPU_ARCH_COLDFIRE))
#define PSP_MQX_CPU_IS_ARM_CORTEX_M0P   ((PSP_GET_CPU_ARCH(MQX_CPU) == PSP_CPU_ARCH_ARM_CORTEX_M0P))
#define PSP_MQX_CPU_IS_ARM_CORTEX_M4    ((PSP_GET_CPU_ARCH(MQX_CPU) == PSP_CPU_ARCH_ARM_CORTEX_M4))
#define PSP_MQX_CPU_IS_ARM_CORTEX_A5    ((PSP_GET_CPU_ARCH(MQX_CPU) == PSP_CPU_ARCH_ARM_CORTEX_A5))

#define PSP_MQX_CPU_IS_ARM              ( PSP_MQX_CPU_IS_ARM_CORTEX_M0P || \
                                          PSP_MQX_CPU_IS_ARM_CORTEX_M4  || \
                                          PSP_MQX_CPU_IS_ARM_CORTEX_A5  || \
                                          PSP_MQX_CPU_IS_ARM_CORTEX_A8  || \
                                          PSP_MQX_CPU_IS_ARM_OTHER)


#endif /* __mqx_cpudef_h__ */
