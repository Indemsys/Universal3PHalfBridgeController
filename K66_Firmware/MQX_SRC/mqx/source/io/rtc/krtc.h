/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
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
*   Processor family specific file needed for RTC module.
*
*
*END************************************************************************/

#ifndef __krtc_h__
#define __krtc_h__

/******************************************************************************
 * Interrupt masks definitions (RTC_ISR and RTC_IER registers)                *
 ******************************************************************************/

#define RTC_ISR_TOF            (0x02u)
#define RTC_ISR_TIF            (0x04u)
#define RTC_ISR_ERROR          (0x08u)

#endif   /* __krtc_h__ */

/* EOF */
