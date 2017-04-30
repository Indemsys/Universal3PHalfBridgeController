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
#include <sh_mfs.h>

#define SECTOR_SIZE 512

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   Shell_disect
* Returned Value   :  int32_t error code
* Comments  :  Reads from a file .
*
*END*---------------------------------------------------------------------*/


int32_t  Shell_disect(int32_t argc, char *argv[] )
{ /* Body */
   int32_t            return_code = SHELL_EXIT_SUCCESS;
   bool           print_usage, shorthelp = FALSE;
   uint32_t           sector,num_sectors;
   int32_t            offset=0;
   MQX_FILE_PTR      fd, fs;
   uint32_t           e,i;
   unsigned char         *buffer;

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if ((argc < 2) || (argc > 5)) {
         printf("Error, invalid number of parameters\n\r");
         return_code = SHELL_EXIT_ERROR;
         print_usage=TRUE;
      } else if ( !Shell_parse_uint_32(argv[1], (uint32_t *) &offset ))  {
         printf("Error, invalid length\n\r");
         return_code = SHELL_EXIT_ERROR;
         print_usage=TRUE;
      } else {
         num_sectors = 1;

         if (argc >= 3) {
            if ( !Shell_parse_uint_32(argv[2], (uint32_t *) &num_sectors )) {
               num_sectors = 1;
            }
         }
         if (argc >= 4) {
            fd = fopen(argv[3], "r");
            if (!fd) {
               printf("Error, unable to open file %s.\n\r", argv[1] );
               return_code = SHELL_EXIT_ERROR;
            }
         } else {
            fs = Shell_get_current_filesystem(argv);
            _io_ioctl(fs, IO_IOCTL_GET_DEVICE_HANDLE, &fd);
         }

         if (fd)  {
            buffer = _mem_alloc(SECTOR_SIZE);
            if (buffer) {
               for(sector=0;sector<num_sectors;sector++) {
                  if (fseek(fd, offset+sector, IO_SEEK_SET) == IO_ERROR)  {
                     printf("Error, unable to seek to sector %s.\n\r", argv[1] );
                     return_code = SHELL_EXIT_ERROR;
                  } else if (_io_read(fd, (char *) buffer, 1) != 1) {
                     printf("Error, unable to read sector %s.\n\r", argv[1] );
                     return_code = SHELL_EXIT_ERROR;
                  } else {
                     printf("\nSector # %d\n\r",offset+sector);
                     for (e=0;e<16;e++)  {
                        for (i=0;i<32;i++) {
                           printf("%02x ",(uint32_t) buffer[e*32+i]);
                        }
                        printf("\n\r");
                     }
                  }
               }
               _mem_free(buffer);
            }
         }
         printf("\n\r");
         if (argc >= 4) {
            fclose(fd);
         }
      }
   }

   if (print_usage)  {
      if (shorthelp)  {
         printf("%s <sector> [<device>]\n\r", argv[0]);
      } else  {
         printf("Usage: %s <sector> [<device>]\n\r", argv[0]);
         printf("   <sector>     = sector number\n\r");
         printf("   <device>     = low level device\n\r");
      }
   }

   return return_code;
} /* Endbody */

#endif // SHELLCFG_USES_MFS

/* EOF*/
