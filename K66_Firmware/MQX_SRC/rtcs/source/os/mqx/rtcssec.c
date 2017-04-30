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
* Function Name   : RTCS_time_get_sec
* Returned Value  : seconds elapsed since boot
* Comments        : Called to get number of seconds since bootup.
*
*END*-----------------------------------------------------------------*/

uint32_t RTCS_time_get_sec
   (
      void
   )
{ /* Body */
   TIME_STRUCT    time;

   _time_get_elapsed(&time);
   return time.SECONDS;

} /* Endbody */


/* EOF */
