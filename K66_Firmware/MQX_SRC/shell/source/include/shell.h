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
*   This file contains the shell API.
*
*
*END************************************************************************/

#ifndef __shell_h__
#define __shell_h__

#if MQX_USE_IO_OLD
#include <fio.h>
#else
#include <stdio.h>
#endif

#ifdef __rtcs_h__
#include <sh_rtcs.h>
#endif

#ifdef __mfs_h__
#include <sh_mfs.h>
#endif

/** MGCT: <category name="Shell Settings"> */

/*
** MGCT: <option type="bool"/>
*/
#ifndef SHELLCFG_USES_MFS
#define SHELLCFG_USES_MFS   1
#endif

/*
** MGCT: <option type="bool"/>
*/
#ifndef SHELLCFG_USES_RTCS
#define SHELLCFG_USES_RTCS  1
#endif

/** MGCT: </category> */

#ifdef BSP_DEFAULT_ENET_DEVICE
#include <sh_enet.h>
#endif

#define SHELL_MAX_ARGS       16

#define SHELL_EXIT_SUCCESS   0
#define SHELL_EXIT_ERROR    -1

#define SHELL_MAX_DEVLEN    8
#define SHELL_MAX_FILELEN   256
#define SHELL_PATHNAME_LEN  260
#define SHELL_BLOCK_SIZE    128

#define MEM_TYPE_SHELL_BASE                   ( (IO_SHELL_COMPONENT) << (MEM_TYPE_COMPONENT_SHIFT))
#define MEM_TYPE_SHELL_CONTEXT                (MEM_TYPE_SHELL_BASE+1)

#define hexnum(c)  ((c<='9')?(c-'0'):((c<='F')?(c-'A'+10) :(c-'a'+10)))

#define SHELL_COMMAND(cmd) { #cmd, Shell_##cmd },

typedef struct shell_command_struct  {
   char  *COMMAND;
   int32_t      (*SHELL_FUNC)(int32_t argc, char *argv[]);
} SHELL_COMMAND_STRUCT, * SHELL_COMMAND_PTR;

typedef struct shell_getopt_context
{
    int   opterr;
    int   optind;
    int   optopt;
    int   sp;
    char  *optarg;
}SHELL_GETOPT_CONTEXT;

/*
** Function prototypes
*/

#ifdef __cplusplus
extern "C" {
#endif

extern MQX_FILE_PTR Shell_default_fs;
extern char *Shell_default_fs_name;
extern const SHELL_COMMAND_STRUCT Shell_commands[];

extern int32_t  Shell( const SHELL_COMMAND_STRUCT[], char *);
void Shell_getopt_init(SHELL_GETOPT_CONTEXT* ctx);
extern int32_t  Shell_getopt(int argc, char** argv, char* opts, SHELL_GETOPT_CONTEXT* ctx);
extern int32_t  Shell_parse_command_line( char *command_line_ptr, char *argv[] );
extern bool Shell_parse_number( char *arg, uint32_t *num_ptr);
extern bool Shell_check_help_request(int32_t argc, char *argv[], bool  *short_ptr );
extern bool Shell_parse_uint_32( char *arg, uint32_t *num_ptr);
extern bool Shell_parse_uint_16( char *arg, uint16_t *num_ptr);
extern bool Shell_parse_int_32( char *arg, int32_t *num_ptr);
extern bool Shell_parse_hexnum( char *arg, uint32_t *num_ptr);
extern void Shell_create_prefixed_filename( char *new_ptr, char *in_ptr,  void *argv);
extern int32_t Shell_abort(int32_t argc, char *argv[] );
extern int32_t Shell_command_list(int32_t argc, char *argv[] );
extern int32_t Shell_exit(int32_t argc, char *argv[] );
extern int32_t Shell_help(int32_t argc, char *argv[] );
extern int32_t Shell_kill(int32_t argc, char *argv[] );
extern int32_t Shell_pause(int32_t argc, char *argv[] );
extern int32_t Shell_tad(int32_t argc, char *argv[] );
extern int32_t Shell_sh(int32_t argc, char *argv[] );
extern int32_t Shell_md(int32_t argc, char *argv[] );

extern MQX_FILE_PTR Shell_get_current_filesystem(void *argv);
extern char *Shell_get_current_filesystem_name(void *argv);

extern int32_t Shell_set_current_filesystem(void *argv, MQX_FILE_PTR fp);
extern int32_t Shell_set_current_filesystem_by_name(void *argv, char *dev_name);


extern int32_t Shell_erase(int32_t argc, char *argv[] );
extern int32_t Shell_write_block(int32_t argc, char *argv[] );
extern int32_t Shell_read_block(int32_t argc, char *argv[] );

#ifdef __cplusplus
}
#endif

#endif
/*EOF */
