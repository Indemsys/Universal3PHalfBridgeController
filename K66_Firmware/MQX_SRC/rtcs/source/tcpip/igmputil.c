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
*   This file contains the implementation of the Internet
*   Group Management Protocol.  For more details, refer
*   to RFC1112 for version 1 and RFC2236 for version 2.
*
*
*END************************************************************************/

#include <rtcs.h>
#include "rtcs_prv.h"
#include "tcpip.h"
#include "ip_prv.h"

#if RTCSCFG_ENABLE_IGMP && RTCSCFG_ENABLE_IP4

/*
** Warning: IGMP is OPTIONAL.  The functions below (most notably
**          IGMP_ipif_add, IGMP_ipif_bind, IGMP_ipif_unbind_nosock
**          and IGMP_is_member) may be called without a prior call
**          to IGMP_init.
*/

uint32_t IGMP_join_if_local      (IP_IF_PTR, ip_mreq *);
uint32_t IGMP_leave_if_local     (IP_IF_PTR, MC_MEMBER_PTR *);
uint32_t IGMP_ipif_unbind_nosock (IP_IF_PTR, _ip_address);


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IGMP_filter_add
* Returned Values : uint32_t
* Comments        :
*     add a IGMP_filter in the physical interface
*
*END*-----------------------------------------------------------------*/

uint32_t IGMP_filter_add
   (
      IP_IF_PTR   ipif,
      _ip_address group_ip
   )
{ /* Body */
   if (!ipif->DEVICE.JOIN) {
      return RTCS_OK;
   } /* Endif */

   /* join the group on the physical interface */
   return ipif->DEVICE.JOIN(ipif, group_ip);
} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IGMP_filter_rm
* Returned Values : uint32_t
* Comments        :
*     remove a IGMP_filter in the physical interface
*
*END*-----------------------------------------------------------------*/

uint32_t IGMP_filter_rm
   (
      IP_IF_PTR   ipif,
      _ip_address group_ip
   )
{ /* Body */
   if (!ipif->DEVICE.LEAVE) {
      return RTCS_OK;
   } /* Endif */

   /* leave the group on the physical interface */
   return ipif->DEVICE.LEAVE(ipif, group_ip);
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IGMP_member_find
* Returned Values : MC_MEMBER_PTR
* Comments        :
*     look for a given group through a list of joined group
*
*END*-----------------------------------------------------------------*/

MC_MEMBER_PTR  *IGMP_member_find
   (
      MC_MEMBER_PTR   *phead,
      const ip_mreq         *igrp
   )
{ /* Body */

   for (; *phead ; phead = &(*phead)->NEXT) {
      if ((*phead)->IGRP.imr_multiaddr.s_addr == igrp->imr_multiaddr.s_addr
       && (*phead)->IGRP.imr_interface.s_addr == igrp->imr_interface.s_addr) {
         /* the group is already joined */
         return phead;
      } /* Endif */
   } /* Endfor */

   return NULL;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IGMP_member_create
* Returned Values : MC_MEMBER_PTR
* Comments        :
*     add a new group in a list
*
*END*-----------------------------------------------------------------*/

MC_MEMBER_PTR  *IGMP_member_create
   (
      MC_MEMBER_PTR   *phead,
      const ip_mreq         *igrp
   )
{ /* Body */
   IP_CFG_STRUCT_PTR IP_cfg_ptr = RTCS_getcfg(IP);
   MC_MEMBER_PTR     group;

   group = RTCS_part_alloc_zero(IP_cfg_ptr->MCB_PARTID);
   if (!group) {
      return NULL;
   } /* Endif */
   group->IGRP = *igrp;
   group->UCOUNT = 1;
   group->NEXT = *phead;
   *phead = group;

   return phead;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IGMP_member_delete
* Returned Values : uint32_t
* Comments        :
*     remove a group from a list
*
*END*-----------------------------------------------------------------*/

uint32_t IGMP_member_delete
   (
      MC_MEMBER_PTR   *pmember
   )
{ /* Body */
   MC_MEMBER_PTR  group;

   group = *pmember;
   *pmember = group->NEXT;
   RTCS_part_free(group);

   return RTCS_OK;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IGMP_join_if_local
* Returned Values : uint32_t
* Comments        :
*     join a local group on a given interface
*
*END*-----------------------------------------------------------------*/

uint32_t IGMP_join_if_local
   (
      IP_IF_PTR      ipif, /* [IN] the interface descriptor */
      ip_mreq   *igrp  /* [IN] the group description */
   )
{ /* Body */
   MC_MEMBER_PTR       *pgroup;
   uint32_t              status;

   /* join the group */
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

   return RTCS_OK;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IGMP_leave_if_local
* Returned Values : uint32_t
* Comments        :
*     leave the group on a given interface
*
*END*-----------------------------------------------------------------*/

uint32_t IGMP_leave_if_local
   (
      IP_IF_PTR            ipif,    /* [IN] the interface descriptor */
      MC_MEMBER_PTR   *pmember  /* [IN] the group to leave */
   )
{ /* Body */
   uint32_t  status, tmp;

   /* leave the group on the physical interface */
   status = IGMP_filter_rm(ipif, (*pmember)->IGRP.imr_multiaddr.s_addr);

   /* leave the group */
   tmp = IGMP_member_delete(pmember);
   if (tmp != RTCS_OK && status == RTCS_OK) {
      status = tmp;
   } /* Endif */

   return status;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IGMP_ipif_add
* Returned Values : error code
* Comments        :
*     init the IGMP part of a given interface. called by IPIF_add
*
*END*-----------------------------------------------------------------*/

uint32_t IGMP_ipif_add
   (
      IP_IF_PTR   ipif /* [IN] the interface descriptor */
   )
{ /* Body */
   ipif->IGMP_MEMBER = NULL;
   ipif->IGMP_UNBIND = IGMP_ipif_unbind_nosock;
#ifdef IGMP_V2
   ipif->IGMP_V1_ROUTER_FLAG = FALSE;
#endif
   return RTCS_OK;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IGMP_ipif_bind
* Returned Values : error code
* Comments        :
*     bind the IGMP part of a given interface. called by IPIF_bind
*     mainly for joining the ALL_HOSTS group.
*
*END*-----------------------------------------------------------------*/

uint32_t IGMP_ipif_bind
   (
      IP_IF_PTR   ipif,       /* [IN] the interface descriptor */
      _ip_address ip_address  /* [IN] the ip address of the interface */
   )
{ /* Body */
   ip_mreq  group;

   /* join the ALL_HOSTS group */
   group.imr_multiaddr.s_addr = INADDR_ALLHOSTS_GROUP;
   group.imr_interface.s_addr = ip_address;
   return IGMP_join_if_local(ipif, &group);
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IGMP_ipif_unbind_nosock
* Returned Values : error code
* Comments        :
*     unbind a address to an interface. called by IPIF_unbind.
*     leave the groups still joined on this address/interface.
*
*END*-----------------------------------------------------------------*/

uint32_t IGMP_ipif_unbind_nosock
   (
      IP_IF_PTR   ipif,       /* [IN] the interface descriptor */
      _ip_address ip_address  /* [IN] the ip address of the interface */
   )
{ /* Body */
   MC_MEMBER_PTR       *pmember;
   uint32_t              status, tmp;

   status = RTCS_OK;

   /* leave all joined groups */
   for (pmember = &ipif->IGMP_MEMBER; *pmember;) {
      if ((*pmember)->IGRP.imr_interface.s_addr != ip_address) {
         pmember = &(*pmember)->NEXT;
         continue;
      } /* Endif */
      tmp = IGMP_leave_if_local(ipif, pmember);
      if (tmp != RTCS_OK && status == RTCS_OK) {
         status = tmp;
      } /* Endif */
   } /* Endfor */

   return status;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IGMP_is_member
* Returned Values : bool
* Comments        :
*     return TRUE if the socket is a member of a given group.
*
*END*-----------------------------------------------------------------*/

struct IGMP_is_member_test_struct {
   _ip_address          multiaddr;
   IP_IF_PTR            ipif;
   MC_MEMBER_PTR       *phead;
   bool              result;
};

static bool IGMP_is_member_test
   (
      _ip_address    node_ip,
      _ip_address    node_mask,
      void          *node_data,
      void          *data
   )
{ /* Body */
   struct IGMP_is_member_test_struct        *testdata = data;
   IP_ROUTE_PTR                              route = node_data;
   IP_ROUTE_DIRECT_PTR                       search_ptr;
   ip_mreq                                   group;

   if (route && route->DIRECT) {
      search_ptr = route->DIRECT;
      group.imr_multiaddr.s_addr = testdata->multiaddr;

      do { /* Body */
         if (search_ptr->NETIF == testdata->ipif) {
            group.imr_interface.s_addr = search_ptr->ADDRESS;
            if (IGMP_member_find(testdata->phead, &group)) {
               testdata->result = TRUE;
               return TRUE;
            } /* Endif */
         } /* Endif */

         search_ptr = search_ptr->NEXT;
      } while(search_ptr != route->DIRECT);
   } /* Endif */

   return FALSE;
} /* Body */


bool IGMP_is_member
   (
      MC_MEMBER_PTR   *phead,      /* [IN] head of MCB list */
      IP_IF_PTR            ipif,       /* [IN] the incoming interface */
      _ip_address          multiaddr   /* [IN] the multicast ip */
   )
{ /* Body */
   IP_CFG_STRUCT_PTR                   IP_cfg_ptr = RTCS_getcfg(IP);
   struct IGMP_is_member_test_struct   testdata;

   if (!*phead) {
      return FALSE;
   } /* Endif */

   testdata.multiaddr   = multiaddr;
   testdata.ipif        = ipif;
   testdata.phead       = phead;
   testdata.result      = FALSE;

   /*
   ** Scan all directly connected networks, to have all the ip
   ** address of this interface
   */
   IPRADIX_walk(&IP_cfg_ptr->ROUTE_ROOT.NODE, IGMP_is_member_test, &testdata);

   return testdata.result;

} /* Endbody */
#endif

/* EOF */
