#ifndef __hwtimer_systick_h__
#define __hwtimer_systick_h__
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
*   hwtimer low level Systick modul.
*
*
*END************************************************************************/
#include "hwtimer.h"

/*! \brief Instance of HWTIMER_DEVIF_STRUCT structure initialized \n
 * with pointers to API functions the  systick driver implements
 */
extern const HWTIMER_DEVIF_STRUCT systick_devif;

#endif
