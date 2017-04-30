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
*   Low Power Manager private definitions.
*
*
*END************************************************************************/

#ifndef __lpm_prv_h__
#define __lpm_prv_h__


/*-------------------------------------------------------------------------*/
/*
**                            CONSTANT DEFINITIONS
*/


/*-------------------------------------------------------------------------*/
/*
**                            MACRO DECLARATIONS
*/


/*-------------------------------------------------------------------------*/
/*
**                            DATATYPE DECLARATIONS
*/

typedef struct lpm_driver_entry_struct {
    /* Previous in the driver list */
    struct lpm_driver_entry_struct      *PREVIOUS;

    /* Next in the driver list */
    struct lpm_driver_entry_struct      *NEXT;
    
    /* Driver registration record */
    LPM_REGISTRATION_STRUCT              REGISTRATION;

    /* Driver specific data passed to callbacks */
    void                                *DATA;
    
    /* Unique registration identification */
    _mqx_uint                            ID;

} LPM_DRIVER_ENTRY_STRUCT, * LPM_DRIVER_ENTRY_STRUCT_PTR;

typedef struct lpm_state_struct {
    /* CPU core operation mode behavior specification */
    const LPM_CPU_OPERATION_MODE      *CPU_OPERATION_MODES;
    
    /* Current system operation mode */
    LPM_OPERATION_MODE                 OPERATION_MODE;
    
    /* List of registered drivers */
    LPM_DRIVER_ENTRY_STRUCT_PTR        DRIVER_ENTRIES;
    
    /* Unique ID counter */
    _mqx_uint                          COUNTER;
    
    /* LPM functions synchronization */
    LWSEM_STRUCT                       SEMAPHORE;
    
    /* Whether idle sleep is turned ON */
    bool                            IDLE_SLEEP;
    
    /* LPM LLWU user wakeup callback */
    void (*                         LLWU_WAKEUP_CALLBACK)(uint32_t);
} LPM_STATE_STRUCT, * LPM_STATE_STRUCT_PTR;


/*-------------------------------------------------------------------------*/
/*
**                            FUNCTION PROTOTYPES
*/

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif


#endif

/* EOF */
