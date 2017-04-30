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
*   This file contains the source for an MFS shell function.
*
*
*END************************************************************************/

#include <string.h>
#include <mqx.h>
#include <fio.h>
#include "shell.h"
#include "sh_prv.h"
//#include "fs_if.h"


#if SHELLCFG_USES_MFS
/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   Shell_sh
* Returned Value   :  int32_t error code
* Comments  :  Reads from a file .
*
*END*---------------------------------------------------------------------*/

int32_t  Shell_sh(int32_t argc, char *argv[] )
{ /* Body */
   bool              print_usage, shorthelp = FALSE;
   int32_t               return_code = SHELL_EXIT_SUCCESS, error;
   SHELL_CONTEXT_PTR    shell_ptr = Shell_get_context( argv );
   MQX_FILE_PTR         fd;
   char                 abs_path[SHELL_MAX_FILELEN];

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if (argc != 2) {
         printf("Error, invalid number of parameters\n\r");
         return_code = SHELL_EXIT_ERROR;
         print_usage=TRUE;
      } else  {
        /* check if filesystem is mounted */
        if (NULL == Shell_get_current_filesystem(argv))  {
           printf("Error, file system not mounted\n\r");
           return_code = SHELL_EXIT_ERROR;
        } else {
        error = _io_rel2abs(abs_path,shell_ptr->CURRENT_DIR,(char *) argv[1],sizeof(abs_path),shell_ptr->CURRENT_DEVICE_NAME);
         if (!error)
         {
            fd = fopen(abs_path, "r");
            if (fd)  {
               shell_ptr = Shell_get_context( argv );
               if (shell_ptr->COMMAND_FP != stdin)  {
                 fclose(shell_ptr->COMMAND_FP);
               }
               shell_ptr->COMMAND_FP = fd;
            } else  {
               printf("Error, unable to open file %s.\n\r", argv[1] );
               return_code = SHELL_EXIT_ERROR;
            }
         } else  {
            printf("Error, unable to open file %s.\n\r", argv[1] );
            return_code = SHELL_EXIT_ERROR;
         }
        }
      }
   }

   if (print_usage)  {
      if (shorthelp)  {
         printf("%s <filename>\n\r", argv[0]);
      } else  {
         printf("Usage: %s <filename>\n\r", argv[0]);
         printf("   <filename>   = filename to execute\n\r");
      }
   }
   return return_code;


} /* Endbody */
#endif //SHELLCFG_USES_MFS
/* EOF*/
