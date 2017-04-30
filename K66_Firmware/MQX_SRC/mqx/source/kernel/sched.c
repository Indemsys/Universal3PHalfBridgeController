
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
*   This file contains functions of the Scheduler component.
*
*
*END************************************************************************/

#include "mqx_inc.h"

/*!
 * \brief Gets the maximum priority that a task can have.
 *
 * This function always returns 0, the highest priority a task may have under MQX.
 * \n POSIX compatibility requires this function and the parameter.
 *
 * \param[in] policy Not used, all task priorities are same for RR or FIFO.
 *
 * \return 0 (Always.)
 *
 * \see _sched_get_min_priority
 */
_mqx_uint _sched_get_max_priority
(
    _mqx_uint policy
)
{ /* Body */

    return (0);

} /* Endbody */

/*!
 * \brief Gets the minimum priority that an application task can have.
 *
 * POSIX compatibility requires this function and the parameter.
 * \n The minimum priority that a task can be is set when MQX starts; it is the
 * priority of the lowest-priority task in the task template list.
 *
 * \param[in] policy Not used.
 *
 * \return Minimum priority that an application task can be (the numerical value
 * one less than the priority of Idle Task).
 *
 * \see _sched_get_max_priority
 */
_mqx_uint _sched_get_min_priority
(
    _mqx_uint policy
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);

    return (kernel_data->LOWEST_TASK_PRIORITY);

} /* Endbody */

#if !MQX_LITE_VERSION_NUMBER
/*!
 * \brief Sets the scheduling policy for a task or the system.
 *
 * \param[in] task_id One of the following:
 * \n - Task on this processor for which to get info.
 * \n - MQX_DEFAULT_TASK_ID (Set the policy for the processor.)
 * \n - MQX_NULL_TASK_ID (Set the policy for the calling task.)
 * \param[in] policy  New scheduling policy; one of the following:
 * \n - MQX_SCHED_FIFO
 * \n - MQX_SCHED_RR
 *
 * \return Previous scheduling policy MQX_SCHED_FIFO or MQX_SCHED_RR (Success.)
 * \return MAX_MQX_UINT (Failure.)
 *
 * \warning On failure, _task_set_error() is called to set the following task
 * error codes:
 * \n - MQX_SCHED_INVALID_POLICY (Policy is not one of the allowed policies.)
 * \n - MQX_SCHED_INVALID_TASK_ID (Task_id is not a valid task on this processor.)
 *
 * \see _sched_get_policy
 * \see _task_set_error
 */
_mqx_uint _sched_set_policy
(
    _task_id  task_id,
    _mqx_uint policy
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR  kernel_data = NULL;
    (void)                  kernel_data; /* suppress 'unused variable' warning */
    TD_STRUCT_PTR           td_ptr = NULL;
    (void)                  td_ptr; /* suppress 'unused variable' warning */
    _mqx_uint               old_policy = MQX_SCHED_FIFO;

    _GET_KERNEL_DATA(kernel_data);

    _KLOGE3(KLOG_sched_set_policy, (_mqx_uint)task_id, policy);
#if MQX_HAS_TIME_SLICE

#if MQX_CHECK_ERRORS
    if (! ((policy == MQX_SCHED_FIFO) || (policy == MQX_SCHED_RR)))
    {
        _task_set_error(MQX_SCHED_INVALID_POLICY);
        _KLOGX3(KLOG_sched_set_policy, MAX_MQX_UINT, MQX_SCHED_INVALID_POLICY);
        return(MAX_MQX_UINT);
    } /* Endif */
#endif

    /* Handle default case */
    if (task_id == MQX_DEFAULT_TASK_ID)
    {
        old_policy = kernel_data->SCHED_POLICY;
        kernel_data->SCHED_POLICY = policy;
    }
    else
    {
        td_ptr = (TD_STRUCT_PTR)_task_get_td(task_id);
        if (td_ptr == NULL)
        {
            _task_set_error(MQX_SCHED_INVALID_TASK_ID);
            _KLOGX3(KLOG_sched_set_policy, MAX_MQX_UINT, MQX_SCHED_INVALID_TASK_ID);
            return(MAX_MQX_UINT);
        } /* Endif */
        if (td_ptr->FLAGS & MQX_TIME_SLICE_TASK)
        {
            old_policy = MQX_SCHED_RR;
        }
        else
        {
            old_policy = MQX_SCHED_FIFO;
        } /* Endif */
        _int_disable();
        if (policy == MQX_SCHED_RR)
        {
            td_ptr->FLAGS |= MQX_TIME_SLICE_TASK;
        }
        else
        {
            td_ptr->FLAGS &= ~MQX_TIME_SLICE_TASK;
        } /* Endif */
        _int_enable();
    } /* Endif */
#else

#if MQX_CHECK_ERRORS
    if (policy != MQX_SCHED_FIFO)
    {
        _task_set_error(MQX_SCHED_INVALID_POLICY);
        _KLOGX3(KLOG_sched_set_policy, MAX_MQX_UINT, MQX_SCHED_INVALID_POLICY);
        return (MAX_MQX_UINT);
    } /* Endif */
    old_policy = MQX_SCHED_FIFO;
#endif

#endif

    _KLOGX3(KLOG_sched_set_policy, old_policy, 0L);
    return (old_policy);

} /* Endbody */

/*!
 * \brief Gets the scheduling policy for a task or the system.
 *
 * \param[in]  task_id    One of the following:
 * \n - Task on this processor for which to get info.
 * \n - MQX_DEFAULT_TASK_ID (Get the policy for the processor.)
 * \n - MQX_NULL_TASK_ID (Get the policy for the calling task.)
 * \param[out] policy_ptr Pointer to the scheduling policy:
 * \n - MQX_SCHED_FIFO (FIFO)
 * \n - MQX_SCHED_RR (Round robin)
 *
 * \return MQX_OK
 * \return MQX_SCHED_INVALID_TASK_ID (Task_id is not a valid task on this processor.)
 *
 * \see _sched_set_policy
 */
_mqx_uint _sched_get_policy
(
    _task_id      task_id,
    _mqx_uint_ptr policy_ptr
)
{ /* Body */
#if MQX_HAS_TIME_SLICE
    KERNEL_DATA_STRUCT_PTR kernel_data;
    TD_STRUCT_PTR          td_ptr;
    _mqx_uint              old_policy;

    _GET_KERNEL_DATA(kernel_data);

    /* Handle default case */
    if (task_id == MQX_DEFAULT_TASK_ID)
    {
        old_policy = kernel_data->SCHED_POLICY;
    }
    else
    {
        td_ptr = (TD_STRUCT_PTR)_task_get_td(task_id);
        if (td_ptr == NULL)
        {
            return(MQX_SCHED_INVALID_TASK_ID);
        } /* Endif */
        if (td_ptr->FLAGS & MQX_TIME_SLICE_TASK)
        {
            old_policy = MQX_SCHED_RR;
        }
        else
        {
            old_policy = MQX_SCHED_FIFO;
        } /* Endif */
    } /* Endif */

    *policy_ptr = old_policy;
#else
    *policy_ptr = MQX_SCHED_FIFO;
#endif

    return (MQX_OK);

} /* Endbody */

#if MQX_HAS_TIME_SLICE

/*!
 * \brief Get the time slice in milliseconds for the specified task.
 *
 * \param[in]     task_id One of the following:
 * \n - Task on this processor for which to get info.
 * \n - MQX_DEFAULT_TASK_ID (Get the time slice for the processor.)
 * \n - MQX_NULL_TASK_ID (Get the time slice for the calling task.)
 * \param[in,out] ms_ptr  Pointer to the time slice (in milliseconds).
 *
 * \return MQX_OK
 * \return MAX_UINT_32 (Failure.)
 *
 * \see _sched_get_rr_interval_ticks
 * \see _sched_set_rr_interval
 * \see _sched_set_rr_interval_ticks
 * \see _task_set_error
 */
uint32_t _sched_get_rr_interval
(
    _task_id   task_id,
    uint32_t *ms_ptr
)
{ /* Body */
    TIME_STRUCT     time;
    MQX_TICK_STRUCT tick;
    uint32_t         slice;

    if (_sched_get_rr_interval_ticks(task_id, &tick) != MQX_OK)
    {
        *ms_ptr = MAX_UINT_32;
        return MAX_UINT_32;
    } /* Endif */

    PSP_TICKS_TO_TIME(&tick, &time);

    if (time.SECONDS >= (MAX_UINT_32/1000))
    {
        *ms_ptr = MAX_UINT_32;
        return(MAX_UINT_32);
    } /* Endif */

    slice = time.SECONDS * 1000;
    if (slice >= (MAX_UINT_32 - time.MILLISECONDS))
    {
        *ms_ptr = MAX_UINT_32;
        return(MAX_UINT_32);
    } /* Endif */

    *ms_ptr = slice + time.MILLISECONDS;

    return(MQX_OK);

} /* Endbody */

/*!
 * \brief Get the time slice in tick time for the specified task.
 *
 * \param[in]     task_id  One of the following:
 * \n - Task on this processor for which to get info.
 * \n - MQX_DEFAULT_TASK_ID (Get the time slice for the processor.)
 * \n - MQX_NULL_TASK_ID (Get the time slice for the calling task.)
 * \param[in,out] tick_ptr Pointer to the time slice (in tick time).
 *
 * \return MQX_OK
 * \return MQX_SCHED_INVALID_PARAMETER_PTR (Time_ptr is NULL.)
 * \return MQX_SCHED_INVALID_TASK_ID (Task_id is not a valid task on this processor.)
 *
 * \warning On failure, calls _task_set_error() to set the task error codes (see
 * return Error Codes).
 *
 * \see _sched_get_rr_interval
 * \see _sched_set_rr_interval
 * \see _sched_set_rr_interval_ticks
 * \see _task_set_error
 * \see MQX_TICK_STRUCT
 */
_mqx_uint _sched_get_rr_interval_ticks
(
    _task_id            task_id,
    MQX_TICK_STRUCT_PTR tick_ptr
)
{ /* Body */
    register KERNEL_DATA_STRUCT_PTR kernel_data;
    TD_STRUCT_PTR                   td_ptr;

    _GET_KERNEL_DATA(kernel_data);

#if MQX_CHECK_ERRORS
    /* Validate parameters */
    if ( tick_ptr == NULL )
    {
        _task_set_error( MQX_SCHED_INVALID_PARAMETER_PTR );
        return( MQX_SCHED_INVALID_PARAMETER_PTR );
    } /* Endif */
#endif

    /* Handle default case */
    if ( task_id == MQX_DEFAULT_TASK_ID )
    {

        *tick_ptr = kernel_data->SCHED_TIME_SLICE;

    }
    else
    {

        td_ptr = (TD_STRUCT_PTR)_task_get_td( task_id );
        if ( td_ptr == NULL )
        {
            _task_set_error( MQX_SCHED_INVALID_TASK_ID );
            return( MQX_SCHED_INVALID_TASK_ID );
        } /* Endif */

        *tick_ptr = td_ptr->TIME_SLICE;

    } /* Endif */

    return( MQX_OK );

} /* Endbody */

/*!
 * \brief Set the time slice in milliseconds.
 *
 * \param[in] task_id     One of the following:
 * \n - Task ID for a task on this processor for which to set info.
 * \n - MQX_DEFAULT_TASK_ID (Set the time slice for the processor.)
 * \n - MQX_NULL_TASK_ID (Set the time slice for the calling task.)
 * \param[in] rr_interval New time slice (in milliseconds).
 *
 * \return old_rr_interval Previous time slice (Success.)
 * \return MAX_UINT_32
 *
 * \warning On failure, calls _task_set_error() to set the task error code to
 * MQX_SCHED_INVALID_TASK_ID.
 *
 * \see _sched_set_rr_interval_ticks
 * \see _sched_get_rr_interval
 * \see _sched_get_rr_interval_ticks
 * \see _task_set_error
 */
uint32_t _sched_set_rr_interval
(
    _task_id task_id,
    uint32_t  rr_interval
)
{ /* Body */
    _KLOGM(KERNEL_DATA_STRUCT_PTR kernel_data);
    uint32_t         old_rr_interval;
    MQX_TICK_STRUCT ticks;
    MQX_TICK_STRUCT old_ticks;
    _mqx_uint       result;

    _KLOGM(_GET_KERNEL_DATA(kernel_data));

    _KLOGE3(KLOG_sched_set_rr_interval, (_mqx_uint)task_id, rr_interval);

#if MQX_CHECK_ERRORS
    /* Validate parameters */
    if (0 == rr_interval)
    {
        _KLOGX2(KLOG_sched_set_rr_interval, MAX_UINT_32);
        _task_set_error( MQX_SCHED_INVALID_PARAMETER_PTR );
        return (MAX_UINT_32);
    } /* Endif */
#endif

    /* Compute the number of tick events required to accomplish the least amount of time[ms]. */
    /* tick_events = (required_time[ms] + (time_per_tick[ms] - 1)) / time_per_tick[ms])  -->
     * tick_events = ((required_time[ms] - 1) / time_per_tick[ms]) + 1
     */
    rr_interval--;
    /* Convert milliseconds to ticks, truncated */
    PSP_MILLISECONDS_TO_TICKS_QUICK(rr_interval, &ticks);
    /* Resolve truncation by adding one tick. */
    PSP_ADD_TICKS_TO_TICK_STRUCT(&ticks, 1, &ticks);

    result = _sched_set_rr_interval_internal(task_id, &ticks, &old_ticks);

    if (result != MQX_OK)
    {
        _task_set_error(result);
        _KLOGX2(KLOG_sched_set_rr_interval, MAX_UINT_32);
        return(MAX_UINT_32);
    } /* Endif */

    old_rr_interval = PSP_TICKS_TO_MILLISECONDS(&old_ticks, &result);

    _KLOGX2(KLOG_sched_set_rr_interval, old_rr_interval);

    return(old_rr_interval);

} /* Endbody */

/*!
 * \brief Set the time slice in tick time.
 *
 * \param[in]  task_id             One of the following:
 * \n - Task ID for a task on this processor for which to set info.
 * \n - MQX_DEFAULT_TASK_ID (Set the time slice for the processor.)
 * \n - MQX_NULL_TASK_ID (Set the time slice for the calling task.)
 * \param[in]  new_rr_interval_ptr Pointer to the new time slice (in tick time).
 * \param[out] old_rr_interval_ptr Pointer to the previous time slice (in tick time).
 *
 * \return Previous time slice (Success.)
 * \return MAX_UINT_32
 *
 * \warning On failure, calls _task_set_error() to set the task error code to
 * MQX_SCHED_INVALID_TASK_ID.
 *
 * \see _sched_set_rr_interval
 * \see _sched_get_rr_interval
 * \see _sched_get_rr_interval_ticks
 * \see _task_set_error
 * \see MQX_TICK_STRUCT
 */
_mqx_uint _sched_set_rr_interval_ticks
(
    _task_id            task_id,
    MQX_TICK_STRUCT_PTR new_rr_interval_ptr,
    MQX_TICK_STRUCT_PTR old_rr_interval_ptr

)
{ /* Body */
    _KLOGM(KERNEL_DATA_STRUCT_PTR kernel_data);
    _mqx_uint result;

    _KLOGM(_GET_KERNEL_DATA(kernel_data));

    _KLOGE2(KLOG_sched_set_rr_interval_ticks, task_id);

    result = _sched_set_rr_interval_internal(task_id, new_rr_interval_ptr,
                    old_rr_interval_ptr);

    if (result != MQX_OK)
    {
        _task_set_error(result);
    } /* Endif */

    _KLOGX2(KLOG_sched_set_rr_interval_ticks, result);

    return result;

} /* Endbody */

#endif /* MQX_HAS_TIME_SLICE */
#endif /* MQX_LITE_VERSION_NUMBER */

/*!
 * \brief Puts the active task at the end of its ready queue.
 *
 * This function effectively performs a timeslice. If there are no other tasks in
 * this ready queue, the task continues to be the active task.
 *
 * \warning Might dispatch another task.
 * \warning Cannot be called within interrupt, calls _task_set_error() to set the
 * following task error code:
 * \li MQX_CANNOT_CALL_FUNCTION_FROM_ISR
 */
void _sched_yield(void)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    TD_STRUCT_PTR          td_ptr;

    _GET_KERNEL_DATA(kernel_data);

    _KLOG(_klog_yield_internal();)

#if MQX_CHECK_ERRORS
    if (kernel_data->IN_ISR)
    {
        _task_set_error(MQX_CANNOT_CALL_FUNCTION_FROM_ISR);
        _KLOGX2(KLOG_sched_yield, MQX_CANNOT_CALL_FUNCTION_FROM_ISR);
        return;
    } /* Endif */
#endif /* MQX_CHECK_ERRORS */

    td_ptr = kernel_data->ACTIVE_PTR;

    _INT_DISABLE();
    _QUEUE_UNLINK(td_ptr);
    _TASK_READY(td_ptr, kernel_data);
    _sched_execute_scheduler_internal();
    _INT_ENABLE();

} /* Endbody */

#if MQX_HAS_DYNAMIC_PRIORITIES

/*!
 * \private
 *
 * \brief Boosts a task to a temporarily higher priority.
 *
 * This function MUST BE CALLED with DISABLED interrupts.
 *
 * \param[in] td_ptr   Pointer to the task descriptor whose priority is to change.
 * \param[in] priority The new task priority, MUST be a valid priority.
 *
 * \see TD_STRUCT
 */
void _sched_boost_priority_internal
(
    register TD_STRUCT_PTR td_ptr,
    register _mqx_uint     priority
)
{ /* Body */
    _KLOGM(KERNEL_DATA_STRUCT_PTR kernel_data);

    _KLOGM(_GET_KERNEL_DATA(kernel_data));
    _KLOGE4(KLOG_sched_boost_priority, td_ptr, priority, td_ptr->MY_QUEUE->PRIORITY);

    td_ptr->FLAGS |= TASK_PRIORITY_BOOSTED;
    ++td_ptr->BOOSTED;

    _sched_set_priority_internal(td_ptr, priority);

    /* Allow higher priority tasks to run */
    _CHECK_RUN_SCHEDULER();

    _KLOGX1(KLOG_sched_boost_priority);

} /* Endbody */

/*!
 * \private
 *
 * \brief Boosts a task to a temporarily higher priority.
 *
 * This function MUST BE CALLED with DISABLED interrupts.
 *
 * \param[in] td_ptr           Pointer to the task descriptor whose priority is
 * to change.
 * \param[in] number_of_boosts The number of times to 'unboost' the task.
 *
 * \see TD_STRUCT
 */
void _sched_unboost_priority_internal
(
    register TD_STRUCT_PTR td_ptr,
    register _mqx_uint     number_of_boosts
)
{ /* Body */
    _KLOGM(register KERNEL_DATA_STRUCT_PTR kernel_data);

    _KLOGM(_GET_KERNEL_DATA(kernel_data));
    _KLOGE5(KLOG_sched_unboost_priority, td_ptr, number_of_boosts, td_ptr->BOOSTED, td_ptr->HOME_QUEUE->PRIORITY);

    if (td_ptr->FLAGS & TASK_PRIORITY_BOOSTED)
    {
        if (td_ptr->BOOSTED > number_of_boosts)
        {
            td_ptr->BOOSTED = td_ptr->BOOSTED - number_of_boosts;
        }
        else
        {
            td_ptr->BOOSTED = 0;
            td_ptr->FLAGS &= ~TASK_PRIORITY_BOOSTED;
            _sched_set_priority_internal(td_ptr, (_mqx_uint) td_ptr->HOME_QUEUE->PRIORITY);
            /* Allow higher priority tasks to run */
            _CHECK_RUN_SCHEDULER();
        } /* Endif */
    } /* Endif */

    _KLOGX1(KLOG_sched_unboost_priority);

} /* Endbody */

#endif /* MQX_HAS_DYNAMIC_PRIORITIES */

/*!
 * \private
 *
 * \brief Inserts a task descriptor into a task descriptor queue by order of task
 * priority.
 *
 * \param[in] queue_ptr Pointer to the queue header.
 * \param[in] td_ptr    Pointer to the task descriptor to insert.
 *
 * \see QUEUE_STRUCT
 * \see TD_STRUCT
 */
void _sched_insert_priorityq_internal
(
    register QUEUE_STRUCT_PTR queue_ptr,
    register TD_STRUCT_PTR    td_ptr
)
{ /* Body */
    register TD_STRUCT_PTR td2_ptr;
    register TD_STRUCT_PTR td_prev_ptr;
    register _mqx_uint     priority;
    register _mqx_uint     count;

    td_prev_ptr = (TD_STRUCT_PTR) ((void *) queue_ptr);
    td2_ptr = (TD_STRUCT_PTR) ((void *) queue_ptr->NEXT);
    count = _QUEUE_GET_SIZE(queue_ptr) + 1;
    priority = td_ptr->MY_QUEUE->PRIORITY;
    while (--count)
    {
        if (td2_ptr->MY_QUEUE->PRIORITY > priority)
        {
            break;
        } /* Endif */
        td_prev_ptr = td2_ptr;
        td2_ptr = td2_ptr->TD_NEXT;
    } /* Endwhile */
    _QUEUE_INSERT(queue_ptr,
                    (QUEUE_ELEMENT_STRUCT_PTR)((void *)td_prev_ptr),
                    (QUEUE_ELEMENT_STRUCT_PTR)((void *)td_ptr));

} /* Endbody */

/*!
 * \private
 *
 * \brief Gets the maximum of the priorities of the tasks on the waiting queue.
 *
 * \param[in] queue_ptr Pointer to the queue header.
 *
 * \return The highest task priority from specified queue.
 *
 * \see QUEUE_STRUCT
 */
_mqx_uint _sched_get_max_priority_on_q_internal
(
    register QUEUE_STRUCT_PTR queue_ptr
)
{ /* Body */
    register TD_STRUCT_PTR td_ptr;
    register _mqx_uint     priority;
    register _mqx_uint     count;

    td_ptr = (TD_STRUCT_PTR) ((void *) queue_ptr->NEXT);
    count = _QUEUE_GET_SIZE(queue_ptr) + 1;
    priority = MAX_MQX_UINT;
    while (--count)
    {
        if (td_ptr->MY_QUEUE->PRIORITY < priority)
        {
            priority = td_ptr->MY_QUEUE->PRIORITY;
        } /* Endif */
        td_ptr = td_ptr->TD_NEXT;
    } /* Endwhile */
    return priority;

} /* Endbody */

/*!
 * \private
 *
 * \brief Puts the specified td onto the specified priority queue.
 *
 * This function MUST BE CALLED with DISABLED interrupts.
 *
 * \param[in] td_ptr Pointer to the task descriptor whose priority is to change.
 * \param[in] new_priority The new task priority, MUST be a valid priority.
 *
 * \see TD_STRUCT
 */
void _sched_set_priority_internal
(
    register TD_STRUCT_PTR td_ptr,
    register _mqx_uint     new_priority
)
{ /* Body */
    register KERNEL_DATA_STRUCT_PTR kernel_data;
    register READY_Q_STRUCT_PTR     ready_q_ptr;
    register _mqx_uint              old_priority;

    _GET_KERNEL_DATA(kernel_data);

    /* Find the new queue */
    ready_q_ptr = kernel_data->READY_Q_LIST;
    old_priority = td_ptr->MY_QUEUE->PRIORITY;

    ready_q_ptr = ready_q_ptr - new_priority;

    /*
      * Remove the current task from the head of the queue, and
      * place the task at the back of the specified queue.
      */
    td_ptr->TASK_SR = ready_q_ptr->ENABLE_SR;
    td_ptr->MY_QUEUE = ready_q_ptr;
    if (td_ptr->STATE == READY)
    {
        _QUEUE_UNLINK(td_ptr);
        _TASK_READY(td_ptr,kernel_data);
        if (kernel_data->ACTIVE_PTR == td_ptr)
        {
            /* Changing the current task's priority */
            kernel_data->ACTIVE_SR = td_ptr->TASK_SR;
            if (new_priority > old_priority)
            {
                /*
                  * Save state, and re-run the scheduler
                  * since the actice task's priority has been lowered,
                  * and now a higher priority task may be ready to run
                  */
                _sched_execute_scheduler_internal();
            } /* Endif */
        }
        else
        {
            /* May need to mod sr value of task on stack */
            if (td_ptr->DISABLED_LEVEL == 0)
            {
                _task_sync_priority_internal(td_ptr);
            } /* Endif */
        } /* Endif */
    }
    else
    {
        /* May need to mod sr value of task on stack */
        if (td_ptr->DISABLED_LEVEL == 0)
        {
            _task_sync_priority_internal(td_ptr);
        } /* Endif */
    } /* Endif */

} /* Endbody */

#if !MQX_LITE_VERSION_NUMBER
#if MQX_HAS_TIME_SLICE

/*!
 * \private
 *
 * \brief Gets the current time slice in tick time.
 *
 * \param[in]     task_id  One of the following:
 * \n - Task on this processor for which to get info.
 * \n - MQX_DEFAULT_TASK_ID (Get the time slice for the processor.)
 * \n - MQX_NULL_TASK_ID (Get the time slice for the calling task.)
 * \param[in,out] tick_ptr Pointer to the time slice (in tick time).
 *
 * \return MQX_OK
 * \return MQX_SCHED_INVALID_TASK_ID
 * \return MQX_SCHED_INVALID_PARAMETER_PTR
 *
 * \see _sched_get_rr_interval_ticks
 * \see MQX_TICK_STRUCT
 */
_mqx_uint _sched_get_rr_interval_internal
(
    _task_id            task_id,
    MQX_TICK_STRUCT_PTR tick_ptr
)
{ /* Body */
    register KERNEL_DATA_STRUCT_PTR kernel_data;
    TD_STRUCT_PTR                   td_ptr;

    _GET_KERNEL_DATA(kernel_data);

#if MQX_CHECK_ERRORS
    /* Validate parameters */
    if ( tick_ptr == NULL )
    {
        _task_set_error( MQX_SCHED_INVALID_PARAMETER_PTR );
        return( MQX_SCHED_INVALID_PARAMETER_PTR );
    } /* Endif */
#endif

    /* Handle default case */
    if ( task_id == MQX_DEFAULT_TASK_ID )
    {

        *tick_ptr = kernel_data->SCHED_TIME_SLICE;

    }
    else
    {

        td_ptr = (TD_STRUCT_PTR)_task_get_td( task_id );
        if ( td_ptr == NULL )
        {
            _task_set_error( MQX_SCHED_INVALID_TASK_ID );
            return( MQX_SCHED_INVALID_TASK_ID );
        } /* Endif */

        *tick_ptr = td_ptr->TIME_SLICE;

    } /* Endif */

    return( MQX_OK );

} /* Endbody */

/*!
 * \private
 *
 * \brief Sets various the scheduling rr_interval for a task or the system.
 *
 * \param[in]  task_id             One of the following:
 * \n - Task ID for a task on this processor for which to set info.
 * \n - MQX_DEFAULT_TASK_ID (Set the time slice for the processor.)
 * \n - MQX_NULL_TASK_ID (Set the time slice for the calling task.)
 * \param[in]  new_rr_tick_ptr Pointer to the new time slice (in tick time).
 * \param[out] old_rr_tick_ptr Pointer to the previous time slice (in tick time).
 *
 * \return MQX_OK
 * \return MQX_SCHED_INVALID_TASK_ID
 *
 * \see _sched_set_rr_interval_ticks
 * \see MQX_TICK_STRUCT
 */
_mqx_uint _sched_set_rr_interval_internal
(
    _task_id            task_id,
    MQX_TICK_STRUCT_PTR new_rr_tick_ptr,
    MQX_TICK_STRUCT_PTR old_rr_tick_ptr

)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    TD_STRUCT_PTR          td_ptr;

    _GET_KERNEL_DATA(kernel_data);

    /* Handle default case */
    if (task_id == MQX_DEFAULT_TASK_ID)
    {
        *old_rr_tick_ptr = kernel_data->SCHED_TIME_SLICE;
        _int_disable();
        kernel_data->SCHED_TIME_SLICE = *new_rr_tick_ptr;
        _int_enable();
    }
    else
    {
        td_ptr = (TD_STRUCT_PTR)_task_get_td(task_id);
        if (td_ptr == NULL)
        {
            return(MQX_SCHED_INVALID_TASK_ID);
        } /* Endif */
        *old_rr_tick_ptr = td_ptr->TIME_SLICE;
        _int_disable();
        td_ptr->TIME_SLICE = *new_rr_tick_ptr;
        _int_enable();
    } /* Endif */

    return(MQX_OK);

} /* Endbody */

#endif /* MQX_HAS_TIME_SLICE */
#endif /* MQX_LITE_VERSION_NUMBER */

/* EOF */
