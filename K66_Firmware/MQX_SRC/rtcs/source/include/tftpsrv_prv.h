#ifndef __tftpsrv_prv_h__
#define __tftpsrv_prv_h__
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
*   Private header for TFTP server application.
*
*
*END************************************************************************/

#include "tftpsrv.h"
#include "tftp_prv.h"

#if !MQX_USE_IO_OLD
#include <stdio.h>
#endif

#define TFTPSRV_SERVER_STACK_SIZE   (2000)
#define TFTPSRV_SESSION_STACK_SIZE  (2000)
#define TFTPSRV_FILENAME_MAX_LENGTH (128)

/*
 * Session process function prototype
 */
typedef void(*TFTPSRV_SES_FUNC)(void* server, void *session);

/*
* TFTP session structure
*/
typedef struct tftpsrv_session_struct
{
    volatile bool         valid;        /* non zero (1) = session is valid - data in this entry is valid */
    TFTPSRV_SES_FUNC      process_func; /* Session process function */
    TFTP_TRANSACTION      trans;        /* TFTP transaction structure */
} TFTPSRV_SESSION_STRUCT;

/*
 * Parameters for server task
 */
typedef struct tftpsrv_server_task_param
{
    uint32_t             handle;    /* [out] Server handle, non-zero if initialization was successful */
    TFTPSRV_PARAM_STRUCT *params;   /* [in] Server parameters */
}TFTPSRV_SERVER_TASK_PARAM;

/*
 * TFTP server context structure
 */
typedef struct tftpsrv_struct
{
    TFTPSRV_PARAM_STRUCT   params;          /* server parameters */
    uint32_t               sock_v4;         /* listening socket for IPv4 */
    uint32_t               sock_v6;         /* listening socket for IPv6 */
    TFTPSRV_SESSION_STRUCT **session;       /* array of pointers to sessions */
    volatile _task_id      server_tid;      /* Server task ID */
    volatile _task_id      *ses_tid;        /* Session task IDs */
    LWSEM_STRUCT           tid_sem;         /* Semaphore for session TID array locking */
    LWSEM_STRUCT           ses_cnt;         /* Session counter */
}TFTPSRV_STRUCT;

typedef struct tftpsrv_ses_task_param
{
    TFTPSRV_STRUCT *server;   /* Pointer to server context. */
    uint32_t       sock;      /* Socket with detected activity. */
}TFTPSRV_SES_TASK_PARAM;

#ifdef __cplusplus
extern "C" {
#endif

void tftpsrv_server_task(void *dummy, void *creator);
TFTPSRV_STRUCT* tftpsrv_create_server(TFTPSRV_PARAM_STRUCT* params);
int32_t tftpsrv_destroy_server(TFTPSRV_STRUCT* server);
#if MQX_USE_IO_OLD
int32_t tftpsrv_open_file(uint16_t opcode, char *filename, char *root, char *filemode, MQX_FILE_PTR *file_ptr);
#else
int32_t tftpsrv_open_file(uint16_t opcode, char *filename, char *root, char *filemode, FILE **file_ptr);
#endif

void tftpsrv_transaction_prologue(void* server_v, void *session_v);
void tftpsrv_transaction_epilogue(void* server_v, void *session_v);

#ifdef __cplusplus
}
#endif

#endif
