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

#include <ctype.h>
#include <string.h>
#include <mqx.h>
#include "shell.h" 
#if SHELLCFG_USES_RTCS
#include <rtcs.h>
#include "sh_rtcs.h" 



/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   Shell_sendto
* Returned Value   :  int32_t error code
* Comments  :  Send a UDP broadcase packet to the local network.
*
*END*---------------------------------------------------------------------*/

int32_t  Shell_sendto(int32_t argc, char *argv[] )
{ /* Body */
   bool           print_usage, shorthelp = FALSE;
   int32_t            return_code = SHELL_EXIT_SUCCESS;
   uint32_t           length = 64, count = 1, port =0;
   _ip_address       hostaddr = 0;
   char              hostname[MAX_HOSTNAMESIZE] = {0};

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if ((argc<3) || (argc>5)) {
         printf("Error, invalid number of parameters\n");
         return_code = SHELL_EXIT_ERROR;
         print_usage=TRUE;
      } else {
         if (argc>4) {
            if (! Shell_parse_number( argv[4], &count ))  {
               printf("Error, invalid count\n");
               return_code = SHELL_EXIT_ERROR;
            }
         }
         if (argc>3) {
            if (! Shell_parse_number( argv[3], &length ))  {
               printf("Error, invalid length\n");
               return_code = SHELL_EXIT_ERROR;
            }
         }

         if (! Shell_parse_number( argv[2], &port ))  {
            printf("Error, invalid port\n");
            return_code = SHELL_EXIT_ERROR;
         }

         RTCS_resolve_ip_address( argv[1], &hostaddr, hostname, MAX_HOSTNAMESIZE ); 

         if (!hostaddr)  {
            printf("Unable to resolve host\n");
            return_code = SHELL_EXIT_ERROR;
         } else  {
            printf("Sending to %s [%ld.%ld.%ld.%ld]:\n", hostname, IPBYTES(hostaddr));
         }

      }
      if (return_code != SHELL_EXIT_ERROR) {
         unsigned char   *buffer = _mem_alloc(length);
         uint32_t     sock = socket(PF_INET, SOCK_DGRAM, 0);
         sockaddr_in laddr, raddr;
         uint32_t     result;
         uint32_t     i;
         uint16_t     rlen = sizeof(raddr);

         if ((buffer!=NULL) && (sock!=RTCS_SOCKET_ERROR)) {
            memset((char *) &laddr, 0, sizeof(laddr));
            laddr.sin_family = AF_INET;
            laddr.sin_port = 0;
            laddr.sin_addr.s_addr = INADDR_ANY;

            raddr.sin_family = AF_INET;
            raddr.sin_port = (uint16_t) port;
            raddr.sin_addr.s_addr = hostaddr;


            result = bind(sock, (const struct sockaddr *)&laddr, sizeof (sockaddr_in));
            if (result != RTCS_OK) {   
               printf("Error, unable to bind socket\n");
               return_code = SHELL_EXIT_ERROR;
            } else {

               for (i=0;i<length;i++) {
                  buffer[i] = (unsigned char) (i & 0xff);
               }
               printf("\n");
               for (i=0;i<count;i++) {
                  printf("Sending  packet # %lu\r",i+1);
                  sendto(sock, buffer, length, 0, (sockaddr *)&raddr, rlen);
               }
               printf("\n");
            }
         } else {
            printf("Error, unable to allocate resources for this command\n");
            return_code = SHELL_EXIT_ERROR;
         }
         if (sock!=RTCS_SOCKET_ERROR) {
            shutdown(sock, FLAG_ABORT_CONNECTION);
         }
         if (buffer!=NULL) {
            _mem_free(buffer);
         }
      }
   }
   
   if (print_usage)  {
      if (shorthelp)  {
         printf("%s <host> <port> [<length>] [<count>]\n", argv[0]);
      } else  {
         printf("Usage: %s <host> <port> [<length>] [<count>]\n", argv[0]);
         printf("   <host>   = destination host\n");
         printf("   <port>   = destination port\n");
         printf("   <length> = number of bytes\n");
         printf("   <count>  = number of messages\n");
      }
   }
   return return_code;
} /* Endbody */
#endif /* SHELLCFG_USES_RTCS */
/* EOF*/
