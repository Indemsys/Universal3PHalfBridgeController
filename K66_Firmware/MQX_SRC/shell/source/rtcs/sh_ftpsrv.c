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
#include "ftpsrv.h"
#include "sh_rtcs.h"

static const FTPSRV_AUTH_STRUCT ftpsrv_users[] =
{
    {"developer", "freescale", NULL},
    {0, 0, 0}
};

uint32_t ftpsrv_handle;

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  Shell_FTPd
*  Returned Value:  none
*  Comments  :  SHELL utility to start and stop FTP server
*  Usage:  ftpsrv start/stop
*
*END*-----------------------------------------------------------------*/

int32_t  Shell_ftpsrv(int32_t argc, char *argv[] )
{
    bool              print_usage;
    bool              shorthelp = FALSE;
    int32_t           return_code = SHELL_EXIT_SUCCESS;    

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (argc < 2)
   {
      printf("Error, %s invoked with incorrect number of arguments\n", argv[0]);
      print_usage = TRUE;
   }

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
      return return_code;
   }

   if (strcmp(argv[1], "start") == 0)
   {
      FTPSRV_PARAM_STRUCT  params = {0};
      uint32_t old_handle = ftpsrv_handle;
      #if RTCSCFG_ENABLE_IP4
      params.af |= AF_INET;
      #endif
      #if RTCSCFG_ENABLE_IP6
      params.af |= AF_INET6;
      #endif
      params.auth_table = (FTPSRV_AUTH_STRUCT*) ftpsrv_users;
      params.root_dir = "a:";
          
      ftpsrv_handle = FTPSRV_init(&params);
      if (ftpsrv_handle != 0)
      {
         printf("FTP Server Started. Root directory is set to \"%s\", login: \"%s\", password: \"%s\".\n", 
            params.root_dir,
            ftpsrv_users[0].uid, 
            ftpsrv_users[0].pass);
      }
      else
      {
         char *message = "";
         
         ftpsrv_handle = old_handle;
         if (old_handle != 0)
         {
            message = "Server is already running.\n";
         }
         printf("Unable to start FTP Server.\n%s", message);
         return_code = SHELL_EXIT_ERROR;
      }
   }
   else if (strcmp(argv[1], "stop") == 0)
   {
      uint32_t result;

      result = FTPSRV_release(ftpsrv_handle);
      if (result == FTPSRV_OK)
      {
         printf("FTP server stopped.\n");
      }
      else
      {
         printf("Unable to stop FTP Server, error = 0x%x\n",result);
         return_code = SHELL_EXIT_ERROR;
      }
   }
   else
   {
      printf("Error, %s invoked with incorrect option\n", argv[0]);
   }

   return return_code;
} /* Endbody */

#endif /* SHELLCFG_USES_RTCS */

/* EOF */
