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
*   This file contains the NAT interface functions.
*
*
*END************************************************************************/


#include <rtcsrtos.h>
#include <rtcs.h>
#include <rtcs_prv.h>

#if RTCSCFG_ENABLE_NAT

#include "nat.h"
#include "nat_prv.h"


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : NAT_init
* Returned Value  : RTCS_OK or error code
* Comments        :
*        Starts network address translation.
*
*END*-----------------------------------------------------------------*/

uint32_t NAT_init
   (
      _ip_address    private_network,
      _ip_address    private_netmask
   )
{ /* Body */
   NAT_PARM    parms;
   
   parms.IP_PRV = private_network;
   parms.IP_MASK = private_netmask;
   
   return RTCSCMD_issue(parms, NAT_init_internal);
   
} /* Endbody */
   
/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : NAT_add_network
* Returned Value  : RTCS_OK or error code
* Comments        :
*        Add another private network.
*
*END*-----------------------------------------------------------------*/

uint32_t NAT_add_network
   (
      _ip_address    private_network,
      _ip_address    private_netmask
   )
{ /* Body */
   NAT_PARM    parms;
   
   parms.IP_PRV = private_network;
   parms.IP_MASK = private_netmask;
   
   return RTCSCMD_issue(parms, NAT_add_network_internal);
   
} /* Endbody */
   

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : NAT_remove_network
* Returned Value  : RTCS_OK or error code
* Comments        :
*        Add another private network.
*
*END*-----------------------------------------------------------------*/

uint32_t NAT_remove_network
   (
      _ip_address    private_network,
      _ip_address    private_netmask
   )
{ /* Body */
   NAT_PARM    parms;
   
   parms.IP_PRV = private_network;
   parms.IP_MASK = private_netmask;
   
   return RTCSCMD_issue(parms, NAT_remove_network_internal);
   
} /* Endbody */
   

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : NAT_close
* Returned Value  : RTCS_OK or error code
* Comments        :
*        Stops network address translation.
*
*END*-----------------------------------------------------------------*/
   
uint32_t NAT_close
   (
      void
   )
{ /* Body */
   NAT_PARM    parms;

   return RTCSCMD_issue(parms, NAT_close_internal);
   
} /* Endbody */
      

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : NAT_stats
* Returned Value  : NULL, or a pointer to the NAT stats
* Comments        :
*        Returns a pointer to the NAT statistics
*
*END*-----------------------------------------------------------------*/

NAT_STATS_PTR NAT_stats
   (
      void
   )
{ /* Body */
   NAT_CFG_STRUCT_PTR nat_cfg_ptr = RTCS_getcfg(NAT);
   
   if (nat_cfg_ptr) {
      return &nat_cfg_ptr->STATS;
   } /* Endif */
 
   return NULL;  
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : NAT_networks
* Returned Value  : NULL, or a pointer to the NAT networks structure
* Comments        :
*        Returns a pointer to the NAT statistics
*
*END*-----------------------------------------------------------------*/

NAT_NETWORK_STRUCT_PTR NAT_networks
   (
      void
   )
{ /* Body */
   NAT_CFG_STRUCT_PTR nat_cfg_ptr = RTCS_getcfg(NAT);
   
   if (nat_cfg_ptr) {
      return &nat_cfg_ptr->PRIVATE_NETWORKS;
   } /* Endif */
 
   return NULL;  
} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : NAT_find_next_session
* Returned Value  : 
* Comments        :
*        
*
*END*-----------------------------------------------------------------*/

void *NAT_find_next_session
   (
      void    *session,
      uint32_t  tree
   )
{ /* Body */
   NAT_PARM    parms;
   uint32_t     result;
   
   parms.CONFIG = session;
   parms.OPTION = tree;
   
   result = RTCSCMD_issue(parms, NAT_find_next_session_internal);
   if (result != RTCS_OK) {
      return NULL;
   }
 
   return parms.CONFIG;  
} /* Endbody */

#endif


/* EOF */
