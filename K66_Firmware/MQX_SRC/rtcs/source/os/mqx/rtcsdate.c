/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
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
*   This file contains the interface to the RTOS
*   date services.
*
*
*END************************************************************************/

#include <rtcsrtos.h>


/* Factor to convert milliseconds into fraction of a second */
#define SECOND_FRACTION_TO_MILLISECONDS     (0xFFFFFFFF / 1000)

/*
** Time, in secondes, between Jan 1 0h 1900, and Jan 1 0h 1970:
**
** 17 leap years: 17 days + 365 days * 70 years = 25567 days
** 25567 days * 24 hours/day * 60 minutes/hour * 60 seconds/minute
** = 2208988800 seconds
*/
#define SECONDS_1900_1970  2208988800u


/*FUNCTION*-------------------------------------------------------------
*
* Function Name    : RTCS_date_get
* Returned Value   : void
* Comments  :  Gets the current date in seconds and fraction of seconds
*     since 0h Jan 1 1900
*
*END*-----------------------------------------------------------------*/

void RTCS_date_get
   (
      uint32_t *seconds,
      /* [OUT] Elapsed seconds since 0h Jan 1 1900 */
      uint32_t *fraction
      /* [OUT] Second fraction in 2^-32 second units */
   )
{ /* Body */
   TIME_STRUCT time;

   _time_get(&time);
   *seconds = time.SECONDS + SECONDS_1900_1970;
   *fraction = time.MILLISECONDS * SECOND_FRACTION_TO_MILLISECONDS;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name    : RTCS_date_set
* Returned Value   : void
* Comments  :  Sets the current date in seconds and fraction of seconds
*     since 0h Jan 1 1900
*
*END*-----------------------------------------------------------------*/

void RTCS_date_set
   (
      uint32_t  seconds,
      /* [IN] Elapsed seconds since 0h Jan 1 1900 */
      uint32_t  fraction
      /* [IN] Fraction of a second in 2^-32 second units */
   )
{ /* Body */
   TIME_STRUCT time;

   time.SECONDS = seconds - SECONDS_1900_1970;
   time.MILLISECONDS = fraction / SECOND_FRACTION_TO_MILLISECONDS;
   _time_set(&time);
} /* Endbody */


/* EOF */
