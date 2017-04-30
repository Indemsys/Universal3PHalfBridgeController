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
*   This file contains the DNAT specific functions.
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
* Function Name   : DNAT_lookup_rule
* Returned Value  : RTCS_OK or error code
* Comments        :
*     Lookup all dnat rule from the dnat rule table and return
*     the match for the current port.
*
*END*-----------------------------------------------------------------*/

DNAT_RULE_STRUCT_PTR DNAT_lookup_rule
   (
      NAT_CFG_STRUCT_PTR      nat_cfg_ptr,
      IP_HEADER_PTR           ip_header_ptr,
      bool                 pub_to_prv
    )
{ /* Body */
   TRANSPORT_UNION            transport;
   DNAT_ELEMENT_STRUCT_PTR    element_ptr;
   _ip_address                source_ip = mqx_ntohl(ip_header_ptr->SOURCE);
   uint32_t                    ip_protocol;
   uint16_t                    source_port, destination_port; /* source port and destination port */
 
   transport.PTR = TRANSPORT_PTR(ip_header_ptr);
   
   ip_protocol = mqx_ntohc(ip_header_ptr->PROTOCOL);

   /* NAT spports ICMP, UDP and TCP transport layer protocols */
   switch (ip_protocol) {
      case IPPROTO_TCP:
          destination_port = mqx_ntohs(transport.TCP_PTR->dest_port);
          source_port = mqx_ntohs(transport.TCP_PTR->source_port);
          break;
      
      case IPPROTO_UDP:
          destination_port = mqx_ntohs(transport.UDP_PTR->DEST_PORT);
          source_port = mqx_ntohs(transport.UDP_PTR->SRC_PORT);
          break;
      
      case IPPROTO_ICMP:
          /* Allow all ICMP request/reply */
          return NULL;
   } /* Endswitch */
     
     
   element_ptr = (DNAT_ELEMENT_STRUCT_PTR) _queue_head(&nat_cfg_ptr->RULE_QUEUE);  

   /*
   ** Check for the target port and then forward the packet to the corresponding 
   ** DNAT rule target ip.
   */
   while (element_ptr != NULL) {
      if (element_ptr->RULE.IP_PROTOCOL == ip_protocol) {
         if (pub_to_prv) {
            if ((destination_port >= element_ptr->RULE.PUBLIC_START_PORT) &&
                (destination_port <= element_ptr->RULE.PUBLIC_END_PORT)) {
               break;
            }
         } else {
            if ((source_ip == element_ptr->RULE.PRIVATE_IP) &&
                (source_port >= element_ptr->RULE.PRIVATE_START_PORT) &&  
                (source_port <= element_ptr->RULE.PRIVATE_END_PORT)) {
               break;  
            }
         }
      }
      element_ptr = (DNAT_ELEMENT_STRUCT_PTR)  _queue_next(&nat_cfg_ptr->RULE_QUEUE, &element_ptr->ELEMENT);
   } /* Endwhile */
   
   if (element_ptr!=NULL) {
      return &element_ptr->RULE;
   } else {
      return NULL;
   }
}/* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : DNAT_add_rule_internal
* Returned Value  : RTCS_OK or error code
* Parameters     : 
* Comments        : Add rule to internal structures.
*        
*
*END*-----------------------------------------------------------------*/

void DNAT_add_rule_internal
   (
      DNAT_PARM_PTR           dparm_ptr      /* [IN] */   
   )
{ /* Body */
   NAT_CFG_STRUCT_PTR         nat_cfg_ptr = RTCS_getcfg(NAT);
   DNAT_ELEMENT_STRUCT_PTR    new_element_ptr, prev_element_ptr, next_element_ptr;

   if (nat_cfg_ptr == NULL) {
      RTCSCMD_complete(dparm_ptr, RTCSERR_NAT_NOT_INITIALIZED);
      return;
   }

   if (! NAT_is_private_addr(&nat_cfg_ptr->PRIVATE_NETWORKS,dparm_ptr->RULE.PRIVATE_IP)) {
      RTCSCMD_complete(dparm_ptr, RTCSERR_NAT_INVALID_PRIVATE_ADDRESS);
      return;
   }

   if (IP_is_local(NULL, dparm_ptr->RULE.PRIVATE_IP)) {
      RTCSCMD_complete(dparm_ptr, RTCSERR_NAT_INVALID_PRIVATE_ADDRESS);
      return;
   }
        
   prev_element_ptr = NULL;
   next_element_ptr = (DNAT_ELEMENT_STRUCT_PTR) _queue_head(&nat_cfg_ptr->RULE_QUEUE);  
   
   while ((next_element_ptr != NULL) && 
          (next_element_ptr->RULE.PRIORITY > dparm_ptr->RULE.PRIORITY)) {
      prev_element_ptr = next_element_ptr;
      next_element_ptr = (DNAT_ELEMENT_STRUCT_PTR) 
         _queue_next(&nat_cfg_ptr->RULE_QUEUE, &prev_element_ptr->ELEMENT);
   }
   
   if (next_element_ptr != NULL) {
      if (next_element_ptr->RULE.PRIORITY == dparm_ptr->RULE.PRIORITY) {
         // A rule of the given priority exist
         RTCSCMD_complete(dparm_ptr, RTCSERR_NAT_DUPLICATE_PRIORITY);
         return;
      }
   }

   new_element_ptr = (DNAT_ELEMENT_STRUCT_PTR)_mem_alloc_system_zero(sizeof(DNAT_ELEMENT_STRUCT));

   if (new_element_ptr == NULL) {
      RTCSCMD_complete(dparm_ptr, RTCSERR_OUT_OF_MEMORY);
      return;
   } /* Endif */

   new_element_ptr->RULE    = dparm_ptr->RULE;

   _queue_insert(&nat_cfg_ptr->RULE_QUEUE, &prev_element_ptr->ELEMENT, &new_element_ptr->ELEMENT);    
   RTCSCMD_complete(dparm_ptr, RTCS_OK);
}/* Endbody */
  
  
  
/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : DNAT_delete_rule_internal
* Returned Value  : RTCS_OK or error code
* Comments        : Removes a rule from the dnat rule table.
*
*END*-----------------------------------------------------------------*/

void  DNAT_delete_rule_internal
   (
      DNAT_PARM_PTR           dparm_ptr      /* [IN] */
    )
{ /* Body */   
   NAT_CFG_STRUCT_PTR      nat_cfg_ptr = RTCS_getcfg(NAT);
   DNAT_ELEMENT_STRUCT_PTR element_ptr;
   uint32_t                 priority = dparm_ptr->RULE.PRIORITY;

   if (nat_cfg_ptr == NULL) {
      RTCSCMD_complete(dparm_ptr, RTCSERR_NAT_NOT_INITIALIZED);
      return;
   }

   element_ptr = (DNAT_ELEMENT_STRUCT_PTR) _queue_head(&nat_cfg_ptr->RULE_QUEUE); 

   while (element_ptr != NULL) {
      if (element_ptr->RULE.PRIORITY == priority) {
         _queue_unlink(&nat_cfg_ptr->RULE_QUEUE, &element_ptr->ELEMENT); 
         _mem_free((void *)element_ptr);
         RTCSCMD_complete(dparm_ptr, RTCS_OK);
         return;
      } 
      element_ptr = (DNAT_ELEMENT_STRUCT_PTR) _queue_next(&nat_cfg_ptr->RULE_QUEUE, &element_ptr->ELEMENT);
   }
   
   RTCSCMD_complete(dparm_ptr, RTCSERR_NAT_INVALID_RULE);
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : DNAT_add_rule_internal
* Returned Value  : RTCS_OK or error code
* Parameters     : 
* Comments        : Add rule to internal structures.
*        
*
*END*-----------------------------------------------------------------*/

void DNAT_get_next_rule_internal
   (
      DNAT_PARM_PTR           dparm_ptr      /* [IN] */   
   )
{ /* Body */
   NAT_CFG_STRUCT_PTR         nat_cfg_ptr = RTCS_getcfg(NAT);
   DNAT_ELEMENT_STRUCT_PTR    prev_element_ptr, next_element_ptr;

   if (nat_cfg_ptr == NULL) {
      RTCSCMD_complete(dparm_ptr, RTCSERR_NAT_NOT_INITIALIZED);
      return;
   }

        
   prev_element_ptr = NULL;
   next_element_ptr = (DNAT_ELEMENT_STRUCT_PTR) _queue_head(&nat_cfg_ptr->RULE_QUEUE);  
   
   while ((next_element_ptr != NULL) && 
          (next_element_ptr->RULE.PRIORITY >= dparm_ptr->RULE.PRIORITY)) {
      prev_element_ptr = next_element_ptr;
      next_element_ptr = (DNAT_ELEMENT_STRUCT_PTR) _queue_next(&nat_cfg_ptr->RULE_QUEUE, &prev_element_ptr->ELEMENT);
   }
   
   if (next_element_ptr == NULL) {
      RTCSCMD_complete(dparm_ptr, RTCSERR_NAT_END_OF_RULES);
   } else {
      dparm_ptr->RULE    = next_element_ptr->RULE;

      RTCSCMD_complete(dparm_ptr, RTCS_OK);
   }
}/* Endbody */

#endif
  
  
/* EOF */
