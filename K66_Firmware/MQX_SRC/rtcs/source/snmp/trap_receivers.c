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
*   
*
*
*END************************************************************************/
#include <rtcs.h>
#include "rtcs_prv.h"
#include "snmpcfg.h"
#include "asn1.h"
#include "snmp.h"

#if RTCSCFG_ENABLE_SNMP && RTCSCFG_ENABLE_IP4 
/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : RTCS_insert_trap_receiver_internal
*  Parameters     :
*
*     _ip_address          address     will be inserted into 
*                                      RTCS_trap_receivers_table
*
*  Comments       :
*        Registers an SNMP trap receiver with RTCS.
*
*END*-----------------------------------------------------------------*/

void RTCS_insert_trap_receiver_internal
   (
      TRAP_PARM  *parm
   )
{ /* Body */
   volatile SNMP_PARSE_PTR    snmpcfg = SNMP_get_data();
   _ip_address       ip_addr = parm->address;
   _mqx_uint         i;
   
   /* Find an empty entry in the receivers list */
   for (i = 0; i < SNMPCFG_MAX_TRAP_RECEIVERS; i++) {
      if (snmpcfg->trap_receiver_list[i] == 0) {
         /* Found a space */
         snmpcfg->trap_receiver_list[i] = ip_addr;
         break;
      } /* Endif */     
   } /* Endfor */
      
   if (i < SNMPCFG_MAX_TRAP_RECEIVERS) {
      RTCSCMD_complete(parm, RTCS_OK);
   } else {
      RTCSCMD_complete(parm, RTCSERR_TRAP_INSERT_FAILED);
   } /* Endif */

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : RTCS_remove_trap_receiver_internal
*  Parameters     :
*
*     _ip_address          address     will be removed from 
*                                      RTCS_trap_receivers_table
*
*  Comments       :
*        Unregisters an SNMP trap receiver from RTCS.
*
*END*-----------------------------------------------------------------*/

void RTCS_remove_trap_receiver_internal
   (
      TRAP_PARM  *parm
   )
{ /* Body */
   volatile SNMP_PARSE_PTR    snmpcfg = SNMP_get_data();
   _ip_address       ip_addr = parm->address;
   _mqx_uint         i;
   
   /* Traverse the receivers list looking for our entry */
   for (i = 0; i < SNMPCFG_MAX_TRAP_RECEIVERS; i++) {
      if (snmpcfg->trap_receiver_list[i] == ip_addr) {
         /* Found it */
         snmpcfg->trap_receiver_list[i] = 0;
         break;
      } /* Endif */     
   } /* Endfor */
      
   if (i < SNMPCFG_MAX_TRAP_RECEIVERS) {
      RTCSCMD_complete(parm, RTCS_OK);
   } else {
      RTCSCMD_complete(parm, RTCSERR_TRAP_REMOVE_FAILED);
   } /* Endif */

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : RTCS_count_trap_receivers_internal
*  Parameters     :
*  Comments       : Returns the number of receivers installed
*
*END*-----------------------------------------------------------------*/

_mqx_uint RTCS_count_trap_receivers_internal
   (
      void
   )
{ /* Body */
    volatile SNMP_PARSE_PTR    snmpcfg = SNMP_get_data();
    _mqx_uint         i;
    _mqx_uint         count = 0;

    for (i = 0; i < SNMPCFG_MAX_TRAP_RECEIVERS; i++) {
       if (snmpcfg->trap_receiver_list[i] != 0) {
         count++;
       } /* Endif */     
    } /* Endfor */
   
   return count;
} /* Endbody */

#endif /* RTCSCFG_ENABLE_SNMP && RTCSCFG_ENABLE_IP4 */
/* EOF */
