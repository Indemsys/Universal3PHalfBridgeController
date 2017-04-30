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
*   This file contains the interface to the RTCS
*   time delay services.
*
*
*END************************************************************************/

#include <rtcs.h>
#include "rtcs_prv.h"
#include "rtcstime.h"


typedef struct rtcstime_delay_parm {
   TCPIP_PARM     COMMON;
   TCPIP_EVENT    EXPIRE;
   uint32_t        milliseconds;
} RTCSTIME_DELAY_PARM, * RTCSTIME_DELAY_PARM_PTR;


static void RTCSTIME_delay(RTCSTIME_DELAY_PARM_PTR);
static bool RTCSTIME_delay_expire(TCPIP_EVENT_PTR);


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_delay
* Returned Value  : void
* Comments        :
*
*  Blocks the calling task until the time (specified in milliseconds)
*  expires.
*
*END*-----------------------------------------------------------------*/

void RTCS_delay
   (
      /*[IN] time length in milliseconds to block task */
      uint32_t milliseconds
   )
{ /* Body */
   RTCSTIME_DELAY_PARM   parms;

   if (milliseconds) {
      parms.milliseconds = milliseconds;
      RTCSCMD_issue(parms, RTCSTIME_delay);
   } /* Endif */

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCSTIME_delay
* Returned Value  : void
* Comments        :
*
*  Adds an event to the TCPIP timer queue.
*
*END*-----------------------------------------------------------------*/

static void RTCSTIME_delay
   (
      RTCSTIME_DELAY_PARM_PTR     parms
   )
{ /* Body */

   parms->EXPIRE.TIME    = parms->milliseconds;
   parms->EXPIRE.EVENT   = RTCSTIME_delay_expire;
   parms->EXPIRE.PRIVATE = parms;
   TCPIP_Event_add(&parms->EXPIRE);

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCSTIME_delay_expire
* Returned Value  : FALSE
* Comments        :
*
*  Unblocks the task that called RTCS_time_delay
*
*END*-----------------------------------------------------------------*/

static bool RTCSTIME_delay_expire
   (
      TCPIP_EVENT_PTR   event
   )
{ /* Body */
   RTCSTIME_DELAY_PARM_PTR  parms = event->PRIVATE;

   RTCSCMD_complete(parms, RTCS_OK);
   return FALSE;

} /* Endbody */


/* EOF */
