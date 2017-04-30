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
#ifndef __io_gpio_h__
#define __io_gpio_h__

#include "ioctl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t GPIO_PIN_STRUCT;
typedef void (* IRQ_FUNC)(void *);

/*----------------------------------------------------------------------*/
/*
**                          CONSTANT DEFINITIONS
*/


#define GPIO_IOCTL_ADD_PINS         _IO(IO_TYPE_GPIO,0x01)  /* Add pins to the file */
#define GPIO_IOCTL_WRITE_LOG1       _IO(IO_TYPE_GPIO,0x02)  /* Set pins on output port */
#define GPIO_IOCTL_WRITE_LOG0       _IO(IO_TYPE_GPIO,0x03)  /* Clear (set to 0) pins on output port */
#define GPIO_IOCTL_READ             _IO(IO_TYPE_GPIO,0x04)  /* Read data from input port */
#define GPIO_IOCTL_WRITE            _IO(IO_TYPE_GPIO,0x05)  /* Write data to output port */
#define GPIO_IOCTL_SET_IRQ_FUNCTION _IO(IO_TYPE_GPIO,0x06)  /* Set IRQ handler function */
#define GPIO_IOCTL_ENABLE_IRQ       _IO(IO_TYPE_GPIO,0x07)  /* Enable pins opened as IRQ in file */
#define GPIO_IOCTL_DISABLE_IRQ      _IO(IO_TYPE_GPIO,0x08)  /* Disable pins opened as IRQ in file */
#define GPIO_IOCTL_CPU_SPECIFIC     _IO(IO_TYPE_GPIO,0x09)  /* Next commands are specific for CPU */


#define GPIO_LIST_END           0x00000000
#define GPIO_PIN_VALID          0x80000000
#define GPIO_PIN_STATUS         0x40000000
#define GPIO_PIN_STATUS_0       0x00000000
#define GPIO_PIN_STATUS_1       0x40000000
#define GPIO_PIN_IRQ_RISING     0x02000000
#define GPIO_PIN_IRQ_FALLING    0x01000000
#define GPIO_PIN_IRQ_ZERO       0x04000000
#define GPIO_PIN_IRQ_ONE        0x08000000
#define GPIO_PIN_ADDR           0x00FFFFFF

#if PSP_MQX_CPU_IS_KINETIS
  #define GPIO_PIN_IRQ_LIST       GPIO_PIN_IRQ_RISING | GPIO_PIN_IRQ_FALLING | GPIO_PIN_IRQ_ZERO | GPIO_PIN_IRQ_ONE
  #define GPIO_PIN_PORT_SHIFT     0x05
  #define GPIO_PIN_BIT_MASK       0x1F
  #define GPIO_GET_PIN_USED(pin_map, addr, pin) ( pin_map.memory32[addr] & pin )
  #define GPIO_SET_PIN_USED(pin_map, addr, pin) { pin_map.memory32[addr] |= pin; }
#else
  #define GPIO_PIN_IRQ_LIST       GPIO_PIN_IRQ_RISING | GPIO_PIN_IRQ_FALLING
  #define GPIO_PIN_PORT_SHIFT     0x03
  #define GPIO_PIN_BIT_MASK       0x07
  #define GPIO_GET_PIN_USED(pin_map, addr, pin) ( pin_map.memory8[addr] & pin )
  #define GPIO_SET_PIN_USED(pin_map, addr, pin) { pin_map.memory8[addr] |= pin; }
#endif

#define GPIO_GET_PORT(x)          ((x) & GPIO_PIN_ADDR) >> GPIO_PIN_PORT_SHIFT
#define GPIO_GET_BIT_MASK(x)      (1 << ((x) & GPIO_PIN_BIT_MASK )) 


#define GPIO_PIN_IRQ    GPIO_PIN_IRQ_RISING /* support of legacy IRQ pin definition */

#define GPIO_PIN0   (0)
#define GPIO_PIN1   (1)
#define GPIO_PIN2   (2)
#define GPIO_PIN3   (3)
#define GPIO_PIN4   (4)
#define GPIO_PIN5   (5)
#define GPIO_PIN6   (6)
#define GPIO_PIN7   (7)
#define GPIO_PIN8   (8)
#define GPIO_PIN9   (9)
#define GPIO_PIN10  (10)
#define GPIO_PIN11  (11)
#define GPIO_PIN12  (12)
#define GPIO_PIN13  (13)
#define GPIO_PIN14  (14)
#define GPIO_PIN15  (15)
#define GPIO_PIN16  (16)
#define GPIO_PIN17  (17)
#define GPIO_PIN18  (18)
#define GPIO_PIN19  (19)
#define GPIO_PIN20  (20)
#define GPIO_PIN21  (21)
#define GPIO_PIN22  (22)
#define GPIO_PIN23  (23)
#define GPIO_PIN24  (24)
#define GPIO_PIN25  (25)
#define GPIO_PIN26  (26)
#define GPIO_PIN27  (27)
#define GPIO_PIN28  (28)
#define GPIO_PIN29  (29)
#define GPIO_PIN30  (30)
#define GPIO_PIN31  (31)
#define GPIO_PIN(x) (x)

/*----------------------------------------------------------------------*/
/*
**                          TYPE DEFINITIONS
*/

/*----------------------------------------------------------------------*/
/*
**                          FUNCTION PROTOTYPES
*/

extern _mqx_uint _io_gpio_install(char *);
void   *_bsp_get_gpio_base_address(void);

/*----------------------------------------------------------------------*/
/*
**                          DEVICE-SPECIFIC DEFINITIONS
*/

#if MQX_CPU == PSP_CPU_MCF5282
  #include "io_gpio_mcf5235.h"
#elif PSP_MQX_CPU_IS_MCF5225X
  #include "io_gpio_mcf5225.h"
#elif PSP_MQX_CPU_IS_MCF5227X
  #include "io_gpio_mcf5227.h"
#elif PSP_MQX_CPU_IS_MCF51AC
  #include "io_gpio_mcf51ac.h"  
#elif PSP_MQX_CPU_IS_MCF51JM
  #include "io_gpio_mcf51jm.h"  
#elif PSP_MQX_CPU_IS_MCF51AG
  #include "io_gpio_mcf51ag.h"   
#elif PSP_MQX_CPU_IS_MCF51CN
  #include "io_gpio_mcf51cn.h"
#elif PSP_MQX_CPU_IS_MCF51EM
  #include "io_gpio_mcf51em.h"
#elif PSP_MQX_CPU_IS_MCF51MM
  #include "io_gpio_mcf51mm.h"
 #elif PSP_MQX_CPU_IS_MCF51JE
  #include "io_gpio_mcf51je.h"   
#elif PSP_MQX_CPU_IS_MCF5222X
  #include "io_gpio_mcf5222.h"
#elif PSP_MQX_CPU_IS_MCF5223X
  #include "io_gpio_mcf5223.h"
#elif PSP_MQX_CPU_IS_MCF5441X
  #include "io_gpio_mcf5441.h"
#elif PSP_MQX_CPU_IS_MCF5301X
  #include "io_gpio_mcf5301.h"
#elif PSP_MQX_CPU_IS_MCF532X
  #include "io_gpio_mcf532.h"
#elif PSP_MQX_CPU_IS_MCF520X
  #include "io_gpio_mcf520.h"
#elif PSP_MQX_CPU_IS_KINETIS
  #include "io_gpio_kgpio.h"
#elif MQX_CPU==PSP_CPU_MPC8306
  #include "io_gpio_mpc8306.h"
#elif MQX_CPU==PSP_CPU_MPC8308
  #include "io_gpio_mpc8308.h"
#elif MQX_CPU==PSP_CPU_MPC8309
  #include "io_gpio_mpc8309.h"
#elif PSP_MQX_CPU_IS_MCF51FD
  #include "io_gpio_mcf51fd.h"
#elif PSP_MQX_CPU_IS_MCF51JF
  #include "io_gpio_mcf51jf.h"
#elif PSP_MQX_CPU_IS_MCF51QM
  #include "io_gpio_mcf51qm.h"

#else
  #error IO_GPIO device driver not supported for processor.
#endif

#ifdef __cplusplus
}
#endif

#endif

/* EOF */
