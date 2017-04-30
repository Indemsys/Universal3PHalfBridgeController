
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
*   This file contains the function for initializing
*   the handling of interrupts.
*
*
*END************************************************************************/

#include "mqx_inc.h"

/*!
 * \brief This function initializes the kernel interrupt tables as well as the low 
 *  level interrupt table.
 * 
 * \param[in] first_user_isr_vector_number the first (lower) user ISR vector number
 * \param[in] last_user_isr_vector_number the last user ISR vector number
 */
_mqx_uint _psp_int_init
   (
      /* [IN] the first (lower) user ISR vector number */
      _mqx_uint       first_user_isr_vector_number,

      /* [IN] the last user ISR vector number */
      _mqx_uint       last_user_isr_vector_number
   )
{ /* Body */
   uint32_t            error;

   /* Install kernel interrupt services */
   error = _int_init(first_user_isr_vector_number, last_user_isr_vector_number);

   /* Install PSP interrupt services */
   if (error == MQX_OK) {
      _psp_int_install();
   } /* Endif */

   return error;

} /* Endbody */

/* EOF */
