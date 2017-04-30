
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
*   This file contains the functions for zeroing memory
*
*
*END************************************************************************/

#include "mqx_inc.h"


/*!
 * \brief This function zeros the specified number of bytes at the specified location. 
 *  The zeroing is optimized to avoid alignment problems, and attempts to zero 
 *  32bit numbers optimally
 * 
 * \param[in] from_ptr the address to start zeroing memory from
 * \param[in] number_of_bytes the number of bytes to zero
 */
#if MQX_USE_MEM_ZERO

void _mem_zero
   (
      /* [IN] the address to start zeroing memory from */
      register void *from_ptr,

      /* [IN] the number of bytes to zero */
      register _mem_size number_of_bytes
   )
{ /* Body */
#if MQX_USE_SMALL_MEM_ZERO
   register uint8_t *from8_ptr = (uint8_t *) from_ptr;

   if (number_of_bytes) {
      while (number_of_bytes--) {
         *from8_ptr++ = 0;
      } /* Endwhile */
   } /* Endif */
#else
   uint8_t *from8_ptr = (uint8_t *) from_ptr;
   uint16_t *from16_ptr = (uint16_t *) from_ptr;
   register uint32_t *from32_ptr = (uint32_t *) from_ptr;
   register uint32_t loops;

   if (number_of_bytes > 3) {

      /* Try to align source on word */
      if ((uint32_t)from_ptr & 1) {
         from8_ptr = (uint8_t *) from_ptr;
         *from8_ptr++ = 0;

         from_ptr = from8_ptr;
         --number_of_bytes;
      } /* Endif */

      /* Try to align source on longword */
      if ((uint32_t)from_ptr & 2) {
         from16_ptr = (uint16_t *) from_ptr;

         *from16_ptr++ = 0;

         from_ptr = from16_ptr;
         number_of_bytes -= 2;
      } /* Endif */

      from32_ptr = (uint32_t *) from_ptr;
#if MQX_USE_BLOCK_MEM_ZERO
      /*
      ** so lets copy longwords...
      ** to increase performance, we will do 64 bytes (16 * longwords) at once and after that jumping
      ** This consumes more RAM, more flash, but gets less instruction cycles.
      */
      for (loops = number_of_bytes >> 6; loops != 0; loops--) {
         /* copy 16 longwords */
         *from32_ptr++ = 0;
         *from32_ptr++ = 0;
         *from32_ptr++ = 0;
         *from32_ptr++ = 0;
         *from32_ptr++ = 0;
         *from32_ptr++ = 0;
         *from32_ptr++ = 0;
         *from32_ptr++ = 0;
         *from32_ptr++ = 0;
         *from32_ptr++ = 0;
         *from32_ptr++ = 0;
         *from32_ptr++ = 0;
         *from32_ptr++ = 0;
         *from32_ptr++ = 0;
         *from32_ptr++ = 0;
         *from32_ptr++ = 0;
      } /* Endwhile */

      /* Now, write the rest of bytes */
      switch ((number_of_bytes >> 2) & 0xF) {
         case 15: *from32_ptr++ = 0;
         case 14: *from32_ptr++ = 0;
         case 13: *from32_ptr++ = 0;
         case 12: *from32_ptr++ = 0;
         case 11: *from32_ptr++ = 0;
         case 10: *from32_ptr++ = 0;
         case 9:  *from32_ptr++ = 0;
         case 8:  *from32_ptr++ = 0;
         case 7:  *from32_ptr++ = 0;
         case 6:  *from32_ptr++ = 0;
         case 5:  *from32_ptr++ = 0;
         case 4:  *from32_ptr++ = 0;
         case 3:  *from32_ptr++ = 0;
         case 2:  *from32_ptr++ = 0;
         case 1:  *from32_ptr++ = 0;
      } /* Endswitch */

#else /* MQX_USE_BLOCK_MEM_ZERO */
      for (loops = number_of_bytes >> 2; loops != 0; loops--) {
         *from32_ptr++ = 0;
      }
#endif /* MQX_USE_BLOCK_MEM_ZERO */
      from_ptr = from32_ptr;
   } /* Endif */

   /* Copy all remaining bytes */
   if (number_of_bytes & 2) {
      from16_ptr = (uint16_t *) from_ptr;

      *from16_ptr++ = 0;

      from_ptr = from16_ptr;
   } /* Endif */
   if (number_of_bytes & 1) {
      * (uint8_t *) from_ptr = 0;
   } /* Endif */
#endif
} /* Endbody */

#endif

/* EOF */

