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
#include "tftpsrv.h"

static uint32_t tftpsrv_handle;

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  shell_tftpsrv
*  Returned Value:  none
*  Comments  :  Shell function to start TFTP server.
*
*END*-----------------------------------------------------------------*/

int32_t Shell_tftpsrv(int32_t argc, char *argv[])
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
        goto EXIT;
    }

    if (strcmp(argv[1], "start") == 0)
    {
        TFTPSRV_PARAM_STRUCT params = {0};
        
        params.root_dir = "a:";
        if (tftpsrv_handle == 0)
        {
            tftpsrv_handle = TFTPSRV_init(&params);
            status_string = (tftpsrv_handle == 0) ? "failed" : "successful";
        }
        else
        {
            status_string = "aborted. TFTP server already running";
        }
        print_status = TRUE;
    }
    else if (strcmp(argv[1], "stop") == 0)
    {
        if (tftpsrv_handle != 0)
        {
            result = TFTPSRV_release(tftpsrv_handle);
            status_string = (result == RTCS_OK) ? "successful" : "failed";
            tftpsrv_handle = 0;
        }
        else
        {
            status_string = "aborted. TFTP server is not running";
        }
        print_status = TRUE;
    }
    else
    {
        printf("Error, %s invoked with incorrect option.\n", argv[0]);
    }

    if (print_status)
    {
        printf("TFTP server %s %s.\n", argv[1], status_string);
    }

    EXIT:
    return return_code;
}
#endif /* SHELLCFG_USES_RTCS */
