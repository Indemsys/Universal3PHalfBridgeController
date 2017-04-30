
/*HEADER**********************************************************************
*
* Copyright 2012 Freescale Semiconductor, Inc.
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
* ARM Nested Vectored Interrupt Controller (NVIC)
*
*
*END************************************************************************/

#include "mqx_inc.h"
#include "nvic.h"

/*!
 * \brief 	Initialize a specific interrupt in the cortex core nvic
 * 
 * \param[in] irq Interrupt number
 * \param[in] prior Interrupt priority 
 * \param[in] enable enable the interrupt now?
 *
 * \return uint32_t MQX_OK or error code
 */
_mqx_uint _nvic_int_init
   (
      // [IN] Interrupt number
      _mqx_uint irq,

      // [IN] Interrupt priority
      _mqx_uint prior,

      // [IN] enable the interrupt now?
      bool enable
   )
{
	VCORTEX_NVIC_STRUCT_PTR nvic = (VCORTEX_NVIC_STRUCT_PTR)&(((CORTEX_SCS_STRUCT_PTR)CORTEX_PRI_PERIPH_IN_BASE)->NVIC);
	_mqx_uint ext_irq_no = irq - 16;

	// check priority value, must be below maximal enabled/set value
	if (prior >= (1 << CORTEX_PRIOR_IMPL)) {
		return MQX_INVALID_PARAMETER;
	}

	if (irq >= PSP_INT_FIRST_INTERNAL && irq <= PSP_INT_LAST_INTERNAL) {
		nvic->PRIORITY[ext_irq_no >> 2] = (nvic->PRIORITY[ext_irq_no >> 2] & ~(0xff << ((ext_irq_no & 3) * 8))) | (((prior << CORTEX_PRIOR_SHIFT) & CORTEX_PRIOR_MASK) << ((ext_irq_no & 3) * 8));

		if (enable)
			_nvic_int_enable(irq);
		else
			_nvic_int_disable(irq);

    }
    else
        return MQX_INVALID_PARAMETER;

    return MQX_OK;
}

/*!
 * \brief Enable interrupt on cortex core NVIC
 * 
 * \param[in] irq Interrupt number 
 *
 * \return uint32_t MQX_OK or error code
 */
_mqx_uint _nvic_int_enable
   (
      // [IN] Interrupt number
      _mqx_uint  irq
   )
{
    VCORTEX_NVIC_STRUCT_PTR nvic = (VCORTEX_NVIC_STRUCT_PTR)&(((CORTEX_SCS_STRUCT_PTR)CORTEX_PRI_PERIPH_IN_BASE)->NVIC);
    uint32_t ext_irq_no = irq - 16;

    if (ext_irq_no >= PSP_INT_FIRST_INTERNAL && ext_irq_no <= PSP_INT_LAST_INTERNAL) {
        nvic->ENABLE[ext_irq_no >> 5] = 1 << (ext_irq_no & 0x1f);
    }
    else
        return MQX_INVALID_PARAMETER;

    return MQX_OK;
}

/*!
 * \brief Disable interrupt on cortex core NVIC
 * 
 * \param[in] irq Interrupt number
 *
 * \return uint32_t MQX_OK or error code
 */
_mqx_uint _nvic_int_disable
   (
      // [IN] Interrupt number
      _mqx_uint  irq
   )
{
    VCORTEX_NVIC_STRUCT_PTR nvic = (VCORTEX_NVIC_STRUCT_PTR)&(((CORTEX_SCS_STRUCT_PTR)CORTEX_PRI_PERIPH_IN_BASE)->NVIC);
    uint32_t ext_irq_no = irq - 16;

    if (ext_irq_no >= PSP_INT_FIRST_INTERNAL && ext_irq_no <= PSP_INT_LAST_INTERNAL) {
        nvic->DISABLE[ext_irq_no >> 5] = 1 << (ext_irq_no & 0x1f);
    }
    else
        return MQX_INVALID_PARAMETER;

    return MQX_OK;
}

/*!
 * \brief Invokes interrupt on cortex core NVIC
 * 
 * \param[in] irq Interrupt number
 *
 * \return uint32_t MQX_OK or error code
 */
_mqx_uint _nvic_int_invoke
   (
      // [IN] Interrupt number
      _mqx_uint  irq
   )
{
    VCORTEX_NVIC_STRUCT_PTR nvic = (VCORTEX_NVIC_STRUCT_PTR)&(((CORTEX_SCS_STRUCT_PTR)CORTEX_PRI_PERIPH_IN_BASE)->NVIC);
    uint32_t ext_irq_no = irq - 16;

    if (ext_irq_no >= PSP_INT_FIRST_INTERNAL && ext_irq_no <= PSP_INT_LAST_INTERNAL) {
        nvic->SET[ext_irq_no >> 5] = 1 << (ext_irq_no & 0x1f);
    }
    else
        return MQX_INVALID_PARAMETER;

    return MQX_OK;
}
