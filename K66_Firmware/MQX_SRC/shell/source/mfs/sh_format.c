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

static const char *BYTE_SIZES[] = { "bytes", "kB", "MB", "GB", "TB" };

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   Shell_format
* Returned Value   :  int32_t error code
* Comments  :  mount a filesystem on a device.
*
* Usage:  format <drive:> [<volume label>]
*
*END*---------------------------------------------------------------------*/

int32_t  Shell_format(int32_t argc, char *argv[] )
{ /* Body */
   bool                    print_usage, shorthelp = FALSE;
   int32_t                     return_code = SHELL_EXIT_SUCCESS;
   int32_t                     error_code = IO_ERROR;
   MQX_FILE_PTR               fs_ptr = NULL;
   char                   *volume_ptr[SFILENAME_SIZE];
   uint64_t                    big_number;
   uint32_t                    small_number;

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if (argc < 2 || argc >  3) {
         printf("Error, invalid number of parameters\n\r");
         return_code = SHELL_EXIT_ERROR;
         print_usage=TRUE;
      } else  {
         if (argc == 3)  {
            memcpy (volume_ptr, argv[2], SFILENAME_SIZE);
         } else  {
            volume_ptr[0] = 0;
         }
         fs_ptr = _io_get_fs_by_name(argv[1]);
         if (fs_ptr == NULL)  {
            printf("Error, file system %s not found\n\r", argv[1] );
            return_code = SHELL_EXIT_ERROR;
         } else {
            printf("\nFormatting...\n\r");
            error_code = ioctl(fs_ptr, IO_IOCTL_DEFAULT_FORMAT,  NULL);
            if ( !error_code && (*volume_ptr)) {
               error_code = ioctl(fs_ptr, IO_IOCTL_SET_VOLUME,  (void *) volume_ptr);
            }
            if (error_code) {
               printf("Error while formatting: 0x%x\n\r", error_code);
            } else  {
               /* print disk information */
               error_code = ioctl(fs_ptr, IO_IOCTL_GET_VOLUME, (uint32_t*)volume_ptr);
               printf("Done. Volume name is %s\n\r", volume_ptr);
               ioctl(fs_ptr, IO_IOCTL_FREE_SPACE, &big_number);
               for (small_number = 0; big_number > 128 * 1024; big_number >>= 10) small_number++;
               printf("Free disk space: %lu %s\n\r", (uint32_t)big_number, BYTE_SIZES[small_number]);
            }
         }
      }
   }

   if (print_usage)  {
      if (shorthelp)  {
         printf("%s <drive:> [<volume label>]\n\r", argv[0]);
      } else  {
         printf("Usage: %s <drive:> [<volume label>]\n\r", argv[0]);
         printf("   <drive:> = specifies the drive name (followed by a colon)\n\r");
         printf("   <volume label>  = specifies the volume label\n\r");
      }
   }
   return return_code;
}

#endif // SHELLCFG_USES_MFS

/* EOF*/
