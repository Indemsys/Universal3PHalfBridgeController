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

#define COMPARE_BLOCK_SIZE 512

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :  Shell_compare
* Returned Value   :  uint32_t error code
* Comments  :  Renames or moves a file.
*
*END*---------------------------------------------------------------------*/

int32_t  Shell_compare(int32_t argc, char *argv[] )
{ /* Body */
   bool      print_usage, shorthelp = FALSE;
   int32_t      size1, size2, return_code = SHELL_EXIT_SUCCESS;
   MQX_FILE_PTR in_fd_1=NULL, in_fd_2=NULL;
   char         *file1=NULL, *file2=NULL;
   SHELL_CONTEXT_PTR    shell_ptr = Shell_get_context( argv );
   int32_t               error = 0;

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if (argc != 3)  {
         printf("Error, invalid number of parameters\n\r");
         return_code = SHELL_EXIT_ERROR;
         print_usage=TRUE;
      }
      /* check if filesystem is mounted */
      else if (NULL == Shell_get_current_filesystem(argv))
      {
         printf("Error, file system not mounted\n\r");
         return_code = SHELL_EXIT_ERROR;
      }
      else if (MFS_alloc_path(&file1) != MFS_NO_ERROR) {
         printf("Error, unable to allocate memory for paths\n\r" );
         return_code = SHELL_EXIT_ERROR;
      } else {
         error = _io_rel2abs(file1,shell_ptr->CURRENT_DIR,(char *) argv[1],PATHNAME_SIZE,shell_ptr->CURRENT_DEVICE_NAME);
         if(!error)
         {
            in_fd_1 = fopen(file1, "r");
         }

         error = _io_rel2abs(file1,shell_ptr->CURRENT_DIR,(char *) argv[2],PATHNAME_SIZE,shell_ptr->CURRENT_DEVICE_NAME);
         if(!error)
         {
            in_fd_2 = fopen(file1, "r");
         }

         MFS_free_path(file1);

         if (in_fd_1 == NULL)  {
             printf("Error, unable to open file %s\n\r", argv[1] );
             return_code = SHELL_EXIT_ERROR;
         } else  if (in_fd_2 == NULL)  {
             printf("Error, unable to open file %s\n\r", argv[2] );
             return_code = SHELL_EXIT_ERROR;
         } else {
            file1 = _mem_alloc_zero(COMPARE_BLOCK_SIZE);
            file2 = _mem_alloc_zero(COMPARE_BLOCK_SIZE);
            if ((file1==NULL) || (file2==NULL)) {
               printf("Error, unable to allocate buffer space" );
               return_code = SHELL_EXIT_ERROR;
            } else {
               do {
                  size1 = read(in_fd_1, file1, COMPARE_BLOCK_SIZE);
                  size2 = read(in_fd_2, file2, COMPARE_BLOCK_SIZE);
                  if (size1!=size2) {
                     printf("Compare failed, files have different sizes\n\r" );
                     return_code = SHELL_EXIT_ERROR;
                  } else if (size1 > 0) {
                     if (memcmp(file1,file2,COMPARE_BLOCK_SIZE)!=0) {
                        printf("Compare failed, files are different\n\r" );
                        return_code = SHELL_EXIT_ERROR;
                     }
                  }
               } while ((size1>0) && (return_code == SHELL_EXIT_SUCCESS));
               if (return_code == SHELL_EXIT_SUCCESS) {
                  printf("The files are identical\n\r" );
               }
            }
            if (file1) _mem_free(file1);
            if (file2) _mem_free(file2);
         }
         if (in_fd_1) fclose(in_fd_1);
         if (in_fd_2) fclose(in_fd_2);
      }
   }


   if (print_usage)  {
      if (shorthelp)  {
         printf("%s <file1> <file2> \n\r", argv[0]);
      } else  {
         printf("Usage: %s <file1> <file2>\n\r", argv[0]);
         printf("   <file1> = first file to compare\n\r");
         printf("   <file2> = second file to compare\n\r");
      }
   }
   return return_code;
} /* Endbody */

#endif //SHELLCFG_USES_MFS
/* EOF*/
