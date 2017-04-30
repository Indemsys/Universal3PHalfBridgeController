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
#include <sh_mfs.h>

#if SHELLCFG_USES_MFS
#include <mfs.h>


#define SECTOR_SIZE MFS_DEFAULT_SECTOR_SIZE

typedef struct
{
   unsigned char    JMP[3];
   unsigned char    OEMNAME[8];
   unsigned char    BPB_SECTOR_SIZE[2];
   unsigned char    SECTORS_PER_CLUSTER[1];
   unsigned char    RESERVED_SECTORS[2];
   unsigned char    NUMBER_OF_FAT[1];
   unsigned char    ROOT_ENTRIES[2];
   unsigned char    NUMBER_SECTORS[2];
   unsigned char    MEDIA_TYPE[1];
   unsigned char    SECTORS_PER_FAT[2];
   unsigned char    SECTORS_PER_TRACK[2];
   unsigned char    NUM_HEADS[2];
   unsigned char    HIDDEN_SECTORS[4];
   unsigned char    MEGA_SECTORS[4];

   unsigned char    FAT_SIZE[4];
   unsigned char    EXT_FLAGS[2];
   unsigned char    FS_VER[2];
   unsigned char    ROOT_CLUSTER[4];
   unsigned char    FS_INFO[2];
   unsigned char    BK_BOOT_SEC[2];
   unsigned char    RESERVED[12];
   unsigned char    DRVNUM[1];
   unsigned char    RESERVED1[1];
   unsigned char    BOOTSIG[1];
   unsigned char    VOLID[4];
   unsigned char    VOLLAB[11];
   unsigned char    FILSYSTYPE[8];
} BIOS_PARAM_STRUCT_DISK, * BIOS_PARAM_STRUCT_DISK_PTR;

/* The file system info struct. For FAT32 only */
typedef struct filesystem_info_disk  {
   unsigned char    LEAD_SIG[4];
   unsigned char    RESERVED1[480];
   unsigned char    STRUCT_SIG[4];
   unsigned char    FREE_COUNT[4];
   unsigned char    NEXT_FREE[4];
   unsigned char    RESERVED2[12];
   unsigned char    TRAIL_SIG[4];
} FILESYSTEM_INFO_DISK, * FILESYSTEM_INFO_DISK_PTR;



static uint32_t print_bpb(void *bpb) {
   BIOS_PARAM_STRUCT_DISK_PTR bpb_ptr = (BIOS_PARAM_STRUCT_DISK_PTR) bpb;
   uint32_t i,backup=0;


   printf("OEMNAME            = ");
   for(i=0;i<8;i++) printf("%c", bpb_ptr->OEMNAME[i]);  printf("\n\r");
   printf("SECTOR_SIZE        = %02x%02x\n\r",bpb_ptr->BPB_SECTOR_SIZE[1] ,bpb_ptr->BPB_SECTOR_SIZE[0] );
   printf("SECTORS_PER_CLUSTER= %02x%\n\r",(uint32_t) bpb_ptr->SECTORS_PER_CLUSTER[0]  );
   printf("RESERVED_SECTORS   = %02x%02x\n\r",bpb_ptr->RESERVED_SECTORS[1] ,bpb_ptr->RESERVED_SECTORS[0] );
   printf("NUMBER_OF_FAT      = %02x\n\r",(uint32_t) bpb_ptr->NUMBER_OF_FAT[0]  );
   printf("ROOT_ENTRIES       = %02x%02x\n\r",bpb_ptr->ROOT_ENTRIES[1] ,bpb_ptr->ROOT_ENTRIES[0] );
   printf("NUMBER_SECTORS     = %02x%02x\n\r",bpb_ptr->NUMBER_SECTORS[1] ,bpb_ptr->NUMBER_SECTORS[0] );
   printf("MEDIA_TYPE         = %02x\n\r",(uint32_t) bpb_ptr->MEDIA_TYPE[0] );
   printf("SECTORS_PER_FAT    = %02x%02x\n\r",bpb_ptr->SECTORS_PER_FAT[1] ,bpb_ptr->SECTORS_PER_FAT[0] );
   printf("SECTORS_PER_TRACK  = %02x%02x\n\r",bpb_ptr->SECTORS_PER_TRACK[1] ,bpb_ptr->SECTORS_PER_TRACK[0] );
   printf("NUM_HEADS          = %02x%02x\n\r",bpb_ptr->NUM_HEADS[1] ,bpb_ptr->NUM_HEADS[0] );
   printf("HIDDEN_SECTORS     = %02x%02x%02x%02x\n\r",bpb_ptr->HIDDEN_SECTORS[3] ,bpb_ptr->HIDDEN_SECTORS[2],bpb_ptr->HIDDEN_SECTORS[1] ,bpb_ptr->HIDDEN_SECTORS[0] );
   printf("MEGA_SECTORS       = %02x%02x%02x%02x\n\r",bpb_ptr->MEGA_SECTORS[3] ,bpb_ptr->MEGA_SECTORS[2],bpb_ptr->MEGA_SECTORS[1] ,bpb_ptr->MEGA_SECTORS[0] );

   if ( bpb_ptr->SECTORS_PER_FAT[1]|| bpb_ptr->SECTORS_PER_FAT[0] ) {
      // FAT 12/16

   } else {

      // FAT 32
      printf("FAT_SIZE           = %02x%02x%02x%02x\n\r",bpb_ptr->FAT_SIZE[3] ,bpb_ptr->FAT_SIZE[2],bpb_ptr->FAT_SIZE[1] ,bpb_ptr->FAT_SIZE[0] );
      printf("EXT_FLAGS          = %02x%02x\n\r",bpb_ptr->EXT_FLAGS[1] ,bpb_ptr->EXT_FLAGS[0] );
      printf("FS_VER             = %02x%02x\n\r",bpb_ptr->FS_VER[1] ,bpb_ptr->FS_VER[0] );

      printf("ROOT_CLUSTER       = %02x%02x%02x%02x\n\r",bpb_ptr->ROOT_CLUSTER[3] ,bpb_ptr->ROOT_CLUSTER[2],bpb_ptr->ROOT_CLUSTER[1] ,bpb_ptr->ROOT_CLUSTER[0] );
      printf("FS_INFO            = %02x%02x\n\r",bpb_ptr->FS_INFO[1] ,bpb_ptr->FS_INFO[0] );
      printf("BK_BOOT_SEC        = %02x%02x\n\r",bpb_ptr->BK_BOOT_SEC[1] ,bpb_ptr->BK_BOOT_SEC[0] );

      backup = (bpb_ptr->BK_BOOT_SEC[1] << 8) | bpb_ptr->BK_BOOT_SEC[0];
      printf("DRVNUM             = %02x\n\r",(uint32_t) bpb_ptr->DRVNUM[0] );
      printf("BOOTSIG            = %02x\n\r",(uint32_t) bpb_ptr->BOOTSIG[0] );
      printf("VOLID              = %02x%02x%02x%02x\n\r",bpb_ptr->VOLID[3] ,bpb_ptr->VOLID[2],bpb_ptr->VOLID[1] ,bpb_ptr->VOLID[0] );
      printf("VOLLAB             = ");
      for(i=0;i<11;i++) printf("%c", bpb_ptr->VOLLAB[i]);  printf("\n\r");
      printf("FILSYSTYPE         = ");
      for(i=0;i<8;i++) printf("%c", bpb_ptr->FILSYSTYPE[i]);  printf("\n\r");

   }
   return backup;
}


static void print_fsi(void *bpb)
{
   FILESYSTEM_INFO_DISK_PTR bpb_ptr = (FILESYSTEM_INFO_DISK_PTR) bpb;

   printf("LEAD_SIG   = %02x%02x%02x%02x\n\r",bpb_ptr->LEAD_SIG[3],  bpb_ptr->LEAD_SIG[2],  bpb_ptr->LEAD_SIG[1],  bpb_ptr->LEAD_SIG[0] );
   printf("STRUCT_SIG = %02x%02x%02x%02x\n\r",bpb_ptr->STRUCT_SIG[3],bpb_ptr->STRUCT_SIG[2],bpb_ptr->STRUCT_SIG[1],bpb_ptr->STRUCT_SIG[0] );
   printf("FREE_COUNT = %02x%02x%02x%02x\n\r",bpb_ptr->FREE_COUNT[3],bpb_ptr->FREE_COUNT[2],bpb_ptr->FREE_COUNT[1],bpb_ptr->FREE_COUNT[0] );
   printf("NEXT_FREE  = %02x%02x%02x%02x\n\r",bpb_ptr->NEXT_FREE[3], bpb_ptr->NEXT_FREE[2], bpb_ptr->NEXT_FREE[1], bpb_ptr->NEXT_FREE[0] );
   printf("TRAIL_SIG  = %02x%02x%02x%02x\n\r",bpb_ptr->TRAIL_SIG[3], bpb_ptr->TRAIL_SIG[2], bpb_ptr->TRAIL_SIG[1], bpb_ptr->TRAIL_SIG[0] );
}


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   Shell_di
* Returned Value   :  int32_t error code
* Comments  :  Reads from a file .
*
*END*---------------------------------------------------------------------*/

int32_t  Shell_di(int32_t argc, char *argv[] )
{ /* Body */
   bool           print_usage, shorthelp = FALSE;
   int32_t            return_code = SHELL_EXIT_SUCCESS;
   int32_t            offset;

   MQX_FILE_PTR      fd, fs;

   uint32_t           backup=0;
   unsigned char         *buffer=NULL;

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if ((argc < 2 ) || (argc > 3)) {
         printf("Invalid number of parameters\n\r");
         return_code = SHELL_EXIT_ERROR;
         print_usage=TRUE;
      } else if ( !Shell_parse_uint_32(argv[1], (uint32_t *) &offset ))  {
         printf("Error, invalid length\n\r");
         return_code = SHELL_EXIT_ERROR;
         print_usage=TRUE;
      } else {
         buffer = _mem_alloc(SECTOR_SIZE);
         if (buffer==NULL) {
            printf("Error, unable to allocate sector buffer\n\r");
            return  SHELL_EXIT_ERROR;
         }

         if (argc==3) {
            fd = fopen(argv[2], "b");
         } else {
            fs = Shell_get_current_filesystem(argv);
            _io_ioctl(fs, IO_IOCTL_GET_DEVICE_HANDLE, &fd);
         }


         if (fd)  {
            if (fseek(fd, offset, IO_SEEK_SET) == IO_ERROR)  {
               printf("Error, unable to seek to sector %s.\n\r", argv[1] );
               return_code = SHELL_EXIT_ERROR;
            } else if (_io_read(fd, (char *) buffer, 1) != 1) {
               printf("Error, unable to read sector %s.\n\r", argv[1] );
               return_code = SHELL_EXIT_ERROR;
            }

            if (return_code == SHELL_EXIT_SUCCESS) {
               printf("\n\r");
               backup = print_bpb(buffer);
               if (backup) {
                  if (fseek(fd, backup, IO_SEEK_SET) == IO_ERROR)  {
                     printf("Error, unable to seek to sector %s.\n\r", argv[1] );
                     return_code = SHELL_EXIT_ERROR;
                  } else if (_io_read(fd, (char *) buffer, 1) != 1) {
                     printf("Error, unable to read sector %s.\n\r", argv[1] );
                     return_code = SHELL_EXIT_ERROR;
                  }
                  if (return_code == SHELL_EXIT_SUCCESS) {
                     printf("\n\r");
                     print_bpb(buffer);
                  }
               }
            }
            if (fseek(fd, 1, IO_SEEK_SET) == IO_ERROR)  {
               printf("Error, unable to seek to sector %s.\n\r", argv[1] );
               return_code = SHELL_EXIT_ERROR;
            } else if (_io_read(fd, (char *) buffer, 1) != 1) {
               printf("Error, unable to read sector %s.\n\r", argv[1] );
               return_code = SHELL_EXIT_ERROR;
            }
            if (return_code == SHELL_EXIT_SUCCESS) {
               printf("\n\r");
               print_fsi(buffer);
            }
            if (argc==3) {
               fclose(fd);
            }
         }
         _mem_free(buffer);
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

#endif //SHELLCFG_USES_MFS
