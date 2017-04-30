#ifndef __telnetsrv_prv_h__
#define __telnetsrv_prv_h__
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
*   Private header for telnet server application.
*
*
*END************************************************************************/

#include "telnetsrv.h"
#if !MQX_USE_IO_OLD
#include <stdio.h>
#endif

#define TELNETSRV_SERVER_STACK_SIZE (1500)
#define TELNETSRV_SESSION_STACK_SIZE (2000)

#if TELNETSRVCFG_USE_WELCOME_STRINGS
    #define TELNETRSV_WELCOME_STRING "RTCS v%d.%02d.%02d Telnet server\n"
    #define TELNETSRV_GOODBYE_STRING "Goodbye\n"
#else
    #define TELNETRSV_WELCOME_STRING ""
    #define TELNETSRV_GOODBYE_STRING ""
#endif

/*
 * Parameters for server task
 */
typedef struct telnetsrv_server_task_param
{
    uint32_t               handle;    /* [out] Server handle, non-zero if initialization was successful */
    TELNETSRV_PARAM_STRUCT *params;   /* [in] Server parameters */
}TELNETSRV_SERVER_TASK_PARAM;

/*
 * Session process function prototype
 */
typedef void(*TELNETSRV_SES_FUNC)(void *server, void *session);

/*
* Telnet session structure
*/
typedef struct telnetsrv_session_struct
{
    volatile bool      valid;       /* non zero (1) = session is valid - data in this entry is valid */
    volatile _mqx_int  sock;        /* Session socket */
    volatile uint32_t  time;        /* Session time. Updated when there is some activity in session. Used for timeout detection. */
    uint32_t           timeout;     /* Session timeout in ms. timeout_time = time + timeout */
    #if !MQX_USE_IO_OLD
    FILE               *sockfd;     /* Sockio socket file descriptor */
    FILE               *telnetfd;   /* telnetio file descriptor */
    #else
    MQX_FILE_PTR       sockfd;      /* Sockio socket file descriptor */
    MQX_FILE_PTR       telnetfd;    /* telnetio file descriptor */
    #endif
    TELNETSRV_SES_FUNC process_func; /* Session process function */
} TELNETSRV_SESSION_STRUCT;

/*
** Telnet server main structure.
*/
typedef struct telnetsrv_struct
{
    TELNETSRV_PARAM_STRUCT   params;          /* server parameters */
    uint32_t                 sock_v4;         /* listening socket for IPv4 */
    uint32_t                 sock_v6;         /* listening socket for IPv6 */
    TELNETSRV_SESSION_STRUCT **session;       /* array of pointers to sessions */
    volatile _task_id        server_tid;      /* Server task ID */
    volatile _task_id        *ses_tid;        /* Session task IDs */
    LWSEM_STRUCT             tid_sem;         /* Semaphore for session TID array locking */
    LWSEM_STRUCT             ses_cnt;         /* Session counter */
} TELNETSRV_STRUCT;

/*
** Parameter for session task
*/
typedef struct telnetsrv_ses_task_param
{
    TELNETSRV_STRUCT *server; /* Pointer to server structure */
    uint32_t         sock;    /* Socket to be used by session */
} TELNETSRV_SES_TASK_PARAM;

#ifdef __cplusplus
extern "C" {
#endif

void telnetsrv_server_task(void *init_ptr, void *creator);
TELNETSRV_STRUCT* telnetsrv_create_server(TELNETSRV_PARAM_STRUCT* params);
int32_t telnetsrv_destroy_server(TELNETSRV_STRUCT* server);
uint32_t telnetsrv_wait_for_conn(TELNETSRV_STRUCT *server);
uint32_t telnetsrv_accept(uint32_t sock);
void telnetsrv_abort(uint32_t sock);

#ifdef __cplusplus
}
#endif

#endif
