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
*   This file contains the definitions of constants and structures
*   required for the ethernet drivers for the mk60 processor
*
*
*END************************************************************************/
#ifndef _macnet_mk60_h
#define _macnet_mk60_h

#define MACNET_DEVICE_0                0

#define MACNET_DEVICE_COUNT            1
   
#define MACNET_RX_BUFFER_ALIGNMENT     16  
#define MACNET_TX_BUFFER_ALIGNMENT     16
#define MACNET_BD_ALIGNMENT            16

/* Needs to be here, because a SMALL packet must be a multiple of the RX_BUFFER_ALIGNMENT */
#define MACNET_SMALL_PACKET_SIZE           ENET_ALIGN(64, MACNET_RX_BUFFER_ALIGNMENT)
       
/* MACNET 1588 TIMER the precise 1588timebase is linked to */
#define MACNET_PTP_TIMER  2 

/* EXTAL is used for clocking the 1588 timer */
#define MACNET_1588_CLOCK_SRC (BSP_CLOCK_SRC)

/* Value of the Timer Period Register - should be initialized to 1000000000
   to represent a timer wrap around of one second */
#define MACNET_1588_ATPER_VALUE (1000000000)

/* 1588 timer increment */
#define MACNET_1588_CLOCK_INC (MACNET_1588_ATPER_VALUE / (MACNET_1588_CLOCK_SRC))

#endif /* _macnet_mk60_h */

