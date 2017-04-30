
/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
* Copyright 2004-2008 Embedded Access Inc.
* Copyright 1989-2008 ARC International
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
*   light weight timer component.
*
*
*END************************************************************************/

#ifndef __lwtimer_h__
#define __lwtimer_h__ 1

#include <mqx_cnfg.h>

#if (! MQX_USE_LWTIMER) && (! defined (MQX_DISABLE_CONFIG_CHECK))
#error LWTIMER component is currently disabled in MQX kernel. Please set MQX_USE_LWTIMER to 1 in user_config.h and recompile kernel.
#endif

/*--------------------------------------------------------------------------*/
/*                     DATA STRUCTURE DEFINITIONS                           */

/*!
 * \brief Lightweight timer.
 *
 * This structure defines a light weight timer.
 * These timers implement a system where the specified function will be called
 * at a periodic interval.
 *
 * \see _lwtimer_add_timer_to_queue
 * \see _lwtimer_cancel_timer
 * \see LWTIMER_PERIOD_STRUCT
 */
typedef struct lwtimer_struct
{

   /*! \brief Queue data structures. */
   QUEUE_ELEMENT_STRUCT       LINK;

   /*! \brief The relative number of ticks until this timer is to expire. */
   _mqx_uint                  RELATIVE_TICKS;

   /*! \brief When the timer is added to the timer queue, MQX initializes the field.
    * When the timer or the timer queue that the timer is in is cancelled, MQX
    * clears the field.
    */
   _mqx_uint                  VALID;

   /*! \brief Function that is called when the timer expires. */
   LWTIMER_ISR_FPTR           TIMER_FUNCTION;

   /*! \brief Parameter that is passed to the timer function. */
   void                      *PARAMETER;

   /*! \brief Pointer to the lightweight timer queue to which the timer is attatched. */
   void                      *PERIOD_PTR;

} LWTIMER_STRUCT, * LWTIMER_STRUCT_PTR;


/*!
 * \brief Lightweight timer queue.
 *
 * This structure controls any number of lightweight timers wishing to be executed
 * at the periodic rate defined by this structure.
 * The periodic rate will be a multiple of the BSP_ALARM_RESOLUTION.
 *
 * \see _lwtimer_add_timer_to_queue
 * \see _lwtimer_cancel_period
 * \see _lwtimer_create_periodic_queue
 * \see LWTIMER_STRUCT
 */
typedef struct lwtimer_period_struct
{
   /*! \brief Queue of lightweight timers. */
   QUEUE_ELEMENT_STRUCT       LINK;

   /*! \brief The period of this group of timers (in ticks), a multiple of
    * BSP_ALARM_RESOLUTION. */
   _mqx_uint                  PERIOD;

   /*! \brief Number of ticks that have elapsed in this period. */
   _mqx_uint                  EXPIRY;

   /*!
    * \brief Number of ticks to wait before starting to process this queue.*/
   _mqx_uint                  WAIT;

   /*! \brief A queue of timers to expire at this periodic rate. */
   QUEUE_STRUCT               TIMERS;

   /*! \brief Pointer to the last timer on the queue that was processed */
   LWTIMER_STRUCT_PTR         TIMER_PTR;

   /*! \brief When the timer queue is created, MQX initializes the field. When
    * the queue is cancelled, MQX clears the field. */
   _mqx_uint                  VALID;

} LWTIMER_PERIOD_STRUCT, * LWTIMER_PERIOD_STRUCT_PTR;


/*--------------------------------------------------------------------------*/
/*                       EXTERNAL DECLARATIONS                              */

#ifdef __cplusplus
extern "C" {
#endif
#ifndef __TAD_COMPILE__

extern _mqx_uint _lwtimer_add_timer_to_queue(LWTIMER_PERIOD_STRUCT_PTR,
   LWTIMER_STRUCT_PTR, _mqx_uint, LWTIMER_ISR_FPTR, void *);
extern _mqx_uint _lwtimer_cancel_period(LWTIMER_PERIOD_STRUCT_PTR);
extern _mqx_uint _lwtimer_cancel_timer(LWTIMER_STRUCT_PTR);
extern _mqx_uint _lwtimer_create_periodic_queue(LWTIMER_PERIOD_STRUCT_PTR,
   _mqx_uint, _mqx_uint);
extern _mqx_uint _lwtimer_test(void **, void **);

#endif
#ifdef __cplusplus
}
#endif

#endif  /* __lwtimer_h__ */
/* EOF */
