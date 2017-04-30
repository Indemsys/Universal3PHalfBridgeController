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
#include <mqx.h>
#include "shell.h"
#if SHELLCFG_USES_RTCS

#include <rtcs.h>
#include "sh_rtcs.h"

#define SHELL_CLOCK_SERVER_PRIO        (RTCSCFG_DEFAULT_RTCSTASK_PRIO+2)
#define SHELL_CLOCK_SERVER_STACK       4000
#define SHELL_CLOCK_CHILD_PRIO         (RTCSCFG_DEFAULT_RTCSTASK_PRIO+1)
#define SHELL_CLOCK_CHILD_STACK        4000

#define CREATE_WITH_RTCS 1

/*TASK*-----------------------------------------------------------------
*
* Function Name  : Clock_child_task
* Returned Value : void
* Comments       :
*
*END------------------------------------------------------------------*/

static void Clock_child_task
   (
#if CREATE_WITH_RTCS
      void    *param,
      void    *creator
#else
   uint32_t     sock
#endif
   )

{
#if CREATE_WITH_RTCS
   uint32_t     sock = (uint32_t) param;
#endif
   int32_t      size;
   char        buffer[256];
   TIME_STRUCT time;

#if CREATE_WITH_RTCS
   RTCS_task_resume_creator(creator, RTCS_OK);
#endif

   printf("Clock child: Servicing socket %x\n", sock);


   while (sock) {
      _time_get(&time);
      size = sprintf(buffer, "\r\nSeconds elapsed = %d.%03d", time.SECONDS, time.MILLISECONDS);
      size++; // account for null byte
      if (size != send(sock, buffer, size, 0)) {
         printf("\nClock child: closing socket %x\n",sock);
         shutdown(sock, FLAG_CLOSE_TX);
         sock=0;
      } else {
         _time_delay(5000);
      }
   }
   printf("Clock child: Terminating\n");

}

/*TASK*-----------------------------------------------------------------
*
* Function Name  : Clock_server_task
* Returned Value : void
* Comments       :
*
*END------------------------------------------------------------------*/

static void Clock_server_task
(
   void   *unused1,
   void   *unused2
)
{
   sockaddr_in    laddr, raddr={0};
   uint32_t        sock, listensock;
   uint32_t        error;
   uint16_t        rlen;


   /* Clock server services port 999 */
   laddr.sin_family      = AF_INET;
   laddr.sin_port        = 999;
   laddr.sin_addr.s_addr = INADDR_ANY;


   /* Listen on TCP port */
   listensock= socket(PF_INET, SOCK_STREAM, 0);
   if (listensock == RTCS_HANDLE_ERROR) {
      printf("\nCreate stream socket failed");
      _task_block();
   } 
   error = bind(listensock, (const struct sockaddr *)&laddr, sizeof(laddr));
   if (error != RTCS_OK) {
      printf("\nStream bind failed - 0x%lx", error);
      _task_block();
   } 
   error = listen(listensock, 0);
   if (error != RTCS_OK) {
      printf("\nListen failed - 0x%lx", error);
      _task_block();
   } 

   printf("\nClock Server active on port 999\n");

   for (;;) {
      /* Connection requested; accept it */
      rlen = sizeof(raddr);
      printf("Clock server: Waiting on accept\n");
      sock = accept(listensock, (struct sockaddr *)&raddr, &rlen);
      if (sock == RTCS_HANDLE_ERROR) {
         printf("\n\n*** Clock server: Accept failed, error 0x%lx *** \n\n\n",
            RTCS_geterror(listensock));
      } else {
         printf("Clock server: Connection from %ld.%ld.%ld.%ld, port %d, socket %x\n",
            (raddr.sin_addr.s_addr >> 24) & 0xFF,
            (raddr.sin_addr.s_addr >> 16) & 0xFF,
            (raddr.sin_addr.s_addr >>  8) & 0xFF,
             raddr.sin_addr.s_addr        & 0xFF,
             raddr.sin_port, sock);

         /* Create a task to look after it */
         printf("Clock server: detaching socket %x\n",sock);

         printf("Clock server: spawning child task\n");
         #if CREATE_WITH_RTCS
         RTCS_task_create("Clock_child", SHELL_CLOCK_CHILD_PRIO,
             SHELL_CLOCK_CHILD_STACK, Clock_child_task, (void *) sock);
         #else
            {
               TASK_TEMPLATE_STRUCT    task_template = {0};
               task_template.TASK_NAME          = "Clock_child";
               task_template.TASK_PRIORITY      = SHELL_CLOCK_CHILD_PRIO;
               task_template.TASK_STACKSIZE     = SHELL_CLOCK_CHILD_STACK;
               task_template.TASK_ADDRESS       = Clock_child_task;
               task_template.CREATION_PARAMETER = (uint32_t)sock;
               if (_task_create(0, 0, (uint32_t)&task_template) == MQX_NULL_TASK_ID) {
                  printf("Clock server: failed to spawn child task\n");
               } 
            }
         #endif
      } 
   }
} /* Endbody */



/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  Shell_Clock_server_start
*  Returned Value:  none
*  Comments  :  SHELL utility to start clock server
*  Usage:  Clock_server 
*
*END*-----------------------------------------------------------------*/

int32_t  Shell_Clock_server_start(int32_t argc, char *argv[] )
{
   uint32_t  result;
   bool              print_usage, shorthelp = FALSE;
   int32_t               return_code = SHELL_EXIT_SUCCESS;

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if (argc == 1)  {
         result =RTCS_task_create("Clock_server", SHELL_CLOCK_SERVER_PRIO, 
            SHELL_CLOCK_SERVER_STACK, Clock_server_task, NULL);
         if (result ==  0)  {
            printf("Clock Server Started.\n");
         } else  {
            printf("Unable to start Clock Server, error = 0x%x\n",result);
            return_code = SHELL_EXIT_ERROR;
         }
      } else  {
         printf("Error, %s invoked with incorrect number of arguments\n", argv[0]);
         print_usage = TRUE;
      }
   }
   
   if (print_usage)  {
      if (shorthelp)  {
         printf("%s \n", argv[0]);
      } else  {
         printf("Usage: %s\n",argv[0]);
      }
   }
   return return_code;
} /* Endbody */

#endif /* SHELLCFG_USES_RTCS */

/* EOF */
