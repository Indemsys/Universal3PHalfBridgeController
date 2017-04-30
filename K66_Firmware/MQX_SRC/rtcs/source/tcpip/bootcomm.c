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
*   This file contains the implementation of the
*   functions common to BOOTP and DHCP clients.
*
*
*END************************************************************************/

#include <rtcs.h>

#if RTCSCFG_ENABLE_IP4

#include "rtcs_prv.h"
#include "tcpip.h"
#include "ip_prv.h"


extern uint32_t          BOOT_count;
extern UCB_STRUCT_PTR   BOOT_port;


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : BOOT_open
*  Returned Value : RTCS_OK or error code
*  Comments       :
*        Opens the BOOTP/DHCP client port.
*
*END*-----------------------------------------------------------------*/

uint32_t BOOT_open
   (
      void (_CODE_PTR_ service)(RTCSPCB_PTR, UCB_STRUCT_PTR)
   )
{ /* Body */
   uint32_t error;

   if (BOOT_count == 0) {
      error = UDP_openbind_internal(IPPORT_BOOTPC, service, &BOOT_port);
      if (error) {
         return error;
      } /* Endif */
   } /* Endif */

   BOOT_count++;
   return RTCS_OK;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : BOOT_close
*  Returned Value : RTCS_OK or error code
*  Comments       :
*        Closes the BOOTP/DHCP client port.
*
*END*-----------------------------------------------------------------*/

uint32_t BOOT_close
   (
      void
   )
{ /* Body */
   uint32_t error = RTCS_OK;

   BOOT_count--;

   if (BOOT_count == 0) {
      error = UDP_close_internal(BOOT_port);
      BOOT_port = NULL;
   } /* Endif */

   return error;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : BOOT_send
*  Returned Value : void
*  Comments       :
*        Sends a BOOTP/DHCP packet.
*
*END*-----------------------------------------------------------------*/

uint32_t BOOT_send
   (
      RTCSPCB_PTR    rtcs_pcb,         /* [IN]     outgoing packet  */
      void          *if_ptr            /* [IN]     target interface */
   )
{ /* Body */

   return UDP_send_IF(BOOT_port, if_ptr, IPPORT_BOOTPS, rtcs_pcb);

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : BOOT_service
*  Returned Value : void
*  Comments       :
*        Services a BOOTP/DHCP packet.
*
*END*-----------------------------------------------------------------*/

void BOOT_service
   (
      RTCSPCB_PTR    rtcs_pcb,         /* [IN/OUT] incoming packet  */
      UCB_STRUCT_PTR ucb_ptr           /* [IN]     target UCB       */
   )
{ /* Body */
   IP_IF_PTR if_ptr = (IP_IF_PTR)rtcs_pcb->IFSRC;

   if (if_ptr->BOOTFN) {
      (*if_ptr->BOOTFN)(rtcs_pcb);
   } else {
      RTCSLOG_PCB_FREE(rtcs_pcb, RTCS_OK);
      RTCSPCB_free(rtcs_pcb);
   } /* Endif */

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/* EOF */
