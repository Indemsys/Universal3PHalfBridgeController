
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
*   This file contains functions of the Lightweight message component.
*
*
*END************************************************************************/

#include "mqx_inc.h"

#if MQX_USE_LWMSGQ
#include "lwmsgq.h"
#include "lwmsgq_prv.h"

/*!
 * \private
 *
 * \brief Initializes the light weight message queue.
 *
 * \param[in] location     Pointer to memory to create a message queue.
 * \param[in] num_messages Number of messages in the queue.
 * \param[in] msg_size     Specifies message size as a multiplier factor of
 * _mqx_max_type items.
 * \param[in] user         User mode functionality.
 *
 * \return MQX_OK
 * \return MQX_EINVAL (The location already points to a valid lightweight message
 * queue.)
 *
 * \see _lwmsgq_init
 */
_mqx_uint _lwmsgq_init_internal
(
    void   *location,
    _mqx_uint num_messages,
    _mqx_uint msg_size,
    bool user
)
{
    KERNEL_DATA_STRUCT_PTR kernel_data;
    LWMSGQ_STRUCT_PTR      q_ptr = (LWMSGQ_STRUCT_PTR) location;
    LWMSGQ_STRUCT_PTR      lwmsg_chk_ptr;

#if MQX_ENABLE_USER_MODE
    if (user && !_psp_mem_check_access_mask((uint32_t)q_ptr, sizeof(LWMSGQ_STRUCT), MPU_UM_R, MPU_UM_RW))
    {
        return MQX_EINVAL;
    }
#endif

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE4(KLOG_lwmsgq_init, location, num_messages, msg_size);

    _QUEUE_INIT(&q_ptr->WAITING_WRITERS,0);
    _QUEUE_INIT(&q_ptr->WAITING_READERS,0);
    q_ptr->MSG_SIZE = msg_size;
    q_ptr->MAX_SIZE = num_messages;
    q_ptr->CURRENT_SIZE  = 0;
    q_ptr->MSG_START_LOC = (_mqx_max_type_ptr) ((unsigned char *) q_ptr + sizeof(LWMSGQ_STRUCT));
    q_ptr->MSG_END_LOC   = q_ptr->MSG_START_LOC + msg_size * num_messages;
    q_ptr->MSG_WRITE_LOC = q_ptr->MSG_START_LOC;
    q_ptr->MSG_READ_LOC  = q_ptr->MSG_START_LOC;

#if MQX_ENABLE_USER_MODE
    if (user)
    {
        if (kernel_data->USR_LWMSGQS.NEXT == NULL)
        {
            /* Initialize the light weight message queue */
            _QUEUE_INIT(&kernel_data->USR_LWMSGQS, 0);
        }
    }
    else
#endif
    {
        if (kernel_data->LWMSGQS.NEXT == NULL)
        {
            /* Initialize the light weight message queue */
            _QUEUE_INIT(&kernel_data->LWMSGQS, 0);
        }
    }

    q_ptr->VALID = LWMSGQ_VALID;

#if MQX_CHECK_ERRORS
    /* Check if lwmsgq is already initialized */
#if MQX_ENABLE_USER_MODE
    if (user)
    {
        lwmsg_chk_ptr = (LWMSGQ_STRUCT_PTR)((void *)kernel_data->USR_LWMSGQS.NEXT);
        while (lwmsg_chk_ptr != (LWMSGQ_STRUCT_PTR)((void *)&kernel_data->USR_LWMSGQS))
        {
            if (lwmsg_chk_ptr == q_ptr)
            {
                _KLOGX2(KLOG_lwmsgq_init, MQX_EINVAL);
                return(MQX_EINVAL);
            }
            lwmsg_chk_ptr = (LWMSGQ_STRUCT_PTR)((void *)lwmsg_chk_ptr->LINK.NEXT);
        }
    }
    else
#endif /* MQX_ENABLE_USER_MODE */
    {
        lwmsg_chk_ptr = (LWMSGQ_STRUCT_PTR) ((void *) kernel_data->LWMSGQS.NEXT);
        while (lwmsg_chk_ptr != (LWMSGQ_STRUCT_PTR) ((void *) &kernel_data->LWMSGQS))
        {
            if (lwmsg_chk_ptr == q_ptr)
            {
                _KLOGX2(KLOG_lwmsgq_init, MQX_EINVAL);
                return (MQX_EINVAL);
            }
            lwmsg_chk_ptr = (LWMSGQ_STRUCT_PTR) ((void *) lwmsg_chk_ptr->LINK.NEXT);
        }
    }
#endif

#if MQX_ENABLE_USER_MODE
    if (user)
    {
        _QUEUE_ENQUEUE(&kernel_data->USR_LWMSGQS, &q_ptr->LINK);
    }
    else
#endif /* MQX_ENABLE_USER_MODE */
    {
        _QUEUE_ENQUEUE(&kernel_data->LWMSGQS, &q_ptr->LINK);
    }

    _KLOGX2(KLOG_lwmsgq_init, MQX_OK);

    return MQX_OK;
}

/*!
 * \private
 *
 * \brief Deinitializes the light weight message queue.
 *
 * \param[in] location     Pointer to memory to create a message queue.
 * \param[in] user         User mode functionality.
 *
 * \return MQX_OK
 * \return MQX_EINVAL (The location points to a non-valid or not-initialized lightweight message queue.)
 *
 * \see _lwmsgq_init
 */
_mqx_uint _lwmsgq_deinit_internal
(
    void *location,
    bool user
)
{
    KERNEL_DATA_STRUCT_PTR kernel_data;
    LWMSGQ_STRUCT_PTR      q_ptr = (LWMSGQ_STRUCT_PTR) location;
    LWMSGQ_STRUCT_PTR      lwmsg_chk_ptr;
    TD_STRUCT_PTR          td_ptr;
    _mqx_uint              result = MQX_EINVAL;

#if MQX_ENABLE_USER_MODE
    if (user && !_psp_mem_check_access_mask((uint32_t)q_ptr, sizeof(LWMSGQ_STRUCT), MPU_UM_R, MPU_UM_RW))
    {
        return MQX_EINVAL;
    }
#endif

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_lwmsgq_deinit, location);

    _int_disable();
#if MQX_CHECK_ERRORS
    /* Check if lwmsgq is already initialized */
#if MQX_ENABLE_USER_MODE
    if (user)
    {
        lwmsg_chk_ptr = (LWMSGQ_STRUCT_PTR)((void *)kernel_data->USR_LWMSGQS.NEXT);
        while (lwmsg_chk_ptr != (LWMSGQ_STRUCT_PTR)((void *)&kernel_data->USR_LWMSGQS))
        {
            if (lwmsg_chk_ptr == q_ptr)
            {
               result = MQX_OK;
               break;
            }
            lwmsg_chk_ptr = (LWMSGQ_STRUCT_PTR)((void *)lwmsg_chk_ptr->LINK.NEXT);
        }
    }
    else
#endif /* MQX_ENABLE_USER_MODE */
    {
        lwmsg_chk_ptr = (LWMSGQ_STRUCT_PTR) ((void *)kernel_data->LWMSGQS.NEXT);
        while (lwmsg_chk_ptr != (LWMSGQ_STRUCT_PTR) ((void *)&kernel_data->LWMSGQS))
        {
            if (lwmsg_chk_ptr == q_ptr)
            {
               result = MQX_OK;
               break;
            }
            lwmsg_chk_ptr = (LWMSGQ_STRUCT_PTR) ((void *)lwmsg_chk_ptr->LINK.NEXT);
        }
    }
    if (result != MQX_OK)
    {
        _int_enable();
        _KLOGX2(KLOG_lwmsgq_deinit, result);

        return result;
    }
#endif

    /* Invalidate lwmsgq's valid flag so next tasks cannot access it.
    ** Invalid number is any number which does not equal the valid one.
    */
    q_ptr->VALID = LWMSGQ_VALID - 1;

    /* Unblock all waiting writers to the queue */
    while (!_QUEUE_IS_EMPTY(&q_ptr->WAITING_WRITERS))
    {
        _QUEUE_DEQUEUE(&q_ptr->WAITING_WRITERS, td_ptr);
        _BACKUP_POINTER(td_ptr, TD_STRUCT, AUX_QUEUE);
        td_ptr->INFO = LWMSGQ_DEINIT_INFO; /* Signal that LWMSGQ is to be deinitialized. */
        _TASK_READY(td_ptr, kernel_data);
    } /* Endif */

    /* Unblock all waiting readers from the queue */
    while (!_QUEUE_IS_EMPTY(&q_ptr->WAITING_READERS))
    {
        _QUEUE_DEQUEUE(&q_ptr->WAITING_READERS, td_ptr);
        _BACKUP_POINTER(td_ptr, TD_STRUCT, AUX_QUEUE);
        _TIME_DEQUEUE(td_ptr, kernel_data);
        td_ptr->INFO = LWMSGQ_DEINIT_INFO; /* Signal that LWMSGQ is to be deinitialized. */
        _TASK_READY(td_ptr, kernel_data);
    }

#if MQX_ENABLE_USER_MODE
    if (user)
    {
        _QUEUE_REMOVE(&kernel_data->USR_LWMSGQS, q_ptr);
    }
    else
#endif /* MQX_ENABLE_USER_MODE */
    {
        _QUEUE_REMOVE(&kernel_data->LWMSGQS, q_ptr);
    }

    _CHECK_RUN_SCHEDULER(); /* Let higher priority task run */
    _int_enable();

    _KLOGX2(KLOG_lwmsgq_deinit, MQX_OK);

    return MQX_OK;
}

/*!
 * \brief Creates a lightweight message queue.
 *
 * This function creates a message queue at location. There must be sufficient memory
 * allocated to hold num_messages of msg_size * sizeof(_mqx_max_type) plus the size
 * of LWMSGQ_STRUCT.
 *
 * \param[in] location     Pointer to memory to create a message queue.
 * \param[in] num_messages Number of messages in the queue.
 * \param[in] msg_size     Specifies message size as a multiplier factor of
 * _mqx_max_type items.
 *
 * \return MQX_OK
 * \return MQX_EINVAL (The location already points to a valid lightweight message
 * queue.)
 *
 * \see _lwmsgq_usr_check
 * \see _lwmsgq_receive
 * \see _lwmsgq_send
 */
_mqx_uint _lwmsgq_init
(
    void   *location,
    _mqx_uint num_messages,
    _mqx_uint msg_size
)
{
#if MQX_ENABLE_USER_MODE && MQX_ENABLE_USER_STDAPI
    if (MQX_RUN_IN_USER_MODE)
    {
        return _usr_lwmsgq_init(location, num_messages, msg_size);
    }
#endif

    return _lwmsgq_init_internal(location, num_messages, msg_size, FALSE);
}

#if MQX_ENABLE_USER_MODE

/*!
 * \brief Creates a lightweight message queue.
 *
 * This function is an equivalent to the _lwmsgq_init() API call but it can be
 * executed from within the User task or other code running in the CPU User mode.
 * Parameters passed to this function by pointer are required to meet the memory
 * protection requirements as described in the parameter list below.
 *
 * \param[in] location     Read only. Pointer to memory to create a message queue.
 * \param[in] num_messages Number of messages in the queue.
 * \param[in] msg_size     Specifies message size as a multiplier factor of
 * _mqx_max_type items.
 *
 * \return MQX_OK
 * \return MQX_EINVAL (The location already points to a valid lightweight message
 * queue.)
 *
 * \see _lwmsgq_init
 * \see _usr_lwmsgq_receive
 * \see _usr_lwmsgq_send
 */
_mqx_uint _usr_lwmsgq_init
(
    void   *location,
    _mqx_uint num_messages,
    _mqx_uint msg_size
)
{
    MQX_API_CALL_PARAMS params =
    {   (uint32_t)location, (uint32_t)num_messages, (uint32_t)msg_size, 0, 0};
    return _mqx_api_call(MQX_API_LWMSGQ_INIT, &params);
}

/*!
 * \brief Checks whether the specified pointer corresponds with a valid lightweight
 * message queue.
 *
 * \param[in] tst_lwmsgq_ptr Pointer to lightweight message.
 *
 * \return MQX_OK (Valid lightweight message.)
 * \return LWMSGQ_INVALID (Specified lightweight message is not valid.)
 */
_mqx_uint _lwmsgq_usr_check
(
    void   *tst_lwmsgq_ptr
)
{

    KERNEL_DATA_STRUCT_PTR kernel_data;
    LWMSGQ_STRUCT_PTR      lwmsgq_ptr;
    _mqx_uint              result = LWMSGQ_INVALID;
    _mqx_uint              queue_size;

    _GET_KERNEL_DATA(kernel_data);

    lwmsgq_ptr = (LWMSGQ_STRUCT_PTR)((void *)kernel_data->USR_LWMSGQS.NEXT);
    queue_size = _QUEUE_GET_SIZE(&kernel_data->USR_LWMSGQS);

    while (queue_size--)
    {
        if (lwmsgq_ptr->VALID != LWMSGQ_VALID)
        {
            break;
        }

        if (tst_lwmsgq_ptr == lwmsgq_ptr)
        {
            result = MQX_OK;
            break;
        }

        lwmsgq_ptr = (LWMSGQ_STRUCT_PTR)(void *)lwmsgq_ptr->LINK.NEXT;
    }

    return result;
}

#endif /* MQX_ENABLE_USER_MODE */

/*!
 * \brief Deinitializes a lightweight message queue.
 *
 * This function deinitializes a message queue at location. All the tasks waiting
 * for a message or are blocked upon full message queue are released.
 *
 * \param[in] location     Pointer to memory to create a message queue.
 *
 * \return MQX_OK
 * \return MQX_EINVAL (The location already points to a valid lightweight message
 * queue.)
 *
 * \see _lwmsgq_receive
 * \see _lwmsgq_send
 */
_mqx_uint _lwmsgq_deinit
(
    void *location
)
{
#if MQX_ENABLE_USER_MODE && MQX_ENABLE_USER_STDAPI
    if (MQX_RUN_IN_USER_MODE)
    {
        return _usr_lwmsgq_deinit(location);
    }
#endif

    return _lwmsgq_deinit_internal(location, FALSE);
}

#if MQX_ENABLE_USER_MODE

/*!
 * \brief Deinitializes a lightweight message queue.
 *
 * This function is an equivalent to the _lwmsgq_deinit() API call but it can be
 * executed from within the User task or other code running in the CPU User mode.
 * Parameters passed to this function by pointer are required to meet the memory
 * protection requirements as described in the parameter list below.
 *
 * \param[in] location     Read only. Pointer to memory to create a message queue.
 *
 * \return MQX_OK
 * \return MQX_EINVAL (The location already points to a valid lightweight message
 * queue.)
 *
 * \see _lwmsgq_deinit
 */
_mqx_uint _usr_lwmsgq_deinit
(
    void *location
)
{
    MQX_API_CALL_PARAMS params =
    {   (uint32_t)location, 0, 0, 0, 0};
    return _mqx_api_call(MQX_API_LWMSGQ_DEINIT, &params);
}

#endif /* MQX_ENABLE_USER_MODE */

/*!
 * \brief Gets a message from a lightweight message queue.
 *
 * This function removes the first message from the queue and copies the message
 * to the user buffer.
 * \n The message becomes a resource of the task.
 *
 * \param[in]  handle   Pointer to the message queue created by _lwmsgq_init.
 * \param[out] message  Received message.
 * \param[in]  flags    LWMSGQ_RECEIVE_BLOCK_ON_EMPTY Block the reading task if msgq is empty.
 * LWMSGQ_TIMEOUT_UNTIL Perform a timeout using the tick structure (tick_ptr input parameter value) as the absolute time.
 * LWMSGQ_TIMEOUT_FOR Perform a timeout using the tick structure (tick_ptr input parameter value) as the relative time.
 * \param[in]  ticks    The maximum number of ticks to wait. If set to zero waiting time is controlled by the tick_ptr input parameter value.
 *                      When just LWMSGQ_RECEIVE_BLOCK_ON_EMPTY flags is set and ticks input parameters is set to zero the receive function waits for an unlimited amount of time. 
 * \param[in]  tick_ptr Pointer to the tick structure to use when ticks input parameter is set to zero. 
 *
 * \return MQX_OK
 * \return LWMSGQ_INVALID (The handle was not valid.)
 * \return LWMSGQ_EMPTY (The LWMSGQ_RECEIVE_BLOCK_ON_EMPTY flag was NOT USED and no
 * messages were in the message queue.)
 * \return LWMSGQ_TIMEOUT (No messages were in the message queue before the timeout
 * expired.)
 * \return LWMSGQ_DEINIT (The queue was asynchronously deinitialized during wait on
 * queue operation.)
 * \return MQX_CANNOT_CALL_FUNCTION_FROM_ISR (The function cannot be called from ISR
 * when using inappropriate blocking flags.)
 *
 * \see _lwmsgq_init
 * \see _lwmsgq_usr_check
 * \see _lwmsgq_send
 * \see MQX_TICK_STRUCT
 */
_mqx_uint _lwmsgq_receive
(
    void   *handle,
    _mqx_max_type_ptr message,
    _mqx_uint flags,
    _mqx_uint ticks,
    MQX_TICK_STRUCT_PTR tick_ptr
)
{/* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    TD_STRUCT_PTR          td_ptr;
    LWMSGQ_STRUCT_PTR      q_ptr = (LWMSGQ_STRUCT_PTR) handle;
    _mqx_uint              i;
    _mqx_max_type_ptr      from_ptr;
    _mqx_max_type_ptr      to_ptr;

#if MQX_ENABLE_USER_MODE && MQX_ENABLE_USER_STDAPI
    if (MQX_RUN_IN_USER_MODE)
    {
        return _usr_lwmsgq_receive(handle, message, flags, ticks, tick_ptr);
    }
#endif

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE6(KLOG_lwmsgq_receive, handle, message, flags, ticks, tick_ptr);

#if MQX_CHECK_ERRORS
    if (kernel_data->IN_ISR)
    {
        _KLOGX2(KLOG_lwmsgq_receive, MQX_CANNOT_CALL_FUNCTION_FROM_ISR);
        return MQX_CANNOT_CALL_FUNCTION_FROM_ISR;
    } /* Endif */
#endif

    _int_disable();
#if MQX_CHECK_VALIDITY
    if (q_ptr->VALID != LWMSGQ_VALID)
    {
        _int_enable();
        _KLOGX2(KLOG_lwmsgq_receive, LWMSGQ_INVALID);
        return LWMSGQ_INVALID;
    } /* Endif */
#endif
    if (LWMSGQ_IS_EMPTY(q_ptr))
    {
        if (flags & LWMSGQ_RECEIVE_BLOCK_ON_EMPTY)
        {
            td_ptr = kernel_data->ACTIVE_PTR;
            while (LWMSGQ_IS_EMPTY(q_ptr))
            {
                td_ptr->STATE = LWMSGQ_READ_BLOCKED;
                td_ptr->INFO = (_mqx_uint) &q_ptr->WAITING_READERS;
                _QUEUE_UNLINK(td_ptr);
                _QUEUE_ENQUEUE(&q_ptr->WAITING_READERS, &td_ptr->AUX_QUEUE);
                if (ticks || (flags & (LWMSGQ_TIMEOUT_UNTIL | LWMSGQ_TIMEOUT_FOR)))
                {
                    if (ticks)
                    {
                        PSP_ADD_TICKS_TO_TICK_STRUCT(&kernel_data->TIME, ticks,
                                        &td_ptr->TIMEOUT);
                    }
                    else if (flags & LWMSGQ_TIMEOUT_UNTIL)
                    {
                        td_ptr->TIMEOUT = *tick_ptr;
                    }
                    else
                    {
                        PSP_ADD_TICKS(tick_ptr, &kernel_data->TIME, &td_ptr->TIMEOUT);
                    } /* Endif */
                    _time_delay_internal(td_ptr);
                    if (td_ptr->INFO == LWMSGQ_DEINIT_INFO)
                    {
                        _int_enable();
                        _KLOGX2(KLOG_lwmsgq_receive, LWMSGQ_DEINIT);
                        return LWMSGQ_DEINIT;
                    }
                    else if (td_ptr->INFO != 0)
                    {
                        _int_enable();
                        _KLOGX2(KLOG_lwmsgq_receive, LWMSGQ_TIMEOUT);
                        return LWMSGQ_TIMEOUT;
                    } /* Endif */
                }
                else
                {
                    _sched_execute_scheduler_internal(); /* Let other tasks run */
                    if (td_ptr->INFO == LWMSGQ_DEINIT_INFO)
                    {
                        _int_enable();
                        _KLOGX2(KLOG_lwmsgq_receive, LWMSGQ_DEINIT);
                        return LWMSGQ_DEINIT;
                    }
                } /* Endif */
            } /* Endwhile */
        }
        else
        {
            _int_enable();
            _KLOGX2(KLOG_lwmsgq_receive, LWMSGQ_EMPTY);
            return LWMSGQ_EMPTY;
        } /* Endif */
    }/* Endif */
    from_ptr = q_ptr->MSG_READ_LOC;
    to_ptr = message;
    i = q_ptr->MSG_SIZE + 1;
    while (--i)
    {
        *to_ptr++ = *from_ptr++;
    } /* Endwhile */
    q_ptr->MSG_READ_LOC += q_ptr->MSG_SIZE;
    if (q_ptr->MSG_READ_LOC >= q_ptr->MSG_END_LOC)
    {
        q_ptr->MSG_READ_LOC = q_ptr->MSG_START_LOC;
    } /* Endif */
    q_ptr->CURRENT_SIZE--;
    if (!_QUEUE_IS_EMPTY(&q_ptr->WAITING_WRITERS))
    {
        _QUEUE_DEQUEUE(&q_ptr->WAITING_WRITERS, td_ptr);
        _BACKUP_POINTER(td_ptr, TD_STRUCT, AUX_QUEUE);
        td_ptr->INFO = 0; /* Signal that post is activating the task */
        _TASK_READY(td_ptr, kernel_data);
        _CHECK_RUN_SCHEDULER(); /* Let higher priority task run */
    } /* Endif */
    _int_enable();
    _KLOGX2(KLOG_lwmsgq_receive, MQX_OK);
    return MQX_OK;

}/* Endbody */

#if MQX_ENABLE_USER_MODE
/*!
 * \brief Gets a message from a lightweight message queue.
 *
 * This function is an equivalent to the _lwmsgq_receive() API call but it can be
 * executed from within the User task or other code running in the CPU User mode.
 * Parameters passed to this function by pointer are required to meet the memory
 * protection requirements as described in the parameter list below.
 *
 * \param[in]  handle   Read only. Pointer to the message queue created by _lwmsgq_init.
 * \param[out] message  Received message.
 * \param[in]  flags    LWMSGQ_RECEIVE_BLOCK_ON_EMPTY Block the reading task if msgq is empty.
 * LWMSGQ_TIMEOUT_UNTIL Perform a timeout using the tick structure as the absolute time.
 * LWMSGQ_TIMEOUT_FOR Perform a timeout using the tick structure as the relative time.
 * \param[in]  ticks    The maximum number of ticks to wait or NULL (unlimited wait).
 * \param[in]  tick_ptr Pointer to the tick structure to use.
 *
 * \return MQX_OK
 * \return LWMSGQ_INVALID (The handle was not valid.)
 * \return LWMSGQ_EMPTY (The LWMSGQ_RECEIVE_BLOCK_ON_EMPTY flag was NOT USED and no
 * messages were in the message queue.)
 * \return LWMSGQ_TIMEOUT (No messages were in the message queue before the timeout
 * expired.)
 * \return LWMSGQ_DEINIT (The message queue was deinitialized.)
 *
 * \see _lwmsgq_receive
 * \see _usr_lwmsgq_init
 * \see _usr_lwmsgq_send
 * \see MQX_TICK_STRUCT
 */
_mqx_uint _usr_lwmsgq_receive
(
    void   *handle,
    _mqx_max_type_ptr message,
    _mqx_uint flags,
    _mqx_uint ticks,
    MQX_TICK_STRUCT_PTR tick_ptr
)
{
    MQX_API_CALL_PARAMS params =
    {   (uint32_t)handle, (uint32_t)message, (uint32_t)flags, (uint32_t)ticks, (uint32_t)tick_ptr};
    return _mqx_api_call(MQX_API_LWMSGQ_RECEIVE, &params);
}

#endif /* MQX_ENABLE_USER_MODE */

/*!
 * \brief Puts a message on a lightweight message queue.
 *
 * This function posts a message on the queue. If the queue is full, the task can
 * block and wait or this function returns with LWMSGQ_FULL.
 *
 * \param[in] handle Pointer to the message queue created by _lwmsgq_init.
 * \param[in] message Pointer to the message to send.
 * \param[in] flags LWMSGQ_SEND_BLOCK_ON_FULL (Block the task if queue is full.)
 * or LWMSGQ_SEND_BLOCK_ON_SEND (Block the task after the message is sent.)
 *
 * \return MQX_OK
 * \return LWMSGQ_INVALID (The handle was not valid.)
 * \return LWMSGQ_FULL (The LWMSGQ_SEND_BLOCK_ON_FULL flag was NOT USED and message
 * queue was full.)
 * \return LWMSGQ_DEINIT (The queue was asynchronously deinitialized during wait on
 * queue operation.)
 * \return MQX_CANNOT_CALL_FUNCTION_FROM_ISR (The function cannot be called from ISR
 * when using inappropriate blocking flags.)
 *
 * \see _lwmsgq_init
 * \see _lwmsgq_usr_check
 * \see _lwmsgq_receive
 */
_mqx_uint _lwmsgq_send
(
    void   *handle,
    _mqx_max_type_ptr message,
    _mqx_uint flags
)
{/* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    TD_STRUCT_PTR          td_ptr;
    LWMSGQ_STRUCT_PTR      q_ptr = (LWMSGQ_STRUCT_PTR) handle;
    _mqx_uint              i;
    _mqx_max_type_ptr      from_ptr;
    _mqx_max_type_ptr      to_ptr;

#if MQX_ENABLE_USER_MODE && MQX_ENABLE_USER_STDAPI
    if (MQX_RUN_IN_USER_MODE)
    {
        return _usr_lwmsgq_send(handle, message, flags);
    }
#endif

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE4(KLOG_lwmsgq_send, handle, message, flags);

#if MQX_CHECK_ERRORS
    if (kernel_data->IN_ISR && (flags & (LWMSGQ_SEND_BLOCK_ON_FULL | LWMSGQ_SEND_BLOCK_ON_SEND)))
    {
        _KLOGX2(KLOG_lwmsgq_send, MQX_CANNOT_CALL_FUNCTION_FROM_ISR);
        return MQX_CANNOT_CALL_FUNCTION_FROM_ISR;
    } /* Endif */
#endif

    _int_disable();
#if MQX_CHECK_VALIDITY
    if (q_ptr->VALID != LWMSGQ_VALID)
    {
        _int_enable();
        _KLOGX2(KLOG_lwmsgq_send, LWMSGQ_INVALID);
        return LWMSGQ_INVALID;
    } /* Endif */
#endif
    if (LWMSGQ_IS_FULL(q_ptr))
    {
        if (flags & LWMSGQ_SEND_BLOCK_ON_FULL)
        {
            td_ptr = kernel_data->ACTIVE_PTR;
            td_ptr->STATE = LWMSGQ_WRITE_BLOCKED;
            while (LWMSGQ_IS_FULL(q_ptr))
            {
                td_ptr->INFO = (_mqx_uint) &q_ptr->WAITING_WRITERS;
                _QUEUE_UNLINK(td_ptr);
                _QUEUE_ENQUEUE(&q_ptr->WAITING_WRITERS, &td_ptr->AUX_QUEUE);
                _sched_execute_scheduler_internal(); /* Let other tasks run */
                if (td_ptr->INFO == LWMSGQ_DEINIT_INFO)
                {
                    _int_enable();
                    _KLOGX2(KLOG_lwmsgq_send, LWMSGQ_DEINIT);
                    return LWMSGQ_DEINIT;
                }
            } /* Endwhile */
        }
        else
        {
            _int_enable();
            _KLOGX2(KLOG_lwmsgq_send, LWMSGQ_FULL);
            return LWMSGQ_FULL;
        } /* Endif */
    } /* Endif */
    to_ptr = q_ptr->MSG_WRITE_LOC;
    from_ptr = message;
    i = q_ptr->MSG_SIZE + 1;
    while (--i)
    {
        *to_ptr++ = *from_ptr++;
    } /* Endwhile */
    q_ptr->MSG_WRITE_LOC += q_ptr->MSG_SIZE;
    if (q_ptr->MSG_WRITE_LOC >= q_ptr->MSG_END_LOC)
    {
        q_ptr->MSG_WRITE_LOC = q_ptr->MSG_START_LOC;
    } /* Endif */
    q_ptr->CURRENT_SIZE++;
    if (!_QUEUE_IS_EMPTY(&q_ptr->WAITING_READERS))
    {
        _QUEUE_DEQUEUE(&q_ptr->WAITING_READERS, td_ptr);
        _BACKUP_POINTER(td_ptr, TD_STRUCT, AUX_QUEUE);
        _TIME_DEQUEUE(td_ptr, kernel_data);
        td_ptr->INFO = 0; /* Signal that post is activating the task */
        _TASK_READY(td_ptr, kernel_data);
        if (flags & LWMSGQ_SEND_BLOCK_ON_SEND)
        {
            _task_block();
        }
        else
        {
            _CHECK_RUN_SCHEDULER(); /* Let higher priority task run */
        }/* Endif */
    }
    else
    {
        if (flags & LWMSGQ_SEND_BLOCK_ON_SEND)
        {
            _task_block();
        }/* Endif */
    } /* Endif */
    _int_enable();
    _KLOGX2(KLOG_lwmsgq_send, MQX_OK);
    return MQX_OK;

}/* Endbody */

#if MQX_ENABLE_USER_MODE
/*!
 * \brief Puts a message on a lightweight message queue.
 *
 * This function is an equivalent to the _lwmsgq_send() API call but it can be
 * executed from within the User task or other code running in the CPU User mode.
 * Parameters passed to this function by pointer are required to meet the memory
 * protection requirements as described in the parameter list below.
 *
 * \param[in] handle Read only. Pointer to the message queue created by _lwmsgq_init.
 * \param[in] message Pointer to the message to send.
 * \param[in] flags LWMSGQ_SEND_BLOCK_ON_FULL (Block the task if queue is full.)
 * or LWMSGQ_SEND_BLOCK_ON_SEND (Block the task after the message is sent.)
 *
 * \return MQX_OK
 * \return LWMSGQ_INVALID (The handle was not valid.)
 * \return LWMSGQ_FULL (The LWMSGQ_SEND_BLOCK_ON_FULL flag was NOT USED and message
 * queue was full.)
 *
 * \see _lwmsgq_send
 * \see _usr_lwmsgq_init
 * \see _usr_lwmsgq_receive
 */
_mqx_uint _usr_lwmsgq_send
(
    void   *handle,
    _mqx_max_type_ptr message,
    _mqx_uint flags
)
{
    MQX_API_CALL_PARAMS params =
    {   (uint32_t)handle, (uint32_t)message, (uint32_t)flags, 0, 0};
    return _mqx_api_call(MQX_API_LWMSGQ_SEND, &params);
}

#endif /* MQX_ENABLE_USER_MODE */

#endif /* MQX_USE_LWMSGQ */
