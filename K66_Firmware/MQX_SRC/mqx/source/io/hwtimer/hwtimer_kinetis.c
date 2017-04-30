
/*HEADER**********************************************************************
*
* Copyright 2013 Freescale Semiconductor, Inc.
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
*   This file contains kinetis specific functions of the hwtimer component.
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>
#include "hwtimer.h"

/*!
 * \cond DOXYGEN_PRIVATE
 * Macro return number of item in pit_vectors_table array
 */
#define PIT_INTERRUPT_COUNT 4

/* Array of PIT interrupt vectors*/
/*!
 * \cond DOXYGEN_PRIVATE
 * \brief Array of PIT interrupt vectors
 */
const _mqx_uint pit_vectors_table[] =
    {
        INT_PIT0,
        INT_PIT1,
        INT_PIT2,
        INT_PIT3
    };

/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief This function performs BSP-specific initialization related to pit
 *
 * \param dev_num[in]   Number of PIT module.
 *
 * \return MQX_OK Success.
 *
 * \see pit_get_vectors
 * \see pit_get_hwtimers_array
 */
_mqx_int pit_io_init
(
    uint32_t dev_num
)
{
    switch (dev_num)
    {
        case 0:
            SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;    /* PIT clock enablement */
            break;
        default:
            /* Do nothing if bad dev_num was selected */
            return -1;
    }

    return MQX_OK;
}

/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief This function get array of vectors and count of items in this array.
 *
 * \param pit_vectors_table_ptr[out]  Used to get pit_vectors_table.
 *
 * \return PIT_INTERRUPT_COUNT Count of interrupt vectors for PIT module.
 *
 * \see pit_io_init
 * \see pit_get_hwtimers_array
 */
uint32_t pit_get_vectors
(
    uint32_t pit_number,
    const _mqx_uint **pit_vectors_table_ptr
)
{
    switch (pit_number)
    {
        case 0:
            *pit_vectors_table_ptr = pit_vectors_table;
            break;
        default:
            *pit_vectors_table_ptr = NULL;
    }

    return PIT_INTERRUPT_COUNT;
}

/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief This function is useless for this platform
 *
 * \param hwtimers_array[out] Array of hwtimers. Filled by NULL for this platform
 *
 * \return 0 This platform doesnt need hwtimers_array.
 *
 * \see pit_io_init
 * \see pit_get_vectors
 */
uint32_t pit_get_hwtimers_array
(
    HWTIMER_PTR ** hwtimers_array
)
{
    *hwtimers_array = NULL;
    return 0;
}
/*******************SysTick********************/

/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief This function get Interrupt Number.
 *
 * \param pit_vectors_table_ptr[out]  Used to get pit_vectors_table.
 *
 * \return Interrupt Number for SysTick module.
 *
 */
uint32_t systick_get_vector()
{
    return INT_SysTick;
}

/*******************LPT********************/

/* Array of LPT base addresses*/
/*!
 * \cond DOXYGEN_PRIVATE
 * \brief Array of LPT base addresses
 */
static const void *lpt_address[] =
{
    (void *)LPTMR0_BASE_PTR
};

/* Array of LPT interrupt vectors*/
/*!
 * \cond DOXYGEN_PRIVATE
 * \brief Array of LPT interrupt vectors
 */
static const uint32_t lpt_vectors[] =
{
    INT_LPTimer
};

/* Array of LPT clock sources */
/*!
 * \cond DOXYGEN_PRIVATE
 * \brief Array of LPT interrupt vectors
 */
const int32_t lpt_pcsclk[] =
{
    BSP_HWTIMER_LPT0_DEFAULT_PCSCLK
};

/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief This function get base address.
 *
 * \param dev_num[in]  LPT module.
 *
 * \return base address of LPT module.
 *
 */
void *lpt_get_base_address
    (
        /* [IN] LPT index */
        uint8_t dev_num
    )
{
    if (dev_num < ELEMENTS_OF(lpt_address)) 
    {
        return (void *)lpt_address[dev_num];
    }
    return NULL;
}

/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief This function get Interrupt Number.
 *
 * \param dev_num[in]  LPT module.
 *
 * \return Interrupt Number for LPT module.
 *
 */
uint32_t lpt_get_vector
    (
        /* [IN] LPT index */
        uint8_t dev_num
    )
{
    if (dev_num < ELEMENTS_OF(lpt_vectors)) 
    {
        return lpt_vectors[dev_num];
    }
    else
    {
        return 0;
    }
}

/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief This function performs BSP-specific initialization related to LPT
 *
 * \param dev_num[in]   Number of LPT module.
 *
 * \return MQX_OK Success.
 */
_mqx_int lpt_io_init
(
    uint32_t dev_num
)
{
    if (dev_num < ELEMENTS_OF(lpt_address)) 
    {
        SIM_SCGC5 |= SIM_SCGC5_LPTMR_MASK;
        return MQX_OK;
    }
    return MQX_INVALID_DEVICE;
}

/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief This function check validation of input LPT module
 *
 * \param dev_num[in]   Number of LPT module.
 * \param isr_prior[in] ISR priority.
 *
 * \return MQX_OK Valid.
 */
_mqx_int lpt_validate_module
(
    uint32_t dev_num,
    uint32_t isr_prior
)
{
    if (dev_num >= ELEMENTS_OF(lpt_address)) 
    {
        return MQX_INVALID_DEVICE;
    }
    
    /* Check ISR priority */
    if (0 == CORTEX_PRIOR(isr_prior))
    {
        return MQX_INVALID_PARAMETER;
    }
    
    return MQX_OK;
}


