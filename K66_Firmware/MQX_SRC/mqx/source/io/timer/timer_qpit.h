/*HEADER**********************************************************************
*
* Copyright 2008-2009 Freescale Semiconductor, Inc.
* Copyright 2004-2010 Embedded Access Inc.
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
*    This file contains definitions for the Qoriwa PIT Driver
*
*
*END************************************************************************/

#ifndef __timer_qpit_h__
#define __timer_qpit_h__

#include "qpit.h"

#ifdef __cplusplus
extern "C" {
#endif


extern VQPIT_REG_STRUCT_PTR _bsp_get_qpit_base_address(uint32_t device);
extern PSP_INTERRUPT_TABLE_INDEX _bsp_get_qpit_vector(uint32_t  device, uint32_t channel);
extern bool _bsp_qpit_enable_access(uint32_t device);
extern void _bsp_qpit_clk_en (uint32_t timer);

extern uint32_t _qpit_timer_install(uint32_t device, uint32_t channel, uint32_t tickfreq, uint32_t clk, uint32_t priority, void (_CODE_PTR_ isr_ptr)(void *));
extern uint32_t _qpit_timer_install_kernel(uint32_t device, uint32_t channel, uint32_t tickfreq, uint32_t clk, uint32_t priority);
extern uint32_t _qpit_timer_stop(uint32_t device, uint32_t channel);


uint32_t _qpit_init_freq(uint32_t, uint32_t, uint32_t, uint32_t, bool);
void _qpit_clear_int(uint32_t, uint32_t);
void _qpit_unmask_int(uint32_t, uint32_t);
void _qpit_mask_int(uint32_t, uint32_t);

#ifdef __cplusplus
}
#endif


#endif
