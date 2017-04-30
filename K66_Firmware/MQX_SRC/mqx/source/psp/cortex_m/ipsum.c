
/*HEADER**********************************************************************
*
* Copyright 2010 Freescale Semiconductor, Inc.
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
* See license agreement file for full license terms including other restrictions.
*****************************************************************************
*
* Comments:
*
*   This file contains the implementation for a one's complement checksum.
*   The function can handle any alignment of the memory area 
*   being checksum'd.
*
*
*END************************************************************************/

#include "mqx_inc.h"

/*!
 * \brief Returns a one's complement checksum over a block of memory, as used for 
 *  packets in Internet protocols (see RFC791 for definition). Note This function 
 *  returns 0 iff all summands are 0.
 * 
 * \param sum 
 * \param length 
 * \param loc 
 *
 * \return one's complement checksum
 */
uint32_t _mem_sum_ip
   (
      register uint32_t   sum,
      register uint32_t   length,
      register void   *loc
   )
{ /* Body */
   register uint32_t   total = sum;
   register uint32_t   temp;
   register unsigned char *buf = (unsigned char *)loc;

   if (length & 1) {
      length--;
      total += buf[length] << 8;
   } /* Endif */

   length >>= 1;
   while (length--) {
      temp   = (uint16_t)(*buf);
      total += temp<<8 | buf[1];
      buf   += 2;
   } /* Endwhile */

   sum = ((total >> 16) & 0xFFFF);
   if (sum) {
      total = (total & 0xFFFF) + sum;
      sum = ((total >> 16) & 0xFFFF);
      if (sum) {
         total = (total & 0xFFFF) + sum;
      } /* Endif */
   } /* Endif */

   return total;

}
