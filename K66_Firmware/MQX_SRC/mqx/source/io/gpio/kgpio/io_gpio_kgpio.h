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
*   The file contains definitions used in user program and/or in other
*   kernel modules to access GPIO pins
*
*
*END************************************************************************/

#ifndef __io_gpio_cpu_h__
#define __io_gpio_cpu_h__

#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------*/
/*
**                          CONSTANT DEFINITIONS
*/


#define GPIO_PORT_A ((0x0000 << GPIO_PIN_PORT_SHIFT) | GPIO_PIN_VALID)
#define GPIO_PORT_B ((0x0001 << GPIO_PIN_PORT_SHIFT) | GPIO_PIN_VALID)
#define GPIO_PORT_C ((0x0002 << GPIO_PIN_PORT_SHIFT) | GPIO_PIN_VALID)
#define GPIO_PORT_D ((0x0003 << GPIO_PIN_PORT_SHIFT) | GPIO_PIN_VALID)
#define GPIO_PORT_E ((0x0004 << GPIO_PIN_PORT_SHIFT) | GPIO_PIN_VALID)
#ifdef PORTF_BASE_PTR 
	#define GPIO_PORT_F	((0x0005 << GPIO_PIN_PORT_SHIFT) | GPIO_PIN_VALID)
#endif
/*----------------------------------------------------------------------*/
/*
**                          TYPE DEFINITIONS
*/


typedef struct irq_map_struct {
   uint8_t  portnq;
} GPIO_IRQ_MAP, * GPIO_IRQ_MAP_PTR;



#ifdef __cplusplus
}
#endif

#endif

/* EOF */
