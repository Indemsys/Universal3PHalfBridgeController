/*HEADER**********************************************************************
*
* Copyright 2014 Freescale Semiconductor, Inc.
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
*   This file contains command line client for RFC 862 (ECHO)
*
*
*END************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <mqx.h>
#include "shell.h"
#if SHELLCFG_USES_RTCS

#include <rtcs.h>
#if !MQX_USE_IO_OLD
#include <stdio.h>
#endif
#include <echocln.h>
#include "sh_rtcs.h" 

#if RTCSCFG_ENABLE_IP6
  #if RTCSCFG_ENABLE_IP4
  #define SHELL_ECHOCLN_AF (AF_INET | AF_INET6)
#else
  #define SHELL_ECHOCLN_AF (AF_INET6)
#endif
#else
  #define SHELL_ECHOCLN_AF (AF_INET)
#endif

#define STR1(x) #x
#define STR2(x) STR1(x)
#define SHELL_ECHOCLN_DEFAULT_SERVICE_STR STR2(IPPORT_ECHO)

static const char * const ECHOCLN_TASK_NAME = "Echo client";
static const int32_t ECHOCLN_TASK_STACK = 2000;

typedef struct echocln_parms_struct
{ 
  uint32_t  sock;
  char *    buffer;
  uint32_t  buflen;
  int32_t   loop_cnt;
} ECHOCLN_PARMS, * ECHOCLN_PARMS_PTR;

enum fsm_machine_states
{
  FSM_START = 0,
  FSM_GETADDR,
  FSM_CONNECT,
  FSM_START_ECHOCLN,
  FSM_W_START_ECHOCLN,
  FSM_EXIT
};

static void ECHOCLN_task
   (
      void   *dummy,
      void   *creator
   )
{
  int32_t i_result;
  TIME_STRUCT diff_time;
  ECHOCLN_PARMS local_parms;
  ECHOCLN_PARMS_PTR parms = (ECHOCLN_PARMS_PTR)dummy;
  
  /* get local copy of parameters */  
  memcpy(&local_parms, parms, sizeof(ECHOCLN_PARMS));
  
  /* allocate memory for the local copy of send buffer */
  local_parms.buffer = RTCS_mem_alloc_system_zero(parms->buflen);
  if(!local_parms.buffer || !parms->buffer)
  {
    RTCS_task_exit(creator, ECHOCLN_ERR_OUT_OF_MEMORY);
  }
  /* get local copy of the send buffer */
  memcpy(local_parms.buffer, parms->buffer, parms->buflen);
  
  /* resume shell task. parms are no longer valid after this. */
  RTCS_task_resume_creator(creator, RTCS_OK);
  
  /* now we send/recv all data, all count. blocks until all data is sent/received or an error occurs */
  i_result = ECHOCLN_process(local_parms.sock, local_parms.buffer, local_parms.buflen, local_parms.loop_cnt, &diff_time);
  if(RTCS_OK != i_result)
  {
    fprintf(stdout, "ECHO client error %i after run time %i.%i\n", i_result, diff_time.SECONDS, diff_time.MILLISECONDS);
  }
  else
  {
    fprintf(stdout, "exchanged %d echo packets, total time: %i.%i s\n", local_parms.loop_cnt, diff_time.SECONDS, diff_time.MILLISECONDS);
  }
  shutdown(local_parms.sock, 0);
  _mem_free(local_parms.buffer);
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  Shell_echo
*  Returned Value:  none
*  Comments  :  Echo server UDP TCP
*  Usage:  echo <server_name> [-p <protocol>] [-f <sa_family>]
*            <protocol> tcp | udp (default is TCP)
*            <sa_family> 4 | 6
*          echo 192.168.1.202
*          echo fe80::........ -p udp
*          echo localhost -p tcp -f 4
*          echo localhost -p udp -f 6
*          echo ::1 -p udp
*            <sa_family> is determined be getaddrinfo() from <server_name> if not specified on the command line
*
*END*-----------------------------------------------------------------*/
int32_t Shell_echo(int32_t argc, char *argv[])
{  
  bool print_usage;
  bool shorthelp = FALSE;
  int32_t return_code = SHELL_EXIT_ERROR;
  
  int32_t    i_result;
  uint32_t   sock = RTCS_SOCKET_ERROR;
  struct     addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
                    
  /* command line parameters: user input */
  uint16_t cmdline_sa_family = AF_UNSPEC;
  uint16_t cmdline_ai_protocol = IPPROTO_TCP;
  char *   cmdline_hostname = NULL;
  char *   cmdline_servname = SHELL_ECHOCLN_DEFAULT_SERVICE_STR;  
  char *   cmdline_payload = NULL;
  int32_t  cmdline_concat_num = 1; /* data_payload string is repeated this number times to build a large packet with repeating sequence */
  int32_t  cmdline_loop_cnt = RTCSCFG_ECHOCLN_DEFAULT_LOOPCNT; /* default number of echo loops */
  _mqx_uint cmdline_priority = (RTCSCFG_DEFAULT_RTCSTASK_PRIO+1); /* default priority for task running ECHOCLN_process() */

  /* local finite state machine */
  int32_t fsm = FSM_GETADDR;
  /* need for RTCS_task_create to pass parameters into it */
  int32_t in_out_param;
  
  /* length of the send data */
  int32_t len_send_data;
  
  int32_t i;

  print_usage = Shell_check_help_request(argc, argv, &shorthelp );
    
  cmdline_payload = RTCS_mem_alloc_system_zero(RTCSCFG_ECHOCLN_DEFAULT_BUFLEN);
  if(NULL == cmdline_payload)
  {
    fputs("Echo Client: not enough memory for the send buffer", stdout);
    goto exit;
  }
  /* default send data */
  strncpy(cmdline_payload,"hello",RTCSCFG_ECHOCLN_DEFAULT_BUFLEN);
  
  /* parse command line parameters */
  for(i=0; i<argc; i++)
  {
    switch(i)
    {
      case 0:
      break;
    
      case 1: /* IPv4 or IPv6 address of server */
        cmdline_hostname = argv[i];
      break;
      
      default:
        /* protocol */
        if(0 == strcmp(argv[i],"-p"))
        {
          if(0 == strcmp(argv[i+1],"tcp"))
          {
            cmdline_ai_protocol = IPPROTO_TCP;
          }
          else
          {
            cmdline_ai_protocol = IPPROTO_UDP;
          }
          i++; continue;
        }
        /* payload string */
        if(0 == strcmp(argv[i],"-d"))
        {
          strncpy(cmdline_payload, argv[i+1], RTCSCFG_ECHOCLN_DEFAULT_BUFLEN);
          i++; continue;
        }        
        /* number the data_payload_string will be repeated to build large packet */
        if(0 == strcmp(argv[i],"-n"))
        {
          cmdline_concat_num = (int32_t) strtol(argv[i+1], NULL, 0);
          i++; continue;
        }
        /* sa_family */
        if(0 == strcmp(argv[i],"-f"))
        {
          if(0 == strcmp(argv[i+1],"6"))
          {
            cmdline_sa_family = AF_INET6;
          }
          else
          {
            cmdline_sa_family = AF_INET;
          }
          i++; continue;
        }
        /* service name (port number) */
        if(0 == strcmp(argv[i], "-s"))
        {
          cmdline_servname = argv[i+1];
          i++; continue;
        }
        /* count = number of echo loops. one echo loop is: client_send->server_recv_send->client_recv */
        if(0 == strcmp(argv[i],"-c"))
        {          
          cmdline_loop_cnt = (int32_t) strtol(argv[i+1], NULL, 0);
          i++; continue;
        }
      break;
    } /* switch(i) */
  } /* for */
  
  if(!print_usage)  
  {
    while(fsm != FSM_EXIT)
    {
      switch(fsm)
      {
        case FSM_GETADDR:
          memset( &hints, 0, sizeof(hints) );
          hints.ai_family = cmdline_sa_family;
          if(IPPROTO_TCP == cmdline_ai_protocol)
          {
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
          }
          else
          {
            hints.ai_socktype = SOCK_DGRAM;
            hints.ai_protocol = IPPROTO_UDP;
          }
          
          /* Resolve the server address and port */
          i_result = getaddrinfo(cmdline_hostname, cmdline_servname, &hints, &result);
          if ( i_result != 0 ) 
          {
            fprintf(stdout, "getaddrinfo failed with error: %d\n", i_result);
            goto exit;
          }
          fsm = FSM_CONNECT;
        break;
        
        case FSM_CONNECT:
          /* Connect socket */
          for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) 
          {
            /* Create a SOCKET for connecting to server */
            sock = ECHOCLN_connect(ptr);
            if(sock == RTCS_SOCKET_ERROR)
            {
              continue;
            }
            break;
          }
          freeaddrinfo(result);

          if(sock == RTCS_SOCKET_ERROR) 
          {
            fprintf(stdout, "Unable to connect to server!\n");
            goto exit;
          }
          fsm = FSM_START_ECHOCLN;
        break;
        
        case FSM_START_ECHOCLN:
        {
          ECHOCLN_PARMS parms;
        
          if(cmdline_concat_num>1)
          {            
            int32_t used_init = strlen(cmdline_payload);
            int32_t used = used_init;
            while((cmdline_concat_num--) > 1)
            {        
              strncpy(cmdline_payload+used, cmdline_payload, used_init);        
              used += used_init;
              if(used>(RTCSCFG_ECHOCLN_DEFAULT_BUFLEN-used_init)) break;
            }      
          }
          len_send_data = strlen(cmdline_payload);
          fprintf(stderr, "Echo client: size of echo data %i!\n", len_send_data);
        
          /* Run client task. */
          parms.sock = sock;
          parms.buffer = cmdline_payload; /* pointer to data to send */
          parms.buflen = len_send_data;         /* size of data to send */
          parms.loop_cnt = cmdline_loop_cnt; /* no of echo loops: from client to server and back */
          in_out_param = (int32_t)&parms;    /* ECHOCLN_PARMS_PTR is the parameter for RTCS_task_create() */
          if (RTCS_task_create((char*)ECHOCLN_TASK_NAME, cmdline_priority, 
                                  ECHOCLN_TASK_STACK, ECHOCLN_task, (void*)in_out_param) 
                != RTCS_OK)
            {
              fprintf(stdout, "Unable to create ECHOCLN_task!\n");
              shutdown(sock, FLAG_ABORT_CONNECTION);
              goto exit;
            }
          fsm = FSM_W_START_ECHOCLN;
        }
        break;

        case FSM_W_START_ECHOCLN:
          return_code = SHELL_EXIT_SUCCESS;
          fsm = FSM_EXIT;          
        break;
          
        default:
        break;
      } /* switch(fsm) */
    } /* while(fsm!=FSM_EXIT) */
  }
 
  if(print_usage)  
  {
    if(shorthelp)  
    {
      fprintf(stdout, "%s <server> [-p protocol] [-d data_pattern] [-n pattern_repeat] [-f family] [-s service] [-c count]\n", argv[0]);
    }
    else
    {
      fprintf(stdout, "%s <server> [-p protocol] [-d data_pattern] [-n pattern_repeat] [-f family] [-s service] [-c count]\n",argv[0]);
      fprintf(stdout, "   -p protocol: tcp | udp. default: tcp\n");
      fprintf(stdout, "   -d byte pattern for send data. default: hello\n");
      fprintf(stdout, "   -n number of byte patterns in the send data. default: 1\n");
      fprintf(stdout, "   -f socket family IP4 or IP6: 4 | 6. default: 4.\n");
      fprintf(stdout, "   -s service. default: 7\n");
      fprintf(stdout, "   -c count of echo loops. default: 1\n");
    }
  }
exit:
  if(cmdline_payload)
  {
    _mem_free(cmdline_payload);
  }
  return return_code;
} 

#endif /* SHELLCFG_USES_RTCS */

/* EOF */
