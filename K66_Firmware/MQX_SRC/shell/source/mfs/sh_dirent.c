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
#include <ctype.h>

#if SHELLCFG_USES_MFS
#include <mfs.h>
#include <mfs_prv.h>
#include <sh_mfs.h>

#define SECTOR_SIZE MFS_DEFAULT_SECTOR_SIZE
#define DIRENTS_PER_SECTOR (SECTOR_SIZE/sizeof(DIR_ENTRY_DISK))

static bool is_zero(unsigned char *buffer, uint32_t size)
{
   uint32_t i;

   for (i=0;i<size;i++) {
      if (buffer[i]) {
         return FALSE;
      }
   }
   return TRUE;
}



static void print_dirent(DIR_ENTRY_DISK_PTR dirent)
{
   bool     skip;
   uint32_t     i;
   unsigned char   *lfn = (unsigned char *) dirent;

   skip = FALSE;

   if (dirent->ATTRIBUTE[0] == MFS_ATTR_LFN) {

      printf("\nLFN: %02d%c ",dirent->NAME[0]&0x3f, (dirent->NAME[0]&0x40)?'*':'+');
      for (i=0;i<5;i++) {
         printf("%c",lfn[i*2+1]!=0xff?lfn[i*2+1]:' ');
      }
      for (i=0;i<6;i++) {
         printf("%c",lfn[i*2+15]!=0xff?lfn[i*2+14]:' ');
      }
      for (i=0;i<2;i++) {
         printf("%c",lfn[i*2+29]!=0xff?lfn[i*2+28]:' ');
      }
   } else {
      printf("\nSFN: ");
      if (dirent->NAME[0] == 0) {
         printf(" Unused             ");
      } else if ((unsigned char)dirent->NAME[0] == 0xe5) {
         printf("Deleted             ");
      } else {
         for (i=0;i<8;i++) {
            printf("%c",(isgraph((int)dirent->NAME[i])?dirent->NAME[i]:' '));
         }
         printf(".");
         for (i=0;i<3;i++) {
            printf("%c",(dirent->TYPE[i]?dirent->TYPE[i]:' '));
         }

         printf(" %c%c%c%c%c%c ",
            (dirent->ATTRIBUTE[0] & MFS_ATTR_READ_ONLY)?'R':' ',
            (dirent->ATTRIBUTE[0] & MFS_ATTR_HIDDEN_FILE)?'H':' ',
            (dirent->ATTRIBUTE[0] & MFS_ATTR_SYSTEM_FILE)?'S':' ',
            (dirent->ATTRIBUTE[0] & MFS_ATTR_VOLUME_NAME)?'V':' ',
            (dirent->ATTRIBUTE[0] & MFS_ATTR_DIR_NAME)?'D':' ',
            (dirent->ATTRIBUTE[0] & MFS_ATTR_ARCHIVE)?'A':' ');
         printf("FClust %02x%02x%02x%02x",dirent->HFIRST_CLUSTER[1] ,dirent->HFIRST_CLUSTER[0],
            dirent->LFIRST_CLUSTER[1] ,dirent->LFIRST_CLUSTER[0] );
         printf(", Size %02x%02x%02x%02x",dirent->FILE_SIZE[3] ,dirent->FILE_SIZE[2], dirent->FILE_SIZE[1] ,dirent->FILE_SIZE[0] );
         skip = TRUE;
      }
   }

   if (!skip) {
      for (i=0;i<sizeof(DIR_ENTRY_DISK);i++) {
         printf("%02x ",(uint32_t) lfn[i]);
      }
   }
}



/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   Shell_dirent
* Returned Value   :  int32_t error code
* Comments  :  Reads from a file .
*
*END*---------------------------------------------------------------------*/

int32_t  Shell_dirent(int32_t argc, char *argv[] )
{ /* Body */
   bool           print_usage, shorthelp = FALSE;
   int32_t            return_code = SHELL_EXIT_SUCCESS;
   uint32_t           sector,num_sectors;
   int32_t            offset;
   MQX_FILE_PTR      fd, fs;
   uint32_t           e;
   unsigned char         *buffer;
   DIR_ENTRY_DISK_PTR dirents;

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if ((argc < 2) || (argc > 4)) {
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
         if (argc == 4) {
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
                  } else  if (_io_read(fd, (char *) buffer, 1) !=1) {
                     printf("Error, unable to read sector %s.\n\r", argv[1] );
                     return_code = SHELL_EXIT_ERROR;
                  } else if (!is_zero(buffer, SECTOR_SIZE)) {
                     printf("\nEntry # %d",offset+sector);
                     dirents = (DIR_ENTRY_DISK_PTR) buffer;

                     for (e=0;e<DIRENTS_PER_SECTOR;e++)  {
                        print_dirent(&dirents[e]);
                     }
                     printf("\n\r");
                  }
               }
               _mem_free(buffer);
            } else {
               printf("Error, unable to allocate sector buffer.\n\r" );
               return_code = SHELL_EXIT_ERROR;
            }
            if (argc >= 4) {
               fclose(fd);
            }
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
