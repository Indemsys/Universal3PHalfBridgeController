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
*   This file contains the implementation of DNAT specific
*   user interface functions
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
* Function Name   : DNAT_add_rule
* Returned Value  : RTCS_OK or error code
* Comments        :  
*    Adds a dnat rule to the dnat rule table. Rules in the dnat 
*    rule are adminisratively ordered based on priority. The priority 
*    must be unique.
*
*END*-----------------------------------------------------------------*/

uint32_t DNAT_add_rule
   (
      uint32_t         priority,        /* [IN] */   
      uint16_t         protocol,        /* [IN] */
      uint16_t         start_port,      /* [IN] */
      uint16_t         end_port,        /* [IN] */
      _ip_address     private_ip,      /* [IN]  */
      uint16_t         private_port
   )
{ /* Body */
   
   DNAT_PARM          dparms;

   if (priority == 0) {
      return RTCSERR_NAT_INVALID_RULE;
   }

   dparms.RULE.PRIORITY           = priority;
   dparms.RULE.IP_PROTOCOL        = protocol;
   dparms.RULE.PUBLIC_START_PORT  = start_port;
   dparms.RULE.PUBLIC_END_PORT    = end_port;
   dparms.RULE.PRIVATE_START_PORT = private_port;
   dparms.RULE.PRIVATE_END_PORT   = (uint16_t) ((uint32_t)private_port+end_port-start_port);
   dparms.RULE.PRIVATE_IP         = private_ip;
   // NAT event add needs to sort timeouts before a non-default (non-zero) timeout will work
   dparms.RULE.TIMEOUT            = 0;  
   return RTCSCMD_issue(dparms, DNAT_add_rule_internal);
     
}/* Endbody */
  
/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : DNAT_delete_rule
* Returned Value  : RTCS_OK or error code
* Comments        :
*     Removes a dnat rule from the dnat rule table.
*
*END*-----------------------------------------------------------------*/

uint32_t  DNAT_delete_rule
   (
    uint32_t  priority
    )
{ /* Body */   
   DNAT_PARM          dparms;
   
   if (priority == 0) {
      return RTCSERR_NAT_INVALID_RULE;
   }

   dparms.RULE.PRIORITY = priority;
   
   return RTCSCMD_issue(dparms, DNAT_delete_rule_internal);
   
} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : DNAT_get_next_rule
* Returned Value  : RTCS_OK or error code
* Comments        :
*     Returns a dnat rul from the rule table.
*
*END*-----------------------------------------------------------------*/

uint32_t  DNAT_get_next_rule
   (
      uint32_t          *priority_ptr,    /* [IN/OUT] */  
      uint16_t          *protocol_ptr,    /* [OUT] */
      uint16_t          *start_port_ptr,  /* [OUT] */
      uint16_t          *end_port_ptr,    /* [OUT] */
      _ip_address     *private_ip_ptr,  /* [OUT] */
      uint16_t          *private_port_ptr /* [OUT] */
    )
{ /* Body */   
   DNAT_PARM      dparms;
   uint32_t        result;
      
   dparms.RULE.PRIORITY = MAX_UINT_32;
   
   if (priority_ptr != NULL)
   {
	   dparms.RULE.PRIORITY = (*priority_ptr) ? (*priority_ptr) : (MAX_UINT_32); 
   }
	   
   result = RTCSCMD_issue(dparms, DNAT_get_next_rule_internal);

   if (result == RTCS_OK) {
      if (priority_ptr != NULL)     *priority_ptr     = dparms.RULE.PRIORITY;
      if (protocol_ptr != NULL)     *protocol_ptr     = dparms.RULE.IP_PROTOCOL;
      if (start_port_ptr != NULL)   *start_port_ptr   = dparms.RULE.PUBLIC_START_PORT;
      if (end_port_ptr != NULL)     *end_port_ptr     = dparms.RULE.PUBLIC_END_PORT;
      if (private_port_ptr != NULL) *private_port_ptr = dparms.RULE.PRIVATE_START_PORT;
      if (private_ip_ptr != NULL)   *private_ip_ptr   = dparms.RULE.PRIVATE_IP;
   }
   return result;

} /* Endbody */

#endif

/* EOF */
