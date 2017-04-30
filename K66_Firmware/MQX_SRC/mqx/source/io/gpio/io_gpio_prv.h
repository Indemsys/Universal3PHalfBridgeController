#ifndef __io_gpio_driver_h__
#define __io_gpio_driver_h__
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
*   The file contains functions prototype, defines, structure 
*   definitions private to the gpio driver.
*
*
*END************************************************************************/

/*----------------------------------------------------------------------*/
/*
**                    DATATYPE DEFINITIONS
*/

/* Internal functions to IO_GPIO */
#ifdef __cplusplus
extern "C" {
#endif

#if MQX_CPU == PSP_CPU_MCF5282
  #include "io_gpio_mcf5235_prv.h"
#elif PSP_MQX_CPU_IS_MCF5225X
  #include "io_gpio_mcf5225_prv.h"
#elif PSP_MQX_CPU_IS_MCF5227X
  #include "io_gpio_mcf5227_prv.h"
#elif PSP_MQX_CPU_IS_MCF51AC
  #include "io_gpio_mcf51ac_prv.h"   
#elif PSP_MQX_CPU_IS_MCF51JM
  #include "io_gpio_mcf51jm_prv.h"  
#elif PSP_MQX_CPU_IS_MCF51AG
  #include "io_gpio_mcf51ag_prv.h"  
#elif PSP_MQX_CPU_IS_MCF51CN
  #include "io_gpio_mcf51cn_prv.h"
#elif PSP_MQX_CPU_IS_MCF51EM
  #include "io_gpio_mcf51em_prv.h"
#elif PSP_MQX_CPU_IS_MCF51MM
  #include "io_gpio_mcf51mm_prv.h" 
#elif PSP_MQX_CPU_IS_MCF51JE
  #include "io_gpio_mcf51je_prv.h" 
#elif PSP_MQX_CPU_IS_MCF5222X
  #include "io_gpio_mcf5222_prv.h"
#elif PSP_MQX_CPU_IS_MCF5223X
  #include "io_gpio_mcf5223_prv.h"
#elif PSP_MQX_CPU_IS_MCF5441X
  #include "io_gpio_mcf5441_prv.h"
#elif PSP_MQX_CPU_IS_MCF5301X
  #include "io_gpio_mcf5301_prv.h"
#elif PSP_MQX_CPU_IS_MCF532X
  #include "io_gpio_mcf532_prv.h"
#elif PSP_MQX_CPU_IS_MCF520X
  #include "io_gpio_mcf520_prv.h"
#elif PSP_MQX_CPU_IS_KINETIS
  #include "io_gpio_kgpio_prv.h"  
#elif MQX_CPU==PSP_CPU_MPC8306
  #include "io_gpio_mpc8306_prv.h"
#elif MQX_CPU==PSP_CPU_MPC8308
  #include "io_gpio_mpc8308_prv.h"
#elif MQX_CPU==PSP_CPU_MPC8309
  #include "io_gpio_mpc8309_prv.h"
#elif PSP_MQX_CPU_IS_MCF51FD
  #include "io_gpio_mcf51fd_prv.h"
#elif PSP_MQX_CPU_IS_MCF51JF
  #include "io_gpio_mcf51jf_prv.h"
#elif PSP_MQX_CPU_IS_MCF51QM
  #include "io_gpio_mcf51qm_prv.h"
	#else
  #error IO_GPIO device driver not supported for processor.
#endif

typedef struct gpio_dev_data {
    struct gpio_dev_data      *NEXT;             /* this is used only to link IRQ maps */
    DEVICE_TYPE type;
    IRQ_FUNC         irq_func;
    GPIO_PIN_MAP     pin_map;
    GPIO_PIN_MAP     irqp_map;
    GPIO_PIN_MAP     irqn_map;
#if (defined PSP_MQX_CPU_IS_PIONEER2) || PSP_MQX_CPU_IS_KINETIS  
    GPIO_PIN_MAP     irql_map;
#endif    
} GPIO_DEV_DATA, * GPIO_DEV_DATA_PTR;

#ifdef __cplusplus
}
#endif

#endif

/* EOF */
