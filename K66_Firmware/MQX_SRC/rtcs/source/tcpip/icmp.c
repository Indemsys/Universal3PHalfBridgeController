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
*   This file contains the implementation of a minimal
*   Internet Control Message Protocol.  For more details,
*   refer to RFC792.
*
*
*END************************************************************************/

#include <rtcs.h>

#if RTCSCFG_ENABLE_ICMP && RTCSCFG_ENABLE_IP4

#include "rtcs_prv.h"
#include "tcpip.h"
#include "icmp_prv.h"
#include "tcp_prv.h"
#include "udp_prv.h"
#include "ip_prv.h"

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : ICMP_init
* Returned Values : uint32_t
* Comments        :
*     Initialize the ICMP layer.
*
*END*-----------------------------------------------------------------*/

uint32_t ICMP_init
   (
      void
   )
{ /* Body */
   ICMP_CFG_STRUCT_PTR  icmp_cfg_ptr;
   uint32_t              status = RTCS_OK;     /* return code */

   icmp_cfg_ptr = RTCS_mem_alloc_zero(sizeof(ICMP_CFG_STRUCT));
   if (icmp_cfg_ptr == NULL) {
      return RTCSERR_OUT_OF_MEMORY;
   }

   _mem_set_type(icmp_cfg_ptr, MEM_TYPE_ICMP_CFG_STRUCT);
   
   RTCS_setcfg(ICMP, icmp_cfg_ptr);

   IP_open(IPPROTO_ICMP, ICMP_service, NULL, &status);

   return status;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : ICMP_service
* Returned Values : void
* Comments        :
*     Process incoming ICMP packets.  Called from IP_service.
*
*END*-----------------------------------------------------------------*/

void ICMP_service
   (
      RTCSPCB_PTR    pcb,        /* [IN/OUT] incoming packet */
      void          *dummy       /* [IN]     not used        */
   )
{ /* Body */
   ICMP_CFG_STRUCT_PTR  ICMP_cfg_ptr;
   ICMP_HEADER_PTR      packet;
   _ip_address          source, dest;
   uint32_t              error;
   uint16_t              chksum;
   unsigned char                type;

#if BSPCFG_ENET_HW_TX_PROTOCOL_CHECKSUM
    _ip_address         if_addr;
    IP_IF_PTR           if_ptr;
#endif


   ICMP_cfg_ptr = RTCS_getcfg(ICMP);

   IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.COMMON.ST_RX_TOTAL++);
   packet = (ICMP_HEADER_PTR)RTCSPCB_DATA(pcb);
   source = IP_source(pcb);
   dest   = IP_dest(pcb);
   type = mqx_ntohc(packet->TYPE);

   /*
   ** Make sure that
   **    sizeof(ICMP_HEADER) <= RTCSPCB_SIZE(pcb)
   */
   if (RTCSPCB_SIZE(pcb) < sizeof(ICMP_HEADER)) {
      IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.COMMON.ST_RX_DISCARDED++);
      IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.ST_RX_SMALL_DGRAM++);
      RTCSLOG_PCB_FREE(pcb, RTCSERR_ICMP_BAD_HEADER);
      RTCSPCB_free(pcb);
      return;
   } /* Endif */


#if BSPCFG_ENET_HW_RX_PROTOCOL_CHECKSUM
    /* HW-offload.*/
    if( ((pcb->TYPE & RTCSPCB_TYPE_HW_PROTOCOL_CHECKSUM)==0) 
    #if RTCSCFG_LINKOPT_8023
        ||(pcb->LINK_OPTIONS.RX.OPT_8023 == 1)
    #endif
      )
#endif
    {
        /* Verify the checksum */
        if (IP_Sum_PCB(0, pcb) != 0xFFFF)
        {
            IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.COMMON.ST_RX_DISCARDED++);
            IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.ST_RX_BAD_CHECKSUM++);
            RTCSLOG_PCB_FREE(pcb, RTCSERR_ICMP_BAD_CHECKSUM);
            RTCSPCB_free(pcb);
            return;
        }
    }

   RTCSLOG_PCB_READ(pcb, RTCS_LOGCTRL_PROTO(IPPROTO_ICMP), 0);

   switch (type) {

   case ICMPTYPE_REDIRECT:
#if RTCSCFG_ENABLE_GATEWAYS

      { /* Scope */
         _ip_address          origdest, gateway;
         ICMP_ERR_HEADER_PTR  rdpacket = (ICMP_ERR_HEADER_PTR)packet;
         IPIF_PARM            parms;

         /*
         ** Make sure that
         **    sizeof(ICMP_ERR_HEADER) <= RTCSPCB_SIZE(pcb)
         */
         if (RTCSPCB_SIZE(pcb) < sizeof(ICMP_ERR_HEADER)) {
            IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.COMMON.ST_RX_DISCARDED++);
            IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.ST_RX_SMALL_DGRAM++);
            RTCSLOG_PCB_FREE(pcb, RTCSERR_ICMP_BAD_HEADER);
            RTCSPCB_free(pcb);
            return;
         } /* Endif */

         gateway  = mqx_ntohl(rdpacket->DATA);
         origdest = mqx_ntohl(rdpacket->IP.DEST);

        /* If we receive a redirect to ourselves, silently discard it.*/
        /* Ignore unasigned address.*/
        if( IP_is_local(NULL, gateway) || (gateway == INADDR_ANY))
        {
            IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.COMMON.ST_RX_DISCARDED++);
        }
        else  if(IP_is_gate(source, origdest))
        {
            parms.address = gateway;
            parms.network = origdest;
            parms.netmask = 0xFFFFFFFFL;
            parms.locmask = 0;
            RTCSCMD_internal(parms, IPIF_gate_add_redirect);
        }
        else
        {
            IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.COMMON.ST_RX_DISCARDED++);
            IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.ST_RX_RD_NOTGATE++);
        } /* Endif */
        
    } /* Endscope */
#else
            IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.COMMON.ST_RX_DISCARDED++);
#endif
      break;

    case ICMPTYPE_ECHO_REQ:
        /* RFC1122: An ICMP Echo Request destined to an IP broadcast or IP
         * multicast address MAY be silently discarded.*/
        if((dest == INADDR_BROADCAST) || (IN_MULTICAST(dest) && (ip_if_is_joined(pcb->IFSRC, dest) == false)) )
        {
            IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.COMMON.ST_RX_DISCARDED++);
        }
        else
        {
            error = RTCSPCB_next(pcb, sizeof(ICMP_HEADER));
            if (error) {
                IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.COMMON.ST_RX_ERRORS++);
                IF_ICMP_STATS_ENABLED(RTCS_seterror(&ICMP_cfg_ptr->STATS.ERR_RX, error, (uint32_t)pcb));
                RTCSLOG_PCB_FREE(pcb, error);
                RTCSPCB_free(pcb);
                return;
            } /* Endif */

            /*
            ** RTCSPCB_fork() failing isn't a serious error, so we don't check
            ** the error code
            */
            RTCSPCB_fork(pcb);

            IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.COMMON.ST_TX_TOTAL++);
            RTCSLOG_PCB_WRITE(pcb, RTCS_LOGCTRL_PROTO(IPPROTO_ICMP), 0);

            error = RTCSPCB_insert_header(pcb, sizeof(ICMP_HEADER));
            if (error) {
                IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.ST_RX_ECHO_REQ++);
                IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.COMMON.ST_TX_MISSED++);
                IF_ICMP_STATS_ENABLED(RTCS_seterror(&ICMP_cfg_ptr->STATS.ERR_TX, error, (uint32_t)pcb));
                RTCSLOG_PCB_FREE(pcb, error);
                RTCSPCB_free(pcb);
                return;
            }

            /* Change type from Echo to Echo Reply and recalculate checksum */
            packet = (ICMP_HEADER_PTR)RTCSPCB_DATA(pcb);
            mqx_htonc(packet->TYPE, ICMPTYPE_ECHO_REPLY);
            mqx_htonc(packet->CODE, 0);
            mqx_htons(packet->CHECKSUM, 0);
            pcb->IP_SUM_PTR = NULL;


    #if BSPCFG_ENET_HW_TX_PROTOCOL_CHECKSUM
            /* HW-offload.*/
            if_addr = IP_route_find(source /* Destination*/, 1);
            if_ptr = IP_find_if(if_addr);
            if( (if_ptr 
                && (if_ptr->FEATURES & IP_IF_FEATURE_HW_TX_PROTOCOL_CHECKSUM)
                && (IP_will_fragment(if_ptr, RTCSPCB_SIZE(pcb)) == FALSE))
            #if RTCSCFG_LINKOPT_8023
                && (pcb->LINK_OPTIONS.TX.OPT_8023 == 0)
            #endif
                )
            {
                pcb->TYPE |= RTCSPCB_TYPE_HW_PROTOCOL_CHECKSUM;
            }
            else
    #endif
            {
                chksum = IP_Sum_PCB(0, pcb);
                chksum = IP_Sum_invert(chksum);
                mqx_htons(packet->CHECKSUM, chksum);
                
                pcb->TYPE &= ~RTCSPCB_TYPE_HW_PROTOCOL_CHECKSUM;
            }

            if(IN_MULTICAST(dest) || IP_addr_is_broadcast(pcb, dest) )
            {
                dest = IP_get_ipif_addr(pcb->IFSRC);
            }

            /* Send the Echo Reply whence came the Echo */
            IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.ST_TX_ECHO_REPLY++);
            IP_send(pcb, IPPROTO_ICMP, dest /* Source*/, source /* Destination*/, 0);
            pcb = NULL;
        }
        break;

   case ICMPTYPE_ECHO_REPLY:
      { /* Scope */
         ICMP_ECHO_HEADER_PTR echopacket = (ICMP_ECHO_HEADER_PTR)packet;
         ICMP_ECHO_PARAM_PTR        parms;
         uint16_t              id, seq;

         /*
         ** Make sure that
         **    sizeof(ICMP_ECHO_HEADER) <= RTCSPCB_SIZE(pcb)
         */
         if (RTCSPCB_SIZE(pcb) < sizeof(ICMP_ECHO_HEADER)) {
            IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.COMMON.ST_RX_DISCARDED++);
            IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.ST_RX_SMALL_DGRAM++);
            RTCSLOG_PCB_FREE(pcb, RTCSERR_ICMP_BAD_HEADER);
            RTCSPCB_free(pcb);
            return;
         } /* Endif */

         /*
         ** get the ID and Sequence number from the packet
         */
         id  = mqx_ntohs(echopacket->ID);
         seq = mqx_ntohs(echopacket->SEQ);

         /*
         ** Find a match for the ID and sequence number
         */
         for (parms=ICMP_cfg_ptr->ECHO_PARAM_HEAD; parms; parms=parms->NEXT) {

            if ((parms->ping_param->id == id) && (parms->seq == seq)) {
               /*  received reply for the ping request */

               if (parms->NEXT) {
                  parms->NEXT->PREV = parms->PREV;
               }
               *parms->PREV = parms->NEXT;

               TCPIP_Event_cancel(&parms->EXPIRE);

               /* Calculate round trip time */
               parms->ping_param->round_trip_time = RTCS_timer_get_interval(parms->start_time, RTCS_time_get());
               
                /* IP address of echo-reply message.*/
                {
                    IP_HEADER_PTR iphead = (IP_HEADER_PTR)RTCSPCB_DATA_NETWORK(pcb);
                   
                    memset(&parms->ping_param->addr, 0, sizeof(parms->ping_param->addr));
                    parms->ping_param->addr.sa_family = AF_INET;
                    ((sockaddr_in*)(&parms->ping_param->addr))->sin_addr.s_addr = mqx_ntohl(iphead->SOURCE);
                }

               RTCSCMD_complete(parms, RTCS_OK);
               break;
            } /* Endif */
         } /* Endfor */
      } /* Endscope */
      break;

   case ICMPTYPE_DESTUNREACH:
   case ICMPTYPE_TIMEEXCEED:
      { /* Scope */
         IP_HEADER_PTR       ip;
         ICMP_ERR_HEADER_PTR icmp_err = (ICMP_ERR_HEADER_PTR)packet;
         uint32_t             len, remain;
         bool             discard = TRUE;
         unsigned char               code;

         /*
         ** Check if the attached IP header is IP over IP, and if so, strip IP
         ** headers until we find one whose source address is not local. Then we
         ** forward the ICMP error to that IP address
         */

         remain = RTCSPCB_SIZE(pcb);

         /* Make sure we have at least a full IP header */
         if (remain >= sizeof(ICMP_HEADER) + 4 + sizeof(IP_HEADER)) {
            ip = (IP_HEADER_PTR)((unsigned char *)packet + sizeof(ICMP_HEADER) + 4);
            remain -= sizeof(ICMP_HEADER) + 4;

            do {
               /* Make sure the IP header is IP over IP */
               if (mqx_ntohc(ip->PROTOCOL) != IPPROTO_IPIP) {
                  break;
               } /* Endif */

               /* Make sure we have a full IP header + 8 bytes */
               len = (mqx_ntohc(ip->VERSLEN) & 0x0F) << 2;
               if (remain < len + sizeof(IP_HEADER)) {
                  break;
               } /* Endif */

               /* Get next header */
               ip = (IP_HEADER_PTR)((unsigned char *)(ip) + len);
               remain -= len;
               source = mqx_ntohl(ip->SOURCE);

               discard = IP_is_local(NULL, source);

            } while(discard);

            len = (mqx_ntohc(ip->VERSLEN) & 0x0F) << 2;

            /*
            ** discard is true if we are the originator of the IP packet
            ** in error, or if there was not enough information to find the
            ** originator. We make sure discard is false, and there is at
            ** least a full IP header + 8 bytes of data left
            */
            if (!discard && (len + 8 <= remain)) {
               if (type == ICMPTYPE_DESTUNREACH) {
                  code = mqx_ntohc(packet->CODE);
                  switch (code) {
                  case ICMPCODE_DU_PROTO_UNREACH:
                     /*
                     ** If we are sending back to the originator, and the
                     ** originator did not use IP over IP, the protocol
                     ** unreachable error is useless.
                     */
                     code = ICMPCODE_DU_NET_UNREACH;
                     break;
                  case ICMPCODE_DU_PORT_UNREACH:
                     /* It doesn't make sense to receive this */
                     discard = TRUE;
                     break;
                  case ICMPCODE_DU_SRCROUTE:
                     discard = TRUE;
                     break;
                  } /* Endswitch */
               } else {
                  /*
                  ** Type is ICMPTYPE_TIMEEXCEED
                  **
                  ** Problem with routing loops within tunnel. Originator
                  ** does not need to know about tunnel.
                  */
                  type = ICMPTYPE_DESTUNREACH;
                  code = ICMPCODE_DU_HOST_UNREACH;
               } /* Endif */

               if (!discard) {
                  ICMP_send_error_internal(type, code, mqx_ntohl(icmp_err->DATA),
                     ip, NULL, remain);
               } /* Endif */
            } /* Endif */
         } /* Endif */
      } /* Endscope */
      break;
   } /* End Switch */

#if RTCSCFG_ENABLE_ICMP_STATS
   /* Update the statistics */
   switch (type) {
   case ICMPTYPE_DESTUNREACH: ICMP_cfg_ptr->STATS.ST_RX_DESTUNREACH++; break;
   case ICMPTYPE_TIMEEXCEED:  ICMP_cfg_ptr->STATS.ST_RX_TIMEEXCEED++;  break;
   case ICMPTYPE_PARMPROB:    ICMP_cfg_ptr->STATS.ST_RX_PARMPROB++;    break;
   case ICMPTYPE_SRCQUENCH:   ICMP_cfg_ptr->STATS.ST_RX_SRCQUENCH++;   break;
   case ICMPTYPE_REDIRECT:    ICMP_cfg_ptr->STATS.ST_RX_REDIRECT++;    break;
   case ICMPTYPE_ECHO_REQ:    ICMP_cfg_ptr->STATS.ST_RX_ECHO_REQ++;    break;
   case ICMPTYPE_ECHO_REPLY:  ICMP_cfg_ptr->STATS.ST_RX_ECHO_REPLY++;  break;
   case ICMPTYPE_TIME_REQ:    ICMP_cfg_ptr->STATS.ST_RX_TIME_REQ++;    break;
   case ICMPTYPE_TIME_REPLY:  ICMP_cfg_ptr->STATS.ST_RX_TIME_REPLY++;  break;
   case ICMPTYPE_INFO_REQ:    ICMP_cfg_ptr->STATS.ST_RX_INFO_REQ++;    break;
   case ICMPTYPE_INFO_REPLY:  ICMP_cfg_ptr->STATS.ST_RX_INFO_REPLY++;  break;
   default:                   
      ICMP_cfg_ptr->STATS.ST_RX_OTHER++;       
      ICMP_cfg_ptr->STATS.COMMON.ST_RX_DISCARDED++;
      break;
   } /* Endswitch */
#endif

   if (pcb) {
      RTCSLOG_PCB_FREE(pcb, RTCS_OK);
      RTCSPCB_free(pcb);
   } /* Endif */

} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : ICMP_send_error_internal
* Returned Values : void
* Comments        :
*     Send a ICMP error with the IP header that produced it.
*
* Warning         :
*     This packet assumes that the IP header is correct,
*     i.e. IP must not use this function on packets recorded
*     in ST_RX_HDR_ERRORS.
*
*END*-----------------------------------------------------------------*/

void ICMP_send_error_internal
   (
      uint8_t         type,    /* [IN] the type to send */
      uint8_t         code,    /* [IN] the code to send */
      uint32_t        param,   /* [IN] a parameter */
      IP_HEADER_PTR  iph,     /* [IN] the IP header */
      RTCSPCB_PTR    origpcb, /* [IN] pcb with bad packet */
      uint32_t        maxlen   /* [IN] the max data len to send, 0 = default */
   )
{ /* Body */
   ICMP_CFG_STRUCT_PTR  ICMP_cfg_ptr = RTCS_getcfg(ICMP);
   RTCSPCB_PTR          pcb;
   ICMP_ERR_HEADER_PTR  icmph;
   _ip_address          ipsrc = mqx_ntohl(iph->SOURCE);
   _ip_address          ipdst = mqx_ntohl(iph->DEST);
   uint16_t              iphdrlen = (mqx_ntohc(iph->VERSLEN) & 0x0F) << 2;
   uint16_t              ippktlen = mqx_ntohs(iph->LENGTH) - iphdrlen;
   uint16_t              checksum;
   _ip_address          icmpsrc = IP_is_local(NULL,ipdst) ? ipdst : INADDR_ANY;
   uint32_t              error;
   unsigned char            *buffer;
   uint32_t              temp;
#if RTCSCFG_ENABLE_NAT   
   TCP_HEADER_PTR       tcp_hdr;
   UDP_HEADER_PTR       udp_hdr;
   IP_HEADER_PTR        ip_hdr;
   uint32_t              protocol;
   uint16_t              src_port, dest_port;
   uint32_t (_CODE_PTR_   *nat_exec)(RTCSPCB_PTR *);
#endif 
#if BSPCFG_ENET_HW_TX_PROTOCOL_CHECKSUM
    _ip_address         if_addr;
    IP_IF_PTR           if_ptr;
#endif
   
   /*
   ** Only include up to a maximum of maxlen bytes of data from the
   ** original IP datagram
   */
   if (!maxlen) {
      maxlen = IP_DEFAULT_MTU - sizeof(IP_HEADER) - sizeof(ICMP_HEADER) - 4;
   } /* Endif */

   if (origpcb) {
      temp = RTCSPCB_DATA(origpcb) - RTCSPCB_DATA_NETWORK(origpcb);
      if (maxlen >  origpcb->HEADER_FRAG_USED + temp) {
         maxlen = origpcb->HEADER_FRAG_USED + temp;
      } /* Endif */   
   } /* Endif */

   if (ippktlen + iphdrlen > maxlen) {
      ippktlen = maxlen - iphdrlen;
   } /* Endif */

   /* Don't send an error in response to an ICMP error */
   if (mqx_ntohc(iph->PROTOCOL) == IPPROTO_ICMP) {
      /* Make sure the packet has at least a 'TYPE' field */
      if (ippktlen == 0) {
         return;
      } /* Endif */
      icmph = (ICMP_ERR_HEADER_PTR)((unsigned char *)iph + iphdrlen);
      if (!ICMPTYPE_ISQUERY(mqx_ntohc(icmph->HEAD.TYPE))) {
         return;
      } /* Endif */
   } /* Endif */

   IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.COMMON.ST_TX_TOTAL++);

   /* Allocate a PCB */
   pcb = RTCSPCB_alloc_send();
   if (pcb == NULL) {
      IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.COMMON.ST_TX_MISSED++);
      return;
   } /* Endif */

   //RTCSLOG_PCB_ALLOC(pcb);

   if (origpcb) {

      /* Add a dependency and a pointer to the ICMP data */
      RTCSPCB_depend(pcb, origpcb);
      error = RTCSPCB_append_fragment(pcb, iphdrlen + ippktlen, (unsigned char *)iph);

   } else {
      /* Reserve space for the ICMP data */
      buffer = RTCS_mem_alloc_system(iphdrlen + ippktlen);
      if (!buffer) {
         IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.COMMON.ST_TX_MISSED++);
         IF_ICMP_STATS_ENABLED(RTCS_seterror(&ICMP_cfg_ptr->STATS.ERR_TX, RTCSERR_OUT_OF_MEMORY, (uint32_t)pcb));
         RTCSLOG_PCB_FREE(pcb, RTCSERR_OUT_OF_MEMORY);
         RTCSPCB_free(pcb);
         return;
      } /* Endif */
   
      _mem_set_type(buffer, MEM_TYPE_ICMP_DATA);

      _mem_copy(iph, buffer, iphdrlen + ippktlen);
      error = RTCSPCB_append_fragment_autofree(pcb, iphdrlen + ippktlen, buffer);
      if (error) {
         _mem_free(buffer);
      } /* Endif */

   } /* Endif */

   if (!error) {
      error = RTCSPCB_insert_header(pcb, sizeof(ICMP_HEADER) + 4);
   } /* Endif */

   if (error) {
      IF_ICMP_STATS_ENABLED(ICMP_cfg_ptr->STATS.COMMON.ST_TX_MISSED++);
      IF_ICMP_STATS_ENABLED(RTCS_seterror(&ICMP_cfg_ptr->STATS.ERR_TX, error, (uint32_t)pcb));
      RTCSLOG_PCB_FREE(pcb, error);
      RTCSPCB_free(pcb);
      return;
   } /* Endif */

   RTCSLOG_PCB_WRITE(pcb, RTCS_LOGCTRL_PROTO(IPPROTO_ICMP), 0);

   /* Build the header */
   icmph = (ICMP_ERR_HEADER_PTR)RTCSPCB_DATA(pcb);
   mqx_htonc(icmph->HEAD.TYPE,     type);
   mqx_htonc(icmph->HEAD.CODE,     code);
   mqx_htons(icmph->HEAD.CHECKSUM, 0);
   mqx_htonl(icmph->DATA,          param);


#if BSPCFG_ENET_HW_TX_PROTOCOL_CHECKSUM
    /* HW-offload.*/
    if_addr = IP_route_find(ipsrc /* Destination*/, 1);
    if_ptr = IP_find_if(if_addr);
    if( (if_ptr 
        && (if_ptr->FEATURES & IP_IF_FEATURE_HW_TX_PROTOCOL_CHECKSUM)
        && (IP_will_fragment(if_ptr, RTCSPCB_SIZE(pcb)) == FALSE))
    #if RTCSCFG_LINKOPT_8023
        && (pcb->LINK_OPTIONS.TX.OPT_8023 == 0)
    #endif
        )
    {
        pcb->TYPE |= RTCSPCB_TYPE_HW_PROTOCOL_CHECKSUM;
    }
    else
#endif
    {
        checksum = IP_Sum_PCB (0, pcb);
        checksum = IP_Sum_invert(checksum);
        mqx_htons(icmph->HEAD.CHECKSUM, checksum);

        pcb->TYPE &= ~RTCSPCB_TYPE_HW_PROTOCOL_CHECKSUM;
    }

#if RTCSCFG_ENABLE_ICMP_STATS
   /* Update the statistics */
   switch (type) {
   case ICMPTYPE_DESTUNREACH: ICMP_cfg_ptr->STATS.ST_TX_DESTUNREACH++; break;
   case ICMPTYPE_TIMEEXCEED:  ICMP_cfg_ptr->STATS.ST_TX_TIMEEXCEED++;  break;
   case ICMPTYPE_PARMPROB:    ICMP_cfg_ptr->STATS.ST_TX_PARMPROB++;    break;
   case ICMPTYPE_SRCQUENCH:   ICMP_cfg_ptr->STATS.ST_TX_SRCQUENCH++;   break;
   case ICMPTYPE_REDIRECT:    ICMP_cfg_ptr->STATS.ST_TX_REDIRECT++;    break;
   case ICMPTYPE_ECHO_REQ:    ICMP_cfg_ptr->STATS.ST_TX_ECHO_REQ++;    break;
   case ICMPTYPE_ECHO_REPLY:  ICMP_cfg_ptr->STATS.ST_TX_ECHO_REPLY++;  break;
   case ICMPTYPE_TIME_REQ:    ICMP_cfg_ptr->STATS.ST_TX_TIME_REQ++;    break;
   case ICMPTYPE_TIME_REPLY:  ICMP_cfg_ptr->STATS.ST_TX_TIME_REPLY++;  break;
   case ICMPTYPE_INFO_REQ:    ICMP_cfg_ptr->STATS.ST_TX_INFO_REQ++;    break;
   case ICMPTYPE_INFO_REPLY:  ICMP_cfg_ptr->STATS.ST_TX_INFO_REPLY++;  break;
   default:                   ICMP_cfg_ptr->STATS.ST_TX_OTHER++;       break;
   } /* Endswitch */
#endif


#if RTCSCFG_ENABLE_NAT
   /* Reverse NAT (if it is installed) on the origpcb,
      otherwise the icmp error will not get sent to the
      original src */
   nat_exec = RTCS_getcfg(NAT);
   if (origpcb && nat_exec && *nat_exec) {
      // swap src and dst IPs and ports so NAT_apply
      // will process the pcb
      ip_hdr = (IP_HEADER_PTR)RTCSPCB_DATA(origpcb);
      protocol = mqx_ntohc(ip_hdr->PROTOCOL);
      // Swap ports if it is udp or tcp
      if ((protocol == IPPROTO_TCP) || (protocol == IPPROTO_UDP)) {
         switch(protocol)
         {
            case IPPROTO_TCP:
               tcp_hdr = (TCP_HEADER_PTR)((unsigned char *)ip_hdr + IPH_LEN(ip_hdr));
               dest_port = mqx_ntohs(tcp_hdr->dest_port);
               src_port  = mqx_ntohs(tcp_hdr->source_port);
               mqx_htons(tcp_hdr->dest_port, src_port);
               mqx_htons(tcp_hdr->source_port, dest_port);
               break;
            case IPPROTO_UDP:
               udp_hdr = (UDP_HEADER_PTR)((unsigned char *)ip_hdr + IPH_LEN(ip_hdr));
               dest_port = mqx_ntohs(udp_hdr->DEST_PORT);
               src_port  = mqx_ntohs(udp_hdr->SRC_PORT);
               mqx_htons(udp_hdr->DEST_PORT, src_port);
               mqx_htons(udp_hdr->SRC_PORT, dest_port);            
               break;
            default:
               // should not get here
               break;
         }
      }
      // swap IPs
      ipsrc = mqx_ntohl(ip_hdr->SOURCE);
      ipdst = mqx_ntohl(ip_hdr->DEST);
      mqx_htonl(ip_hdr->SOURCE, ipdst);
      mqx_htonl(ip_hdr->DEST,ipsrc);

      // call NAT
      error = (*nat_exec)(&origpcb);

      if (!error) {
         // swap IPs and ports back
         ip_hdr = (IP_HEADER_PTR)RTCSPCB_DATA(origpcb);
         protocol = mqx_ntohc(ip_hdr->PROTOCOL);
         // swap ports if it is udp or tcp
         if ((protocol == IPPROTO_TCP) || (protocol == IPPROTO_UDP)) {
            switch(protocol)
            {
               case IPPROTO_TCP:
                  tcp_hdr = (TCP_HEADER_PTR)((unsigned char *)ip_hdr + IPH_LEN(ip_hdr));
                  dest_port = mqx_ntohs(tcp_hdr->dest_port);
                  src_port  = mqx_ntohs(tcp_hdr->source_port);
                  mqx_htons(tcp_hdr->dest_port, src_port);
                  mqx_htons(tcp_hdr->source_port, dest_port);
                  break;
               case IPPROTO_UDP:
                  udp_hdr = (UDP_HEADER_PTR)((unsigned char *)ip_hdr + IPH_LEN(ip_hdr));
                  dest_port = mqx_ntohs(udp_hdr->DEST_PORT);
                  src_port  = mqx_ntohs(udp_hdr->SRC_PORT);
                  mqx_htons(udp_hdr->DEST_PORT, src_port);
                  mqx_htons(udp_hdr->SRC_PORT, dest_port);            
                  break;
               default:
                  // should not get here
                  break;
            }
         }
         // swap IPs
         ipsrc = mqx_ntohl(ip_hdr->SOURCE);
         ipdst = mqx_ntohl(ip_hdr->DEST);
         mqx_htonl(ip_hdr->SOURCE, ipdst);
         mqx_htonl(ip_hdr->DEST,ipsrc);   

         // Recalculate the cksum
         mqx_htons(icmph->HEAD.CHECKSUM, 0);
         checksum = IP_Sum_PCB (0, pcb);
         checksum = IP_Sum_invert(checksum);
         mqx_htons(icmph->HEAD.CHECKSUM, checksum);

         // recalculate icmpsrc, and use new ipsrc.
         ipdst = mqx_ntohl(ip_hdr->DEST);
         ipsrc = mqx_ntohl(ip_hdr->SOURCE);
         icmpsrc = IP_is_local(NULL,ipdst) ? ipdst : INADDR_ANY;
      }
   }
#endif
   
   /* Send it */
   IP_send(pcb, IPPROTO_ICMP, icmpsrc, ipsrc, 0);

} /* Endbody */



/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : ICMP_send_error
* Returned Values : void
* Comments        :
*     Send a ICMP error with the IP header that produced it.
*
* Warning         :
*     This packet assumes that the IP header is correct,
*     i.e. IP must not use this function on packets recorded
*     in ST_RX_HDR_ERRORS.
*
*END*-----------------------------------------------------------------*/

void ICMP_send_error
   (
      uint8_t         type,    /* [IN] the type to send */
      uint8_t         code,    /* [IN] the code to send */
      uint32_t        param,   /* [IN] a parameter */
      RTCSPCB_PTR    inpcb,   /* [IN] the packet which caused the error */
      int32_t         layer    /* [IN] IP layer, relative to current */
   )
{ /* Body */
   IP_HEADER_PTR  iph = (IP_HEADER_PTR)RTCSPCB_DATA_NETWORK(inpcb);

   /* Don't send an error in response to a broadcast packet */
   if (inpcb->TYPE & RTCSPCB_TYPE_BROADCAST) {
      return;
   } /* Endif */

   ICMP_send_error_internal(type, code, param, iph, inpcb, 0);

} /* Endbody */

#endif /* RTCSCFG_ENABLE_ICMP && RTCSCFG_ENABLE_IP4 */

/* EOF */
