/*HEADER**********************************************************************
*
* Copyright 2013 Freescale Semiconductor, Inc.
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
*   This file contains the HTTPSRV implementation.
*
*
*END************************************************************************/

#include "httpsrv.h"
#include "httpsrv_supp.h"
#include "httpsrv_prv.h"
#include "rtcs_assert.h"
#include <string.h>

#define HTTPSRV_SERVER_TASK_NAME "HTTP server"

/*
** Function for starting the HTTP server 
**
** IN:
**      HTTPSRV_PARAM_STRUCT*   params - server parameters (port, ip, index page etc.)
**
** OUT:
**      none
**
** Return Value: 
**      uint32_t      server handle if successful, NULL otherwise
*/
uint32_t HTTPSRV_init(HTTPSRV_PARAM_STRUCT *params)
{
    HTTPSRV_SERVER_TASK_PARAM server_params;

    server_params.params = params;

    /* Server must run with lower priority than TCP/IP task. */
    if (params->server_prio == 0)
    {
        params->server_prio = HTTPSRVCFG_DEF_SERVER_PRIO;
    }
    else if (params->server_prio < _RTCSTASK_priority)
    {
        return(0);
    }
    
    /* Run server task. */
    if (RTCS_task_create(HTTPSRV_SERVER_TASK_NAME, params->server_prio, 
                        (params->ssl_params != NULL) ? HTTPSRV_SSL_SERVER_STACK_SIZE : HTTPSRV_SERVER_STACK_SIZE, 
                        httpsrv_server_task, &server_params) != RTCS_OK)
    {
        HTTPSRV_release(server_params.handle);
        return(0);
    }

    return(server_params.handle);
}

/*
** Function for releasing/stopping HTTP server 
**
** IN:
**      uint32_t       server_h - server handle
**
** OUT:
**      none
**
** Return Value: 
**      uint32_t      error code. HTTPSRV_OK if everything went right, positive number otherwise
*/
uint32_t HTTPSRV_release(uint32_t server_h)
{
    HTTPSRV_STRUCT* server = (void *) server_h;

    RTCS_ASSERT(server != NULL);
    
    if (server == NULL)
    {
        return((uint32_t)RTCS_ERROR);
    }
    
    /* Shutdown server task and wait for its termination. */
    if (server->sock_v4)
    {
        shutdownsocket(server->sock_v4, SHUT_RDWR);
    }
    if (server->sock_v6)
    {
        shutdownsocket(server->sock_v6, SHUT_RDWR);
    }
    
    while(server->server_tid)
    {
        _sched_yield();
    }
    _mem_free(server);
    return(RTCS_OK);
}

/*
** Write data to client from CGI script
**
** IN:
**      HTTPSRV_CGI_RES_STRUCT* response - CGI response structure used for forming response
**
** OUT:
**      none
**
** Return Value:
**      uint_32 - Number of bytes written
*/
uint32_t HTTPSRV_cgi_write(HTTPSRV_CGI_RES_STRUCT* response)
{
    HTTPSRV_SESSION_STRUCT* session = (HTTPSRV_SESSION_STRUCT*) response->ses_handle;
    uint32_t retval = 0;
    int32_t  wrote;

    if (session == NULL)
    {
        return(0);
    }

    if (!(session->flags & HTTPSRV_FLAG_HEADER_SENT))
    {
        session->response.status_code = response->status_code;
        session->response.content_type = response->content_type;
        session->response.length = response->content_length;

        if (response->content_length < 0)
        {
            session->flags |= HTTPSRV_FLAG_IS_TRANSCODED;
        }

        /* 
         * Ignore rest of received data in buffer (set buffer offset to zero).
         * We do this because otherwise any buffered but unread data would be 
         * part of response (before header) and render it invalid. 
         */
        if (session->buffer.offset <= session->request.content_length)
        {
            session->request.content_length -= session->buffer.offset;
        }
        session->buffer.offset = 0;
        httpsrv_sendhdr(session, response->content_length, 1);
    }

    if (session->flags & HTTPSRV_FLAG_IS_TRANSCODED)
    {
        char length_str[sizeof("FFFFFFFF\r\n")] = {0};

        /* Write length. */
        snprintf(length_str, sizeof(length_str), "%x\r\n", response->data_length);
        wrote = httpsrv_write(session, length_str, strlen(length_str));
        if (wrote < 0)
        {
            retval = 0;
            goto EXIT;
        }
        /* Write data. */
        wrote = httpsrv_write(session, response->data, response->data_length);
        if (wrote < 0)
        {
            retval = 0;
            goto EXIT;
        }
        retval = wrote;
        /* Write tail. */
        wrote = httpsrv_write(session, "\r\n", strlen("\r\n"));
        if (wrote < 0)
        {
            retval = 0;
            goto EXIT;
        }
    }
    else if ((response->data != NULL) && (response->data_length > 0))
    {
        retval = httpsrv_write(session, response->data, response->data_length);
    }
    
    EXIT:
    session->time = RTCS_time_get();
    return(retval);
}

/*
** Read data from client to CGI script
**
** IN:
**      uint32_t ses_handle - handle to session used for reading
**      char*   buffer - user buffer to read data to
**      uint32_t length - size of buffer in bytes
**
** OUT:
**      none
**
** Return Value:
**      uint32_t - Number of bytes read
*/
uint32_t HTTPSRV_cgi_read(uint32_t ses_handle, char* buffer, uint32_t length)
{
    HTTPSRV_SESSION_STRUCT* session = (HTTPSRV_SESSION_STRUCT*) ses_handle;
    uint32_t retval;

    if 
        (
            (session == NULL) ||
            (buffer == NULL) ||
            (length == 0)
        )
    {
        return(0);
    }

    retval = httpsrv_read(session, buffer, length);

    if (retval > 0)
    {
        RTCS_ASSERT(retval <= session->request.content_length);
        if (retval <= session->request.content_length)
        {
            session->request.content_length -= retval;
        }
    }
    else
    {
        if (RTCS_geterror(session->sock) == RTCSERR_TCP_TIMED_OUT)
        {
            retval = 0;
        }
    }
    session->time = RTCS_time_get();
    return(retval);
}

/*
** Write data to client from server side include
**
** IN:
**      uint32_t ses_handle - session foe writing
**      char*   data - user data to write
**      uint32_t length - size of data in bytes
**
** OUT:
**      none
**
** Return Value:
**      uint32_t - Number of bytes written
*/
uint32_t HTTPSRV_ssi_write(uint32_t ses_handle, char* data, uint32_t length)
{
    HTTPSRV_SESSION_STRUCT* session = (HTTPSRV_SESSION_STRUCT*) ses_handle;
    uint32_t retval = 0;

    if ((session != NULL) && (data != NULL) && (length))
    {
        retval = httpsrv_write(session, data, length);
    }
    
    return(retval);
}
/* EOF */
