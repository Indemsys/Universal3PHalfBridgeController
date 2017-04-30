#ifndef __hwtimer_h__
#define __hwtimer_h__
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
*   This include file is used to define constants and data types for the
*   hwtimer component
*
*
*END************************************************************************/

#include <mqx.h>

#define HWTIMER_LL_CONTEXT_LEN 5
/* Type definitions */

typedef void  (_CODE_PTR_  HWTIMER_CALLBACK_FPTR)(void *);
/*!
 * \brief hwtimer structure.
 *
 * This structure defines a hwtimer.
 * The context structure is passed to all API functions (besides other parameters).
 *
 * \warning Application should not access members of this structure directly.
 *
 * \see hwtimer_init
 * \see hwtimer_deinit
 * \see hwtimer_start
 * \see hwtimer_stop
 * \see hwtimer_set_freq
 * \see hwtimer_get_freq
 * \see hwtimer_set_period
 * \see hwtimer_get_period
 * \see hwtimer_get_modulo
 * \see hwtimer_get_time
 * \see hwtimer_get_ticks
 * \see hwtimer_callback_reg
 * \see hwtimer_callback_block
 * \see hwtimer_callback_unblock
 * \see hwtimer_callback_cancel
 */
typedef struct hwtimer
{
    /*! \brief Pointer to device interface structure */
    const struct hwtimer_devif_struct *   devif;
    /*! \brief Clock identifier used for obtaining timer's source clock. */
    uint32_t                         clock_id;
    /*! \brief Actual total divider */
    uint32_t                         divider;
    /*! \brief Determine how many subticks are in one tick */
    uint32_t                         modulo;
    /*! \brief Number of elapsed ticks */
    volatile uint64_t                ticks;
    /*! \brief Function pointer to be called when the timer expires. */
    HWTIMER_CALLBACK_FPTR           callback_func;
    /*! \brief Arbitrary pointer passed as parameter to the callback function. */
    void                           *callback_data;
    /*! \brief Indicate pending callback. 
     *  If the timer overflows when callbacks are blocked the callback becomes pending. */
    volatile int                    callback_pending;
    /*! \brief Indicate blocked callback */
    int                             callback_blocked;
    /*! \brief Private storage locations for arbitrary data keeping the context of the lower layer driver */
    uint32_t                         ll_context[HWTIMER_LL_CONTEXT_LEN];
} HWTIMER, * HWTIMER_PTR;

/*!
 * \brief hwtimer_time structure.
 *
 * hwtimer time structure represents a timestamp consisting of timer elapsed periods (TICKS) and current value of the timer counter (SUBTICKS).
 *
 * \see hwtimer_get_time
 */
typedef struct hwtimer_time_struct
{
    /*! \brief Ticks of timer */
    uint64_t TICKS;
    /*! \brief Subticks of timer */
    uint32_t SUBTICKS;
} HWTIMER_TIME_STRUCT, * HWTIMER_TIME_PTR;

typedef _mqx_int (_CODE_PTR_  HWTIMER_DEVIF_INIT_FPTR)(HWTIMER_PTR, uint32_t, uint32_t);
typedef _mqx_int (_CODE_PTR_  HWTIMER_DEVIF_DEINIT_FPTR)(HWTIMER_PTR);
typedef _mqx_int (_CODE_PTR_  HWTIMER_DEVIF_SET_DIV_FPTR)(HWTIMER_PTR, uint32_t);
typedef _mqx_int (_CODE_PTR_  HWTIMER_DEVIF_START_FPTR)(HWTIMER_PTR);
typedef _mqx_int (_CODE_PTR_  HWTIMER_DEVIF_STOP_FPTR)(HWTIMER_PTR);
typedef _mqx_int (_CODE_PTR_  HWTIMER_DEVIF_RESET_FPTR)(HWTIMER_PTR);
typedef _mqx_int (_CODE_PTR_  HWTIMER_DEVIF_GET_TIME_FPTR)(HWTIMER_PTR, HWTIMER_TIME_PTR);

/*!
 * \brief hwtimer_devif structure.
 *
 * Each low layer driver exports instance of this structure initialized with pointers to API functions the driver implements.
 * The functions themselves should be declared as static (not exported directly).
 *
 * \see hwtimer_init
 * \see hwtimer_deinit
 */
typedef struct hwtimer_devif_struct
{
     /*! \brief Function pointer to lower layer initialization */
    HWTIMER_DEVIF_INIT_FPTR             INIT;
     /*! \brief Function pointer to lower layer de-initialization */
    HWTIMER_DEVIF_DEINIT_FPTR           DEINIT;
     /*! \brief Function pointer to lower layer set divider functionality */
    HWTIMER_DEVIF_SET_DIV_FPTR          SET_DIV;
     /*! \brief Function pointer to lower layer start functionality */
    HWTIMER_DEVIF_START_FPTR            START;
     /*! \brief Function pointer to lower layer stop functionality */
    HWTIMER_DEVIF_STOP_FPTR             STOP;
     /*! \brief Function pointer to lower layer get time functionality */
    HWTIMER_DEVIF_GET_TIME_FPTR         GET_TIME;
} HWTIMER_DEVIF_STRUCT, * HWTIMER_DEVIF_STRUCT_PTR;


/* Function prototypes */
_mqx_int hwtimer_init(HWTIMER_PTR, const HWTIMER_DEVIF_STRUCT *, uint32_t, uint32_t);
_mqx_int hwtimer_deinit(HWTIMER_PTR);
_mqx_int hwtimer_set_freq(HWTIMER_PTR, uint32_t, uint32_t);
_mqx_int hwtimer_set_period(HWTIMER_PTR, uint32_t, uint32_t);
uint32_t hwtimer_get_freq(HWTIMER_PTR);
uint32_t hwtimer_get_period(HWTIMER_PTR);
_mqx_int hwtimer_start(HWTIMER_PTR);
_mqx_int hwtimer_stop(HWTIMER_PTR);
_mqx_int hwtimer_reset(HWTIMER_PTR);
uint32_t hwtimer_get_modulo(HWTIMER_PTR);
_mqx_int hwtimer_get_time(HWTIMER_PTR, HWTIMER_TIME_PTR);
uint32_t hwtimer_get_ticks(HWTIMER_PTR);
_mqx_int hwtimer_callback_reg(HWTIMER_PTR, HWTIMER_CALLBACK_FPTR, void *);
_mqx_int hwtimer_callback_block(HWTIMER_PTR);
_mqx_int hwtimer_callback_unblock(HWTIMER_PTR);
_mqx_int hwtimer_callback_cancel(HWTIMER_PTR);

#endif
