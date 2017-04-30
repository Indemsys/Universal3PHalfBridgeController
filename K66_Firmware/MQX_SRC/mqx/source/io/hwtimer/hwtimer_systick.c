
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
*   This file contains functions of the low level PIT module for hwtimer
*   component.
*
*
*END************************************************************************/

#include "hwtimer.h"
#include "hwtimer_systick.h"
#include <bsp.h>

extern uint32_t systick_get_vector(void);

static void hwtimer_systick_isr(void *);
static _mqx_int hwtimer_systick_init(HWTIMER_PTR , uint32_t, uint32_t);
static _mqx_int hwtimer_systick_deinit(HWTIMER_PTR);
static _mqx_int hwtimer_systick_set_div(HWTIMER_PTR, uint32_t);
static _mqx_int hwtimer_systick_start(HWTIMER_PTR);
static _mqx_int hwtimer_systick_stop(HWTIMER_PTR);
static _mqx_int hwtimer_systick_get_time(HWTIMER_PTR , HWTIMER_TIME_PTR);


const HWTIMER_DEVIF_STRUCT systick_devif =
{
    hwtimer_systick_init,
    hwtimer_systick_deinit,
    hwtimer_systick_set_div,
    hwtimer_systick_start,
    hwtimer_systick_stop,
    hwtimer_systick_get_time
};

/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief Interrupt service routine.
 *
 * This ISR is used when SysTick counted to 0.
 * Checks whether callback_func is not NULL,
 * and unless callback is blocked by callback_blocked being non-zero it calls the callback function with callback_data as parameter,
 * otherwise callback_pending is set to non-zero value.
 *
 * \param p[in]   Pointer to hwtimer struct.
 *
 * \return void
 *
 * \see hwtimer_systick_deinit
 * \see hwtimer_systick_set_div
 * \see hwtimer_systick_start
 * \see hwtimer_systick_stop
 * \see hwtimer_systick_get_time
 */
static void hwtimer_systick_isr(void *p)
{
    HWTIMER_PTR hwtimer     = (HWTIMER_PTR) p;
    SysTick_MemMapPtr systick_base  = (SysTick_MemMapPtr) hwtimer->ll_context[0];


    /* Check if interrupt is enabled for this systick. Cancel spurious interrupt */
    if (!(SysTick_CSR_TICKINT_MASK & SysTick_CSR_REG(systick_base)))
    {
        return;
    }

    /* Following part of function is typically the same for all low level hwtimer drivers */
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
 * \param hwtimer[in]    Returns initialized hwtimer structure handle.
 * \param systick_id[in] Determines Systick modul( Always 0).
 * \param isr_prior[in]  Interrupt priority for PIT
 *
 * \return MQX_OK                       Success.
 * \return MQX_INVALID_PARAMETER        When systick_id does not exist(When systick_id is not zero).
 * \return -1                           When systick is used byt PE or when _int_install_isr faild.
 *
 * \see hwtimer_systick_deinit
 * \see hwtimer_systick_set_div
 * \see hwtimer_systick_start
 * \see hwtimer_systick_stop
 * \see hwtimer_systick_get_time
 * \see hwtimer_systick_isr
 */
static _mqx_int hwtimer_systick_init(HWTIMER_PTR hwtimer, uint32_t systick_id, uint32_t isr_prior)
{
    PSP_INTERRUPT_TABLE_INDEX   vector;
    SysTick_MemMapPtr           systick_base;

    /* We count only with one systick module inside core */
    #if MQX_CHECK_ERRORS
    if ( 1 <= systick_id)
    {
        return MQX_INVALID_PARAMETER;
    }
    /* Check isr priority */
    if (0 == CORTEX_PRIOR(isr_prior))
    {
        return MQX_INVALID_PARAMETER;
    }
    #endif

    /* We need to store SysTick base in context struct */
    hwtimer->ll_context[0] = (uint32_t)SysTick_BASE_PTR;
    systick_base = (SysTick_MemMapPtr) hwtimer->ll_context[0];

    #if PE_LDD_VERSION
    if (PE_PeripheralUsed((uint32_t)systick_base))
    {
        return -1;
    }
    #endif
    /* Disable timer and interrupt. Set clock source to processor clock */
    /* But mostly The CLKSOURCE bit in SysTick Control and Status register is always set to select the core clock. */
    SysTick_CSR_REG(systick_base) = SysTick_CSR_CLKSOURCE_MASK;

    /* Reset reload value register. A start value of 0 is possible, but has no effect */
    SysTick_RVR_REG(systick_base) = SysTick_RVR_RELOAD(0x0);
    /* A write of any value to current value register clears the field to 0, and also clears the SYST_CSR COUNTFLAG bit to 0. */
    SysTick_CVR_REG(systick_base) = SysTick_CVR_CURRENT(0x0);

    /* Set isr for timer*/
    vector = (PSP_INTERRUPT_TABLE_INDEX) systick_get_vector();
    if (NULL == _int_install_isr(vector, (INT_ISR_FPTR) hwtimer_systick_isr, (void *) hwtimer))
    {
        return -1;
    }
    /* Set interrupt priority and enable interrupt */
    SCB_SHPR3 = (SCB_SHPR3 & ~(SCB_SHPR3_PRI_15_MASK)) | SCB_SHPR3_PRI_15(CORTEX_PRIOR(isr_prior));

    return MQX_OK;
}

/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief Initialization of systick timer module
 *
 * Called by hwtimer_deinit.
 * Disables the peripheral.
 * Unregisters ISR.
 *
 * \param hwtimer[in] Pointer to hwtimer structure.
 *
 * \return MQX_OK                       Success.
 * \return MQX_INVALID_COMPONENT_HANDLE When doesnt have any interrupt vectors, or systick does not exist.
 *
 * \see hwtimer_systick_init
 * \see hwtimer_systick_set_div
 * \see hwtimer_systick_start
 * \see hwtimer_systick_stop
 * \see hwtimer_systick_get_time
 * \see hwtimer_systick_isr
 */
static _mqx_int hwtimer_systick_deinit(HWTIMER_PTR hwtimer)
{
    PSP_INTERRUPT_TABLE_INDEX   vector;
    /* Every channel has own interrupt vector */
    vector = (PSP_INTERRUPT_TABLE_INDEX) systick_get_vector();
    /* Disable interrupt on vector */
    _bsp_int_disable(vector);
    /* Install default isr routine for our systick */
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
 * \param hwtimer[in] Pointer to hwtimer structure.
 * \param divider[in] Value which divide input clock of systick timer module to obtain requested period of timer.
 *
 * \return MQX_OK                Success.
 * \return MQX_INVALID_PARAMETER Divider is equal to zero.
 *
 * \see hwtimer_systick_init
 * \see hwtimer_systick_deinit
 * \see hwtimer_systick_start
 * \see hwtimer_systick_stop
 * \see hwtimer_systick_get_time
 * \see hwtimer_systick_isr
 */
static _mqx_int hwtimer_systick_set_div(HWTIMER_PTR hwtimer, uint32_t divider)
{
    SysTick_MemMapPtr           systick_base = (SysTick_MemMapPtr) hwtimer->ll_context[0];

    #if MQX_CHECK_ERRORS
    if ((0 == divider ) || ((divider - 1) & ~SysTick_RVR_RELOAD_MASK))
    {
        return MQX_INVALID_PARAMETER;
    }
    #endif

    SysTick_RVR_REG(systick_base)   = SysTick_RVR_RELOAD(divider - 1);

    hwtimer->divider    = divider;
    hwtimer->modulo     = divider;

    return MQX_OK;
}


/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief Start systick timer module
 *
 * This function enables the timer and leaves it running, timer is
 * periodically generating interrupts.
 *
 * \param hwtimer[in] Pointer to hwtimer structure.
 *
 * \return MQX_OK Success.
 *
 * \see hwtimer_systick_init
 * \see hwtimer_systick_deinit
 * \see hwtimer_systick_set_div
 * \see hwtimer_systick_stop
 * \see hwtimer_systick_get_time
 * \see hwtimer_systick_isr
 */
static _mqx_int hwtimer_systick_start(HWTIMER_PTR hwtimer)
{
    SysTick_MemMapPtr systick_base = (SysTick_MemMapPtr) hwtimer->ll_context[0];

    /* A write of any value to current value register clears the field to 0, and also clears the SYST_CSR COUNTFLAG bit to 0. */
    SysTick_CVR_REG(systick_base) = SysTick_CVR_CURRENT(0x0);

    /* Run timer and enable interrupt */
    SysTick_CSR_REG(systick_base) = SysTick_CSR_CLKSOURCE_MASK | SysTick_CSR_ENABLE_MASK | SysTick_CSR_TICKINT_MASK;

    return MQX_OK;
}

/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief Stop systick timer module
 *
 * Disable timer and interrupt
 *
 * \param hwtimer[in] Pointer to hwtimer structure.
 *
 * \return MQX_OK Success.
 *
 * \see hwtimer_systick_init
 * \see hwtimer_systick_deinit
 * \see hwtimer_systick_set_div
 * \see hwtimer_systick_start
 * \see hwtimer_systick_get_time
 * \see hwtimer_systick_isr
 */
static _mqx_int hwtimer_systick_stop(HWTIMER_PTR hwtimer)
{
    SysTick_MemMapPtr systick_base = (SysTick_MemMapPtr) hwtimer->ll_context[0];

    /* Disable timer and interrupt */
    SysTick_CSR_REG(systick_base) = 0x00;

    /* A write of any value to current value register clears the field to 0, and also clears the SYST_CSR COUNTFLAG bit to 0. */
    SysTick_CVR_REG(systick_base) = SysTick_CVR_CURRENT(0x0);

    return MQX_OK;
}

/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief Atomically captures current time into HWTIMER_TIME_STRUCT structure
 *
 * Corrects/normalizes the values if necessary (interrupt pending, etc.)
 *
 * \param hwtimer[in] Pointer to hwtimer structure.
 * \param time[out]   Pointer to time structure. This value is filled with current value of the timer.
 *
 * \return MQX_OK Success.
 *
 * \warning This function calls _int_enable and _int_disable functions
 *
 * \see hwtimer_systick_init
 * \see hwtimer_systick_deinit
 * \see hwtimer_systick_set_div
 * \see hwtimer_systick_start
 * \see hwtimer_systick_stop
 * \see hwtimer_systick_isr
 */
static _mqx_int hwtimer_systick_get_time(HWTIMER_PTR hwtimer, HWTIMER_TIME_PTR time)
{
    SysTick_MemMapPtr systick_base = (SysTick_MemMapPtr) hwtimer->ll_context[0];
    uint32_t temp_cvr;

    _int_disable();
    time->TICKS = hwtimer->ticks;
    temp_cvr = SysTick_CVR_REG(systick_base);
    /* Check systick pending interrupt flag */
    if (SCB_ICSR & SCB_ICSR_PENDSTSET_MASK)
    {
        _int_enable();
        time->SUBTICKS = hwtimer->modulo - 1;
    }
    else
    {
        _int_enable();
        /* interrupt flag is set upon 1->0 transition, not upon reload - wrap around */
        time->SUBTICKS = SysTick_RVR_REG(systick_base) - temp_cvr + 1;
    }

    return MQX_OK;
}
