#ifndef __ticker_h__
#define __ticker_h__
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
* See license agreement file for full license terms including other
* restrictions.
*****************************************************************************
*
* Comments:
*
*   Timer interface definitions for the TCP layer.
*
*
*END************************************************************************/


/************************************************************************/
/* Constants                                                            */
/************************************************************************/

#if 0
#ifdef _MSDOS_
#define TICK_LENGTH   55   /* in milliseconds */
#else
#define TICK_LENGTH   5    /* in milliseconds */
#endif
#endif
#define TICK_LENGTH _time_get_resolution()


/************************************************************************/
/* Macros                                                               */
/************************************************************************/

#define TimerInit(timeq_ptr)  (_Zeromem((void *)(timeq_ptr), sizeof(TimeQ)))

/************************************************************************/
/* Typedefs                                                             */
/************************************************************************/

/*  The following structure is used for delays (where client blocks),
 *   oneshot timers, and cyclic timers, i.e. all items scheduled by
 *   the TickerServer.
 */
typedef struct TimeQ {

    struct TimeQ       *next;
    struct TimeQ       *prev;

    struct TimeQ            **donelist;
                              /* where to put completed oneshots */

    struct TimeQ            **curlist;
                              /* the list which currently holds the
                              ** timer (usually &qhead or donelist),
                              ** 0 if none */

    uint16_t       RESERVED;
    uint16_t       count;      /* nb of timeouts (ignored for delay) */
    int32_t        delay;      /* time delta before this node done */
    uint32_t       abs_to;     /* time at which this node should timeout */
    int32_t        reload;     /* <0==delay, 0==oneshot, >0==cyclic
                              ** timer's repeated delay */

/*  task_id       client; */  /* task to Signal() (timers) or
                                 Reply() to (delays); cleared when
                                 oneshot or delay has expired;
                                 if set, this means this timer is
                                 active, and in the schedule list
                                 (`qhead', see ticker.c) */

} TimeQ;

/************************************************************************/
/* Functions                                                            */
/************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void TCP_Timer_schedule
(
   TimeQ *,        /* IN/OUT - The timer to schedule */
   TimeQ **        /* IN/OUT - The head of the timer queue */
);

void  TCP_Timer_expire
(
   TimeQ *                /* IN/OUT - The expired timer */
);

void  TCP_Timer_remove
(
   TimeQ *                /* IN/OUT - The timer to remove */
);

int32_t TCP_Timer_stop
(
   TimeQ *,        /* The timer to stop */
   TimeQ **        /* The timer queue head */
);

int32_t TCP_Timer_start
(
   TimeQ *,        /* IN/OUT - The timer */
   int32_t            ,        /* IN     - No. msec until timeout */
   int32_t            ,        /* IN     - No. msec to dly after 1st to */
   TimeQ **        /* IN/OUT - Head of timer queue */
);

int32_t TCP_Timer_advance
(
   TimeQ *,        /* IN/OUT - timer to reschedule */
   int32_t            ,        /* IN     - no. msec to increment timeout */
   TimeQ **        /* IN/OUT - timer queue head */
);

int32_t TCP_Timer_oneshot_max
(
   TimeQ *,        /* IN/OUT - timer to start */
   int32_t            ,        /* IN     - timeout */
   TimeQ **        /* IN/OUT - timer queue head */
);

uint32_t TCP_Tick_server
(
   void
);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
