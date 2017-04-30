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
#include <shell.h>

#if SHELLCFG_USES_MFS
#include <mfs.h>

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   Shell_flush_cache
* Returned Value   :  int32_t error code
* Comments  :  flush the write cache.
*
* Usage:  flush [filesys:]
*
*END*---------------------------------------------------------------------*/

int32_t  Shell_flush_cache(int32_t argc, char *argv[] )
{ /* Body */
   bool                    print_usage, shorthelp = FALSE;
   int32_t                     return_code = SHELL_EXIT_SUCCESS;
   MQX_FILE_PTR               fs_ptr;
   char                   *name_ptr = NULL;


   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if (argc > 2) {
         printf("Error, invalid number of parameters\n\r");
         return_code = SHELL_EXIT_ERROR;
         print_usage=TRUE;
      } else {
         if (argc==2) {
            name_ptr = argv[1];
         } else {
            name_ptr = Shell_get_current_filesystem_name(argv);
         }

         if (_io_validate_device(name_ptr)) {
            fs_ptr = fopen(name_ptr,0);
            if (fs_ptr == NULL )  {
               printf("Error, unable to access file system\n\r" );
               return_code = SHELL_EXIT_ERROR;
            } else  {
               ioctl(fs_ptr, IO_IOCTL_FLUSH_OUTPUT, NULL);
               fclose(fs_ptr);
            }
         }
      }
   }


   if (print_usage)  {
      if (shorthelp)  {
         printf("%s <filesys:>\n\r", argv[0]);
      } else  {
         printf("Usage: %s <filesys:>\n\r", argv[0]);
         printf("   filesys: = name of MFS file system\n\r");
      }
   }
   return return_code;
} /* Endbody */

#endif // SHELLCFG_USES_MFS

/* EOF*/
