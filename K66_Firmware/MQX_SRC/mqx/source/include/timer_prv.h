
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
*   timer component's internal use.
*
*
*END************************************************************************/
#ifndef __timer_prv_h__
#define __timer_prv_h__ 1

/*--------------------------------------------------------------------------*/
/*                        CONSTANT DEFINITIONS                              */

/* The timer validity check value */
#define TIMER_VALID               (_mqx_uint)(0x74696d72)    /* "timr" */

/* The types of timers */
#define TIMER_TYPE_ONESHOT_AFTER  (0x1)
#define TIMER_TYPE_ONESHOT_AT     (0x2)
#define TIMER_TYPE_PERIODIC_EVERY (0x11)
#define TIMER_TYPE_PERIODIC_AT    (0x12)

#define IS_ONESHOT(x) (x <= TIMER_TYPE_ONESHOT_AT)

/*--------------------------------------------------------------------------*/
/*
 *                      QUEUE MANIPULATION MACROS
 */

/*
 *  CR171 & CR172
 *
 *  For a time delay queues we insert the task into the queue after
 *  all tasks of the same delay period.  We do not need to do this, we
 *  can just insert the task into the queue before the task waiting
 *  for the same time, as all tasks will be activated anyways.
 *  Running the queues takes a long time, and is done with interrupts
 *  disabled.  There will be a slight behavioural difference in that
 *  two tasks that delay for the same time at the same priority level,
 *  will not run in the order that they called delay.
 *
 *  Set the following define to 1 if you want the original FIFO
 *  behavior where tasks are activated from the time delay queue into
 *  the ready queue in FIFO order (longer enqueue search with
 *  interrupts disabled).
 */
#ifndef MQX_DELAY_ENQUEUE_FIFO_ORDER
#define MQX_DELAY_ENQUEUE_FIFO_ORDER    0  /* 0=LIFO, 1=FIFO */
#endif
#if MQX_DELAY_ENQUEUE_FIFO_ORDER
#define MQX_DELAY_ENQUEUE_POLICY(result)  (result > 0)  /* FIFO original */
#else
#define MQX_DELAY_ENQUEUE_POLICY(result)  (result >= 0) /* LIFO faster */
#endif

/*--------------------------------------------------------------------------*/
/*
 *                    TYPEDEFS FOR _CODE_PTR_ FUNCTIONS
 */
typedef void (_CODE_PTR_  TIMER_NOTIFICATION_FPTR)(void);

/*--------------------------------------------------------------------------*/
/*                       DATATYPE DEFINITIONS                               */

/*  TIMER ENTRY STRUCT */
/*!
 * \cond DOXYGEN_PRIVATE
 *  
 * \brief This structure defines what an entry in the timer table looks like.
 */
typedef struct timer_entry_struct
{

   /*! \brief Queue pointers. */
   QUEUE_ELEMENT_STRUCT QUEUE_ELEMENT;
   
   /*! \brief The requested timer frequency. */
   MQX_TICK_STRUCT      CYCLE;

   /*! \brief Where the expiry time is kept to know when next to fire the timer. */
   MQX_TICK_STRUCT      EXPIRATION_TIME;

   /*! \brief The function to call when the timer expires. */
   TIMER_NOTIFICATION_FPTR NOTIFICATION_FUNCTION;

   /*! \brief The data to pass as parameter 2 when calling the expiration function. */
   void                *NOTIFICATION_DATA_PTR;
   
   /*! \brief Task requesting this service. */
   TD_STRUCT_PTR        TD_PTR;

   /*! \brief Validity field for this structure. */
   _mqx_uint            VALID;

   /*! \brief Set to true if using tick interface functions. */
   bool              USES_TICKS;

   /*! \brief What type of timer is this. */
   uint16_t              TIMER_TYPE;
   
   /*! \brief Time mode used. */
   uint16_t              MODE;

   /*! \brief Timer ID. */
   _timer_id            ID;

} TIMER_ENTRY_STRUCT, * TIMER_ENTRY_STRUCT_PTR;
/*! \endcond */

/* TIMER COMPONENT STRUCT */
/*!
 * \cond DOXYGEN_PRIVATE
 *  
 * \brief This structure contains the definitions used by the timer component.
 *  
 * The address of this structure is stored in the kernel data COMPONENT array.
 */
typedef struct timer_component_struct
{

   /*! \brief The queue of timers using the elapsed time. */
   QUEUE_STRUCT           ELAPSED_TIMER_ENTRIES;
   
   /*! \brief The queue of timers using the kernel time. */
   QUEUE_STRUCT           KERNEL_TIMER_ENTRIES;

   /*! \brief Mutual exclusion semaphore for the timer entries. */
   LWSEM_STRUCT           TIMER_ENTRIES_LWSEM;
  
   /*! \brief The timer validity field. */
   _mqx_uint              VALID;

   /*! \brief The task id of the timer task. */
   _task_id               TIMER_TID;
   
   /*! \brief The task descriptor of the timer task. */
   TD_STRUCT_PTR          TIMER_TD_PTR;
  
   /*! \brief Mutual exclusion semaphore for the timer ISR. */
   LWSEM_STRUCT           TIMER_ISR_LWSEM;

   /*! \brief Timer ID counter. */
   _mqx_uint              ID;

   /*! \brief The current timer entry being serviced. */
   TIMER_ENTRY_STRUCT_PTR ENTRY_PTR;

} TIMER_COMPONENT_STRUCT, * TIMER_COMPONENT_STRUCT_PTR;
/*! \endcond */

/*--------------------------------------------------------------------------*/
/*                       PROTOTYPE DEFINITIONS                              */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TAD_COMPILE__
extern _timer_id              _timer_alloc_id_internal(
   TIMER_COMPONENT_STRUCT_PTR);
extern void                   _timer_cleanup(TD_STRUCT_PTR);
extern TIMER_ENTRY_STRUCT_PTR _timer_find_entry_internal(
   TIMER_COMPONENT_STRUCT_PTR, _timer_id);
extern void      _timer_insert_queue_internal(QUEUE_STRUCT_PTR, 
   TIMER_ENTRY_STRUCT_PTR);
extern void      _timer_isr(void);
extern _timer_id _timer_start_oneshot_after_internal(TIMER_NOTIFICATION_FPTR,
   void *, _mqx_uint, MQX_TICK_STRUCT_PTR, bool);
extern _timer_id _timer_start_oneshot_at_internal(TIMER_NOTIFICATION_FPTR, 
   void *, _mqx_uint, MQX_TICK_STRUCT_PTR, bool);
extern _timer_id _timer_start_periodic_at_internal(TIMER_NOTIFICATION_FPTR,
   void *,  _mqx_uint, MQX_TICK_STRUCT_PTR, MQX_TICK_STRUCT_PTR, bool);
extern _timer_id _timer_start_periodic_every_internal(TIMER_NOTIFICATION_FPTR,
   void *, _mqx_uint, MQX_TICK_STRUCT_PTR, bool);
extern void      _timer_task(uint32_t);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __timer_prv_h__ */
/* EOF */

