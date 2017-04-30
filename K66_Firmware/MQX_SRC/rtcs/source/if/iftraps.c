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

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_trap_target_add
* Returned Value  : RTCS_OK or error code
* Comments        :
*        Register an IP address to direct SNMP traps to.
*
*END*-----------------------------------------------------------------*/

uint32_t RTCS_trap_target_add(_ip_address ip_addr) {

#if RTCSCFG_ENABLE_IP4

   TRAP_PARM   parms;
   uint32_t     error;

   parms.address = ip_addr;
   
   error = RTCSCMD_issue(parms, RTCS_insert_trap_receiver_internal);

   return error;

#else

    return RTCSERR_IP_IS_DISABLED; 

#endif /* RTCSCFG_ENABLE_IP4 */   

}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_trap_target_remove
* Returned Value  : RTCS_OK or error code
* Comments        :
*        Register an IP address to direct SNMP traps to.
*
*END*-----------------------------------------------------------------*/

uint32_t RTCS_trap_target_remove(_ip_address ip_addr) {

#if RTCSCFG_ENABLE_IP4

   TRAP_PARM   parms;
   uint32_t     error;

   parms.address = ip_addr;
   
   error = RTCSCMD_issue(parms, RTCS_remove_trap_receiver_internal);

   return error;


#else

    return RTCSERR_IP_IS_DISABLED; 

#endif /* RTCSCFG_ENABLE_IP4 */     

}

/* EOF */
