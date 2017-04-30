#ifndef _ki2c_h
#define _ki2c_h 1
/*HEADER**********************************************************************
*
* Copyright 2008-2014 Freescale Semiconductor, Inc.
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
*   This file contains the definitions of constants and structures
*   required for the I2C drivers for the Kinetis family.
*
*
*END************************************************************************/


/*--------------------------------------------------------------------------*/
/*
**                    DATATYPE DECLARATIONS
*/

/*
** KI2C_INIT_STRUCT
**
** This structure defines the initialization parameters to be used
** when a serial port is initialized.
*/
typedef struct ki2c_init_struct
{
   /* The I2C channel to initialize */
   uint8_t                 CHANNEL;

   /* Default operating mode */
   uint8_t                 MODE;

   /* The I2C station address for the channel */
   uint8_t                 STATION_ADDRESS;

   /* Desired baud rate */
   uint32_t                BAUD_RATE;

   
#if !(BSP_TWRMCF51FD || BSP_TWRMCF51JF || BSP_TWRMCF51QM)   
   /* Interrupt level to use if interrupt driven */
   _int_level             LEVEL;

   /* Sub level within the interrupt level to use if interrupt driven */
   _int_priority          SUBLEVEL;
#endif
   /* Tx buffer size (interrupt only) */
   uint32_t                TX_BUFFER_SIZE;

   /* Rx buffer size (interrupt only) */
   uint32_t                RX_BUFFER_SIZE;
  
#if BSPCFG_ENABLE_LEGACY_II2C_SLAVE
   /* I2C bus port base address */
#if PSP_MQX_CPU_IS_COLDFIRE
   PCTL_MemMapPtr          PORT_BASE;
#elif PSP_MQX_CPU_IS_KINETIS
   PORT_MemMapPtr          PORT_BASE;
#else
   #error CPU is not supported!
#endif

   /* I2C bus port vector number */
   uint32_t                PORT_VECTOR;

   /* I2C bus sda pin number */
   uint32_t                SDA_PIN_NUM;
#endif
   
} KI2C_INIT_STRUCT, * KI2C_INIT_STRUCT_PTR;

typedef const KI2C_INIT_STRUCT * KI2C_INIT_STRUCT_CPTR;

/*--------------------------------------------------------------------------*/
/* 
**                        FUNCTION PROTOTYPES
*/

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t _ki2c_polled_install (char *, KI2C_INIT_STRUCT_CPTR);
extern uint32_t _ki2c_int_install (char *, KI2C_INIT_STRUCT_CPTR);
extern void   *_bsp_get_i2c_base_address (uint8_t);
extern uint32_t _bsp_get_i2c_vector (uint8_t);
#if BSPCFG_ENABLE_LEGACY_II2C_SLAVE
extern uint32_t _bsp_get_i2c_stop_detect_vector(uint8_t, void *);
#endif

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
