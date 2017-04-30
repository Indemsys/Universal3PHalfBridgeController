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
#include "tftp_prv.h"
#include <lwmsgq.h>

static inline void tftpcln_dummy_callback (uint32_t unused);
static inline void tftpcln_dummy_err_callback(uint16_t unused0, char* unused1);

/*
 * Close handles, free context.
 */
void tftpcln_cleanup(TFTPCLN_CONTEXT *context, bool free_context)
{
    if (context != NULL)
    {   
        if (context->trans.sock)
        {
            closesocket(context->trans.sock);
        }
        if (context->trans.file)
        {
            fflush(context->trans.file);
            fclose(context->trans.file);
        }
        if (context->api_queue)
        {
            _lwmsgq_deinit(context->api_queue);
            _mem_free(context->api_queue);
        }
        if (context->trans.buffer)
        {
            _mem_free(context->trans.buffer);
        }
        context->valid = 0;
        if (free_context)
        {
            _mem_free(context);
        }
    }
}

/*
 * Allocate context structure and fill it.
 */
TFTPCLN_CONTEXT * tftpcln_create_context(TFTPCLN_PARAM_STRUCT *params)
{
    TFTPCLN_CONTEXT *context;

    context = RTCS_mem_alloc_system_zero(sizeof(TFTPCLN_CONTEXT));
    RTCS_ASSERT(context != NULL);
    if (context != NULL)
    {
        context->params = *params;
        context->valid = TFTPCLN_CONTEXT_VALID;
        if (context->params.recv_callback == NULL)
        {
            context->params.recv_callback = tftpcln_dummy_callback;
        }
        if (context->params.send_callback == NULL)
        {
            context->params.send_callback = tftpcln_dummy_callback;
        }
        if (context->params.error_callback == NULL)
        {
            context->params.error_callback = tftpcln_dummy_err_callback;
        }
        context->api_queue = RTCS_mem_alloc(sizeof(LWMSGQ_STRUCT) + TFTPCLN_MSG_COUNT*sizeof(TFTPCLN_API_MESSAGE)*sizeof(_mqx_max_type));
        RTCS_ASSERT(context->api_queue != NULL);
        if (context->api_queue != NULL)
        {
            if (MQX_OK != _lwmsgq_init(context->api_queue, TFTPCLN_MSG_COUNT, sizeof(TFTPCLN_API_MESSAGE)/sizeof(_mqx_max_type)))
            {
                tftpcln_cleanup(context, true);
                context = NULL;
            }

            context->trans.buffer = RTCS_mem_alloc(TFTP_MAX_MESSAGE_SIZE);
            RTCS_ASSERT(context->trans.buffer != NULL);
            if (context->trans.buffer == NULL)
            {
                tftpcln_cleanup(context, true);
                context = NULL;
            }
        }
        else
        {
            tftpcln_cleanup(context, true);
            context = NULL;
        }
        context->trans.remote_sa = context->params.sa_remote_host;
        context->trans.flags = TFTP_FLAG_SAVE_REMOTE;
    }
    return(context);
}

/*
 * Initialize socket and connect to remote host.
 */
TFTPCLN_ERROR tftpcln_init_socket(TFTPCLN_CONTEXT *context)
{
    uint32_t sock;
    sockaddr *remote;

    RTCS_ASSERT(context != NULL);
    remote = &context->params.sa_remote_host;

    sock = socket(remote->sa_family, SOCK_DGRAM, 0);
    RTCS_ASSERT(sock != RTCS_SOCKET_ERROR);
    if (sock == RTCS_SOCKET_ERROR)
    {
        return(TFTPCLN_ERR_NO_SOCKET);
    }

    context->trans.sock = sock;
    return(TFTPCLN_OK);
}

/*
 * Dummy function - called instead of user callback if it is not specified.
 */
static inline void tftpcln_dummy_callback (uint32_t data_length)
{
    return;
}

static inline void tftpcln_dummy_err_callback(uint16_t unused0, char* unused1)
{
    return;
}
