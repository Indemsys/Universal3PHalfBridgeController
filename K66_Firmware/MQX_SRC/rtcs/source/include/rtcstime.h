#ifndef __rtcstime_h__
#define __rtcstime_h__
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
*   TCP/IP internal Time Server definitions
*
*
*END************************************************************************/


/***************************************
**
** Type definitions
**
*/

/*
** A timer event
*/
typedef struct tcpip_event {

   uint32_t                    TIME;
   bool (_CODE_PTR_        EVENT)(struct tcpip_event *);
   void                      *PRIVATE;

   struct tcpip_event        *NEXT;

} TCPIP_EVENT, * TCPIP_EVENT_PTR;


/***************************************
**
** Prototypes
**
*/

#ifdef __cplusplus
extern "C" {
#endif

void TCPIP_Event_init(void);

void TCPIP_Event_add(TCPIP_EVENT_PTR);
void TCPIP_Event_cancel(TCPIP_EVENT_PTR);

uint32_t TCPIP_Event_expire(TCPIP_EVENT_PTR);

uint32_t TCPIP_Event_time(uint32_t);

#define TCPIP_Event_launch(var, time, event, event_private) \
         {                             \
            (var)->TIME = time;        \
            (var)->EVENT = event;      \
            (var)->PRIVATE = event_private;  \
            TCPIP_Event_add(var);      \
         }

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
