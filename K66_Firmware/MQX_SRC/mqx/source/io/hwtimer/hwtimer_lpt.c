
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
*   This file contains functions of the low level PIT module for HWTIMER
*   component.
*
*
*END************************************************************************/

#include "hwtimer.h"
#include "hwtimer_lpt.h"
#include <bsp.h>

extern void *lpt_get_base_address(uint8_t);
extern uint32_t lpt_get_vector(uint8_t);
extern _mqx_int lpt_io_init(uint32_t);
extern _mqx_int lpt_validate_module(uint32_t, uint32_t);

static void hwtimer_lpt_isr(void *);
static _mqx_int hwtimer_lpt_init(HWTIMER_PTR , uint32_t, uint32_t);
static _mqx_int hwtimer_lpt_deinit(HWTIMER_PTR);
static _mqx_int hwtimer_lpt_set_div(HWTIMER_PTR, uint32_t);
static _mqx_int hwtimer_lpt_start(HWTIMER_PTR);
static _mqx_int hwtimer_lpt_stop(HWTIMER_PTR);
static _mqx_int hwtimer_lpt_get_time(HWTIMER_PTR , HWTIMER_TIME_PTR);


const HWTIMER_DEVIF_STRUCT lpt_devif =
{
    hwtimer_lpt_init,
    hwtimer_lpt_deinit,
    hwtimer_lpt_set_div,
    hwtimer_lpt_start,
    hwtimer_lpt_stop,
    hwtimer_lpt_get_time
};

extern const int32_t lpt_pcsclk[];

/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief Interrupt service routine.
 *
 * This ISR is used when LPT counted to 0.
 * Checks whether callback_func is not NULL,
 * and unless callback is blocked by callback_blocked being non-zero it calls the callback function with callback_data as parameter,
 * otherwise callback_pending is set to non-zero value.
 *
 * \param p[in]   Pointer to HWTIMER structure.
 *
 * \return void
 *
 * \see hwtimer_lpt_deinit
 * \see hwtimer_lpt_set_div
 * \see hwtimer_lpt_start
 * \see hwtimer_lpt_stop
 * \see hwtimer_lpt_get_time
 */
static void hwtimer_lpt_isr(void *p)
{
    HWTIMER_PTR hwtimer     = (HWTIMER_PTR) p;
    LPTMR_MemMapPtr lpt_ptr = (LPTMR_MemMapPtr) hwtimer->ll_context[0];

    /* Check if interrupt is enabled for this LPT. Cancel spurious interrupt */
    if (!(lpt_ptr->CSR & LPTMR_CSR_TIE_MASK))
    {
        return;
    }

    /* Clear interrupt flag */
    lpt_ptr->CSR |= LPTMR_CSR_TCF_MASK;

    /* Following part of function is typically the same for all low level HWTIMER drivers */
    hwtimer->ticks++;

    if (NULL != hwtimer->callback_func)
    {
        if (hwtimer->callback_blocked)
        {
            hwtimer->callback_pending = 1;
        }
        else
        {
            /* Run user function*/
            hwtimer->callback_func(hwtimer->callback_data);
        }
    }
}


/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief This function initializes caller allocated structure according to given
 * numerical identifier of the timer.
 *
 * Called by hwtimer_init().
 * Initializes the HWTIMER structure.
 * Sets interrupt priority and registers ISR.
 *
 * \param hwtimer[in]   Returns initialized HWTIMER structure handle.
 * \param lpt_id[in]    Determines LPT module.
 * \param isr_prior[in] Interrupt priority for LPT
 *
 * \return MQX_OK                       Success.
 * \return MQX_INVALID_PARAMETER        When lpt_id does not exist(When lpt_id is not zero).
 * \return -1                           When LPT is used by PE or when _int_install_isr failed.
 *
 * \see hwtimer_lpt_deinit
 * \see hwtimer_lpt_set_div
 * \see hwtimer_lpt_start
 * \see hwtimer_lpt_stop
 * \see hwtimer_lpt_get_time
 * \see hwtimer_lpt_isr
 */
static _mqx_int hwtimer_lpt_init(HWTIMER_PTR hwtimer, uint32_t lpt_id, uint32_t isr_prior)
{
    IRQInterruptIndex   vector;
    LPTMR_MemMapPtr     lpt_ptr;

    /* We count only with one LPT module inside core */
#if MQX_CHECK_ERRORS
    if (lpt_validate_module(lpt_id, isr_prior) != MQX_OK)
    {
        return MQX_INVALID_PARAMETER;
    }
#endif

    /* We need to store LPT base in context structure */
    hwtimer->ll_context[0] = (uint32_t)lpt_get_base_address(lpt_id);
    hwtimer->ll_context[1] = lpt_id;
    lpt_ptr = (LPTMR_MemMapPtr) hwtimer->ll_context[0];
    if (NULL == lpt_ptr)
    {
        return MQX_INVALID_PARAMETER;
    }

#if PE_LDD_VERSION
    if (PE_PeripheralUsed((uint32_t)lpt_ptr))
    {
        return -1;
    }
#endif

    if (MQX_OK != lpt_io_init(lpt_id))
    {
        return MQX_INVALID_PARAMETER;
    }

    if (MQX_OK != hwtimer_lpt_set_clksrc(hwtimer, lpt_pcsclk[lpt_id]))
    {
        return MQX_INVALID_PARAMETER;
    }

    /* Disable LPT */
    lpt_ptr->CSR &= (~ LPTMR_CSR_TEN_MASK);
    /* Disable LPT interrupt */
    lpt_ptr->CSR &= (~ LPTMR_CSR_TIE_MASK);
    /* Clear interrupt flag */
    if (lpt_ptr->CSR & LPTMR_CSR_TCF_MASK)
    {
        lpt_ptr->CSR |= LPTMR_CSR_TCF_MASK;
    }

    /* Set ISR for timer*/
    vector = (IRQInterruptIndex) lpt_get_vector(lpt_id);
    if (NULL == _int_install_isr(vector, (INT_ISR_FPTR) hwtimer_lpt_isr, (void *) hwtimer))
    {
        return -1;
    }
    _bsp_int_init (vector, isr_prior, 0, TRUE);
    _bsp_int_enable (vector);

    return MQX_OK;
}

/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief Initialization of LPT timer module
 *
 * Called by hwtimer_deinit.
 * Disables the peripheral.
 * Unregisters ISR.
 *
 * \param hwtimer[in] Pointer to HWTIMER structure.
 *
 * \return MQX_OK                       Success.
 * \return MQX_INVALID_COMPONENT_HANDLE When doesn't have any interrupt vectors, or LPT does not exist.
 *
 * \see hwtimer_lpt_init
 * \see hwtimer_lpt_set_div
 * \see hwtimer_lpt_start
 * \see hwtimer_lpt_stop
 * \see hwtimer_lpt_get_time
 * \see hwtimer_lpt_isr
 */
static _mqx_int hwtimer_lpt_deinit(HWTIMER_PTR hwtimer)
{
    IRQInterruptIndex vector;
    uint32_t lpt_id  = (uint32_t)hwtimer->ll_context[1];

    /* Every channel has own interrupt vector */
    vector = (IRQInterruptIndex)lpt_get_vector(lpt_id);
    if (vector == 0)
    {
        return MQX_INVALID_PARAMETER;
    }

    /* Disable interrupt on vector */
    _bsp_int_disable(vector);
    /* Install default ISR routine for our LPT */
    _int_install_isr(vector, _int_get_default_isr(), NULL);

    return MQX_OK;
}

/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief Sets up timer with divider settings closest to the requested total divider factor.
 *
 * Called by hwtimer_set_freq() and hwtimer_set_period().
 * Fills in the divider (actual total divider) and modulo (sub-tick resolution) members of the HWTIMER structure.
 *
 * \param hwtimer[in] Pointer to HWTIMER structure.
 * \param divider[in] Value which divide input clock of LPT timer module to obtain requested period of timer.
 *
 * \return MQX_OK                Success.
 * \return MQX_INVALID_PARAMETER Divider is equal to zero.
 *
 * \see hwtimer_lpt_init
 * \see hwtimer_lpt_deinit
 * \see hwtimer_lpt_start
 * \see hwtimer_lpt_stop
 * \see hwtimer_lpt_get_time
 * \see hwtimer_lpt_isr
 */
static _mqx_int hwtimer_lpt_set_div(HWTIMER_PTR hwtimer, uint32_t divider)
{
    LPTMR_MemMapPtr lpt_ptr = (LPTMR_MemMapPtr)hwtimer->ll_context[0];
    uint32_t cmr = 0;

#if MQX_CHECK_ERRORS
    if (0 == divider)
    {
        return MQX_INVALID_PARAMETER;
    }
#endif

    if (NULL == lpt_ptr)
    {
        return MQX_INVALID_PARAMETER;
    }

    cmr = LPTMR_CMR_COMPARE(divider - 1);
    lpt_ptr->CMR = cmr;

    hwtimer->divider = divider;
    hwtimer->modulo  = divider;

    return MQX_OK;
}


/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief Start LPT timer module
 *
 * This function enables the timer and leaves it running, timer is
 * periodically generating interrupts.
 *
 * \param hwtimer[in] Pointer to HWTIMER structure.
 *
 * \return MQX_OK Success.
 *
 * \see hwtimer_lpt_init
 * \see hwtimer_lpt_deinit
 * \see hwtimer_lpt_set_div
 * \see hwtimer_lpt_stop
 * \see hwtimer_lpt_get_time
 * \see hwtimer_lpt_isr
 */
static _mqx_int hwtimer_lpt_start(HWTIMER_PTR hwtimer)
{
    LPTMR_MemMapPtr lpt_ptr = (LPTMR_MemMapPtr)hwtimer->ll_context[0];

    if (NULL == lpt_ptr)
    {
        return MQX_INVALID_PARAMETER;
    }

    /* Enable LPT interrupt */
    lpt_ptr->CSR |= LPTMR_CSR_TIE_MASK;
    /* Enable LPT */
    lpt_ptr->CSR |= LPTMR_CSR_TEN_MASK;

    return MQX_OK;
}

/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief Stop LPT timer module
 *
 * Disable timer and interrupt
 *
 * \param hwtimer[in] Pointer to HWTIMER structure.
 *
 * \return MQX_OK Success.
 *
 * \see hwtimer_lpt_init
 * \see hwtimer_lpt_deinit
 * \see hwtimer_lpt_set_div
 * \see hwtimer_lpt_start
 * \see hwtimer_lpt_get_time
 * \see hwtimer_lpt_isr
 */
static _mqx_int hwtimer_lpt_stop(HWTIMER_PTR hwtimer)
{
    LPTMR_MemMapPtr	lpt_ptr = (LPTMR_MemMapPtr)hwtimer->ll_context[0];

    if (NULL == lpt_ptr)
    {
        return MQX_INVALID_PARAMETER;
    }

    /* Disable LPT */
    lpt_ptr->CSR &= (~ LPTMR_CSR_TEN_MASK);
    /* Disable LPT interrupt */
    lpt_ptr->CSR &= (~ LPTMR_CSR_TIE_MASK);
    /* Clear LPT interrupt flag */
    if (lpt_ptr->CSR & LPTMR_CSR_TCF_MASK)
    {
        lpt_ptr->CSR |= LPTMR_CSR_TCF_MASK;
    }

    return MQX_OK;
}

/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief Atomically captures current time into HWTIMER_TIME_STRUCT structure
 *
 * Corrects/normalizes the values if necessary (interrupt pending, etc.)
 *
 * \param hwtimer[in] Pointer to HWTIMER structure.
 * \param time[out]   Pointer to time structure. This value is filled with current value of the timer.
 *
 * \return MQX_OK Success.
 *
 * \warning This function calls _int_enable and _int_disable functions
 *
 * \see hwtimer_lpt_init
 * \see hwtimer_lpt_deinit
 * \see hwtimer_lpt_set_div
 * \see hwtimer_lpt_start
 * \see hwtimer_lpt_stop
 * \see hwtimer_lpt_isr
 */
static _mqx_int hwtimer_lpt_get_time(HWTIMER_PTR hwtimer, HWTIMER_TIME_PTR time)
{
    LPTMR_MemMapPtr lpt_ptr = (LPTMR_MemMapPtr) hwtimer->ll_context[0];

    if (NULL == lpt_ptr)
    {
        return MQX_INVALID_PARAMETER;
    }

    _int_disable();
    time->TICKS = hwtimer->ticks;

    /* Another full TICK period has expired since we handled the last timer interrupt.
    ** We need to read the counter again, since the wrap may have
    ** occurred between the previous read and the checking of the overflow bit. */
    if (lpt_ptr->CSR & LPTMR_CSR_TCF_MASK)
    {
        _int_enable();
        time->SUBTICKS = hwtimer->modulo - 1;
    }
    else
    {
        _int_enable();
        time->SUBTICKS = lpt_ptr->CNR;
    }

    return MQX_OK;
}

/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief Verify and set clock source for LPTMR
 *
 * \param hwtimer[in]   Pointer to HWTIMER structure.
 * \param pcs_mux[in]   MUX value used by Prescaler Clock Select.
 *
 * \return MQX_OK Success.
 *
 * \see hwtimer_lpt_init
 */
_mqx_int hwtimer_lpt_set_clksrc(HWTIMER_PTR hwtimer, int32_t pcs_mux)
{
    LPTMR_MemMapPtr lpt_ptr;

    /* Verify that HWTIMER pointer belongs to LPTMR */
    if ((hwtimer == NULL) || (hwtimer->devif != &lpt_devif))
    {
        return MQX_INVALID_PARAMETER;
    }

    /* Verify PCS mux */
    if ((pcs_mux >= 0) && (pcs_mux <= LPTMR_PSR_PCS_MASK))
    {
        lpt_ptr = (LPTMR_MemMapPtr)hwtimer->ll_context[0];
        lpt_ptr->PSR = LPTMR_PSR_PBYP_MASK | LPTMR_PSR_PCS(pcs_mux);
        return MQX_OK;
    }
    else
    {
        return MQX_INVALID_PARAMETER;
    }
}

/* EOF */
