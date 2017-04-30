
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
*   This file contains the functions for converting ticks to picoseconds
*
*
*END************************************************************************/

#include "mqx_inc.h"

/*!
 * \brief This function converts ticks into picoseconds, with rounding or truncating
 * 
 * \param[in] tick_ptr Ticks to be converted
 * \param[out] overflow_ptr pointer to where the overflow bool is to be written
 * \param[in] round bool to control rounding or truncating
 *
 * \return uint32_t - number of picoseconds
 */
static uint32_t __psp_ticks_to_picoseconds
   (
      /* [IN] Ticks to be converted */
      PSP_TICK_STRUCT_PTR tick_ptr,

      /* [OUT] pointer to where the overflow bool is to be written */
      bool        *overflow_ptr,
      
      /* [IN] bool to control rounding or truncating */
      bool             round
   )
{ /* Body */
   PSP_128_BIT_UNION      tmp;
   KERNEL_DATA_STRUCT_PTR kernel_data;

   _GET_KERNEL_DATA(kernel_data);

#if (PSP_ENDIAN == MQX_BIG_ENDIAN)
   tmp.LLW[1] = tick_ptr->TICKS[0];
   tmp.LLW[0] = 0;
#else
   tmp.LLW[0] = tick_ptr->TICKS[0];
   tmp.LLW[1] = 0;
#endif

   /* Convert ticks to hardware ticks */
   _psp_mul_128_by_32(&tmp, kernel_data->HW_TICKS_PER_TICK, &tmp);

   /* Add in hardware ticks */
   _psp_add_element_to_array(tmp.LW, tick_ptr->HW_TICKS[0], 
      sizeof(PSP_128_BIT_UNION) / sizeof(uint32_t), 
      tmp.LW);

   /* 
   ** Convert hardware ticks to ps. (H / (T/S * H/T) * 1000000000000)
   ** Multiply by an extra 10 for rounding purposes.
   */
   _psp_mul_128_by_32(&tmp, 1000000000, &tmp);
   _psp_mul_128_by_32(&tmp, 10000, &tmp);
   _psp_div_128_by_32(&tmp, kernel_data->TICKS_PER_SECOND, &tmp);
   _psp_div_128_by_32(&tmp, kernel_data->HW_TICKS_PER_TICK, &tmp);
      /* Round OR Truncate*/
   if (round)
   {
       _psp_add_element_to_array(tmp.LW, 5, sizeof(PSP_128_BIT_UNION) / sizeof(uint32_t), tmp.LW);
   }
   _psp_div_128_by_32(&tmp, 10, &tmp);

#if (PSP_ENDIAN == MQX_BIG_ENDIAN)
   *overflow_ptr = (bool)(tmp.LLW[0] || tmp.LW[2]);
   return tmp.LW[3];
#else
   *overflow_ptr = (bool)(tmp.LLW[1] || tmp.LW[1]);
   return tmp.LW[0];
#endif

} /* Endbody */
/*!
 * \brief This function converts ticks into picoseconds, with rounding
 * 
 * \param[in] tick_ptr Ticks to be converted
 * \param[out] overflow_ptr pointer to where the overflow bool is to be written
 *
 * \return uint32_t - number of picoseconds
 */
uint32_t _psp_ticks_to_picoseconds
   (
      /* [IN] Ticks to be converted */
      PSP_TICK_STRUCT_PTR tick_ptr,

      /* [OUT] pointer to where the overflow bool is to be written */
      bool        *overflow_ptr
   )
{ /* Body */
    return __psp_ticks_to_picoseconds(tick_ptr, overflow_ptr, TRUE);
}

/*!
 * \brief This function converts ticks into picoseconds, with truncating
 * 
 * \param[in] tick_ptr Ticks to be converted
 * \param[out] overflow_ptr pointer to where the overflow bool is to be written
 *
 * \return uint32_t - number of picoseconds
 */
uint32_t _psp_ticks_to_picoseconds_truncate
   (
      /* [IN] Ticks to be converted */
      PSP_TICK_STRUCT_PTR tick_ptr,

      /* [OUT] pointer to where the overflow bool is to be written */
      bool        *overflow_ptr
   )
{ /* Body */
   return __psp_ticks_to_picoseconds(tick_ptr, overflow_ptr, FALSE);
}
/* EOF */
