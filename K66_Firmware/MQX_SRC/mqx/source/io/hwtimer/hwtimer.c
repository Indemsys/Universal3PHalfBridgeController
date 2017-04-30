
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
*   This file contains functions of the hwtimer component.
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>
#include "hwtimer.h"

/*!
 * \brief Initializes caller allocated structure according to given parameters.
 *
 * The device interface pointer determines low layer driver to be used.
 * Device interface structure is exported by each low layer driver and is opaque to the applications.
 * Please refer to chapter concerning low layer driver below for details.
 * Meaning of the numerical identifier varies depending on low layer driver used.
 * Typically, it identifies particular timer channel to initialize.
 * The initialization function has to be called prior using any other API function.
 *
 * \param hwtimer[out] Returns initialized hwtimer structure handle.
 * \param devif[in]    Structure determines low layer driver to be used.
 * \param id[in]       Numerical identifier of the timer.
 *
 * \return MQX_OK or an error code Returned from low level SET_DIV function.
 * \return MQX_INVALID_PARAMETER   When input parameter hwtimer or his device structure are NULL pointers. This value is also returned when the period parameter is 0.
 * \return MQX_INVALID_POINTER     When low level SET_DIV function point to NULL.
 *
 * \warning The initialization function has to be called prior using any other API function.
 *
 * \see hwtimer_deinit
 * \see hwtimer_start
 * \see hwtimer_stop
 */
_mqx_int hwtimer_init(HWTIMER_PTR hwtimer, const HWTIMER_DEVIF_STRUCT * devif, uint32_t id, uint32_t isr_prior)
 {
    #if MQX_CHECK_ERRORS
    if ((hwtimer == NULL) || (devif == NULL))
    {
        return MQX_INVALID_PARAMETER;
    }
    if (devif->INIT == NULL)
    {
        return MQX_INVALID_POINTER;
    }
    #endif
    hwtimer->devif      = devif;
    hwtimer->ticks      = 0;
    hwtimer->divider    = 0;
    hwtimer->clock_id   = 0;
    hwtimer->modulo     = 0;
    hwtimer->callback_func = NULL;
    hwtimer->callback_data = NULL;
    hwtimer->callback_pending = 0;
    hwtimer->callback_blocked = 0;
    return hwtimer->devif->INIT(hwtimer, id, isr_prior);
 }

/*!
 * \brief De-initialization of hwtimer.
 *
 * Calls lower layer stop function to stop timer, then calls low layer de-initialization function and afterwards invalidates hwtimer structure by clearing it.
 *
 * \param hwtimer[in] Pointer to hwtimer structure.
 *
 * \return MQX_OK or an error code Returned from low level DEINIT function.
 * \return MQX_INVALID_PARAMETER   When input parameter hwtimer or his device structure are NULL pointers.
 * \return MQX_INVALID_POINTER     When low level DEINIT function point to NULL.
 *
 * \see hwtimer_init
 * \see hwtimer_start
 * \see hwtimer_stop
 */
_mqx_int hwtimer_deinit(HWTIMER_PTR hwtimer)
 {
    _mqx_int result;
    #if MQX_CHECK_ERRORS
    if ((NULL == hwtimer) || (NULL == hwtimer->devif))
    {
        return MQX_INVALID_PARAMETER;
    }
    if (NULL == hwtimer->devif->DEINIT)
    {
        return MQX_INVALID_POINTER;
    }
    #endif
    
    result = hwtimer->devif->STOP(hwtimer);
    if (MQX_OK != result)
    {
        return result;
    }
    
    result = hwtimer->devif->DEINIT(hwtimer);
    if (MQX_OK != result)
    {
        return result;
    }
    
    hwtimer->devif      = NULL;
    hwtimer->ticks      = 0;
    hwtimer->clock_id   = 0;
    hwtimer->divider    = 0;
    hwtimer->modulo     = 0;
    hwtimer->callback_func = NULL;
    hwtimer->callback_data = NULL;
    hwtimer->callback_pending = 0;
    hwtimer->callback_blocked = 0;

    return MQX_OK;
 }

/*!
 * \brief The function configures timer to tick at frequency as close as possible to the requested one.
 *
 * Actual accuracy depends on the timer module.
 * The function gets the value of the base frequency of the timer via clock manager, calculates required divider ratio and calls low layer driver to setup the timer accordingly.
 * Call to this function might be expensive as it may require complex calculation to choose the best configuration of dividers. The actual complexity depends on timer module implementation.
 * If there is only single divider or counter preload value (typical case) then there is no significant overhead.
 *
 * \param hwtimer[in]   Pointer to hwtimer structure.
 * \param clock_id[in]  Clock identifier used for obtaining timer's source clock.
 * \param freq[in]      Required frequency of timer in Hz.
 *
 * \return MQX_OK or an error code Returned from low level SETDIV function.
 * \return MQX_INVALID_PARAMETER   When input parameter hwtimer or his device structure are NULL pointers.
 * \return MQX_INVALID_POINTER     When low level SETDIV function point to NULL.
 *
 * \see hwtimer_get_freq
 * \see hwtimer_set_period
 * \see hwtimer_get_period
 * \see hwtimer_get_modulo
 * \see hwtimer_get_time
 * \see hwtimer_get_ticks
 */
_mqx_int hwtimer_set_freq(HWTIMER_PTR hwtimer, uint32_t clock_id, uint32_t freq)
{
    uint32_t clock_freq;
    uint32_t divider;
    #if MQX_CHECK_ERRORS
    if ((NULL == hwtimer) || (NULL == hwtimer->devif) || (0 == freq))
    {
        return MQX_INVALID_PARAMETER;
    }
    if (NULL == hwtimer->devif->SET_DIV)
    {
        return MQX_INVALID_POINTER;
    }
    #endif
    /* Store clock_id in struct*/
    hwtimer->clock_id = clock_id;
    /* Find out input frequency*/
    clock_freq = _bsp_get_clock(_bsp_get_clock_configuration(), (CM_CLOCK_SOURCE)clock_id);
    divider = clock_freq / freq;
    /* If Reqired frequency is higher than input clock frequency, we set divider 1 (for setting highest possible frequency)*/
    if (0 == divider)
    {
        divider = 1;
    }
    return hwtimer->devif->SET_DIV(hwtimer, divider);
}

/*!
 * \brief Set period of hwtimer.
 *
 * The function provides an alternate way to setup the timer to desired period specified in microseconds rather than to frequency in Hz.
 * The function gets the value of the base frequency of the timer via clock manager, calculates required divider ratio and calls low layer driver to setup the timer accordingly.
 * Call to this function might be expensive as it may require complex calculation to choose the best configuration of dividers.
 * The actual complexity depends on timer module implementation.
 * If there is only single divider or counter preload value (typical case) then there is no significant overhead.
 *
 * \param hwtimer[in]   Pointer to hwtimer structure.
 * \param clock_id[in]  Clock identifier used for obtaining timer's source clock.
 * \param period[in]    Required period of timer in micro seconds.
 *
 * \return MQX_OK or an error code Returned from low level SETDIV function.
 * \return MQX_INVALID_PARAMETER   When input parameter hwtimer or his device structure are NULL pointers.
 * \return MQX_INVALID_POINTER     When low level SETDIV function point to NULL.
 *
 * \see hwtimer_set_freq
 * \see hwtimer_get_freq
 * \see hwtimer_get_period
 * \see hwtimer_get_modulo
 * \see hwtimer_get_time
 * \see hwtimer_get_ticks
 */
_mqx_int hwtimer_set_period(HWTIMER_PTR hwtimer, uint32_t clock_id, uint32_t period)
{
    uint32_t clock_freq;
    uint64_t divider;
    #if MQX_CHECK_ERRORS
    if ((NULL == hwtimer) || (NULL == hwtimer->devif) || (0 == period))
    {
        return MQX_INVALID_PARAMETER;
    }
    if (NULL == hwtimer->devif->SET_DIV)
    {
        return MQX_INVALID_POINTER;
    }
    #endif
    /* Store clock_id in struct*/
    hwtimer->clock_id = clock_id;
    /* Find out input frequency*/
    clock_freq = _bsp_get_clock(_bsp_get_clock_configuration(), (CM_CLOCK_SOURCE)clock_id);
    divider = (((uint64_t)clock_freq * period)) / 1000000 ;
    /* If Reqired frequency is higher than input clock frequency, we set divider 1 (for setting highest possible frequency)*/
    if (0 == divider)
    {
        divider = 1;
    }
    /* if divider is greater than 32b value we set divider to max 32b value*/
    else if (divider & 0xFFFFFFFF00000000)
    {
        divider = 0xFFFFFFFF;
    }
    return hwtimer->devif->SET_DIV(hwtimer, (uint32_t)divider);
}

/*!
 * \brief Get frequency of hwtimer.
 *
 * The function returns current frequency of the timer calculated from the base frequency and actual divider settings of the timer or zero in case of an error.
 *
 * \param hwtimer[in]   Pointer to hwtimer structure.
 *
 * \return frequency    Already set frequency of hwtimer
 * \return 0            When input parameter hwtimer is NULL pointer or his divider member is zero.
 *
 * \see hwtimer_set_freq
 * \see hwtimer_set_period
 * \see hwtimer_get_period
 * \see hwtimer_get_modulo
 * \see hwtimer_get_time
 * \see hwtimer_get_ticks
 */
uint32_t hwtimer_get_freq(HWTIMER_PTR hwtimer)
{
    uint32_t clock_freq;
    #if MQX_CHECK_ERRORS
    if (NULL == hwtimer)
    {
        return 0;   //MQX_INVALID_PARAMETER;
    }
    /* Uninitialized hwtimer contains value of 0 for divider*/
    if (0 == hwtimer->divider)
    {
        return 0;   //MQX_INVALID_POINTER
    }
    #endif
    clock_freq = _bsp_get_clock(_bsp_get_clock_configuration(), (CM_CLOCK_SOURCE)hwtimer->clock_id);
    return clock_freq / hwtimer->divider;
}

/*!
 * \brief Get period of hwtimer.
 *
 * The function returns current period of the timer in microseconds calculated from the base frequency and actual divider settings of the timer.
 *
 * \param hwtimer[in]   Pointer to hwtimer structure.
 *
 * \return period       Already set period of hwtimer.
 * \return 0            Input parameter hwtimer is NULL pointer.
 *
 * \see hwtimer_set_freq
 * \see hwtimer_get_freq
 * \see hwtimer_set_period
 * \see hwtimer_get_modulo
 * \see hwtimer_get_time
 * \see hwtimer_get_ticks
 */
uint32_t hwtimer_get_period(HWTIMER_PTR hwtimer)
{
    uint32_t clock_freq;
    uint32_t period;
    #if MQX_CHECK_ERRORS
    if (NULL == hwtimer)
    {
        return 0;   //MQX_INVALID_PARAMETER;
    }
    #endif
    clock_freq = _bsp_get_clock(_bsp_get_clock_configuration(), (CM_CLOCK_SOURCE)hwtimer->clock_id);
    /* The result is always less than UINT32_MAX */
    period = ((uint64_t)1000000 * hwtimer->divider) / clock_freq;
    return period;
}

/*!
 * \brief Enables the timer and leaves it running.
 *
 * The timer starts counting a new period generating interrupts every time the timer rolls over.
 *
 * \param hwtimer[in] Returns initialized hwtimer structure handle.
 *
 * \return MQX_OK or an error code Returned from low level START function.
 * \return MQX_INVALID_PARAMETER   When input parameter hwtimer or his device structure are NULL pointers.
 * \return MQX_INVALID_POINTER     When low level START function point to NULL.
 *
 * \see hwtimer_init
 * \see hwtimer_deinit
 * \see hwtimer_stop
 */
_mqx_int hwtimer_start(HWTIMER_PTR hwtimer)
{
    #if MQX_CHECK_ERRORS
    if ((NULL == hwtimer) || (NULL == hwtimer->devif))
    {
        return MQX_INVALID_PARAMETER;
    }
    if (NULL == hwtimer->devif->START)
    {
        return MQX_INVALID_POINTER;
    }
    #endif
    return hwtimer->devif->START(hwtimer);
}

/*!
 * \brief The timer stops counting after this function is called.
 *
 * Pending interrupts and callbacks are canceled.
 *
 * \param hwtimer[in] Returns initialized hwtimer structure handle.
 *
 * \return MQX_OK or an error code Returned from low level STOP function.
 * \return MQX_INVALID_PARAMETER   When input parameter hwtimer or his device structure are NULL pointers.
 * \return MQX_INVALID_POINTER     When low level STOP function point to NULL.
 *
 * \see hwtimer_init
 * \see hwtimer_deinit
 * \see hwtimer_start
 */
_mqx_int hwtimer_stop(HWTIMER_PTR hwtimer)
{
    _mqx_int result;
    #if MQX_CHECK_ERRORS
    if ((NULL == hwtimer) || (NULL == hwtimer->devif))
    {
        return MQX_INVALID_PARAMETER;
    }
    if (NULL == hwtimer->devif->STOP)
    {
        return MQX_INVALID_POINTER;
    }
    #endif
    result = hwtimer->devif->STOP(hwtimer);
    hwtimer->callback_pending = 0;
    return result;
}

/*!
 * \brief The function returns period of the timer in sub-ticks.
 *
 * It is typically called after hwtimer_set_freq() or hwtimer_set_period() to obtain actual resolution of the timer in the current configuration.
 *
 * \param hwtimer[in]   Pointer to hwtimer structure.
 *
 * \return modulo       resolution of hwtimer.
 * \return 0            Input parameter hwtimer is NULL pointer.
 *
 * \see hwtimer_set_freq
 * \see hwtimer_get_freq
 * \see hwtimer_set_period
 * \see hwtimer_get_period
 * \see hwtimer_get_time
 * \see hwtimer_get_ticks
 */
uint32_t hwtimer_get_modulo(HWTIMER_PTR hwtimer)
{
    #if MQX_CHECK_ERRORS
    if (NULL == hwtimer)
    {
        return 0;   //MQX_INVALID_PARAMETER;
    }
    #endif
    return hwtimer->modulo;
}

/*!
 * \brief The function reads the current value of the hwtimer
 *
 * Elapsed periods(ticks) and current value of the timer counter (sub-ticks) are filled into the HWTIMER_TIME structure.
 * The sub-ticks number always counts up and is reset to zero when the timer overflows regardless of the counting direction of the underlying device.
 * The returned value corresponds with lower 32 bits of elapsed periods (ticks).
 *
 * \param hwtimer[in]   Pointer to hwtimer structure.
 * \param time[out]     Pointer to time structure. This value is filled with current value of the timer.
 *
 * \return _OK or an error code     Returned from low level GET_TIME function.
 * \return MQX_INVALID_PARAMETER    When input parameter hwtimer, his device structure or input parameter time are NULL pointers.
 * \return MQX_INVALID_POINTER      When low level GET_TIME function point to NULL.
 *
 * \see hwtimer_set_freq
 * \see hwtimer_get_freq
 * \see hwtimer_set_period
 * \see hwtimer_get_period
 * \see hwtimer_get_modulo
 * \see hwtimer_get_ticks
 */
_mqx_int hwtimer_get_time(HWTIMER_PTR hwtimer, HWTIMER_TIME_PTR time)
{
    #if MQX_CHECK_ERRORS
    if ((NULL == hwtimer) || (NULL == hwtimer->devif) || (NULL == time))
    {
        return MQX_INVALID_PARAMETER;
    }
    if (NULL == hwtimer->devif->GET_TIME)
    {
        return MQX_INVALID_POINTER;
    }
    #endif
    return hwtimer->devif->GET_TIME(hwtimer, time);
}

/*!
 * \brief The function reads the current value of the hwtimer
 *
 * The function returns lower 32 bits of elapsed periods (ticks).
 * The value is guaranteed to be obtained atomically without necessity to mask timer interrupt.
 * Lower layer driver is not involved at all, thus call to this function is considerably faster than hwtimer_get_time.
 *
 * \param hwtimer[in]   Pointer to hwtimer structure.
 *
 * \return Low 32 bits of 64 bit tick value.
 * \return 0  When input parameter is NULL pointer.
 *
 * \see hwtimer_set_freq
 * \see hwtimer_get_freq
 * \see hwtimer_set_period
 * \see hwtimer_get_period
 * \see hwtimer_get_modulo
 * \see hwtimer_get_time
 */
uint32_t hwtimer_get_ticks(HWTIMER_PTR hwtimer)
{
    #if MQX_CHECK_ERRORS
    if (NULL == hwtimer)
    {
        return 0;//MQX_INVALID_PARAMETER;
    }
    #endif
    /* return low 32b of 64 bit value */
    return (uint32_t)hwtimer->ticks;
}

/*!
 * \brief Registers function to be called when the timer expires.
 *
 * The callback_data is arbitrary pointer passed as parameter to the callback function.
 *
 *
 * \param hwtimer[in]        Pointer to hwtimer structure.
 * \param callback_func [in] Function pointer to be called when the timer expires.
 * \param callback_data [in] Data pointer for the function callback_func.
 *
 * \return MQX_INVALID_PARAMETER When input parameter hwtimer is NULL pointer.
 * \return MQX_OK                When registration callback succeed.
 *
 * \warning This function must not be called from a callback routine.
 *
 * \see hwtimer_callback_block
 * \see hwtimer_callback_unblock
 * \see hwtimer_callback_cancel
 */
_mqx_int hwtimer_callback_reg(HWTIMER_PTR hwtimer, HWTIMER_CALLBACK_FPTR callback_func, void *callback_data)
{
    HWTIMER volatile *hwtimer_vol;
    
    #if MQX_CHECK_ERRORS
    if (NULL == hwtimer)
    {
        return MQX_INVALID_PARAMETER;
    }
    #endif
    
    hwtimer_vol = hwtimer;
    hwtimer_vol->callback_func = NULL;
    hwtimer_vol->callback_pending = 0;
    hwtimer_vol->callback_data = callback_data;
    hwtimer_vol->callback_func = callback_func;
    
    return MQX_OK;
}

/*!
 * \brief The function is used to block callbacks in circumstances when execution of the callback function is undesired.
 *
 * If the timer overflows when callbacks are blocked the callback becomes pending.
 *
 * \param hwtimer[in] Pointer to hwtimer structure.
 *
 * \return MQX_INVALID_PARAMETER When input parameter hwtimer is NULL pointer.
 * \return MQX_OK                When callback block succeed.
 *
 * \warning This function must not be called from a callback routine.
 *
 * \see hwtimer_callback_reg
 * \see hwtimer_callback_unblock
 * \see hwtimer_callback_cancel
 */
_mqx_int hwtimer_callback_block(HWTIMER_PTR hwtimer)
{
    #if MQX_CHECK_ERRORS
    if (NULL == hwtimer)
    {
        return MQX_INVALID_PARAMETER;
    }
    #endif
    hwtimer->callback_blocked = 1;
    return MQX_OK;
}

/*!
 * \brief The function is used to unblock previously blocked callbacks.
 *
 * If there is a callback pending, it gets immediately executed.
 * This function must not be called from a callback routine (it does not make sense to do so anyway as callback function never gets executed while callbacks are blocked).
 *
 * \param hwtimer[in] Pointer to hwtimer structure.
 *
 * \return MQX_INVALID_PARAMETER When input parameter hwtimer is NULL pointer.
 * \return MQX_OK                When callback unblock succeed.
 *
 * \warning This function must not be called from a callback routine.
 *
 * \see hwtimer_callback_reg
 * \see hwtimer_callback_block
 * \see hwtimer_callback_cancel
 */
_mqx_int hwtimer_callback_unblock(HWTIMER_PTR hwtimer)
{
    HWTIMER_CALLBACK_FPTR callback_func;
    #if MQX_CHECK_ERRORS
    if (NULL == hwtimer)
    {
        return MQX_INVALID_PARAMETER;
    }
    #endif
    /* Unblock callbacks in ISR. No more pending request could arrive after this. */
    hwtimer->callback_blocked = 0;
    /* Check for any previously set pending requests during blocked state */
    if (hwtimer->callback_pending)
    {
        callback_func = hwtimer->callback_func;
        if (NULL != callback_func)
        {
            /* Prevent invocation of callback from ISR (callback may not be reentrant) */
            hwtimer->callback_func = NULL;
            callback_func(hwtimer->callback_data);
            /* Allow invocation of callback from ISR */
            hwtimer->callback_func = callback_func;
        }
        /* Clear pending flag, callback just serviced */
        hwtimer->callback_pending = 0;
    }
    return MQX_OK;
}

/*!
 * \brief The function cancels pending callback, if any.
 *
 * \param hwtimer[in] Pointer to hwtimer structure.
 *
 * \return MQX_INVALID_PARAMETER When input parameter hwtimer is NULL pointer.
 * \return MQX_OK                When callback cancel succeed.
 *
 * \warning This function must not be called from a callback routine (it does not make sense to do so anyway as callback function never gets executed while callbacks are blocked).
 *
 * \see hwtimer_callback_reg
 * \see hwtimer_callback_block
 * \see hwtimer_callback_unblock
 */
_mqx_int hwtimer_callback_cancel(HWTIMER_PTR hwtimer)
{
    #if MQX_CHECK_ERRORS
    if (NULL == hwtimer)
    {
        return MQX_INVALID_PARAMETER;
    }
    #endif
    hwtimer->callback_pending = 0;
    return MQX_OK;
}
