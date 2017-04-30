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

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <mqx.h>
#include "shell.h"
#if SHELLCFG_USES_RTCS

#include <rtcs.h>
#include "tftpcln.h"
#include "sh_rtcs.h"

#if !MQX_USE_IO_OLD
    #include <stdio.h>
    #define MQX_FILE_PTR FILE *
#else
    #include <fio.h>
#endif

#define SHELL_TFTPCLN_DEFAULT_PORT "69"

static void sh_tftpcln_print_usage(char* name, bool shorthelp);
static void sh_tftpcln_callback (uint32_t data_length);
static void sh_tftpcln_error(uint16_t error_code, char* error_string);

/*
 * Download/Upload file from/to TFTP server.
 */
int32_t Shell_tftpcln(int32_t argc, char *argv[])
{
    bool                   shorthelp = FALSE;
    int32_t                return_code = SHELL_EXIT_SUCCESS;
    TFTPCLN_PARAM_STRUCT   params = {0};
    char                   *optstring = ":h:pm:r:l:f:";
    SHELL_GETOPT_CONTEXT   gopt_context;
    int32_t                next_option;
    char                   *host;
    char                   *port;
    char                   *remote_filename;
    char                   *local_filename;
    char                   *mode;
    struct addrinfo        hints = {0};
    struct addrinfo        *getadd_result;
    int                    error;
    uint32_t               handle = 0;
    int32_t                result = 0;
    char*                  result_string;

    if (Shell_check_help_request(argc, argv, &shorthelp))
    {
        sh_tftpcln_print_usage(argv[0], shorthelp);
        return(return_code);
    }

    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    port = SHELL_TFTPCLN_DEFAULT_PORT;
    host = NULL;
    mode = NULL;
    remote_filename = NULL;
    local_filename = NULL;
    params.recv_callback = sh_tftpcln_callback;
    params.send_callback = sh_tftpcln_callback;
    params.error_callback = sh_tftpcln_error;

    /* Parse command line options */
    Shell_getopt_init(&gopt_context);
    do
    {
        next_option = Shell_getopt(argc, argv, optstring, &gopt_context);
        switch (next_option)
        {
            case 'h':
                host = gopt_context.optarg;
                break;
            case 'p':
                if(strtoul(gopt_context.optarg, NULL, 10) != 0)
                {
                    port = gopt_context.optarg;
                }
                break;
            case 'm':
                mode = gopt_context.optarg;
                break;
            case 'r':
                remote_filename = gopt_context.optarg;
                break;
            case 'l':
                local_filename = gopt_context.optarg;
                break;
            case 'f':
                if(strtoul(gopt_context.optarg, NULL, 10) == 4)
                {
                    hints.ai_family = AF_INET;
                }
                else if(strtoul(gopt_context.optarg, NULL, 10) == 6)
                {
                    hints.ai_family = AF_INET6;
                }
                break;
            case '?': /* User specified an invalid option. */
                printf("Unknown option -%c.\n", next_option);
                sh_tftpcln_print_usage(argv[0], TRUE);
                return_code = SHELL_EXIT_ERROR;
                next_option = -1;
                break;
            case ':': /* Option has a missing parameter. */
                printf("Option -%c requires a parameter.\n", next_option);
                return_code = SHELL_EXIT_ERROR;
                next_option = -1;
                break;
            case -1: /* Done with options. */
                break;
            default:
                break;
        }
    }while(next_option != -1);
    
    if (return_code == SHELL_EXIT_ERROR)
    {
        return(return_code);
    }

    /* check if we have all required parameters. */
    if (host == NULL)
    {
        fputs("Host not specified!\n", stdout);
        sh_tftpcln_print_usage(argv[0], TRUE);
        return(SHELL_EXIT_ERROR);
    }

    error = getaddrinfo(host, port, &hints, &getadd_result);
    if (error == 0)
    {
        params.sa_remote_host = *getadd_result->ai_addr;

        freeaddrinfo(getadd_result);
        fprintf(stdout, "Connecting to %s, port %s...", host, port);

        handle = TFTPCLN_connect(&params);
        if (handle == 0)
        {
            fputs("FAIL\n", stdout);
            return(SHELL_EXIT_ERROR);
        }
        else
        {
            fputs("OK\n", stdout);
        }
    }
    else
    {
        printf("Failed to resolve %s:%s\n", host, port);
    }

    if (!strcmp(mode, "PUT"))
    {
        if (local_filename == NULL)
        {
            fputs("Local file name not specified!\n", stdout);
            sh_tftpcln_print_usage(argv[0], TRUE);
            return(SHELL_EXIT_ERROR);
        }
        fprintf(stdout, "Uploading local file %s to %s:", local_filename, remote_filename);
        result = TFTPCLN_put(handle, local_filename, remote_filename);
    }
    else if (!strcmp(mode, "GET"))
    {
        if (remote_filename == NULL)
        {
            fputs("Remote file name not specified!\n", stdout);
            sh_tftpcln_print_usage(argv[0], TRUE);
            return(SHELL_EXIT_ERROR);
        }
        fprintf(stdout, "Downloading remote file %s to %s:", remote_filename, local_filename);
        result = TFTPCLN_get(handle, local_filename, remote_filename);
    }
    if (result == RTCS_OK)
    {
        result_string = "successful";
    }
    else
    {
        result_string = "failed";
    }
    fprintf(stdout, "Transaction %s.\n", result_string);
    TFTPCLN_disconnect(handle);
    return(return_code);
}

static void sh_tftpcln_print_usage(char* name, bool shorthelp)
{
    if (!shorthelp)
    {
        puts("Usage:");
    }
    printf("%s -h <host> [-p <port>] [-f <family>] -m <operation_mode> [-r <remote_filename>] [-l <local_filename>]\n" , name);
    if (!shorthelp)
    {
        puts("    <host>   = remote host IP address or hostname\n"
             "    <port>   = remote port\n"
             "    <family> = preferred address family for connection\n"
             "    <operation mode> = 'GET' for reading or 'PUT' writing file from/to server\n"
             "    <remote_filename> = File to read from/write to on remote host. Mandatory in GET mode\n"
             "    <local_filename>  = File to read from/write to on local host. Mandatory in PUT mode\n");
    }
}

static void sh_tftpcln_callback(uint32_t data_length)
{
    fputc('.', stdout);
    if (data_length < TFTPCLN_MAX_DATA_SIZE)
    {
        fputs("DONE\n", stdout);
    }
}

static void sh_tftpcln_error(uint16_t error_code, char* error_string)
{
    fprintf(stdout, "FAIL\nError: %s\n", error_string);
}

#endif /* SHELLCFG_USES_RTCS */
