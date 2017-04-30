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
*   This file contains the interface to the RTOS
*   time services.
*
*
*END************************************************************************/

#include <rtcsrtos.h>


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_time_get
* Returned Value  : milliseconds elapsed since boot
* Comments        : Called to get number of milliseconds since
*                   bootup.
*
*END*-----------------------------------------------------------------*/
uint32_t RTCS_time_get(void)
{
   TIME_STRUCT    time;

   _time_get_elapsed(&time);
   return (time.SECONDS * 1000 + time.MILLISECONDS);
} 


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_timer_get_interval
* Returned Value  : Interval value between two time moments
* Comments        : This function calculates an interval between two moments in time. 
*                   This function takes into account also a possible counter 
*                   overrun (start>end).
*
*END*-----------------------------------------------------------------*/
uint32_t RTCS_timer_get_interval( uint32_t start_time, uint32_t end_time)
{ 
    if(start_time <= end_time)
        return (end_time - start_time);
    else
        return (0xffffffff - start_time + end_time + 1);
}



