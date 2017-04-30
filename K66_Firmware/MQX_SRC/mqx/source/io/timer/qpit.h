/*HEADER**********************************************************************
*
* Copyright 2008-2009 Freescale Semiconductor, Inc.
* Copyright 2004-2010 Embedded Access Inc.
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
*    This file contains definitions for the Qoriwa PIT registers
*
*
*END************************************************************************/

#ifndef __qpit_h__
#define __qpit_h__


#define QPIT_NUM_TIMERS 4

#define QPIT_MCR_MDIS      (1<<1)
#define QPIT_MCR_FRZ       (1<<0)

#define QPIT_TCTRL_TIE     (1<<1)
#define QPIT_TCTRL_TEN     (1<<0)

#define QPIT_TFLG_TIF      (1<<0)


typedef struct qpit_timer_reg_struct {
   uint32_t  LDVAL;
   uint32_t  CVAL;
   uint32_t  TCTRL;
   uint32_t  TFLG;
} QPIT_TIMER_REG_STRUCT, * QPIT_TIMER_REG_STRUCT_PTR;

typedef volatile struct qpit_timer_reg_struct * VQPIT_TIMER_REG_STRUCT_PTR;

typedef struct qpit_reg_struct {
   uint32_t                 MCR;
   RESERVED_REGISTER(0x0004,0x0100);
   QPIT_TIMER_REG_STRUCT   TIMERS[QPIT_NUM_TIMERS];
   RESERVED_REGISTER(0x0140,0x4000);
} QPIT_REG_STRUCT, * QPIT_REG_STRUCT_PTR;

typedef volatile struct qpit_reg_struct * VQPIT_REG_STRUCT_PTR;


#endif

