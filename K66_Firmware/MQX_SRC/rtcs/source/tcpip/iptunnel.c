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
*   This file contains the IP tunnel encapsulation functionnality.
*
*
*END************************************************************************/

#include <string.h>
#include <rtcs.h>
#include "rtcs_prv.h"
#include "tcpip.h"
#include "iptunnel.h"

#if RTCSCFG_ENABLE_IPIP && RTCSCFG_ENABLE_IP4 
/*
** The structure used by RTCS to bind an IP address
** to an IP over IP tunnel driver.
*/

static RTCS_IF_STRUCT rtcs_iptunnel = {
   IPIP_open,
   IPIP_close,
#if RTCSCFG_ENABLE_IP4
   IPIP_send,
   NULL,
   NULL
#endif
};

const RTCS_IF_STRUCT_PTR RTCS_IF_IPIP = &rtcs_iptunnel;


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IPIP_init
* Returned Values : RTCS_OK or error code
* Comments        :
*     Initialize the IP over IP tunnel device.
*
*END*-----------------------------------------------------------------*/

uint32_t IPIP_init
   (
      void
   )
{ /* Body */
   IPIP_CFG_STRUCT_PTR  cfg;
   uint32_t              status = RTCS_OK;
   IPIF_PARM            parms;

   cfg = RTCS_mem_alloc_zero(sizeof(IPIP_CFG_STRUCT));

   if (!cfg) {
      return RTCSERR_OUT_OF_MEMORY;
   } /* Endif */

   cfg->IPIP_PART = RTCS_part_create(sizeof(IPIP_TUNNEL), IPIP_INITIAL_COUNT,
      IPIP_GROW_COUNT, 0, NULL, NULL);

   if (!cfg->IPIP_PART) {
      return RTCSERR_CREATE_PARTITION_FAILED;
   } /* Endif */

   /* Add the tunnel device interface to RTCS */
   parms.mhandle = NULL;
   parms.if_ptr = RTCS_IF_IPIP;
   status = RTCSCMD_internal(parms, IPIF_add);
   if (!status) {
      cfg->IPIP_IF = parms.ihandle;
      cfg->IPIP_IF->MTU_FN = IPIP_MTU;
      IP_open(IPPROTO_IPIP, IPIP_service, NULL, &status);
      if (status) {
         RTCSCMD_internal(parms, IPIF_remove);
      } /* Endif */
   } /* Endif */

   if (status) {
      RTCS_part_destroy(cfg->IPIP_PART);
      _mem_free(cfg);
      return status;
   } /* Endif */

   RTCS_setcfg(IPIP, cfg);

   return RTCS_OK;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IPIP_service
* Returned Values : void
* Comments        :
*     Services IP over IP packets
*
*END*-----------------------------------------------------------------*/

void IPIP_service
   (
      RTCSPCB_PTR    pcb_ptr,          /* [IN] incoming packet */
      void          *dummy             /* [IN] not used        */
   )
{ /* Body */
   IPIP_CFG_STRUCT_PTR     cfg = RTCS_getcfg(IPIP);
   pcb_ptr->IFSRC = cfg->IPIP_IF;
   RTCSCMD_service(pcb_ptr, IP_service);
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IPIP_insert
*  Parameters     :
*
*  _ip_address    INNER_SOURCE;         [IN] Embedded header source
*  _ip_address    INNER_SOURCE_NETMASK; [IN] Embedded header source mask
*  _ip_address    INNER_DEST;           [IN] Embedded header destination
*  _ip_address    INNER_DEST_NETMASK;   [IN] Embedded header destination mask
*  _ip_address    OUTER_SOURCE;         [IN] Source for outer header
*  _ip_address    OUTER_DEST;           [IN] Destination for outer header
*  uint32_t        FLAGS;                [IN] Behaviour flags
*
*  Comments       :
*        Adds a new IP encasulation tunnel to the IP routing table and to the
*        IPTunnel configuration
*
*END*-----------------------------------------------------------------*/

void IPIP_insert
   (
      IPIP_PARM_PTR  parms
   )
{ /* Body */
   IPIP_CFG_STRUCT_PTR     cfg = RTCS_getcfg(IPIP);
   IPIP_TUNNEL_PTR         tunnel = &parms->TUNNEL;
   uint32_t                 error = RTCS_OK;

   /* Make sure IPIP is initialized */
   if (!cfg) {
      RTCSCMD_complete(parms, RTCSERR_IPIP_NOT_INITIALIZED);
      return;
   } /* Endif */

   /*
   ** Make sure the netmask is valid.  We use the fact that
   ** (x & x+1) == 0  <=>  x = 2^n-1.
   */
   if ((~tunnel->INNER_DEST_NETMASK & (~tunnel->INNER_DEST_NETMASK + 1)) ||
       (~tunnel->INNER_SOURCE_NETMASK & (~tunnel->INNER_SOURCE_NETMASK + 1)))
   {
      RTCSCMD_complete(parms, RTCSERR_IP_BIND_MASK);
      return;
   } /* Endif */

   tunnel = RTCS_part_alloc(cfg->IPIP_PART);
   if (!tunnel) {
      RTCSCMD_complete(parms, RTCSERR_OUT_OF_MEMORY);
      return;
   } /* Endif */

   tunnel->NEXT                   = cfg->NEXT;
   tunnel->INNER_SOURCE           = parms->TUNNEL.INNER_SOURCE;
   tunnel->INNER_SOURCE_NETMASK   = parms->TUNNEL.INNER_SOURCE_NETMASK;
   tunnel->INNER_DEST             = parms->TUNNEL.INNER_DEST;
   tunnel->INNER_DEST_NETMASK     = parms->TUNNEL.INNER_DEST_NETMASK;
   tunnel->OUTER_SOURCE           = parms->TUNNEL.OUTER_SOURCE;
   tunnel->OUTER_DEST             = parms->TUNNEL.OUTER_DEST;
   tunnel->FLAGS                  = parms->TUNNEL.FLAGS;

   /* Create the IPIP route entry, if neccessary */
   error = IP_route_add_virtual(
      tunnel->INNER_DEST, tunnel->INNER_DEST_NETMASK,
      INADDR_ANY,
      tunnel->INNER_SOURCE, tunnel->INNER_SOURCE_NETMASK,
      cfg->IPIP_IF, tunnel);

   if (!error) {
      cfg->NEXT = tunnel;        /* Add tunnel to tunnel list  */
      cfg->TUNNELS++;            /* Increment count of tunnels */
   } else {
      RTCS_part_free(tunnel);
   } /* Endif */

   RTCSCMD_complete(parms, error);
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IPIP_delete
*  Parameters     :
*
*  _ip_address    INNER_SOURCE;         [IN] Embedded header source
*  _ip_address    INNER_SOURCE_NETMASK; [IN] Embedded header source mask
*  _ip_address    INNER_DEST;           [IN] Embedded header destination
*  _ip_address    INNER_DEST_NETMASK;   [IN] Embedded header destination mask
*  _ip_address    OUTER_SOURCE;         [IN] Source for outer header
*  _ip_address    OUTER_DEST;           [IN] Destination for outer header
*  uint32_t        FLAGS;                [IN] Behaviour flags
*
*  Comments       :
*        Removes a IP over IP encasulation tunnel
*
*END*-----------------------------------------------------------------*/

void IPIP_delete
   (
      IPIP_PARM_PTR  parms
   )
{ /* Body */
   IPIP_CFG_STRUCT_PTR     cfg = RTCS_getcfg(IPIP);
   IPIP_TUNNEL_PTR         tunnel, * search;

   if (!cfg) {
      RTCSCMD_complete(parms, RTCS_OK);
      return;
   } /* Endif */

   /* Find the tunnel information struct */
   RTCS_LIST_SEARCHMOD(cfg->NEXT, search) {
      parms->TUNNEL.NEXT = (*search)->NEXT;
      if (memcmp(*search, &parms->TUNNEL, sizeof(IPIP_TUNNEL)) == 0) {
         /* Found it */
         break;
      } /* Endif */
   } /* EndSEARCH */

   /* If it doesn't exist we are done */
   if (!*search) {
      RTCSCMD_complete(parms, RTCS_OK);
      return;
   } /* Endif */

   /* Remove the tunnel from the routing table */
   tunnel = *search;
   IP_route_remove_virtual(
      tunnel->INNER_DEST, tunnel->INNER_DEST_NETMASK,
      INADDR_ANY,
      tunnel->INNER_SOURCE, tunnel->INNER_SOURCE_NETMASK,
      cfg->IPIP_IF);

   cfg->TUNNELS--;

   *search = tunnel->NEXT;
   RTCS_part_free(tunnel);

   RTCSCMD_complete(parms, RTCS_OK);
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IPIP_open
*  Returned Value : RTCS_OK
*  Comments       :
*        Registers the IP tunnel device as an IP interface.
*
*END*-----------------------------------------------------------------*/

uint32_t IPIP_open
   (
      /* [IN] the IP interface structure */
      IP_IF_PTR   if_ptr
   )
{ /* Body */

   if_ptr->MTU          = IP_MAX_MTU;
   if_ptr->ARP          = NULL;
   if_ptr->DEV_TYPE     = 0;
   if_ptr->DEV_ADDRLEN  = 0;

#if RTCSCFG_ENABLE_SNMP && RTCSCFG_ENABLE_IP4
   if_ptr->SNMP_IF_TYPE = IPIFTYPE_TUNNEL;
#endif

   return RTCS_OK;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IPIP_close
*  Returned Value : RTCS_OK
*  Comments       :
*        Unregisters the IP tunnel device as an IP interface.
*
*END*-----------------------------------------------------------------*/

uint32_t IPIP_close
   (
      /* [IN] the IP interface structure */
      IP_IF_PTR   if_ptr
   )
{ /* Body */
   return RTCS_OK;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IPIP_send
*  Returned Value : RTCS_OK or error code
*  Comments       :
*        Tunnels a packet (IP over IP).
*
*END*-----------------------------------------------------------------*/

uint32_t IPIP_send
   (
      /* [IN] the IP interface structure */
      IP_IF_PTR         if_ptr,
      /* [IN] the packet to send */
      RTCSPCB_PTR       pcb,
      /* [IN] the next-hop source address */
      _ip_address       src,
      /* [IN] the next-hop destination address */
      _ip_address       dest,
      /* [IN] the data pointer in the routing table */
      void             *dataptr
   )
{ /* Body */
   IP_HEADER_PTR           ip;
   _ip_address             orig_dest;
   IPIP_CFG_STRUCT_PTR     cfg = RTCS_getcfg(IPIP);
   IP_CFG_STRUCT_PTR       IP_cfg = RTCS_getcfg(IP);
   IPIP_TUNNEL_PTR         data = dataptr;

   ip = (IP_HEADER_PTR)RTCSPCB_DATA(pcb);
   orig_dest = IP_dest(pcb);

   /*
   ** IP over IP RFC requires routers to not tunnel forwarded datagrams when
   ** one of the following is true:
   **
   ** 1. The IP source address of the datagram matches the IP source address
   **    of the routers network interfaces, OR
   **
   ** 2. The IP source address of the datagram matches the IP destination
   **    address of the tunnel
   */
   if ((pcb->IFSRC != RTCS_IF_LOCALHOST_PRV && IP_is_local(NULL, src)) ||
       (src == data->OUTER_DEST))
   {
      RTCSLOG_PCB_FREE(pcb, RTCSERR_IPIP_LOOP);
      RTCSPCB_free(pcb);
      return RTCSERR_IPIP_LOOP;
   } /* Endif */

   pcb->IPIP_SOURCE = data->OUTER_SOURCE;
   pcb->IPIP_DEST = data->OUTER_DEST;

   if (!RTCSCMD_service(pcb, IPIP_send_internal)) {
      RTCSLOG_PCB_FREE(pcb, RTCSERR_OUT_OF_BUFFERS);
      RTCSPCB_free(pcb);
      return RTCSERR_OUT_OF_BUFFERS;
   } /* Endif */

   return RTCS_OK;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IPIP_send_internal
* Returned Values : void
* Comments        :
*     Sends an IP over IP packet.
*
*END*-----------------------------------------------------------------*/

void IPIP_send_internal
   (
      /* [IN] the packet to send */
      RTCSPCB_PTR      pcb
   )
{ /* Body */
   uint32_t        proto = IPPROTO_IPIP;
   uint32_t        flags = 0;
   IP_HEADER_PTR  ip    = (IP_HEADER_PTR)RTCSPCB_DATA(pcb);

   /* Copy the TOS of the inner header to the outer header */
   proto |= IPTOS(mqx_ntohc(ip->TOS));

   if (mqx_ntohl(ip->DEST) == pcb->IPIP_DEST) {
      flags |= IPROUTEOPT_NOVIRTUAL;
   } /* Endif */

   pcb->IP_SUM_PTR = NULL;

   IP_send(pcb, proto, pcb->IPIP_SOURCE, pcb->IPIP_DEST, flags);

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IPIP_MTU
*  Returned Value : the MTU
*  Comments       :
*        Finds the MTU difference for a destination IP going through a tunnel.
*     Returns the final source and destination of the last encapsulation.
*
*END*-----------------------------------------------------------------*/

uint32_t IPIP_MTU
   (
      /* [IN] pointer to a IPIP_TUNNEL structure */
      void                 *data_ptr
   )
{ /* Body */
   IPIP_TUNNEL_PTR       tunnel = data_ptr;

   /*
   ** Recursively call IP_MTU. If there is a loop in the tunnels the stack will
   ** be blown, but the fault lies with the application
   */
   return IP_MTU(tunnel->OUTER_SOURCE, tunnel->OUTER_DEST) - sizeof(IP_HEADER);

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IPIP && RTCSCFG_ENABLE_IP4 */

/* EOF */
