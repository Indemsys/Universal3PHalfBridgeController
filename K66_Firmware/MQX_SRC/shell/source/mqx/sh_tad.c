/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
* Copyright 2004-2008 Embedded Access Inc.
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
*   This file contains the MQX shell command.
*
*
*END************************************************************************/


#include <ctype.h>
#include <string.h>
#include <mqx.h>
#include <fio.h>
#include <tad.h>
#include "shell.h"


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  Shell_tad
*  Returned Value:  none
*  Comments  :  SHELL utility to print memory and stack usage.
*
*END*-----------------------------------------------------------------*/

int32_t  Shell_tad (int32_t argc, char *argv[])
{ /* Body */
   bool              print_usage, shorthelp = FALSE;
   int32_t               return_code = SHELL_EXIT_SUCCESS;

   print_usage = Shell_check_help_request (argc, argv, &shorthelp);

   if (! print_usage)
   {
      if (argc == 1)
      {
         _tad_stack_usage ();
         _tad_lightweight_memory_blocks ();
      }
      else if (argc == 2)
      {
         if (! strcmp ("stack", argv[1]))
         {
            _tad_stack_usage ();
         }
         else if (! strcmp ("lwmemblock", argv[1]))
         {
            _tad_lightweight_memory_blocks ();
         }
         else
         {
            printf("Error, %s invoked with unknown argument %s\n\r", argv[0], argv[1]);
            return_code = SHELL_EXIT_ERROR;
            print_usage = TRUE;
         }
      }
      else
      {
         printf("Error, %s invoked with incorrect number of arguments\n\r", argv[0]);
         return_code = SHELL_EXIT_ERROR;
         print_usage = TRUE;
      }
   }

   if (print_usage)
   {
      if (shorthelp)
      {
         printf("%s [stack | lwmemblock]\n\r", argv[0]);
      }
      else
      {
         printf("Usage: %s [stack | lwmemblock]\n\r", argv[0]);
         printf("   stack      = prints stack usage\n\r");
         printf("   lwmemblock = prints lightweight memory blocks\n\r");
      }
   }
   return return_code;
} /* Endbody */



/* EOF */
