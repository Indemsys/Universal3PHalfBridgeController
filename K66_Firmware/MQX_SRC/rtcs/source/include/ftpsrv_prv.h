#ifndef __ftpsrv_prv_h__
#define __ftpsrv_prv_h__
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
*   This file contains the private FTP server definitions.
*
*
*END************************************************************************/

#include <ftpsrv.h>
#if !MQX_USE_IO_OLD
#include <stdio.h>
#endif

/* Size configuration */
#define FTPSRV_LINELEN            (128)   /* Length of command buffer */
#define FTPSRV_BUF_SIZE           (256)
#define FTPSRV_SERVER_STACK_SIZE  (1200)
#define FTPSRV_SESSION_STACK_SIZE (3000)
#define FTPSRV_PATH_APPENDIX      "*.*"

#define FTPSRV_CMD_SOCK_BUFFER_SIZE (512)

#define FTPSRV_PROT_IPV4    (1)
#define FTPSRV_PROT_IPV6    (2)
#define FTPSRV_PROT_INVALID (99)

#define FTPSRV_TRANSFER_TASK_PRIO FTPSRVCFG_DEF_SERVER_PRIO
#define FTPSRV_MODE_READ    (0x00000001)
#define FTPSRV_MODE_WRITE   (0x00000002)
#define FTPSRV_MODE_APPEND  (0x00000004)

#define FTPSRV_NUM_MESSAGES 2

#define FTPSRV_CMD_ABORT 5
#define FTPSRV_CMD_STAT 6

typedef enum ftpsrv_auth_state
{
    FTPSRV_LOGGED_OUT,
    FTPSRV_USER,
    FTPSRV_LOGGED_IN
} FTPSRV_AUTH_STATE;

typedef enum ftpsrv_ses_state
{
    FTPSRV_STATE_INIT,
    FTPSRV_STATE_LISTEN,
    FTPSRV_STATE_CONNECT,
    FTPSRV_STATE_BAD_PROT,
    FTPSRV_STATE_RNFR,
    FTPSRV_STATE_TRANSFER,
    FTPSRV_STATE_IDLE
}FTPSRV_SES_STATE;

/*!
 * \brief Script message structure.
 *
 * This message is used for communication between ftppsrv_session_task()
 * and ftpsrv_transfer_task(). Only 32 bits long variables must be added!
 */
typedef struct ftpsrv_transfer_msg
{
    /*! \brief Command for transfer task */
    uint32_t                command;
    /*! \brief Result of command */
    uint32_t                result;
}FTPSRV_TRANSFER_MSG;

typedef struct ftpsrv_session_struct 
{
    uint32_t            connected;
    FTPSRV_AUTH_STATE   auth_state;
    FTPSRV_AUTH_STRUCT* auth_tbl;
    FTPSRV_AUTH_STRUCT  auth_input;
#if MQX_USE_IO_OLD    
    MQX_FILE_PTR        fs_ptr;
#else
    int                 fs_ptr;
#endif
    volatile _task_id   transfer_tid;
    uint32_t            start_time;
    volatile uint32_t   control_sock;
    volatile uint32_t   data_sock;
    volatile uint32_t   state;
    uint32_t            passive;
    struct sockaddr     data_sockaddr;
    char*               message;
    char*               command;
    char*               cmd_arg;
    char*               cur_dir;
    char*               root_dir;
    char*               buffer;
    char*               rnfr_path;
    void*               msg_queue;
} FTPSRV_SESSION_STRUCT;

typedef struct ftpsrv_command_struct
{
    const char* command;
    int32_t     (*function)(FTPSRV_SESSION_STRUCT* session);
    bool        auth_req;
} FTPSRV_COMMAND_STRUCT;

/*
** ftp server main structure.
*/
typedef struct ftpsrv_struct
{
    FTPSRV_PARAM_STRUCT    params;       /* server parameters */
    /* 
    ** ------------------------------!!----------------------------------------
    ** Do not change order of following two variables (sock_v4 and sock_v6)
    ** sock_v6 MUST always follow sock_v4.
    ** ------------------------------!!----------------------------------------
    */
    uint32_t               sock_v4;         /* listening socket for IPv4 */
    uint32_t               sock_v6;         /* listening socket for IPv6 */
    volatile _mqx_int      run;             /* run flag */
    FTPSRV_SESSION_STRUCT  **session;       /* array of pointers to sessions */
    _task_id               server_tid;      /* Server task ID */
    _task_id*              ses_tid;         /* Session task IDs */
    LWSEM_STRUCT           tid_sem;         /* Semaphore for session TID array locking */
    LWSEM_STRUCT           ses_cnt;         /* Session counter */
} FTPSRV_STRUCT;

typedef struct ftpsrv_session_param
{
    uint32_t        sock;
    FTPSRV_STRUCT*  server;
} FTPSRV_SESSION_PARAM;

typedef struct ftpsrv_transfer_param
{
    uint32_t               sock;
#if MQX_USE_IO_OLD
    MQX_FILE_PTR           file;
#else
    FILE                   *file;
#endif
    uint32_t               mode;
    FTPSRV_SESSION_STRUCT* session;
}FTPSRV_TRANSFER_PARAM;

/*
 * Parameters for server task
 */
typedef struct ftpsrv_server_task_param
{
    volatile uint32_t    handle;    /* [out] Server handle, non-zero if initialization was successful */
    FTPSRV_PARAM_STRUCT  *params;    /* [in] Server parameters */
}FTPSRV_SERVER_TASK_PARAM;


#ifdef __cplusplus
extern "C" {
#endif

int32_t ftpsrv_abor(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_appe(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_cwd(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_cdup(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_dele(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_epsv(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_eprt(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_feat(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_help(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_list(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_mkdir(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_nlst(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_noop(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_opts(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_pass(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_pasv(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_port(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_pwd(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_quit(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_rmdir(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_retr(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_rnfr(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_rnto(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_site(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_size(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_stat(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_stor(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_syst(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_type(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_user(FTPSRV_SESSION_STRUCT* session);
int32_t ftpsrv_unrecognized(FTPSRV_SESSION_STRUCT* session);

void ftpsrv_server_task(void* init_ptr, void* creator);
void ftpsrv_transfer_task(void* init_ptr, void* creator);
void ftpsrv_send_msg(FTPSRV_SESSION_STRUCT* session, const char *message);
bool ftpsrv_process_auth(FTPSRV_SESSION_STRUCT* session, uint32_t auth_req);
bool ftpsrv_check_authtbl(FTPSRV_SESSION_STRUCT* session);
char* ftpsrv_get_relative_path(FTPSRV_SESSION_STRUCT* session, char* abs_path);
char* ftpsrv_get_full_path(FTPSRV_SESSION_STRUCT* session, char* path, uint32_t *wrong_path);
void ftpsrv_get_prots_str(char** str, uint16_t* family);
uint32_t ftpsrv_max_cmd_length (void);
uint32_t ftpsrv_open_data_connection(FTPSRV_SESSION_STRUCT* session);
uint16_t ftpsrv_open_passive_conn(FTPSRV_SESSION_STRUCT* session, uint32_t family);
FTPSRV_STRUCT * ftpsrv_create_server(FTPSRV_PARAM_STRUCT* params);
int32_t ftpsrv_destroy_server(FTPSRV_STRUCT *server);
uint32_t ftpsrv_wait_for_conn(FTPSRV_STRUCT *server);
uint32_t ftpsrv_accept(uint32_t sock);
void ftpsrv_abort(uint32_t sock);
#ifdef __cplusplus
}
#endif

#endif
