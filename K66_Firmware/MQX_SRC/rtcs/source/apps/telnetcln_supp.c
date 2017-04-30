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
*   This file contains the Telnet client implementation.
*
*
*END************************************************************************/

#include "telnetcln_prv.h"
#if PLATFORM_SDK_ENABLED
#include "nio/ioctl.h"
#endif

/*
 * Dummy callback.
 */
static inline void telnetcln_dummy(void *param)
{
    return;
}

/*
 * Close handles, free context.
 */
void telnetcln_cleanup(TELNETCLN_CONTEXT *context)
{
    uint32_t flags;

    if (context != NULL)
    {
        /* This will hopefully enable translation of CR+LF in serial driver. */
        ioctl(context->params.fd_in, IO_IOCTL_SERIAL_GET_FLAGS, &flags);
        flags |= IO_SERIAL_TRANSLATION;
        ioctl(context->params.fd_in, IO_IOCTL_SERIAL_SET_FLAGS, &flags);

        if (context->telnetfd)
        {
            fclose(context->telnetfd);
        }
        if (context->sockfd)
        {
            fclose(context->sockfd);
        }
        if (context->sock)
        {
            closesocket(context->sock);
        }
        context->valid = 0;
    }
}

/*
 * Allocate context structure and fill it.
 */
TELNETCLN_CONTEXT * telnetcln_create_context(TELNETCLN_PARAM_STRUCT *params)
{
    TELNETCLN_CONTEXT *context;

    context = RTCS_mem_alloc_system_zero(sizeof(TELNETCLN_CONTEXT));
    if (context != NULL)
    {
        context->params = *params;
        context->valid = TELNETCLN_CONTEXT_VALID;
        context->tx_tid = 1;
        context->rx_tid = 1;
        if (context->params.callbacks.on_connected == NULL)
        {
            context->params.callbacks.on_connected = telnetcln_dummy;
        }
        if (context->params.callbacks.on_disconnected == NULL)
        {
            context->params.callbacks.on_disconnected = telnetcln_dummy;
        }
    }
    return(context);
}

/*
 * Initialize socket and connect to remote host.
 */
TELNETCLN_ERROR telnetcln_init_socket(TELNETCLN_CONTEXT *context)
{
    uint32_t sock;
    uint32_t error;
    sockaddr *remote;

    remote = &context->params.sa_remote_host;

    sock = socket(remote->sa_family, SOCK_STREAM, 0);
    if (sock == RTCS_SOCKET_ERROR)
    {
        return(TELNETCLN_ERR_NO_SOCKET);
    }

    error = connect(sock, (const sockaddr *)(remote), sizeof(*remote));
    if (error != RTCS_OK)
    {
        struct linger l_options;

        /* Set linger options for RST flag sending. */
        l_options.l_onoff = 1;
        l_options.l_linger_ms = 0;
        setsockopt(sock, SOL_SOCKET, SO_LINGER, &l_options, sizeof(l_options));
        closesocket(sock);
        return(TELNETCLN_ERR_NO_CONN);
    }
    context->sock = sock;
    return(TELNETCLN_OK);
}

/*
 * Initialize socket and telnet drivers.
 */
TELNETCLN_ERROR telnetcln_init_devices(TELNETCLN_CONTEXT *context)
{
    TELNETCLN_ERROR retval;
    uint32_t        flags;

    retval = TELNETCLN_OK;

    #if MQX_USE_IO_OLD
    _io_socket_install("socket:");
    _io_telnet_install("telnet:");

    context->sockfd = fopen("socket:", (char *) context->sock);
    if (context->sockfd != NULL)
    {
        context->telnetfd = fopen("telnet:", (char *) context->sockfd);
    }
    /* This will hopefully disable translation of CR+LF in serial driver. */
    ioctl(context->params.fd_in, IO_IOCTL_SERIAL_GET_FLAGS, &flags);
    flags &= ~IO_SERIAL_TRANSLATION; 
    ioctl(context->params.fd_in, IO_IOCTL_SERIAL_SET_FLAGS, &flags);
    /* Disable remote echo */
    ioctl(context->telnetfd, IO_IOCTL_SERIAL_GET_FLAGS, &flags);
    flags &= ~IO_SERIAL_ECHO; 
    ioctl(context->telnetfd, IO_IOCTL_SERIAL_SET_FLAGS, &flags);
    /* Set line ending mode */
    ioctl(context->telnetfd, IO_IOCTL_SET_STREAM, (uint32_t *) &context->params.fd_in);
    flags = false;
    ioctl(context->telnetfd, IO_IOCTL_SET_CRLF, &flags);
    flags = true;
    ioctl(context->telnetfd, IO_IOCTL_SET_RAW, &flags);

    #else /* NIO version */
    #define TELNETCLN_DEV_NAME_MAX (20)

    char dev_name[TELNETCLN_DEV_NAME_MAX] = {0}; 

    _nio_dev_install("socket:", &_io_socket_dev_fn, NULL, NULL);
    _nio_dev_install("telnet:", &_io_telnet_dev_fn, NULL, NULL);

    snprintf(dev_name, TELNETCLN_DEV_NAME_MAX, "socket:%i", context->sock);
    context->sockfd = fopen(dev_name, "r+");

    if (context->sockfd != NULL)
    {
        snprintf(dev_name, TELNETCLN_DEV_NAME_MAX, "telnet:%i", context->sockfd);
        context->telnetfd = fopen(dev_name, "r+");
    }

    ioctl(fileno(context->telnetfd), IO_IOCTL_SET_STREAM, (uint32_t *) &context->params.fd_in);
    flags = false;
    ioctl(fileno(context->telnetfd), IO_IOCTL_SET_CRLF, &flags);
    flags = true;
    ioctl(fileno(context->telnetfd), IO_IOCTL_SET_RAW, &flags);
    #endif
   
    if (context->telnetfd == NULL)
    {
        fclose(context->sockfd);
        context->sockfd = NULL;
        retval = TELNETCLN_FAIL;
    }
    return(retval);
}
