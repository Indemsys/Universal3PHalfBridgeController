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
*   This file contains the interface between IP and
*   the local host.
*
*
*END************************************************************************/

#include <rtcs.h>
#include "rtcs_prv.h"
#include "tcpip.h"
#include "ip_prv.h"


/*
** The structure used by the application to bind an IP address
** to an Ethernet packet driver.
*/

static const RTCS_IF_STRUCT rtcs_local =
{
    IPLOCAL_open,
    IPLOCAL_close
#if RTCSCFG_ENABLE_IP4    
    ,
    IPLOCAL_send,
    NULL,
    NULL
#endif /* RTCSCFG_ENABLE_IP4 */

#if RTCSCFG_ENABLE_IP6
    ,
    IP6LOCAL_send,   /* SEND6 */
    NULL,            /* JOIN6 */
    NULL             /* LEAVE6 */
#endif
};

const RTCS_IF_STRUCT_PTR RTCS_IF_LOCALHOST = (RTCS_IF_STRUCT_PTR)&rtcs_local;

IP_IF_PTR   RTCS_IF_LOCALHOST_PRV = 0;  /* the local host interface */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IP_open
* Returned Value  : ICB_STRUCT_PTR
* Comments        :
*     Registers a transport protocol with IP.
*
*END*-----------------------------------------------------------------*/

ICB_STRUCT_PTR IP_open
   (
      unsigned char             protocol,   /* [IN]  Protocol to use */
      IP_SERVICE_FN     service,    /* [IN]  Packet receive function */
      void             *protocol_cfg,    /* [IN]  Owner's config block */
      uint32_t       *status      /* [OUT] return code */
   )
{ /* Body */
 
#if RTCSCFG_ENABLE_IP4  
 
   IP_CFG_STRUCT_PTR    IP_cfg_ptr;
   ICB_STRUCT_PTR icb, search_ptr;

   IP_cfg_ptr = RTCS_getcfg(IP);

   /* Make sure the protocol isn't already is use */
   RTCS_LIST_SEARCH(IP_cfg_ptr->ICB_HEAD, search_ptr) {
      if (search_ptr->PROTOCOL == protocol) {
         *status = RTCSERR_IP_PROT_OPEN;
         return NULL;
      } /* Endif */
   } /* End SEARCH */

   icb = RTCS_mem_alloc(sizeof(ICB_STRUCT));
   if (!icb) {
      *status = RTCSERR_IP_ICB_ALLOC;
      return NULL;
   } /* Endif */

   _mem_set_type(icb, MEM_TYPE_ICB);

   icb->PROTOCOL = protocol & 0xFF;
   icb->SERVICE  = service;
   icb->PRIVATE  = protocol_cfg;

   /*
   ** Add the record into the chain of open ICBs
   */
   RTCS_LIST_INS(IP_cfg_ptr->ICB_HEAD, icb);

   /*
   ** Open new ICB succeeded
   */
   *status = RTCS_OK;
   return icb;

#else

    *status = RTCSERR_IP_IS_DISABLED;
    return NULL;

#endif /* RTCSCFG_ENABLE_IP4 */

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IPLOCAL_open
*  Returned Value : RTCS_OK or error code
*  Comments       :
*        Registers the local host as an IP interface.
*
*END*-----------------------------------------------------------------*/

uint32_t IPLOCAL_open
   (
      IP_IF_PTR   if_ptr
         /* [IN] the IP interface structure */
   )
{ /* Body */
#if RTCSCFG_ENABLE_LOOPBACK && RTCSCFG_ENABLE_IP4
    uint32_t error = RTCS_OK;
#endif
    if_ptr->MTU = IP_MAX_MTU;
    if_ptr->ARP = NULL;

    if_ptr->DEV_TYPE     = 0;
    if_ptr->DEV_ADDRLEN  = 0;
#if RTCSCFG_ENABLE_SNMP && RTCSCFG_ENABLE_IP4
    if_ptr->SNMP_IF_TYPE = IPIFTYPE_LOOPBACK;
#endif


/* parms.ihandle set by IPIF_add() */ //PFI Move to IPLOCAL_open()
#if RTCSCFG_ENABLE_LOOPBACK && RTCSCFG_ENABLE_IP4
    /* Create the localhost route entry */
    error = IP_route_add_direct(INADDR_LOOPBACK, IN_CLASSA_NET, if_ptr, if_ptr);

    if (error)
    {
        return error;
    }

    #if RTCSCFG_ENABLE_IGMP 
       /* IGMP_ipif_bind failure isn't sufficient reason to abort */
       IGMP_ipif_bind(if_ptr, INADDR_LOOPBACK);
    #endif
#endif 



#if RTCSCFG_ENABLE_IP6
    IP6LOCAL_open (if_ptr);
#endif    

    return RTCS_OK;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IPLOCAL_close
*  Returned Value : RTCS_OK or error code
*  Comments       :
*        Unregisters the local host as an IP interface.
*
*END*-----------------------------------------------------------------*/

uint32_t IPLOCAL_close
   (
      IP_IF_PTR   if_ptr
         /* [IN] the IP interface structure */
   )
{ /* Body */

   return RTCS_OK;

} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IPLOCAL_send
*  Returned Value : RTCS_OK or error code
*  Comments       :
*        Sends a packet.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4  

uint32_t IPLOCAL_send
   (
      IP_IF_PTR      if_ptr,
         /* [IN] the IP interface structure */
      RTCSPCB_PTR    rtcs_pcb,
         /* [IN] the packet to send */
      _ip_address    src,
         /* [IN] the next-hop source address */
      _ip_address    dest,
         /* [IN] the next-hop destination address */
      void          *data
         /* [IN] unused */
   )
{ 
    IP_CFG_STRUCT_PTR   IP_cfg_ptr = RTCS_getcfg(IP);
    IP_HEADER_PTR       iph = (IP_HEADER_PTR)RTCSPCB_DATA(rtcs_pcb);
    uint32_t             error;

    RTCSLOG_FNE4(RTCSLOG_FN_IPLOCAL_send, if_ptr, rtcs_pcb, src);
   
    /*
    ** ST_RX_TOTAL is incremented in IP_service(), which catches
    ** all packets except those which arrived through the loopback
    ** interface.  Those are caught here.
    **
    ** Additionally, those packets that arrive by loopback are
    ** enqueued back onto the TCPIP task's message queue to
    ** prevent the task's stack requirements from getting too
    ** large (this is mainly for TCP loopback).
    */
    if (rtcs_pcb->IFSRC == RTCS_IF_LOCALHOST_PRV)
    {
        IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_RX_TOTAL++);

        if (!RTCSCMD_service(rtcs_pcb, IPLOCAL_service))
        {
            IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_RX_MISSED++);
            RTCSLOG_PCB_FREE(rtcs_pcb, RTCSERR_OUT_OF_BUFFERS);
            RTCSPCB_free(rtcs_pcb);
         
            RTCSLOG_FNX2(RTCSLOG_FN_IPLOCAL_send, RTCSERR_OUT_OF_BUFFERS);
            return RTCSERR_OUT_OF_BUFFERS;
        }

        error = RTCS_OK;
    } 
    else
    {

        /* If it is a fragmented packet, reassemble it */
        if (mqx_ntohs(iph->FRAGMENT) & (IP_FRAG_MF | IP_FRAG_MASK))
        {
        #if RTCSCFG_ENABLE_IP_REASSEMBLY
            error = IP_reasm(rtcs_pcb, &rtcs_pcb);
            IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_RX_FRAG_RECVD++);
            if (rtcs_pcb)
            {
                IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_RX_FRAG_REASMD++);
            } 
            else
            {
                if (error)
                {
                    IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_RX_FRAG_DISCARDED++);
                }
            
                RTCSLOG_FNX2(RTCSLOG_FN_IPLOCAL_send, error);
                return error;
            }
        #else
            IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_RX_FRAG_DISCARDED++);
            RTCSLOG_PCB_FREE(rtcs_pcb, RTCSERR_FEATURE_NOT_ENABLED);
            RTCSPCB_free(rtcs_pcb);
         
            RTCSLOG_FNX2(RTCSLOG_FN_IPLOCAL_send, RTCSERR_FEATURE_NOT_ENABLED);
            return RTCSERR_FEATURE_NOT_ENABLED;

        #endif
        }
    #if BSPCFG_ENET_HW_RX_PROTOCOL_CHECKSUM
        else
        {
            if( ((((IP_IF_PTR)(rtcs_pcb->IFSRC))->FEATURES & IP_IF_FEATURE_HW_RX_PROTOCOL_CHECKSUM) && 
                ( (mqx_ntohc(iph->PROTOCOL) == IPPROTO_UDP) ||
                  (mqx_ntohc(iph->PROTOCOL) == IPPROTO_TCP) ||
                  (mqx_ntohc(iph->PROTOCOL) == IPPROTO_ICMP)) )
            #if RTCSCFG_LINKOPT_8023
                && (rtcs_pcb->LINK_OPTIONS.RX.OPT_8023 == 0)
            #endif
                )
            {
                rtcs_pcb->TYPE |= RTCSPCB_TYPE_HW_PROTOCOL_CHECKSUM;
            }
        }
    #endif

        /* If not, don't wait and forward it to the upper layer */
        error = IPLOCAL_service(rtcs_pcb);
    }

    RTCSLOG_FNX2(RTCSLOG_FN_IPLOCAL_send, error);
    return error;
}

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IPLOCAL_service
*  Returned Value : RTCS_OK or error code
*  Comments       :
*        Delivers a packet to the loopback interface.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4 

uint32_t IPLOCAL_service
   (
      RTCSPCB_PTR    rtcs_pcb
         /* [IN] the packet to deliver */
   )
{ /* Body */
   IP_CFG_STRUCT_PTR    IP_cfg_ptr;
   ICB_STRUCT_PTR       search_ptr;
   unsigned char                hdrlen;
   unsigned char                protocol;
   uint32_t              error;
   
   RTCSLOG_FNE2(RTCSLOG_FN_IPLOCAL_service, rtcs_pcb);

   IP_cfg_ptr = RTCS_getcfg(IP);

   /* Find an ICB for the received packet's protocol */
   protocol = RTCSPCB_TRANSPORT_PROTL(rtcs_pcb);
   RTCS_LIST_SEARCH(IP_cfg_ptr->ICB_HEAD, search_ptr) {
      if (search_ptr->PROTOCOL == protocol) {
         break;
      } /* Endif */
   } /* End SEARCH */

   if (!search_ptr) {
      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_RX_DISCARDED++);
      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_RX_NO_PROTO++);
      #if RTCSCFG_ENABLE_ICMP
      ICMP_send_error(ICMPTYPE_DESTUNREACH, ICMPCODE_DU_PROTO_UNREACH, 0, rtcs_pcb, 0);
      #endif
      RTCSLOG_PCB_FREE(rtcs_pcb, RTCS_OK);
      RTCSPCB_free(rtcs_pcb);
      
      RTCSLOG_FNX2(RTCSLOG_FN_IPLOCAL_service, RTCS_OK);
      return RTCS_OK;
   } /* Endif */

   hdrlen = RTCSPCB_TRANSPORT_DELTA(rtcs_pcb);
   error = RTCSPCB_next(rtcs_pcb, hdrlen);
   if (error) {
      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_RX_ERRORS++);
      IF_IP_STATS_ENABLED(RTCS_seterror(&IP_cfg_ptr->STATS.ERR_RX, error, (uint32_t)rtcs_pcb));
      RTCSLOG_PCB_FREE(rtcs_pcb, error);
      RTCSPCB_free(rtcs_pcb);
      
      RTCSLOG_FNX2(RTCSLOG_FN_IPLOCAL_service, error);
      return error;
   } /* Endif */

   IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_RX_DELIVERED++);
   search_ptr->SERVICE(rtcs_pcb, search_ptr->PRIVATE);
   
   RTCSLOG_FNX2(RTCSLOG_FN_IPLOCAL_service, RTCS_OK);
   return RTCS_OK;
} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/************************************************************************
* NAME: IPLOCAL_init
*
* DESCRIPTION: Adds LOCALHOST interface.
*************************************************************************/
uint32_t IPLOCAL_init (void)
{
   uint32_t           error;
   IPIF_PARM         parms;
   
   parms.mhandle = NULL;
   parms.if_ptr  = RTCS_IF_LOCALHOST;
   error = RTCSCMD_internal(parms, IPIF_add);
   if (error) {
      return error;
   } /* Endif */
   
   RTCS_IF_LOCALHOST_PRV = parms.ihandle;
   
   return RTCS_OK;
}
