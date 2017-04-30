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
*   This file has functions to retrieve statistics structures
*   for each protocol in RTCS.
*
*
*END************************************************************************/

#include <rtcs.h>
#include "rtcs_prv.h"
#include "tcpip.h"
#include "tcp.h"
#include "udp_prv.h"
#include "icmp_prv.h"
#include "ip_prv.h"

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  IP_stats
* Returned Value  :  IP statistics
* Comments        :
*     Returns a pointer to the IP layer statistics.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP_STATS
IP_STATS_PTR IP_stats
   (
      void
   )
{
#if RTCSCFG_ENABLE_IP4
   IP_CFG_STRUCT_PTR   ip_cfg_ptr;

   ip_cfg_ptr = RTCS_getcfg(IP);

   if (ip_cfg_ptr != NULL) {
      return &ip_cfg_ptr->STATS;
   }
#endif /* RTCSCFG_ENABLE_IP4 */
   return NULL;
}

#endif

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  IP6_stats
* Returned Value  :  IPv6 statistics
* Comments        :
*     Returns a pointer to the IPv6 layer statistics.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP6_STATS
IP6_STATS_PTR IP6_stats
   (
      void
   )
{ 
#if RTCSCFG_ENABLE_IP6
   IP6_CFG_STRUCT_PTR   ip6_cfg_ptr;

   ip6_cfg_ptr = RTCS_getcfg(IP6);

   if (ip6_cfg_ptr != NULL) {
      return &ip6_cfg_ptr->STATS;
   }
#endif /* RTCSCFG_ENABLE_IP6 */
   return NULL;
} 

#endif


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  ICMP_stats
* Returned Value  :  ICMP statistics
* Comments        :
*     Returns a pointer to the ICMP layer statistics.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_ICMP_STATS
ICMP_STATS_PTR ICMP_stats
   (
      void
   )
{
#if RTCSCFG_ENABLE_IP4
   ICMP_CFG_STRUCT_PTR   icmp_cfg_ptr;

   icmp_cfg_ptr = RTCS_getcfg(ICMP);

   if (icmp_cfg_ptr != NULL) {
      return &icmp_cfg_ptr->STATS;
   }
#endif /* RTCSCFG_ENABLE_IP4 */
   return NULL;
}

#endif

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  ICMP6_stats
* Returned Value  :  ICMPv6 statistics
* Comments        :
*     Returns a pointer to the ICMPv6 layer statistics.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_ICMP6_STATS
ICMP6_STATS_PTR ICMP6_stats
   (
      void
   )
{
#if RTCSCFG_ENABLE_IP6
   ICMP6_CFG_STRUCT_PTR   icmp6_cfg_ptr;

   icmp6_cfg_ptr = RTCS_getcfg(ICMP6);

   if (icmp6_cfg_ptr != NULL) {
      return &icmp6_cfg_ptr->STATS;
   }
#endif /* RTCSCFG_ENABLE_IP6 */
   return NULL;
} 

#endif


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  UDP_stats
* Returned Value  :  UDP statistics
* Comments        :
*     Returns a pointer to the UDP layer statistics.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_UDP_STATS
UDP_STATS_PTR UDP_stats
   (
      void
   )
{ /* Body */
   UDP_CFG_STRUCT_PTR   udp_cfg_ptr;

   udp_cfg_ptr = RTCS_getcfg(UDP);

   if (udp_cfg_ptr != NULL) {
      return &udp_cfg_ptr->STATS;
   } /* Endif */

   return NULL;
} /* Endbody */
#endif

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  TCP_stats
* Returned Value  :  TCP statistics
* Comments        :
*     Returns a pointer to the TCP layer statistics.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_TCP_STATS
TCP_STATS_PTR TCP_stats
   (
      void
   )
{ /* Body */
   TCP_CFG_STRUCT_PTR   tcp_cfg_ptr;

   tcp_cfg_ptr = RTCS_getcfg(TCP);

   if (tcp_cfg_ptr != NULL) {
      return &tcp_cfg_ptr->STATS;
   } /* Endif */

   return NULL;
} /* Endbody */
#endif


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  IPIF_stats
* Returned Value  :  IPIF statistics
* Comments        :
*     Returns a pointer to an interface's statistics.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IPIF_STATS
IPIF_STATS_PTR IPIF_stats
   (
      _rtcs_if_handle   handle
         /* [IN] the RTCS interface state structure */
   )
{ /* Body */
   IP_IF_PTR if_ptr = (IP_IF_PTR)handle;
   return &if_ptr->STATS;
} /* Endbody */
#endif


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  ARP_stats
* Returned Value  :  ARP statistics
* Comments        :
*     Returns a pointer to an interface's ARP statistics.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_ARP_STATS
ARP_STATS_PTR ARP_stats
   (
      _rtcs_if_handle   handle
         /* [IN] the RTCS interface state structure */
   )
{ /* Body */
#if RTCSCFG_ENABLE_IP4
    IP_IF_PTR if_ptr = (IP_IF_PTR)handle;
    return (ARP_STATS_PTR)if_ptr->ARP;
#else
    return NULL;   
#endif /* RTCSCFG_ENABLE_IP4 */
} /* Endbody */
#endif


/* EOF */
