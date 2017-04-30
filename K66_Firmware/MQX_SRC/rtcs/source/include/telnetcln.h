#ifndef __telnetcln_h__
#define __telnetcln_h__
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
*   This file contains public definitions for Telnet client application.
*
*END************************************************************************/
#include "rtcs.h"

#if !MQX_USE_IO_OLD
#include <stdio.h>
#define MQX_FILE_PTR FILE *
#endif

#define TELNETCLN_STATUS_STOPPED (0)
#define TELNETCLN_STATUS_RUNNING (1)

/*
 * Prototype for telnet client callback.
 */
typedef void (TELNETCLN_CALLBACK)(void *param);

/*
 * Structure containing telnet all client callbacks
 */
typedef struct telnetcln_callbacks_struct
{
    TELNETCLN_CALLBACK *on_connected;    /* Invoked when client is connected. */
    TELNETCLN_CALLBACK *on_disconnected; /* Invoked when client is disconnected. */
    void  *param;                        /* Parameter for callbacks. */
}TELNETCLN_CALLBACKS_STRUCT;

typedef struct telnetcln_param_struct
{
    sockaddr                   sa_remote_host; /* Information about remote host client will connect to. */
    bool                       use_nagle;      /* enable/disable nagle algorithm for server sockets. */
    MQX_FILE_PTR               fd_in;          /* Input file descriptor. */
    MQX_FILE_PTR               fd_out;         /* Output file descriptor. */
    TELNETCLN_CALLBACKS_STRUCT callbacks;      /* Callbacks structure. */
}TELNETCLN_PARAM_STRUCT;

uint32_t TELNETCLN_connect(TELNETCLN_PARAM_STRUCT *params);
uint32_t TELNETCLN_disconnect(uint32_t handle);
uint32_t TELNETCLN_get_status(uint32_t handle);

#endif // __telnetcln_h__

