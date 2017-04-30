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
#include <rtcs.h>

#if MQX_USE_IO_OLD
#include <fio.h>
#else
#include <stdio.h>
#include <fs_supp.h>
#define IO_ERROR -1
#endif
#include <ftpc.h>

#define FTPC_MAX_ARGS 8
extern  const FTPc_COMMAND_STRUCT FTPc_commands[];

static int32_t FTP_parse_command_line( char *command_line_ptr, char *argv[] )
{
   char             *sptr;
   uint32_t              i;
   int32_t               argc;

   sptr = command_line_ptr;
   argc = 0;
   for (i=0;i<FTPC_MAX_ARGS;i++)  {
      while (*sptr && !isgraph((unsigned char)*sptr)) sptr++;
      if (!*sptr) {
         argv[i] = NULL;
      } else {
         argc++;
         argv[i] = sptr;
         while ((*sptr != '\0') && isgraph((int) *sptr)) sptr++;
         if (*sptr) *sptr++ = '\0';
      } 
   }
   return argc;
}

_mqx_int FTP_client(char *dest)
{
   FTPc_CONTEXT_PTR  ftpc_context_ptr;
   uint32_t           i;
   _mqx_int          response;
   //char open_str[20] = {0};
   char open_str[] = "open";

   ftpc_context_ptr = _mem_alloc_zero( sizeof( FTPc_CONTEXT_STRUCT ));
   if (ftpc_context_ptr == NULL)  {
      printf("Error, unable to allocate memory for FTP client.\n");
      return IO_ERROR;
   }

   _mem_set_type(ftpc_context_ptr, MEM_TYPE_FTPc_CONTEXT_STRUCT);

   printf("\nRTCS v%d.%02d.%02d FTP client\n",
          RTCS_VERSION_MAJOR, RTCS_VERSION_MINOR, RTCS_VERSION_REV);

   if (dest) {
      ftpc_context_ptr->ARGC = 2;
      //strncpy(open_str, "open", 20);
      ftpc_context_ptr->ARGV[0] = open_str;
      //ftpc_context_ptr->ARGV[0] = "open";
      ftpc_context_ptr->ARGV[1] = dest;
   } else  {
      ftpc_context_ptr->ARGC = 0;
   }
   
   while (!ftpc_context_ptr->EXIT) {
      if (ftpc_context_ptr->ARGC > 0) {
#if MQX_USE_IO_OLD
         _io_strtolower(ftpc_context_ptr->ARGV[0]);
#else
         strtolower(ftpc_context_ptr->ARGV[0]);
#endif
         for (i=0; FTPc_commands[i].COMMAND;i++) { 
            if (strcmp(ftpc_context_ptr->ARGV[0], FTPc_commands[i].COMMAND) == 0)  {
               response = (*FTPc_commands[i].FTPc_FUNC)(ftpc_context_ptr, 
                  ftpc_context_ptr->ARGC, ftpc_context_ptr->ARGV);
               if (response == RTCS_ERROR)  {
                  printf("FTP connection closed.\n");
                  response = 0;
                  ftpc_context_ptr->HOSTADDR = 0;
                  ftpc_context_ptr->HANDLE = NULL;   
               } else  {
                   while ((response >= 100) && (response < 200))   {
                     response = FTP_receive_message(ftpc_context_ptr->HANDLE,stdout);
                   }
               }   
               break;   
            }
         } /* Endwhile */
   
         if (FTPc_commands[i].COMMAND == NULL)  {
           printf("Invalid command.  Type 'help' for a list of commands.\n");
         } /* Endif */
      } /* Endif */

      if( ftpc_context_ptr->EXIT)  {
         break;
      }
      
      if (ftpc_context_ptr->HOSTADDR)  {
         printf("ftp [%s]> ", ftpc_context_ptr->HOSTNAME);
      } else  {
         printf("ftp> ");
      }

      if (!fgets(ftpc_context_ptr->CMD_LINE, sizeof(ftpc_context_ptr->CMD_LINE), stdin)) {
         break;
      } /* Endif */

      if (strcmp(ftpc_context_ptr->CMD_LINE, "!") == 0)  {
         strncpy(ftpc_context_ptr->CMD_LINE,ftpc_context_ptr->HISTORY,sizeof(ftpc_context_ptr->CMD_LINE));
      } else  {
         strncpy(ftpc_context_ptr->HISTORY,ftpc_context_ptr->CMD_LINE,sizeof(ftpc_context_ptr->HISTORY));
      }

      ftpc_context_ptr->ARGC = FTP_parse_command_line(ftpc_context_ptr->CMD_LINE, &ftpc_context_ptr->ARGV[0] );

   } /* Endwhile */
   printf("FTP terminated\n");
   _mem_free(ftpc_context_ptr);

   return MQX_OK;
}
