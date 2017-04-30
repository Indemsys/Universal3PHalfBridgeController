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

#define SHELL_ECHO_PRIO        (RTCSCFG_DEFAULT_RTCSTASK_PRIO+1)

#if RTCSCFG_ENABLE_IP6
  #if RTCSCFG_ENABLE_IP4
  #define SHELL_ECHO_AF (AF_INET | AF_INET6)
#else
  #define SHELL_ECHO_AF (AF_INET6)
#endif
#else
  #define SHELL_ECHO_AF (AF_INET)
#endif

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  Shell_echosrv
*  Returned Value:  none
*  Comments  :  Echo server UDP TCP
*  Usage:  echosrv start|stop
*
*END*-----------------------------------------------------------------*/

int32_t  Shell_echosrv(int32_t argc, char *argv[] )
{
   static void * server_h = 0;
   void * pTemp = NULL;
   uint32_t result;
   bool  print_usage, shorthelp = FALSE;
   int32_t   return_code = SHELL_EXIT_SUCCESS;
   
   ECHOSRV_PARAM_STRUCT params = {
     SHELL_ECHO_AF, /* AF_INET | AF_INET6 for IPv4+IPv6, AF_INET for IPv4, AF_INET6 for IPv6 */
     7,         /* service runs on port 7 by default */
   #if RTCSCFG_ENABLE_IP4
     INADDR_ANY,   /* Listening IPv4 address */
   #endif
   #if RTCSCFG_ENABLE_IP6
     IN6ADDR_ANY_INIT,   /* Listening IPv6 address */
     0,  /* Scope ID for IPv6. 0 is for any Interface. */
   #endif
     SHELL_ECHO_PRIO
   };

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if (argc == 2)  {
         if (strcmp(argv[1], "start") == 0)  {
            if(0 == server_h)
            {
              pTemp = ECHOSRV_init(&params);
              if (pTemp !=  0)  {
                 printf("ECHO Server Started.\n");
                 server_h = pTemp;
              } else  {
                 printf("Unable to start ECHO Server, error = 0x%x\n",RTCS_errno);
                 _task_set_error(MQX_OK); /* clear RTCS_errno */
                 return_code = SHELL_EXIT_ERROR;
              }  
            }
            else 
            {
              printf("ECHO Server already running\n");
            }
            
         } else if (strcmp(argv[1], "stop") == 0)  {
            result = ECHOSRV_release(server_h);
            if (result ==  RTCS_OK)  {
               printf("ECHO Server Stopped.\n");
               server_h = 0;
            } else  {
               printf("Unable to stop ECHO Server, error = 0x%x\n",result);
               return_code = SHELL_EXIT_ERROR;
            }
         } else  {
         printf("Error, %s invoked with incorrect option\n", argv[0]);
            print_usage = TRUE;
         }
      } else  {
         printf("Error, %s invoked with incorrect number of arguments\n", argv[0]);
         print_usage = TRUE;
      }
   }
   
   if (print_usage)  {
      if (shorthelp)  {
         printf("%s [start|stop]\n", argv[0]);
      } else  {
         printf("Usage: %s [start|stop]\n",argv[0]);
      }
   }
   return return_code;
} /* Endbody */

#endif /* SHELLCFG_USES_RTCS */

/* EOF */
