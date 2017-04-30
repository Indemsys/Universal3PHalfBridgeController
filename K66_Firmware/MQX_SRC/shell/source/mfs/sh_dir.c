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
#include <sh_prv.h>

#if SHELLCFG_USES_MFS
#include <mfs.h>

#define BUFFER_SIZE  256

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   Shell_dir
* Returned Value   :  int32_t error code
* Comments  :  Reads from a file .
*
*END*---------------------------------------------------------------------*/

int32_t  Shell_dir(int32_t argc, char *argv[] )
{ /* Body */
   bool              print_usage, shorthelp = FALSE;
   int32_t               return_code = SHELL_EXIT_SUCCESS;
   MQX_FILE_PTR         fs_ptr;
   char             *path_ptr, *mode_ptr;
   void                *dir_ptr;
   char             *buffer = NULL;
   SHELL_CONTEXT_PTR    shell_ptr = Shell_get_context( argv );
   int32_t               error = 0;


   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if (argc > 3)  {
         printf("Error, invalid number of parameters\n\r");
         return_code = SHELL_EXIT_ERROR;
         print_usage=TRUE;
      } else {
         path_ptr  ="*.*";
         mode_ptr = "m";
         if (argc >= 2)  {
            path_ptr = argv[1];
            if (argc== 3)  {
               mode_ptr = argv[2];
            }
         }

         fs_ptr = Shell_get_current_filesystem(argv);
         /* check if filesystem is mounted */
         if (fs_ptr == NULL)  {
             printf("Error, file system not mounted\n\r");
             return_code = SHELL_EXIT_ERROR;
         } else  {
            buffer = _mem_alloc(BUFFER_SIZE);
            error = ioctl(fs_ptr, IO_IOCTL_CHANGE_CURRENT_DIR, shell_ptr->CURRENT_DIR);
            if (buffer && !error) {

               dir_ptr = _io_mfs_dir_open(fs_ptr, path_ptr, mode_ptr );

               if (dir_ptr == NULL)  {
                  printf("File not found.\n\r");
                  return_code = SHELL_EXIT_ERROR;
               } else {
                  while ((_io_is_fs_valid(fs_ptr)) && (_io_mfs_dir_read(dir_ptr, buffer, BUFFER_SIZE) > 0)) {
                     printf(buffer);
                  }
                  _io_mfs_dir_close(dir_ptr);
               }
               _mem_free(buffer);
            } else {
               if(buffer == NULL){
                 printf("Error, unable to allocate space.\n\r" );
               } else {
                 printf("Error, directory does not exist.\n\r" );
               }
               return_code = SHELL_EXIT_ERROR;
            }
         }
      }
   }

   if (print_usage)  {
      if (shorthelp)  {
         printf("%s [<filespec>] [<attr>]]\n\r", argv[0]);
      } else  {
         printf("Usage: %s [<filespec> [<attr>]]\n\r", argv[0]);
         printf("   <filespec>   = files to list\n\r");
         printf("   <attr>       = attributes of files: adhrsv*\n\r");
      }
   }
   return return_code;
} /* Endbody */
#endif //SHELLCFG_USES_MFS
/* EOF*/
