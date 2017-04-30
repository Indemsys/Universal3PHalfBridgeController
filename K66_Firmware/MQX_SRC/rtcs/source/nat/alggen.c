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
*   This file contains the generic NAT ALG code.
*
*
*END************************************************************************/
#include <rtcs.h>

#if RTCSCFG_ENABLE_NAT

#include <rtcs_prv.h>
#if MQX_USE_IO_OLD
  #include <fio.h>
#else
  #include <stdio.h>
#endif

#include "nat.h"
#include "nat_prv.h"


/*
** List of function pointers used to free ALG specific data
** Index into table is ALG_TYPE.
*/
NAT_ALG_FREE_FUNC NAT_alg_free_func_table[] =  {
   NULL,                /* NAT_ALG_TFTP_TYPE */
   RTCS_part_free       /* NAT_ALG_FTP_TYPE */
};

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : NAT_init_algs
* Returned Value  : error code
* Comments        :
*        Initializes all of the ALGs in the NAT_alg_table.
*
*END*-----------------------------------------------------------------*/

uint32_t NAT_init_algs
   (
      NAT_CFG_STRUCT_PTR   nat_cfg_ptr     /* [IN] NAT config struct */
   )
{ /* Body */
   NAT_ALG                *table_ptr = NAT_alg_table;
   NAT_ALG_CFG_STRUCT_PTR  cfg_ptr, next_cfg_ptr;
   uint32_t                 error = RTCS_OK;

   while ((*table_ptr != NAT_ALG_ENDLIST) && (error==RTCS_OK))  {
      error = (*table_ptr)(nat_cfg_ptr);
      table_ptr++;
   } /* Endfor */

   if (error != RTCS_OK)  {
      /* An error occured. Free memory associated with ALGs */
      cfg_ptr = nat_cfg_ptr->ALG_INFO_PTR;
      while (cfg_ptr)  {
         next_cfg_ptr = cfg_ptr->NEXT;
         _mem_free(cfg_ptr);
         cfg_ptr = next_cfg_ptr;
      } /* Endwhile */

      nat_cfg_ptr->ALG_INFO_PTR = NULL;
   } /* Endif */

   return error;
} /* Endbody */



/*FUNCTION*-------------------------------------------------------------
*
* Function Name  : NAT_ALG_TCP_checksum
* Returned Value : void
* Comments       : Clears current value in the checksum field and
*                  recalculates it.
*
*END------------------------------------------------------------------*/

void NAT_ALG_TCP_checksum
   (
      IP_HEADER_PTR    ip_header_ptr    /* [IN]  pointer to IP header */
   )
{ /* Body */
   TRANSPORT_UNION    transport;
   uint16_t           checksum;
   uint16_t           protocol;
   uint16_t           iplen = IPH_LEN(ip_header_ptr);
   uint16_t           len = mqx_ntohs(ip_header_ptr->LENGTH) - iplen;

   /* Get TCP header */
   transport.PTR = TRANSPORT_PTR(ip_header_ptr);

   mqx_htons(transport.TCP_PTR->checksum, 0);    /* Clear checksum field */
   protocol = mqx_ntohc(ip_header_ptr->PROTOCOL); /* PROTOCOL */
   checksum = (uint16_t) _mem_sum_ip(protocol, 8, ip_header_ptr->SOURCE);  /* IP SRC and DST ADDR */
   checksum = (uint16_t) _mem_sum_ip(checksum, len, transport.PTR);

   /* TCP LENGTH */
   checksum = IP_Sum_immediate(checksum, len);

   checksum = IP_Sum_invert(checksum);
   mqx_htons(transport.TCP_PTR->checksum, checksum);

} /* Endbody */

#endif

