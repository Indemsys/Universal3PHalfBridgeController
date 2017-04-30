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
*   This file contains Low Power Manager functionality.
*
*
*END************************************************************************/


#include "mqx_inc.h"
#include "bsp.h"
#include "lpm_prv.h"
#include "cm.h"


#if MQX_ENABLE_LOW_POWER


/* Global LPM state */
static LPM_STATE_STRUCT lpm_state_struct = {0}; 


/*FUNCTION*------------------------------------------------------------------
* 
* Function Name    : _lpm_restart_idle_task
* Returned Value   : MQX error code
* Comments         :
*    This function restarts idle task if idle sleep settings have changed.
*    Restart matrix description:
*    
*    sleep enabled      idle task parameter    sleep available    idle task_restart
*    0                  0                      0                  no (2)
*    0                  0                      1                  no (2)
*    0                  1                      0                  yes (parameter = 0)
*    0                  1                      1                  yes (parameter = 0)
*    1                  0                      0                  no (2)
*    1                  0                      1                  yes (parameter = 1)
*    1                  1                      0                  yes (parameter = 0)
*    1                  1                      1                  no (2)
*
*END*----------------------------------------------------------------------*/

static _mqx_uint _lpm_restart_idle_task 
    (
        /* [IN] Low power operation mode identifier */
        LPM_OPERATION_MODE target_mode,
          
        /* [IN] Idle sleep enable flag */
        bool            enabled
    )
{
    _mqx_uint              parameter, result = MQX_OK;
    _task_id               idle_task_id;
    bool                available;
    static const char      lpm_idle_task_restart_matrix[2][2][2] = {
        {{2, 2}, {0, 0}},
        {{2, 1}, {0, 2}}
    };
    
    idle_task_id = _task_get_id_from_name (MQX_IDLE_TASK_NAME);
    if (MQX_NULL_TASK_ID == idle_task_id)
    {
        result = MQX_INVALID_TASK_ID;
    }
    else
    {
        parameter = _task_get_parameter_for (idle_task_id);
        available = _lpm_idle_sleep_check ();
        parameter = lpm_idle_task_restart_matrix[enabled][parameter][available];
        if (parameter <= 1)
        {
            result = _task_restart (idle_task_id, &parameter, FALSE);
        }
    }
    return result;
}


/*FUNCTION*------------------------------------------------------------------
* 
* Function Name    : _lpm_install
* Returned Value   : MQX error code
* Comments         :
*    This function installs LPM into MQX.
*
*END*----------------------------------------------------------------------*/

_mqx_uint _lpm_install
    ( 
        /* [IN] Specification of CPU core low power operation modes available */
        const LPM_CPU_OPERATION_MODE  *cpu_operation_modes,

        /* [IN] Default low power operation mode identifier */
        LPM_OPERATION_MODE                 default_mode
    )
{
    _mqx_uint result;
    
    /* Check parameters */
    if ((NULL == cpu_operation_modes) || (LPM_OPERATION_MODES <= (_mqx_uint)default_mode))
    {
        return MQX_INVALID_PARAMETER;
    }
    
    /* Check if LPM not installed and initialize */
    if (LWSEM_VALID == lpm_state_struct.SEMAPHORE.VALID)
    {
        return MQX_COMPONENT_EXISTS;
    }
    result = _lwsem_create (&(lpm_state_struct.SEMAPHORE), 1);
    if (MQX_OK != result)
    {
        return MQX_IO_OPERATION_NOT_AVAILABLE;
    }
    
    /* LPM state setup */
    lpm_state_struct.CPU_OPERATION_MODES = cpu_operation_modes;
    lpm_state_struct.OPERATION_MODE = default_mode;
    lpm_state_struct.DRIVER_ENTRIES = NULL;
    lpm_state_struct.COUNTER = 0;
    lpm_state_struct.IDLE_SLEEP = FALSE;
    _lpm_restart_idle_task (default_mode, lpm_state_struct.IDLE_SLEEP);
    
    return MQX_OK;    
}


/*FUNCTION*------------------------------------------------------------------
* 
* Function Name    : _lpm_uninstall
* Returned Value   : MQX error code
* Comments         :
*    This function uninstalls LPM from MQX.
*
*END*----------------------------------------------------------------------*/

_mqx_uint _lpm_uninstall
    ( 
    )
{
    LPM_DRIVER_ENTRY_STRUCT_PTR entry_ptr, tmp_ptr;
    _mqx_uint                   result;
    
    /* Check if LPM installed and lock */
    result = _lwsem_wait (&(lpm_state_struct.SEMAPHORE));
    if (MQX_OK != result)
    {
        return MQX_IO_OPERATION_NOT_AVAILABLE;
    }
    
    /* Final restart of idle task to disable sleep feature */
    lpm_state_struct.IDLE_SLEEP = FALSE;
    _lpm_restart_idle_task (lpm_state_struct.OPERATION_MODE, lpm_state_struct.IDLE_SLEEP);
    
    /* De-initialize global LPM state and unlock */
    lpm_state_struct.CPU_OPERATION_MODES = NULL;
    lpm_state_struct.OPERATION_MODE = (LPM_OPERATION_MODE)0;
    lpm_state_struct.COUNTER = 0;
    for (entry_ptr = lpm_state_struct.DRIVER_ENTRIES; NULL != entry_ptr; )
    {
        tmp_ptr = entry_ptr->NEXT;
        _mem_free (entry_ptr);
        entry_ptr = tmp_ptr;
    }
    _lwsem_destroy (&(lpm_state_struct.SEMAPHORE));
    
    return MQX_OK;    
}


/*FUNCTION*------------------------------------------------------------------
* 
* Function Name    : _lpm_register_driver 
* Returned Value   : MQX error code and driver registration handle
* Comments         :
*    This function registers driver into LPM.
*
*END*----------------------------------------------------------------------*/

_mqx_uint _lpm_register_driver 
    ( 
        /* [IN] Driver low power callbacks specification */
        const LPM_REGISTRATION_STRUCT_PTR driver_registration_ptr,
        
        /* [IN] Driver specific data */
        const void                     *driver_specific_data_ptr,
        
        /* [OUT] Unique driver registration handle */
        _mqx_uint_ptr                     registration_handle_ptr
    )
{
    LPM_DRIVER_ENTRY_STRUCT_PTR           entry_ptr, tmp_ptr;
    _mqx_uint                             result;
    
    /* Check parameters */
    if ((NULL == driver_registration_ptr) || (NULL == registration_handle_ptr))
    {
        return MQX_INVALID_PARAMETER;
    }
    
    /* Check if LPM installed and lock */
    result = _lwsem_wait (&(lpm_state_struct.SEMAPHORE));
    if (MQX_OK != result)
    {
        return MQX_IO_OPERATION_NOT_AVAILABLE;
    }
    
    /* Allocate new registration record */
    entry_ptr = (LPM_DRIVER_ENTRY_STRUCT_PTR)_mem_alloc_system_zero ((_mem_size) sizeof (LPM_DRIVER_ENTRY_STRUCT));
    if (NULL == entry_ptr) 
    {
        _lwsem_post (&(lpm_state_struct.SEMAPHORE));
        return MQX_OUT_OF_MEMORY;
    }
    _mem_set_type (entry_ptr, MEM_TYPE_IO_LPM_DRIVER_ENTRY);
    
    /* Get new unique identifier */
    while (0 == (++lpm_state_struct.COUNTER))
    {
        ;
    }
    
    /* File new registration record (sort by dependency level) */
    entry_ptr->PREVIOUS = NULL;
    entry_ptr->NEXT = NULL;
    _mem_copy (driver_registration_ptr, &(entry_ptr->REGISTRATION), sizeof (entry_ptr->REGISTRATION));
    entry_ptr->DATA = (void *)driver_specific_data_ptr;
    entry_ptr->ID = lpm_state_struct.COUNTER;
    for (tmp_ptr = lpm_state_struct.DRIVER_ENTRIES; NULL != tmp_ptr; tmp_ptr = tmp_ptr->NEXT)
    {
        if (entry_ptr->REGISTRATION.DEPENDENCY_LEVEL < tmp_ptr->REGISTRATION.DEPENDENCY_LEVEL)
        {
            entry_ptr->NEXT = tmp_ptr;
            if (NULL != tmp_ptr->PREVIOUS)
            {
                tmp_ptr->PREVIOUS->NEXT = entry_ptr;
                entry_ptr->PREVIOUS = tmp_ptr->PREVIOUS;
            }
            tmp_ptr->PREVIOUS = entry_ptr;
            break;
        }
        if (NULL == tmp_ptr->NEXT)
        {
            tmp_ptr->NEXT = entry_ptr;
            entry_ptr->PREVIOUS = tmp_ptr;
            break;
        }        
    }
    if (NULL == entry_ptr->PREVIOUS)
    {
        lpm_state_struct.DRIVER_ENTRIES = entry_ptr;
    } 
    
    /* Return driver registration handle */
    *registration_handle_ptr = entry_ptr->ID;
    
    /* Unlock LPM */
    _lwsem_post (&(lpm_state_struct.SEMAPHORE));
    
    return MQX_OK;
}


/*FUNCTION*------------------------------------------------------------------
* 
* Function Name    : _lpm_unregister_driver 
* Returned Value   : MQX error code
* Comments         :
*    This function unregisters driver from LPM.
*
*END*----------------------------------------------------------------------*/

_mqx_uint _lpm_unregister_driver 
    ( 
        /* [IN] Driver registration handle returned by LPM registration function */
        _mqx_uint               registration_handle
    )
{
    LPM_DRIVER_ENTRY_STRUCT_PTR entry_ptr;
    _mqx_uint                   result;
    
    /* Check parameters */
    if (0 == registration_handle)
    {
        return MQX_INVALID_PARAMETER;
    }
    
    /* Check if LPM installed and lock */
    result = _lwsem_wait (&(lpm_state_struct.SEMAPHORE));
    if (MQX_OK != result)
    {
        return MQX_IO_OPERATION_NOT_AVAILABLE;
    }
    
    /* Find driver registration record according to given handle and remove it from the list */
    for (entry_ptr = lpm_state_struct.DRIVER_ENTRIES; NULL != entry_ptr; entry_ptr = entry_ptr->NEXT)
    {
        if (registration_handle == entry_ptr->ID)
        {
            if (NULL == entry_ptr->PREVIOUS)
            {
                if (NULL != entry_ptr->NEXT)
                {
                    entry_ptr->NEXT->PREVIOUS = NULL;
                }
                lpm_state_struct.DRIVER_ENTRIES = entry_ptr->NEXT;
            }
            else
            {
                if (NULL != entry_ptr->NEXT)
                {
                    entry_ptr->NEXT->PREVIOUS = entry_ptr->PREVIOUS;
                }
                entry_ptr->PREVIOUS->NEXT = entry_ptr->NEXT;
            }
            break;
        }
    }
    if (NULL == entry_ptr)
    {
        _lwsem_post (&(lpm_state_struct.SEMAPHORE));
        return MQX_INVALID_HANDLE;
    }
    
    /* Release the record found */
    _mem_free (entry_ptr);
    
    /* Unlock LPM */
    _lwsem_post (&(lpm_state_struct.SEMAPHORE));
    
    return MQX_OK;    
}


/*FUNCTION*------------------------------------------------------------------
* 
* Function Name    : _lpm_set_clock_configuration
* Returned Value   : MQX error code
* Comments         :
*    This function changes low power clock configuration.
*
*END*----------------------------------------------------------------------*/

_mqx_uint _lpm_set_clock_configuration
    ( 
        /* [IN] Low power clock configuration identifier */
        BSP_CLOCK_CONFIGURATION clock_configuration
    )
{
    LPM_DRIVER_ENTRY_STRUCT_PTR entry_ptr;
    LPM_NOTIFICATION_STRUCT     notification;
    _mqx_uint                   result;
    
    /* Check parameters */
    if (BSP_CLOCK_CONFIGURATIONS <= (_mqx_uint)clock_configuration)
    {
        return MQX_INVALID_PARAMETER;
    }
    
    /* Check if LPM installed and lock */
    result = _lwsem_wait (&(lpm_state_struct.SEMAPHORE));
    if (MQX_OK != result)
    {
        return MQX_IO_OPERATION_NOT_AVAILABLE;
    }
    
    /* Phase 1 - send pre-notifications before clock configuration change */
    for (entry_ptr = lpm_state_struct.DRIVER_ENTRIES; NULL != entry_ptr; entry_ptr = entry_ptr->NEXT)
    {
        if (NULL != entry_ptr->REGISTRATION.CLOCK_CONFIGURATION_CALLBACK)
        {
            notification.NOTIFICATION_TYPE = LPM_NOTIFICATION_TYPE_PRE;
            notification.CLOCK_CONFIGURATION = clock_configuration;
            notification.OPERATION_MODE = lpm_state_struct.OPERATION_MODE;
            if (LPM_NOTIFICATION_RESULT_OK != ((*(entry_ptr->REGISTRATION.CLOCK_CONFIGURATION_CALLBACK)) (&notification, entry_ptr->DATA)))
            {
                result = MQX_IO_OPERATION_NOT_AVAILABLE;
                entry_ptr = entry_ptr->PREVIOUS;
                break;
            }
        }
        if (NULL == entry_ptr->NEXT)
        {
            break;
        }
    }

    /* Phase 2 - change clock configuration if all pre-notifications passed */
    if (MQX_OK == result)
    {
        result = _cm_set_clock_configuration (clock_configuration);
    }

    /* Phase 3 - send post-notifications after clock configuration change or failure */
    clock_configuration = _cm_get_clock_configuration();
    for ( ; NULL != entry_ptr; entry_ptr = entry_ptr->PREVIOUS)
    {
        if (NULL != entry_ptr->REGISTRATION.CLOCK_CONFIGURATION_CALLBACK)
        {
            notification.NOTIFICATION_TYPE = LPM_NOTIFICATION_TYPE_POST;
            notification.CLOCK_CONFIGURATION = clock_configuration;
            notification.OPERATION_MODE = lpm_state_struct.OPERATION_MODE;
            (*(entry_ptr->REGISTRATION.CLOCK_CONFIGURATION_CALLBACK)) (&notification, entry_ptr->DATA);
        }
    }

    /* Unlock LPM */
    _lwsem_post (&(lpm_state_struct.SEMAPHORE));

    return result;    
}


/*FUNCTION*------------------------------------------------------------------
* 
* Function Name    : _lpm_get_clock_configuration 
* Returned Value   : Current low power clock configuration identifier
* Comments         :
*    This function returns current low power clock configuration identifier.
*
*END*----------------------------------------------------------------------*/

BSP_CLOCK_CONFIGURATION _lpm_get_clock_configuration 
    ( 
    )
{
    BSP_CLOCK_CONFIGURATION clock_configuration;
    _mqx_uint               result;
    
    /* Check if LPM installed and lock */
    result = _lwsem_wait (&(lpm_state_struct.SEMAPHORE));
    if (MQX_OK != result)
    {
        return (BSP_CLOCK_CONFIGURATION)-1;
    }
    
    /* Get current clock configuration */
    clock_configuration = _cm_get_clock_configuration();

    /* Unlock LPM */
    _lwsem_post (&(lpm_state_struct.SEMAPHORE));

    return clock_configuration;
}


/*FUNCTION*------------------------------------------------------------------
* 
* Function Name    : _lpm_set_operation_mode 
* Returned Value   : MQX error code
* Comments         :
*    This function changes low power operation mode. It may also restart idle task.
*
*END*----------------------------------------------------------------------*/

_mqx_uint _lpm_set_operation_mode 
    ( 
        /* [IN] Low power operation mode identifier */
        LPM_OPERATION_MODE      operation_mode
    )
{
    LPM_DRIVER_ENTRY_STRUCT_PTR entry_ptr;
    LPM_NOTIFICATION_STRUCT     notification;
    BSP_CLOCK_CONFIGURATION     clock_configuration;
    _mqx_uint                   result;
    
    /* Check parameters */
    if (LPM_OPERATION_MODES <= (_mqx_uint)operation_mode)
    {
        return MQX_INVALID_PARAMETER;
    }
    
    /* Check if LPM installed and lock */
    result = _lwsem_wait (&(lpm_state_struct.SEMAPHORE));
    if (MQX_OK != result)
    {
        return MQX_IO_OPERATION_NOT_AVAILABLE;
    }
    
    /* Phase 1 - send pre-notifications before operation mode change */
    clock_configuration = _cm_get_clock_configuration();
    for (entry_ptr = lpm_state_struct.DRIVER_ENTRIES; NULL != entry_ptr; entry_ptr = entry_ptr->NEXT)
    {
        if (NULL != entry_ptr->REGISTRATION.OPERATION_MODE_CALLBACK)
        {
            notification.NOTIFICATION_TYPE = LPM_NOTIFICATION_TYPE_PRE;
            notification.CLOCK_CONFIGURATION = clock_configuration;
            notification.OPERATION_MODE = operation_mode;
            if (LPM_NOTIFICATION_RESULT_OK != ((*(entry_ptr->REGISTRATION.OPERATION_MODE_CALLBACK)) (&notification, entry_ptr->DATA)))
            {
                result = MQX_IO_OPERATION_NOT_AVAILABLE;
                entry_ptr = entry_ptr->PREVIOUS;
                break;
            }
        }
        if (NULL == entry_ptr->NEXT)
        {
            break;
        }
    }

    /* Phase 2 - change CPU operation mode if all pre-notifications passed */
    if (MQX_OK == result)
    {
        result =_lpm_set_cpu_operation_mode (lpm_state_struct.CPU_OPERATION_MODES, operation_mode);
        if (MQX_OK == result)
        {
            lpm_state_struct.OPERATION_MODE = operation_mode;
            _lpm_restart_idle_task (lpm_state_struct.OPERATION_MODE, lpm_state_struct.IDLE_SLEEP);
        }
    }
    
    /* Roll back in case of failure */
    if (MQX_OK != result)
    {
        for ( ; NULL != entry_ptr; entry_ptr = entry_ptr->PREVIOUS)
        {
            if (NULL != entry_ptr->REGISTRATION.OPERATION_MODE_CALLBACK)
            {
                notification.NOTIFICATION_TYPE = LPM_NOTIFICATION_TYPE_PRE;
                notification.CLOCK_CONFIGURATION = clock_configuration;
                notification.OPERATION_MODE = lpm_state_struct.OPERATION_MODE;
                (*(entry_ptr->REGISTRATION.OPERATION_MODE_CALLBACK)) (&notification, entry_ptr->DATA);
            }
        }
    }

    /* Unlock LPM */
    _lwsem_post (&(lpm_state_struct.SEMAPHORE));
    
    return result;    
}


/*FUNCTION*------------------------------------------------------------------
* 
* Function Name    : _lpm_get_operation_mode 
* Returned Value   : Current low power operation mode identifier
* Comments         :
*    This function returns current low power operation mode identifier.
*
*END*----------------------------------------------------------------------*/

LPM_OPERATION_MODE _lpm_get_operation_mode 
    ( 
    )
{
    LPM_OPERATION_MODE operation_mode;
    _mqx_uint          result;
    
    /* Check if LPM installed and lock */
    result = _lwsem_wait (&(lpm_state_struct.SEMAPHORE));
    if (MQX_OK != result)
    {
        return (LPM_OPERATION_MODE)-1;
    }
    
    /* Get current operation mode */
    operation_mode = lpm_state_struct.OPERATION_MODE;

    /* Unlock LPM */
    _lwsem_post (&(lpm_state_struct.SEMAPHORE));
    
    return operation_mode;    
}


/*FUNCTION*------------------------------------------------------------------
* 
* Function Name    : _lpm_idle_sleep_setup 
* Returned Value   : MQX error code
* Comments         :
*    This function turns on/off the idle sleep feature. It may restart idle task.
*
*END*----------------------------------------------------------------------*/

_mqx_uint _lpm_idle_sleep_setup 
    (
        /* [IN] Whether to turn on/off the idle sleep feature */
        bool idle_sleep
    )
{
    _mqx_uint   result;
    
    /* Check if LPM installed and lock */
    result = _lwsem_wait (&(lpm_state_struct.SEMAPHORE));
    if (MQX_OK != result)
    {
        return MQX_IO_OPERATION_NOT_AVAILABLE;
    }
    
    /* Change idle sleep settings and optionally restart idle task */
    if (idle_sleep != lpm_state_struct.IDLE_SLEEP)
    {
        result = _lpm_restart_idle_task (lpm_state_struct.OPERATION_MODE, idle_sleep);
        if (MQX_OK == result)
        {
            lpm_state_struct.IDLE_SLEEP = idle_sleep;
        }
    }

    /* Unlock LPM */
    _lwsem_post (&(lpm_state_struct.SEMAPHORE));

    return result;
}


/*FUNCTION*------------------------------------------------------------------
* 
* Function Name    : _lpm_register_wakeup_callback 
* Returned Value   : MQX error code
* Comments         :
*    This function is used to install ISR for LLUW interrupt and register
* user wakeup callback function
*
*END*----------------------------------------------------------------------*/

void _lpm_register_wakeup_callback
    (
        /* [IN] LLWU vector */
        uint32_t llwu_vector,

        /* [IN] Priority of LLUW interrupt */
        uint8_t llwu_prior,

        /* [IN] LLUW user wakeupp callback*/
        void (* llwu_user_wakeup_callback)(uint32_t)
    )
{
    /* Enable LLWU interrupt on LLS recovery */
    if (lpm_state_struct.CPU_OPERATION_MODES[LPM_OPERATION_MODE_STOP].MODE_INDEX == LPM_CPU_POWER_MODE_LLS)
    {
        _int_install_isr(llwu_vector, _lpm_llwu_isr, NULL);
        _bsp_int_init(llwu_vector, llwu_prior, 0, TRUE);
        lpm_state_struct.LLWU_WAKEUP_CALLBACK = llwu_user_wakeup_callback;
    }
}


/*FUNCTION*------------------------------------------------------------------
* 
* Function Name    : _lpm_unregister_wakeup_callback 
* Returned Value   : MQX error code
* Comments         :
*    This function is used to uninstall ISR for LLUW interrupt and unregister
* user wakeup callback function
*
*END*----------------------------------------------------------------------*/

void _lpm_unregister_wakeup_callback
    (
        /* [IN] LLWU vector */
        uint32_t llwu_vector
    )
{
    /* Enable LLWU interrupt on LLS recovery */
    if (lpm_state_struct.CPU_OPERATION_MODES[LPM_OPERATION_MODE_STOP].MODE_INDEX == LPM_CPU_POWER_MODE_LLS)
    {
        _int_install_isr(llwu_vector, _int_get_default_isr(), NULL);
        _bsp_int_disable(llwu_vector);
        lpm_state_struct.LLWU_WAKEUP_CALLBACK = NULL;
    }
}


/*FUNCTION*------------------------------------------------------------------
*
* Function Name    : _lpm_llwu_isr
* Returned Value   : Void
* Comments         :
*    This function is ISR function for LLWU interrupt
*
*END*----------------------------------------------------------------------*/

void _lpm_llwu_isr
    (
        /* [IN] LLWU parameter pointer */
        void *llwu_param_ptr
    )
{
    uint32_t llwu_fx;

    _lpm_llwu_clear_flag(&llwu_fx);

    if (lpm_state_struct.LLWU_WAKEUP_CALLBACK != NULL) {
        lpm_state_struct.LLWU_WAKEUP_CALLBACK(llwu_fx); 
    }
}


#endif


/* EOF */
