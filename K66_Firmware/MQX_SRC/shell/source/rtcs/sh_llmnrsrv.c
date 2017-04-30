/*HEADER**********************************************************************
*
* Copyright 2008, 2014 Freescale Semiconductor, Inc.
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
#ifdef BSP_ENET_DEVICE_COUNT 
#if (BSP_ENET_DEVICE_COUNT > 0)

#define SHELL_LLMNRSRV_PRIO        (RTCSCFG_DEFAULT_RTCSTASK_PRIO+1)

#if RTCSCFG_ENABLE_IP6
    #if RTCSCFG_ENABLE_IP4
        #define SHELL_LLMNRSRV_AF (AF_INET | AF_INET6)
    #else
        #define SHELL_LLMNRSRV_AF (AF_INET6)
    #endif
#else
    #define SHELL_LLMNRSRV_AF (AF_INET)
#endif

#define SH_LLMNRSRV_HOST_NAME               "rtcs"
#define SH_LLMNRSRV_HOST_NAME_DELIMITER     ';'    

static LLMNRSRV_HOST_NAME_STRUCT *shell_llmnrsrv_str_to_host_name_table(char *host_name_list);

static uint32_t sh_llmnr_desc[BSP_ENET_DEVICE_COUNT]={0};

/************************************************************************
* NAME: shell_llmnrsrv_str_to_host_name_table
* RETURNS: none
* DESCRIPTION: Allocate host name table and init according to 
*              host name string <host_name>;<host_name>
*************************************************************************/
static LLMNRSRV_HOST_NAME_STRUCT *shell_llmnrsrv_str_to_host_name_table(char *host_name_list)
{
    uint32_t                    host_name_list_len = strlen(host_name_list);
    LLMNRSRV_HOST_NAME_STRUCT   *host_name_table = NULL;
    uint32_t                    host_name_table_size =1;
    char*                       delimiter_p;

    if(host_name_list_len)
    {
        /* Calculater number of host names.*/
        delimiter_p = host_name_list;
        while( (delimiter_p = strchr(delimiter_p, SH_LLMNRSRV_HOST_NAME_DELIMITER)) != NULL)
        {
            delimiter_p++; /* Skip delimeter symbol.*/
            host_name_table_size++;
        }
        
        /* Allocate table and host names. */
        host_name_table = _mem_alloc_system_zero((sizeof(LLMNRSRV_HOST_NAME_STRUCT) * (host_name_table_size+1/*emty structure*/)) + (host_name_list_len +1/*zero*/) );

        /* Init table*/
        if(host_name_table)
        {
            int                         i;
            char                        *host_name;
            char                        *delimiter;
            
            /* Copy host name.*/
            host_name = memcpy(&host_name_table[host_name_table_size+1/*emty structure*/], host_name_list, host_name_list_len);
            
            /* Init first host_name_table_size-1 entries.*/

            for(i=0; i < (host_name_table_size-1/* except last entry*/); i++)
            {
                host_name_table[i].host_name = host_name;
                                
                delimiter = strchr(host_name, SH_LLMNRSRV_HOST_NAME_DELIMITER);
                *delimiter = '\0';

                host_name = delimiter+1; 
            }
            /* Init last entry */
            host_name_table[i].host_name = host_name;
        }
    }

    return host_name_table;
}

/************************************************************************
* NAME: Shell_llmnrsrv
* RETURNS: none
* DESCRIPTION: LLMNR server
*************************************************************************/
int32_t  Shell_llmnrsrv(int32_t argc, char *argv[] )
{
    _rtcs_if_handle         ihandle;
    bool                    print_usage, shorthelp = FALSE;
    int32_t                 return_code = SHELL_EXIT_SUCCESS;
    uint32_t                enet_device = BSP_DEFAULT_ENET_DEVICE;
    uint32_t                index = 1;
    LLMNRSRV_PARAM_STRUCT   llmnr_params;

    char                    *host_name = NULL;
    SHELL_GETOPT_CONTEXT    gopt_context;
    int32_t                 next_option;

    print_usage = Shell_check_help_request (argc, argv, &shorthelp);

    if (!print_usage)
    {
        if (argc > index)
        {
            /* Parse [<device>] */
            if (Shell_parse_number (argv[index], &enet_device))
            {
                index++;
            }
        }

        if (enet_device >= BSP_ENET_DEVICE_COUNT)
        {
            printf ("Wrong number of ethernet device (%d)!\n", enet_device);
            return_code = SHELL_EXIT_ERROR;
        }
        else
        {
            ihandle = ipcfg_get_ihandle (enet_device);
            if (ihandle == NULL)
            {
                printf("Initialize Device Using ipconfig\n");
                return_code = SHELL_EXIT_ERROR;
            }   
            else
            {
                /* Parse command line options [-h <host name>]*/
                Shell_getopt_init(&gopt_context);
                do
                {
                    next_option = Shell_getopt(argc, argv, "h:", &gopt_context);
                    switch (next_option)
                    {
                        case 'h':
                            host_name = gopt_context.optarg;
                            index+=2;
                            break;
                        case '?': /* User specified an invalid option. */
                            goto INCORRECT_OPTION;
                        case ':': /* Option has a missing parameter. */
                            printf("Option -%c requires a parameter.\n", next_option);
                            return(SHELL_EXIT_ERROR);
                        case -1: /* Done with options. */
                            break;
                        default:
                            break;
                    }
                }
                while(next_option != -1);


                if (argc == (index+1))
                { 
                    /* Parse start|stop */
                    if (strcmp(argv[index], "start") == 0)
                    {
                        if(0 == sh_llmnr_desc[enet_device])
                        {
                            uint32_t                    llmnr_desc;

                            _mem_zero(&llmnr_params, sizeof(llmnr_params));
                            llmnr_params.interface = ihandle; 
                            if(host_name == NULL)
                            {
                                host_name = SH_LLMNRSRV_HOST_NAME;
                            }
                            
                            /* Allocate host name table */
                            llmnr_params.host_name_table = shell_llmnrsrv_str_to_host_name_table(host_name);
                            if(llmnr_params.host_name_table)
                            {
                                /* Start LLMNR server.*/
                                llmnr_desc = LLMNRSRV_init(&llmnr_params);

                                if (llmnr_desc !=  0)
                                {
                                    printf("LLMNR Server Started. Host Name = %s\n", host_name);
                                    sh_llmnr_desc[enet_device] = llmnr_desc;
                                }
                                else
                                {
                                    printf("Unable to start LLMNR Server\n");
                                    return_code = SHELL_EXIT_ERROR;
                                }  
                           
                                _mem_free(llmnr_params.host_name_table);
                            }
                            else
                            {
                                printf ("No free memory!\n");
                                return_code = SHELL_EXIT_ERROR;
                            }
                        }
                        else 
                        {
                            printf("LLMNR[%d] Server already running\n", enet_device);
                        }
                    }
                    else if (strcmp(argv[index], "stop") == 0)
                    {
                        /* Stop LLMNR server.*/
                        LLMNRSRV_release(sh_llmnr_desc[enet_device]);
                        sh_llmnr_desc[enet_device] = 0;
                        printf("LLMNR Server Stopped.\n");
                    } 
                    else
                    {
    INCORRECT_OPTION:
                        printf("Error, %s invoked with incorrect option\n", argv[0]);
                        print_usage = TRUE;
                    }
                } 
                else
                {
                    printf("Error, %s invoked with incorrect number of arguments\n", argv[0]);
                    print_usage = TRUE;
                }
            }
        }
    }
   
    if (print_usage)
    {
        if (shorthelp)
        {
            printf("%s [<device>] [-h <host name>[;<host name>...]] start|stop\n", argv[0]);
        } 
        else
        {
            printf("Usage: %s [<device>] [-h <host name>[;<host name>...]] start|stop\n",argv[0]);
            printf("    <device>    = Ethernet device number (default %d).\n", BSP_DEFAULT_ENET_DEVICE);
            printf("    <host name> = Link-local host name advertised by LLMNR server (default %s).\n", SH_LLMNRSRV_HOST_NAME);
        }
    }
    return return_code;
}
#endif /* BSP_ENET_DEVICE_COUNT > 0 */
#endif /* BSP_ENET_DEVICE_COUNT */

#endif /* SHELLCFG_USES_RTCS */

/* EOF */
