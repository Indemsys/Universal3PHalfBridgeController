/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
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
*   This file contains the RTCS shell.
*
*
*END************************************************************************/

#define FTPC_BUF_SIZE      1460
#define FTPC_LINELEN       128
#define FTPC_MAX_ARGS      8

//#define FTPd_send_msg(p,m) send(p->CONTROL_SOCK, m, strlen(m), 0)
void FTPd_send_msg(void *p, const char * m);


typedef struct ftpc_context_struct  {
   char             *ARGV[FTPC_MAX_ARGS];
   int32_t               ARGC;
   char                 CMD_LINE[FTPC_LINELEN];
   char                 HISTORY[FTPC_LINELEN];
   char                 BUFFER[FTPC_BUF_SIZE];
   
   _ip_address          HOSTADDR;
   char                 HOSTNAME[RTCS_SNAME_LEN];
   void                *HANDLE;
   
   bool              EXIT;
} FTPc_CONTEXT_STRUCT, * FTPc_CONTEXT_PTR;

typedef struct FTPc_command_struct  {
   char  *COMMAND;
   int32_t      (*FTPc_FUNC)(FTPc_CONTEXT_PTR context_ptr,int32_t argc, char *argv[]);
} FTPc_COMMAND_STRUCT, * FTPc_COMMAND_PTR; 


#ifdef __cplusplus
extern "C" {
#endif

extern const FTPc_COMMAND_STRUCT FTPc_commands[];


extern int32_t  FTPc_help(FTPc_CONTEXT_PTR context_ptr, int32_t argc, char *argv[] );
extern int32_t  FTPc_ascii(FTPc_CONTEXT_PTR context_ptr, int32_t argc, char *argv[] );
extern int32_t  FTPc_binary(FTPc_CONTEXT_PTR context_ptr, int32_t argc, char *argv[] );
extern int32_t  FTPc_bye(FTPc_CONTEXT_PTR context_ptr, int32_t argc, char *argv[] );
extern int32_t  FTPc_cd(FTPc_CONTEXT_PTR context_ptr, int32_t argc, char *argv[] );
extern int32_t  FTPc_close(FTPc_CONTEXT_PTR context_ptr, int32_t argc, char *argv[] );
extern int32_t  FTPc_delete(FTPc_CONTEXT_PTR context_ptr, int32_t argc, char *argv[] );
extern int32_t  FTPc_get(FTPc_CONTEXT_PTR context_ptr, int32_t argc, char *argv[] );
extern int32_t  FTPc_ls(FTPc_CONTEXT_PTR context_ptr, int32_t argc, char *argv[] );
extern int32_t  FTPc_mkdir(FTPc_CONTEXT_PTR context_ptr, int32_t argc, char *argv[] );
extern int32_t  FTPc_open(FTPc_CONTEXT_PTR context_ptr, int32_t argc, char *argv[] );
extern int32_t  FTPc_pass(FTPc_CONTEXT_PTR context_ptr, int32_t argc, char *argv[] );
extern int32_t  FTPc_put(FTPc_CONTEXT_PTR context_ptr, int32_t argc, char *argv[] );
extern int32_t  FTPc_pwd(FTPc_CONTEXT_PTR context_ptr, int32_t argc, char *argv[] );
extern int32_t  FTPc_remotehelp(FTPc_CONTEXT_PTR context_ptr, int32_t argc, char *argv[] );
extern int32_t  FTPc_rename(FTPc_CONTEXT_PTR context_ptr, int32_t argc, char *argv[] );
extern int32_t  FTPc_rmdir(FTPc_CONTEXT_PTR context_ptr, int32_t argc, char *argv[] );
extern int32_t  FTPc_user(FTPc_CONTEXT_PTR context_ptr, int32_t argc, char *argv[] );

extern _mqx_int FTP_client(char *dest);

#if !MQX_USE_IO_OLD
#define MQX_FILE_PTR FILE*
#endif

extern int32_t  FTP_open         (void **, _ip_address, MQX_FILE_PTR);
extern int32_t  FTP_command      (void *, char *, MQX_FILE_PTR);
extern int32_t  FTP_command_data (void *, char *, MQX_FILE_PTR, MQX_FILE_PTR, uint32_t);
extern int32_t  FTP_close        (void *, MQX_FILE_PTR);
extern int32_t  FTP_receive_message(void *, MQX_FILE_PTR);

#ifdef __cplusplus
}
#endif

/*EOF*/
