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
*   packet driver interface.
*
*
*END************************************************************************/

#include <rtcs.h>
#include "rtcs_prv.h"
#include "tcpip.h"
#include "arpif.h"

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_arp_add
* Returned Value  : RTCS_OK or error code
* Comments        :
*        Add an entry to an interfaces ARP cache.
*
*END*-----------------------------------------------------------------*/

uint32_t RTCS_arp_add
   (
      _rtcs_if_handle    *ihandle,
         /* [IN] the RTCS interface state structure */
      _ip_address              paddr,
         /* [IN] the address to add */
      char           laddr[6]
   )
{

#if RTCSCFG_ENABLE_IP4

    ARPIF_PARM   parms;
    uint32_t     error;

    parms.ihandle  = ihandle;
    parms.PADDR  = paddr;
    _mem_copy(laddr, parms.LADDR, 6); 

   error = RTCSCMD_issue(parms, ARPIF_add);

   return error;

   
#else

   return RTCSERR_IP_IS_DISABLED;
   
#endif /* RTCSCFG_ENABLE_IP4 */

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_arp_delete
* Returned Value  : RTCS_OK or error code
* Comments        :
*        Add an entry to an interfaces ARP cache.
*
*END*-----------------------------------------------------------------*/

uint32_t RTCS_arp_delete
   (
      _rtcs_if_handle    *ihandle,
         /* [IN] the RTCS interface state structure */
      _ip_address                paddr
         /* [IN] the address to add */
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   ARPIF_PARM   parms;
   uint32_t     error;

   parms.ihandle  = ihandle;
   parms.PADDR  = paddr;

   error = RTCSCMD_issue(parms, ARPIF_delete);

   return error;
   
#else

   return RTCSERR_IP_IS_DISABLED;
   
#endif /* RTCSCFG_ENABLE_IP4 */


} /* Endbody */


/* EOF */
