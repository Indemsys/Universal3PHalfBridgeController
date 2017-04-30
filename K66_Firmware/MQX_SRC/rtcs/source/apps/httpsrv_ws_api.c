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
*   This file contains API functions for WebSocket protocol.
*
*END************************************************************************/

#include <httpsrv_ws_prv.h>
#include <lwmsgq.h>

/*
 * Send data through WebSocket.
 */
uint32_t WS_send(WS_USER_CONTEXT_STRUCT* context)
{
    WS_CONTEXT_STRUCT *ws_context;
    WS_FRAME_STRUCT   *frame;
    uint32_t          retval;
    WS_DATA_STRUCT    *data;
    WS_API_CALL_MSG   message;
    uint32_t          n_exept;

    RTCS_ASSERT(context != NULL);
    ws_context = (WS_CONTEXT_STRUCT*) context->handle;
    data = &(context->data);
    n_exept = WS_COMMAND_SEND;

    /* Check input validity. */
    if ((data->type == WS_DATA_INVALID) ||
        (data->data_ptr == NULL) ||
        (ws_context == NULL))
    {
        return(0);
    }
    
    _mem_zero(&message, sizeof(message));
    frame = &message.frame;
    /* Fill frame structure and send it */
    frame->opcode = data->type;
    frame->length = data->length;
    frame->data = data->data_ptr;
    frame->fin = (bool) context->fin_flag;

    message.command = WS_COMMAND_SEND;
    message.data = (void *) _task_get_id();
    
    retval = _lwmsgq_send(ws_context->api_queue, (_mqx_max_type *) &message, LWMSGQ_SEND_BLOCK_ON_FULL);
    setsockopt(ws_context->sock, SOL_SOCKET, SO_EXCEPTION, &n_exept, sizeof(n_exept));
    /* Block calling task. It will be unblocked as soon as message is processed. */
    if ((_task_id) message.data != ws_context->tid)
    {
        _task_block();
    }
    return(retval);
}

/*
 * Close WebSocket.
 */
uint32_t WS_close(uint32_t handle)
{
    WS_CONTEXT_STRUCT *ws_context;
    WS_API_CALL_MSG   message;
    uint32_t          retval;
    uint32_t          n_exept;

    ws_context = (WS_CONTEXT_STRUCT*) handle;
    RTCS_ASSERT(ws_context != NULL);
    retval = (uint32_t)RTCS_ERROR;
    n_exept = WS_COMMAND_CLOSE;

    if (ws_context != NULL)
    {
        message.command = WS_COMMAND_CLOSE;
        message.data = (void *) _task_get_id();
        
        retval = _lwmsgq_send(ws_context->api_queue, (_mqx_max_type *) &message, LWMSGQ_SEND_BLOCK_ON_FULL);
        setsockopt(ws_context->sock, SOL_SOCKET, SO_EXCEPTION, &n_exept, sizeof(n_exept));
        if ((_task_id) message.data != ws_context->tid)
        {
            _task_block();
        }
        retval = RTCS_OK;
    }
    return(retval);
}
