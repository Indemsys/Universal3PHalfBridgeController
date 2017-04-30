#ifndef FSL_FlexCAN_INT_h
#define FSL_FlexCAN_INT_h 1
/*HEADER**********************************************************************
*
* Copyright 2008-2014 Freescale Semiconductor, Inc.
* Copyright 2004-2008 Embedded Access Inc.
* Copyright 1989-2008 ARC International
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
*   This include file is used to provide constant and structure definitions
*   specific to the FlexCAN Serial Communications Controller
*   Revision History:
*   Apr 21, 2003   2.50          Initial version
*
*
*END************************************************************************/

#include <mqx.h>
#include <psp.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void   *_bsp_get_flexcan_base_address(uint8_t);
extern uint32_t _bsp_get_flexcan_vector (uint8_t,uint8_t,uint32_t);

extern uint32_t flexcan_int_enable(uint8_t,uint32_t);
extern uint32_t flexcan_int_disable(uint8_t,uint32_t);
extern uint32_t flexcan_install_isr(uint8_t,uint32_t,INT_ISR_FPTR);
extern uint32_t flexcan_uninstall_isr(uint8_t);
#if !(PSP_MQX_CPU_IS_VYBRID)
extern uint32_t flexcan_error_int_enable(uint8_t);
extern uint32_t flexcan_error_int_disable(uint8_t);
extern uint32_t flexcan_install_isr_err_int(uint8_t,INT_ISR_FPTR);
extern uint32_t flexcan_uninstall_isr_err_int(uint8_t);
extern uint32_t flexcan_install_isr_boff_int(uint8_t,INT_ISR_FPTR);
extern uint32_t flexcan_uninstall_isr_boff_int(uint8_t);
extern uint32_t flexcan_install_isr_wake_int(uint8_t,INT_ISR_FPTR);
extern uint32_t flexcan_uninstall_isr_wake_int(uint8_t);
#endif

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
