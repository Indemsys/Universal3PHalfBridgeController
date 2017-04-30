#ifndef __tftpcln_prv_h__
#define __tftpcln_prv_h__
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
*   This file contains private definitions needed by TFTP client.
*
*END************************************************************************/

#include "tftpcln.h"
#include "tftp_prv.h"

#if !MQX_USE_IO_OLD
#include <stdio.h>
#endif

#define TFTPCLN_STACK_SIZE     (2000)
#define TFTPCLN_TASK_NAME      "TFTPCLN task"
#define TFTPCLN_CONTEXT_VALID  (0x5446434c)
#define TFTPCLN_BUFFER_SIZE    (256)
#define TFTPCLN_MSG_COUNT      (3)

#define TFTPCLN_RD_MAX               (30000)
#define TFTPCLN_RC_MAX               (10)
#define TFTPCLN_RC_INIT              (1000)

typedef enum tftpcln_command
{
    TFTPCLN_COMMAND_SEND,
    TFTPCLN_COMMAND_RECEIVE,
    TFTPCLN_COMMAND_STOP
}TFTPCLN_COMMAND;

typedef struct tftpcln_api_message
{
    TFTPCLN_COMMAND command;
    uint32_t        src_tid;
    char            *l_file;
    char            *r_file;
    int32_t         *result;
}TFTPCLN_API_MESSAGE;

typedef enum tftpcln_error
{
    TFTPCLN_OK,
    TFTPCLN_FAIL,
    TFTPCLN_ERR_NO_SOCKET,
    TFTPCLN_ERR_NO_CONN,
    TFTPCLN_ERR_OPEN_FAIL
}TFTPCLN_ERROR;

typedef struct tftpcln_context
{
    volatile uint32_t     valid;          /* Context validity flag. */
    uint32_t              tid;            /* Task ID of client task. */
    void                  *api_queue;     /* Message queue for receiving API calls. */
    TFTPCLN_PARAM_STRUCT  params;         /* Parameters for client initialization. */
    TFTP_TRANSACTION      trans;          /* Transaction structure. */
}TFTPCLN_CONTEXT;

typedef struct tftpcln_task_params_struct
{
    TFTPCLN_PARAM_STRUCT *params;
    TFTPCLN_CONTEXT      *context;
}TFTPCLN_TASK_PARAMS_STRUCT;

#ifdef __cplusplus
extern "C" {
#endif

void tftpcln_task(void *v_context, void * creator);
TFTPCLN_ERROR tftpcln_init_socket(TFTPCLN_CONTEXT *context);
TFTPCLN_CONTEXT * tftpcln_create_context(TFTPCLN_PARAM_STRUCT *params);
void tftpcln_cleanup(TFTPCLN_CONTEXT *context, bool free_context);

#ifdef __cplusplus
}
#endif

#endif
