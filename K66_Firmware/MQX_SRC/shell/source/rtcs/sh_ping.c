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
*   This file contains the RTCS shell ping commnad.
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
#include "sh_rtcs.h"
#include <enet.h>

typedef struct shell_ping_parameter
{
	const char  *parameter;
   	int32_t          value;
} SHELL_PING_PARAMETER, * SHELL_PING_PARAMETER_PTR; 


static int32_t shell_ping_take_value(int32_t *value, char *param,char *message )	;

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  Shell_ping
*  Returned Value:  none
*  Comments  :  SHELL utility to ping a host.
*
*END*-----------------------------------------------------------------*/
int32_t  Shell_ping(int32_t argc, char *argv[] )
{
    uint32_t             i;
    uint32_t             x;
    uint16_t             pingid = 0;
    bool             print_usage;
    bool             shorthelp = FALSE;
    int32_t              return_code = SHELL_EXIT_SUCCESS;
    uint32_t             error_flag=0;
    int32_t              error=0;
    char                addr_str[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"];
    char                *ping_send_buffer=NULL;
    struct addrinfo     hints;          /* Used for getaddrinfo()*/
    struct addrinfo     *addrinfo_res;  /* Used for getaddrinfo()*/
    PING_PARAM_STRUCT   ping_params;

    /* ping default parameters. */
    SHELL_PING_PARAMETER sh_ping_params[] =
    {
        { "-n", 0	},  /* Resolve or not numeric IP addresses.*/ /* Ignored.*/
        { "-c", 4 	},  /* Amount of ping.*/
        { "-h", 64	},  /* Max hops for IPv6 , or TTL for IPv4.*/
        { "-i", 1000},  /* Wait interval milliseconds between sending each packet.*/
        { "-p", 0xff }, /* Byte pattern for data of paket.*/
        { "-s", 56	},  /* number of data bytes to be sent.*/
        { "-I", 0	}   /* Interface address.*/ /* Ignored.*/
    };
    #define SHELL_PING_PARAMETERS_NUMBER (sizeof(sh_ping_params)/sizeof(SHELL_PING_PARAMETER))


    print_usage = Shell_check_help_request(argc, argv, &shorthelp );

    if (!print_usage)
    {
        if (argc >= 2)
        {
            if (argc > 2)
            {
                i=1;
                while(i < argc)
                {/* Checking all input parameters.*/
                    x=0;
                    error_flag=0;   /*=0 - no error for unknown parameters.*/
                    while(x < SHELL_PING_PARAMETERS_NUMBER)
                    {/* Looking for valid parameters.*/
                        if((strcmp(argv[i],sh_ping_params[x].parameter))==0)
                        {/* If we found something.*/
                            switch (x)
                            {
                                case (0): 
                                    sh_ping_params[x].value = 1;
                                    printf("num output only\n");
                                    error_flag = 0;
                                    break; /* Ignored.*/
                                case (1):
                                    i++;
                                    error_flag = shell_ping_take_value(&(sh_ping_params[x].value), argv[i], "count");       /* Amount of ping.*/
                                    break;
                                case (2):
                                    i++;
                                    error_flag = shell_ping_take_value(&(sh_ping_params[x].value), argv[i], "hoplimit");    /* Something like TTL(time to live)*/
                                    break;
                                case (3):
                                    i++;
                                    error_flag = shell_ping_take_value(&(sh_ping_params[x].value), argv[i], "p_wait");      /* Time interval between two packets.*/
                                    sh_ping_params[x].value = sh_ping_params[x].value * 1000;
                                    break;
                                case (4):
                                    i++;
                                    error_flag = shell_ping_take_value(&(sh_ping_params[x].value), argv[i], "pattern");     /* What digit we will use for packets data.*/
                                    break;
                                case (5):
                                    i++;
                                    error_flag = shell_ping_take_value(&(sh_ping_params[x].value), argv[i], "packet size"); /* Size of packet.*/
                                    break;
                                case (6):
                                    i++;
                                    sh_ping_params[x].value = 0;
                                    error_flag = 0;
                                    break; /* Ignored.*/
                                default:
                                    error_flag=0;
                            }/* switch (x) */
                        }/* if((strcmp(argv[i],ping6_param[x].parameter))==0) */
                        x++;
                    }/* while(x < SHELL_PING_PARAMETERS_NUMBER) */
                    i++;
                }/* while(i<argc) */
            } /* if (argc > 2) */
           
            if(error_flag)
            {
                printf("parameters parse error\n");
                return(SHELL_EXIT_ERROR); /* We can return right here and do not need free freeaddrinfo(addrinfo_res)*/
            }
            

            /* Extract IP address and detect family, here we will get scope_id too. */
            memset(&hints,0,sizeof(hints));
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_DGRAM;
            if (getaddrinfo(argv[argc-1], NULL, &hints, &addrinfo_res) != 0)
            {
                printf("GETADDRINFO error\n");
                return(SHELL_EXIT_ERROR); /* We can return right here and do not need free freeaddrinfo(addrinfo_res)*/
            }

            /* Prepare ping */
            if(inet_ntop(addrinfo_res->ai_family, &((struct sockaddr_in6 *)((*addrinfo_res).ai_addr))->sin6_addr, addr_str, sizeof(addr_str)))
			{
                /* Allocate data buffer for ping. */
                ping_send_buffer = RTCS_mem_alloc(sh_ping_params[5].value); /* ping6_param[5].value - value of bytes in packet */
                if(ping_send_buffer)
                {
                    /* Set value of bytes of data buffer in according with patterns
                     * ping6_param[4].value - patterns.
                     * ping6_param[5].value - size of buffer.
                     */
                    memset(ping_send_buffer, sh_ping_params[4].value, sh_ping_params[5].value);
                }
                else
                {
                    printf("Ping error - can not allocate ping buffer. Size is too big.\n");
                    return_code = SHELL_EXIT_ERROR;
                    goto EXIT_1;
                }
			}
			else
			{
		  	    printf("Address parameter is wrong.\n");
                return_code = SHELL_EXIT_ERROR;
                goto EXIT_1;
			}
            
            printf("Pinging %s:\n", addr_str);
            printf("Press [q] to cancel.\n");

            /* Start of sending of packets. */
            for (i=0; i< sh_ping_params[1].value; i++) /* Amount of how much packets we will send.*/
            {

                if (fstatus(stdin))
                { 
                    unsigned char c = fgetc(stdin); 
                    if (tolower(c)=='q')
                    { 
                        printf("\n"); 
                        goto EXIT_1; 
                    } 
                } 
                
                /* Set ping parameters.*/
                _mem_zero(&ping_params, sizeof(ping_params));  
                ping_params.addr = *addrinfo_res->ai_addr;
                ping_params.timeout = sh_ping_params[3].value; /* Maximum waiting time for answer */
                ping_params.id = ++pingid;
                ping_params.hop_limit = (uint8_t)sh_ping_params[2].value;          
                ping_params.data_buffer = ping_send_buffer;             
                ping_params.data_buffer_size = sh_ping_params[5].value;   
               
                /* Send ping.*/
                error = RTCS_ping(&ping_params);

                if (error)
                {
                    if (error == RTCSERR_ICMP_ECHO_TIMEOUT)
                    {
                        printf("Request timed out\n");
                    }
                    else
                    {
                    #if BSP_ENET_DEVICE_COUNT > 0
                        printf("Error 0x%04lX - %s\n", error,  ENET_strerror(error));
                    #else
                        printf("Error 0x%04lX \n", error);
                    #endif
                    }
                }
                else
                {
                    inet_ntop(ping_params.addr.sa_family, &((struct sockaddr_in *)(&ping_params.addr))->sin_addr.s_addr, addr_str, sizeof(addr_str));

                    if(ping_params.round_trip_time < 1)
                    {
                        printf("Reply from [%s]: time<1ms\n", addr_str);
                    }
                    else
                    {
                        printf("Reply from [%s]: time=%ldms\n", addr_str, ping_params.round_trip_time);
                    }
                }
                
                /* Possible timeout*/
                if ((ping_params.round_trip_time < sh_ping_params[3].value) && (i<(sh_ping_params[1].value-1)))
                {
                    RTCS_time_delay(sh_ping_params[3].value - ping_params.round_trip_time); /* ping6_param[3].value - time interval between two packets*/
                }
            } 

            error = _mem_free(ping_send_buffer);
            if( error != MQX_OK)
            {
                printf("Ping buffer mem free error - %d \n", error);
                return_code = SHELL_EXIT_ERROR;
                goto EXIT_1;
            }
        }/* if (argc >= 2) */
        else
        {
            printf("Error, %s invoked with incorrect number of arguments\n", argv[0]);
            print_usage = TRUE;
        }
    }

    if (print_usage)
    {
        if (shorthelp)
        {
            printf("%s [-c count] [-h hoplimit] [-i interval] [-p pattern] [-s size] <host>\n", argv[0]);
        }
        else
        {
            printf("Usage: %s [-c count][-h hoplimit][-i wait][-p pattern][-s size] <host>\n", argv[0]);
        #if 0 /* TBD */
            printf("   -n numeric output only. (IPv6) \n");
        #endif
            printf("   -c Stop after sending count packets.\n");
            printf("   -h Set IPv6 hoplimit, or IPv4 TTL.\n");
            printf("   -i Wait interval seconds between sending each packet.\n");
            printf("   -p Byte pattern for data of paket.\n");
            printf("   -s Number of data bytes to be sent.\n");
        }
        goto EXIT_2;
    }

EXIT_1:
    freeaddrinfo(addrinfo_res);
EXIT_2:
    return return_code;
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  shell_ping_take_value
*  Returned Value:  Error.
*  Comments  :  Convert sring to paraneter value.
*
*END*-----------------------------------------------------------------*/
static int32_t shell_ping_take_value(int32_t *value, char *param, char *message )	
{
    char    *e;

	*value = strtol(param, &e, 0);
    if (*e == '\0')
    {	
        printf("num. of %s= %d\n", message, (*value));
        return(0);
    }
    else
    {
        printf("%s is not valid number of %s\n", param, message);
        return(1);
    }
}

#endif /* SHELLCFG_USES_RTCS */


