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
#include <mqx.h>
#include "shell.h"
#if SHELLCFG_USES_RTCS
#include <rtcs.h>
#include "sh_rtcs.h"
#include "telnetsrv.h"

uint32_t telnetsrv_handle;

extern const SHELL_COMMAND_STRUCT Telnetsrv_shell_commands[];

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  Shell_telnetsrv
*  Returned Value:  none
*
*END*-----------------------------------------------------------------*/

int32_t Shell_telnetsrv(int32_t argc, char *argv[])
{
    uint32_t  result;
    bool      print_usage;
    bool      shorthelp = FALSE;
    bool      print_status = FALSE;
    int32_t   return_code = SHELL_EXIT_SUCCESS;
    char      *status_string = NULL;

    print_usage = Shell_check_help_request(argc, argv, &shorthelp );

    if (print_usage)
    {
        if (shorthelp)
        {
            printf("%s [start|stop]\n", argv[0]);
        }
        else
        {
            printf("Usage: %s [start|stop]\n",argv[0]);
        }
        goto EXIT;
    }
    
    if (argc != 2)
    {
        printf("Error, %s invoked with incorrect number of arguments.\n", argv[0]);
        print_usage = TRUE;
        goto EXIT;
    }

    if (strcmp(argv[1], "start") == 0)
    {
        TELNETSRV_PARAM_STRUCT params = {0};

        params.shell_commands = (void *) Telnetsrv_shell_commands;
        params.shell = (TELNET_SHELL_FUNCTION) Shell;
        if (telnetsrv_handle == 0)
        {
            telnetsrv_handle = TELNETSRV_init(&params);
            status_string = (telnetsrv_handle == 0) ? "failed" : "successful";
        }
        else
        {
            status_string = "aborted. Telnet server already running";
        }
        print_status = TRUE;
    }
    else if (strcmp(argv[1], "stop") == 0)
    {
        if (telnetsrv_handle != 0)
        {
            result = TELNETSRV_release(telnetsrv_handle);
            status_string = (result == RTCS_OK) ? "successful" : "failed";
            telnetsrv_handle = 0;
        }
        else
        {
            status_string = "aborted. Telnet server is not running";
        }
        print_status = TRUE;
    }
    else
    {
        printf("Error, %s invoked with incorrect option.\n", argv[0]);
        print_usage = TRUE;
    }

    if (print_status)
    {
        printf("Telnet server %s %s.\n", argv[1], status_string);
    }

    EXIT:
    return return_code;
}

#endif /* SHELLCFG_USES_RTCS */
