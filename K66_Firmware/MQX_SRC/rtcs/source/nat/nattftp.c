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
*   This file contains the NAT TFTP ALG.
*
*
*END************************************************************************/
#include <rtcs.h>

#if RTCSCFG_ENABLE_NAT

#if MQX_USE_IO_OLD
  #include <fio.h>
#else
  #include <stdio.h>
#endif
#include <rtcs_prv.h>
#include <tftp_prv.h>


#include "nat.h"
#include "nat_prv.h"



/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : NAT_ALG_tftp
* Returned Value  : error code
* Comments        :
*        Initializes the NAT TFTP ALG.
*
*END*-----------------------------------------------------------------*/

uint32_t NAT_ALG_tftp
   (
      void   *ptr       /* [IN] pointer to NAT config struct */
   )
{ /* Body */
   NAT_CFG_STRUCT_PTR      nat_cfg_ptr = ptr;
   NAT_ALG_CFG_STRUCT_PTR  tftp_ptr;

   tftp_ptr = _mem_alloc_system(sizeof(NAT_ALG_CFG_STRUCT));

   if (tftp_ptr)  {
      tftp_ptr->NEXT = nat_cfg_ptr->ALG_INFO_PTR;
      nat_cfg_ptr->ALG_INFO_PTR = tftp_ptr;
      tftp_ptr->ALG_TYPE = NAT_ALG_TFTP_TYPE;
      tftp_ptr->ALG_EXEC = NAT_ALG_tftp_apply;
      return RTCS_OK;
   } /* Endif */

   return RTCSERR_OUT_OF_MEMORY;

} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : NAT_ALG_tftp_apply
* Returned Value  : error code
* Comments        :
*        Applies the NAT TFTP ALG to a packet.
*
*END*-----------------------------------------------------------------*/

uint32_t NAT_ALG_tftp_apply
   (
      RTCSPCB_PTR     *pcb_ptr_ptr,               /* [IN/OUT] PCB containing packet   */
      bool              pub_to_prv,                /* [IN] Direction of packet         */
      void                *alg_cfg_ptr,               /* [IN] Pointer to TFTP config      */
      void           **session_ptr_ptr   /* [OUT] Pointer to session         */
   )
{ /* Body */
   NAT_SESSION_STRUCT_PTR  nat_session_ptr = *((NAT_SESSION_STRUCT_PTR *)session_ptr_ptr);
   unsigned char               *data_ptr = (void *)RTCSPCB_DATA(*pcb_ptr_ptr);
   IP_HEADER_PTR           ip_header_ptr = (IP_HEADER_PTR)(void *)data_ptr;
   TRANSPORT_UNION         transport;
   TFTP_HEADER_PTR         tftp_header_ptr;
   uint16_t                 opcode, block;

   if (nat_session_ptr == NULL) {
      return RTCS_OK;
   } /* Endif */


   /* TFTP uses UDP */
   if (mqx_ntohc(ip_header_ptr->PROTOCOL) != IPPROTO_UDP) {
      return RTCS_OK;
   } /* Endif */

   transport.PTR = TRANSPORT_PTR(ip_header_ptr);

   /* The UDP length should be long enough to contain a TFTP header */
   if (mqx_ntohs(transport.UDP_PTR->LENGTH) < sizeof(UDP_HEADER) + sizeof(TFTP_HEADER)) {
      return RTCS_OK;
   } /* Endif */

   tftp_header_ptr = (TFTP_HEADER_PTR)(void *)(transport.UDP_PTR + 1);

   opcode = mqx_ntohs(tftp_header_ptr->OP);
   block = mqx_ntohs(tftp_header_ptr->BLOCK);

   /*
   ** A normal reply should have:  OP 3 (DATA) and BLOCK 1 (first block) or
   ** an OP 4 and BLOCK 0. Error replies should have: OP 5, and a variable
   ** error code
   */
   if ((opcode != 3 || block != 1) &&
       (opcode != 4 || block != 0) &&
       (opcode != 5))
   {
      return RTCS_OK;
   } /* Endif */

   if (nat_session_ptr->SNAT_OR_DNAT == SNAT) {
      /* We only need to consider the first packet returned by the server */
      if (!pub_to_prv) {
         return RTCS_OK;
      } /* Endif */

      /* Make sure port number is the TFTP port number */
      if (nat_session_ptr->PUB_PORT != IPPORT_TFTP) {
         return RTCS_OK;
      } /* Endif */

      /* Set new server port number */
      nat_session_ptr->PUB_PORT = mqx_ntohs(transport.UDP_PTR->SRC_PORT);
   } else {
      /* We only need to consider the first packet returned by the server */
      if (pub_to_prv){
         return RTCS_OK;
      } /* Endif */

      /* Make sure port number is the TFTP port number */
      if (nat_session_ptr->PRV_PORT != IPPORT_TFTP) {
         return RTCS_OK;
      } /* Endif */

      /* Set new server port number */
      nat_session_ptr->PRV_PORT = mqx_ntohs(transport.UDP_PTR->SRC_PORT);
   } /* Endif */

   return RTCS_OK;
} /* Endbody */

#endif

/* EOF */
