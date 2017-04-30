
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
#include "hwtimer_pit.h"
#include <bsp.h>

/*!
 * \cond DOXYGEN_PRIVATE
 * Macro allows the timers to be stopped when the device enters the Debug mode.
 */
#ifndef BSPCFG_HWTIMER_PIT_FREEZE
/* Timers continue to run in Debug mode */
#define  BSPCFG_HWTIMER_PIT_FREEZE 0
#endif

/*!
 * \cond DOXYGEN_PRIVATE
 * Macro return number of pitmodule from pit_id
 */
#define GET_PIT_NUMBER_FROM_PITID(pit_id)   ((pit_id >> 16) & 0x0000FFFF)
/*!
 * \cond DOXYGEN_PRIVATE
 * Macro return pit channel from pit_id
 */
#define GET_PIT_CHANNEL_FROM_PITID(pit_id)  (pit_id & 0x0000FFFF)

extern void pit_io_init(uint32_t);
extern uint32_t pit_get_vectors(uint32_t, const _mqx_uint **);
extern uint32_t pit_get_hwtimers_array(HWTIMER_PTR **);


static void hwtimer_pit_isr_shared(void *);
static void hwtimer_pit_isr(void *);
static _mqx_int hwtimer_pit_init(HWTIMER_PTR , uint32_t, uint32_t);
static _mqx_int hwtimer_pit_deinit(HWTIMER_PTR);
static _mqx_int hwtimer_pit_set_div(HWTIMER_PTR, uint32_t);
static _mqx_int hwtimer_pit_start(HWTIMER_PTR);
static _mqx_int hwtimer_pit_stop(HWTIMER_PTR);
static _mqx_int hwtimer_pit_get_time(HWTIMER_PTR , HWTIMER_TIME_PTR);


const HWTIMER_DEVIF_STRUCT pit_devif =
{
    hwtimer_pit_init,
    hwtimer_pit_deinit,
    hwtimer_pit_set_div,
    hwtimer_pit_start,
    hwtimer_pit_stop,
    hwtimer_pit_get_time
};

/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief Shared interrupt service routine.
 *
 * This ISR is used when more channels share one vector in vector table.
 * Checks whether callback_func is not NULL,
 * and unless callback is blocked by callback_blocked being non-zero it calls the callback function with callback_data as parameter,
 * otherwise callback_pending is set to non-zero value.
 * \param p[in]   Null pointer.
 *
 * \return void
 *
 * \see hwtimer_pit_deinit
 * \see hwtimer_pit_set_div
 * \see hwtimer_pit_start
 * \see hwtimer_pit_stop
 * \see hwtimer_pit_get_time
 * \see hwtimer_pit_isr
 */
static void hwtimer_pit_isr_shared(void *p)
{
    HWTIMER_PTR * pit_hwtimers_array = NULL;
    uint32_t pit_hwtimers_array_size;
    uint32_t i;
    HWTIMER_PTR hwtimer;
    PIT_MemMapPtr pit_base;
    uint32_t pit_channel;

    pit_hwtimers_array_size = pit_get_hwtimers_array(&pit_hwtimers_array);

    for (i = 0; i < pit_hwtimers_array_size; i++)
    {
        hwtimer =  pit_hwtimers_array[i];
        /* If hwtimer exist*/
        if (NULL != hwtimer)
        {
            pit_base    = (PIT_MemMapPtr) hwtimer->ll_context[0];
            pit_channel =  GET_PIT_CHANNEL_FROM_PITID(hwtimer->ll_context[1]);

            /* Check if interrupt is enabled for this channel. */
            if (!(PIT_TCTRL_TIE_MASK & PIT_TCTRL_REG(pit_base, pit_channel)))
            {
                continue;
            }

            /* If interrupt occur for this pit and channel*/
            if(PIT_TFLG_REG((PIT_MemMapPtr)pit_base, pit_channel) & PIT_TFLG_TIF_MASK)
            {
                /* Clear interrupt flag */
                PIT_TFLG_REG((PIT_MemMapPtr)pit_base, pit_channel) = PIT_TFLG_TIF_MASK;
                /* Errata for OM33Z: e2682: PIT: Does not generate a subsequent interrupt after clearing the interrupt flag */
                /* Workaround: The user must access any PIT register after clearing the interrupt flag in the ISR. */
                PIT_TFLG_REG(pit_base, pit_channel);

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
        }
    }
}
/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief Interrupt service routine.
 *
 * This ISR is used when every channel has its own vector in vector table.
 * Checks whether callback_func is not NULL,
 * and unless callback is blocked by callback_blocked being non-zero it calls the callback function with callback_data as parameter,
 * otherwise callback_pending is set to non-zero value.
 *
 * \param p[in]   Pointer to hwtimer struct.
 *
 * \return void
 *
 * \see hwtimer_pit_deinit
 * \see hwtimer_pit_set_div
 * \see hwtimer_pit_start
 * \see hwtimer_pit_stop
 * \see hwtimer_pit_get_time
 * \see hwtimer_pit_isr_shared
 */
static void hwtimer_pit_isr(void *p)
{
    HWTIMER_PTR hwtimer     = (HWTIMER_PTR) p;
    PIT_MemMapPtr pit_base  = (PIT_MemMapPtr) hwtimer->ll_context[0];
    uint32_t pit_channel     = GET_PIT_CHANNEL_FROM_PITID(hwtimer->ll_context[1]);

    /* Check if interrupt is enabled for this channel. Cancel spurious interrupt */
    if (!(PIT_TCTRL_TIE_MASK & PIT_TCTRL_REG(pit_base, pit_channel)))
    {
        return;
    }

    /* Clear interrupt flag */
    PIT_TFLG_REG(pit_base, pit_channel) = PIT_TFLG_TIF_MASK;
    /* Errata for OM33Z: e2682: PIT: Does not generate a subsequent interrupt after clearing the interrupt flag */
    /* Workaround: The user must access any PIT register after clearing the interrupt flag in the ISR. */
    PIT_TFLG_REG(pit_base, pit_channel);

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
 * \param hwtimer[in]   Returns initialized hwtimer structure handle.
 * \param pit_id[in]    Determines PIT modul and pit channel.
 * \param isr_prior[in] Interrupt priority for PIT
 *
 * \return MQX_OK                       Success.
 * \return MQX_INVALID_PARAMETER        When channel number does not exist in pit module.
 * \return MQX_INVALID_COMPONENT_HANDLE Doesnt have any interrupt vectors, or pit does not exist.
 * \return -1                           When pit is used byt PE or when _int_install_isr faild.
 *
 * \see hwtimer_pit_deinit
 * \see hwtimer_pit_set_div
 * \see hwtimer_pit_start
 * \see hwtimer_pit_stop
 * \see hwtimer_pit_get_time
 * \see hwtimer_pit_isr
 * \see hwtimer_pit_isr_shared
 */
static _mqx_int hwtimer_pit_init(HWTIMER_PTR hwtimer, uint32_t pit_id, uint32_t isr_prior)
{
    HWTIMER_PTR * pit_hwtimers_array = NULL;
    uint32_t pit_hwtimers_array_size;
    PSP_INTERRUPT_TABLE_INDEX    vector;
    PIT_MemMapPtr pit_base;
    uint32_t pit_number;
    uint32_t pit_channel;
    uint32_t pit_channels_count;
    uint32_t pit_vectors_count;
    _mqx_uint i;
    const _mqx_uint *pit_vectors = NULL;
    PIT_MemMapPtr pit;
    /* Count of chanels is computed, because this information missing in generated iomap file */
    pit_channels_count = sizeof(pit->CHANNEL) / sizeof(pit->CHANNEL[0]);

    #if MQX_CHECK_ERRORS
    if (pit_channels_count <= pit_id)
    {
        return MQX_INVALID_PARAMETER;
    }
    #endif

    /* We need to store pit_id of timer in context struct */
    hwtimer->ll_context[0] = (uint32_t)PIT_BASE_PTR;
    hwtimer->ll_context[1] = pit_id;

    pit_base    = (PIT_MemMapPtr) hwtimer->ll_context[0];
    pit_channel = GET_PIT_CHANNEL_FROM_PITID(hwtimer->ll_context[1]);
    pit_number  = GET_PIT_NUMBER_FROM_PITID(hwtimer->ll_context[1]);
    #if PE_LDD_VERSION
    if (PE_PeripheralUsed((uint32_t)pit_base))
    {
        return -1;
    }
    #endif
    /* Enable PIT Module Clock */
    pit_io_init(0);

    /* pit module enabling after pit clk gate enabled several clock cycles, workaround for some silicon */
    i = PIT_MCR_REG(pit_base);

    /* Enable PIT module */
    PIT_MCR_REG(pit_base) =  0;

    #if BSPCFG_HWTIMER_PIT_FREEZE
    /* Allows the timers to be stopped when the device enters the Debug mode. */
    PIT_MCR_REG(pit_base) |= PIT_MCR_FRZ_MASK;
    #endif

    /* Disable timer and interrupt */
    PIT_TCTRL_REG(pit_base, pit_channel) = 0;
    /* Clear any pending interrupt */
    PIT_TFLG_REG(pit_base, pit_channel) = PIT_TFLG_TIF_MASK;

    /* Set isr for timer*/
    pit_vectors_count =  pit_get_vectors(pit_number, &pit_vectors);
    #if MQX_CHECK_ERRORS
    if ((NULL == pit_vectors) || (0 == pit_vectors_count))
    {
        return MQX_INVALID_COMPONENT_HANDLE;  //doesnt have any interrupt vectors, or pit does not exist
    }
    #endif
    if (pit_channels_count <= pit_vectors_count)
    {
        /* Every channel has own interrupt vector */
        vector = (PSP_INTERRUPT_TABLE_INDEX) (pit_vectors[pit_channel]);
        if (NULL == _int_install_isr(vector, (INT_ISR_FPTR) hwtimer_pit_isr, (void *) hwtimer))
        {
            return -1;
        }
        _bsp_int_init(vector, isr_prior, 0, TRUE);
    }
    else
    {
        pit_hwtimers_array_size = pit_get_hwtimers_array(&pit_hwtimers_array);
        #if MQX_CHECK_ERRORS
        if ((NULL == pit_hwtimers_array) || (0 == pit_hwtimers_array_size))
        {
            return MQX_INVALID_COMPONENT_HANDLE;
        }
        #endif
        /* Pit has shared interrupt vectors */
        pit_hwtimers_array[pit_channel] = hwtimer;
        for (i = 0; i < pit_vectors_count; i++)
        {
            vector = (PSP_INTERRUPT_TABLE_INDEX) (pit_vectors[i]);
            if (NULL == _int_install_isr(vector, (INT_ISR_FPTR) hwtimer_pit_isr_shared, NULL))
            {
                return -1;
            }
            _bsp_int_init(vector, isr_prior, 0, TRUE);
        }
    }
    return MQX_OK;
}

/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief Initialization of pit timer module
 *
 * Called by hwtimer_deinit.
 * Disables the peripheral.
 * Unregisters ISR.

 *
 * \param hwtimer[in] Pointer to hwtimer structure.
 *
 * \return MQX_OK                       Success.
 * \return MQX_INVALID_COMPONENT_HANDLE When doesnt have any interrupt vectors, or pit does not exist.
 *
 * \see hwtimer_pit_init
 * \see hwtimer_pit_set_div
 * \see hwtimer_pit_start
 * \see hwtimer_pit_stop
 * \see hwtimer_pit_get_time
 * \see hwtimer_pit_isr
 * \see hwtimer_pit_isr_shared
 */
static _mqx_int hwtimer_pit_deinit(HWTIMER_PTR hwtimer)
{
    HWTIMER_PTR * pit_hwtimers_array = NULL;
    uint32_t pit_hwtimers_array_size;

    PSP_INTERRUPT_TABLE_INDEX    vector;
    uint32_t pit_channel = GET_PIT_CHANNEL_FROM_PITID(hwtimer->ll_context[1]);
    uint32_t pit_number  = GET_PIT_NUMBER_FROM_PITID(hwtimer->ll_context[1]);

    uint32_t pit_channels_count;
    uint32_t pit_vectors_count;
    _mqx_uint i;
    const _mqx_uint *pit_vectors = NULL;

    PIT_MemMapPtr pit;
    /* Count of chanels is computed, because this information missing in generated iomap file */
    pit_channels_count = sizeof(pit->CHANNEL) / sizeof(pit->CHANNEL[0]);

    pit_vectors_count =  pit_get_vectors(pit_number, &pit_vectors);
    #if MQX_CHECK_ERRORS
    if ((NULL == pit_vectors) || (0 == pit_vectors_count))
    {
        return MQX_INVALID_COMPONENT_HANDLE;  //doesnt have any interrupt vectors, or pit does not exist
    }
    #endif
    
    /* If number of pit channels is the same as number of vector */
    if (pit_channels_count <= pit_vectors_count)
    {
        /* Every channel has own interrupt vector */
        vector = (PSP_INTERRUPT_TABLE_INDEX) (pit_vectors[pit_channel]);
        /* Disable interrupt on vector */
        _bsp_int_disable(vector);
        /* Install default isr routine for our pit */
        _int_install_isr(vector, _int_get_default_isr(), NULL);
    }
    else
    {
        pit_hwtimers_array_size = pit_get_hwtimers_array(&pit_hwtimers_array);
        #if MQX_CHECK_ERRORS
        if ((NULL == pit_hwtimers_array) || (0 == pit_hwtimers_array_size))
        {
            return MQX_INVALID_COMPONENT_HANDLE;
        }
        #endif
        /* Pit has shared interrupt vectors. We need undregister interrupt only when all hwtimers are deinited(set to NULL) */
        pit_hwtimers_array[pit_channel] = NULL;
        /* Check if this is last hwtimer in pit_hwtimers_array */
        for (i = 0; i < pit_hwtimers_array_size; i++)
        {
            if (NULL != pit_hwtimers_array[i])
            {
                break;
            }
        }

        if (i == pit_hwtimers_array_size)
        {
            for (i = 0; i < pit_vectors_count; i++)
            {
                vector = (PSP_INTERRUPT_TABLE_INDEX) (pit_vectors[i]);
                /* Disable interrupt on vector */
                _bsp_int_disable(vector);
                /* Install default isr routine for our pit */
                _int_install_isr(vector, _int_get_default_isr(), NULL);
            }
        }
    }

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
 * \param divider[in] Value which divide input clock of pit timer module to obtain requested period of timer.
 *
 * \return MQX_OK                Success.
 * \return MQX_INVALID_PARAMETER Divider is equal to zero.
 *
 * \see hwtimer_pit_init
 * \see hwtimer_pit_deinit
 * \see hwtimer_pit_start
 * \see hwtimer_pit_stop
 * \see hwtimer_pit_get_time
 * \see hwtimer_pit_isr
 * \see hwtimer_pit_isr_shared
 */
static _mqx_int hwtimer_pit_set_div(HWTIMER_PTR hwtimer, uint32_t divider)
{
    PIT_MemMapPtr pit_base  = (PIT_MemMapPtr) hwtimer->ll_context[0];
    uint32_t pit_channel     = GET_PIT_CHANNEL_FROM_PITID(hwtimer->ll_context[1]);
    #if MQX_CHECK_ERRORS
    if (0 == divider)
    {
        return MQX_INVALID_PARAMETER;
    }
    #endif

    PIT_LDVAL_REG(pit_base, pit_channel)   = divider - 1;

    hwtimer->divider    = divider;
    hwtimer->modulo     = divider;

    return MQX_OK;
}


/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief Start pit timer module
 *
 * This function enables the timer and leaves it running, timer is
 * periodically generating interrupts.
 *
 * \param hwtimer[in] Pointer to hwtimer structure.
 *
 * \return MQX_OK Success.
 *
 * \see hwtimer_pit_init
 * \see hwtimer_pit_deinit
 * \see hwtimer_pit_set_div
 * \see hwtimer_pit_stop
 * \see hwtimer_pit_get_time
 * \see hwtimer_pit_isr
 * \see hwtimer_pit_isr_shared
 */
static _mqx_int hwtimer_pit_start(HWTIMER_PTR hwtimer)
{
    PIT_MemMapPtr pit_base  = (PIT_MemMapPtr) hwtimer->ll_context[0];
    uint32_t pit_channel     = GET_PIT_CHANNEL_FROM_PITID(hwtimer->ll_context[1]);

    PIT_TCTRL_REG(pit_base, pit_channel) = 0;  // Stop timer to force reseting the counter
    PIT_TFLG_REG(pit_base, pit_channel)  = PIT_TFLG_TIF_MASK; // Clear flag
    PIT_TCTRL_REG(pit_base, pit_channel) = PIT_TCTRL_TEN_MASK | PIT_TCTRL_TIE_MASK; // Run timer and enable interrupts

    return MQX_OK;
}

/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief Stop pit timer module
 *
 * Disable timer and interrupt
 *
 * \param hwtimer[in] Pointer to hwtimer structure.
 *
 * \return MQX_OK Success.
 *
 * \see hwtimer_pit_init
 * \see hwtimer_pit_deinit
 * \see hwtimer_pit_set_div
 * \see hwtimer_pit_start
 * \see hwtimer_pit_get_time
 * \see hwtimer_pit_isr
 * \see hwtimer_pit_isr_shared
 */
static _mqx_int hwtimer_pit_stop(HWTIMER_PTR hwtimer)
{
    PIT_MemMapPtr pit_base  = (PIT_MemMapPtr) hwtimer->ll_context[0];
    uint32_t pit_channel     = GET_PIT_CHANNEL_FROM_PITID(hwtimer->ll_context[1]);

    /* Disable timer and interrupt */
    PIT_TCTRL_REG(pit_base, pit_channel) = 0;
    PIT_TFLG_REG(pit_base, pit_channel) = PIT_TFLG_TIF_MASK; // Clear interrupt flag
    /* Errata for OM33Z: e2682: PIT: Does not generate a subsequent interrupt after clearing the interrupt flag */
    /* Workaround: The user must access any PIT register after clearing the interrupt flag in the ISR. */
    PIT_TFLG_REG(pit_base, pit_channel);

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
 * \see hwtimer_pit_init
 * \see hwtimer_pit_deinit
 * \see hwtimer_pit_set_div
 * \see hwtimer_pit_start
 * \see hwtimer_pit_stop
 * \see hwtimer_pit_isr
 * \see hwtimer_pit_isr_shared
 */
static _mqx_int hwtimer_pit_get_time(HWTIMER_PTR hwtimer, HWTIMER_TIME_PTR time)
{
    PIT_MemMapPtr   pit_base    = (PIT_MemMapPtr) hwtimer->ll_context[0];
    uint32_t        pit_channel = GET_PIT_CHANNEL_FROM_PITID(hwtimer->ll_context[1]);
    uint32_t        temp_cval;

    /* Disable interrupt from timer*/
    _int_disable();
    time->TICKS = hwtimer->ticks;

    temp_cval  = PIT_CVAL_REG(pit_base, pit_channel);
    /* Check pending interrupt flag */
    if(PIT_TFLG_REG(pit_base, pit_channel) & PIT_TFLG_TIF_MASK)
    {
        _int_enable();
        time->SUBTICKS = hwtimer->modulo - 1;
    }
    else
    {
        _int_enable();
        time->SUBTICKS = PIT_LDVAL_REG(pit_base, pit_channel) - temp_cval;
    }

    return MQX_OK;
}
