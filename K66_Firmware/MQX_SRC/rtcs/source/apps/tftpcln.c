/*HEADER**********************************************************************
*
* Copyright 2014 Freescale Semiconductor, Inc.
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
*   This file contains the TFTP client implementation.
*
*END************************************************************************/


#include <rtcs.h>

#if RTCSCFG_ENABLE_UDP

#if MQX_USE_IO_OLD
#include <fio.h>
#else
#include <stdio.h>
#endif

#include "tftp_prv.h"
#include "tftpcln_prv.h"
#include <lwmsgq.h>

/*
 * Start the client task/connect to server.
 */
uint32_t TFTPCLN_connect(TFTPCLN_PARAM_STRUCT *params)
{
    TFTPCLN_ERROR              error;
    uint32_t                   prio;
    TFTPCLN_TASK_PARAMS_STRUCT task_params;

    error = TFTPCLN_OK;

    if (MQX_OK != _task_get_priority(MQX_NULL_TASK_ID, &prio))
    {
        error = TFTPCLN_FAIL;
        goto EXIT;
    }
    task_params.params = params;
    /* Start TFTP client task on current task priority. */
    if (RTCS_task_create(TFTPCLN_TASK_NAME, prio, TFTPCLN_STACK_SIZE, tftpcln_task, (void*) &task_params) != RTCS_OK)
    {
        error = TFTPCLN_FAIL;
        goto EXIT;
    }

    EXIT:
    if (error != TFTPCLN_OK)
    {
        tftpcln_cleanup(task_params.context, true);
        task_params.context = NULL;
    }
    return((uint32_t) task_params.context);
}

/*
 * Stop the client task
 */
int32_t TFTPCLN_disconnect(uint32_t handle)
{
    TFTPCLN_CONTEXT     *context;
    TFTPCLN_API_MESSAGE message;
    int32_t             retval;

    context = (TFTPCLN_CONTEXT *) handle;

    RTCS_ASSERT(context != NULL);
    RTCS_ASSERT(context->valid == TFTPCLN_CONTEXT_VALID);

    if (context->valid != TFTPCLN_CONTEXT_VALID)
    {
        return(RTCS_ERROR);
    }
    message.command = TFTPCLN_COMMAND_STOP;
    message.src_tid = _task_get_id();
    message.result = &retval;
    if (_lwmsgq_send(context->api_queue, (_mqx_max_type *) &message, LWMSGQ_SEND_BLOCK_ON_FULL) == MQX_OK)
    {
        _task_block();
    }
    /* Wait until task stops. */
    while (context->tid != 0)
    {
        _sched_yield();
    }
    _mem_free(context);
    return(retval);
}

/*
 * Receive data from server.
 */
int32_t TFTPCLN_get(uint32_t handle, char *local_file, char *remote_file)
{
    TFTPCLN_CONTEXT *context;
    int32_t         retval;

    retval = TFTPCLN_FAIL;
    context = (TFTPCLN_CONTEXT *) handle;

    RTCS_ASSERT(context != NULL);
    RTCS_ASSERT(context->valid == TFTPCLN_CONTEXT_VALID);
    RTCS_ASSERT(remote_file != NULL);

    if 
        (
            (context != NULL) && 
            (context->valid == TFTPCLN_CONTEXT_VALID) && 
            (remote_file != NULL)
        )
    {
        TFTPCLN_API_MESSAGE message;

        message.command = TFTPCLN_COMMAND_RECEIVE;
        message.r_file = remote_file;
        if (local_file == NULL)
        {
            message.l_file = remote_file;
        }
        else
        {
            message.l_file = local_file;
        }
        message.src_tid = _task_get_id();
        message.result = &retval;
        if (_lwmsgq_send(context->api_queue, (_mqx_max_type *) &message, LWMSGQ_SEND_BLOCK_ON_FULL) == MQX_OK)
        {
            _task_block();
        }
    }
    return(retval);
}

/*
 * Send data to server.
 */
int32_t TFTPCLN_put(uint32_t handle, char *local_file, char *remote_file)
{
    TFTPCLN_CONTEXT *context;
    int32_t         retval;

    retval = TFTPCLN_FAIL;
    context = (TFTPCLN_CONTEXT *) handle;

    RTCS_ASSERT(context != NULL);
    RTCS_ASSERT(context->valid == TFTPCLN_CONTEXT_VALID);
    RTCS_ASSERT(local_file != NULL);

    if 
        (
            (context != NULL) && 
            (context->valid == TFTPCLN_CONTEXT_VALID) && 
            (local_file != NULL) 
        )
    {
        TFTPCLN_API_MESSAGE message;

        message.command = TFTPCLN_COMMAND_SEND;
        if (remote_file == NULL)
        {
            message.r_file = local_file;
        }
        else
        {
            message.r_file = remote_file;
        }
        message.l_file = local_file;
        message.src_tid = _task_get_id();
        message.result = &retval;
        if (_lwmsgq_send(context->api_queue, (_mqx_max_type *) &message, LWMSGQ_SEND_BLOCK_ON_FULL) == MQX_OK)
        {
            _task_block();
        }
    }
    return(retval);
}

#endif
