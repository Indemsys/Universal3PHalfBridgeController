#ifndef _usb_dcd_kn__prv_h
#define _usb_dcd_kn__prv_h
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
*   This file contains the definitions of constants and structures
*   required for the USB DCD drivers for Kinetis family.
*
*
*END************************************************************************/

#include "usb_dcd_kn.h"

/*--------------------------------------------------------------------------*/
/*
**                    DATATYPE DECLARATIONS
*/

/*
** KUSB_DCD_INFO_STRUCT
** Run time state information for each serial channel
*/
typedef struct kusb_dcd_info_struct
{  
	/* Current initialized values */
   KUSB_DCD_INIT_STRUCT              INIT;
   
   USBDCD_MemMapPtr                  USB_DCD_PTR;
   
   /* USB_DCD Method (polled or interrupt) */
   uint8_t										 METHOD;
   
   /* The previous interrupt handler and data */
   void                  (_CODE_PTR_ OLD_ISR)(void *);
   void                             *OLD_ISR_DATA;

   /* DCD DETECTION SEQUENCE STATE */
   uint8_t                            STATE;

} KUSB_DCD_INFO_STRUCT, * KUSB_DCD_INFO_STRUCT_PTR; 


#endif
/* EOF */
