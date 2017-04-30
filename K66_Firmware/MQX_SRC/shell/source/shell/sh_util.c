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

// Here's something you've all been waiting for:  the AT&T public domain
// source for getopt(3).  It is the code which was given out at the 1985
// UNIFORUM conference in Dallas.  I obtained it by electronic mail
// directly from AT&T.  The people there assure me that it is indeed
// in the public domain.

// There is no manual page.  That is because the one they gave out at
// UNIFORUM was slightly different from the current System V Release 2
// manual page.  The difference apparently involved a note about the
// famous rules 5 and 6, recommending using white space between an option
// and its first argument, and not grouping options that have arguments.
// Getopt itself is currently lenient about both of these things White
// space is allowed, but not mandatory, and the last option in a group can
// have an argument.  That particular version of the man page evidently
// has no official existence, and my source at AT&T did not send a copy.
// The current SVR2 man page reflects the actual behavor of this getopt.
// However, I am not about to post a copy of anything licensed by AT&T.

#include <ctype.h>
#include <string.h>
#include <mqx.h>
#include <fio.h>
#include <shell.h>
#include <sh_prv.h>

#define ERR(s, c, opterr) if(opterr){\
   char errbuf[3];\
   errbuf[0] = c; errbuf[1] = '\n';errbuf[2] = '\r';\
   write(stderr, argv[0], strlen(argv[0]));\
   write(stderr, s, strlen(s));\
   write(stderr, errbuf, 3);}


#if SHELLCFG_USES_MFS
MQX_FILE_PTR Shell_default_fs = NULL;
char *Shell_default_fs_name = NULL;


static void Shell_init_current_filesystem(void *argv);
#endif //SHELLCFG_USES_MFS

void Shell_getopt_init(SHELL_GETOPT_CONTEXT* ctx)
{
    ctx->opterr = 1;
    ctx->optind = 1;
    ctx->optopt = 0;
    ctx->sp = 1;
    ctx->optarg = NULL;
}

int32_t Shell_getopt(int argc, char** argv, char* opts, SHELL_GETOPT_CONTEXT* ctx)
{
    register int c;
    register char *cp;

    if(ctx->sp == 1)
    {
        if(ctx->optind >= argc || argv[ctx->optind][0] != '-' || argv[ctx->optind][1] == '\0')
        {
           ctx->optind = 1;
           return(-1);
        }
        else if(strcmp(argv[ctx->optind], "--") == 0)
        {
            ctx->optind++;
            return(-1);
        }
    }

    ctx->optopt = c = argv[ctx->optind][ctx->sp];
    if(c == ':' || (cp=strchr(opts, c)) == NULL)
    {
        ERR(": illegal option -- ", c, ctx->opterr);
        if(argv[ctx->optind][++ctx->sp] == '\0')
        {
            ctx->optind++;
            ctx->sp = 1;
        }
        return('?');
    }
    if(*++cp == ':')
    {
        if(argv[ctx->optind][ctx->sp+1] != '\0')
        {
            ctx->optarg = &argv[ctx->optind++][ctx->sp+1];
        }
        else if(++ctx->optind >= argc)
        {
            ERR(": option requires an argument -- ", c, ctx->opterr);
            ctx->sp = 1;
            return('?');
        }
        else
        {
            ctx->optarg = argv[ctx->optind++];
        }
        ctx->sp = 1;
    }
    else
    {
        if(argv[ctx->optind][++ctx->sp] == '\0')
        {
            ctx->sp = 1;
            ctx->optind++;
        }
        ctx->optarg = NULL;
    }
    return(c);
}

int32_t Shell_parse_command_line(char *command_line_ptr, char *argv[])
{
   char             *sptr = command_line_ptr;
   uint32_t              i;
   int32_t               argc = 0;
   bool              inquotes = 0;

   for (i = 0; i < SHELL_MAX_ARGS; i++)
   {
      while (*sptr && !isgraph((unsigned char)*sptr)) sptr++;
      if (!*sptr)
      {
         argv[i] = NULL;
      }
      else
      {
         char qchar = 0;
         argc++;
         if ((*sptr == '\'') || (*sptr == '\"'))
         {
            inquotes = 1;
            qchar = *sptr;
            sptr++;
         }
         argv[i] = sptr;

         if (!inquotes)
         {
            while (*sptr && isgraph((int)*sptr)) sptr++;
         }
         else
         {
            while (*sptr && (*sptr != qchar)) sptr++;
            inquotes = 0;
         }

         if (*sptr)
         {
            *sptr++ = '\0';
         }
      }
   }
   return argc;
}



bool Shell_parse_number( char *arg, uint32_t *num_ptr)
{
   uint32_t i=0,j=0;

   if (num_ptr == NULL) return FALSE;
   while (isdigit((int)arg[i]))  {
      j = j*10 + (arg[i++]-'0');
   }
   if (arg[i]=='\0')
   {
      *num_ptr=j;
      return TRUE;
   }
   return FALSE;
}

bool Shell_parse_uint_32( char *arg, uint32_t *num_ptr)
{
   uint32_t i=0,j=0;

   if (num_ptr == NULL) return FALSE;
   while (isdigit((int)arg[i]))  {
      j = j*10 + (arg[i++]-'0');
   }
   if (arg[i]=='\0')
   {
      *num_ptr=j;
      return TRUE;
   }
   return FALSE;
}

bool Shell_parse_uint_16( char *arg, uint16_t *num_ptr)
{
   uint32_t i=0;
   uint16_t j=0;

   if (num_ptr == NULL) return FALSE;
   while (isdigit((int)arg[i]))  {
      j = j*10 + (arg[i++]-'0');
   }
   if (arg[i]=='\0')
   {
      *num_ptr=j;
      return TRUE;
   }
   return FALSE;
}

bool Shell_parse_int_32( char *arg, int32_t *num_ptr)
{
   uint32_t i=0;
   int32_t sign=1,j=0;

   if (num_ptr == NULL) return FALSE;
   if (arg[i]=='-')  {
      sign = -1;
      i++;
   }
   while (isdigit((int)arg[i]))  {
      j = j*10 + (arg[i++]-'0');
   }
   if (arg[i]=='\0')
   {
      *num_ptr=j * sign;
      return TRUE;
   }
   return FALSE;
}

bool Shell_parse_hexnum( char *arg, uint32_t *num_ptr)
{
   uint32_t i=2,j=0;

   if (num_ptr == NULL) return FALSE;

   for (i=0;i<=8;i++) {
      if (isdigit((int)arg[i])) {
         j = j*16 + (arg[i]-'0');
      } else if ((arg[i] >='a') && (arg[i] <='f') ) {
         j = j*16 + (arg[i]-'a'+10);
      } else if ((arg[i] >='A') && (arg[i] <='F') ) {
         j = j*16 + (arg[i]-'A'+10);
      } else if (arg[i]==0) {
         *num_ptr=j;
         return TRUE;
      } else {
         return FALSE;
      }
   }
   return FALSE;
}


bool Shell_check_help_request(int32_t argc, char *argv[], bool  *short_ptr )
{
   if ((argc == 3) && (strcmp(argv[1], "help") == 0))  {
      *short_ptr = (strcmp(argv[2], "short")==0)?TRUE:FALSE;
      return TRUE;
   }
   return FALSE;
}


#if SHELLCFG_USES_MFS
void Shell_create_prefixed_filename( char *new_ptr, char *in_ptr,  void *argv)
{
   _io_create_prefixed_filename(new_ptr,in_ptr,Shell_get_current_filesystem_name(argv));
}


MQX_FILE_PTR Shell_get_current_filesystem(void *argv)
{
   SHELL_CONTEXT_PTR    shell_ptr = Shell_get_context( argv );

   if(shell_ptr->CURRENT_DEVICE_FP == NULL)
   {
      // shell current filesystem is not initialized
      Shell_init_current_filesystem(argv);
      return shell_ptr->CURRENT_DEVICE_FP;
   }
   else if(!_io_is_fs_valid(shell_ptr->CURRENT_DEVICE_FP))
   {
      // current filesystem was closed
      Shell_init_current_filesystem(argv);
      return shell_ptr->CURRENT_DEVICE_FP;
   }
   else
   {
      // everything is ok, return stored filesystem ptr
      return shell_ptr->CURRENT_DEVICE_FP;
   }
}

char *Shell_get_current_filesystem_name(void *argv)
{
   SHELL_CONTEXT_PTR    shell_ptr = Shell_get_context( argv );

   if(shell_ptr->CURRENT_DEVICE_FP == NULL)
   {
      // shell current filesystem is not initialized
      Shell_init_current_filesystem(argv);
      return shell_ptr->CURRENT_DEVICE_NAME;
   }
   else if(!_io_is_fs_valid(shell_ptr->CURRENT_DEVICE_FP))
   {
      // current filesystem was closed
      Shell_init_current_filesystem(argv);
      return shell_ptr->CURRENT_DEVICE_NAME;
   }
   else
   {
      // everything is ok, return stored device name
      return shell_ptr->CURRENT_DEVICE_NAME;
   }
}

int32_t Shell_set_current_filesystem(void *argv, MQX_FILE_PTR fp)
{
   SHELL_CONTEXT_PTR    shell_ptr = Shell_get_context( argv );

   if(fp == NULL)
   {
      shell_ptr->CURRENT_DEVICE_FP       = NULL;
      shell_ptr->CURRENT_DEVICE_NAME[0]  = '\0';
      strcpy(shell_ptr->CURRENT_DIR,"\\");
   }
   else if(_io_is_fs_valid(fp))
   {
      _io_get_fs_name(fp,shell_ptr->CURRENT_DEVICE_NAME,sizeof(shell_ptr->CURRENT_DEVICE_NAME));
      shell_ptr->CURRENT_DEVICE_FP = fp;
      strcpy(shell_ptr->CURRENT_DIR,"\\");
   }
   else
   {
      return MQX_INVALID_POINTER;
   }
   return MQX_OK;
}

int32_t Shell_set_current_filesystem_by_name(void *argv, char *dev_name)
{
   MQX_FILE_PTR         temp_fs;

   temp_fs = _io_get_fs_by_name(dev_name);
   // if valid filesystem entry is returned, fill in internal variables
   if(temp_fs != NULL)
   {
      Shell_set_current_filesystem(argv,temp_fs);
   }
   else
   {
      return MQX_INVALID_POINTER;
   }
   return MQX_OK;
}


static void Shell_init_current_filesystem(void *argv)
{
   MQX_FILE_PTR         temp_fs;

   temp_fs   = _io_get_first_valid_fs();
   // if valid filesystem entry is returned, fill in internal variables
   // otherwise set default values
   Shell_set_current_filesystem(argv,temp_fs);
}
#endif //SHELLCFG_USES_MFS
/* EOF */
