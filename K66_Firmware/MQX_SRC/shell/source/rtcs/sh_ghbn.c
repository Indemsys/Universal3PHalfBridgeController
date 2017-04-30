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


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  Shell_get_host_by_name
*  Returned Value:  none
*  Comments  :  SHELL utility to resolve a host name
*
*END*-----------------------------------------------------------------*/

int32_t  Shell_get_host_by_name(int32_t argc, char *argv[] )
{
    bool            print_usage, shorthelp = FALSE;
    int32_t         return_code = SHELL_EXIT_SUCCESS;

    print_usage = Shell_check_help_request(argc, argv, &shorthelp );

    if (!print_usage)
    {
        if (argc == 2)
        {
            struct addrinfo     addrinfo_hints;
            struct addrinfo     *addrinfo_result;
            struct addrinfo     *addrinfo_result_first;
            int32_t             retval = 0;
            char                addr_str[RTCS_IP6_ADDR_STR_SIZE];
            
            _mem_zero(&addrinfo_hints, sizeof(addrinfo_hints));
            addrinfo_hints.ai_flags = AI_CANONNAME;

            retval = getaddrinfo(argv[1], NULL, &addrinfo_hints, &addrinfo_result);
            if (retval == 0) /* OK */
            {
                addrinfo_result_first = addrinfo_result;
                /* Print all resolved IP addresses.*/
                while(addrinfo_result)
                {
                    if(inet_ntop(addrinfo_result->ai_family, 
                                &RTCS_SOCKADDR_ADDR(addrinfo_result->ai_addr), 
                                addr_str, sizeof(addr_str)) != 0)
                    {
                        printf("%s\t%s\n", addrinfo_result->ai_canonname, addr_str);
                    }
                    addrinfo_result = addrinfo_result->ai_next;
                }

                freeaddrinfo(addrinfo_result_first);
            }
            else
            {
                printf("Unable to resolve host\n");
                return_code = SHELL_EXIT_ERROR;
            }
        }
        else
        {
            printf("Error, %s invoked with incorrect number of arguments\n", argv[0]);
            return_code = SHELL_EXIT_ERROR;
            print_usage = TRUE;
        }
    }

    if (print_usage)
    {
        if (shorthelp)
        {
            printf("%s <host>\n", argv[0]);
        }
        else
        {
            printf("Usage: %s <host>\n", argv[0]);
            printf("   <host>   = host ip address or name\n");
        }
    }

    return return_code;
}

#endif /* SHELLCFG_USES_RTCS */

