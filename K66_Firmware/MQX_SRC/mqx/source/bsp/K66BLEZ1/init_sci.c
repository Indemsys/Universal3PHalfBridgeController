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
*   This file contains the definition for the baud rate for the serial
*   channel
*
*
*END************************************************************************/

#include "mqx.h"
#include "bsp.h"



#if MQX_ENABLE_LOW_POWER

const KUART_OPERATION_MODE_STRUCT _bsp_sci0_operation_modes[LPM_OPERATION_MODES] =
{
    /* LPM_OPERATION_MODE_RUN */
    {
        IO_PERIPHERAL_PIN_MUX_ENABLE | IO_PERIPHERAL_CLOCK_ENABLE | IO_PERIPHERAL_MODULE_ENABLE,
        0,
        0,
        0
    },
    /* LPM_OPERATION_MODE_WAIT */
    {
        IO_PERIPHERAL_PIN_MUX_ENABLE | IO_PERIPHERAL_CLOCK_ENABLE | IO_PERIPHERAL_MODULE_ENABLE,
        0,
        0,
        0
    },
    /* LPM_OPERATION_MODE_SLEEP */
    {
        IO_PERIPHERAL_PIN_MUX_ENABLE | IO_PERIPHERAL_CLOCK_ENABLE | IO_PERIPHERAL_MODULE_ENABLE,
        0,
        0,
        0
    },
    /* LPM_OPERATION_MODE_STOP */
    {
        IO_PERIPHERAL_PIN_MUX_DISABLE | IO_PERIPHERAL_CLOCK_DISABLE,
        0,
        0,
        0
    },
    /* LPM_OPERATION_MODE_HSRUN */
    {
        IO_PERIPHERAL_PIN_MUX_ENABLE | IO_PERIPHERAL_CLOCK_ENABLE | IO_PERIPHERAL_MODULE_ENABLE,
        0,
        0,
        0
    }
};

const KUART_OPERATION_MODE_STRUCT _bsp_sci1_operation_modes[LPM_OPERATION_MODES] =
{
    /* LPM_OPERATION_MODE_RUN */
    {
        IO_PERIPHERAL_PIN_MUX_ENABLE | IO_PERIPHERAL_CLOCK_ENABLE | IO_PERIPHERAL_MODULE_ENABLE,
        0,
        0,
        0
    },
    /* LPM_OPERATION_MODE_WAIT */
    {
        IO_PERIPHERAL_PIN_MUX_ENABLE | IO_PERIPHERAL_CLOCK_ENABLE | IO_PERIPHERAL_MODULE_ENABLE,
        0,
        0,
        0
    },
    /* LPM_OPERATION_MODE_SLEEP */
    {
        IO_PERIPHERAL_PIN_MUX_ENABLE | IO_PERIPHERAL_CLOCK_ENABLE | IO_PERIPHERAL_MODULE_ENABLE,
        0,
        0,
        0
    },
    /* LPM_OPERATION_MODE_STOP */
    {
        IO_PERIPHERAL_PIN_MUX_DISABLE | IO_PERIPHERAL_CLOCK_DISABLE,
        0,
        0,
        0
    },
    /* LPM_OPERATION_MODE_HSRUN */
    {
        IO_PERIPHERAL_PIN_MUX_ENABLE | IO_PERIPHERAL_CLOCK_ENABLE | IO_PERIPHERAL_MODULE_ENABLE,
        0,
        0,
        0
    }
};

const KUART_OPERATION_MODE_STRUCT _bsp_sci2_operation_modes[LPM_OPERATION_MODES] =
{
    /* LPM_OPERATION_MODE_RUN */
    {
        IO_PERIPHERAL_PIN_MUX_ENABLE | IO_PERIPHERAL_CLOCK_ENABLE | IO_PERIPHERAL_MODULE_ENABLE,
        0,
        0,
        0
    },

    /* LPM_OPERATION_MODE_WAIT */
    {
        IO_PERIPHERAL_PIN_MUX_ENABLE | IO_PERIPHERAL_CLOCK_ENABLE | IO_PERIPHERAL_MODULE_ENABLE,
        0,
        0,
        0
    },

    /* LPM_OPERATION_MODE_SLEEP */
    {
        IO_PERIPHERAL_PIN_MUX_ENABLE | IO_PERIPHERAL_CLOCK_ENABLE | IO_PERIPHERAL_MODULE_ENABLE | IO_PERIPHERAL_WAKEUP_ENABLE | IO_PERIPHERAL_WAKEUP_SLEEPONEXIT_DISABLE,
        0,
        0,
        0
    },

    /* LPM_OPERATION_MODE_STOP */
    {
        IO_PERIPHERAL_PIN_MUX_DISABLE | IO_PERIPHERAL_CLOCK_DISABLE,
        0,
        0,
        0
    },
    /* LPM_OPERATION_MODE_HSRUN */
    {
        IO_PERIPHERAL_PIN_MUX_ENABLE | IO_PERIPHERAL_CLOCK_ENABLE | IO_PERIPHERAL_MODULE_ENABLE,
        0,
        0,
        0
    }
};

const KUART_OPERATION_MODE_STRUCT _bsp_sci3_operation_modes[LPM_OPERATION_MODES] =
{
    /* LPM_OPERATION_MODE_RUN */
    {
        IO_PERIPHERAL_PIN_MUX_ENABLE | IO_PERIPHERAL_CLOCK_ENABLE | IO_PERIPHERAL_MODULE_ENABLE,
        0,
        0,
        0
    },

    /* LPM_OPERATION_MODE_WAIT */
    {
        IO_PERIPHERAL_PIN_MUX_ENABLE | IO_PERIPHERAL_CLOCK_ENABLE | IO_PERIPHERAL_MODULE_ENABLE,
        0,
        0,
        0
    },

    /* LPM_OPERATION_MODE_SLEEP */
    {
        IO_PERIPHERAL_PIN_MUX_ENABLE | IO_PERIPHERAL_CLOCK_ENABLE | IO_PERIPHERAL_MODULE_ENABLE,
        0,
        0,
        0
    },

    /* LPM_OPERATION_MODE_STOP */
    {
        IO_PERIPHERAL_PIN_MUX_DISABLE | IO_PERIPHERAL_CLOCK_DISABLE,
        0,
        0,
        0
    },
    /* LPM_OPERATION_MODE_HSRUN */
    {
        IO_PERIPHERAL_PIN_MUX_ENABLE | IO_PERIPHERAL_CLOCK_ENABLE | IO_PERIPHERAL_MODULE_ENABLE,
        0,
        0,
        0
    }
};


const KUART_OPERATION_MODE_STRUCT _bsp_sci4_operation_modes[LPM_OPERATION_MODES] =
{
    /* LPM_OPERATION_MODE_RUN */
    {
        IO_PERIPHERAL_PIN_MUX_ENABLE | IO_PERIPHERAL_CLOCK_ENABLE | IO_PERIPHERAL_MODULE_ENABLE,
        0,
        0,
        0
    },

    /* LPM_OPERATION_MODE_WAIT */
    {
        IO_PERIPHERAL_PIN_MUX_ENABLE | IO_PERIPHERAL_CLOCK_ENABLE | IO_PERIPHERAL_MODULE_ENABLE,
        0,
        0,
        0
    },

    /* LPM_OPERATION_MODE_SLEEP */
    {
        IO_PERIPHERAL_PIN_MUX_ENABLE | IO_PERIPHERAL_CLOCK_ENABLE | IO_PERIPHERAL_MODULE_ENABLE,
        0,
        0,
        0
    },

    /* LPM_OPERATION_MODE_STOP */
    {
        IO_PERIPHERAL_PIN_MUX_DISABLE | IO_PERIPHERAL_CLOCK_DISABLE,
        0,
        0,
        0
    },
    /* LPM_OPERATION_MODE_HSRUN */
    {
        IO_PERIPHERAL_PIN_MUX_ENABLE | IO_PERIPHERAL_CLOCK_ENABLE | IO_PERIPHERAL_MODULE_ENABLE,
        0,
        0,
        0
    }
};

#endif



const KUART_INIT_STRUCT _bsp_sci0_init = {
   /* queue size         */ BSPCFG_SCI0_QUEUE_SIZE,
   /* Channel            */ 0,
   /* Clock Speed        */ BSP_SYSTEM_CLOCK,       /* SCI0 operates only on system clock */
   /* Baud rate          */ BSPCFG_SCI0_BAUD_RATE,
   /* RX/TX Int vect     */ INT_UART0_RX_TX,
   /* ERR Int vect       */ INT_UART0_ERR,
   /* RX/TX priority     */ 3,
   /* ERR priority       */ 4
#if MQX_ENABLE_LOW_POWER
   ,
   /* Clock source       */ CM_CLOCK_SOURCE_CORE,
   /* LPM operation info */ _bsp_sci0_operation_modes
#endif
   ,
   1,
   2
};

const KUART_INIT_STRUCT _bsp_sci1_init = {
   /* queue size         */ BSPCFG_SCI1_QUEUE_SIZE,
   /* Channel            */ 1,
   /* Clock Speed        */ BSP_SYSTEM_CLOCK,   	/* SCI1 operates only on system clock */
   /* Baud rate          */ BSPCFG_SCI1_BAUD_RATE,
   /* RX/TX Int vect     */ INT_UART1_RX_TX,
   /* ERR Int vect       */ INT_UART1_ERR,
   /* RX/TX priority     */ 3,
   /* ERR priority       */ 4
#if MQX_ENABLE_LOW_POWER
   ,
   /* Clock source       */ CM_CLOCK_SOURCE_CORE,
   /* LPM operation info */ _bsp_sci1_operation_modes
#endif
   ,
   1,
   2
};

const KUART_INIT_STRUCT _bsp_sci2_init = {
   /* queue size         */ BSPCFG_SCI2_QUEUE_SIZE,
   /* Channel            */ 2,
   /* Clock Speed        */ BSP_BUS_CLOCK,       	/* SCI2 operates only on bus clock */
   /* Baud rate          */ BSPCFG_SCI2_BAUD_RATE,
   /* RX/TX Int vect     */ INT_UART2_RX_TX,
   /* ERR Int vect       */ INT_UART2_ERR,
   /* RX/TX priority     */ 3,
   /* ERR priority       */ 4
#if MQX_ENABLE_LOW_POWER
   ,
   /* Clock source       */ CM_CLOCK_SOURCE_BUS,
   /* LPM operation info */ _bsp_sci2_operation_modes
#endif
   ,
   1,
   2
};

const KUART_INIT_STRUCT _bsp_sci3_init = {
   /* queue size         */ BSPCFG_SCI3_QUEUE_SIZE,
   /* Channel            */ 3,
   /* Clock Speed        */ BSP_BUS_CLOCK,       	/* SCI3 operates only on bus clock */
   /* Baud rate          */ BSPCFG_SCI3_BAUD_RATE,
   /* RX/TX Int vect     */ INT_UART3_RX_TX,
   /* ERR Int vect       */ INT_UART3_ERR,
   /* RX/TX priority     */ 3,
   /* ERR priority       */ 4
#if MQX_ENABLE_LOW_POWER
   ,
   /* Clock source       */ CM_CLOCK_SOURCE_BUS,
   /* LPM operation info */ _bsp_sci3_operation_modes
#endif
   ,
   1,
   2
};

const KUART_INIT_STRUCT _bsp_sci4_init = {
   /* queue size         */ BSPCFG_SCI4_QUEUE_SIZE,
   /* Channel            */ 4,
   /* Clock Speed        */ BSP_BUS_CLOCK,       	/* SCI4 operates only on bus clock */
   /* Baud rate          */ BSPCFG_SCI4_BAUD_RATE,
   /* RX/TX Int vect     */ INT_UART4_RX_TX,
   /* ERR Int vect       */ INT_UART4_ERR,
   /* RX/TX priority     */ 3,
   /* ERR priority       */ 4
#if MQX_ENABLE_LOW_POWER
   ,
   /* Clock source       */ CM_CLOCK_SOURCE_BUS,
   /* LPM operation info */ _bsp_sci4_operation_modes
#endif
   ,
   1,
   2
};


/* EOF */
