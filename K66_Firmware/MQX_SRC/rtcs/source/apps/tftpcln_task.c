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

#include "tftpcln_prv.h"
#include <lwmsgq.h>

static TFTPCLN_ERROR tftpcln_init_trans(TFTPCLN_CONTEXT *context, TFTPCLN_API_MESSAGE *message);
static TFTPCLN_ERROR tftpcln_run_trans(TFTPCLN_CONTEXT *context);

void tftpcln_task(void *v_params, void * creator)
{
    TFTPCLN_CONTEXT            *context;
    TFTPCLN_API_MESSAGE        message;
    TFTPCLN_TASK_PARAMS_STRUCT *task_params;

    RTCS_ASSERT(v_params != NULL);
    task_params = (TFTPCLN_TASK_PARAMS_STRUCT *) v_params;
    context = tftpcln_create_context(task_params->params);
    RTCS_ASSERT(context != NULL);
    if (context == NULL)
    {
        RTCS_task_resume_creator(creator, (uint32_t) RTCS_ERROR);
        return;
    }

    context->tid = _task_get_id();
    if (tftpcln_init_socket(context) != TFTPCLN_OK)
    {
        RTCS_task_resume_creator(creator, (uint32_t) RTCS_ERROR);
        return;
    }
    task_params->context = context;
    RTCS_task_resume_creator(creator, RTCS_OK);

    while(1)
    {
        if (_lwmsgq_receive(context->api_queue, (_mqx_max_type *) &message, LWMSGQ_RECEIVE_BLOCK_ON_EMPTY, 0, NULL) != MQX_OK)
        {
            goto ERROR;
        }
        *message.result = RTCS_OK;
        if (message.command == TFTPCLN_COMMAND_STOP)
        {
            break;
        }
        if (tftpcln_init_trans(context, &message) != TFTPCLN_OK)
        {
            goto ERROR;
        }
        do
        {
            if (tftpcln_run_trans(context) == TFTPCLN_FAIL)
            {
                goto ERROR;
            }
        }
        while(!context->trans.last);
        _task_ready(_task_get_td(message.src_tid));
    }
    context->tid = 0;
    tftpcln_cleanup(context, false);
    if (message.command == TFTPCLN_COMMAND_STOP)
    {
       _task_ready(_task_get_td(message.src_tid));
    }
    return;

    ERROR:
    context->tid = 0;
    tftpcln_cleanup(context, true);
    _task_ready(_task_get_td(message.src_tid));
    *message.result = RTCS_ERROR;
    return;
}

/*
 * Initiate transaction - send first GET/PUT request to determine communication parameters.
 */
static TFTPCLN_ERROR tftpcln_init_trans(TFTPCLN_CONTEXT *context, TFTPCLN_API_MESSAGE *message)
{
    TFTPCLN_ERROR    retval;
    char             *open_flags = NULL;
    char             *buffer;
    uint32_t         filename_length;
    int32_t          result;
    TFTP_TRANSACTION *trans;

    RTCS_ASSERT(context != NULL);
    RTCS_ASSERT(message != NULL);
    retval = TFTPCLN_OK;
    trans = &context->trans;
    buffer = trans->buffer;

    /* Create read/write request. */
    if (message->command == TFTPCLN_COMMAND_SEND)
    {
        *((uint16_t *) buffer) = htons(TFTP_OPCODE_WRQ);
        open_flags = "rb";
    }
    else if (message->command == TFTPCLN_COMMAND_RECEIVE)
    {
        *((uint16_t *) buffer) = htons(TFTP_OPCODE_RRQ);
        open_flags = "wb";
    }
    buffer += sizeof(uint16_t);
    filename_length = strlen(message->r_file)+1;
    _mem_copy(message->r_file, buffer, filename_length);
    buffer += filename_length;
    _mem_copy(TFTP_MODE_OCTET, buffer, TFTP_MODE_OCTET_SIZE);
    buffer += TFTP_MODE_OCTET_SIZE;

    /* Prepare local file. */
    trans->file = fopen(message->l_file, open_flags);
    if (trans->file == NULL)
    {
        return(TFTPCLN_ERR_OPEN_FAIL);
    }

    /* Send request and read response to determine remote communication port. */
    result = sendto(trans->sock, trans->buffer, buffer-trans->buffer, 0, &trans->remote_sa, sizeof(trans->remote_sa));
    if (result == RTCS_ERROR)
    {
        return(TFTPCLN_ERR_NO_SOCKET);
    }

    if (message->command == TFTPCLN_COMMAND_SEND)
    {
        result = tftp_recv_ack(trans);
        if (result == RTCS_ERROR)
        {
            return(TFTPCLN_ERR_NO_SOCKET);
        }
        else if (result >= 0)
        {
            trans->state = TFTP_STATE_SEND_DATA;
            trans->flags &= ~TFTP_FLAG_SAVE_REMOTE;
            result = connect(trans->sock, &trans->remote_sa, sizeof(trans->remote_sa));
            if (result != RTCS_OK)
            {
                return(TFTPCLN_ERR_NO_CONN);
            }
        }
    }
    else if (message->command == TFTPCLN_COMMAND_RECEIVE)
    {
        trans->state = TFTP_STATE_READ_DATA;
    }

    return(retval);
}

/*
 * Run transaction until it is complete or error occurs.
 */
static TFTPCLN_ERROR tftpcln_run_trans(TFTPCLN_CONTEXT *context)
{
    int32_t          data_length;
    TFTPCLN_ERROR    retval;
    TFTP_TRANSACTION *trans;

    RTCS_ASSERT(context != NULL);
    trans = &context->trans;
    retval = TFTPCLN_OK;
    switch(trans->state)
    {
        case TFTP_STATE_SEND_DATA:
            data_length = tftp_send_data(trans);
            if (data_length == RTCS_ERROR)
            {
                uint16_t error_code;

                retval = TFTPCLN_FAIL;
                error_code = TFTP_ERR_UNKNOWN;
                context->params.error_callback(error_code, trans->err_string);
            }
            else if (data_length > 0)
            {
                trans->state = TFTP_STATE_READ_ACK;
                context->params.send_callback(data_length);
            }
            break;
        case TFTP_STATE_READ_DATA:
            data_length = tftp_recv_data(trans);
            if (data_length == RTCS_ERROR)
            {
                uint16_t error_code;

                retval = TFTPCLN_FAIL;
                error_code = ntohs(*((uint16_t *) trans->buffer + 1));
                context->params.error_callback(error_code, trans->err_string);
            }
            else if (data_length >= 0)
            {
                trans->state = TFTP_STATE_SEND_ACK;
                context->params.recv_callback(data_length);
            }

            if (trans->flags & TFTP_FLAG_SAVE_REMOTE)
            {
                int32_t result;

                /* Initial packet exchange is done, so we reset flag. */
                trans->flags &= ~TFTP_FLAG_SAVE_REMOTE;
                result = connect(trans->sock, &trans->remote_sa, sizeof(trans->remote_sa));
                if (result != RTCS_OK)
                {
                    retval = TFTPCLN_ERR_NO_CONN;
                }
            }
            break;
        case TFTP_STATE_READ_ACK:
            data_length = tftp_recv_ack(trans);
            if (data_length == RTCS_ERROR)
            {
                retval = TFTPCLN_FAIL;
            }
            else if (data_length >= 0)
            {
                trans->state = TFTP_STATE_SEND_DATA;
            }
            break;
        case TFTP_STATE_SEND_ACK:
            data_length = tftp_send_ack(trans);
            if (data_length == RTCS_ERROR)
            {
                retval = TFTPCLN_FAIL;
            }
            else if (data_length > 0)
            {
                trans->state = TFTP_STATE_READ_DATA;
            }
            break;
        default:
            break;
    }
    return(retval);
}
