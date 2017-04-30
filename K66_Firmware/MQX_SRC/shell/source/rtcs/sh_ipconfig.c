/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
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
*   Example using RTCS Library.
*
*
*END************************************************************************/


#include <mqx.h>
#include "shell.h"
#if SHELLCFG_USES_RTCS

#include <bsp.h>
#include <rtcs.h>
#include <enet.h>
#include <ipcfg.h>
#include <dhcpcln6.h>
#include "sh_rtcs.h"
#include "sh_enet.h"
#include "string.h"

#ifdef BSP_ENET_DEVICE_COUNT
#if  (BSP_ENET_DEVICE_COUNT > 0)

#if RTCSCFG_ENABLE_IP6
static uint32_t dhcp6_handle;
#endif

static int32_t Shell_ipconfig_status (uint32_t enet_device)
{
    IPCFG_STATE             state;
    bool                    link;
    bool                    task;
    _enet_address           mac = {0};
    IPCFG_IP_ADDRESS_DATA   ip_data ;
#if RTCSCFG_IPCFG_ENABLE_DNS
    #if RTCSCFG_ENABLE_IP4
    char        addr_str[RTCS_IP_ADDR_STR_SIZE];
    int         i;
    sockaddr    dns_server;
    #endif
#endif
#if RTCSCFG_IPCFG_ENABLE_BOOT
    _ip_address             tftp_serveraddress;
    unsigned char           *tftp_servername;
    unsigned char           *boot_filename;
#endif
#if RTCSCFG_ENABLE_IP6
    uint32_t                n;
    IPCFG6_GET_ADDR_DATA    data;
    char prn_addr6[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"];
#endif


    state = ipcfg_get_state (enet_device);
    if (state != IPCFG_STATE_INIT)
    {
        link = ipcfg_get_link_active (enet_device);
        task = ipcfg_task_status ();
        ipcfg_get_mac (enet_device, mac);
        ipcfg_get_ip (enet_device, &ip_data);

#if RTCSCFG_IPCFG_ENABLE_BOOT
        tftp_serveraddress = ipcfg_get_tftp_serveraddress (enet_device);
        tftp_servername = ipcfg_get_tftp_servername (enet_device);
        boot_filename = ipcfg_get_boot_filename (enet_device);
#endif
        printf ("Eth#     : %d\n", enet_device);
        printf ("Link     : %s\n", link ? "on" : "off");
        printf ("MTU      : %d\n", RTCS_if_get_mtu(ipcfg_get_ihandle(enet_device)));
        printf ("MAC      : %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

#if RTCSCFG_ENABLE_IP4

        printf ("IP4      : %d.%d.%d.%d Type: %s\n", IPBYTES(ip_data.ip), ipcfg_get_state_string (state) );
        printf ("IP4 Mask : %d.%d.%d.%d\n", IPBYTES(ip_data.mask));
#if RTCSCFG_ENABLE_GATEWAYS
        printf ("IP4 Gate : %d.%d.%d.%d\n", IPBYTES(ip_data.gateway));
#endif
#if RTCSCFG_IPCFG_ENABLE_DNS
        #if RTCSCFG_ENABLE_IP4
        printf ("IP4 DNS  :");
        /* Print server address list.*/
        for(i=0;;i++)
        {
            if(DNSCLN_get_dns_addr(ipcfg_get_ihandle(enet_device), i, &dns_server) == TRUE)
            {
                printf (" %s ", inet_ntop(dns_server.sa_family, &RTCS_SOCKADDR_ADDR(&dns_server), addr_str, sizeof(addr_str)));
            }
            else
            {
                break;
            }
        }
        printf ("\n");
        #endif
#endif

#endif /* RTCSCFG_ENABLE_IP4 */

/************************************************/
    if(RTCS6_if_is_disabled(ipcfg_get_ihandle(enet_device)) == TRUE)
    {
        printf("IP6      : DISABLED\n");
    }
    else
    {
#if RTCSCFG_ENABLE_IP6
        n = 0;
        while(!ipcfg6_get_addr(enet_device, n, &data))
  	    {
  	        if(inet_ntop(AF_INET6,&data.ip_addr, prn_addr6, sizeof(prn_addr6)))
            {
                printf("IP6      : %s ",prn_addr6);
                /* IPv6 address state */
                printf("State: ");
                switch(data.ip_addr_state)
                {
                    case IP6_ADDR_STATE_PREFERRED:
                        printf("PREFERRED ");
                        break;
                    case IP6_ADDR_STATE_TENTATIVE:
                        printf("TENTATIVE ");
                        break;
                    case IP6_ADDR_STATE_NOT_USED:
                        printf("NOT_USED ");
                        break;
                    default:
                        printf("%d ", data.ip_addr_state);
                        break;
                }

                /* IPv6 address type */
                printf("Type: ");
                switch(data.ip_addr_type)
                {
                    case IP6_ADDR_TYPE_MANUAL:
                        printf("MANUAL\n");
                        break;
                    case IP6_ADDR_TYPE_AUTOCONFIGURABLE:
                        printf("AUTOCONFIGURABLE\n");
                        break;
                    case IP6_ADDR_TYPE_DHCP:
                        printf("DHCP\n");
                        break;
                    default:
                        printf("%d\n", data.ip_addr_type);
                        break;
                }
            }
            else
            {
                printf("IP6 : ERROR IP CONVERT\n");
            }
            n++;
        }
        /* Print Scope ID.*/
        printf("ScopeID  : %d\n", ipcfg6_get_scope_id(enet_device));
#endif /* RTCSCFG_ENABLE_IP6 */
    }

#if RTCSCFG_ENABLE_IP4

#if RTCSCFG_IPCFG_ENABLE_BOOT
        printf ("TFTP: %d.%d.%d.%d '%s'\n", IPBYTES(tftp_serveraddress), tftp_servername);
        printf ("BOOT: '%s'\n", boot_filename);
#endif

#endif /* RTCSCFG_ENABLE_IP4 */
        printf ("Link status task %s\n", task ? "running" : "stopped");
    }
    else
    {
        printf ("Ethernet device %d not yet initialized.\n", enet_device);
    }
    return SHELL_EXIT_SUCCESS;
}



static int32_t Shell_ipconfig_init (uint32_t enet_device, uint32_t index, int32_t argc, char *argv[])
{
    uint32_t        error;
    _enet_address   mac;

    ipcfg_get_mac(enet_device, mac);
    mac[5] += enet_device;

    if (argc > ++index)
    {
        if (! Shell_parse_enet_address(argv[index], mac))
        {
            printf ("Error init ethernet device %d, invalid MAC address!\n", enet_device);
            return SHELL_EXIT_ERROR;
        }
    }

    error = ipcfg_init_device (enet_device, mac);
    if (error != 0)
    {
        printf ("Error init ethernet device %d, interface error %08x!\n", enet_device, error);
        return SHELL_EXIT_ERROR;
    }
    printf ("Ethernet device %d initialization successful.\n", enet_device);
    return SHELL_EXIT_SUCCESS;
}

static int32_t Shell_ipconfig_release (uint32_t enet_device, uint32_t index, int32_t argc, char *argv[])
{
    uint32_t         error;

    error = ipcfg_release_device (enet_device);
    if (error != 0)
    {
        printf ("Error release ethernet device %d, error %08x!\n", enet_device, error);
        return SHELL_EXIT_ERROR;
    }

    printf ("Ethernet device %d release successful.\n", enet_device);
    return SHELL_EXIT_SUCCESS;
}



static int32_t Shell_ipconfig_task (uint32_t enet_device, uint32_t index, int32_t argc, char *argv[])
{
    uint32_t error;
    uint32_t priority;
    uint32_t period;

    if (argc > ++index)
    {
        if (strcmp (argv[index], "start") == 0)
        {
        #if 0 /* Preority is defined by user. */
            if (argc > ++index)
            {
                if (! Shell_parse_number(argv[index], &priority))
                {
                    printf ("Error in task command, invalid priority parameter!\n");
                    return SHELL_EXIT_ERROR;
                }
            }
            else
            {
                printf ("Error in task command, missing priority parameter!\n");
                return SHELL_EXIT_ERROR;
            }
        #else /* Priority is set to RTCS plus 1.*/
           priority = _RTCSTASK_priority+1;
        #endif

            if (argc > ++index)
            {
                if (! Shell_parse_number(argv[index], &period))
                {
                    printf ("Error in task command, invalid priority parameter!\n");
                    return SHELL_EXIT_ERROR;
                }
            }
            else
            {
                printf ("Error in task command, missing period parameter!\n");
                return SHELL_EXIT_ERROR;
            }

            error = ipcfg_task_create (priority, period);
            if (error != 0)
            {
                printf ("Error in task command, create failed!\n", enet_device, error);
                return SHELL_EXIT_ERROR;
            }
        }
        else if (strcmp (argv[index], "stop") == 0)
        {
            ipcfg_task_destroy (TRUE);
        }
        else
        {
            printf ("Error in task command, invalid parameter!\n");
            return SHELL_EXIT_ERROR;
        }
    }

    printf ("Link status task %s\n", ipcfg_task_status () ? "running" : "stopped");
    return SHELL_EXIT_SUCCESS;
}



#if RTCSCFG_IPCFG_ENABLE_DNS
static int32_t Shell_ipconfig_dns (uint32_t enet_device, uint32_t index, int32_t argc, char *argv[])
{
    bool            res;
#if RTCSCFG_ENABLE_IP4
    _ip_address     dns_addr;
#endif
#if RTCSCFG_ENABLE_IP6
    in6_addr        dns6_addr;
#endif

    if (argc > ++index)
    {
        if (strcmp (argv[index], "add") == 0)
        {
            if (argc > ++index)
            {
            #if RTCSCFG_ENABLE_IP4
                if(inet_pton(AF_INET, argv[index], &dns_addr, sizeof(dns_addr)) == RTCS_OK)
                {
                    res = ipcfg_add_dns_ip(enet_device, dns_addr);
                }
                else
            #endif
            #if RTCSCFG_ENABLE_IP6
                if(inet_pton(AF_INET6, argv[index], &dns6_addr, sizeof(dns6_addr)) == RTCS_OK)
                {
                    res = ipcfg6_add_dns_ip(enet_device, &dns6_addr);
                }
                else
            #endif
                {
                    printf ("Error in dns command, invalid ip address!\n");
                    return SHELL_EXIT_ERROR;
                }

                if(res == TRUE)
                {
                    printf ("Add dns ip successful.\n");
                }
                else
                {
                    printf ("Add dns ip failed!\n");
                    return SHELL_EXIT_ERROR;
                }
            }
            else
            {
                printf ("Error in dns command, missing ip parameter!\n");
                return SHELL_EXIT_ERROR;
            }
        }
        else if (strcmp (argv[index], "del") == 0)
        {
            if (argc > ++index)
            {
            #if RTCSCFG_ENABLE_IP4
                if(inet_pton(AF_INET, argv[index], &dns_addr, sizeof(dns_addr)) == RTCS_OK)
                {
                    res = ipcfg_del_dns_ip(enet_device, dns_addr);
                }
                else
            #endif
            #if RTCSCFG_ENABLE_IP6
                if(inet_pton(AF_INET6, argv[index], &dns6_addr, sizeof(dns6_addr)) == RTCS_OK)
                {
                    res = ipcfg6_del_dns_ip(enet_device, &dns6_addr);
                }
                else
            #endif
                {
                    printf ("Error in dns command, invalid ip address!\n");
                    return SHELL_EXIT_ERROR;
                }

                if(res == TRUE)
                {
                    printf ("Del dns ip successful.\n");
                }
                else
                {
                    printf ("Del dns ip failed!\n");
                    return SHELL_EXIT_ERROR;
                }
            }
            else
            {
                printf ("Error in dns command, missing ip parameter!\n");
                return SHELL_EXIT_ERROR;
            }
        }
        else
        {
            printf ("Error in dns command, unknown parameter!\n");
            return SHELL_EXIT_ERROR;
        }

    }
    else
    {
        char        addr_str[RTCS_IP_ADDR_STR_SIZE];
        int         i;
        sockaddr    dns_server;

        /* Print server address list.*/
        for(i=0;;i++)
        {
            if(DNSCLN_get_dns_addr(ipcfg_get_ihandle(enet_device), i, &dns_server) == TRUE)
            {
                printf ("[%d] %s\n", i + 1, inet_ntop(dns_server.sa_family, &RTCS_SOCKADDR_ADDR(&dns_server), addr_str, sizeof(addr_str)));
            }
            else
            {
                break;
            }
        }
    }

    return SHELL_EXIT_SUCCESS;
}
#endif



static int32_t Shell_ipconfig_staticip (uint32_t enet_device, uint32_t index, int32_t argc, char *argv[])
{
    uint32_t                 error;

#if RTCSCFG_ENABLE_IP4
    IPCFG_IP_ADDRESS_DATA   ip_data;
#endif


#if RTCSCFG_ENABLE_IP6
    /*IPv6 adds*/
    struct addrinfo         hints;          // used for getaddrinfo()
    struct addrinfo         * addrinfo_res; // used for getaddrinfo()
    IPCFG6_BIND_ADDR_DATA	ip6_bind_data;  // structure for bind interface
#endif
    /* Here we need validate IP address and detect it family */
    /* Dont forget destroy addrinfo_res by freeaddrinfo(addrinfo_res) */


#if RTCSCFG_ENABLE_IP6

    memset(&hints,0,sizeof(hints));
    hints.ai_family 	= AF_UNSPEC;
    hints.ai_socktype 	= SOCK_DGRAM;
    hints.ai_flags 		= AI_NUMERICHOST;
    if (getaddrinfo ( argv[index+1], NULL, &hints, &addrinfo_res) != 0)
    {
        printf("GETADDRINFO error\n");
        return(SHELL_EXIT_ERROR); // we can return right here and do not need free freeaddrinfo(addrinfo_res)
    }
    if(addrinfo_res->ai_family == AF_INET6)
    {
        IN6_ADDR_COPY(&((struct sockaddr_in6 *)((*addrinfo_res).ai_addr))->sin6_addr,&ip6_bind_data.ip_addr);
        ip6_bind_data.ip_addr_type = IP6_ADDR_TYPE_MANUAL;
        freeaddrinfo(addrinfo_res);



        error = ipcfg6_bind_addr (enet_device, &ip6_bind_data);
        if (error != RTCS_OK)
        {
            printf("\nIPCFG Bind IP6(1) is failed, error = %X\n", error);
            return SHELL_EXIT_ERROR;
        }
        printf ("IP6 bind successful.\n");
        return SHELL_EXIT_SUCCESS;
    }
 #if RTCSCFG_ENABLE_IP4

    else if(addrinfo_res->ai_family == AF_INET)
    {
        freeaddrinfo(addrinfo_res);
 #endif

#endif //RTCSCFG_ENABLE_IP6

    #if RTCSCFG_ENABLE_IP4

        if (argc > ++index)
        {
            if (! Shell_parse_ip_address (argv[index], &ip_data.ip))
            {
                printf ("Error in static ip command, invalid ip address!\n");
                return SHELL_EXIT_ERROR;
            }
        }
        else
        {
            printf ("Error in static ip command, missing ip address parameter!\n");
            return SHELL_EXIT_ERROR;
        }

        if (argc > ++index)
        {
            if (! Shell_parse_ip_address (argv[index], &ip_data.mask))
            {
                printf ("Error in static ip command, invalid mask!\n");
                return SHELL_EXIT_ERROR;
            }
        }
        else
        {
            printf ("Error in static ip command, missing mask parameter!\n");
            return SHELL_EXIT_ERROR;
        }

#if RTCSCFG_ENABLE_GATEWAYS
        if (argc > ++index)
        {
            if (! Shell_parse_ip_address (argv[index], &ip_data.gateway))
            {
                printf ("Error in static ip command, invalid gateway!\n");
                return SHELL_EXIT_ERROR;
            }
        }
#endif

        error = ipcfg_bind_staticip (enet_device, &ip_data);
        if (error != 0)
        {
            printf ("Error during static ip bind %08x!\n", error);
            return SHELL_EXIT_ERROR;
        }

        printf ("Static IP4 bind successful.\n");
        return SHELL_EXIT_SUCCESS;
     #endif


#if RTCSCFG_ENABLE_IP6
    #if RTCSCFG_ENABLE_IP4
    }//end if family
    #endif
    printf ("Static bind unsuccessful. Unknown family detected.\n");
    return SHELL_EXIT_ERROR;
#endif


}

#if RTCSCFG_IPCFG_ENABLE_DHCP
static int32_t Shell_ipconfig_dhcp (uint32_t enet_device, uint32_t index, int32_t argc, char *argv[])
{
    uint32_t                 error;
    IPCFG_IP_ADDRESS_DATA    auto_ip_data;
    bool                     auto_ip = FALSE ;

    if (argc > ++index)
    {
        if (strcmp (argv[index], "noauto") == 0)
        {
            auto_ip = FALSE;
        }
        else
        {
            if (! Shell_parse_ip_address (argv[index], &auto_ip_data.ip))
            {
                printf ("Error in dhcp command, invalid ip address!\n");
                return SHELL_EXIT_ERROR;
            }

            if (argc > ++index)
            {
                if (! Shell_parse_ip_address (argv[index], &auto_ip_data.mask))
                {
                    printf ("Error in dhcp command, invalid mask!\n");
                    return SHELL_EXIT_ERROR;
                }
            }
            else
            {
                printf ("Error in dhcp command, missing mask parameter!\n");
                return SHELL_EXIT_ERROR;
            }

#if RTCSCFG_ENABLE_GATEWAYS
            if (argc > ++index)
            {
                if (! Shell_parse_ip_address (argv[index], &auto_ip_data.gateway))
                {
                    printf ("Error dhcp command, invalid gateway!\n");
                    return SHELL_EXIT_ERROR;
                }
            }
            else
            {
                printf ("Error in dhcp command, missing gateway parameter!\n");
                return SHELL_EXIT_ERROR;
            }
#endif
            auto_ip = TRUE;
        }
    }

    error = ipcfg_bind_dhcp_wait (enet_device, auto_ip, &auto_ip_data);
    if (error != IPCFG_OK)
    {
        printf ("Error during dhcp bind %08x!\n", error);
        return SHELL_EXIT_ERROR;
    }

    printf ("Bind via dhcp successful.\n");
    return SHELL_EXIT_SUCCESS;
}
#endif

#if RTCSCFG_ENABLE_IP6
static int32_t Shell_ipconfig_dhcp6 (uint32_t enet_device, uint32_t index, int32_t argc, char *argv[])
{
    DHCPCLN6_PARAM_STRUCT  dhcp6_params = {0};
    in6_addr               address;


    if (argc > (index+1))
    {
        struct addrinfo        hints;
        struct addrinfo        *addrinfo_res;

        /* Translate address string to addrinfo structure. */
        memset(&hints, 0, sizeof(hints));
        hints.ai_family     = AF_INET6;
        hints.ai_flags      = AI_NUMERICHOST;

        if (getaddrinfo(argv[index+1], NULL, &hints, &addrinfo_res) != 0)
        {
            printf("Address conversion failed (input is probably not IPv6 address).\n");
            return(SHELL_EXIT_ERROR);
        }
        if(addrinfo_res->ai_family == AF_INET6)
        {
            IN6_ADDR_COPY(&((struct sockaddr_in6 *)((*addrinfo_res).ai_addr))->sin6_addr,&address);
            dhcp6_params.preferred = &address;
        }
        freeaddrinfo(addrinfo_res);
    }

    dhcp6_params.flags |= DHCPCLN6_FLAG_CHECK_LINK;
    dhcp6_params.interface = RTCS_if_get_handle(enet_device);
    printf("DHCPv6 initialization ");
    dhcp6_handle = DHCPCLN6_init(&dhcp6_params);
    printf("%s.\n", (dhcp6_handle == 0) ? "failed" : "successful");

    if (dhcp6_handle != 0)
    {
        uint32_t i;
        /* Wait 15 seconds for address. */
        for(i = 0; i < SHELL_IPCONFIG_DHCPCLN6_WAIT; i++)
        {
            if (DHCPCLN6_get_status(dhcp6_handle) == DHCPCLN6_STATUS_BOUND)
            {
                printf("Address from DHCPv6 server obtained.\n");
                break;
            }
            _time_delay(1000);
        }

        if (i == SHELL_IPCONFIG_DHCPCLN6_WAIT)
        {
            printf("Failed to obtain address from DHCPv6 server!\n");
        }
    }

    return(SHELL_EXIT_SUCCESS);
}
#endif

#if RTCSCFG_ENABLE_IP4
static int32_t Shell_ipconfig_unbind (uint32_t enet_device)
{
    uint32_t error;

    error = ipcfg_unbind (enet_device);
    if (error != 0)
    {
        printf ("Error during unbind %08x!\n", error);
        return SHELL_EXIT_ERROR;
    }

    printf ("Unbind successful.\n");
    return SHELL_EXIT_SUCCESS;
}
#endif

#if RTCSCFG_ENABLE_IP6
/* undind IP6 address on device */
static int32_t Shell_ipconfig_unbind6 (uint32_t enet_device, uint32_t index, int32_t argc, char *argv[])
{
    uint32_t                 error;
    IPCFG6_UNBIND_ADDR_DATA ip6_data;
    struct addrinfo         hints;          // used for getaddrinfo()
    struct addrinfo         * addrinfo_res; // used for getaddrinfo()

    /* Here we need validate IP address and detect it family */
    memset(&hints,0,sizeof(hints));
    hints.ai_family 	= AF_INET6;
    hints.ai_socktype 	= SOCK_DGRAM;
    hints.ai_flags 		= AI_NUMERICHOST;
    if (getaddrinfo ( argv[index+1], NULL, &hints, &addrinfo_res) != 0)
    {
        printf("GETADDRINFO error\n");
        return(SHELL_EXIT_ERROR); // we can return right here and do not need free freeaddrinfo(addrinfo_res)
    }
    if(addrinfo_res->ai_family == AF_INET6)
    {
        IN6_ADDR_COPY(&((struct sockaddr_in6 *)((*addrinfo_res).ai_addr))->sin6_addr,&ip6_data.ip_addr);
        freeaddrinfo(addrinfo_res);

        error = ipcfg6_unbind_addr ( enet_device, &ip6_data);
        if (error != 0)
        {
            printf ("Error during unbind %08x!\n", error);
            return SHELL_EXIT_ERROR;
        }
        printf ("Unbind successful.\n");
        return SHELL_EXIT_SUCCESS;
    }
        printf ("Unbind unsuccessful. Unexpected inet family\n");
        return SHELL_EXIT_SUCCESS;
}
#endif

static int32_t Shell_ipcfg_phy_registers(uint32_t enet_device)
{
    uint32_t  i,regs[32];

    if (ipcfg_phy_registers (enet_device,32,regs))
    {
    printf ("Phy registers.\n");
    for (i=0;i<32;i++) {
       printf("Register %2d = 0x%04x\n",i,regs[i]);
    }
   } else {
        printf ("Error reading phy registers!\n");
    }
    return SHELL_EXIT_SUCCESS;
}

/*TASK*-----------------------------------------------------------------
*
* Function Name  : Shell_ipconfig
* Returned Value : void
* Comments       :
*
*END------------------------------------------------------------------*/

int32_t Shell_ipconfig(int32_t argc, char *argv[] )
{ /* Body */
    bool                print_usage, shorthelp = FALSE;
    int32_t             return_code = SHELL_EXIT_SUCCESS;
    uint32_t            enet_device = BSP_DEFAULT_ENET_DEVICE;
    uint32_t            index = 1;

    print_usage = Shell_check_help_request (argc, argv, &shorthelp);

    if (!print_usage)
    {
        if (argc > index)
        {
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
            if (argc > index)
            {
                if (strcmp (argv[index], "init") == 0)
                {
                    return_code = Shell_ipconfig_init (enet_device, index, argc, argv);
                }
                else if (strcmp (argv[index], "release") == 0)
                {
                    return_code = Shell_ipconfig_release (enet_device, index, argc, argv);
                }
                else if (strcmp (argv[index], "task") == 0)
                {
                    return_code = Shell_ipconfig_task (enet_device, index, argc, argv);
                }
#if RTCSCFG_IPCFG_ENABLE_DNS
                else if (strcmp (argv[index], "dns") == 0)
                {
                    return_code = Shell_ipconfig_dns (enet_device, index, argc, argv);
                }
#endif
                else if (strcmp (argv[index], "ip") == 0)
                {
                    return_code = Shell_ipconfig_staticip (enet_device, index, argc, argv);
                }
#if RTCSCFG_IPCFG_ENABLE_DHCP
                else if (strcmp (argv[index], "dhcp") == 0)
                {
                    return_code = Shell_ipconfig_dhcp (enet_device, index, argc, argv);
                }
#endif
#if RTCSCFG_ENABLE_IP6
                else if (strcmp (argv[index], "dhcp6") == 0)
                {
                    return_code = Shell_ipconfig_dhcp6(enet_device, index, argc, argv);
                }
#endif
#if RTCSCFG_IPCFG_ENABLE_BOOT
                else if (strcmp (argv[index], "boot") == 0)
                {
                    return_code = Shell_ipconfig_boot (enet_device);
                }
#endif
                else if (strcmp (argv[index], "unbind") == 0)
                {


            #if RTCSCFG_ENABLE_IP6
                #if RTCSCFG_ENABLE_IP4
                    if(argc > 2)
                    {
                #endif
                       return_code = Shell_ipconfig_unbind6 (enet_device, index, argc, argv);
                #if RTCSCFG_ENABLE_IP4
                    }
                    else
                    {
                #endif
            #endif

            #if RTCSCFG_ENABLE_IP4
                        return_code = Shell_ipconfig_unbind (enet_device);
            #endif

            #if RTCSCFG_ENABLE_IP6
                 #if RTCSCFG_ENABLE_IP4
                    }
                 #endif
            #endif

                }
                else if (strcmp (argv[index], "phyinfo") == 0)
                {
                    return_code = Shell_ipcfg_phy_registers (enet_device);
                }
                else
                {
                    printf ("Unknown ipconfig command!\n");
                    return_code = SHELL_EXIT_ERROR;
                }
            }
            else
            {
                return_code = Shell_ipconfig_status (enet_device);
            }
        }
    }

    if ((print_usage) || (return_code != SHELL_EXIT_SUCCESS))
    {
        if (shorthelp)
        {
            printf ("%s [<device>] [<command>]\n", argv[0]);
        }
        else
        {
            printf ("Usage: %s [<device>] [<command>]\n", argv[0]);
            printf ("  Commands:\n");

            printf ("    init [<mac>]\n");
            printf ("        Initialize ethernet device (once).\n");
            printf ("    release\n");
            printf ("        Release ethernet device (once).\n");
            printf ("    task [start <period ms> | stop]\n");
            printf ("        Manage link status checking task.\n");

#if RTCSCFG_IPCFG_ENABLE_DNS
            printf ("    dns [add <ip> | del <ip>]\n");
            printf ("        Manage dns ip list.\n");
#endif
            printf ("    ip <ip> <mask> [<gateway>]\n");
            printf ("        Bind with ip for IPv4. For IPv6 you should put ipv6 address only.\n");
            printf ("        Like \'ip <ipv6>\' to  bind IPv6 address manually.\n");

#if RTCSCFG_IPCFG_ENABLE_DHCP
            printf ("    dhcp [<ipv4> <mask> [<gateway>]]\n");
            printf ("        Bind with dhcp [use <ip> & <mask> in case dhcp fails].\n");
            printf ("        Support IPv4 only.\n");
#endif
#if RTCSCFG_ENABLE_IP6
            printf ("    dhcp6 [<ipv6>]\n");
            printf ("        Start DHCPv6 client and use <ipv6> as preferred address.\n");
            printf ("        This command is IPv6 version of \"ipconfig dhcp\" command.\n");
#endif
#if RTCSCFG_IPCFG_ENABLE_BOOT
            printf ("    boot\n");
            printf ("        Bind with boot protocol.\n");
#endif
            printf ("    unbind [<ipv6>]\n");
            printf ("        Unbind network interface. Using \'unbind\' without parameter\n");
            printf ("        will unbind IPv4 address from interface. In case IPv6 you should\n");
            printf ("        use ipv6 address like parameter to unbind it from interface.\n");


            printf ("  Parameters:\n");
            printf ("    <device>   = Ethernet device number (default %d).\n", BSP_DEFAULT_ENET_DEVICE);
            printf ("    <mac>      = Ethernet MAC address.\n"  );
            printf ("    <priority> = Link status task MQX priority.\n");
            printf ("    <period>   = Link status task check period (ms).\n");
            printf ("    <ip>       = IP address to use, both families. \n");
            printf ("    <ipv4>     = IPv4 address to use. \n");
            printf ("    <ipv6>     = IPv6 address to use. \n");
            printf ("    <mask>     = Network mask to use. \n");
            printf ("    <gateway>  = Network gateway to use.\n");
        }
    }

    return return_code;
} /* Endbody */
#else /* (BSP_ENET_DEVICE_COUNT > 0) */
int32_t Shell_ipconfig(int32_t argc, char *argv[] )
{
    printf ("Cannot use this command, no enet device driver available in this BSP.");
    return SHELL_EXIT_ERROR;
}
#endif /* (BSP_ENET_DEVICE_COUNT > 0) */
#else /* BSP_ENET_DEVICE_COUNT */
int32_t Shell_ipconfig(int32_t argc, char *argv[] )
{
    printf ("Cannot use this command, no enet device driver available in this BSP.");
    return SHELL_EXIT_ERROR;
}
#endif /* BSP_ENET_DEVICE_COUNT */
#endif /* SHELLCFG_USES_RTCS */
/* EOF */

