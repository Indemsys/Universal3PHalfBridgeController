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
*   This file contains board-specific USB DCD initialization functions.
*
*
*END************************************************************************/


#include <mqx.h>
#include <bsp.h>
#include "usb_dcd_kn.h"

/*FUNCTION****************************************************************
*
* Function Name    : _bsp_get_usb_dcd_base_address
* Returned Value   : address if successful, NULL otherwise
* Comments         :
*    This function returns the base register address of the corresponding USB DCD device.
*
*END*********************************************************************/

void *_bsp_get_usb_dcd_base_address
(
)
{
    return (void *) USBDCD_BASE_PTR;
}

/*FUNCTION****************************************************************
*
* Function Name    : _bsp_get_usb_dcd_vector
* Returned Value   : vector number if successful, 0 otherwise
* Comments         :
*    This function returns desired interrupt vector number for specified USB DCD device.
*
*END*********************************************************************/

uint32_t _bsp_get_usb_dcd_vector
(
)
{
    return INT_USBDCD;
}
/* EOF */
