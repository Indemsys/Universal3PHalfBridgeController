
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
*   This file contains the math functions.
*
*
*END************************************************************************/

#include "mqx_inc.h"


/*!
 * \brief This function divides a 4 32 bit quantity by a 32 bit quantity.
 * 
 * \param[in] n_ptr Pointer to the numerator array (4 long words)
 * \param[in] div The divisor
 * \param[out] r_ptr Pointer to a 4 long word array where the result will be stored
 *
 * \return Returns the remainder from the calculation
 */
uint32_t _psp_div_128_by_32
   (
      /* [IN] Pointer to the numerator array (4 long words) */
      PSP_128_BIT_UNION_PTR  n_ptr,

      /* [IN] The divisor */
      uint32_t                div,

      /* [OUT] Pointer to a 4 long word array where the result will be stored */
      PSP_128_BIT_UNION_PTR  r_ptr

   )
{ /* Body */
   PSP_64_BIT_UNION      d;
   PSP_128_BIT_UNION     tmp;
   uint64_t               w, r;
   _mqx_int              i;

   if (!div) {
      return MAX_UINT_32;
   } /* Endif */
   
   if (div == 1) {
      *r_ptr = *n_ptr;
      return 0;
   } /* Endif */

   if (n_ptr->LLW[0] || n_ptr->LLW[1]) {
      tmp.LLW[0] = tmp.LLW[1] = 0;
#if PSP_ENDIAN == MQX_LITTLE_ENDIAN
      for ( i = 2, r = n_ptr->LW[3]; i >= 0; i-- ) {
         w     = (r << 32) + n_ptr->LW[i];
         r     = w % div;
         d.LLW = w / div;
         tmp.LW[i+1] += d.LW[1];
         tmp.LW[i]   += d.LW[0];
      } /* Endfor */
#else
      for ( i = 1, r = n_ptr->LW[0]; i <= 3; i++ ) {
         w     = (r << 32) + n_ptr->LW[i];
         r     = w % div;
         d.LLW = w / div;
         tmp.LW[i-1] += d.LW[0];
         tmp.LW[i]   += d.LW[1];
      } /* Endfor */
#endif
      *r_ptr = tmp;
      return (uint32_t)r;
   } /* Endif */

   *r_ptr = *n_ptr;

   return 0;

} /* Endbody */


/* EOF */
