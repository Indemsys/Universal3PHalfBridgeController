#ifndef __telnetcln_prv_h__
#define __telnetcln_prv_h__
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
*   This file contains private definitions for Telnet client application.
*
*END************************************************************************/
#include "telnetcln.h"
#include "telnet.h"

#if !MQX_USE_IO_OLD
#include <stdio.h>
extern const NIO_DEV_FN_STRUCT _io_socket_dev_fn;
extern const NIO_DEV_FN_STRUCT _io_telnet_dev_fn;
#define MQX_FILE_PTR FILE *
#endif

#define TELNETCLN_STACK_SIZE     (2000)
#define TELNETCLN_IN_TASK_NAME   "TELNETCLN read task"
#define TELNETCLN_OUT_TASK_NAME  "TELNETCLN write task"
#define TELNETCLN_EV_IN_EOF      (0x00000004)
#define TELNETCLN_EV_OUT_EOF     (0x00000008)
#define TELNETCLN_EV_ALL         (TELNETCLN_EV_IN_EOF | TELNETCLN_EV_OUT_EOF)
#define TELNETCLN_CONTEXT_VALID  (0x54434C56)
#define TELNETCLN_BUFFER_SIZE    (256)

#define TELNETCLN_BINARY_FLAG    (0x00000001)
#define TELNETCLN_ECHO_FLAG      (0x00000002)
#define TELNETCLN_SUPRESS_FLAG   (0x00000004)

typedef enum telnetcln_error
{
    TELNETCLN_OK,
    TELNETCLN_FAIL,
    TELNETCLN_ERR_NO_SOCKET,
    TELNETCLN_ERR_NO_CONN,
    TELNETCLN_ERR_OPEN_FAIL
}TELNETCLN_ERROR;

typedef struct telnetcln_context
{
    uint32_t                valid;          /* Context validity flag. */
    TELNETCLN_PARAM_STRUCT  params;         /* Parameters for client initialization. */
    uint32_t                sock;           /* Socket */
    MQX_FILE_PTR            sockfd;         /* Socket file descriptor for communication with server. */
    MQX_FILE_PTR            telnetfd;       /* Telnet device file descriptor. */
    uint32_t                local_options;  /* Flags specifying local options. */
    uint32_t                remote_options; /* Flags specifying remote options. */
    uint32_t                rx_tid;         /* Task ID of RX task. */
    uint32_t                tx_tid;         /* Task ID of TX task. */
} TELNETCLN_CONTEXT;

void telnetcln_in_task(void *v_context, void * creator);
void telnetcln_out_task(void *v_context, void * creator);
void telnetcln_cleanup(TELNETCLN_CONTEXT *context);
TELNETCLN_ERROR telnetcln_init_socket(TELNETCLN_CONTEXT *context);
TELNETCLN_CONTEXT * telnetcln_create_context(TELNETCLN_PARAM_STRUCT *params);
TELNETCLN_ERROR telnetcln_init_devices(TELNETCLN_CONTEXT *context);

#endif // __telnetcln_prv_h__
