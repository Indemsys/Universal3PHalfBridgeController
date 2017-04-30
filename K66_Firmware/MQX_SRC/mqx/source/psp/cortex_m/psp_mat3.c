
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
 * \brief This function adds a single element to an array. r = a[] + val
 * 
 * \param[in] s1_ptr Pointer to the array to be added to
 * \param[in] val The value to add the array
 * \param[in] size The size of the array to add in long words
 * \param[out] res_ptr Pointer to where the result is to be stored
 *
 * \return uint32_t - 1 if overflow, 0 otherwise
 */
uint32_t _psp_add_element_to_array
   (
      /* [IN] Pointer to the array to be added to */
      uint32_t *s1_ptr,

      /* [IN] The value to add the array */
      uint32_t     val,

      /* [IN] The size of the array to add in long words */
      uint32_t     size,

      /* [OUT] Pointer to where the result is to be stored */
      uint32_t *res_ptr

   )
{ /* Body */
   register uint32_t x, y, z, cy;
   register int32_t  j;

#if PSP_ENDIAN == MQX_LITTLE_ENDIAN
   x  = s1_ptr[0];
   y  = x + val;
   cy = (y < val);
   res_ptr[0] = y;
   for ( j = 1; j < size; j++) {
      z = s1_ptr[j];
      x = z + cy;
      res_ptr[j] = x;
      cy = (x < z);
   } /* Endfor */
#else
   x  = s1_ptr[size-1];
   y  = x + val;
   cy = (y < val);
   res_ptr[size-1] = y;
   for ( j = (size-2); j >= 0; j--) {
      z = s1_ptr[j];
      x = z + cy;
      res_ptr[j] = x;
      cy = (x < z);
   } /* Endfor */
#endif
   return cy;

} /* Endbody */

/* EOF */
