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
*   This is the main task for the TCP/IP protocol suite.
*
*
*END************************************************************************/

#include <rtcs.h>
#include "rtcs_prv.h"
#include "ticker.h"
#include "tcpip.h"
#include "arp.h"


uint32_t (_CODE_PTR_ TCP_tick)(void);

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : TCPIP_fake_tick
* Returned Value  : 0
* Comments        :
*
*  The fake TCP_Tick_server(), used when TCP is not installed.
*
*END*-----------------------------------------------------------------*/

static uint32_t TCPIP_fake_tick
   (
      void
   )
{ /* Body */
   return 0;
} /* Endbody */

/*TASK*------------------------------------------------------------------
*
* Task Name      :   TCPIP_task
* Comments       :
*     This task services requests from the socket layer and the
*     application, and services incoming packets from the link layer(s).
*
*END*------------------------------------------------------------------*/
extern TCPIP_EVENT_PTR TCPIP_Event_head;

void TCPIP_task
   (
      void    *dummy,
      void    *creator
   )
{ /* Body */
   TCPIP_CFG_STRUCT           TCPIP_cfg;
   RTCS_DATA_PTR              RTCS_data_ptr;
   uint32_t                    i;
   TCPIP_MESSAGE_PTR          tcpip_msg;
   uint32_t                    timeout = 1, timebefore, timeafter, timedelta;
   uint32_t                    status;           /* Return status */
   _queue_id                  tcpip_qid;
   
    RTCSLOG_FNE2(RTCSLOG_FN_TCPIP_task, creator);

   RTCS_data_ptr = RTCS_get_data();
   RTCS_setcfg(TCPIP, &TCPIP_cfg);

   TCPIP_cfg.status = RTCS_OK;

   tcpip_qid = RTCS_msgq_open(TCPIP_QUEUE, 0);
   if (tcpip_qid == 0) {
      RTCS_task_exit(creator, RTCSERR_OPEN_QUEUE_FAILED);
   } /* Endif */

   RTCS_data_ptr->TCPIP_TASKID = RTCS_task_getid();

   /*
   ** Initialize the Time Service
   */
   TCP_tick = TCPIP_fake_tick;
   TCPIP_Event_init();
   timebefore = RTCS_time_get();

   /*
   ** Allocate a block of PCBs
   */
   status = RTCSPCB_init();
   if (status != RTCS_OK) {
      RTCS_task_exit(creator, status);
   } /* Endif */
    
    IP_IF_LIST    = NULL; 
    
   /*
   ** Initialize the protocols
   */
   
#if RTCSCFG_ENABLE_IP4
    /*********************************************
    *   Initialize IPv4
    **********************************************/
    status = IP_init();
    if (status)
    {
        RTCS_task_exit(creator, status);
    } 

#if RTCSCFG_ENABLE_ICMP

    status = ICMP_init();
    if (status)
    {
        RTCS_task_exit(creator, status);
    }
   
#endif /* RTCSCFG_ENABLE_ICMP */
    
    BOOT_init();

#endif /* RTCSCFG_ENABLE_IP4 */

#if RTCSCFG_ENABLE_IP6

    /*********************************************
    *   Initialize IPv6
    **********************************************/
    status = IP6_init();
    if (status)
    {
      RTCS_task_exit(creator, status);
    } 

    /* Init ICMP6. */ 
    status = ICMP6_init(); //TBD Add it to RTCS6_protocol_table
    if (status)
    {
        RTCS_task_exit(creator, status);
    } 
    
#endif /* RTCSCFG_ENABLE_IP6*/

#if (RTCSCFG_ENABLE_IP_REASSEMBLY && RTCSCFG_ENABLE_IP4) || (RTCSCFG_ENABLE_IP6_REASSEMBLY && RTCSCFG_ENABLE_IP6)
    /* Initialize the reassembler */
    status = IP_reasm_init();
    if (status)
    {
        RTCS_task_exit(creator, status);
    } 
#endif

    /* Add loopback interface.*/
    status = IPLOCAL_init ();    
    if (status)
    {
        RTCS_task_exit(creator, status);
    };

   for (i = 0; RTCS_protocol_table[i]; i++) {
      status = (*RTCS_protocol_table[i])();
      if (status) {
         RTCS_task_exit(creator, status);
      } /* Endif */
   } /* Endfor */

   _RTCS_initialized= TRUE;
   RTCS_task_resume_creator(creator, RTCS_OK);

    while (1)
    {
        TCPIP_EVENT_PTR queue = TCPIP_Event_head;
        
        tcpip_msg = (TCPIP_MESSAGE_PTR)RTCS_msgq_receive(tcpip_qid, timeout, RTCS_data_ptr->TCPIP_msg_pool);
      
        if (tcpip_msg)
        {
            if (NULL != tcpip_msg->COMMAND)
            {
                tcpip_msg->COMMAND(tcpip_msg->DATA);
            }
            RTCS_msg_free(tcpip_msg);
        }
        
        timeout = TCP_tick();
        timeafter = RTCS_time_get();
        
        /* If head changed set time delta to zero to prevent immidiate event */
        if (queue == TCPIP_Event_head)
        {
            timedelta = RTCS_timer_get_interval(timebefore, timeafter); 
        }
        else
        {
            timedelta = 0;
        }
        
        timebefore = timeafter;
        timedelta = TCPIP_Event_time(timedelta);
        
        if (timedelta != 0)
        {
            if ((timedelta < timeout) || (timeout == 0))
            {
                timeout = timedelta;
            }
        }
    }
} /* Endbody */

/* EOF */
