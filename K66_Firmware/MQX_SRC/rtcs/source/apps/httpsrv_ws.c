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
*   This file contains implementation of WebSocket protocol.
*
*END************************************************************************/

#include <httpsrv_ws_prv.h>
#include <rtcs_sha1.h>
#include <rtcs_base64.h>
#include <rtcs_utf8.h>
#include <lwmsgq.h>

#define UINT16MAX 65536

static uint32_t ws_read_frame_header(uint8_t *data, WS_FRAME_STRUCT *dst, uint32_t length);
static void ws_write_frame(uint8_t *dst, WS_FRAME_STRUCT *src);
static void ws_unmask_data(uint8_t* data, uint32_t mask, uint32_t length, uint8_t mask_offset);
static WS_ERROR_CODE ws_check_frame_header(WS_CONTEXT_STRUCT* context, WS_FRAME_STRUCT *frame);
static uint32_t ws_get_header_size(WS_FRAME_STRUCT *frame);
static inline uint32_t ws_get_frame_size(WS_FRAME_STRUCT *frame);
static inline uint32_t ws_do_nothing(void* param, WS_USER_CONTEXT_STRUCT context);
static uint32_t ws_data_frame(WS_CONTEXT_STRUCT* context);
static uint32_t ws_process_frame(WS_CONTEXT_STRUCT* context);
static void ws_bad_frame(WS_CONTEXT_STRUCT* context, WS_ERROR_CODE frame_status);
static uint32_t ws_check_close_reason(uint16_t reason);
static uint32_t ws_close_frame(WS_CONTEXT_STRUCT* context);
static uint32_t ws_ping_frame(WS_CONTEXT_STRUCT* context);
static uint32_t ws_pong_frame(WS_CONTEXT_STRUCT* context);
static inline void ws_update_mask_offset(WS_CONTEXT_STRUCT *context);
static uint32_t ws_process_api_calls(WS_CONTEXT_STRUCT *context);
static uint32_t ws_send_control_frame(WS_CONTEXT_STRUCT* context, uint8_t *data, uint32_t length, uint32_t opcode);
static uint32_t ws_send_frame(WS_CONTEXT_STRUCT* context, WS_FRAME_STRUCT* frame);
static uint32_t ws_check_utf8(WS_CONTEXT_STRUCT* context, uint8_t *data, uint32_t length);
static uint32_t ws_validate_data(WS_CONTEXT_STRUCT *context);
static bool ws_check_utf8_finalize(uint8_t *data);
static uint32_t ws_recv(WS_CONTEXT_STRUCT *context);
static uint32_t ws_recv_fail(WS_CONTEXT_STRUCT *context);
static uint32_t ws_init(WS_INIT_STRUCT *init_params, WS_CONTEXT_STRUCT **context_out);
static void ws_deinit(WS_CONTEXT_STRUCT* context);
static uint32_t ws_process(WS_CONTEXT_STRUCT* context);

/*
 * WebSocket session task.
 */
void ws_session_task(void *init_ptr, void *creator) 
{
    WS_INIT_STRUCT    *params = (WS_INIT_STRUCT *) init_ptr;
    WS_CONTEXT_STRUCT *context;
    
    RTCS_ASSERT(params != NULL);
    RTCS_ASSERT(creator != NULL);

    if (ws_init(params, &context) != WS_ERR_OK)
    {
        if (context != NULL)
        {
            _mem_free(context);
        }
        RTCS_task_resume_creator(creator, (uint32_t)RTCS_ERROR);
        return;
    }
    
    /* Copy queue pointer to parameters so it is accessible from HTTP session. */
    params->tid = context->tid;
    RTCS_task_resume_creator(creator, RTCS_OK);
    _sched_yield();

    while(ws_process(context) != WS_ERR_FAIL)
    {	
    }

    ws_deinit(context);
}

/*
 * Upgrade HTTP session to WebSocket session.
 */
void ws_handshake(WS_HANDSHAKE_STRUCT *handshake)
{
    const char* guid = WS_GUID_STRING;
    SHA1_CTX    sha1_context;
    char        sha1_sum[SHA1_DIGEST_SIZE];

    RTCS_ASSERT(handshake != NULL);
    /* Get SHA-1 of key and WebSocket GUID and encode it in base64 */
    SHA1_Init(&sha1_context);
    SHA1_Update(&sha1_context, (uint8_t*) handshake->key, WS_KEY_LENGTH);
    SHA1_Update(&sha1_context, (uint8_t*) guid, WS_GUID_LENGTH);
    SHA1_Final(&sha1_context, (uint8_t*) sha1_sum);
    base64_encode_binary(sha1_sum, handshake->accept, SHA1_DIGEST_SIZE);
}

/*
 * Initialize WebSocket protocol.
 */
uint32_t ws_init(WS_INIT_STRUCT *init_params, WS_CONTEXT_STRUCT **context_out)
{
    WS_PLUGIN_STRUCT*           plugin = init_params->plugin;
    uint32_t                    sock = init_params->socket;
    uint32_t                    option = 0;
    WS_CONTEXT_STRUCT*          context;

    RTCS_ASSERT(init_params != NULL);

    /* Set receive timeout to ping period (in seconds) */
    option = WSCFG_PING_PERIOD*1000;
    (void) setsockopt(sock, SOL_TCP, OPT_RECEIVE_TIMEOUT, &option, sizeof(option));
    option = TRUE;
    (void) setsockopt(sock, SOL_TCP, OPT_RECEIVE_NOWAIT, &option, sizeof(option));
    
    /* Set NULL callbacks to empty function */
    if (plugin->on_error == NULL)
    {
        plugin->on_error = ws_do_nothing;
    }
    if (plugin->on_connect == NULL)
    {
        plugin->on_connect = ws_do_nothing;
    }
    if (plugin->on_message == NULL)
    {
        plugin->on_message = ws_do_nothing;
    }
    if (plugin->on_disconnect == NULL)
    {
        plugin->on_disconnect = ws_do_nothing;
    }
    /* Create WebSocket context structure */
    context = _mem_alloc_zero(sizeof(WS_CONTEXT_STRUCT));
    if (context == NULL)
    {
        *context_out = NULL;
        return(WS_ERR_FAIL);
    }
    *context_out = context;

    /* Copy socket handle. */
    context->sock = sock;
    context->plugin = plugin;

    /* Set handle so it is available for callbacks */
    context->u_context.handle = (uint32_t) context;
    context->state = WS_STATE_OPEN;

    /* Init UTF-8 buffer. */
    memset((void *) context->utf8.data, UTF8_TAIL_MIN, sizeof(context->utf8.data));

    /* Setup buffers - re-use HTTP session buffer; split it in half */
    context->tx_buffer.data = init_params->buffer;
    context->tx_buffer.size = init_params->buf_len/2;
    context->tx_buffer.offset = 0;

    context->hdr_buffer.data = context->tx_buffer.data + context->tx_buffer.size;
    context->hdr_buffer.size = WS_MAX_CLIENT_HEADER_LENGTH;
    context->hdr_buffer.offset = 0;

    context->rx_buffer.data = context->hdr_buffer.data + context->hdr_buffer.size;
    context->rx_buffer.size = init_params->buf_len/2 - WS_MAX_CLIENT_HEADER_LENGTH;
    context->rx_buffer.offset = 0;

    context->ctrl_buffer.data = (uint8_t *) _mem_alloc(WS_CONTROL_FRAME_LENGTH_MAX);
    if (context->ctrl_buffer.data == NULL)
    {
        return(WS_ERR_FAIL);
    }
    context->ctrl_buffer.size = WS_CONTROL_FRAME_LENGTH_MAX;
    context->ctrl_buffer.offset = 0;

    context->actual_buffer = &context->hdr_buffer;

    /* Init message queue for API calls*/
    context->api_queue = RTCS_mem_alloc_zero(sizeof(LWMSGQ_STRUCT) + WS_NUM_MESSAGES*sizeof(WS_API_CALL_MSG)*sizeof(_mqx_max_type));
    if 
        (
            (context->api_queue == NULL) ||
            (MQX_OK != _lwmsgq_init(context->api_queue, WS_NUM_MESSAGES, sizeof(WS_API_CALL_MSG)/sizeof(_mqx_max_type)))
        )
    {
        return(WS_ERR_FAIL);
    }
    context->tid = _task_get_id();

    /* Call user connect callback and set session state */
    plugin->on_connect(plugin->cookie, context->u_context);
    return(WS_ERR_OK);
}

/*
 * Process WebSocket session.
 */
static uint32_t ws_process(WS_CONTEXT_STRUCT* context)
{
    WS_FRAME_STRUCT    *rec_frame;
    WS_BUFFER_STRUCT   *buffer;
    uint32_t           retval;
    uint32_t           api_status;

    RTCS_ASSERT(context != NULL);
    retval = WS_ERR_PASS;
    buffer = context->actual_buffer;
    rec_frame = &context->frame;

    api_status = ws_process_api_calls(context);
    if (api_status == WS_ERR_FAIL)
    {
        retval = WS_ERR_FAIL;
        goto EXIT;
    }
    else if (api_status == WS_DO_RECV)
    {
        if (ws_recv(context) != WS_ERR_PASS)
        {
            retval = WS_ERR_FAIL;
            goto EXIT;
        }
        buffer = context->actual_buffer;

        /* Unmask data and set context pointer to it */
        ws_unmask_data(buffer->data+(buffer->offset-context->rec_data_size), rec_frame->mask_key, context->rec_data_size, context->mask_offset);
        ws_update_mask_offset(context);
        if (ws_validate_data(context) != WS_ERR_PASS)
        {
            ws_bad_frame(context, WS_ERR_NO_UTF8);
            retval = WS_ERR_FAIL;
            goto EXIT;
        }

        /* Process frame */
        if (ws_process_frame(context) == (uint32_t) RTCS_ERROR)
        {
            retval = WS_ERR_FAIL;
            goto EXIT;
        }
    }
    
    EXIT:
    return(retval);
}

/*
 * Receive data from socket.
 */
static uint32_t ws_recv(WS_CONTEXT_STRUCT *context)
{
    WS_FRAME_STRUCT   *frame;
    WS_BUFFER_STRUCT  *buffer;
    uint32_t          received_size;
    uint32_t          req_length;
    
    RTCS_ASSERT(context != NULL);
    buffer = context->actual_buffer;
    RTCS_ASSERT(buffer != NULL);
    context->rec_data_size = 0;

    if (buffer == &context->hdr_buffer)
    {
        req_length = WS_MIN_CLIENT_HEADER_LENGTH;
        context->hdr_buffer.offset = 0;
        context->ctrl_buffer.offset = 0;
        context->mask_offset = 0;
    }
    else
    {
        req_length = WS_MIN(buffer->size, context->remaining_data);
    }
    
    frame = &context->frame;

    while (req_length > 0)
    {
        uint32_t size;
        
        size = WS_MIN(buffer->size-buffer->offset, req_length);
        if (size == 0)
        {
            break;
        }

        received_size = recv(context->sock, buffer->data+buffer->offset, size, 0);
        if (received_size == (uint32_t)RTCS_ERROR)
        {
            return(ws_recv_fail(context));
        }

        buffer->offset += received_size;

        /* If we are reading frame header, receive data until whole header is in buffer. */
        if (buffer == &context->hdr_buffer)
        {
            req_length = ws_read_frame_header(buffer->data, frame, buffer->offset);
            if (req_length == 0)
            {
                WS_ERROR_CODE frame_status;

                frame_status = ws_check_frame_header(context, frame);
                if (frame_status != WS_ERR_OK)
                {
                    /* Report bad frame to client and close connection. */
                    ws_bad_frame(context, frame_status);
                    return(WS_ERR_BAD_FRAME);
                }
                if (frame->opcode & WS_OPCODE_CONTROL_MASK)
                {
                    buffer = &context->ctrl_buffer;
                    req_length = frame->length;
                }
                else
                {
                    buffer = &context->rx_buffer;
                    req_length = WS_MIN(frame->length, buffer->size);
                }
                frame->data = buffer->data;
                context->remaining_data = frame->length;
            }
            continue;
        }
        else
        {
            req_length -= received_size;
            context->rec_data_size += received_size;
            context->remaining_data -= received_size;
        }
    }
    context->actual_buffer = buffer;
    return(WS_ERR_PASS);
}

/*
 * Receive from socket failed.
 */
static uint32_t ws_recv_fail(WS_CONTEXT_STRUCT *context)
{
    WS_PLUGIN_STRUCT  *plugin;

    RTCS_ASSERT(context != NULL);
    plugin = context->plugin;
    RTCS_ASSERT(plugin != NULL);

    /* If receive timed out and ping was not send, send it */
    if
    (
        (RTCS_geterror(context->sock) == RTCSERR_TCP_TIMED_OUT) &&
        (context->state != WS_STATE_WAIT_PONG)
    )
    {
       ws_send_control_frame(context, (uint8_t*) WS_PING_STRING, strlen(WS_PING_STRING), WS_OPCODE_PING);
       context->state = WS_STATE_WAIT_PONG;
       return(WS_ERR_FAIL);
    }
    /* 
     * Socket is gone or response to ping was not received (client 
     * disconnected, etc.)
     */
    else
    {
        WS_USER_CONTEXT_STRUCT* user_context;

        user_context = &context->u_context;
        user_context->error = WS_ERR_SOCKET;
        plugin->on_error(plugin->cookie, *user_context);
        user_context->error = WS_ERR_OK;
        return(WS_ERR_FAIL);
    }
}

/*
 * Validate input data.
 */
static uint32_t ws_validate_data(WS_CONTEXT_STRUCT *context)
{
    WS_FRAME_STRUCT*         frame;
    WS_BUFFER_STRUCT*        buffer;
    WS_USER_CONTEXT_STRUCT*  user_context;
    uint32_t                 result;

    RTCS_ASSERT(context != NULL);
    user_context = &context->u_context;
    buffer = context->actual_buffer;
    RTCS_ASSERT(buffer != NULL);
    frame = &context->frame;
    result = WS_ERR_PASS;

    if 
    (
        (
            (frame->opcode == WS_OPCODE_TEXT) ||
            (
                (frame->opcode == WS_OPCODE_CONTINUATION) && 
                (user_context->data.type == WS_DATA_TEXT)
            )
        )
    )
    {
       
        uint8_t  *data;
        uint32_t length;

        data = buffer->data+(buffer->offset-context->rec_data_size);
        length = WS_MIN(context->rec_data_size, frame->length);
        result = ws_check_utf8(context, data, length);
    }
    return(result);
}

/*
 * Read all message from API message queue.
 */
static uint32_t ws_process_api_calls(WS_CONTEXT_STRUCT *context)
{
    WS_API_CALL_MSG  message;
    uint32_t         retval;
    int32_t          active;
    rtcs_fd_set      read_fd;
    rtcs_fd_set      except_fd;

    RTCS_ASSERT(context != NULL);
    RTCS_ASSERT(context->api_queue != NULL);

    retval = WS_ERR_PASS;

    RTCS_FD_ZERO(&read_fd);
    RTCS_FD_ZERO(&except_fd);

    RTCS_FD_SET(context->sock, &read_fd);
    RTCS_FD_SET(context->sock, &except_fd);

    /* Wait for any activity on socket - received data or exception. */
    active = select(1, &read_fd, NULL, &except_fd, 0);
    if (active == RTCS_ERROR)
    {
        retval = WS_ERR_FAIL;
    }
    else if (active >= 1)
    {
        /* "Exception" on socket indicates call of user API. */
        if (RTCS_FD_ISSET(context->sock, &except_fd))
        {
            uint32_t n_except;
            uint32_t s_except;

            /* Read exception number to zero it. */
            getsockopt(context->sock, SOL_SOCKET, SO_EXCEPTION, (void *) &n_except, &s_except);
            while (MQX_OK == _lwmsgq_receive(context->api_queue, (_mqx_max_type *) &message, LWMSGQ_TIMEOUT_FOR, 0, NULL))
            {
                uint16_t close_reason;
                
                switch (message.command)
                {
                    case WS_COMMAND_SEND:
                        /* If user transfer is in progress, set opcode to continuation. */
                        if (context->u_transfer == 1)
                        {
                            message.frame.opcode = WS_OPCODE_CONTINUATION;
                        }
                        /* No FIN -> set user transfer in progress flag. */
                        if (message.frame.fin == FALSE)
                        {
                            context->u_transfer = 1;
                        }
                        if (ws_send_frame(context, &message.frame) == (uint32_t)RTCS_ERROR)
                        {
                            retval = WS_ERR_FAIL;
                        }
                        /* Unblock task, which send the message */
                        _task_ready(_task_get_td((_task_id) message.data));
                        break;
                    case WS_COMMAND_CLOSE:
                        /* Send close frame, set correct state. */
                        retval = WS_ERR_SERVER;
                        close_reason = ntohs(WS_CLOSE_GOING_AWAY);
                        if (ws_send_control_frame(context, (uint8_t*) &close_reason, sizeof(close_reason), WS_OPCODE_CLOSE) == (uint32_t)RTCS_ERROR)
                        {
                            retval = WS_ERR_FAIL;
                        }
                        context->state = WS_STATE_CLOSING;
                        /* Unblock task, which send the message */
                        _task_ready(_task_get_td((_task_id) message.data));
                        break;
                    default:
                        break;
                }
                if ((message.command != WS_COMMAND_SEND) || (retval == WS_ERR_FAIL))
                {
                    break;
                }
            }
        }
        if (RTCS_FD_ISSET(context->sock, &read_fd))
        {
            /* Socket is read-ready. */
            retval = WS_DO_RECV;
        }
    }
    return(retval);
}

/*
 * Update mask offset according to remaining data.
 */
static inline void ws_update_mask_offset(WS_CONTEXT_STRUCT *context)
{
    WS_FRAME_STRUCT*  frame;
    
    RTCS_ASSERT(context != NULL);

    frame = &context->frame;
    /* 
     * If we do not store data on the beginning of buffer, correct mask
     * offset must be calculated.
     */
    if 
        (
            (context->remaining_data != 0) && 
            (context->remaining_data < frame->length)
        )
    {
        context->mask_offset = ((((frame->length-context->remaining_data)/4) * 4) + 4) - (frame->length-context->remaining_data);
    }
}

/*
 * Process received frame.
 */
static uint32_t ws_process_frame(WS_CONTEXT_STRUCT* context)
{
    WS_FRAME_STRUCT*            frame;
    WS_USER_CONTEXT_STRUCT*     user_context;
    uint32_t                    retval;

    RTCS_ASSERT(context != NULL);
    retval = (uint32_t) RTCS_ERROR;
    user_context = &context->u_context;
    frame = &context->frame;

    switch(frame->opcode)
    {
        case WS_OPCODE_TEXT:
            user_context->data.type = WS_DATA_TEXT;
            retval = ws_data_frame(context);
            break;
        case WS_OPCODE_BINARY:
            user_context->data.type = WS_DATA_BINARY;
            retval = ws_data_frame(context);
            break;
        case WS_OPCODE_CONTINUATION:
            retval = ws_data_frame(context);
            break;
        case WS_OPCODE_CLOSE:
            retval = ws_close_frame(context);
            break;
        case WS_OPCODE_PING:
            retval = ws_ping_frame(context);
            break;
        case WS_OPCODE_PONG:
            retval = ws_pong_frame(context);
            break;
        default:
            ws_bad_frame(context, WS_ERR_BAD_OPCODE);
            break;
    }
    if (retval != (uint32_t)RTCS_ERROR)
    {
        retval = RTCS_OK;
    }
    return(retval);
}

/*
 * Validate UTF-8 sequence.
 */
static uint32_t ws_check_utf8(WS_CONTEXT_STRUCT* context, uint8_t *data, uint32_t length)
{
    uint8_t  *bad;
    uint32_t missing_now;
    uint32_t missing_prev;

    RTCS_ASSERT(context != NULL);
    RTCS_ASSERT(data != NULL);

    missing_now = 0;
    missing_prev = context->utf8.missing;
    
    if (length == 0)
    {
        return(WS_ERR_PASS);
    }

    /*
     * If validation in previous step required more data to complete.
     */
    if (missing_prev != 0)
    {
        uint32_t size;

        size = WS_MIN(missing_prev, length);
        _mem_copy(data, context->utf8.data + context->utf8.index, size);
        context->utf8.index += size;

        if (!utf8_is_valid(context->utf8.data, context->utf8.index, &bad, &missing_now))
        {
            /*
             * If validation failed for second time, there are no missing bytes 
             * required for validation and we have last frame of message,
             * validation failed.
             */
            if 
                (
                    (missing_now == 0) ||
                    (
                        (context->remaining_data == 0) && 
                        (context->frame.fin == 1)
                    ) ||
                    !ws_check_utf8_finalize(context->utf8.data)
                )
            {
                return(WS_ERR_FAIL);
            }
            context->utf8.missing = missing_now;
            return(WS_ERR_PASS);
        }
        /*
         * If validation is successful reset stored UTF-8 context  and continue 
         * with next step.
         */
        data += size;
        length -= size;
        memset((void *) context->utf8.data, UTF8_TAIL_MIN, sizeof(context->utf8.data));
        context->utf8.missing = 0;
        context->utf8.index = 0;
    }

    if (!utf8_is_valid(data, length, &bad, &missing_now))
    {
        uint32_t size;
        
        /* Validation failed and there are no missing bytes. */
        if ((missing_now == 0) || (context->frame.fin == 1))
        {
            return(WS_ERR_FAIL);
        }
        size = WS_MIN(sizeof(context->utf8.data) - missing_now, length);
        _mem_copy(bad, context->utf8.data, size);
        context->utf8.missing = missing_now;
        context->utf8.index = size;
    }
    return(WS_ERR_PASS);
}

/*
 * Finalize UTF-8 check.
 */
static bool ws_check_utf8_finalize(uint8_t *data)
{
    uint8_t c;
    
    RTCS_ASSERT(data != NULL);
    c = *data;
    
    /* Find first zero bit and call check function. */
    if ((c | 0x7F) != 0xFF)
    {
        return(utf8_check_1(data));
    }
    else if ((c | 0xDF) != 0xFF)
    {
        return(utf8_check_2(data));
    }
    else if ((c | 0xEF) != 0xFF)
    {
        return(utf8_check_3(data));
    }
    else if ((c | 0xF7) != 0xFF)
    {
        return(utf8_check_4(data));
    }
    else
    {
        return(false);
    }
}

/*
 * Check validity of close reason.
 */
static uint32_t ws_check_close_reason(uint16_t reason)
{
    uint32_t retval = WS_ERR_FAIL;

    reason = ntohs(reason);

    /* Check if close reason lies within valid range */
    if ((reason >= WS_CLOSE_RESERVED_MIN) && (reason <= WS_CLOSE_PRIVATE_MAX))
    {
        if
            (
                (
                	(reason <= WS_CLOSE_SERVER_ERR) &&
                	(reason != WS_CLOSE_RESERVED1) &&
                	(reason != WS_CLOSE_RESERVED2) &&
                	(reason != WS_CLOSE_RESERVED3)
                ) ||
                (reason >= WS_CLOSE_APP_MIN)
            )
        {
            retval = WS_ERR_PASS;
        }
    }
    return(retval);
}

/*
 * Process close frame.
 */
static uint32_t ws_close_frame(WS_CONTEXT_STRUCT* context)
{
    WS_FRAME_STRUCT*   frame;
    uint16_t           close_reason;
    uint32_t           retval;

    RTCS_ASSERT(context != NULL);
    retval = RTCS_OK;
    frame = &context->frame;
    /* We initiated close and we are waiting for close from client */
    if (context->state == WS_STATE_CLOSING)
    {
        context->state = WS_STATE_CLOSED;
        retval = (uint32_t)WS_DO_DEINIT;
    }
    /* Client initiated close, so we reply with close frame + proper code */
    else if (context->remaining_data == 0)
    {
        if (frame->length == 0)
        {
            close_reason = ntohs(WS_CLOSE_OK);
        }
        else if (frame->length >= 2)
        {
            close_reason = *((uint16_t*) frame->data);
            
            /* Check close reason. */
            if (ws_check_close_reason(close_reason) != WS_ERR_PASS)
            {
                close_reason = ntohs(WS_CLOSE_PROT_ERROR);
            }

            /* If there is some message, verify that it is UTF-8 string. */
            if 
                (
                    (frame->length > 2) && 
                    (ws_check_utf8(context, frame->data + sizeof(uint16_t), frame->length - sizeof(uint16_t)) != WS_ERR_PASS)
                )  
            {
                close_reason = ntohs(WS_CLOSE_BAD_TYPE);
            }
        }
        else
        {
            close_reason = ntohs(WS_CLOSE_PROT_ERROR);
        }
        ws_send_control_frame(context, (uint8_t*) &close_reason, sizeof(close_reason), WS_OPCODE_CLOSE);
        context->state = WS_STATE_CLOSED;
        retval = (uint32_t)WS_DO_DEINIT;
    }
    return(retval);
}

/*
 * Process ping frame.
 */
static uint32_t ws_ping_frame(WS_CONTEXT_STRUCT* context)
{
    WS_FRAME_STRUCT*   frame;
    uint32_t           retval;

    RTCS_ASSERT(context != NULL);
    retval = RTCS_OK;
    frame = &context->frame;

    retval = ws_send_control_frame(context, frame->data, frame->length, WS_OPCODE_PONG);
    context->actual_buffer = &context->hdr_buffer;
    return(retval);
}

/*
 * Process pong frame.
 */
static uint32_t ws_pong_frame(WS_CONTEXT_STRUCT* context)
{
    uint32_t retval;

    RTCS_ASSERT(context != NULL);
    retval = RTCS_OK;

    /* If pong is solicited, reset state to open */
    if (context->state == WS_STATE_WAIT_PONG)
    {
        context->state = WS_STATE_OPEN;   
    }
    context->actual_buffer = &context->hdr_buffer;
    return(retval);
}

/*
 * Call user callback if required. 
 */
static uint32_t ws_data_frame(WS_CONTEXT_STRUCT* context)
{
    WS_BUFFER_STRUCT        *buffer;
    WS_USER_CONTEXT_STRUCT  *user_context;
    WS_FRAME_STRUCT         *frame;
    WS_PLUGIN_STRUCT        *plugin;
    uint32_t                retval = RTCS_OK;

    RTCS_ASSERT(context != NULL);
    plugin = context->plugin;
    user_context = &context->u_context;
    buffer = context->actual_buffer;
    RTCS_ASSERT(buffer != NULL);
    frame = &context->frame;

    /* Store length of received data in user context. */
    user_context->data.length += WS_MIN(frame->length, context->rec_data_size);
    user_context->data.data_ptr = buffer->data;
    
    /*
     * If there is no space for frame header or all data are read,
     * call on_message callback with received data.
     */
    if ((buffer->size - buffer->offset) == 0)
    {
        retval = plugin->on_message(plugin->cookie, *user_context);
        user_context->fin_flag = 0;
        user_context->data.length = 0;
        user_context->data.data_ptr = NULL;
        context->remaining_data -= user_context->data.length;
        context->actual_buffer->offset = 0;
    }
    else if (context->remaining_data == 0)
    {
        if (frame->fin == TRUE)
        {
            user_context->fin_flag = 1;
            context->rx_state = WS_RX_START;
            retval = plugin->on_message(plugin->cookie, *user_context);
            user_context->fin_flag = 0;
            user_context->data.length = 0;
            user_context->data.data_ptr = NULL;
            context->actual_buffer->offset = 0;
        }
        context->actual_buffer = &context->hdr_buffer;
    }
    /*
     * If frame is first in transmission and there are going to be continuation
     * frames, set correct receive state; so we can detect wrong frame sequence. 
     */
    if
        (
            (frame->fin == FALSE) &&
            (frame->opcode != WS_OPCODE_CONTINUATION) &&
            (context->rx_state == WS_RX_START)
        )
    {
        context->rx_state = WS_RX_CONT;
    }
    return retval;    
}

/*
 * Release WebSocket.
 */
static void ws_deinit(WS_CONTEXT_STRUCT *context)
{
    WS_PLUGIN_STRUCT* plugin;

    RTCS_ASSERT(context != NULL);
    plugin = context->plugin;
    RTCS_ASSERT(plugin != NULL);

    /* Call user disconnect callback */
    plugin->on_disconnect(plugin->cookie, context->u_context);

    if (context->api_queue != NULL)
    {
        _lwmsgq_deinit(context->api_queue);
        _mem_free(context->api_queue);
    }

    if (context->ctrl_buffer.data != NULL)
    {
        _mem_free(context->ctrl_buffer.data);
    }

    if (context->tx_buffer.data != NULL)
    {
        _mem_free(context->tx_buffer.data);
    }
    closesocket(context->sock);
    _mem_free(context);
}

/*
 * Unmask data from client.
 */
static void ws_unmask_data(uint8_t *data, uint32_t mask, uint32_t length, uint8_t mask_offset)
{
    uint32_t i;
    uint32_t words;
    uint32_t mask_key = mask;
    
    RTCS_ASSERT(data != NULL);
    /* If there is offset, shift mask to correct position first. */
    if (mask_offset > 0)
    {
        mask_offset = mask_offset*8;  
        mask_key <<= mask_offset;
        mask_key |= mask >> (sizeof(mask)*8 - mask_offset);
    }
    
    words = length/4;

    /* Unmask whole 32-bit long words first. */
    for(i = 0; i < words; i++)
    {
        *(((uint32_t*) data)+i) ^= mask_key;
    }

    /* Unmask rest of data (less then four bytes). */
    for(i = words*4; i < length; i++)
    {
        data[i] ^= *((uint8_t*)&mask_key + i%4);
    }
}

/*
 * Fill WebSocket frame structure from received data
 */
static uint32_t ws_read_frame_header(uint8_t *data, WS_FRAME_STRUCT *dst, uint32_t length)
{
    uint8_t *src = data;
    uint32_t req_length;

    /* Minimum is always required. */
    req_length = WS_MIN_CLIENT_HEADER_LENGTH;
    if
        (
            (src == NULL) ||
            (dst == NULL)
        )
    {
        return(req_length);
    }

    if (length < req_length)
    {
        return(req_length - length);
    }

    dst->fin = WS_GET_FIN(src);
    dst->reserved = WS_GET_RSV(src);
    dst->opcode = WS_GET_OPCODE(src);
    src++;
    dst->masked = WS_GET_MASK_FLAG(src);
    dst->length = WS_GET_LENGTH(src);
    src++;
    if (dst->length == WS_LENGTH_EXT16)
    {
        /* Length of 126 indicates that length is stored in next two bytes. */
        req_length += sizeof(uint16_t);
        if (length < req_length)
        {
            return(req_length - length);
        }
        dst->length = ntohs(*((uint16_t*) src));
        src += sizeof(uint16_t);
    }
    else if (dst->length == WS_LENGTH_EXT64)
    {
        /* Length of 127 indicates that length is stored in next eight bytes. */
        req_length += sizeof(uint64_t);
        if (length < req_length)
        {
            return(req_length - length);
        }
        /* Only frame sizes up to 32-bit long are supported by this implementation. */
        dst->length = ntohl(*((uint32_t*) src+1));
        src += sizeof(uint64_t);
    }
    if (dst->masked == TRUE)
    {
       dst->mask_key = *((uint32_t*) src);
       src += sizeof(uint32_t);
    }
    dst->data = data;
    return(0);
}

/*
 * Create frame header in buffer from frame structure.
 */
static void ws_write_frame(uint8_t *dst, WS_FRAME_STRUCT *src)
{
    RTCS_ASSERT(dst != NULL);
    RTCS_ASSERT(src != NULL);

    memmove(dst+ws_get_header_size(src), src->data, src->length);
    *dst = 0;
    if (src->fin)
    {
        *dst |= WS_FIN_MASK; 
    }
    *dst |= *((uint8_t*)(&src->opcode)) & WS_OPCODE_MASK;
    dst++;
    *dst = 0;

    if (src->length <= WS_LENGTH_NON_EXT)
    {
        *dst++ = *((uint8_t*)(&src->length)) & WS_LENGTH_MASK;
    }    
    else if ((src->length > WS_LENGTH_NON_EXT) & (src->length < UINT16MAX))
    {
        *dst++ = WS_LENGTH_EXT16;
        *((uint16_t*) dst) = htons(src->length & 0xFFFF);
        dst += sizeof(uint16_t);
    }
    else if (src->length >= UINT16MAX)
    {
        *dst++ = WS_LENGTH_EXT64;
        *((uint64_t*) dst) = htonl(src->length);
        dst += sizeof(uint64_t);
    }
    
    if (src->masked)
    {
        *((uint32_t*) dst) = src->mask_key;
        dst += sizeof(uint32_t);
    }
}

/*
 * Check frame validity
 */
static WS_ERROR_CODE ws_check_frame_header(WS_CONTEXT_STRUCT* context, WS_FRAME_STRUCT *frame)
{
    WS_ERROR_CODE retval = WS_ERR_OK;

    RTCS_ASSERT(context != NULL);
    RTCS_ASSERT(frame != NULL);

    /* Data MUST be masked. */
    if (frame->masked == FALSE)
    {
        retval = WS_ERR_BAD_FRAME;
    }

    /* There MUST be non-zero mask key. */
    else if (frame->mask_key == 0)
    {
        retval = WS_ERR_BAD_FRAME;
    }

    /* Reserved part of frame header MUST be zero. */
    if (frame->reserved > 0)
    {
        retval = WS_ERR_BAD_FRAME;
    }

    /* Opcode MUST be valid. */
    if 
        (
            (
                (frame->opcode > WS_OPCODE_BINARY) &&
                (frame->opcode < WS_OPCODE_CLOSE)
            ) ||
            (frame->opcode > WS_OPCODE_PONG)
        )
    {
        retval = WS_ERR_BAD_FRAME;
    }

    if
        (
            (frame->opcode & WS_OPCODE_CONTROL_MASK) && /* If frame is control frame and */
            (
                (frame->length > WS_CONTROL_FRAME_LENGTH_MAX) || /* its length is greater than 125 bytes or */
                (frame->fin == FALSE)    /* it is fragmented. */
            )
        )
    {
        retval = WS_ERR_BAD_FRAME; /* Frame is invalid. */
    }

    if
        (
            (
                (context->rx_state == WS_RX_CONT) &&          /* If we expect continuation and */
                !(frame->opcode & WS_OPCODE_CONTROL_MASK) &&  /* frame is not control frame and */
                (frame->opcode != WS_OPCODE_CONTINUATION)     /* frame is not continuation. */
            ) ||                                              /* Or if */
            (
                (context->rx_state == WS_RX_START) &&        /* We expect first frame in sequence and */
                (frame->opcode == WS_OPCODE_CONTINUATION)    /* frame is continuation. */
            )
        )
    {
        retval = WS_ERR_BAD_SEQ; /* Bad frame sequence is received. */
    }

    return(retval);
}

/*
 * Resolve bad frame, and report it to client.
 */
static void ws_bad_frame(WS_CONTEXT_STRUCT* context, WS_ERROR_CODE frame_status)
{
    uint16_t               close_reason = 0;
    WS_USER_CONTEXT_STRUCT *user_context;
    WS_PLUGIN_STRUCT       *plugin;

    RTCS_ASSERT(context != NULL);
    plugin = context->plugin;
    RTCS_ASSERT(plugin != NULL);
    user_context = &context->u_context;

    /* Send close with protocol error. */
    switch(frame_status)
    {
        case WS_ERR_BAD_OPCODE:
            close_reason = htons(WS_CLOSE_POLICY);
            break;
        case WS_ERR_NO_UTF8:
            close_reason = htons(WS_CLOSE_BAD_TYPE);
            break;
        case WS_ERR_BAD_FRAME:
        case WS_ERR_BAD_SEQ:   
        default:
            close_reason = htons(WS_CLOSE_PROT_ERROR);
            break;
    }
    ws_send_control_frame(context, (uint8_t*) &close_reason, sizeof(close_reason), WS_OPCODE_CLOSE);
    context->state = WS_STATE_CLOSING;

    /* Call user callback */
    user_context->error = WS_ERR_BAD_FRAME;
    user_context->data.type = WS_DATA_INVALID;
    user_context->data.data_ptr = NULL;
    user_context->data.length = 0;
    plugin->on_error(plugin->cookie, *user_context);
    user_context->error = WS_ERR_OK;
}

/*
 * Get total frame size (header + data)
 */
static inline uint32_t ws_get_frame_size(WS_FRAME_STRUCT *frame)
{
    return(ws_get_header_size(frame)+frame->length);
}

/*
 * Get frame header size in bytes.
 */
static uint32_t ws_get_header_size(WS_FRAME_STRUCT *frame)
{
    uint32_t retval = sizeof(uint16_t);

    RTCS_ASSERT(frame != NULL);
    if 
        (
            (frame->length > 0) && 
            (frame->length >= WS_LENGTH_EXT16)
        )
    {
        if (frame->length < UINT16MAX)
        {
            retval += sizeof(uint16_t);
        }
        else
        {
            retval += sizeof(uint64_t);
        } 
    }

    if (frame->masked == TRUE)
    {
        retval += sizeof(uint32_t);
    }
    
    return(retval);
}

/*
 * Send data to client. Use framing mechanism if required.
 */
static uint32_t ws_send_frame(WS_CONTEXT_STRUCT* context, WS_FRAME_STRUCT* frame)
{
    WS_BUFFER_STRUCT *buffer;
    uint32_t         data_length;
    uint32_t         retval;
    bool             user_fin;
    WS_PLUGIN_STRUCT *plugin;

    RTCS_ASSERT(context != NULL);
    RTCS_ASSERT(frame != NULL);
    plugin = context->plugin;
    RTCS_ASSERT(plugin != NULL);
    buffer = &context->tx_buffer;
    data_length = frame->length;
    user_fin = frame->fin;
    
    do
    {
        /* First case: We have all data from user and it fits in buffer. */
        if (((data_length + ws_get_header_size(frame)) < buffer->size) && user_fin)
        {
            frame->fin = TRUE;
            frame->length = data_length;
        }
        /* Second case: We have data from user, but it does not fit in buffer. */
        else if (((data_length + ws_get_header_size(frame)) > buffer->size) && user_fin)
        {
            frame->fin = FALSE;
            frame->length = buffer->size;
        }
        /*
         * Third and fourth case: We do not have all data from user (no fin) 
         * and it does fit in buffer OR we do have all data from user but 
         * no fin flag.
         */
        else
        {
            frame->fin = FALSE;
            frame->length = data_length;
        } 
        if (frame->fin == TRUE)
        {
            context->u_transfer = 0;
        }
        /* Write frame to buffer and send it. */
        ws_write_frame(buffer->data, frame);
        retval = send(context->sock, buffer->data, ws_get_frame_size(frame), 0);
        if (retval == (uint32_t)RTCS_ERROR)
        {
            break;
        }
        retval -= ws_get_header_size(frame);
        /*
         * Set opcode to continuation for next frame,
         * move data pointer, decrease remaining data length.
         */
        frame->opcode = WS_OPCODE_CONTINUATION;
        frame->data += retval;
        data_length -= retval;
    } while(data_length > 0);
    
    /* If send failed call error callback. */
    if (retval == (uint32_t)RTCS_ERROR) 
    {
        WS_USER_CONTEXT_STRUCT* user_context;

        user_context = &context->u_context;
        user_context->error = WS_ERR_SOCKET;
        user_context->data.data_ptr = frame->data;
        user_context->data.type = WS_DATA_INVALID;
        user_context->data.length = frame->length;
        plugin->on_error(plugin->cookie, *user_context);
        user_context->error = WS_ERR_OK;
    }

    return(retval);
}

/*
 * Empty function to replace plugin callback if it is set to NULL.
 */
static inline uint32_t ws_do_nothing(void* param, WS_USER_CONTEXT_STRUCT context)
{
    return(0);
}

/*
 * Send control frame.
 */
static uint32_t ws_send_control_frame(WS_CONTEXT_STRUCT* context, uint8_t *data, uint32_t length, uint32_t opcode)
{
    WS_FRAME_STRUCT frame = {0};

    RTCS_ASSERT(context != NULL);
    frame.data = data;
    frame.length = length;
    frame.opcode = opcode;
    frame.fin = TRUE;

    return(ws_send_frame(context, &frame));
}

/* EOF */
