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
#include <fio.h>
#include "shell.h"


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  Shell_pause
*  Returned Value:  none
*  Comments  :  SHELL utility to pause the shell for specified time
*
*END*-----------------------------------------------------------------*/

int32_t  Shell_pause(int32_t argc, char *argv[] )
{ /* Body */
   bool              print_usage, shorthelp = FALSE;
   int32_t               return_code = SHELL_EXIT_SUCCESS;
   uint32_t              pause=10*60*1000;

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if (argc == 1)  {
         printf("Pausing for 5 minutes...\n\r");
        _time_delay(5*60*1000);
          printf("Done\n\r");
      } else if (argc==2) {
         if (!Shell_parse_uint_32(argv[1], &pause  )) {
            printf("Error, invalid rule priority\n\r");
            return_code = SHELL_EXIT_ERROR;
         }
          printf("Pausing for %d minutes...\n\r",pause);
          _time_delay(pause*60*1000);
          printf("Done\n\r");
      } else {
         printf("Error, %s invoked with incorrect number of arguments\n\r", argv[0]);
         return_code = SHELL_EXIT_ERROR;
         print_usage = TRUE;
      }
   }

   if (print_usage)  {
      if (shorthelp)  {
         printf("%s [<minutes>]\n\r", argv[0]);
      } else  {
         printf("Usage: %s <minutes>\n\r", argv[0]);
         printf("   <minutes>   = minutes to pause for\n\r");
      }
   }
   return return_code;
} /* Endbody */



/* EOF */
