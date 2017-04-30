
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
*
*END************************************************************************/

#include "mqx_inc.h"

#include "timer.h"
#include "timer_prv.h"

const int32_t  _timezone = 0;

const uint16_t _time_days_in_month_internal[2][13] =
{
  { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
  { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};


#define NUM_LEAP_YEAR_SINCE_FIRST_YEAR  (CLK_FIRST_YEAR/4 - CLK_FIRST_YEAR/100 + CLK_FIRST_YEAR/400)
static bool normalize(int16_t* , int16_t, int16_t*);

/*!
 * \brief Add time in days to tick time struct.
 *
 * The functions can also be used in conjunction with the global constant
 * _mqx_zero_tick_struct to convert units to tick time.
 *
 * \param[in] tick_ptr Tick time to add to.
 * \param[in] days     The number of days to add.
 *
 * \return Tick time struct.
 *
 * \see _time_add_hour_to_ticks
 * \see _time_add_min_to_ticks
 * \see _time_add_sec_to_ticks
 * \see _time_add_msec_to_ticks
 * \see _time_add_usec_to_ticks
 * \see _time_add_nsec_to_ticks
 * \see _time_add_psec_to_ticks
 * \see _mqx_zero_tick_struct
 */
MQX_TICK_STRUCT_PTR _time_add_day_to_ticks
(
    MQX_TICK_STRUCT_PTR tick_ptr,
    _mqx_uint           days
)
{ /* Body */
    MQX_TICK_STRUCT tmp;

    if (days) {
        PSP_DAYS_TO_TICKS(days, &tmp);
        PSP_ADD_TICKS(tick_ptr, &tmp, tick_ptr);
    } /* Endif */

    return tick_ptr;

} /* Endbody */

/*!
 * \brief Add time in hours to tick time struct.
 *
 * The functions can also be used in conjunction with the global constant
 * _mqx_zero_tick_struct to convert units to tick time.
 *
 * \param[in] tick_ptr Tick time to add to.
 * \param[in] hours    The number of hours to add.
 *
 * \return Tick time struct.
 *
 * \see _time_add_day_to_ticks
 * \see _time_add_min_to_ticks
 * \see _time_add_sec_to_ticks
 * \see _time_add_msec_to_ticks
 * \see _time_add_usec_to_ticks
 * \see _time_add_nsec_to_ticks
 * \see _time_add_psec_to_ticks
 * \see _mqx_zero_tick_struct
 */
MQX_TICK_STRUCT_PTR _time_add_hour_to_ticks
(
    MQX_TICK_STRUCT_PTR tick_ptr,
    _mqx_uint           hours
)
{ /* Body */
    MQX_TICK_STRUCT tmp;

    if (hours) {
        PSP_HOURS_TO_TICKS(hours, &tmp);
        PSP_ADD_TICKS(tick_ptr, &tmp, tick_ptr);
    } /* Endif */

    return tick_ptr;

} /* Endbody */

/*!
 * \brief Add time in minutes to tick time struct.
 *
 * The functions can also be used in conjunction with the global constant
 * _mqx_zero_tick_struct to convert units to tick time.
 *
 * \param[in] tick_ptr Tick time to add to.
 * \param[in] minutes  The number of minutes to add.
 *
 * \return Tick time struct.
 *
 * \see _time_add_day_to_ticks
 * \see _time_add_hour_to_ticks
 * \see _time_add_sec_to_ticks
 * \see _time_add_msec_to_ticks
 * \see _time_add_usec_to_ticks
 * \see _time_add_nsec_to_ticks
 * \see _time_add_psec_to_ticks
 * \see _mqx_zero_tick_struct
 */
MQX_TICK_STRUCT_PTR _time_add_min_to_ticks
(
    MQX_TICK_STRUCT_PTR tick_ptr,
    _mqx_uint           minutes
)
{ /* Body */
    MQX_TICK_STRUCT tmp;

    if (minutes) {
        PSP_MINUTES_TO_TICKS(minutes, &tmp);
        PSP_ADD_TICKS(tick_ptr, &tmp, tick_ptr);
    } /* Endif */

    return tick_ptr;

} /* Endbody */

/*!
 * \brief Add time in milliseconds to tick time struct.
 *
 * The functions can also be used in conjunction with the global constant
 * _mqx_zero_tick_struct to convert units to tick time.
 *
 * \param[in] tick_ptr Tick time to add to.
 * \param[in] msecs    The number of milliseconds to add.
 *
 * \return Tick time struct.
 *
 * \see _time_add_day_to_ticks
 * \see _time_add_hour_to_ticks
 * \see _time_add_min_to_ticks
 * \see _time_add_sec_to_ticks
 * \see _time_add_usec_to_ticks
 * \see _time_add_nsec_to_ticks
 * \see _time_add_psec_to_ticks
 * \see _mqx_zero_tick_struct
 */
MQX_TICK_STRUCT_PTR _time_add_msec_to_ticks
(
    MQX_TICK_STRUCT_PTR tick_ptr,
    _mqx_uint           msecs
)
{ /* Body */
    MQX_TICK_STRUCT tmp;

    if (msecs) {
        PSP_MILLISECONDS_TO_TICKS(msecs, &tmp);
        PSP_ADD_TICKS(tick_ptr, &tmp, tick_ptr);
    } /* Endif */

    return tick_ptr;

} /* Endbody */

/*!
 * \brief Add time in nanoseconds to tick time struct.
 *
 * The functions can also be used in conjunction with the global constant
 * _mqx_zero_tick_struct to convert units to tick time.
 *
 * \param[in] tick_ptr Tick time to add to.
 * \param[in] nsecs    The number of nanoseconds to add.
 *
 * \return Tick time struct.
 *
 * \see _time_add_day_to_ticks
 * \see _time_add_hour_to_ticks
 * \see _time_add_min_to_ticks
 * \see _time_add_sec_to_ticks
 * \see _time_add_msec_to_ticks
 * \see _time_add_usec_to_ticks
 * \see _time_add_psec_to_ticks
 * \see _mqx_zero_tick_struct
 */
MQX_TICK_STRUCT_PTR _time_add_nsec_to_ticks
(
    MQX_TICK_STRUCT_PTR tick_ptr,
    _mqx_uint           nsecs
)
{ /* Body */
    MQX_TICK_STRUCT tmp;

    if (nsecs) {
        PSP_NANOSECONDS_TO_TICKS(nsecs, &tmp);
        PSP_ADD_TICKS(tick_ptr, &tmp, tick_ptr);
    } /* Endif */

    return tick_ptr;

} /* Endbody */

/*!
 * \brief Add time in picoseconds to tick time struct.
 *
 * The functions can also be used in conjunction with the global constant
 * _mqx_zero_tick_struct to convert units to tick time.
 *
 * \param[in] tick_ptr Tick time to add to.
 * \param[in] psecs    The number of picoseconds to add.
 *
 * \return Tick time struct.
 *
 * \see _time_add_day_to_ticks
 * \see _time_add_hour_to_ticks
 * \see _time_add_min_to_ticks
 * \see _time_add_sec_to_ticks
 * \see _time_add_msec_to_ticks
 * \see _time_add_usec_to_ticks
 * \see _time_add_nsec_to_ticks
 * \see _mqx_zero_tick_struct
 */
MQX_TICK_STRUCT_PTR _time_add_psec_to_ticks
(
    MQX_TICK_STRUCT_PTR tick_ptr,
    _mqx_uint           psecs
)
{ /* Body */
    MQX_TICK_STRUCT tmp;

    if (psecs) {
        PSP_PICOSECONDS_TO_TICKS(psecs, &tmp);
        PSP_ADD_TICKS(tick_ptr, &tmp, tick_ptr);
    } /* Endif */

    return tick_ptr;

} /* Endbody */

/*!
 * \brief Add time in seconds to tick time struct.
 *
 * The functions can also be used in conjunction with the global constant
 * _mqx_zero_tick_struct to convert units to tick time.
 *
 * \param[in] tick_ptr Tick time to add to.
 * \param[in] secs     The number of seconds to add.
 *
 * \return Tick time struct.
 *
 * \see _time_add_day_to_ticks
 * \see _time_add_hour_to_ticks
 * \see _time_add_min_to_ticks
 * \see _time_add_msec_to_ticks
 * \see _time_add_usec_to_ticks
 * \see _time_add_nsec_to_ticks
 * \see _time_add_psec_to_ticks
 * \see _mqx_zero_tick_struct
 */
MQX_TICK_STRUCT_PTR _time_add_sec_to_ticks
(
    MQX_TICK_STRUCT_PTR tick_ptr,
    _mqx_uint           secs
)
{ /* Body */
    MQX_TICK_STRUCT tmp;

    if (secs) {
        PSP_SECONDS_TO_TICKS(secs, &tmp);
        PSP_ADD_TICKS(tick_ptr, &tmp, tick_ptr);
    } /* Endif */

    return tick_ptr;

} /* Endbody */

/*!
 * \brief Add time in microseconds to tick time struct.
 *
 * The functions can also be used in conjunction with the global constant
 * _mqx_zero_tick_struct to convert units to tick time.
 *
 * \param[in] tick_ptr Tick time to add to.
 * \param[in] usecs    The number of microseconds to add.
 *
 * \return Tick time struct.
 *
 * \see _time_add_day_to_ticks
 * \see _time_add_hour_to_ticks
 * \see _time_add_min_to_ticks
 * \see _time_add_sec_to_ticks
 * \see _time_add_msec_to_ticks
 * \see _time_add_nsec_to_ticks
 * \see _time_add_psec_to_ticks
 * \see _mqx_zero_tick_struct
 */
MQX_TICK_STRUCT_PTR _time_add_usec_to_ticks
(
    MQX_TICK_STRUCT_PTR tick_ptr,
    _mqx_uint           usecs
)
{ /* Body */
    MQX_TICK_STRUCT tmp;

    if (usecs) {
        PSP_MICROSECONDS_TO_TICKS(usecs, &tmp);
        PSP_ADD_TICKS(tick_ptr, &tmp, tick_ptr);
    } /* Endif */

    return tick_ptr;

} /* Endbody */

#if MQX_HAS_TICK

/*!
 * \brief Suspend the active task for the number of milliseconds.
 *
 * The functions put the active task in the timeout queue for the specified time.
 * \n Before the time expires, any task can remove the task from the timeout
 * queue by calling _time_dequeue().
 *
 * \param[in] milliseconds Minimum number of milliseconds to suspend the task.
 *
 * \warning Blocks the calling task.
 * \warning If parameter "milliseconds" equals zero then only _sched_yield() function is called.
 * \warning On failure, calls _task_set_error() to set the following task error
 * code:
 * \li MQX_CANNOT_CALL_FUNCTION_FROM_ISR
 *
 * \see _time_delay_for
 * \see _time_delay_ticks
 * \see _time_delay_until
 * \see _time_dequeue
 */
void _time_delay
(
    register uint32_t milliseconds
)
{ /* Body */
    register KERNEL_DATA_STRUCT_PTR kernel_data;
    register TD_STRUCT_PTR td_ptr;

#if MQX_ENABLE_USER_MODE && MQX_ENABLE_USER_STDAPI
    if (MQX_RUN_IN_USER_MODE) {
        _usr_time_delay(milliseconds);
        return;
    }
#endif

    _GET_KERNEL_DATA(kernel_data);

    _KLOGE2(KLOG_time_delay, milliseconds);

#if MQX_CHECK_ERRORS
    if (kernel_data->IN_ISR)
    {
        _task_set_error(MQX_CANNOT_CALL_FUNCTION_FROM_ISR);
        _KLOGX2(KLOG_time_delay, MQX_CANNOT_CALL_FUNCTION_FROM_ISR);
        return;
    } /* Endif */
#endif /* MQX_CHECK_ERRORS */

    /* If zero miliseconds specified, suspend the task till next reschedule */
    if ( ! milliseconds ) {
        _sched_yield();
        _KLOGX1(KLOG_time_delay);
        return;
    } /* Endif */

    td_ptr = kernel_data->ACTIVE_PTR;

    /* Compute the number of tick events required to accomplish the least amount of time[ms]. */
    /* tick_events = (required_time[ms] + (time_per_tick[ms] - 1)) / time_per_tick[ms]) + 1  -->
     * tick_events = ((required_time[ms] - 1) / time_per_tick[ms]) + 2
     */
    milliseconds--;
    /* Convert milliseconds to ticks, truncated */
    PSP_MILLISECONDS_TO_TICKS_QUICK(milliseconds, &td_ptr->TIMEOUT);
    /* Resolve truncation by adding one tick. Add another tick to accomplish the requested amount of time. */
    PSP_ADD_TICKS_TO_TICK_STRUCT(&td_ptr->TIMEOUT, 2, &td_ptr->TIMEOUT);

    _INT_DISABLE();
    /* Calculate time to wake up the task */
    PSP_ADD_TICKS(&td_ptr->TIMEOUT, &kernel_data->TIME, &td_ptr->TIMEOUT);

    _time_delay_internal(td_ptr);

    _INT_ENABLE();

    _KLOGX1(KLOG_time_delay);

} /* Endbody */


#if MQX_ENABLE_USER_MODE
/*!
 * \brief Suspend the active task for the number of milliseconds.
 *
 * This function is equivalent to _time_delay() API call but it can be executed
 * from within the User task or other code running in the CPU User mode.
 * Parameters passed to this function by pointer are required to meet the memory
 * protection requirements as described in the parameter list below.
 *
 * \param[in] milliseconds Minimum number of milliseconds to suspend the task.
 *
 * \see _time_delay
 * \see _usr_time_get_elapsed_ticks
 */
void _usr_time_delay
(
    register uint32_t milliseconds
)
{
    MQX_API_CALL_PARAMS params = {milliseconds, 0, 0, 0, 0};
    _mqx_api_call(MQX_API_TIME_DELAY, &params);
}

#endif /* MQX_ENABLE_USER_MODE */
#endif /* MQX_HAS_TICK */

#if MQX_HAS_TICK
/*!
 * \brief Suspend the active task for the number of ticks (in tick time).
 *
 * The functions put the active task in the timeout queue for the specified time.
 * \n Before the time expires, any task can remove the task from the timeout
 * queue by calling _time_dequeue().
 *
 * \param[in] ticks Pointer to the minimum number of ticks to suspend the task.
 *
 * \warning Blocks the calling task.
 * \warning On failure, calls _task_set_error() to set the following task error
 * code:
 * \li MQX_INVALID_PARAMETER
 * \li MQX_CANNOT_CALL_FUNCTION_FROM_ISR
 *
 * \see _time_delay
 * \see _time_delay_ticks
 * \see _time_delay_until
 * \see _time_dequeue
 */
void _time_delay_for
(
    register MQX_TICK_STRUCT_PTR ticks)
{ /* Body */
    register KERNEL_DATA_STRUCT_PTR kernel_data;
    register TD_STRUCT_PTR td_ptr;

    _GET_KERNEL_DATA(kernel_data);

    _KLOGE2(KLOG_time_delay_for, ticks);

#if MQX_CHECK_ERRORS
    if (ticks == NULL) {
        _task_set_error(MQX_INVALID_PARAMETER);
        _KLOGX2(KLOG_time_delay_for, MQX_INVALID_PARAMETER);
        return;
    } /* Endif */

    if (kernel_data->IN_ISR)
    {
        _task_set_error(MQX_CANNOT_CALL_FUNCTION_FROM_ISR);
        _KLOGX2(KLOG_time_delay_for, MQX_CANNOT_CALL_FUNCTION_FROM_ISR);
        return;
    } /* Endif */
#endif

    td_ptr = kernel_data->ACTIVE_PTR;

    _INT_DISABLE();

    /* Calculate time to wake up the task */
    PSP_ADD_TICKS(ticks, &kernel_data->TIME, &td_ptr->TIMEOUT);

    _time_delay_internal(td_ptr);

    _INT_ENABLE();

    _KLOGX1( KLOG_time_delay_for);

} /* Endbody */

#endif /* MQX_HAS_TICK */

#if MQX_HAS_TICK
/*!
 * \private
 *
 * \brief Suspend the active task for the specified time.
 *
 * This function puts a task on the timeout queue for the specified number of
 * ticks, or until removed by another task. Must be called int disabled.
 *
 * \param[in] td_ptr The task to delay.
 *
 * \see _time_delay
 * \see _time_delay_for
 * \see _time_delay_ticks
 * \see _time_delay_until
 */
void _time_delay_internal
   (
      register TD_STRUCT_PTR td_ptr
   )
{ /* Body */
   register KERNEL_DATA_STRUCT_PTR  kernel_data;
   register TD_STRUCT_PTR           td2_ptr;
   register TD_STRUCT_PTR           tdprev_ptr;
   register _mqx_uint               count;
   register _mqx_int                result;

   _GET_KERNEL_DATA(kernel_data);

   /* Remove task from ready to run queue */
   tdprev_ptr = (TD_STRUCT_PTR)((void *)&kernel_data->TIMEOUT_QUEUE);
   if ( _QUEUE_GET_SIZE(&kernel_data->TIMEOUT_QUEUE) ) {

      /* Perform insertion sort by time */
      td2_ptr    = (TD_STRUCT_PTR)((void *)kernel_data->TIMEOUT_QUEUE.NEXT);

      /* SPR P171-0023-01 use pre-decrement on while loop */
      count      = _QUEUE_GET_SIZE(&kernel_data->TIMEOUT_QUEUE) + 1;
      while ( --count ) {
      /* END SPR */
         result = PSP_CMP_TICKS(&td2_ptr->TIMEOUT, &td_ptr->TIMEOUT);
         if (MQX_DELAY_ENQUEUE_POLICY(result)) {
            /* Enqueue before td2_ptr */
            break;
         } /* Endif */

         tdprev_ptr = td2_ptr;
         td2_ptr    = td2_ptr->TD_NEXT;
      } /* Endwhile */

   } /* Endif */

   /* Remove from ready queue */
   _QUEUE_UNLINK(td_ptr);

   /* Insert into timeout queue */
   _QUEUE_INSERT(&kernel_data->TIMEOUT_QUEUE,tdprev_ptr,td_ptr);

   td_ptr->STATE |= IS_ON_TIMEOUT_Q;

   _sched_execute_scheduler_internal();

} /* Endbody */
#endif /* MQX_HAS_TICK */

#if MQX_HAS_TICK
/*!
 * \brief Suspend the active task for the number of ticks.
 *
 * The functions put the active task in the timeout queue for the specified time.
 * \n Before the time expires, any task can remove the task from the timeout
 * queue by calling _time_dequeue().
 *
 * \param[in] time_in_ticks Minimum number of ticks to suspend the task.
 *
 * \warning Blocks the calling task.
 * \warning If parameter "time_in_ticks" equals zero then only _sched_yield() function is called.
 * \warning On failure, calls _task_set_error() to set the following task error
 * code:
 * \li MQX_CANNOT_CALL_FUNCTION_FROM_ISR
 *
 * \see _time_delay
 * \see _time_delay_for
 * \see _time_delay_until
 * \see _time_dequeue
 */
void _time_delay_ticks
(
    register _mqx_uint time_in_ticks
)
{ /* Body */
    register KERNEL_DATA_STRUCT_PTR kernel_data;
    register TD_STRUCT_PTR td_ptr;

#if MQX_ENABLE_USER_MODE && MQX_ENABLE_USER_STDAPI
    if (MQX_RUN_IN_USER_MODE) {
        _usr_time_delay_ticks(time_in_ticks);
        return;
    }
#endif

    _GET_KERNEL_DATA(kernel_data);

    _KLOGE2(KLOG_time_delay_ticks, time_in_ticks);

#if MQX_CHECK_ERRORS
    if (kernel_data->IN_ISR)
    {
        _task_set_error(MQX_CANNOT_CALL_FUNCTION_FROM_ISR);
        _KLOGX2(KLOG_time_delay_ticks, MQX_CANNOT_CALL_FUNCTION_FROM_ISR);
        return;
    } /* Endif */
#endif /* MQX_CHECK_ERRORS */

    /* If zero ticks specified, suspend the task till next reschedule */
    if ( !time_in_ticks ) {
        _sched_yield();
        _KLOGX1(KLOG_time_delay_ticks);
        return;
    } /* Endif */

    td_ptr = kernel_data->ACTIVE_PTR;

    _INT_DISABLE();

    PSP_ADD_TICKS_TO_TICK_STRUCT(&kernel_data->TIME, time_in_ticks, &td_ptr->TIMEOUT);
    _time_delay_internal(td_ptr);

    _INT_ENABLE();
    _KLOGX1(KLOG_time_delay_ticks);

}

#if MQX_ENABLE_USER_MODE
/*!
 * \brief Suspend the active task for the number of ticks.
 *
 * This function is equivalent to _time_delay_ticks() API call but it can be
 * executed from within the User task or other code running in the CPU User mode.
 * Parameters passed to this function by pointer are required to meet the memory
 * protection requirements as described in the parameter list below.
 *
 * \param[in] time_in_ticks Minimum number of ticks to suspend the task.
 *
 * \see _time_delay_ticks
 * \see _usr_time_get_elapsed_ticks
 */
void _usr_time_delay_ticks
(
    register _mqx_uint time_in_ticks
)
{
    MQX_API_CALL_PARAMS params = {(uint32_t)time_in_ticks, 0, 0, 0, 0};
    _mqx_api_call(MQX_API_TIME_DELAY_TICKS, &params);
}

#endif /* MQX_ENABLE_USER_MODE */
#endif /* MQX_HAS_TICK */

#if MQX_HAS_TICK
/*!
 * \brief Suspend the active task until the specified time (in tick time).
 *
 * The functions put the active task in the timeout queue for the specified time.
 * \n Before the time expires, any task can remove the task from the timeout
 * queue by calling _time_dequeue().
 *
 * \param[in] ticks Pointer to the time (in tick time) until which to suspend
 * the task.
 *
 * \warning Blocks the calling task.
 * \warning On failure, calls _task_set_error() to set the following task error
 * code:
 * \li MQX_INVALID_PARAMETER
 * \li MQX_CANNOT_CALL_FUNCTION_FROM_ISR
 *
 * \see _time_delay
 * \see _time_delay_for
 * \see _time_delay_ticks
 * \see _time_dequeue
 */
void _time_delay_until
(
    register MQX_TICK_STRUCT_PTR ticks
)
{ /* Body */
    register KERNEL_DATA_STRUCT_PTR kernel_data;
    register TD_STRUCT_PTR td_ptr;

    _GET_KERNEL_DATA(kernel_data);

    _KLOGE2(KLOG_time_delay_until, ticks);

#if MQX_CHECK_ERRORS
    if (ticks == NULL) {
        _task_set_error(MQX_INVALID_PARAMETER);
        _KLOGX2(KLOG_time_delay_until, MQX_INVALID_PARAMETER);
        return;
    } /* Endif */

    if (kernel_data->IN_ISR)
    {
        _task_set_error(MQX_CANNOT_CALL_FUNCTION_FROM_ISR);
        _KLOGX2(KLOG_time_delay_until, MQX_CANNOT_CALL_FUNCTION_FROM_ISR);
        return;
    } /* Endif */
#endif /* MQX_CHECK_ERRORS */


    td_ptr = kernel_data->ACTIVE_PTR;

    /* Calculate time to wake up the task */
    td_ptr->TIMEOUT = *ticks;

    _INT_DISABLE();

    _time_delay_internal(td_ptr);

    _INT_ENABLE();

    _KLOGX1(KLOG_time_delay_until);

} /* Endbody */

#endif /* MQX_HAS_TICK */

/*!
 * \brief Get the difference between two tick times in days.
 *
 * If the calculation overflows int32_t, the function sets the bool at
 * overflow_ptr to TRUE. If this happens, use the _time_diff function for a
 * larger unit. For example, if _time_diff_hours() sets the overflow, use
 * _time_diff_days().
 * \n The functions can also be used in conjunction with the global constant
 * _mqx_zero_tick_struct to convert tick time to units.
 *
 * \param[in]  end_tick_ptr   Pointer to the ending tick time, which must be
 * greater than the starting tick time.
 * \param[in]  start_tick_ptr Pointer to the starting tick time.
 * \param[out] overflow_ptr   TRUE if overflow occurs (see Description).
 *
 * \return Difference in days.
 * \return MAX_INT_32 (Failure.)
 *
 * \see _time_diff_hours
 * \see _time_diff_minutes
 * \see _time_diff_seconds
 * \see _time_diff_milliseconds
 * \see _time_diff_microseconds
 * \see _time_diff_nanoseconds
 * \see _time_diff_picoseconds
 * \see _mqx_zero_tick_struct
 * \see _time_diff
 * \see _time_diff_ticks
 * \see _time_diff_ticks_int32
 * \see _time_get
 * \see _time_get_ticks
 * \see _time_set
 * \see _time_set_ticks
 * \see MQX_TICK_STRUCT
 */
int32_t _time_diff_days
(
    MQX_TICK_STRUCT_PTR end_tick_ptr,
    MQX_TICK_STRUCT_PTR start_tick_ptr,
    bool        *overflow_ptr
)
{ /* Body */
    MQX_TICK_STRUCT diff_tick;
    _mqx_uint result;

#if MQX_CHECK_ERRORS
    if (overflow_ptr == NULL) {
        return MAX_INT_32;
    } /* Endif */
#endif

    result = _time_diff_ticks(end_tick_ptr, start_tick_ptr, &diff_tick);

#if MQX_CHECK_ERRORS
    if (result != MQX_OK) {
        return MAX_INT_32;
    } /* Endif */
#endif

    return( PSP_TICKS_TO_DAYS(&diff_tick, overflow_ptr) );

} /* Endbody */

/*!
 * \brief Get the difference between two second/millisecond times.
 *
 * \param[in]  start_time_ptr Pointer to the normalized start time in
 * second/millisecond time.
 * \param[in]  end_time_ptr   Pointer to the normalized end time, which must be
 * greater than the start time.
 * \param[out] diff_time_ptr  Pointer to the time difference (the time is
 * normalized).
 *
 * \see _time_diff_ticks
 * \see _time_diff_ticks_int32
 * \see _time_diff_days
 * \see _time_diff_hours
 * \see _time_diff_minutes
 * \see _time_diff_seconds
 * \see _time_diff_milliseconds
 * \see _time_diff_microseconds
 * \see _time_diff_nanoseconds
 * \see _time_diff_picoseconds
 * \see _time_get
 * \see _time_get_ticks
 * \see _time_set
 * \see _time_set_ticks
 * \see MQX_TICK_STRUCT
 * \see TIME_STRUCT
 */
void _time_diff
(
    TIME_STRUCT_PTR start_time_ptr,
    TIME_STRUCT_PTR end_time_ptr,
    TIME_STRUCT_PTR diff_time_ptr
)
{ /* Body */
    TIME_STRUCT temp;

    /*
     * Use temporary variable in case diff_time_ptr is the
     * same as either start or end pointers
     */
    temp.SECONDS = end_time_ptr->SECONDS;
    temp.MILLISECONDS = end_time_ptr->MILLISECONDS;
    if (temp.MILLISECONDS < start_time_ptr->MILLISECONDS) {
        temp.MILLISECONDS += 1000;
        temp.SECONDS--;
    } /* Endif */
    temp.SECONDS -= start_time_ptr->SECONDS;
    temp.MILLISECONDS -= start_time_ptr->MILLISECONDS;

    if (temp.MILLISECONDS > 1000) {
        temp.SECONDS += (temp.MILLISECONDS / 1000);
        temp.MILLISECONDS = temp.MILLISECONDS % 1000;
    } /* Endif */

    *diff_time_ptr = temp;

} /* Endbody */

/*!
 * \brief Get the difference between two tick times.
 *
 * \param[in]  end_tick_ptr   Pointer to the normalized end time, which must be
 * greater than the start time.
 * \param[in]  start_tick_ptr Pointer to the normalized start time in tick time.
 * \param[out] diff_tick_ptr  Pointer to the time difference (the time is
 * normalized).
 *
 * \return MQX_OK
 * \return MQX_INVALID_PARAMETER (One or more pointers are NULL.)
 *
 * \see _time_diff
 * \see _time_diff_ticks_int32
 * \see _time_diff_days
 * \see _time_diff_hours
 * \see _time_diff_minutes
 * \see _time_diff_seconds
 * \see _time_diff_milliseconds
 * \see _time_diff_microseconds
 * \see _time_diff_nanoseconds
 * \see _time_diff_picoseconds
 * \see _time_get
 * \see _time_get_ticks
 * \see _time_set
 * \see _time_set_ticks
 * \see MQX_TICK_STRUCT
 * \see TIME_STRUCT
 */
_mqx_uint _time_diff_ticks
(
    MQX_TICK_STRUCT_PTR end_tick_ptr,
    MQX_TICK_STRUCT_PTR start_tick_ptr,
    MQX_TICK_STRUCT_PTR diff_tick_ptr
)
{ /* Body */

#if MQX_CHECK_ERRORS
    if ((end_tick_ptr == NULL) || (start_tick_ptr == NULL) ||
                    (diff_tick_ptr == NULL))
    {
        return MQX_INVALID_PARAMETER;
    } /* Endif */
#endif

    PSP_SUB_TICKS(end_tick_ptr, start_tick_ptr, diff_tick_ptr);

    return MQX_OK;

} /* Endbody */

/*!
 * \brief Get the difference between two tick times and clamps result into
 * int32_t interval.
 *
 * \param[in]  end_tick_ptr   Pointer to the normalized end time, which must be
 * greater than the start time.
 * \param[in]  start_tick_ptr Pointer to the normalized start time in tick time.
 * \param[out] overflow_ptr  Set to TRUE if overflow occurs, otherwise FALSE.
 *
 * \return ticks difference in int32
 * \return MAX_INT_32 (Failure.)
 *
 * \see _time_diff
 * \see _time_diff_ticks
 * \see _time_diff_days
 * \see _time_diff_hours
 * \see _time_diff_minutes
 * \see _time_diff_seconds
 * \see _time_diff_milliseconds
 * \see _time_diff_microseconds
 * \see _time_diff_nanoseconds
 * \see _time_diff_picoseconds
 * \see _time_get
 * \see _time_get_ticks
 * \see _time_set
 * \see _time_set_ticks
 * \see MQX_TICK_STRUCT
 * \see TIME_STRUCT
 */
int32_t _time_diff_ticks_int32
(
    MQX_TICK_STRUCT_PTR end_tick_ptr,
    MQX_TICK_STRUCT_PTR start_tick_ptr,
    bool        *overflow_ptr
)
{ /* Body */

#if MQX_CHECK_ERRORS
    if ((end_tick_ptr == NULL) || (start_tick_ptr == NULL))
    {
        if (overflow_ptr != NULL) *overflow_ptr = TRUE;
        return MAX_INT_32;
    } /* Endif */
#endif

    return (PSP_SUB_TICKS_INT32(end_tick_ptr, start_tick_ptr, overflow_ptr));
} /* Endbody */

/*!
 * \brief Get the difference between two tick times in hours.
 *
 * If the calculation overflows int32_t, the function sets the bool at
 * overflow_ptr to TRUE. If this happens, use the _time_diff function for a
 * larger unit. For example, if _time_diff_hours() sets the overflow, use
 * _time_diff_days().
 * \n The functions can also be used in conjunction with the global constant
 * _mqx_zero_tick_struct to convert tick time to units.
 *
 * \param[in]  end_tick_ptr   Pointer to the ending tick time, which must be
 * greater than the starting tick time.
 * \param[in]  start_tick_ptr Pointer to the starting tick time.
 * \param[out] overflow_ptr   TRUE if overflow occurs (see Description).
 *
 * \return Difference in hours.
 *
 * \see _time_diff_days
 * \see _time_diff_minutes
 * \see _time_diff_seconds
 * \see _time_diff_milliseconds
 * \see _time_diff_microseconds
 * \see _time_diff_nanoseconds
 * \see _time_diff_picoseconds
 * \see _mqx_zero_tick_struct
 * \see _time_diff
 * \see _time_diff_ticks
 * \see _time_diff_ticks_int32
 * \see _time_get
 * \see _time_get_ticks
 * \see _time_set
 * \see _time_set_ticks
 * \see MQX_TICK_STRUCT
 */
int32_t _time_diff_hours
(
    MQX_TICK_STRUCT_PTR end_tick_ptr,
    MQX_TICK_STRUCT_PTR start_tick_ptr,
    bool        *overflow_ptr
)
{ /* Body */
    MQX_TICK_STRUCT diff_tick;

    _time_diff_ticks(end_tick_ptr, start_tick_ptr, &diff_tick);

    return( PSP_TICKS_TO_HOURS(&diff_tick, overflow_ptr) );

} /* Endbody */

/*!
 * \brief Get the difference between two tick times in minutes.
 *
 * If the calculation overflows int32_t, the function sets the bool at
 * overflow_ptr to TRUE. If this happens, use the _time_diff function for a
 * larger unit. For example, if _time_diff_hours() sets the overflow, use
 * _time_diff_days().
 * \n The functions can also be used in conjunction with the global constant
 * _mqx_zero_tick_struct to convert tick time to units.
 *
 * \param[in]  end_tick_ptr   Pointer to the ending tick time, which must be
 * greater than the starting tick time.
 * \param[in]  start_tick_ptr Pointer to the starting tick time.
 * \param[out] overflow_ptr   TRUE if overflow occurs (see Description).
 *
 * \return Difference in minutes.
 *
 * \see _time_diff_days
 * \see _time_diff_hours
 * \see _time_diff_seconds
 * \see _time_diff_milliseconds
 * \see _time_diff_microseconds
 * \see _time_diff_nanoseconds
 * \see _time_diff_picoseconds
 * \see _mqx_zero_tick_struct
 * \see _time_diff
 * \see _time_diff_ticks
 * \see _time_diff_ticks_int32
 * \see _time_get
 * \see _time_get_ticks
 * \see _time_set
 * \see _time_set_ticks
 * \see MQX_TICK_STRUCT
 */
int32_t _time_diff_minutes
(
    MQX_TICK_STRUCT_PTR end_tick_ptr,
    MQX_TICK_STRUCT_PTR start_tick_ptr,
    bool               *overflow_ptr
)
{ /* Body */
    MQX_TICK_STRUCT diff_tick;

    _time_diff_ticks(end_tick_ptr, start_tick_ptr, &diff_tick);

    return( PSP_TICKS_TO_MINUTES(&diff_tick, overflow_ptr) );

} /* Endbody */

/*!
 * \brief Get the difference between two tick times in milliseconds.
 *
 * If the calculation overflows int32_t, the function sets the bool at
 * overflow_ptr to TRUE. If this happens, use the _time_diff function for a
 * larger unit. For example, if _time_diff_hours() sets the overflow, use
 * _time_diff_days().
 * \n The functions can also be used in conjunction with the global constant
 * _mqx_zero_tick_struct to convert tick time to units.
 *
 * \param[in]  end_tick_ptr   Pointer to the ending tick time, which must be
 * greater than the starting tick time.
 * \param[in]  start_tick_ptr Pointer to the starting tick time.
 * \param[out] overflow_ptr   TRUE if overflow occurs (see Description).
 *
 * \return Difference in milliseconds.
 *
 * \see _time_diff_days
 * \see _time_diff_hours
 * \see _time_diff_minutes
 * \see _time_diff_seconds
 * \see _time_diff_microseconds
 * \see _time_diff_nanoseconds
 * \see _time_diff_picoseconds
 * \see _mqx_zero_tick_struct
 * \see _time_diff
 * \see _time_diff_ticks
 * \see _time_diff_ticks_int32
 * \see _time_get
 * \see _time_get_ticks
 * \see _time_set
 * \see _time_set_ticks
 * \see MQX_TICK_STRUCT
 */
int32_t _time_diff_milliseconds
(
    MQX_TICK_STRUCT_PTR end_tick_ptr,
    MQX_TICK_STRUCT_PTR start_tick_ptr,
    bool               *overflow_ptr
)
{ /* Body */
    MQX_TICK_STRUCT diff_tick;

    _time_diff_ticks(end_tick_ptr, start_tick_ptr, &diff_tick);

    return( PSP_TICKS_TO_MILLISECONDS(&diff_tick, overflow_ptr) );

} /* Endbody */

/*!
 * \brief Get the difference between two tick times in nanoseconds.
 *
 * If the calculation overflows int32_t, the function sets the bool at
 * overflow_ptr to TRUE. If this happens, use the _time_diff function for a
 * larger unit. For example, if _time_diff_hours() sets the overflow, use
 * _time_diff_days().
 * \n The functions can also be used in conjunction with the global constant
 * _mqx_zero_tick_struct to convert tick time to units.
 *
 * \param[in]  end_tick_ptr   Pointer to the ending tick time, which must be
 * greater than the starting tick time.
 * \param[in]  start_tick_ptr Pointer to the starting tick time.
 * \param[out] overflow_ptr   TRUE if overflow occurs (see Description).
 *
 * \return Difference in nanoseconds.
 *
 * \see _time_diff_days
 * \see _time_diff_hours
 * \see _time_diff_minutes
 * \see _time_diff_seconds
 * \see _time_diff_milliseconds
 * \see _time_diff_microseconds
 * \see _time_diff_picoseconds
 * \see _mqx_zero_tick_struct
 * \see _time_diff
 * \see _time_diff_ticks
 * \see _time_diff_ticks_int32
 * \see _time_get
 * \see _time_get_ticks
 * \see _time_set
 * \see _time_set_ticks
 * \see MQX_TICK_STRUCT
 */
int32_t _time_diff_nanoseconds
(
    MQX_TICK_STRUCT_PTR end_tick_ptr,
    MQX_TICK_STRUCT_PTR start_tick_ptr,
    bool        *overflow_ptr
)
{ /* Body */
    MQX_TICK_STRUCT diff_tick;

    _time_diff_ticks(end_tick_ptr, start_tick_ptr, &diff_tick);

    return( PSP_TICKS_TO_NANOSECONDS(&diff_tick, overflow_ptr) );

} /* Endbody */

/*!
 * \brief Get the difference between two tick times in picoseconds.
 *
 * If the calculation overflows int32_t, the function sets the bool at
 * overflow_ptr to TRUE. If this happens, use the _time_diff function for a
 * larger unit. For example, if _time_diff_hours() sets the overflow, use
 * _time_diff_days().
 * \n The functions can also be used in conjunction with the global constant
 * _mqx_zero_tick_struct to convert tick time to units.
 *
 * \param[in]  end_tick_ptr   Pointer to the ending tick time, which must be
 * greater than the starting tick time.
 * \param[in]  start_tick_ptr Pointer to the starting tick time.
 * \param[out] overflow_ptr   TRUE if overflow occurs (see Description).
 *
 * \return Difference in picoseconds.
 *
 * \see _time_diff_days
 * \see _time_diff_hours
 * \see _time_diff_minutes
 * \see _time_diff_seconds
 * \see _time_diff_milliseconds
 * \see _time_diff_microseconds
 * \see _time_diff_nanoseconds
 * \see _mqx_zero_tick_struct
 * \see _time_diff
 * \see _time_diff_ticks
 * \see _time_diff_ticks_int32
 * \see _time_get
 * \see _time_get_ticks
 * \see _time_set
 * \see _time_set_ticks
 * \see MQX_TICK_STRUCT
 */
int32_t _time_diff_picoseconds
(
    MQX_TICK_STRUCT_PTR end_tick_ptr,
    MQX_TICK_STRUCT_PTR start_tick_ptr,
    bool        *overflow_ptr
)
{ /* Body */
    MQX_TICK_STRUCT diff_tick;

    _time_diff_ticks(end_tick_ptr, start_tick_ptr, &diff_tick);

    return( PSP_TICKS_TO_PICOSECONDS(&diff_tick, overflow_ptr) );

} /* Endbody */

/*!
 * \brief Get the difference between two tick times in seconds.
 *
 * If the calculation overflows int32_t, the function sets the bool at
 * overflow_ptr to TRUE. If this happens, use the _time_diff function for a
 * larger unit. For example, if _time_diff_hours() sets the overflow, use
 * _time_diff_days().
 * \n The functions can also be used in conjunction with the global constant
 * _mqx_zero_tick_struct to convert tick time to units.
 *
 * \param[in]  end_tick_ptr   Pointer to the ending tick time, which must be
 * greater than the starting tick time.
 * \param[in]  start_tick_ptr Pointer to the starting tick time.
 * \param[out] overflow_ptr   TRUE if overflow occurs (see Description).
 *
 * \return Difference in seconds.
 *
 * \see _time_diff_days
 * \see _time_diff_hours
 * \see _time_diff_minutes
 * \see _time_diff_milliseconds
 * \see _time_diff_microseconds
 * \see _time_diff_nanoseconds
 * \see _time_diff_picoseconds
 * \see _mqx_zero_tick_struct
 * \see _time_diff
 * \see _time_diff_ticks
 * \see _time_diff_ticks_int32
 * \see _time_get
 * \see _time_get_ticks
 * \see _time_set
 * \see _time_set_ticks
 * \see MQX_TICK_STRUCT
 */
int32_t _time_diff_seconds
(
    MQX_TICK_STRUCT_PTR end_tick_ptr,
    MQX_TICK_STRUCT_PTR start_tick_ptr,
    bool        *overflow_ptr
)
{ /* Body */
    MQX_TICK_STRUCT diff_tick;

    _time_diff_ticks(end_tick_ptr, start_tick_ptr, &diff_tick);

    return( PSP_TICKS_TO_SECONDS(&diff_tick, overflow_ptr) );

} /* Endbody */

/*!
 * \brief Get the difference between two tick times in microseconds.
 *
 * If the calculation overflows int32_t, the function sets the bool at
 * overflow_ptr to TRUE. If this happens, use the _time_diff function for a
 * larger unit. For example, if _time_diff_hours() sets the overflow, use
 * _time_diff_days().
 * \n The functions can also be used in conjunction with the global constant
 * _mqx_zero_tick_struct to convert tick time to units.
 *
 * \param[in]  end_tick_ptr   Pointer to the ending tick time, which must be
 * greater than the starting tick time.
 * \param[in]  start_tick_ptr Pointer to the starting tick time.
 * \param[out] overflow_ptr   TRUE if overflow occurs (see Description).
 *
 * \return Difference in microseconds.
 *
 * \see _time_diff_days
 * \see _time_diff_hours
 * \see _time_diff_minutes
 * \see _time_diff_seconds
 * \see _time_diff_milliseconds
 * \see _time_diff_nanoseconds
 * \see _time_diff_picoseconds
 * \see _mqx_zero_tick_struct
 * \see _time_diff
 * \see _time_diff_ticks
 * \see _time_diff_ticks_int32
 * \see _time_get
 * \see _time_get_ticks
 * \see _time_set
 * \see _time_set_ticks
 * \see MQX_TICK_STRUCT
 */
int32_t _time_diff_microseconds
(
    MQX_TICK_STRUCT_PTR end_tick_ptr,
    MQX_TICK_STRUCT_PTR start_tick_ptr,
    bool        *overflow_ptr
)
{ /* Body */
    MQX_TICK_STRUCT diff_tick;

    _time_diff_ticks(end_tick_ptr, start_tick_ptr, &diff_tick);

    return( PSP_TICKS_TO_MICROSECONDS(&diff_tick, overflow_ptr) );

} /* Endbody */

/*!
 * \brief Removes the task (specified by task ID) from the timeout queue. It
 * does not reschedule the task.
 *
 * The function removes from the timeout queue a task that has put itself there
 * for a period of time (_time_delay()).
 * \n If tid is invalid or represents a task that is on another processor, the
 * function does nothing.
 * \n A task that calls the function must subsequently put the task in the task's
 * ready queue with _task_ready().
 *
 * \param[in] tid Task ID of the task to be removed from the timeout queue.
 *
 * \warning Removes the task from the timeout queue, but does not put it in the
 * task's ready queue.
 *
 * \see _task_ready
 * \see _time_delay
 * \see _time_delay_for
 * \see _time_delay_ticks
 * \see _time_delay_until
 * \see _time_dequeue_td
 */
void _time_dequeue
(
    _task_id tid
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    TD_STRUCT_PTR td_ptr;

    _GET_KERNEL_DATA(kernel_data);

    _KLOGE2(KLOG_time_dequeue, tid);

    td_ptr = (TD_STRUCT_PTR) _task_get_td(tid);
    if (td_ptr == NULL) {
        _KLOGX1( KLOG_time_dequeue);
        return;
    }/* Endif */

    _int_disable();
    _TIME_DEQUEUE(td_ptr, kernel_data);
    _int_enable();

    _KLOGX1( KLOG_time_dequeue);

} /* Endbody */

/*!
 * \brief Removes the task (specified by task descriptor) from the timeout queue.
 * It does not reschedule the task.
 *
 * The function removes from the timeout queue a task that has put itself there
 * for a period of time (_time_delay()).
 * \n If tid is invalid or represents a task that is on another processor, the
 * function does nothing.
 * \n A task that calls the function must subsequently put the task in the task's
 * ready queue with _task_ready().
 *
 * \param[in] td Pointer to the task descriptor of the task to be removed from
 * the timeout queue.
 *
 * \warning Removes the task from the timeout queue, but does not put it in the
 * task's ready queue.
 *
 * \see _task_ready
 * \see _time_delay
 * \see _time_delay_for
 * \see _time_delay_ticks
 * \see _time_delay_until
 * \see _time_dequeue
 */
void _time_dequeue_td
(
    void   *td
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    TD_STRUCT_PTR td_ptr = (TD_STRUCT_PTR) td;

    _GET_KERNEL_DATA(kernel_data);

#if MQX_CHECK_ERRORS
    if (td_ptr == NULL) {
        return;
    }/* Endif */
#endif

    _KLOGE2(KLOG_time_dequeue_td, td);

    _int_disable();
    _TIME_DEQUEUE(td_ptr, kernel_data);
    _int_enable();

    _KLOGX1( KLOG_time_dequeue_td);

} /* Endbody */

/*!
 * \brief Gets the number of seconds and milliseconds since the processor
 * started (without any time offset information).
 *
 * The function always returns elapsed time; it is not affected by _time_set()
 * or _time_set_ticks().
 *
 * \param[in,out] ts_ptr Where to store the elapsed normalized second/millisecond.
 *
 * \see _time_get_elapsed_ticks
 * \see _time_get_elapsed_ticks_fast
 * \see _time_get
 * \see _time_get_ticks
 * \see _time_set
 * \see _time_set_ticks
 * \see TIME_STRUCT
 * \see MQX_TICK_STRUCT
 */
void _time_get_elapsed
(
    TIME_STRUCT_PTR ts_ptr
)
{ /* Body */
    MQX_TICK_STRUCT tick;

#if MQX_CHECK_ERRORS
    if ( ts_ptr == NULL ) {
        return;
    } /* Endif */
#endif

    _time_get_elapsed_ticks(&tick);

    PSP_TICKS_TO_TIME(&tick, ts_ptr);

} /* Endbody */

/*!
 * \brief Gets the number of ticks since the processor started (without any time
 * offset information).
 *
 * The function always returns elapsed time; it is not affected by _time_set()
 * or _time_set_ticks().
 *
 * \param[in,out] tick_ptr Where to store the elapsed tick time.
 *
 * \see _time_get_elapsed
 * \see _time_get_elapsed_ticks_fast
 * \see _time_get
 * \see _time_get_ticks
 * \see _time_set
 * \see _time_set_ticks
 * \see TIME_STRUCT
 * \see MQX_TICK_STRUCT
 */
void _time_get_elapsed_ticks
(
    MQX_TICK_STRUCT_PTR tick_ptr
)
{
    KERNEL_DATA_STRUCT_PTR kernel_data;

#if MQX_CHECK_ERRORS
    if ( tick_ptr == NULL ) {
        return;
    }
#endif

#if MQX_ENABLE_USER_MODE && MQX_ENABLE_USER_STDAPI
    if (MQX_RUN_IN_USER_MODE) {
        _usr_time_get_elapsed_ticks(tick_ptr);
        return;
    }
#endif

    _GET_KERNEL_DATA(kernel_data);

    _INT_DISABLE();

    *tick_ptr = kernel_data->TIME;

    if (kernel_data->GET_HWTICKS) {
        /* The hardware clock may have counted passed it's reference
         * and have an interrupt pending.  Thus, HW_TICKS may exceed
         * kernel_data->HW_TICKS_PER_TICK and this tick_ptr may need
         * normalizing.  This is done in a moment.
         */
        tick_ptr->HW_TICKS = (*kernel_data->GET_HWTICKS)(kernel_data->GET_HWTICKS_PARAM);
    } /* Endif */

    /* The timer ISR may go off and increment kernel_data->TIME */
    _INT_ENABLE();

    /* The tick_ptr->HW_TICKS value might exceed the
     * kernel_data->HW_TICKS_PER_TICK and need to be
     * normalized for the PSP.
     */
    PSP_NORMALIZE_TICKS(tick_ptr);

}

#if MQX_ENABLE_USER_MODE
/*!
 * \brief Gets the number of ticks since the processor started (without any time
 * offset information).
 *
 * This function is an equivalent to the _time_get_elapsed_ticks() API call but it
 * can be executed from within the User task or other code running in the CPU
 * User mode. Parameters passed to this function by pointer are required to meet
 * the memory protection requirements as described in the parameter list below.
 *
 * \param[in,out] tick_ptr Read/write. An address where the time is to be put.
 *
 * \see _time_get_elapsed_ticks
 * \see _usr_time_delay()
 * \see _usr_time_delay_ticks()
 * \see MQX_TICK_STRUCT
 */
void _usr_time_get_elapsed_ticks
(
    MQX_TICK_STRUCT_PTR tick_ptr
)
{
    MQX_API_CALL_PARAMS params = {(uint32_t)tick_ptr, 0, 0, 0, 0};
    _mqx_api_call(MQX_API_TIME_GET_ELAPSED_TICKS, &params);
}

#endif /* MQX_ENABLE_USER_MODE */

#if MQX_HAS_TICK
/*!
 * \brief Gets the number of ticks since the processor started (without any time
 * offset information).
 *
 * The function always returns elapsed time; it is not affected by _time_set()
 * or _time_set_ticks().
 * \n The only difference between _time_get_elapsed_ticks_fast() and
 * _time_get_elapsed_ticks() is that this one is supposed to be called from code
 * with interrupts DISABLED. Do not use this function with interrupts ENABLED.
 *
 * \param[in,out] tick_ptr Where to store the elapsed tick time.
 *
 * \see _time_get_elapsed
 * \see _time_get_elapsed_ticks
 * \see _time_get
 * \see _time_get_ticks
 * \see _time_set
 * \see _time_set_ticks
 * \see TIME_STRUCT
 * \see MQX_TICK_STRUCT
 */
void _time_get_elapsed_ticks_fast
(
    MQX_TICK_STRUCT_PTR tick_ptr
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;

#if MQX_CHECK_ERRORS
    if ( tick_ptr == NULL ) {
        return;
    } /* Endif */
#endif

    _GET_KERNEL_DATA(kernel_data);

    *tick_ptr = kernel_data->TIME;

    if (kernel_data->GET_HWTICKS) {
        /* The hardware clock may have counted passed it's reference
         * and have an interrupt pending.  Thus, HW_TICKS may exceed
         * kernel_data->HW_TICKS_PER_TICK and this tick_ptr may need
         * normalizing.  This is done in a moment.
         */
        tick_ptr->HW_TICKS = (*kernel_data->GET_HWTICKS)(kernel_data->GET_HWTICKS_PARAM);
    } /* Endif */

    /* The tick_ptr->HW_TICKS value might exceed the
     * kernel_data->HW_TICKS_PER_TICK and need to be
     * normalized for the PSP.
     */
    PSP_NORMALIZE_TICKS(tick_ptr);

} /* Endbody */
#endif /* MQX_HAS_TICK */

/*!
 * \brief normalize: force norm_value to be a modulo boundary number and add overflow to high_order
 *
 */
static bool normalize
(
    int16_t *norm_value,
    int16_t  boundary,
    int16_t *high_order
)
{ /* Body*/
    int16_t tmp;

    tmp = *norm_value / boundary;
    *norm_value -= tmp * boundary;

    if (*norm_value < 0) {
        *norm_value += boundary;
         tmp --;
    }

#if MQX_CHECK_ERRORS
    /* Check overflow */
    if (tmp < 0L)
    {
        if ((*high_order < 0L) && (tmp < ( (int16_t)0x1000 - *high_order))) /* 0x1000 is the smallest of int16_t */
        {
            return FALSE;
        }
    }
    else /* if tmp >= 0 */
    {
        if ((*high_order > 0L) && (tmp > ( (int16_t)0x7FFF - *high_order))) /* 0x7FFFF is the largest of int16_t */
        {
            return FALSE;
        }
    } /* Endif */
#endif

    *high_order += tmp;
    return TRUE;
} /* Endbody */

/*!
 * \brief Gets second/millisecond time format from date format.
 *
 * The function verifies that the fields in the input structure are within the
 * following ranges.
 *
 * <table>
 *   <tr>
 *     <td><b>Field</b></td>
 *     <td><b>Minimum</b></td>
 *     <td><b>Maximum</b></td>
 *   </tr>
 *   <tr>
 *     <td><b>YEAR</b></td>
 *     <td>1970</td>
 *     <td>2099</td>
 *   </tr>
 *   <tr>
 *     <td><b>MONTH</b></td>
 *     <td>1</td>
 *     <td>12</td>
 *   </tr>
 *   <tr>
 *     <td><b>DAY</b></td>
 *     <td>1</td>
 *     <td>31 (Depending on the month.)</td>
 *   </tr>
 *   <tr>
 *     <td><b>HOUR</b></td>
 *     <td>0</td>
 *     <td>23 (Since midnight.)</td>
 *   </tr>
 *   <tr>
 *     <td><b>MINUTE</b></td>
 *     <td>0</td>
 *     <td>59</td>
 *   </tr>
 *   <tr>
 *     <td><b>SECOND</b></td>
 *     <td>0</td>
 *     <td>59</td>
 *   </tr>
 *   <tr>
 *     <td><b>MILLISEC</b></td>
 *     <td>0</td>
 *     <td>999</td>
 *   </tr>
 * </table>
 *
 * The function converts the fields in the input structure to the fields in the
 * output structure, taking into account leap years.
 * \n The time is since 0:00:00.00, January 1, 1970.
 *
 * \param[in]  date_ptr Pointer to a date and time structure.
 * \param[out] ts_ptr   Pointer to a normalized second/millisecond time structure.
 *
 * \return TRUE (Success), FALSE (Fields in date_ptr are out of range or
 * date_ptr or ts_ptr are NULL).
 *
 * \see _time_get
 * \see _time_get_ticks
 * \see _time_get_elapsed
 * \see _time_get_elapsed_ticks
 * \see _time_set
 * \see _time_set_ticks
 * \see _time_to_date
 * \see DATE_STRUCT
 * \see TIME_STRUCT
 */
bool _time_from_date
(
    DATE_STRUCT_PTR date_ptr,
    TIME_STRUCT_PTR ts_ptr
)
{ /* Body */
    uint32_t time, day, month, yday;
    uint32_t leap;

    /* Validate each of the parameters before doing time conversion. */

#if MQX_CHECK_ERRORS
    if ((date_ptr == NULL) || (ts_ptr == NULL)) {
        return (FALSE);
    } /* Endif */
#endif

    if (!normalize(&date_ptr->MILLISEC, 1000, &date_ptr->SECOND)) {
        return FALSE;
    } /* Endif */

    if (!normalize(&date_ptr->SECOND, 60, &date_ptr->MINUTE)) {
        return FALSE;
    } /* Endif */

    if (!normalize(&date_ptr->MINUTE, 60, &date_ptr->HOUR)) {
        return FALSE;
    } /* Endif */

    if (!normalize(&date_ptr->HOUR, 24, &date_ptr->DAY)) {
        return FALSE;
    } /* Endif */

    date_ptr->MONTH --; /* becase month starts from 1 */
    if (!normalize(&date_ptr->MONTH, 12, &date_ptr->YEAR)) {
        return FALSE;
    } /* Endif */
    date_ptr->MONTH ++; /* restore month */

    /*
     * Normalize days of month
     */
    while (1) {
        /* Find out if we are in a leap year. */
        leap = (_mqx_uint) _time_check_if_leap(date_ptr->YEAR);

        if (date_ptr->DAY > _time_days_in_month_internal[leap][date_ptr->MONTH]) {
            date_ptr->DAY -= _time_days_in_month_internal[leap][date_ptr->MONTH];
            date_ptr->MONTH++;
        }
        else if (date_ptr->DAY < 1U) { /* The day of month starts from 1 */
            date_ptr->DAY += (uint16_t) _time_days_in_month_internal[leap][date_ptr->MONTH];
            date_ptr->MONTH--;
        }
        else {
            break;
        }/* Endif */

        if (date_ptr->MONTH == 13U) {
            date_ptr->MONTH = 1;
            date_ptr->YEAR ++;
        }
        else if (date_ptr->MONTH == 0U) {
            date_ptr->MONTH = 12;
            date_ptr->YEAR --;
        }
    } /* Endwhile */

    if (date_ptr->YEAR < (uint16_t) CLK_FIRST_YEAR) { /* Year must be larger than or equal to CLK_FIRST_YEAR (1970)*/
        return FALSE;
    } /* Endif */

    /*
     * Determine the number of days since Jan 1, 1970 at 00:00:00
     */
    day = (date_ptr->YEAR - CLK_FIRST_YEAR) * 365;
    /* Add the leap days from 1970 to YEAR */
    day += date_ptr->YEAR / 4 - date_ptr->YEAR / 100 + date_ptr->YEAR / 400 - NUM_LEAP_YEAR_SINCE_FIRST_YEAR;

    /* Find out if we are in a leap year. */
    leap = (_mqx_uint) _time_check_if_leap(date_ptr->YEAR);
    if (leap) day --;

    /*
     * Add the number of day since Jan 1, to the first
     * day of month.
     */
    month = date_ptr->MONTH;
    yday = 0;
    while (--month > 0) {
        yday += _time_days_in_month_internal[leap][month];
    } /* End while */

    /*
     * Add the number of days since the beginning of the month
     */
    yday += (uint32_t)date_ptr->DAY - 1 ;
    day  += yday;
    /*
     * Add the number of seconds in the hours since midnight
     */
    time = (uint32_t) date_ptr->HOUR * 3600;

    /*
     * Add the number of seconds in the minutes since the hour
     */
    time += (uint32_t) date_ptr->MINUTE * 60;

    /*
     * add the number of seconds since the beginning of the minute
     */
    time += (uint32_t) date_ptr->SECOND;


    /* Check if overflow */
    if ( ((MAXIMUM_SECONDS_IN_TIME - time)/ 86400UL) < day )
    {
        return FALSE;
    }
    /*
     * assign the times
     */
    time += day * 86400UL;
    ts_ptr->SECONDS = time;
    ts_ptr->MILLISECONDS = (uint32_t) date_ptr->MILLISEC;

    /*
     * Update WDAY & YDAY for date_ptr
     */
    date_ptr->WDAY = (day + 4) % 7;
    date_ptr->YDAY = yday;

    return (TRUE);

} /* Endbody */

/*!
 * \brief Get the absolute time in second/millisecond time.
 *
 * If the application changed the absolute time with _time_set() (or
 * _time_set_ticks()), _time_get() (or _time_get_ticks()) returns the time that
 * was set plus the number of seconds and milliseconds (or ticks) since the time
 * was set.
 * \n If the application has not changed the absolute time with _time_set() (or
 * _time_set_ticks()), _time_get() (or _time_get_ticks()) returns the same as
 * _time_get_elapsed() (or _time_get_elapsed_ticks()), which is the number of
 * seconds and milliseconds (or ticks) since MQX started.
 *
 * \param[in,out] ts_ptr Where to store the normalized absolute time in
 * second/millisecond time.
 *
 * \see _time_get_ticks
 * \see _time_get_elapsed
 * \see _time_get_elapsed_ticks
 * \see _time_set
 * \see _time_set_ticks
 * \see MQX_TICK_STRUCT
 * \see TIME_STRUCT
 */
void _time_get
(
    register TIME_STRUCT_PTR ts_ptr
)
{ /* Body */

    MQX_TICK_STRUCT ticks;

    _time_get_ticks(&ticks);

    PSP_TICKS_TO_TIME(&ticks, ts_ptr);

} /* Endbody */

#if MQX_HAS_HW_TICKS
/*!
 * \brief Gets the number of hardware ticks since the last tick.
 *
 * \return Number of hardware ticks since the last tick. (Success.)
 * \return 0 (Failure.)
 *
 * \see _time_get_hwticks_per_tick
 * \see _time_set_hwticks_per_tick
 */
uint32_t _time_get_hwticks
(
    void
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);

    if (kernel_data->GET_HWTICKS) {
        return (*kernel_data->GET_HWTICKS)(kernel_data->GET_HWTICKS_PARAM);
    } /* Endif */

    return 0;

} /* Endbody */
#endif /* MQX_HAS_HW_TICKS */

/*!
 * \brief Gets the calculated number of nanoseconds since the last periodic
 * timer interrupt.
 *
 * \return Number of nanoseconds since the last periodic timer interrupt.
 * \return 0 (BSP does not support the feature).
 *
 * \warning Resolution depends on the periodic timer device.
 *
 * \see _time_get_elapsed
 * \see _time_get_elapsed_ticks
 * \see _time_get
 * \see _time_get_ticks
 * \see _time_set
 * \see _time_set_ticks
 */
uint32_t _time_get_nanoseconds
(
    void
)
{
    uint32_t hwticks;
    uint32_t tickper;

    KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);

    if (kernel_data->GET_HWTICKS == NULL) return 0; /* BSP does not support HW ticks */

    hwticks = (*kernel_data->GET_HWTICKS)(kernel_data->GET_HWTICKS_PARAM);

    /* Tick period in time units (ns) */
    tickper = 1000000000 / kernel_data->TICKS_PER_SECOND; /* 32 bit div */

    /* The result fits in 32 bits */
    return (uint32_t)(((uint64_t) hwticks * tickper) / kernel_data->HW_TICKS_PER_TICK); /* 64 bit mul and div */
}

#if MQX_HAS_TICK
/*!
 * \brief Gets the timer frequency (in ticks per second) that MQX uses.
 *
 * \warning If the timer frequency does not correspond with the interrupt period
 * that was programmed at the hardware level, some timing functions will give
 * incorrect results.
 *
 * \return Period of clock interrupt in ticks per second.
 *
 * \see _time_set_ticks_per_sec
 */
_mqx_uint _time_get_ticks_per_sec
(
    void
)
{ /* Body */
    register KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);
    return( kernel_data->TICKS_PER_SECOND );

} /* Endbody */
#endif /* MQX_HAS_TICK */

#if MQX_HAS_TICK
/*!
 * \brief Gets the resolution of the periodic timer interrupt.
 *
 * On each clock interrupt, MQX increments time by the resolution.
 *
 * \return Resolution of the periodic timer interrupt in milliseconds.
 *
 * \see _time_set_resolution
 * \see _time_get_elapsed
 * \see _time_get_elapsed_ticks
 * \see _time_get
 * \see _time_get_ticks
 * \see _time_set
 * \see _time_set_ticks
 * \see TIME_STRUCT
 */
_mqx_uint _time_get_resolution
(
    void
)
{ /* Body */
    register KERNEL_DATA_STRUCT_PTR kernel_data;
    register _mqx_uint result;

    _GET_KERNEL_DATA(kernel_data);

    result = kernel_data->TICKS_PER_SECOND;

    return( 1000 / result );

} /* Endbody */
#endif /* MQX_HAS_TICK */

#if MQX_HAS_TICK
/*!
 * \brief Get the absolute time in tick time.
 *
 * If the application changed the absolute time with _time_set_ticks() (or
 * _time_set()), _time_get_ticks() (or _time_get()) returns the time that was
 * set plus the number of ticks (seconds and milliseconds) since the time was
 * set.
 * \n If the application has not changed the absolute time with
 * _time_set_ticks() (or _time_set()), _time_get_ticks() (or _time_get())
 * returns the same as _time_get_elapsed_ticks() (or _time_get_elapsed()), which
 * is the number of seconds and milliseconds (or ticks) since MQX started.
 *
 * \param[in,out] tick_ptr Where to store the absolute time in tick time.
 *
 * \see _time_get
 * \see _time_get_elapsed
 * \see _time_get_elapsed_ticks
 * \see _time_set
 * \see _time_set_ticks
 * \see MQX_TICK_STRUCT
 * \see TIME_STRUCT
 */
void _time_get_ticks
(
    register MQX_TICK_STRUCT_PTR tick_ptr
)
{ /* Body */
    register KERNEL_DATA_STRUCT_PTR kernel_data;

#if MQX_CHECK_ERRORS
    if ( tick_ptr == NULL ) {
        return;
    } /* Endif */
#endif

    _GET_KERNEL_DATA(kernel_data);

    _INT_DISABLE();

    *tick_ptr = kernel_data->TIME;

    /* The hardware clock keeps counting... */

    if (kernel_data->GET_HWTICKS) {
        /* The hardware clock may have counted passed it's reference
         * and have an interrupt pending.  Thus, HW_TICKS may exceed
         * kernel_data->HW_TICKS_PER_TICK and this tick_ptr may need
         * normalizing.  This is done in a moment.
         */
        tick_ptr->HW_TICKS = (*kernel_data->GET_HWTICKS)(kernel_data->GET_HWTICKS_PARAM);
    } /* Endif */

    PSP_ADD_TICKS(tick_ptr, &kernel_data->TIME_OFFSET, tick_ptr);

    /* The timer ISR may go off and increment kernel_data->TIME */
    _INT_ENABLE();

    /* The tick_ptr->HW_TICKS value might exceed the
     * kernel_data->HW_TICKS_PER_TICK and need to be
     * normalized for the PSP.
     */
    PSP_NORMALIZE_TICKS(tick_ptr);

} /* Endbody */
#endif /* MQX_HAS_TICK */

/*!
 * \brief Gets the calculated number of microseconds since the last periodic
 * timer interrupt.
 *
 * \return Number of microseconds since the last periodic timer interrupt.
 * \return 0 (BSP does not support the feature.)
 *
 * \warning Resolution depends on the periodic timer device.
 *
 * \see _time_get_elapsed
 * \see _time_get_elapsed_ticks
 * \see _time_get
 * \see _time_get_ticks
 * \see _time_set_
 * \see _time_set_ticks
 */
uint16_t _time_get_microseconds
(
    void
)
{
    uint32_t hwticks;
    uint32_t tickper;

    KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);

    /* BSP does not support HW ticks */
    if (kernel_data->GET_HWTICKS == NULL) return 0;

    hwticks = (*kernel_data->GET_HWTICKS)(kernel_data->GET_HWTICKS_PARAM);

    /* Tick period in time units (us) */
    tickper = 1000000 / kernel_data->TICKS_PER_SECOND; /* 32bit div */

    /* The result shall fit into 16 bits */
    return (uint16_t)((hwticks * tickper) / kernel_data->HW_TICKS_PER_TICK); /* 32bit mul and div */
}

#if MQX_HAS_HW_TICKS
/*!
 * \brief Gets the number of hardware ticks per tick.
 *
 * \return Number of hardware ticks per tick.
 *
 * \see _time_set_hwticks_per_tick
 * \see _time_get_hwticks
 */
uint32_t _time_get_hwticks_per_tick
(
    void
)
{ /* Body */
    register KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);

    return kernel_data->HW_TICKS_PER_TICK;

} /* Endbody */
#endif /* MQX_HAS_HW_TICKS */

/*!
 * \brief Initializes a tick time structure with the number of ticks.
 *
 * \param[out] tick_ptr Pointer to the tick time structure to initialize.
 * \param[in]  ticks    Number of ticks with which to initialize the structure.
 *
 * \return MQX_OK
 * \return MQX_INVALID_PARAMETER (Tick_ptr is NULL.)
 *
 * \see _time_set
 * \see _time_set_ticks
 * \see MQX_TICK_STRUCT
 */
_mqx_uint _time_init_ticks
(
    MQX_TICK_STRUCT_PTR tick_ptr,
    _mqx_uint           ticks
)
{ /* Body */

#if MQX_CHECK_ERRORS
    if (tick_ptr == NULL) {
        return MQX_INVALID_PARAMETER;
    } /* Endif */
#endif

    if (ticks) {
        tick_ptr->HW_TICKS = 0;
        PSP_ADD_TICKS_TO_TICK_STRUCT(&_mqx_zero_tick_struct, ticks, tick_ptr);
    }
    else {
        *tick_ptr = _mqx_zero_tick_struct;
    } /* Endif */

    return MQX_OK;

} /* Endbody */

#if MQX_HAS_TICK
/*!
 * \brief The BSP periodic timer ISR calls the function when a periodic timer
 * interrupt occurs.
 *
 * The BSP installs an ISR for the periodic timer interrupt. The ISR calls
 * _time_notify_kernel(), which does the following:
 * \li Increments kernel time.
 * \li If the active task is a time slice task whose time slice has expired,
 * puts it at the end of the task's ready queue.
 * \li If the timeout has expired for tasks on the timeout queue, puts them in
 * their ready queues.
 * \n If the BSP does not have periodic timer interrupts, MQX components that
 * use time will not operate.
 *
 * \warning See Description.
 *
 * \see _time_get_elapsed
 * \see _time_get_elapsed_ticks
 * \see _time_get
 * \see _time_get_ticks
 * \see _time_set
 * \see _time_set_ticks
 * \see TIME_STRUCT
 */
void _time_notify_kernel
(
    void
)
{ /* Body */
    register KERNEL_DATA_STRUCT_PTR kernel_data;
    register TD_STRUCT_PTR td_ptr;
    register TD_STRUCT_PTR next_td_ptr;
    register _mqx_uint count;
    register _mqx_int result;

    _GET_KERNEL_DATA(kernel_data);

    /*
     * Update the current time.
     */
    PSP_INC_TICKS(&kernel_data->TIME);

    _INT_DISABLE();

    if (kernel_data->GET_HWTICKS) {
        /* The hardware clock may have counted passed it's reference
         * and have an interrupt pending.  Thus, HW_TICKS may exceed
         * kernel_data->HW_TICKS_PER_TICK and this tick_ptr may need
         * normalizing.  This is done in a moment.
         */
        kernel_data->TIME.HW_TICKS = (*kernel_data->GET_HWTICKS)(kernel_data->GET_HWTICKS_PARAM);
    } /* Endif */

    /* The tick_ptr->HW_TICKS value might exceed the
     * kernel_data->HW_TICKS_PER_TICK and need to be
     * normalized for the PSP.
     */
    PSP_NORMALIZE_TICKS(&kernel_data->TIME);

    /*
     * Check for tasks on the timeout queue, and wake the appropriate
     * ones up.  The timeout queue is a time-priority queue.
     */
    count = _QUEUE_GET_SIZE(&kernel_data->TIMEOUT_QUEUE);
    if (count) {
        td_ptr = (TD_STRUCT_PTR)((void *) kernel_data->TIMEOUT_QUEUE.NEXT);
        while (count--) {
            next_td_ptr = td_ptr->TD_NEXT;
            result = PSP_CMP_TICKS(&kernel_data->TIME, &td_ptr->TIMEOUT);
            if (result >= 0) {
                --kernel_data->TIMEOUT_QUEUE.SIZE;
                _QUEUE_UNLINK(td_ptr);
                /* td_ptr->STATE &= ~IS_ON_TIMEOUT_Q; //not necessary; we will set the STATE soon */
                if (td_ptr->STATE & TD_IS_ON_AUX_QUEUE) {
                    /* td_ptr->STATE &= ~TD_IS_ON_AUX_QUEUE; //not necessary; we will set the STATE soon */
                    _QUEUE_REMOVE(td_ptr->INFO, &td_ptr->AUX_QUEUE);
                } /* Endif */
                _TASK_READY(td_ptr, kernel_data); /* this sets the STATE to READY */
            }
            else {
                break; /* No more to do */
            } /* Endif */
            td_ptr = next_td_ptr;
        } /* Endwhile */
    } /* Endif */

#if MQX_HAS_TIME_SLICE
    /*
     * Check if the currently running task is a time slice task
     * and if its time has expired, put it at the end of its queue
     */
    td_ptr = kernel_data->ACTIVE_PTR;
    if ( td_ptr->FLAGS & MQX_TIME_SLICE_TASK ) {
        PSP_INC_TICKS(&td_ptr->CURRENT_TIME_SLICE);
        if (! (td_ptr->FLAGS & TASK_PREEMPTION_DISABLED) ) {
            result = PSP_CMP_TICKS(&td_ptr->CURRENT_TIME_SLICE, &td_ptr->TIME_SLICE);
            if ( result >= 0 ) {
                _QUEUE_UNLINK(td_ptr);
                _TASK_READY(td_ptr,kernel_data);
            } /* Endif */
        } /* Endif */
    } /* Endif */
#endif

    _INT_ENABLE();
#if MQX_USE_TIMER
    /* If the timer component needs servicing, call its ISR function */
    if (kernel_data->TIMER_COMPONENT_ISR != NULL) {
        (*kernel_data->TIMER_COMPONENT_ISR)();
    }/* Endif */
#endif

#if MQX_USE_LWTIMER
    /* If the lwtimer needs servicing, call its ISR function */
    if (kernel_data->LWTIMER_ISR != NULL) {
        (*kernel_data->LWTIMER_ISR)();
    }/* Endif */
#endif

} /* Endbody */

#endif /* MQX_HAS_TICK */

#if MQX_HAS_TICK
/*!
 * \brief Set the absolute time in second/millisecond time. The input time is
 * the UCT time.
 *
 * The function affects _time_get() (and _time_get_ticks()), but does not affect
 * time _time_get_elapsed() (or _time_get_elapsed_ticks()).
 *
 * \param[in] ts_ptr Pointer to a structure that contains the new normalized
 * time in second/millisecond time.
 *
 * \see _time_set_ticks
 * \see _time_get
 * \see _time_get_ticks
 * \see _time_get_elapsed
 * \see _time_get_elapsed_ticks
 * \see _time_to_date
 * \see _time_init_ticks
 * \see _time_to_ticks
 * \see _time_from_date
 * \see TIME_STRUCT
 * \see MQX_TICK_TIME
 */
void _time_set
(
    register TIME_STRUCT_PTR ts_ptr
)
{ /* Body */
    register KERNEL_DATA_STRUCT_PTR kernel_data;
    MQX_TICK_STRUCT ticks;

    _GET_KERNEL_DATA(kernel_data);

    _KLOGE4(KLOG_time_set, ts_ptr, ts_ptr->SECONDS, ts_ptr->MILLISECONDS);

    /* Normalize time */
    MQX_NORMALIZE_TIME_STRUCT(ts_ptr);

    /* First convert old time struct into the tick struct */
    PSP_TIME_TO_TICKS(ts_ptr, &ticks);

    _INT_DISABLE();

    /* Calculate offset */
    PSP_SUB_TICKS(&ticks, &kernel_data->TIME, &kernel_data->TIME_OFFSET);

    _INT_ENABLE();

    _KLOGX1(KLOG_time_set);

} /* Endbody */
#endif /* MQX_HAS_TICK */

#if MQX_HAS_TICK
/*!
 * \brief Set the absolute time in tick time. The input time is the UCT time.
 *
 * The function affects _time_get_ticks() (and _time_get()), but does not affect
 * time _time_get_elapsed_ticks() (or _time_get_elapsed()).
 *
 * \param[in] ticks Pointer to the structure that contains the new time in tick
 * time.
 *
 * \see _time_set
 * \see _time_get
 * \see _time_get_ticks
 * \see _time_get_elapsed
 * \see _time_get_elapsed_ticks
 * \see _time_to_date
 * \see _time_init_ticks
 * \see _time_to_ticks
 * \see _time_from_date
 * \see TIME_STRUCT
 * \see MQX_TICK_TIME
 */
void _time_set_ticks
(
    register MQX_TICK_STRUCT_PTR ticks
)
{ /* Body */
    register KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);

    _KLOGE2(KLOG_time_set_ticks, ticks);

    _INT_DISABLE();

    PSP_SUB_TICKS(ticks, &kernel_data->TIME, &kernel_data->TIME_OFFSET);

    _INT_ENABLE();

    _KLOGX1(KLOG_time_set_ticks);

} /* Endbody */
#endif /* MQX_HAS_TICK */

/*!
 * \brief Determines whether given year is a leap year.
 *
 * \param[in] year The year to check.
 *
 * \return TRUE (Year is a leap year.), FALSE (Year is not a leap year.)
 */
bool _time_check_if_leap
(
    uint16_t year
)
{ /* Body */
    bool leap;

    /*
     * If the year is a century year not divisible by 400
     * then it is not a leap year, otherwise if year divisible by
     * four then it is a leap year
     */
    if (year % (uint16_t) 100 == (uint16_t) 0) {
        if (year % (uint16_t) 400 == (uint16_t) 0) {
            leap = TRUE;
        }
        else {
            leap = FALSE;
        } /* Endif */
    }
    else {
        if (year % (uint16_t) 4 == (uint16_t) 0) {
            leap = TRUE;
        }
        else {
            leap = FALSE;
        } /* Endif */

    } /* Endif */

    return leap;
} /* Endbody */


#if MQX_HAS_TICK
/*!
 * \brief Sets the resolution of the periodic timer interrupt.
 *
 * On each clock interrupt, MQX increments time by the resolution.
 *
 * \warning If the resolution does not agree with the interrupt period that was
 * programmed at the hardware level, some timing functions will give incorrect
 * results.
 *
 * \param[in] resolution Periodic timer resolution (in milliseconds) to be used
 * by MQX.
 *
 * \return MQX_OK
 * \return MQX_INVALID_PARAMETER (input resolution is eaqual to 0 or greater than 1000 milliseconds)
 *
 * \see _time_get_resolution
 * \see _time_get_elapsed
 * \see _time_get_elapsed_ticks
 * \see _time_get
 * \see _time_get_ticks
 * \see _time_set
 * \see _time_set_ticks
 * \see TIME_STRUCT
 */
_mqx_uint _time_set_resolution
(
    _mqx_uint resolution
)
{ /* Body */
    register KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);

    /* Verify value input resolution to prevent from dividing by zero */
    if ((resolution == 0) || (resolution > 1000)) return MQX_INVALID_PARAMETER;

    /*
     * Convert resolution into ticks per second so new tick format will
     * work
     */
    kernel_data->TICKS_PER_SECOND = 1000 / resolution;

    /* Also set hw ticks per tick */
    kernel_data->HW_TICKS_PER_TICK = resolution * 1000;

    return MQX_OK;

} /* Endbody */
#endif /* MQX_HAS_TICK */

#if MQX_HAS_TICK
/*!
 * \brief Sets the timer frequency (in ticks per second) that MQX uses.
 *
 * \param[in] ticks_per_sec New timer frequency in ticks per second.
 *
 * \warning If the timer frequency does not agree with the interrupt period that
 * was programmed at the hardware level, some timing functions will give
 * incorrect results.
 *
 * \see _time_get_ticks_per_sec
 */
void _time_set_ticks_per_sec
(
    _mqx_uint ticks_per_sec
)
{ /* Body */
    register KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);

    kernel_data->TICKS_PER_SECOND = ticks_per_sec;

} /* Endbody */

#endif /* MQX_HAS_TICK */

#if MQX_HAS_TICK
/*!
 * \brief Sets the internal periodic timer interrupt vector number that MQX uses.
 *
 * The BSP should call the function during initialization.
 *
 * \param[in] vector Periodic timer interrupt vector to use.
 *
 * \see _time_get
 * \see _time_get_ticks
 * \see _time_get_resolution
 * \see _time_set_resolution
 */
void _time_set_timer_vector
(
    _mqx_uint vector
)
{ /* Body */
    register KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);
    kernel_data->SYSTEM_CLOCK_INT_NUMBER = vector;

} /* Endbody */
#endif /* MQX_HAS_TICK */

#if MQX_HAS_HW_TICKS
/*!
 * \brief Sets the fields in kernel data to get the hardware ticks.
 *
 * \param[in] hwtick_function_ptr Pointer to the function that returns hw tick,
 * to be executed by the kernel.
 * \param[in] parameter           Parameter of the function that returns hw tick.
 */
void _time_set_hwtick_function
(
    MQX_GET_HWTICKS_FPTR hwtick_function_ptr,
    void                *parameter
)
{ /* Body */
    register KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);

    kernel_data->GET_HWTICKS = hwtick_function_ptr;
    kernel_data->GET_HWTICKS_PARAM = parameter;

} /* Endbody */
#endif /* MQX_HAS_HW_TICKS */

#if MQX_HAS_HW_TICKS
/*!
 * \brief Sets the number of hardware ticks per tick.
 *
 * \param[in] new_val New number of hardware ticks per tick.
 *
 * \see _time_get_hwticks_per_tick
 * \see _time_get_hwticks
 */
void _time_set_hwticks_per_tick
(
    uint32_t new_val
)
{ /* Body */
    register KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);

    kernel_data->HW_TICKS_PER_TICK = new_val;

} /* Endbody */

#endif /* MQX_HAS_HW_TICKS */


/*!
 * \brief Converts seconds/msecs value into a date and time from Jan.1 1970.
 *
 * The function verifies that the fields in the input structure are within the
 * following ranges.
 * <table>
 *   <tr>
 *     <td><b>Field</b></td>
 *     <td><b>Minimum</b></td>
 *     <td><b>Maximum</b></td>
 *   </tr>
 *   <tr>
 *     <td><b>SECONDS</b></td>
 *     <td>0</td>
 *     <td>MAXIMUM_SECONDS_IN_TIME (4,102,444,800)</td>
 *   </tr>
 *   <tr>
 *     <td><b>MILLISECONDS</b></td>
 *     <td>0</td>
 *     <td>999</td>
 *   </tr>
 * </table>
 *
 * The function converts the fields in the input structure to the fields in the
 * output structure, taking into account leap years.
 * \n The time is since 0:00:00.00, January 1, 1970.
 *
 * \param[in] ts_ptr   Pointer to time structure.
 * \param[in] date_ptr Pointer to a date/time structure.
 *
 * \return TRUE (Success), FALSE (Failure: ts_ptr or date_ptr is NULL or ts_ptr
 * is out of range).
 *
 * \see _time_get
 * \see _time_get_ticks
 * \see _time_get_elapsed
 * \see _time_get_elapsed_ticks
 * \see _time_set
 * \see _time_set_ticks
 * \see _time_from_date
 * \see DATE_STRUCT
 * \see TIME_STRUCT
 */
bool _time_to_date
(
    TIME_STRUCT_PTR ts_ptr,
    DATE_STRUCT_PTR date_ptr
)
{ /* Body */
    uint32_t   time;
    uint32_t   day, year, tmp;
    _mqx_uint  leap;

#if MQX_CHECK_ERRORS
    if ((ts_ptr == NULL) || (date_ptr == NULL)) {
        return (FALSE);
    } /* Endif */

    if ( (MAXIMUM_SECONDS_IN_TIME - ts_ptr->MILLISECONDS/1000) < ts_ptr->SECONDS) {
        return (FALSE);
    }
#endif

    /* Normalize MILLISECONDS field */
    ts_ptr->SECONDS += ts_ptr->MILLISECONDS / 1000;
    ts_ptr->MILLISECONDS %= 1000;

    time = ts_ptr->SECONDS;

    /* Number of days */
    day = time / 86400U;

    /* Remain Seconds */
    time -= day * 86400U;

    /* Calculate the hour */
    date_ptr->HOUR = (uint16_t)(time / 3600U);
    time -= ((uint32_t) date_ptr->HOUR * 3600U);

    /* Calculate the minute */
    date_ptr->MINUTE = (uint16_t)(time / 60);
    time -= ((uint32_t) date_ptr->MINUTE * 60);

    /* The second */
    date_ptr->SECOND = (uint16_t) time;

    /* The millisecond */
    date_ptr->MILLISEC = (uint16_t) ts_ptr->MILLISECONDS;

    /* Day of week -  1/1/1970 is Thursday */
    date_ptr->WDAY = (uint16_t) ((day + 4) % 7);

    /* Calculate the year */
    day += 365;                    /* Add offset day, the year starts from 1969 */
    year = 4 * (day / 1461);       /* The number days in each four years is 1461 (days) */
    day -= year * 1461 / 4;        /* The remain days */
    tmp = day / 365;               /* Remain years */
    if (tmp == 4) tmp = 3;         /* The maximum of remain years is equal to 3 */
    day -= tmp * 365;              /* remain days of this year */
    date_ptr->YEAR = year + tmp + CLK_FIRST_YEAR - 1; /* CLK_FIRST_YEAR is 1970 */

    /* Day of year */
    date_ptr->YDAY = day;

    /* calculate the month */
    /* Find out if we are in a leap year. */
    leap = (_mqx_uint) _time_check_if_leap(date_ptr->YEAR);
    date_ptr->MONTH = 1;

    while (day >= _time_days_in_month_internal[leap][date_ptr->MONTH])
    {
        day -= _time_days_in_month_internal[leap][date_ptr->MONTH];
        date_ptr->MONTH ++;
    }

    /* calculate the day */
    date_ptr->DAY = day ;
    /* first day is 1*/
    date_ptr->DAY++;

    return (TRUE);

} /* Endbody */


/*!
 * \brief Converts second/millisecond time format to tick time format.
 *
 * The function verifies that the fields in the input structure are within the
 * following ranges.
 * <table>
 *   <tr>
 *     <td><b>Field</b></td>
 *     <td><b>Minimum</b></td>
 *     <td><b>Maximum</b></td>
 *   </tr>
 *   <tr>
 *     <td><b>SECONDS</b></td>
 *     <td>0</td>
 *     <td>MAXIMUM_SECONDS_IN_TIME (4,102,444,800)</td>
 *   </tr>
 *   <tr>
 *     <td><b>MILLISECONDS</b></td>
 *     <td>0</td>
 *     <td>999</td>
 *   </tr>
 * </table>
 *
 * The function converts the fields in the input structure to the fields in the
 * output structure, taking into account leap years.
 *
 * \param[in]  ts_ptr Pointer to a normalized second/millisecond time structure.
 * \param[out] tick_ptr Pointer to the corresponding tick time structure.
 *
 * \return TRUE (Success), FALSE (Failure: tick_ptr or ts_ptr is NULL).
 *
 * \see _ticks_to_time
 * \see MQX_TICK_STRUCT
 * \see TIME_STRUCT
 */
bool _time_to_ticks
(
    TIME_STRUCT_PTR     ts_ptr,
    MQX_TICK_STRUCT_PTR tick_ptr
)
{ /* Body */

#if MQX_CHECK_ERRORS
    if ((tick_ptr == NULL) || (ts_ptr == NULL)) {
        return (FALSE);
    } /* Endif */
#endif

#if MQX_CHECK_ERRORS
    if ((ts_ptr->SECONDS > MAXIMUM_SECONDS_IN_TIME) ||
                    (ts_ptr->MILLISECONDS > 999))
    {
        return( FALSE );
    } /* Endif */
#endif

    PSP_TIME_TO_TICKS(ts_ptr, tick_ptr);

    return (TRUE);

} /* Endbody */

/*!
 * \brief Converts tick format to second/millisecond format.
 *
 * The function verifies that the fields in the input structure are within the
 * following ranges.
 * <table>
 *   <tr>
 *     <td><b>Field</b></td>
 *     <td><b>Minimum</b></td>
 *     <td><b>Maximum</b></td>
 *   </tr>
 *   <tr>
 *     <td><b>TICKS</b></td>
 *     <td>0</td>
 *     <td>(2^64)-1</td>
 *   </tr>
 *   <tr>
 *     <td><b>HW_TICKS</b></td>
 *     <td>0</td>
 *     <td>(2^32)-1</td>
 *   </tr>
 * </table>
 *
 * \param[in]  tick_ptr Pointer to a tick structure.
 * \param[out] ts_ptr Pointer to the corresponding normalized
 * second/millisecond time structure.
 *
 * \return TRUE (Success), FALSE (Failure: tick_ptr or ts_ptr is NULL
 * or overflow occurs).
 *
 * \see _time_to_ticks
 * \see MQX_TICK_STRUCT
 * \see TIME_STRUCT
 */
bool _ticks_to_time
(
    MQX_TICK_STRUCT_PTR tick_ptr,
    TIME_STRUCT_PTR     ts_ptr
)
{ /* Body */

#if MQX_CHECK_ERRORS
    if ((tick_ptr == NULL) || (ts_ptr == NULL)) {
        return (FALSE);
    } /* Endif */
#endif
    return PSP_TICKS_TO_TIME(tick_ptr, ts_ptr);

} /* Endbody */

#if MQX_STD_TIME_API
/*!
 * \brief Converts the broken-down time structure, expressed
 * as local time, to calendar time representation.
 *
 * \param[in] tm_ptr pointer to a tm structure
 *
 * \return time vaule(which is converted from tm struct),
 * 0(Failure: tm_ptr is NULL or overflow occurs),
 *
 * \see gmtime_r
 * \see timegm
 * \see localtime_r
 * \see time_t
 * \see struct tm
 */
time_t mktime
(
    struct tm* tm_ptr
)
{ /* Body */
    TIME_STRUCT     ts;
    DATE_STRUCT     date;

#if MQX_CHECK_ERRORS
    if (tm_ptr == NULL) {
        return (0);
    } /* End if */
#endif
    /*
     * Convert tm struct to DATE_STRUCT
     */
    date.MILLISEC = 0; /* tm struct doesn't support millisecond*/

    date.SECOND = (int32_t)tm_ptr->tm_sec;

    date.MINUTE = (int32_t)tm_ptr->tm_min;

    date.HOUR   = (int32_t)tm_ptr->tm_hour;

    /* The first day of month in DATE_STRUCT is 1, while in tm struct is 0*/
    date.DAY    = (int32_t)tm_ptr->tm_mday + 1;

    /* The first month of year in DATE_STRUCT is 1, while in tm struct is 0*/
    date.MONTH  = (int32_t)tm_ptr->tm_mon  + 1;

    /* The first year in DATE_STRUCT is 0, while in tm struct is 1990*/
    date.YEAR   = (int32_t)tm_ptr->tm_year + 1900;

    /*
     * Convert DATE_STRUCT to TIME_STRUCT
     */
    if (_time_from_date(&date, &ts) == FALSE) {
        return 0;
    } /* End if */

    /* Verify that time is valid */
    if ( (MAXIMUM_SECONDS_IN_TIME - _timezone) < ts.SECONDS) {
        return 0;
    } /* End if */

    /* Update week of day */
    tm_ptr->tm_wday = (int32_t) date.WDAY;

    /* Update year of day */
    tm_ptr->tm_yday = (int32_t) date.YDAY;

    /* Convert TIME_STRUCT to time_t */
    return (ts.SECONDS + _timezone);

} /* Endbody */

/*!
 * \brief  converts the calendar time timep to
 *  broken-down time representation, expressed
 *  in Coordinated Universal Time (UTC)
 *
 * \param[in] time_t pointer to time_t.
 * \param[out] result pointer to tm struct.
 *
 * \return NULL when timep or result is NULL or
 * overflow occurs, pointer to user-supplied struct
 * if SUCCESS
 *
 * \see mktime
 * \see timegm
 * \see localtime_r
 * \see time_t
 * \see struct tm
 */
 struct tm *gmtime_r
(
    const  time_t *timep,
    struct tm     *result
)
{ /* Body */
    TIME_STRUCT ts;
    DATE_STRUCT date;

#if MQX_CHECK_ERRORS
    if ( (timep == NULL) || (result == NULL) ) {
        return (NULL);
    }
#endif

    /*
     * Convert time_t to TIME_STRUCT
     */
    ts.MILLISECONDS = 0; /* time_t doesn't support milisecond */
    ts.SECONDS = *timep;

    /*
     * Convert TIME_STRUCT to DATE_STRUCT
     */
    if (_time_to_date(&ts, &date) == FALSE) {
        return 0;
    } /* End if */

    /*
     * Convert DATE_STRUCT to tm struct
     */

    result->tm_sec = (int32_t)date.SECOND;

    result->tm_min = (int32_t)date.MINUTE;

    result->tm_hour = (int32_t)date.HOUR;

    /* The first day of month in DATE_STRUCT is 1, while in tm struct is 0*/
    result->tm_mday = (int32_t)date.DAY - 1;

    /* The first month of year in DATE_STRUCT is 1, while in tm struct is 0*/
    result->tm_mon = (int32_t)date.MONTH -1;

    /* The first year in DATE_STRUCT is 0, while in tm struct is 1990*/
    result->tm_year = (int32_t)date.YEAR - 1900;

    result->tm_wday = (int32_t)date.WDAY;

    result->tm_yday = (int32_t)date.YDAY;

    return result;

} /* Endbody */

/*!
 * \brief Converts the broken-down time structure, expressed
 * as UTC time, to calendar time representation.
 *
 * \param[in] tm_ptr pointer to tm struct.
 *
 * \return 0 when tm_ptr is NULL or
 * overflow occurs, epoch value if SUCCESS
 *
 * \see mktime
 * \see gmtime_r
 * \see localtime_r
 * \see time_t
 * \see struct tm
 */
time_t timegm
(
    struct tm *tm_ptr
)
{ /* Body */
    TIME_STRUCT     ts;
    DATE_STRUCT     date;

#if MQX_CHECK_ERRORS
    if (tm_ptr == NULL) {
        return (0);
    }
#endif
    /*
     * Convert tm struct to DATE_STRUCT
     */

    date.MILLISEC = 0; /* tm struct doesn't support millisecond*/

    date.SECOND = (int32_t)tm_ptr->tm_sec;

    date.MINUTE = (int32_t)tm_ptr->tm_min;

    date.HOUR   = (int32_t)tm_ptr->tm_hour;

    /* The first day of month in DATE_STRUCT is 1, while in tm struct is 0*/
    date.DAY    = (int32_t)tm_ptr->tm_mday + 1;

    /* The first month of year in DATE_STRUCT is 1, while in tm struct is 0*/
    date.MONTH  = (int32_t)tm_ptr->tm_mon  + 1;

    /* The first year in DATE_STRUCT is 1900, while in tm struct is 0*/
    date.YEAR   = (int32_t)tm_ptr->tm_year + 1900;

    /*
     * Convert DATE_STRUCT to TIME_STRUCT
     */
    if (_time_from_date(&date, &ts) == FALSE) {
        return 0;
    } /* End if */

    /* Convert TIME_STRUCT to time_t */
    return ts.SECONDS;

} /* Endbody */

/*!
 * \brief  converts the calendar time timep to
 *  broken-down time representation, expressed
 *  in local time.
 *
 * \param[in] time_t pointer to time_t.
 * \param[out] result pointer to tm struct.
 *
 * \return NULL when timep or result is NULL or
 * overflow occurs, pointer to user-supplied struct
 * if SUCCESS
 *
 * \see mktime
 * \see timegm
 * \see localtime_r
 * \see time_t
 * \see struct tm
 */
struct tm *localtime_r
(
    const time_t *timep,
    struct tm    *result
)
{ /* Body */
    time_t local;

#if MQX_CHECK_ERRORS
    if (timep == NULL) {
        return (NULL);
    } /* End if */

    if ( ((_timezone > 0) && (*timep < _timezone)) ||
        ((_timezone < 0) && (*timep > MAXIMUM_SECONDS_IN_TIME + _timezone)) )
    {
        return NULL;
    } /* End if */
#endif

    local = *timep - _timezone;

    return gmtime_r(&local, result);
} /* Endbody */

#endif /* MQX_STD_TIME_API */
/* EOF */
