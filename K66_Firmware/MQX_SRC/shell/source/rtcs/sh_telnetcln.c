/*HEADER**********************************************************************
*
* Copyright 2008-2014 Freescale Semiconductor, Inc.
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
*   This file contains the shell command for invoking telnet client.
*
*
*END************************************************************************/

#include <ctype.h>
#include <string.h>
#include <mqx.h>
#include "shell.h"
#if SHELLCFG_USES_RTCS

#include <rtcs.h>
#include <telnetcln.h>
#include "sh_rtcs.h"
#include <stdlib.h>

#define SHELL_TELNETCLN_DEFAULT_PORT "23"

static void Shell_telnetcln_print_usage(char *name, bool shorthelp);
static void Shell_telnetcln_on_connected(void *param);
static void Shell_telnetcln_on_disconnected(void *param);

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  Shell_telnet
*  Returned Value:  none
*  Comments  :  SHELL utility to telnet to a host
*
*END*-----------------------------------------------------------------*/

int32_t  Shell_telnetcln(int32_t argc, char *argv[])
{
    bool                   shorthelp = FALSE;
    int32_t                return_code = SHELL_EXIT_SUCCESS;
    char                   *optstring = ":h:p:f:";
    SHELL_GETOPT_CONTEXT   gopt_context;
    int32_t                next_option;
    char                   *host;
    char                   *port;
    struct addrinfo        hints = {0};
    struct addrinfo        *result;
    int                    error;
    uint32_t               handle;

    if (Shell_check_help_request(argc, argv, &shorthelp))
    {
        Shell_telnetcln_print_usage(argv[0], shorthelp);
        return(return_code);
    }

    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    port = SHELL_TELNETCLN_DEFAULT_PORT;
    host = NULL;

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
                port = gopt_context.optarg;
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
                Shell_telnetcln_print_usage(argv[0], TRUE);
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
        Shell_telnetcln_print_usage(argv[0], TRUE);
        return(SHELL_EXIT_ERROR);
    }

    error = getaddrinfo(host, port, &hints, &result);
    if (error == 0)
    {
        TELNETCLN_PARAM_STRUCT params = {0};
        LWSEM_STRUCT           sema;

        _lwsem_create(&sema, 1);
        params.sa_remote_host = *result->ai_addr;
        params.fd_in = stdin;
        params.fd_out = stdout;
        params.callbacks.on_connected = Shell_telnetcln_on_connected;
        params.callbacks.on_disconnected = Shell_telnetcln_on_disconnected;
        params.callbacks.param = &sema;
        freeaddrinfo(result);
        printf("Connecting to %s, port %s...\n", host, port);

        handle = TELNETCLN_connect(&params);
        if (handle == 0)
        {
            printf("Connection failed\n");
            return_code = SHELL_EXIT_ERROR;
        }
        else
        {
            _lwsem_wait(&sema);
            printf("Connection closed\n");
        }
        _lwsem_destroy(&sema);
    }
    else
    {
        printf("Failed to resolve %s:%s\n", host, port);
    }

    return(return_code);
}

static void Shell_telnetcln_print_usage(char* name, bool shorthelp)
{
    if (!shorthelp)
    {
        puts("Usage:");
    }
    printf("%s -h <host> [-p <port>] [-f <family>]\n" , name);
    if (!shorthelp)
    {
        puts("    <host>   = remote host IP address or hostname\n"
             "    <port>   = remote port\n"
             "    <family> = preferred address family for connection\n");
    }
}

static void Shell_telnetcln_on_connected(void *param)
{
    LWSEM_STRUCT *sema;

    sema = (LWSEM_STRUCT *) param;
    _lwsem_wait(sema);
    puts("Telnet client connected.\n");
}

static void Shell_telnetcln_on_disconnected(void *param)
{
    LWSEM_STRUCT *sema;

    sema = (LWSEM_STRUCT *) param;
    _lwsem_post(sema);
    puts("Telnet client disconnected.\n");
}

#endif /* SHELLCFG_USES_RTCS */
