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
*   Example using RTCS Library.
*
*
*END************************************************************************/

#include <mqx.h>
#include "shell.h"
#if SHELLCFG_USES_RTCS

#include <rtcs.h>
#include "sh_rtcs.h"



/*TASK*-----------------------------------------------------------------
*
* Function Name  : Shell_gate
* Returned Value : void
* Comments       :
*
*END------------------------------------------------------------------*/

int32_t Shell_gate(int32_t argc, char *argv[] )
{ /* Body */
   bool           print_usage, shorthelp = FALSE;
   int32_t            return_code = SHELL_EXIT_SUCCESS;

   _ip_address       gate_ipaddr;
   _ip_address       ipaddr = INADDR_ANY;
   _ip_address       ipmask = INADDR_ANY;
   uint32_t           error = 0;
  
   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if ((argc <= 1) || (argc > 4))  {
         return_code = SHELL_EXIT_ERROR;
         print_usage = TRUE;
      } else  {
         if (! Shell_parse_ip_address( argv[1], &gate_ipaddr  ))  {
            printf("Error, invalid gateway address\n");
            return_code = SHELL_EXIT_ERROR;
         } else if (argc > 2)  {
            if (! Shell_parse_ip_address( argv[2], &ipaddr  ))  {
               printf("Error, invalid ip address\n");
               return_code = SHELL_EXIT_ERROR;
            } else if (argc > 3)  {
               if (! Shell_parse_netmask( argv[3], &ipmask ))  {
                  printf("Error, invalid ip mask\n");
                  return_code = SHELL_EXIT_ERROR;
               }          
            }          
         }          
      }          
         
         
      if (return_code == SHELL_EXIT_SUCCESS)  {
         printf("Adding gateway %d.%d.%d.%d, ip address: %d.%d.%d.%d, netmask: %d.%d.%d.%d\n",
              IPBYTES(gate_ipaddr),IPBYTES(ipaddr),IPBYTES(ipmask));
         error = RTCS_gate_add(gate_ipaddr, ipaddr, ipmask);
         if (error) {
            printf("Error, could not initialize gateway %d.%d.%d.%d\n", gate_ipaddr);
            printf("RTCS_gate_add returned: %d\n", error);
            return_code = SHELL_EXIT_ERROR;
         } 
      } 
   }
   
   if (print_usage)  {
      if (shorthelp)  {
         printf("%s <gateway> [<netaddr>] [<netmask>]\n", argv[0]);
      } else  {
         printf("Usage: %s  <gateway> [<netaddr>] [<netmask>]\n", argv[0]);
         printf("   <gateway> = Gateway IPv4 address\n");
         printf("   <netaddr> = Network IPv4 address\n");
         printf("   <netmask> = Network IPv4 address mask\n");
      }
   }

   return return_code;
} /* Endbody */

#endif /* SHELLCFG_USES_RTCS */
/* EOF */

