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

#include <string.h>
#include <mqx.h>
#include <fio.h>
#include "shell.h"
#include "sh_prv.h"

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  Shell_help
*  Returned Value:  none
*  Comments  :
*
*END*-----------------------------------------------------------------*/

int32_t  Shell_help(int32_t argc, char *argv[] )
{ /* Body */
   bool              print_usage, shorthelp = FALSE;
   int32_t               return_code = SHELL_EXIT_SUCCESS;
   SHELL_CONTEXT_PTR    shell_ptr = Shell_get_context( argv );
   SHELL_COMMAND_PTR    command_ptr;

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if (argc==1)  {
         shell_ptr->ARGV[1] = "help";
         shell_ptr->ARGV[2] = "short";
         shell_ptr->ARGV[3] = NULL;
         printf("Available commands:\n\r");
         command_ptr = shell_ptr->COMMAND_LIST_PTR;
         while (command_ptr->COMMAND != NULL)  {
            shell_ptr->ARGV[0] = command_ptr->COMMAND;
            printf("   ");
            return_code = (*command_ptr->SHELL_FUNC)(3, shell_ptr->ARGV);
            command_ptr++;
         } /* Endwhile */
      } else {
         /* Help on a specific command */
         if (argc==2)  {
            shell_ptr->ARGV[0] = argv[1];
            shell_ptr->ARGV[1] = "help";
            shell_ptr->ARGV[2] = "usage";
            shell_ptr->ARGV[3] = NULL;
            command_ptr = shell_ptr->COMMAND_LIST_PTR;
            while (command_ptr->COMMAND != NULL)  {
               if (strcmp(argv[0], command_ptr->COMMAND) == 0)  {
                  return_code = (*command_ptr->SHELL_FUNC)(3, shell_ptr->ARGV);
                  break;
               }
               command_ptr++;
            } /* Endwhile */
            if (command_ptr->COMMAND == NULL)  {
               printf("Error, command \"%s\" not registered.\n\r", argv[1] );
            }
         } else {
            print_usage = TRUE;
         }
      }
   }

   if (print_usage)  {
      if (shorthelp)  {
         printf("%s [<command>]\n\r", argv[0]);
      } else  {
         printf("Usage: %s [<command>]\n\r", argv[0]);
         printf("   <command> = command to get help on\n\r");
      }
   }
   return return_code;
} /* Endbody */
