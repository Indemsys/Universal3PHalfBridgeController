/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
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
*   This file contains the definitions of constants and structures
*   required for the sci drivers for the MCF51XX
*
*
*END************************************************************************/
#ifndef _serial_kuart_h_
#define _serial_kuart_h_

#if (PSP_MQX_CPU_IS_KINETIS || PSP_MQX_CPU_IS_VYBRID)
#include <lwtimer.h>
#include <dma.h>
#endif
/*--------------------------------------------------------------------------*/
/*
**                    DATATYPE DECLARATIONS
*/

#if MQX_ENABLE_LOW_POWER

/*
** KUART_CLOCK_CONFIGURATION_STRUCT
**
** This structure defines the behavior of KUART in specific
** low power clock configuration.
*/
typedef struct kuart_clock_configuration_struct
{
    /* Whether channel is enabled in particular clock configuration */
    uint8_t      ENABLED;

} KUART_CLOCK_CONFIGURATION_STRUCT, * KUART_CLOCK_CONFIGURATION_STRUCT_PTR;
typedef const KUART_CLOCK_CONFIGURATION_STRUCT * KUART_CLOCK_CONFIGURATION_STRUCT_CPTR;

/*
** KUART_OPERATION_MODE_STRUCT
**
** This structure defines the behavior of KUART in specific
** low power operation mode.
*/
typedef struct kuart_operation_mode_struct
{
    /* HW/wakeup enable/disable flags */
    uint8_t      FLAGS;

    /* Wakeup register bits combination: UART_C2_RWU_MASK, UART_C1_WAKE_MASK, UART_C1_ILT_MASK, UART_C4_MAEN1_MASK, UART_C4_MAEN2_MASK */
    uint8_t      WAKEUP_BITS;

    /* Wakeup settings of register UART_MA1 */
    uint8_t      MA1;

    /* Wakeup settings of register UART_MA2 */
    uint8_t      MA2;

} KUART_OPERATION_MODE_STRUCT, * KUART_OPERATION_MODE_STRUCT_PTR;
typedef const KUART_OPERATION_MODE_STRUCT * KUART_OPERATION_MODE_STRUCT_CPTR;

/*
**
** IO SERIAL LPM STRUCT
**
** This structure is used to store information regarding low power functionality.
**
*/
typedef struct io_serial_lpm_struct
{
   /* Low power manager registration handle */
   _mqx_uint           REGISTRATION_HANDLE;

   /* Low level device lock */
   LWSEM_STRUCT        LOCK;

   /* Special flags */
   uint8_t              FLAGS;

} IO_SERIAL_LPM_STRUCT, * IO_SERIAL_LPM_STRUCT_PTR;

#endif

/*
** KUART_INIT_STRUCT
**
** This structure defines the initialization parameters to be used
** when a serial port is initialized.
*/
typedef struct kuart_init_struct
{

   /* The size of the queues to buffer incoming/outgoing data */
   uint32_t QUEUE_SIZE;

   /* The device to initialize */
   uint32_t DEVICE;

   /* The clock speed of cpu */
   uint32_t CLOCK_SPEED;

   /* The baud rate for the channel */
   uint32_t BAUD_RATE;

   /* RX / TX interrupt vector */
   uint32_t RX_TX_VECTOR;

   /* ERR interrupt vector */
   uint32_t ERR_VECTOR;

   /* RX / TX interrupt vector priority */
   uint32_t RX_TX_PRIORITY;

   /* ERR interrupt vector priority */
   uint32_t ERR_PRIORITY;

#if MQX_ENABLE_LOW_POWER
   /* Clock source */
   CM_CLOCK_SOURCE                          CLOCK_SOURCE;

   /* Low power operation mode */
   KUART_OPERATION_MODE_STRUCT_CPTR         OPERATION_MODE;
#endif

#if (PSP_MQX_CPU_IS_KINETIS || PSP_MQX_CPU_IS_VYBRID)  
    uint8_t TX_DMA_CHANNEL;
    uint8_t RX_DMA_CHANNEL;
#endif        
} KUART_INIT_STRUCT, * KUART_INIT_STRUCT_PTR;
typedef const KUART_INIT_STRUCT * KUART_INIT_STRUCT_CPTR;

/*
** KUART_INFO_STRUCT
** Run time state information for each serial channel
*/
typedef struct kuart_info_struct
{
   /* The current init values for this port */
   KUART_INIT_STRUCT          INIT;

   /* The sci device register */
   UART_MemMapPtr             SCI_PTR;

   /* The previous interrupt handler and data */
   void       (_CODE_PTR_ OLD_ISR)(void *);
   void       (_CODE_PTR_ OLD_ISR_EXCEPTION_HANDLER)(uint32_t, uint32_t, void *,
               void *);
   void                             *OLD_ISR_DATA;

   /* Various flags */
   uint32_t                           FLAGS;
   
   /* Statistical information */
   uint32_t                           INTERRUPTS;
   uint32_t                           RX_CHARS;
   uint32_t                           TX_CHARS;
   uint32_t                           RX_BREAKS;
   uint32_t                           RX_PARITY_ERRORS;
   uint32_t                           RX_FRAMING_ERRORS;
   uint32_t                           RX_OVERRUNS;
   uint32_t                           RX_DROPPED_INPUT;
   uint32_t                           RX_NOISE_ERRORS;
#if (PSP_MQX_CPU_IS_KINETIS || PSP_MQX_CPU_IS_VYBRID)   
   uint8_t TX_DMA_CHAN;
   uint8_t RX_DMA_CHAN;
   uint32_t TX_DMA_HARDWARE_REQUEST;
   uint32_t RX_DMA_HARDWARE_REQUEST;
   uint32_t ERR_INT;
   LWTIMER_PERIOD_STRUCT_PTR LW_TIMER_PTR;
   LWTIMER_STRUCT_PTR LW_TIMER;
   uint8_t TX_KEEP;
   uint8_t RX_KEEP;
   DMA_CHANNEL_HANDLE TX_DCH;
   DMA_CHANNEL_HANDLE RX_DCH;
   DMA_TCD TX_TCD;
   DMA_TCD RX_TCD;
   uint32_t TX_DMA_SEQ;
   uint32_t RX_DMA_SEQ;	
   char *RX_BUF;
   char *TX_BUF;
   uint32_t NUM_BYTES_RQST;
   uint32_t NUM_BYTES_RCVED_IN_CURRENT_RQST;
#endif
} KUART_INFO_STRUCT, * KUART_INFO_STRUCT_PTR;

/*--------------------------------------------------------------------------*/
/*
**                        FUNCTION PROTOTYPES
*/

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t _kuart_polled_init(KUART_INIT_STRUCT_PTR, void **,char *);
extern uint32_t _kuart_polled_install(char *, KUART_INIT_STRUCT_CPTR, uint32_t);
extern uint32_t _kuart_int_install(char *, KUART_INIT_STRUCT_CPTR, uint32_t);
extern void   *_bsp_get_serial_base_address(uint8_t);
extern uint32_t _kuart_dma_install(char *, KUART_INIT_STRUCT_CPTR, uint32_t);
#if PSP_MQX_CPU_IS_KINETIS 
extern uint32_t _bsp_get_serial_error_int_num(uint8_t dev_num);
#endif
#if (PSP_MQX_CPU_IS_KINETIS || PSP_MQX_CPU_IS_VYBRID)
extern uint32_t _bsp_get_serial_tx_dma_request(uint8_t dev_num);
extern uint32_t _bsp_get_serial_rx_dma_request(uint8_t dev_num);
#endif

#ifdef __cplusplus
}
#endif

#endif //_serial_kuart_h_

/* EOF */
