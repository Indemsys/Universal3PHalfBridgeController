/*HEADER**********************************************************************
*
* Copyright 2008, 2014 Freescale Semiconductor, Inc.
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

#include <rtcs.h>
#include <lwevent.h>

#if MQX_USE_IO_OLD
#include <fio.h>
#else
#include <stdio.h>
#include <nio.h>
#include <nio/ioctl.h>
#define IO_EOF EOF
extern const NIO_DEV_FN_STRUCT _io_socket_dev_fn;
#endif

#include "telnet.h"
#include "telnetcln_prv.h"

/*TASK*-----------------------------------------------------------------
*
* Function Name  : TELNETCLN_connect
* Returned Value : Handle to client application if successful, zero otherwise
* Comments       : Initialization function for Telnet client
*
*END------------------------------------------------------------------*/

uint32_t TELNETCLN_connect(TELNETCLN_PARAM_STRUCT *params)
{
    TELNETCLN_ERROR   error;
    uint32_t          prio;
    TELNETCLN_CONTEXT *context;

    error = TELNETCLN_OK;
    context = telnetcln_create_context(params);
    if (context == NULL)
    {
        error = TELNETCLN_FAIL;
        goto EXIT;
    }
    /*
    ** Install device driver for socket and telnet
    */
    error = telnetcln_init_socket(context);
    if (error != TELNETCLN_OK)
    {
        error = TELNETCLN_FAIL;
        goto EXIT;
    }

    error = telnetcln_init_devices(context);
    if (error != TELNETCLN_OK)
    {
        error = TELNETCLN_FAIL;
        goto EXIT;
    }
   
    /* 
     * Create two processes:
     * 1. to read input and send read characters to socket
     * 2. to read from socket and write the received chars to output
     */
    if (MQX_OK != _task_get_priority(MQX_NULL_TASK_ID, &prio))
    {
        error = TELNETCLN_FAIL;
        goto EXIT;
    }

    if (RTCS_task_create(TELNETCLN_OUT_TASK_NAME, prio, TELNETCLN_STACK_SIZE, telnetcln_in_task, (void*)context) != RTCS_OK)
    {
        error = TELNETCLN_FAIL;
        goto EXIT;
    }
    if (RTCS_task_create(TELNETCLN_IN_TASK_NAME, prio, TELNETCLN_STACK_SIZE, telnetcln_out_task, (void*)context) != RTCS_OK)
    {
        error = TELNETCLN_FAIL;
        goto EXIT;
    }
    _task_ready(_task_get_td((_task_id) context->rx_tid));
    context->params.callbacks.on_connected(context->params.callbacks.param);
    EXIT:
    if (error != TELNETCLN_OK)
    {
        telnetcln_cleanup(context);
        _mem_free(context);
        context = NULL;
    }
    return((uint32_t) context);
}

/*
 * Terminate connection to remote host, close handles, free memory.
 */
uint32_t TELNETCLN_disconnect(uint32_t handle)
{
    TELNETCLN_CONTEXT *context;

    context = (TELNETCLN_CONTEXT *) handle;

    if (context->valid != TELNETCLN_CONTEXT_VALID)
    {
        return((uint32_t) RTCS_ERROR);
    }
    
    context->valid = 0;

    while ((context->tx_tid != 0) && (context->rx_tid != 0))
    {
        _sched_yield();
    }
    _mem_free(context);
    return(RTCS_OK);
}

/*
 * Check if client is running
 */
uint32_t TELNETCLN_get_status(uint32_t handle)
{
    TELNETCLN_CONTEXT *context;
    uint32_t retval;

    context = (TELNETCLN_CONTEXT *) handle;
    if (context->valid != TELNETCLN_CONTEXT_VALID)
    {
        retval = TELNETCLN_STATUS_STOPPED;
    }
    else
    {
        retval = TELNETCLN_STATUS_RUNNING;
    }
    return(retval);
}
