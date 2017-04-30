
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
 * \brief This function multiplies a 128 bit quantity by a 32 bit quantity and stores 
 *  the result in a 128 bit quantity
 * 
 * \param[in] m_ptr Pointer to a 4 long word array in which the value to be multiplied is stored
 * \param[in] mul The multiplier
 * \param[out] r_ptr Pointer to a 4 long word array where the result will be stored
 *
 * \return uint32_t - overflow
 */
uint32_t _psp_mul_128_by_32
   (
      /* 
      ** [IN] Pointer to a 4 long word array in which the value to be multiplied
      ** is stored 
      */
      PSP_128_BIT_UNION_PTR   m_ptr,

      /* [IN] The multiplier */
      uint32_t                 mul,

      /* [OUT] Pointer to a 4 long word array where the result will be stored */
      PSP_128_BIT_UNION_PTR   r_ptr

   )
{ /* Body */
   PSP_128_BIT_UNION tmp;
   uint64_t           w,r;
   uint32_t           w0;
   _mqx_uint         i;

#if PSP_ENDIAN == MQX_LITTLE_ENDIAN
   tmp.LLW[0] = 0;
   r = 0;
   if (!mul || (!m_ptr->LLW[0] && !m_ptr->LLW[1])) {
      tmp.LLW[1] = 0;
   } else if (mul == 1) {
      *r_ptr = *m_ptr;
      return r;
   } else {
      for ( i = 0; i < 3; i++ ) {
         w  = (uint64_t)mul * (uint64_t)m_ptr->LW[i];
         w0 = (uint32_t)w;
         tmp.LW[i] += w0;
         tmp.LW[i+1] = (w >> 32) + (tmp.LW[i] < w0);
      } /* Endfor */

      w = (uint64_t)mul * (uint64_t)m_ptr->LW[3];
      w0 = (uint32_t)w;
      tmp.LW[3] += w0;
      r = (w >> 32) + (tmp.LW[3] < w0);
   } /* Endif */
   
   *r_ptr = tmp;
   return r;
#else
   tmp.LLW[1] = 0;
   r = 0;
   if (!mul || (!m_ptr->LLW[0] && !m_ptr->LLW[1])) {
      tmp.LLW[0] = 0;
   } else if (mul == 1) {
      *r_ptr = *m_ptr;
      return r;
   } else {
      for ( i = 3; i > 0; i-- ) {
         w  = (uint64_t)mul * (uint64_t)m_ptr->LW[i];
         w0 = (uint32_t)w;
         tmp.LW[i] += w0;
         tmp.LW[i-1] = (w >> 32) + (tmp.LW[i] < w0);
      } /* Endfor */

      w = (uint64_t)mul * (uint64_t)m_ptr->LW[0];
      w0 = (uint32_t)w;
      tmp.LW[0] += w0;
      r = (w >> 32) + (tmp.LW[0] < w0);
   } /* Endif */
   
   *r_ptr = tmp;
   return r;
#endif

} /* Endbody */

/* EOF */
