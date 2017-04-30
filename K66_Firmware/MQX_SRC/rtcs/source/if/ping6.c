/*HEADER**********************************************************************
*
* Copyright 2011 Freescale Semiconductor, Inc.
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
*   This file contains the implementation of a minimal
*   ICMPv6 echo request.
*
*
*END************************************************************************/

#include <rtcs.h>

#include "rtcs_prv.h"
#include "tcpip.h"
#include "icmp6_prv.h"
#include "arp.h"

bool ICMP6_expire_echo (TCPIP_EVENT_PTR);

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : ICMP_expire_echo
* Returned Values :
* Comments  :
*     Called by the Timer.  Expire a ICMP echo request.
*
*END*-----------------------------------------------------------------*/
bool ICMP6_expire_echo(TCPIP_EVENT_PTR   event)
{
   ICMP_ECHO_PARAM_PTR         parms = event->PRIVATE;

   if (parms->NEXT) {
      parms->NEXT->PREV = parms->PREV;
   } 
   *parms->PREV = parms->NEXT;

   RTCSCMD_complete(parms, RTCSERR_ICMP_ECHO_TIMEOUT);

   return FALSE;
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : ICMP6_send_echo
* Parameters      :
*
*     _ip_address       ipaddress   [IN] destination IP address
*     uint32_t           timeout     [IN/OUT] timeout/rtt
*     uint16_t           id          [IN] identification for echo message.
*     uint16_t           seq         not used
*
* Comments        :
*     Send an ICMP Echo Request packet.
*
*END*-----------------------------------------------------------------*/
void ICMP6_send_echo(ICMP_ECHO_PARAM_PTR     parms)
{

#if RTCSCFG_ENABLE_IP6

    ICMP6_CFG_STRUCT_PTR     ICMP6_cfg_ptr = RTCS_getcfg(ICMP6);
    RTCSPCB_PTR              pcb;
    ICMP6_ECHO_HEADER_PTR    packet;
    _rtcs_if_handle          ihandle_dest = NULL; 
    uint32_t                 error;


    IF_ICMP6_STATS_ENABLED(ICMP6_cfg_ptr->STATS.COMMON.ST_TX_TOTAL++);
    parms->seq = ICMP6_cfg_ptr->ECHO_SEQ++;

    pcb = RTCSPCB_alloc_send();
    if (pcb == NULL)
    {
        IF_ICMP6_STATS_ENABLED(ICMP6_cfg_ptr->STATS.COMMON.ST_TX_MISSED++);
        RTCSCMD_complete(parms, RTCSERR_PCB_ALLOC);
        return;
    } 
        
    if(parms->ping_param->data_buffer && parms->ping_param->data_buffer_size)
    {
        error = RTCSPCB_append_fragment(pcb, parms->ping_param->data_buffer_size, parms->ping_param->data_buffer);
        if (error) 
        {
            IF_ICMP6_STATS_ENABLED(ICMP6_cfg_ptr->STATS.COMMON.ST_TX_ERRORS++);
            RTCSLOG_PCB_FREE(pcb, error);
            RTCSPCB_free(pcb);
            RTCSCMD_complete(parms, error);
            return;
        }
    }

    error = RTCSPCB_insert_header(pcb, sizeof(ICMP6_ECHO_HEADER));
    if (error)
    {
        IF_ICMP6_STATS_ENABLED(ICMP6_cfg_ptr->STATS.COMMON.ST_TX_ERRORS++);
        IF_ICMP6_STATS_ENABLED(RTCS_seterror(&ICMP6_cfg_ptr->STATS.ERR_TX, error, (uint32_t)pcb));
        RTCSLOG_PCB_FREE(pcb, error);
        RTCSPCB_free(pcb);
        RTCSCMD_complete(parms, error);
        return;
    } 

    RTCSLOG_PCB_WRITE(pcb, RTCS_LOGCTRL_PROTO(IPPROTO_ICMPV6), 0);

    packet = (ICMP6_ECHO_HEADER_PTR)RTCSPCB_DATA(pcb);
    pcb->TRANSPORT_LAYER = (unsigned char *)packet;
    mqx_htonc(packet->HEAD.TYPE, ICMP6_TYPE_ECHO_REQ);
    mqx_htonc(packet->HEAD.CODE, 0);
    (void)mqx_htons(packet->ID,  parms->ping_param->id);
    (void)mqx_htons(packet->SEQ, parms->seq);
  
    if(((sockaddr_in6 *)(&parms->ping_param->addr))->sin6_scope_id)
    {
        ihandle_dest = ip6_if_get_by_scope_id(((sockaddr_in6 *)(&parms->ping_param->addr))->sin6_scope_id);
    }

    parms->EXPIRE.TIME    = parms->ping_param->timeout;
    parms->EXPIRE.EVENT   = ICMP6_expire_echo;
    parms->EXPIRE.PRIVATE = parms;
    parms->start_time = RTCS_time_get();   /* get timestamp */

    error = ICMP6_send (pcb,  NULL, &((sockaddr_in6 *)(&parms->ping_param->addr))->sin6_addr, ihandle_dest, parms->ping_param->hop_limit);

    if (error != RTCS_OK)
    {
        IF_ICMP6_STATS_ENABLED(ICMP6_cfg_ptr->STATS.COMMON.ST_TX_MISSED++);
        RTCSCMD_complete(parms, error);
        return;
    }

    /* If there is no error add the parm struct to config struct
    ** and initiate the event timer
    */

    IF_ICMP6_STATS_ENABLED(ICMP6_cfg_ptr->STATS.ST_TX_ECHO_REQ++);

    /* add the prameter structure to ICMP  cfg */
    parms->NEXT = ICMP6_cfg_ptr->ECHO_PARAM_HEAD;
    if (parms->NEXT)
    {
      parms->NEXT->PREV = &parms->NEXT;
    }
    ICMP6_cfg_ptr->ECHO_PARAM_HEAD = parms;
    parms->PREV = &ICMP6_cfg_ptr->ECHO_PARAM_HEAD;

    TCPIP_Event_add(&parms->EXPIRE);

#else
    RTCSCMD_complete(parms, RTCSERR_IP_IS_DISABLED);
#endif /* RTCSCFG_ENABLE_IP6 */
}

