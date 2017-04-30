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
*   This file contains the implementation of getsockopt()
*   and setsockopt() at the SOL_IGMP level.
*
*
*END************************************************************************/

#include <rtcs.h>
#include "rtcs_prv.h"
#include "socket.h"
#include "tcpip.h"
#include "ip_prv.h"
#include "udp_prv.h"

#if RTCSCFG_ENABLE_IGMP && RTCSCFG_ENABLE_IP4

#define RTCS_ENTER(f,a)

#define RTCS_EXIT(f,a)  return a


uint32_t SOL_IGMP_getsockopt  (uint32_t, uint32_t, uint32_t, void *, uint32_t *);
uint32_t SOL_IGMP_setsockopt  (uint32_t, uint32_t, uint32_t, const void *, uint32_t);

extern uint32_t IGMP_leave_if_local (IP_IF_PTR, MC_MEMBER_PTR *);

const RTCS_SOCKOPT_CALL_STRUCT SOL_IGMP_CALL = {
   SOL_IGMP_getsockopt,
   SOL_IGMP_setsockopt
};

static void IGMP_getopt (IGMP_PARM_GET_PTR);
static void IGMP_setopt (IGMP_PARM_SET_PTR);

       uint32_t IGMP_join_if          (const ip_mreq *);
       uint32_t IGMP_leave_if         (const ip_mreq *);
       uint32_t IGMP_leave_if_all     (MC_MEMBER_PTR *);
static uint32_t IGMP_join_socket      (MC_MEMBER_PTR *, const ip_mreq *, uint32_t (_CODE_PTR_ *)(MC_MEMBER_PTR *));
static uint32_t IGMP_leave_socket     (MC_MEMBER_PTR *, const ip_mreq *, uint32_t (_CODE_PTR_ *)(MC_MEMBER_PTR *));
static uint32_t IGMP_leave_socket_all (ip_mreq *);

uint32_t IGMP_get_membership (IGMP_PARM_GET_PTR);
uint32_t IGMP_ipif_unbind    (IP_IF_PTR, _ip_address);


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOL_IGMP_getsockopt
* Returned Value  : error code
* Comments  :  Obtain the current value for a socket option.
*
*END*-----------------------------------------------------------------*/

uint32_t  SOL_IGMP_getsockopt
   (
      uint32_t        sock,
         /* [IN] socket handle */
      uint32_t        level,
         /* [IN] protocol level for the option */
      uint32_t        optname,
         /* [IN] name of the option */
      void          *optval,
         /* [IN] buffer for the current value of the option */
      uint32_t   *optlen
         /* [IN/OUT] length of the option value, in bytes */
   )
{ /* Body */
   IGMP_PARM_GET   parms;
   uint32_t     error;

   RTCS_ENTER(GETSOCKOPT, sock);

   parms.sock   = (SOCKET_STRUCT_PTR)sock;
   parms.option = optname;
   parms.option_value = optval;
   parms.optlen = *optlen;
   error = RTCSCMD_issue(parms, IGMP_getopt);
   if (error) {
      RTCS_EXIT(GETSOCKOPT, error);
   } /* Endif */

   *optlen = parms.optlen;
   RTCS_EXIT(GETSOCKOPT, RTCS_OK);

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOL_IGMP_setsockopt
* Returned Value  : error code
* Comments  :  Modify the current value for a socket option.
*
*END*-----------------------------------------------------------------*/

uint32_t  SOL_IGMP_setsockopt
   (
      uint32_t        sock,
         /* [IN] socket handle */
      uint32_t        level,
         /* [IN] protocol level for the option */
      uint32_t        optname,
         /* [IN] name of the option */
      const void          *optval,
         /* [IN] new value for the option */
      uint32_t        optlen
         /* [IN] length of the option value, in bytes */
   )
{ /* Body */
   IGMP_PARM_SET   parms;
   uint32_t     error;

   RTCS_ENTER(SETSOCKOPT, sock);

   parms.sock   = (SOCKET_STRUCT_PTR)sock;
   parms.option = optname;
   parms.option_value = optval;
   parms.optlen = optlen;
   error = RTCSCMD_issue(parms, IGMP_setopt);
   if (error) {
      RTCS_EXIT(SETSOCKOPT, error);
   } /* Endif */

   RTCS_EXIT(SETSOCKOPT, RTCS_OK);

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IGMP_getopt
* Parameters      :
*
*     SOCKET_STRUCT_PTR sock        [IN] socket
*     uint32_t           option      [IN] option
*     pointer           optptr      [OUT] option value
*     uint32_t           optlen      [IN/OUT] size of optptr
*
* Comments        :
*     Retrieves the value of an IGMP option.
*
*END*-----------------------------------------------------------------*/

static void IGMP_getopt
   (
      IGMP_PARM_GET_PTR     parms
   )
{ /* Body */
   uint32_t     error = RTCS_OK;

   switch (parms->option) {

   case RTCS_SO_IGMP_GET_MEMBERSHIP:
      error = IGMP_get_membership(parms);
      break;

   default:
      error = RTCSERR_SOCK_INVALID_OPTION;
   } /* Endswitch */

   RTCSCMD_complete(parms, error);

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IGMP_setopt
* Parameters      :
*
*     SOCKET_STRUCT_PTR sock        [IN] socket
*     uint32_t           option      [IN] option
*     pointer           optptr      [IN] option value
*     uint32_t           optlen      [IN] size of optptr
*
* Comments        :
*     Sets the value of an IGMP option.
*
*END*-----------------------------------------------------------------*/

static void IGMP_setopt
   (
      IGMP_PARM_SET_PTR     parms
   )
{ /* Body */
   uint32_t     error = RTCS_OK;

   if (parms->optlen < sizeof(ip_mreq)) {
      RTCSCMD_complete(parms, RTCSERR_SOCK_SHORT_OPTION);
      return;
   } /* Endif */

   switch (parms->option) {

   case RTCS_SO_IGMP_ADD_MEMBERSHIP:
      error = IGMP_join_group(parms->sock->MCB_PTR, parms->option_value, parms->sock->IGMP_LEAVEALL);
      break;

   case RTCS_SO_IGMP_DROP_MEMBERSHIP:
      error = IGMP_leave_group(parms->sock->MCB_PTR, parms->option_value, parms->sock->IGMP_LEAVEALL);
      break;

   default:
      error = RTCSERR_SOCK_INVALID_OPTION;
   } /* Endswitch */

   RTCSCMD_complete(parms, error);

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IGMP_join_if
* Returned Values : error code
* Comments        :
*     join the group on a given interface
*
*END*-----------------------------------------------------------------*/

uint32_t IGMP_join_if
   (
      const ip_mreq  *igrp  /* [IN] the group description */
   )
{ /* Body */
   MC_MEMBER_PTR       *pgroup;
   IP_IF_PTR            ipif;
   uint32_t              status;

   /* get the interface pointer with the ip */
   ipif = IP_find_if(igrp->imr_interface.s_addr);
   if (!ipif) {
      return RTCSERR_IGMP_INVALID_IP;
   } /* Endif */

   /* test if the group is already joined */
   pgroup = IGMP_member_find(&ipif->IGMP_MEMBER, igrp);
   if (pgroup) {
      (*pgroup)->UCOUNT++;
      return RTCS_OK;
   } /* Endif */

   /* if it isn't already joined, join it */
   pgroup = IGMP_member_create(&ipif->IGMP_MEMBER, igrp);
   if (!pgroup) {
      return RTCSERR_IGMP_GROUP_ALLOC;
   } /* Endif */

   /* join the group on the physical interface */
   status = IGMP_filter_add(ipif, igrp->imr_multiaddr.s_addr);
   if (status != RTCS_OK) {
      IGMP_member_delete(pgroup);
      return status;
   } /* Endif */

   /*
   ** a local group is not routed, by definition. so we don't advertise it.
   */
   /* send the unsolicited report for nonlocal groups */
   if (!IN_LOCAL_MULTICAST(igrp->imr_multiaddr.s_addr)) {
      IGMP_init_timer(*pgroup);
      IGMP_launch_timer(*pgroup, 0);
   } /* Endif */

   ipif->IGMP_UNBIND = IGMP_ipif_unbind;
   return RTCS_OK;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IGMP_leave_if
* Returned Values : error code
* Comments        :
*     leave the group on a given interface
*
*END*-----------------------------------------------------------------*/

uint32_t IGMP_leave_if
   (
      const ip_mreq  *igrp  /* [IN] the group description */
   )
{ /* Body */
   MC_MEMBER_PTR       *pmember;
   IP_IF_PTR            ipif;
   uint32_t              status;
#ifdef IGMP_V2
   bool              last_reporter;
#endif

   /* get the interface pointer of the interface */
   ipif = IP_find_if(igrp->imr_interface.s_addr);
   if (!ipif) {
      return RTCSERR_IGMP_INVALID_IP;
   } /* Endif */

   pmember = IGMP_member_find(&ipif->IGMP_MEMBER, igrp);
   if (!pmember) {
      return RTCSERR_IGMP_NOT_JOINED;
   } /* Endif */
   (*pmember)->UCOUNT--;
   if ((*pmember)->UCOUNT) {
      return RTCS_OK;
   } /* Endif */

#ifdef IGMP_V2
   last_reporter = (*pmember)->LAST_REPORTER;
#endif

   /* stop the timer if required */
   IGMP_stop_timer(*pmember);

   /* leave the group */
   status = IGMP_member_delete(pmember);

   /* leave the group on the physical interface */
   status = IGMP_filter_rm(ipif, igrp->imr_multiaddr.s_addr);
   if (status != RTCS_OK) {
      return status;
   } /* Endif */

#ifdef IGMP_V2
   /* send the leave report for the nonlocal group */
   if (last_reporter && !IN_LOCAL_MULTICAST(igrp->imr_multiaddr.s_addr)) {
   /* Start CR 2091 */
      IGMP_send_report(igrp, IGMPTYPE_LEAVE);
   /* End CR 2091 */   
   } /* Endif */
#endif

   return RTCS_OK;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IGMP_join_socket
* Returned Values : error code
* Comments        :
*     join the group on a given socket
*
*END*-----------------------------------------------------------------*/

static uint32_t IGMP_join_socket
   (
      MC_MEMBER_PTR         *phead,   /* [IN] head of group list */
      const ip_mreq               *igrp,
      uint32_t (_CODE_PTR_   *pfn)(MC_MEMBER_PTR *)
   )
{ /* Body */
   MC_MEMBER_PTR       *pgroup;

   /* test if the group is already joined */
   pgroup = IGMP_member_find(phead, igrp);
   if (pgroup) {
      (*pgroup)->UCOUNT++;
      return RTCS_OK;
   } /* Endif */

   /* if it isnt already joined, join it */
   pgroup = IGMP_member_create(phead, igrp);
   if (!pgroup) {
      return RTCSERR_IGMP_GROUP_ALLOC;
   } /* Endif */

   *pfn = IGMP_leave_if_all;
   return RTCS_OK;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IGMP_leave_socket
* Returned Values : error code
* Comments        :
*     leave the group on a given socket
*
*END*-----------------------------------------------------------------*/

static uint32_t IGMP_leave_socket
   (
      MC_MEMBER_PTR         *phead,   /* [IN] head of group list */
      const ip_mreq               *igrp,
      uint32_t (_CODE_PTR_   *pfn)(MC_MEMBER_PTR *)
   )
{ /* Body */
   MC_MEMBER_PTR       *pmember;
   uint32_t              status;

   /* test if the group is joined */
   pmember = IGMP_member_find(phead, igrp);
   if (!pmember) {
      return RTCSERR_IGMP_NOT_JOINED;
   } /* Endif */
   (*pmember)->UCOUNT--;
   if ((*pmember)->UCOUNT) {
      return RTCS_OK;
   } /* Endif */

   status = IGMP_member_delete(pmember);

   if (!*phead) {
      *pfn = NULL;
   } /* Endif */
   return status;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IGMP_join_group
* Returned Values : error code
* Comments        :
*     Join a given group
*
*END*-----------------------------------------------------------------*/

uint32_t IGMP_join_group
   (
      MC_MEMBER_PTR         *phead,   /* [IN] head of group list */
      const ip_mreq               *igrp,    /* [IN] group decriptor */
      uint32_t (_CODE_PTR_   *pfn)(MC_MEMBER_PTR *)
   )
{ /* Body */
   uint32_t  status;

   /* add the group in the socket list */
   status = IGMP_join_socket(phead, igrp, pfn);
   if (status != RTCS_OK) {
      return status;
   } /* Endif */

   /* join the group on the interface */
   status = IGMP_join_if(igrp);
   if (status != RTCS_OK) {
      /* leave the group on the socket */
      IGMP_leave_socket(phead, igrp, pfn);
      return status;
   } /* Endif */

   return RTCS_OK;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IGMP_leave_group
* Returned Values : error code
* Comments        :
*     Leave a given group
*
*END*-----------------------------------------------------------------*/

uint32_t IGMP_leave_group
   (
      MC_MEMBER_PTR         *phead,   /* [IN] head of group list */
      const ip_mreq               *igrp,    /* [IN] group decriptor */
      uint32_t (_CODE_PTR_   *pfn)(MC_MEMBER_PTR *)
   )
{ /* Body */
   uint32_t  status, tmp;

   /* leave the group in the socket list */
   status = IGMP_leave_socket(phead, igrp, pfn);

   /* leave the group on the interface */
   tmp = IGMP_leave_if(igrp);
   if (tmp != RTCS_OK && status == RTCS_OK) {
      status = tmp;
   } /* Endif */

   return status;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IGMP_get_membership
* Returned Values : error code
* Comments        :
*     Get the list of all group memberships of a given socket
*
*END*-----------------------------------------------------------------*/

uint32_t IGMP_get_membership
   (
      IGMP_PARM_GET_PTR     parms
   )
{ /* Body */
   ip_mreq       *igrp = parms->option_value;
   uint32_t        i, j;
   MC_MEMBER_PTR  group;

   i = parms->optlen;
   j = 0;

   if (parms->sock->MCB_PTR) {
      for (group = *parms->sock->MCB_PTR;
           group;
           group = group->NEXT, j += sizeof(ip_mreq)) {

         if (i >= sizeof(ip_mreq)) {
            *igrp = group->IGRP;
            igrp++;
            i -= sizeof(ip_mreq);
         } else if (i > 0) {
            ip_mreq fullgroup;
            fullgroup = group->IGRP;
            _mem_copy(&fullgroup, igrp, i);
            i = 0;
         } /* Endif */

      } /* Endfor */
   } /* Endif */

   parms->optlen = j;
   return RTCS_OK;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IGMP_leave_if_all
* Returned Values : error code
* Comments        :
*     Leave all the groups.
*
*END*-----------------------------------------------------------------*/

uint32_t IGMP_leave_if_all
   (
      MC_MEMBER_PTR         *phead
   )
{ /* Body */
   ip_mreq  igrp;
   uint32_t (_CODE_PTR_ temp_pfn)(MC_MEMBER_PTR *);
   uint32_t  status, tmp;

   status = RTCS_OK;

   /* leave all groups not still leaved */
   while (*phead) {
      igrp = (*phead)->IGRP;
      tmp = IGMP_leave_group(phead, &igrp, &temp_pfn);
      /* report only the first error, but leave the other groups anyway */
      if (tmp != RTCS_OK && status == RTCS_OK) {
         status = tmp;
      } /* Endif */
   } /* Endfor */

   return status;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IGMP_leave_socket_all
* Returned Values : error code
* Comments        :
*     leave the group on all the sockets
*
*END*-----------------------------------------------------------------*/

static uint32_t IGMP_leave_socket_all
   (
      ip_mreq  *igrp_ptr
   )
{ /* Body */
   ip_mreq              igrp;
   UDP_CFG_STRUCT_PTR   UDP_cfg_ptr = RTCS_getcfg(UDP);
   UCB_STRUCT_PTR       ucb;
   uint32_t              status, tmp;

   status = RTCS_OK;

   /* copy it in the local stack to be not to removed it during the leave */
   igrp = *igrp_ptr;

   /* test all bound sockets and leave the group if needed */
   RTCS_LIST_SEARCH(UDP_cfg_ptr->BOUND_UCB_HEAD, ucb) {
      /* test if the socket has joined multicast groups */
      if (!ucb->MCB_PTR) {
         continue;
      } /* Endif */
      /* test if the socket has joined this group */
      if (IGMP_member_find(&ucb->MCB_PTR, &igrp)) {
         /* if so, leave it */
         tmp = IGMP_leave_group(&ucb->MCB_PTR, &igrp, &ucb->IGMP_LEAVEALL);
         /* report only the first error, but leave the other groups anyway */
         if (tmp != RTCS_OK && status == RTCS_OK) {
            status = tmp;
         } /* Endif */
      } /* Endif */
   } /* End SEARCH */

   RTCS_LIST_SEARCH(UDP_cfg_ptr->LBOUND_UCB_HEAD, ucb) {
      /* test if the socket has joined multicast groups */
      if (!ucb->MCB_PTR) {
         continue;
      } /* Endif */
      /* test if the socket has joined this group */
      if (IGMP_member_find(&ucb->MCB_PTR, &igrp)) {
         /* if so, leave it */
         tmp = IGMP_leave_group(&ucb->MCB_PTR, &igrp, &ucb->IGMP_LEAVEALL);
         /* report only the first error, but leave the other groups anyway */
         if (tmp != RTCS_OK && status == RTCS_OK) {
            status = tmp;
         } /* Endif */
      } /* Endif */
   } /* End SEARCH */

   /* test all open sockets and leave the group if needed */
   RTCS_LIST_SEARCH(UDP_cfg_ptr->OPEN_UCB_HEAD, ucb) {
      /* test if the socket has joined multicast groups */
      if (!ucb->MCB_PTR) {
         continue;
      } /* Endif */
      /* test if the socket has joined this group */
      if (IGMP_member_find(&ucb->MCB_PTR, &igrp)) {
         /* if so, leave it */
         tmp = IGMP_leave_group(&ucb->MCB_PTR, &igrp, &ucb->IGMP_LEAVEALL);
         /* report only the first error, but leave the other groups anyway */
         if (tmp != RTCS_OK && status == RTCS_OK) {
            status = tmp;
         } /* Endif */
      } /* Endif */
   } /* End SEARCH */

   return status;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IGMP_ipif_unbind
* Returned Values : error code
* Comments        :
*     unbind a address to an interface. called by IPIF_unbind.
*     leave the groups still joined on this address/interface on every socket.
*
*END*-----------------------------------------------------------------*/

uint32_t IGMP_ipif_unbind
   (
      IP_IF_PTR   ipif,        /* [IN] the interface descriptor */
      _ip_address ip_address  /* [IN] the ip address of the interface */
   )
{ /* Body */
   MC_MEMBER_PTR        member;
   MC_MEMBER_PTR       *pmember;
   ip_mreq              group;
   uint32_t              status;

   status = RTCS_OK;

   /* leave the ALL_HOSTS group */
   group.imr_multiaddr.s_addr = INADDR_ALLHOSTS_GROUP;
   group.imr_interface.s_addr = ip_address;
   pmember = IGMP_member_find(&ipif->IGMP_MEMBER, &group);
   if (pmember) {
      IGMP_leave_if_local(ipif, pmember);
   } /* Endif */

   /* leave all joined groups */
   while (ipif->IGMP_MEMBER) {
      /* find a group which use this address */
      for (member = ipif->IGMP_MEMBER; member; member = member->NEXT) {
         if (member->IGRP.imr_interface.s_addr == ip_address) {
            break;
         } /* Endif */
      } /* Endfor */
      /* the interface address is no more used in a igmp group */
      if (!member) {
         break;
      } /* Endif */
      /* if it is still used, leave this group */
      status = IGMP_leave_socket_all(&member->IGRP);
      if (status != RTCS_OK) {
         return status;
      } /* Endif */
   } /* Endwhile */

   return status;
} /* Endbody */

#endif

/* EOF */
