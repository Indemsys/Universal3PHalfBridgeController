
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
* See license agreement file for full license terms including other restrictions.
*****************************************************************************
*
* Comments:
*
*   This file contains functions of the Watchdog component.
*
*
*END************************************************************************/

#include "mqx_inc.h"
#if MQX_USE_SW_WATCHDOGS
#include "watchdog.h"
#include "watchdog_prv.h"

/*!
 * \brief Creates the watchdog component.
 *
 * An application must explicitly create the watchdog component before tasks can
 * use watchdogs.
 * \n The function returns MQX_OK if either:
 * \li Watchdog component is created.
 * \li Watchdog component was previously created and the configuration is not
 * changed.
 *
 * \param[in] timer_interrupt_vector Periodic timer interrupt vector number.
 * \param[in] error_function         Function that MQX calls when a watchdog
 * expires.
 *
 * \return MQX_OK
 * \return MQX_CANNOT_CALL_FUNCTION_FROM_ISR (Function cannot be called from an ISR.)
 * \return WATCHDOG_INVALID_ERROR_FUNCTION (Error_function is NULL.)
 * \return WATCHDOG_INVALID_INTERRUPT_VECTOR (MQX cannot install the periodic
 * timer interrupt vector.)
 * \return MQX_OUT_OF_MEMORY (MQX cannot allocate memory for watchdog component
 * data.)
 *
 * \warning Cannot be called from an ISR.
 *
 * \see _watchdog_start
 * \see _watchdog_start_ticks
 * \see _watchdog_stop
 */
_mqx_uint _watchdog_create_component
(
    _mqx_uint           timer_interrupt_vector,
    WATCHDOG_ERROR_FPTR error_function
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR        kernel_data;
    WATCHDOG_COMPONENT_STRUCT_PTR watchdog_component_ptr;
    void                         *interrupt_data;

    _GET_KERNEL_DATA(kernel_data);

    _KLOGE3(KLOG_watchdog_create_component, timer_interrupt_vector, error_function);

#if MQX_CHECK_ERRORS
    if (kernel_data->IN_ISR)
    {
        _KLOGX2(KLOG_watchdog_create_component, MQX_CANNOT_CALL_FUNCTION_FROM_ISR);
        return(MQX_CANNOT_CALL_FUNCTION_FROM_ISR);
    } /* Endif */
#endif /* MQX_CHECK_ERRORS */

    _lwsem_wait((LWSEM_STRUCT_PTR)(&kernel_data->COMPONENT_CREATE_LWSEM));
    if (kernel_data->KERNEL_COMPONENTS[KERNEL_WATCHDOG] != NULL)
    {
        _lwsem_post((LWSEM_STRUCT_PTR)(&kernel_data->COMPONENT_CREATE_LWSEM));
        _KLOGX2(KLOG_watchdog_create_component, MQX_OK);
        return(MQX_OK);
    } /* Endif */

#if MQX_CHECK_ERRORS
    if (!error_function)
    {
        _lwsem_post((LWSEM_STRUCT_PTR)(&kernel_data->COMPONENT_CREATE_LWSEM));
        _KLOGX2(KLOG_watchdog_create_component, WATCHDOG_INVALID_ERROR_FUNCTION);
        return(WATCHDOG_INVALID_ERROR_FUNCTION);
    } /* Endif */
    if ((timer_interrupt_vector < kernel_data->FIRST_USER_ISR_VECTOR) ||
                    (timer_interrupt_vector > kernel_data->LAST_USER_ISR_VECTOR))
    {
        _lwsem_post((LWSEM_STRUCT_PTR)(&kernel_data->COMPONENT_CREATE_LWSEM));
        _KLOGX2(KLOG_watchdog_create_component, WATCHDOG_INVALID_INTERRUPT_VECTOR);
        return(WATCHDOG_INVALID_INTERRUPT_VECTOR);
    } /* Endif */
#endif /* MQX_CHECK_ERRORS */

    /* Get the watchdog component data structure */
    watchdog_component_ptr = (WATCHDOG_COMPONENT_STRUCT_PTR)
    _mem_alloc_system_zero((_mem_size)sizeof(WATCHDOG_COMPONENT_STRUCT));
#if MQX_CHECK_MEMORY_ALLOCATION_ERRORS
    if (watchdog_component_ptr == NULL)
    {
        _lwsem_post((LWSEM_STRUCT_PTR)(&kernel_data->COMPONENT_CREATE_LWSEM));
        _KLOGX2(KLOG_watchdog_create_component, MQX_OUT_OF_MEMORY);
        return(MQX_OUT_OF_MEMORY);
    } /* Endif */
#endif /* MQX_CHECK_MEMORY_ALLOCATION_ERRORS */
    _mem_set_type(watchdog_component_ptr, MEM_TYPE_WATCHDOG_COMPONENT);

    watchdog_component_ptr->ERROR_FUNCTION = error_function;
    watchdog_component_ptr->VALID = WATCHDOG_VALID;
    watchdog_component_ptr->INTERRUPT_VECTOR = timer_interrupt_vector;

    interrupt_data = _int_get_isr_data(timer_interrupt_vector);
    _INT_DISABLE();
    watchdog_component_ptr->TIMER_INTERRUPT_HANDLER = _int_install_isr(
                    timer_interrupt_vector, _watchdog_isr, interrupt_data);
#if MQX_CHECK_ERRORS
    if (!watchdog_component_ptr->TIMER_INTERRUPT_HANDLER)
    {
        _int_enable();
        _lwsem_post((LWSEM_STRUCT_PTR)(&kernel_data->COMPONENT_CREATE_LWSEM));
        _mem_free(watchdog_component_ptr);
        _KLOGX2(KLOG_watchdog_create_component, WATCHDOG_INVALID_INTERRUPT_VECTOR);
        return(WATCHDOG_INVALID_INTERRUPT_VECTOR);
    } /* Endif */
#endif /* MQX_CHECK_ERRORS */

    kernel_data->KERNEL_COMPONENTS[KERNEL_WATCHDOG] = watchdog_component_ptr;
#if MQX_COMPONENT_DESTRUCTION
    kernel_data->COMPONENT_CLEANUP[KERNEL_WATCHDOG] = _watchdog_cleanup;
#endif /* MQX_COMPONENT_DESTRUCTION */
    _INT_ENABLE();
    _lwsem_post((LWSEM_STRUCT_PTR)(&kernel_data->COMPONENT_CREATE_LWSEM));

    _KLOGX2(KLOG_watchdog_create_component, MQX_OK);

    return(MQX_OK);

} /* Endbody */

/*!
 * \brief This function takes over from the timer interrupt handler in order to
 * process watchdog timeouts.
 *
 * \param[in] parameter The parameter passed by the kernel.
 */
void _watchdog_isr
(
    void   *parameter
)
{ /* Body */
    MQX_TICK_STRUCT                 kd_time;
    KERNEL_DATA_STRUCT_PTR          kernel_data;
    TD_STRUCT_PTR                   td_ptr;
    WATCHDOG_COMPONENT_STRUCT_PTR   watchdog_component_ptr;
    WATCHDOG_ALARM_TABLE_STRUCT_PTR table_ptr;
    _mqx_uint                       i;
    _mqx_int                        result;

    _GET_KERNEL_DATA(kernel_data);
    watchdog_component_ptr = (WATCHDOG_COMPONENT_STRUCT_PTR)
    kernel_data->KERNEL_COMPONENTS[KERNEL_WATCHDOG];

    /* execute standard timer routine (in case of BSP timer, the
     _time_notify_kernel() may get invoked now) */
    (*watchdog_component_ptr->TIMER_INTERRUPT_HANDLER)(parameter);

#if MQX_CHECK_ERRORS
    if (watchdog_component_ptr == NULL)
    {
        (*kernel_data->DEFAULT_ISR)((void *)kernel_data->SYSTEM_CLOCK_INT_NUMBER);
    } /* Endif */
#endif /* MQX_CHECK_ERRORS */
#if MQX_CHECK_VALIDITY
    if (watchdog_component_ptr->VALID != WATCHDOG_VALID)
    {
        (*kernel_data->DEFAULT_ISR)((void *)kernel_data->SYSTEM_CLOCK_INT_NUMBER);
    } /* Endif */
#endif /* MQX_CHECK_VALIDITY */

    table_ptr = &watchdog_component_ptr->ALARM_ENTRIES;
    while (table_ptr != NULL)
    {
        for (i = 0; i < WATCHDOG_TABLE_SIZE; ++i)
        {
            td_ptr = table_ptr->TD_PTRS[i];
            if (td_ptr != NULL)
            {
#if MQX_TASK_DESTRUCTION
                if (! (td_ptr->FLAGS & TASK_WATCHDOG_STARTED))
                {
                    /* The td has been destroyed and re-created */
                    table_ptr->TD_PTRS[i] = NULL;
                }
                else if (td_ptr->TASK_ID == 0)
                {
                    /* The task has been destroyed */
                    table_ptr->TD_PTRS[i] = NULL;
                }
                else
#endif /* MQX_TASK_DESTRUCTION */
                if (td_ptr->FLAGS & TASK_WATCHDOG_RUNNING)
                {
                    /* An enabled watchdog */

                    /* A timer interrupt is about to fire so increment kernel time */
                    kd_time = kernel_data->TIME;
                    result = PSP_CMP_TICKS(&kd_time, &td_ptr->WATCHDOG_TIME);
                    if (result >= 0)
                    {
                        /* EXPIRED */
                        td_ptr->FLAGS &= ~TASK_WATCHDOG_RUNNING;
                        (*watchdog_component_ptr->ERROR_FUNCTION)(td_ptr);
                    } /* Endif */
                } /* Endif */
            } /* Endif */
        } /* Endfor */
        table_ptr = table_ptr->NEXT_TABLE_PTR;

    } /* Endwhile */

} /* Endbody */

/*!
 * \brief Starts or restart the watchdog.
 *
 * The function returns FALSE if either of these conditions is true:
 * \li Watchdog component was not previously created.
 * \li Watchdog component data is no longer valid.
 *
 * \param[in] time Time in milliseconds until the watchdog expires.
 *
 * \return TRUE (success) or FALSE (Failure: see Description.)
 *
 * \see _watchdog_start_ticks
 * \see _time_to_ticks
 * \see _usr_lwevent_clear
 * \see _watchdog_stop
 * \see MQX_TICK_STRUCT
 */
bool _watchdog_start
(
    uint32_t time
)
{
    _KLOGM(KERNEL_DATA_STRUCT_PTR kernel_data);
    bool         result;
    MQX_TICK_STRUCT start_time;

    _KLOGM(_GET_KERNEL_DATA(kernel_data));

    _KLOGE2(KLOG_watchdog_start, time);

    if(time)
    {
        /* Compute the number of tick events required to accomplish the least amount of time[ms]. */
        /* tick_events = (required_time[ms] + (time_per_tick[ms] - 1)) / time_per_tick[ms]) + 1  -->
         * tick_events = ((required_time[ms] - 1) / time_per_tick[ms]) + 2
         */
        time--;
        /* Convert milliseconds to ticks, truncated */
        PSP_MILLISECONDS_TO_TICKS_QUICK(time, &start_time);
        /* Resolve truncation by adding one tick. Add another tick to accomplish the requested amount of time. */
        PSP_ADD_TICKS_TO_TICK_STRUCT(&start_time, 2, &start_time);
    }
    else
    {
        PSP_MILLISECONDS_TO_TICKS_QUICK(time, &start_time);
    }

    result = _watchdog_start_internal((MQX_TICK_STRUCT_PTR)&start_time);

    _KLOGX2(KLOG_watchdog_start, result);

    return(result);

} /* Endbody */

/*!
 * \brief Stops the watchdog.
 *
 * The function returns FALSE if any of these conditions is true:
 * \li Watchdog component was not previously created.
 * \li Watchdog component data is no longer valid.
 * \li Watchdog was not started.
 *
 * \return TRUE (success) or FALSE (Failure: see Description.)
 *
 * \see _usr_lwevent_clear
 * \see _watchdog_start
 * \see _watchdog_start_ticks
 */
bool _watchdog_stop(void)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    TD_STRUCT_PTR          td_ptr;

    _GET_KERNEL_DATA(kernel_data);

    _KLOGE1(KLOG_watchdog_stop);

    td_ptr = kernel_data->ACTIVE_PTR;
    if (td_ptr->FLAGS & TASK_WATCHDOG_STARTED)
    {
        _INT_DISABLE();
        /* td_ptr->FLAGS &= ~(TASK_WATCHDOG_STARTED | TASK_WATCHDOG_RUNNING); */
        td_ptr->FLAGS &= ~TASK_WATCHDOG_RUNNING;
        _INT_ENABLE();
        _KLOGX2(KLOG_watchdog_stop, TRUE);
        return(TRUE);
    } /* Endif */

    _KLOGX2(KLOG_watchdog_stop, FALSE);
    return(FALSE);

} /* Endbody */

/*!
 * \private
 *
 * \brief This function starts a software watchdog for the current task.
 *
 * The timer will expire at the specified number of ticks in the future.
 *
 * \param[in] tick_ptr The time in ticks at which the watchdog expires.
 *
 * \return TRUE (success) or FALSE (Failure: see Description.)
 */
bool _watchdog_start_internal
(
    MQX_TICK_STRUCT_PTR tick_ptr
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR          kernel_data;
    TD_STRUCT_PTR                   td_ptr;
    WATCHDOG_COMPONENT_STRUCT_PTR   watchdog_component_ptr;
    WATCHDOG_ALARM_TABLE_STRUCT_PTR table_ptr;
    MQX_TICK_STRUCT                 kd_time;
    _mqx_uint                       i;

    _GET_KERNEL_DATA(kernel_data);

    td_ptr = kernel_data->ACTIVE_PTR;

    kd_time = kernel_data->TIME;

    if (td_ptr->FLAGS & TASK_WATCHDOG_STARTED)
    {
        _INT_DISABLE();
        PSP_ADD_TICKS(tick_ptr, &kd_time, &td_ptr->WATCHDOG_TIME);
        td_ptr->FLAGS |= TASK_WATCHDOG_RUNNING;
        _INT_ENABLE();
        return(TRUE);
    } /* Endif */

    watchdog_component_ptr = (WATCHDOG_COMPONENT_STRUCT_PTR)
    kernel_data->KERNEL_COMPONENTS[KERNEL_WATCHDOG];
#if MQX_CHECK_ERRORS
    if (watchdog_component_ptr == NULL)
    {
        return(FALSE);
    } /* Endif */
#endif /* MQX_CHECK_ERRORS */
#if MQX_CHECK_VALIDITY
    if (watchdog_component_ptr->VALID != WATCHDOG_VALID)
    {
        return(FALSE);
    } /* Endif */
#endif /* MQX_CHECK_VALIDITY */

    table_ptr = &watchdog_component_ptr->ALARM_ENTRIES;

    _INT_DISABLE();
    while (table_ptr != NULL)
    {
        for (i = 0; i < WATCHDOG_TABLE_SIZE; ++i)
        {
            if (table_ptr->TD_PTRS[i] == NULL)
            {
                table_ptr->TD_PTRS[i] = td_ptr;
                PSP_ADD_TICKS(tick_ptr, &kd_time, &td_ptr->WATCHDOG_TIME);
                td_ptr->FLAGS |= TASK_WATCHDOG_STARTED | TASK_WATCHDOG_RUNNING;
                _INT_ENABLE();
                return(TRUE);
            } /* Endif */
        } /* Endwhile */
        if (table_ptr->NEXT_TABLE_PTR == NULL)
        {
            table_ptr->NEXT_TABLE_PTR = (WATCHDOG_ALARM_TABLE_STRUCT_PTR)
            _mem_alloc_system_zero((_mem_size)sizeof(WATCHDOG_ALARM_TABLE_STRUCT));
            _mem_set_type(table_ptr->NEXT_TABLE_PTR, MEM_TYPE_WATCHDOG_ALARM);
        } /* Endif */
        table_ptr = table_ptr->NEXT_TABLE_PTR;
    } /* Endwhile */

    _int_enable();
    return(FALSE);

} /* Endbody */

/*!
 * \brief Starts or restart the watchdog.
 *
 * The function returns FALSE if either of these conditions is true:
 * \li Watchdog component was not previously created.
 * \li Watchdog component data is no longer valid.
 *
 * \param[in] tick_ptr Pointer to the number of ticks until the watchdog expires.
 *
 * \return TRUE (success) or FALSE (Failure: see Description.)
 *
 * \see _watchdog_start
 * \see _time_to_ticks
 * \see _usr_lwevent_clear
 * \see _watchdog_stop
 * \see MQX_TICK_STRUCT
 */
bool _watchdog_start_ticks
(
    MQX_TICK_STRUCT_PTR tick_ptr
)
{ /* Body */
    _KLOGM(KERNEL_DATA_STRUCT_PTR kernel_data);
    bool result;

    _KLOGM(_GET_KERNEL_DATA(kernel_data));

    _KLOGE2(KLOG_watchdog_start_ticks, tick_ptr);

    result = _watchdog_start_internal(tick_ptr);

    _KLOGX2(KLOG_watchdog_start_ticks, result);

    return(result);

} /* Endbody */
/*!
 * \brief Used during task destruction to clean up any watchdogs started by this
 * task.
 *
 * \param[in] td_ptr The task being destroyed.
 */
void _watchdog_cleanup
(
    TD_STRUCT_PTR td_ptr
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR          kernel_data;
    WATCHDOG_COMPONENT_STRUCT_PTR   watchdog_component_ptr;
    WATCHDOG_ALARM_TABLE_STRUCT_PTR table_ptr;
    _mqx_uint                       i;

    _GET_KERNEL_DATA(kernel_data);

    watchdog_component_ptr = (WATCHDOG_COMPONENT_STRUCT_PTR)
    kernel_data->KERNEL_COMPONENTS[KERNEL_WATCHDOG];
    if (watchdog_component_ptr == NULL)
    {
        return; /* No work to do */
    } /* Endif */

#if MQX_CHECK_VALIDITY
    if (watchdog_component_ptr->VALID != WATCHDOG_VALID)
    {
        return;
    } /* Endif */
#endif /* MQX_CHECK_VALIDITY */

    table_ptr = &watchdog_component_ptr->ALARM_ENTRIES;

    _INT_DISABLE();
    while (table_ptr != NULL)
    {
        for (i = 0; i < WATCHDOG_TABLE_SIZE; ++i)
        {
            if (table_ptr->TD_PTRS[i] == td_ptr)
            {
                table_ptr->TD_PTRS[i] = NULL;
                td_ptr->FLAGS &= ~(TASK_WATCHDOG_STARTED | TASK_WATCHDOG_RUNNING);
                _INT_ENABLE();
                return;
            } /* Endif */
        } /* Endfor */
        table_ptr = table_ptr->NEXT_TABLE_PTR;
    } /* Endwhile */
    _INT_ENABLE();

} /* Endbody */

/*!
 * \brief Tests the watchdog component data for consistency and validity.
 *
 * The function returns MQX_OK if either:
 * \li It did not find an error in watchdog component data.
 * \li Watchdog component was not previously created.
 *
 * \param[out] watchdog_error_ptr       Pointer to the watchdog component base
 * that has an error (NULL if no errors are found).
 * \param[out] watchdog_table_error_ptr Pointer to the watchdog table that has
 * an error (always NULL).
 *
 * \return MQX_OK (see Description)
 * \return MQX_INVALID_COMPONENT_BASE (An error was found.)
 *
 * \see _usr_lwevent_clear
 * \see _watchdog_start
 * \see _watchdog_start_ticks
 * \see _watchdog_stop
 */
_mqx_uint _watchdog_test
(
    void    **watchdog_error_ptr,
    void    **watchdog_table_error_ptr
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR        kernel_data;
    WATCHDOG_COMPONENT_STRUCT_PTR watchdog_component_ptr;

    _GET_KERNEL_DATA(kernel_data);

    _KLOGE3(KLOG_watchdog_test, watchdog_error_ptr, watchdog_table_error_ptr);

    *watchdog_error_ptr = NULL;
    *watchdog_table_error_ptr = NULL;

    watchdog_component_ptr = (WATCHDOG_COMPONENT_STRUCT_PTR)
    kernel_data->KERNEL_COMPONENTS[KERNEL_WATCHDOG];
    if (watchdog_component_ptr == NULL)
    {
        _KLOGX2(KLOG_watchdog_test, MQX_OK);
        return(MQX_OK);
    } /* Endif */

    *watchdog_error_ptr = watchdog_component_ptr;
    if (watchdog_component_ptr->VALID != WATCHDOG_VALID)
    {
        _KLOGX2(KLOG_watchdog_test, MQX_INVALID_COMPONENT_BASE);
        return(MQX_INVALID_COMPONENT_BASE);
    } /* Endif */

    _KLOGX2(KLOG_watchdog_test, MQX_OK);
    return(MQX_OK);

} /* Endbody */

#endif /* MQX_USE_SW_WATCHDOGS */

/* EOF */
