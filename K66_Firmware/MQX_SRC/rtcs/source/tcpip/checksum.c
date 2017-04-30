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
*   This file contains the implementation for a one's
*   complement checksum.
*
*
*END************************************************************************/

#include <rtcs.h>
#include "rtcs_prv.h"
#include "tcpip.h"
#include "ip_prv.h"


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IP_Sum_immediate
* Returned Value  : one's complement checksum
* Comments        :
*     Returns a one's complement checksum of two numbers.
*
*     Note:  This function returns 0 iff both summands are 0.
*
*END*-----------------------------------------------------------------*/

uint16_t IP_Sum_immediate
   (
      uint16_t        sum,
         /* [IN] initial sum */
      uint16_t        immediate
         /* [IN] number to add to sum */
   )
{ /* Body */
   uint32_t total = sum & 0xFFFF;

   total += immediate & 0xFFFF;

   sum = (total >> 16) & 0xFFFF;
   total = (total & 0xFFFF) + sum;
   return total;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IP_Sum_PCB
* Returned Value  : one's complement checksum
* Comments        :
*     Returns a one's complement checksum over all fragments in a PCB.
*
*     Note:  This function returns 0 iff all summands are 0.
*
*END*-----------------------------------------------------------------*/

uint16_t IP_Sum_PCB
   (
      uint16_t        sum,
         /* [IN] initial sum */
      RTCSPCB_PTR    pcb
         /* [IN] the PCB */
   )
{ /* Body */
   PCB_FRAGMENT_PTR  frag = &pcb->PCBPTR->FRAG[1];
   uint32_t           is_odd = pcb->HEADER_FRAG_USED & 1;

   sum = _mem_sum_ip(sum, pcb->HEADER_FRAG_USED, RTCSPCB_DATA(pcb));

   while (frag->LENGTH) {
      if (is_odd) {
         /* If previous fragment was odd, add in first byte in lower 8 bits */
         sum = IP_Sum_immediate(sum, frag->FRAGMENT[0] & 0xFF);
      } /* Endif */

      sum = _mem_sum_ip(sum, frag->LENGTH - is_odd, frag->FRAGMENT + is_odd);
      is_odd = (frag->LENGTH - is_odd) & 1;
      frag++;
   } /* Endwhile */

   return sum;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IP_Sum_pseudo
* Returned Value  : one's complement checksum
* Comments        :
*     Returns a one's complement checksum of the IP pseudo header
*
*     Note:  This function returns 0 iff all summands are 0.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

uint16_t IP_Sum_pseudo
   (
      uint16_t        sum,
         /* [IN] initial sum */
      RTCSPCB_PTR    pcb,
         /* [IN] the PCB */
      int32_t         layer
         /* [IN] IP layer, relative to current */
   )
{ /* Body */
   IP_HEADER_PTR iphead = (IP_HEADER_PTR)RTCSPCB_DATA_NETWORK(pcb);
   uint16_t protocol;

   protocol = RTCSPCB_TRANSPORT_PROTL(pcb);
   sum = _mem_sum_ip(sum, 8, iphead->SOURCE);     /* source and dest */
   sum = IP_Sum_immediate(sum, protocol);    /* protocol */
   return sum;

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/* EOF */
