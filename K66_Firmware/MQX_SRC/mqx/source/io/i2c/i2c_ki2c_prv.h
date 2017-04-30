#ifndef _ki2c_prv_h
#define _ki2c_prv_h 
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
*   required for the I2C drivers for Kinetis family.
*
*
*END************************************************************************/

#include <bsp.h>

/*--------------------------------------------------------------------------*/
/*
**                    DATATYPE DECLARATIONS
*/

/*
** KI2C_INFO_STRUCT
** Run time state information for each serial channel
*/
typedef struct ki2c_info_struct
{  
   /* Current initialized values */
   KI2C_INIT_STRUCT                  INIT;
   
   I2C_MemMapPtr                     I2C_PTR;
   
   /* The previous interrupt handler and data */
   void                  (_CODE_PTR_ OLD_ISR)(void *);
   void                             *OLD_ISR_DATA;

   /* Interrupt vector */
   uint32_t                           VECTOR;
   
#if BSPCFG_ENABLE_LEGACY_II2C_SLAVE
   /* The previous interrupt handler and data */
   void                  (_CODE_PTR_ OLD_PORT_ISR)(void *);
   void                             *OLD_PORT_ISR_DATA;
   
   uint32_t                           PORT_VECTOR;
#if PSP_MQX_CPU_IS_COLDFIRE
   PCTL_MemMapPtr                     PORT_BASE;
#elif PSP_MQX_CPU_IS_KINETIS
   PORT_MemMapPtr                     PORT_BASE;
#endif
   
   uint32_t                           SDA_PIN_NUM;
#endif
   
   /* Actual mode */
   uint8_t                            MODE;

   /* Actual state */
   uint8_t                            STATE;

   /* Destination address */
   uint8_t                            ADDRESSEE;
   
   /* Operation flags */
   uint8_t                            OPERATION;
   
   /* Number of bytes requested for receive */
   uint32_t                           RX_REQUEST;

   /* Pointer to the buffer to use for Tx/Rx data */
   unsigned char                         *RX_BUFFER;

   /* Rx index */
   uint32_t                           RX_INDEX;

   /* Rx buffer size */
   uint32_t                           RX_BUFFER_SIZE;

   /* Pointer to the buffer to use for current Tx data */
   unsigned char                         *TX_BUFFER;

   /* Tx index */
   uint32_t                           TX_INDEX;

   /* Tx buffer size */
   uint32_t                           TX_BUFFER_SIZE;
   
   /* Internal synchronize signal  */
   LWSEM_STRUCT                       LWSEM;

   /* Statistical information */
   I2C_STATISTICS_STRUCT             STATISTICS;

} KI2C_INFO_STRUCT, * KI2C_INFO_STRUCT_PTR; 

typedef volatile struct ki2c_info_struct * VKI2C_INFO_STRUCT_PTR; 

/*
** KI2C_BAUDRATE_STRUCT
*/
typedef struct ki2c_baudrate_struct
{  
   /* Baudrate */
   uint32_t                           BAUD_RATE;
   
   /* Divider */
   uint8_t                            IC;   
   
} KI2C_BAUDRATE_STRUCT, * KI2C_BAUDRATE_STRUCT_PTR; 

#endif
/* EOF */
