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
*   This file contains the interface functions to the
*   RTCS tunnel device.
*
*
*END************************************************************************/

#include <rtcs.h>
#include "rtcs_prv.h"
#include "tcpip.h"
#include "iptunnel.h"


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_tunnel_add
* Returned Value  : RTCS_OK or error code
* Comments        :
*        Adds an IP over IP tunnel to RTCS.
*
*END*-----------------------------------------------------------------*/

uint32_t RTCS_tunnel_add
   (
      /* [IN] Inner IP header source address to tunnel */
      _ip_address  inner_source_addr,
      /* [IN] Inner IP header source address network mask */
      _ip_address  inner_source_netmask,
      /* [IN] Inner IP header destination address to tunnel */
      _ip_address  inner_dest_addr,
      /* [IN] Inner IP header destination network mask */
      _ip_address  inner_dest_netmask,
      /* [IN] Outer IP header source address */
      _ip_address  outer_source_addr,
      /* [IN] Outer IP header destination address */
      _ip_address  outer_dest_addr,
      /* [IN] Flags to set tunnel behaviour */
      uint32_t flags
   )
{ /* Body */
   IPIP_PARM  parms;

   parms.TUNNEL.INNER_SOURCE         = inner_source_addr & inner_source_netmask;
   parms.TUNNEL.INNER_SOURCE_NETMASK = inner_source_netmask;
   parms.TUNNEL.INNER_DEST           = inner_dest_addr & inner_dest_netmask;
   parms.TUNNEL.INNER_DEST_NETMASK   = inner_dest_netmask;
   parms.TUNNEL.OUTER_DEST           = outer_dest_addr;
   parms.TUNNEL.OUTER_SOURCE         = outer_source_addr;
   parms.TUNNEL.FLAGS                = flags;

   return RTCSCMD_issue(parms, IPIP_insert);
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_tunnel_remove
* Returned Value  : RTCS_OK or error code
* Comments        :
*        Removes an IP over IP tunnel from RTCS.
*
*END*-----------------------------------------------------------------*/

uint32_t RTCS_tunnel_remove
   (
      /* [IN] Inner IP header source address to tunnel */
      _ip_address  inner_source_addr,
      /* [IN] Inner IP header source address network mask */
      _ip_address  inner_source_netmask,
      /* [IN] Inner IP header destination address to tunnel */
      _ip_address  inner_dest_addr,
      /* [IN] Inner IP header destination network mask */
      _ip_address  inner_dest_netmask,
      /* [IN] Outer IP header source address */
      _ip_address  outer_source_addr,
      /* [IN] Outer IP header destination address */
      _ip_address  outer_dest_addr,
      /* [IN] Flags to set tunnel behaviour */
      uint32_t flags
   )
{ /* Body */
   IPIP_PARM  parms;

   parms.TUNNEL.INNER_SOURCE         = inner_source_addr & inner_source_netmask;
   parms.TUNNEL.INNER_SOURCE_NETMASK = inner_source_netmask;
   parms.TUNNEL.INNER_DEST           = inner_dest_addr & inner_dest_netmask;
   parms.TUNNEL.INNER_DEST_NETMASK   = inner_dest_netmask;
   parms.TUNNEL.OUTER_DEST           = outer_dest_addr;
   parms.TUNNEL.OUTER_SOURCE         = outer_source_addr;
   parms.TUNNEL.FLAGS                = flags;

   return RTCSCMD_issue(parms, IPIP_delete);

} /* Endbody */


/* EOF */
