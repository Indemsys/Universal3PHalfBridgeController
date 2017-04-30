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
*  Function Name :  Shell_command_list
*  Returned Value:  none
*  Comments  :  SHELL utility to load and execute an executable file
*
*END*-----------------------------------------------------------------*/

int32_t  Shell_command_list(int32_t argc, char *argv[] )
{ /* Body */
   bool              print_usage, shorthelp = FALSE;
   int32_t               return_code = SHELL_EXIT_SUCCESS;
   SHELL_CONTEXT_PTR    shell_ptr = Shell_get_context( argv );
   SHELL_COMMAND_PTR    command_ptr;
   uint32_t              i;

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if (argc==1)  {
         for (i=0,command_ptr = shell_ptr->COMMAND_LIST_PTR;
              command_ptr->COMMAND != NULL;
              i++,command_ptr++)
          {
            printf("%-8s ", command_ptr->COMMAND);
            if ((i&7)==7) printf("\n\r");
         } /* Endwhile */
         if ((i&7)!=0) printf("\n\r");
      } else {
         print_usage = TRUE;
      }
   }

   if (print_usage)  {
      if (shorthelp)  {
         printf("%s\n\r", argv[0]);
      } else  {
         printf("Usage: %s\n\r", argv[0]);
      }
   }
   return return_code;
} /* Endbody */
/* EOF*/
