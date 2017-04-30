/*HEADER**********************************************************************
*
* Copyright 2011 Freescale Semiconductor, Inc.
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
*   Low Power Manager header file.
*
*
*END************************************************************************/

#ifndef __lpm_h__
#define __lpm_h__

/*-------------------------------------------------------------------------*/
/*
**                            CONSTANT DEFINITIONS
*/

typedef enum {
    LPM_NOTIFICATION_TYPE_PRE,
    LPM_NOTIFICATION_TYPE_POST
} LPM_NOTIFICATION_TYPE;

typedef enum {
    LPM_NOTIFICATION_RESULT_OK,
    LPM_NOTIFICATION_RESULT_ERROR
} LPM_NOTIFICATION_RESULT;


/*-------------------------------------------------------------------------*/
/*
**                            MACRO DECLARATIONS
*/

/*--------------------------------------------------------------------------*/
/*
 *                            MQX RESET SOURCE
 *
 */
#define MQX_RESET_SOURCE_INVALID                0x00
#define MQX_RESET_SOURCE_LLWU                   0x01
#define MQX_RESET_SOURCE_LOW_VOLTAGE_DETECT     0x02
#define MQX_RESET_SOURCE_LOSS_OF_CLOCK          0x03
#define MQX_RESET_SOURCE_WATCHDOG               0x04
#define MQX_RESET_SOURCE_EXTERNAL_PIN           0x05
#define MQX_RESET_SOURCE_POWER_ON               0x06
#define MQX_RESET_SOURCE_JTAG                   0x07
#define MQX_RESET_SOURCE_CORE_LOCKUP            0x08
#define MQX_RESET_SOURCE_SOFTWARE               0x09
#define MQX_RESET_SOURCE_MDM_AP                 0x0A
#define MQX_RESET_SOURCE_EZPT                   0x0B
#define MQX_RESET_SOURCE_SACKERR                0x0C
#define MQX_RESET_SOURCE_TAMPER                 0x0D

/*--------------------------------------------------------------------------*/
/*
 *                            MQX WAKEUP SOURCE
 *
 */
#define MQX_WAKEUP_SOURCE_INVALID               0x00
#define MQX_WAKEUP_SOURCE_LLWU_P0               0x01
#define MQX_WAKEUP_SOURCE_LLWU_P1               0x02
#define MQX_WAKEUP_SOURCE_LLWU_P2               0x03
#define MQX_WAKEUP_SOURCE_LLWU_P3               0x04
#define MQX_WAKEUP_SOURCE_LLWU_P4               0x05
#define MQX_WAKEUP_SOURCE_LLWU_P5               0x06
#define MQX_WAKEUP_SOURCE_LLWU_P6               0x07
#define MQX_WAKEUP_SOURCE_LLWU_P7               0x08
#define MQX_WAKEUP_SOURCE_LLWU_P8               0x09
#define MQX_WAKEUP_SOURCE_LLWU_P9               0x0A
#define MQX_WAKEUP_SOURCE_LLWU_P10              0x0B
#define MQX_WAKEUP_SOURCE_LLWU_P11              0x0C
#define MQX_WAKEUP_SOURCE_LLWU_P12              0x0D
#define MQX_WAKEUP_SOURCE_LLWU_P13              0x0E
#define MQX_WAKEUP_SOURCE_LLWU_P14              0x0F
#define MQX_WAKEUP_SOURCE_LLWU_P15              0x10
#define MQX_WAKEUP_SOURCE_MODULE0               0x11
#define MQX_WAKEUP_SOURCE_MODULE1               0x12
#define MQX_WAKEUP_SOURCE_MODULE2               0x13
#define MQX_WAKEUP_SOURCE_MODULE3               0x14
#define MQX_WAKEUP_SOURCE_MODULE4               0x15
#define MQX_WAKEUP_SOURCE_MODULE5               0x16
#define MQX_WAKEUP_SOURCE_MODULE6               0x17
#define MQX_WAKEUP_SOURCE_MODULE7               0x18

/*-------------------------------------------------------------------------*/
/*
**                            DATATYPE DECLARATIONS
*/

typedef struct lpm_notification_struct {
    /* When the notification happens */
    LPM_NOTIFICATION_TYPE   NOTIFICATION_TYPE;
    
    /* Current system operation mode */
    LPM_OPERATION_MODE      OPERATION_MODE;
    
    /* Current system clock configuration */
    BSP_CLOCK_CONFIGURATION CLOCK_CONFIGURATION;
    
} LPM_NOTIFICATION_STRUCT, * LPM_NOTIFICATION_STRUCT_PTR;

typedef LPM_NOTIFICATION_RESULT (_CODE_PTR_ LPM_NOTIFICATION_CALLBACK)(LPM_NOTIFICATION_STRUCT_PTR, void *);

typedef struct lpm_registration_struct {
    /* Callback called when system clock configuration changes */
    LPM_NOTIFICATION_CALLBACK CLOCK_CONFIGURATION_CALLBACK;

    /* Callback called when system operation mode changes */
    LPM_NOTIFICATION_CALLBACK OPERATION_MODE_CALLBACK;

    /* The order (priority) of notifications among other drivers */
    _mqx_uint                 DEPENDENCY_LEVEL;
   
} LPM_REGISTRATION_STRUCT, * LPM_REGISTRATION_STRUCT_PTR;


/*-------------------------------------------------------------------------*/
/*
**                            FUNCTION PROTOTYPES
*/

#ifdef __cplusplus
extern "C" {
#endif


extern _mqx_uint                _lpm_install (const LPM_CPU_OPERATION_MODE *, LPM_OPERATION_MODE);
extern _mqx_uint                _lpm_uninstall (void);
extern _mqx_uint                _lpm_register_driver (const LPM_REGISTRATION_STRUCT_PTR, const void *, _mqx_uint_ptr);
extern _mqx_uint                _lpm_unregister_driver (_mqx_uint);
extern _mqx_uint                _lpm_set_clock_configuration (BSP_CLOCK_CONFIGURATION);
extern BSP_CLOCK_CONFIGURATION  _lpm_get_clock_configuration (void);
extern _mqx_uint                _lpm_set_operation_mode (LPM_OPERATION_MODE);
extern LPM_OPERATION_MODE       _lpm_get_operation_mode (void);
extern _mqx_uint                _lpm_idle_sleep_setup (bool);
extern void                     _lpm_register_wakeup_callback (uint32_t, uint8_t, void (*)(uint32_t));
extern void                     _lpm_unregister_wakeup_callback (uint32_t);
extern void                     _lpm_llwu_isr (void *);
extern void                     _lpm_llwu_clear_flag (uint32_t *);


#ifdef __cplusplus
}
#endif


#endif

/* EOF */
