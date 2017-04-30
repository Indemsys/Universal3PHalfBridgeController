/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
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
*   This include file is used to define the configuration for the
*   timing tests
*
*
*END************************************************************************/

#ifndef __config_h__
  #define __config_h__

  #define TEST_ARRAY_DIV              1
  #define LOOP_OVERHEAD_AVG_SIZE      3

extern _mqx_uint ints;

/*=========================================================================*/
/* Define the Compiler being used */

  #if defined(__CODEWARRIOR__)
    #if defined(PSP_COLDFIRE)
      #define tCOMPILER "Freescale CodeWarrior for ColdFire"
    #elif defined(PSP_POWERPC)
      #define tCOMPILER "Freescale CodeWarrior for PowerPC"
    #elif defined(PSP_MQX_CPU_IS_ARM)
      #define tCOMPILER "Freescale CodeWarrior for ARM"
    #else
      #define tCOMPILER "Freescale CodeWarrior"
    #endif
  #elif defined(__GNUC__)
    #define tCOMPILER       "GCC compiler "
  #elif defined(__IAR_SYSTEMS_ICC__)
    #if defined(__ICCARM__)
      #define tCOMPILER       "IAR Embedded Workbench for ARM"
    #elif defined(__ICCCF__)
      #define tCOMPILER       "IAR Embedded Workbench for ColdFire"
    #endif
    #define tCOMPILER_VERION    __VER__
  #elif defined(__ARMCC_VERSION)
//tCOMPILER is defined in kinetis or vybrid
    #define tCOMPILER_VERION    __VER__
  #else
    #define tCOMPILER "Undefined compiler"
  #endif






  #if PSP_MQX_CPU_IS_KINETIS
    #if defined(__IAR_SYSTEMS_ICC__)
      #ifndef NDEBUG  /* NDEBUG is automatically defined in Release version */
        #define _DEBUG                  1
      #endif

      #if FLASH_TARGET
        #define _TARGET                 "Int Flash"
      #else
        #define _TARGET                 "Int RAM"
      #endif
    #elif defined(__ARMCC_VERSION)
      #define tCOMPILER       "uVision4"
    #endif

/* Only Register ABI is used */
    #define _ABI                    "RegABI"

    #if BSP_K66BLEZ1 == 1
      #define tBOARD              "K66BLEZ1"
      #define tCPU                "MK66FX1M0VLQ18"

      #define DDR_DATA_TARGET 0
      #if DDR_DATA_TARGET
        #define tMEMORY          "128MB External DDR2 RAM"
      #else
        #define tMEMORY          "256K Internal SRAM"
      #endif
    #endif

    #define TEST_ARRAY_SIZE         50 // 50
    #define TASK_ARRAY_SIZE         50 // 50

    #define INTERRUPT_VECTOR        79
    #define MORE_STACK              200

    #define SYSTEM_CLOCK           (BSP_SYSTEM_CLOCK / 1000000)       /* system bus frequency in MHz */


volatile uint32_t *int_request;

void setup_interrupt(void);
void generate_interrupt(void);
void interrupt_ack(void *isr_value);

void setup_interrupt(void)
{
  _bsp_int_init(INT_FTM1, 2, 0, TRUE);
}

void generate_interrupt(void)
{
  /* Enable interrupts in the FTM1 module */
  //FTM1_SC |= FTM_SC_TOIE_MASK;
  NVICSTIR = NVIC_STIR_INTID(INTERRUPT_VECTOR - 16);
}

void interrupt_ack(void *isr_value)
{
  /* Acknowledge Interrupt */
  //FTM1_SC &= ~FTM_SC_TOF_MASK;
  //FTM1_SC &= ~FTM_SC_TOIE_MASK;
  ++ints;
}
  #endif /* PSP_MQX_CPU_IS_KINETIS */

  #ifndef tBOARD
    #error "tBOARD has not been defined at the start of config.h"
  #endif

  #ifndef MORE_STACK
    #error "missing psp and compiler specification"
  #endif


/*=========================================================================*/
/* Target settings */

  #ifdef _TARGET
    #define tTARGET   _TARGET
  #else
    #define tTARGET   "Unknown"
  #endif

  #ifdef _DEBUG
    #define tOPTIM    "Debug"
  #else
    #define tOPTIM    "Release"
  #endif

  #ifdef _ABI
    #define tABI      _ABI
  #else
    #define tABI      "Unknown"
  #endif


/*=========================================================================*/

/* Define the PSP being used */

  #if MQX_USE_LWMEM_ALLOCATOR
    #define MEMBLOCK_STRUCT LWMEM_BLOCK_STRUCT
  #else
    #define MEMBLOCK_STRUCT STOREBLOCK_STRUCT
  #endif

  #define tKERNEL_RAM       sizeof(KERNEL_DATA_STRUCT)
  #define tISR              sizeof(INTERRUPT_TABLE_STRUCT)
  #define tTASK             sizeof(TD_STRUCT)
  #define tMEMORY_BLOCK     sizeof(MEMBLOCK_STRUCT)
  #define tPARTITION        sizeof(INTERNAL_PARTITION_BLOCK_STRUCT)
  #define tTASKQ            sizeof(TASK_QUEUE_STRUCT)
  #define tMESSAGE          sizeof(INTERNAL_MESSAGE_STRUCT)
  #define tMSGQ             sizeof(MSGQ_STRUCT)
  #define tMUTEX            sizeof(MUTEX_STRUCT)
  #define tSEM              sizeof(SEM_STRUCT)+sizeof(NAME_STRUCT)
  #define tEVENT            sizeof(EVENT_STRUCT)+sizeof(NAME_STRUCT)
  #define tTIMER            sizeof(TIMER_ENTRY_STRUCT)

  #define tLWEVENT          sizeof(LWEVENT_STRUCT)
  #define tLWSEM            sizeof(LWSEM_STRUCT)
  #define tLWMEM            sizeof(LWMEM_BLOCK_STRUCT)
  #define tLWTIMER          sizeof(LWTIMER_STRUCT)

/*=========================================================================*/

  #ifndef ALIGN
    #define ALIGN()
  #endif

#endif /* __config_h__ */
/* EOF */
