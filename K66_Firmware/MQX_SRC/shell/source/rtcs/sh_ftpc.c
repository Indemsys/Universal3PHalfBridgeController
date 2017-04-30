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
#include "ftpc.h"

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  Shell_FTP_client
*  Returned Value:  none
*  Comments  :  SHELL utility to TFTP to or from a host
*  Usage:  tftp host get source [destination] [mode] 
*
*END*-----------------------------------------------------------------*/

int32_t  Shell_FTP_client(int32_t inargc, char *inargv[] )
{
   bool              print_usage, shorthelp = FALSE;
   int32_t               return_code = SHELL_EXIT_SUCCESS;
   

   print_usage = Shell_check_help_request(inargc, inargv, &shorthelp );

   if (!print_usage)  {

      if (inargc > 2)  {
         printf("Error, %s invoked with incorrect number of arguments\n", inargv[0]);
         print_usage = TRUE;
      } else  {
         FTP_client(inargv[1] );
      } 
   } 

   if (print_usage)  {
      if (shorthelp)  {
         printf("%s [<host>]\n", inargv[0]);
      } else  {
         printf("Usage: %s [<host>]\n", inargv[0]);
         printf("   <host>   = host ip address or name\n");
      }
   }
   return return_code;
} /* Endbody */

#endif /* SHELLCFG_USES_RTCS */
