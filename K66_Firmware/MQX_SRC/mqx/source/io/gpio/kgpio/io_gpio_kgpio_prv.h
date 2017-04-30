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

#ifndef __io_gpio_cpu_prv_h__
#define __io_gpio_cpu_prv_h__

#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------*/
/*
**                          TYPE DEFINITIONS
*/

typedef enum devices {
     DEV_INPUT,
     DEV_OUTPUT
} DEVICE_TYPE;

typedef union pin_map_struct {
    struct {
        uint32_t  porta;
        uint32_t  portb;
        uint32_t  portc;
        uint32_t  portd;
        uint32_t  porte;
    } reg;
    uint32_t memory32[5];
} GPIO_PIN_MAP, * GPIO_PIN_MAP_PTR;

#ifdef __cplusplus
}
#endif

#endif

/* EOF */
