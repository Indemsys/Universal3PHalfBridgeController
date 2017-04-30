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
*   This file contains board-specific USB initialization functions.
*
*
*END************************************************************************/

#include "mqx.h"
#include "bsp.h"
#include "bsp_prv.h"

struct usb_host_if_struct _bsp_usb_host_khci0_if = {
    &_usb_khci_host_callback_table,
    (void *) &_khci0_host_init_param,
    NULL
};

struct usb_host_if_struct _bsp_usb_host_ehci0_if = {
    &_usb_ehci_host_callback_table,
    (void *) &_ehci0_host_init_param,
    NULL
};

#define USBHS_USBMODE_CM_IDLE_MASK    USBHS_USBMODE_CM(0)
#define USBHS_USBMODE_CM_DEVICE_MASK  USBHS_USBMODE_CM(2)
#define USBHS_USBMODE_CM_HOST_MASK    USBHS_USBMODE_CM(3)

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_usb_host_init
* Returned Value   : 0 for success, -1 for failure
* Comments         :
*    This function performs BSP-specific initialization related to USB
*
*END*----------------------------------------------------------------------*/
_mqx_int _bsp_usb_host_init(struct usb_host_if_struct *usb_if)
{
    _mqx_int result;

    if (usb_if == NULL) {
        return USBBSP_ERROR;
    }

    result = _bsp_usb_host_io_init(usb_if);

    if (result != MQX_OK)
        return result;

    if (usb_if->HOST_INIT_PARAM == &_khci0_host_init_param) {
        /* Do not configure enable USB regulator for host */
//      SIM_SOPT1CFG_REG(SIM_BASE_PTR) |= SIM_SOPT1CFG_URWE_MASK;
//      SIM_SOPT1_REG(SIM_BASE_PTR) |= SIM_SOPT1_USBREGEN_MASK;

        /* reset USB CTRL register */
        USB_USBCTRL_REG(USB0_BASE_PTR) = 0;

        /* setup interrupt */
        _bsp_int_init(INT_USB0, BSP_USB_INT_LEVEL, 0, TRUE);
    }
    else if (usb_if->HOST_INIT_PARAM == &_ehci0_host_init_param) {
        USBHS_USBMODE = USBHS_USBMODE_CM_HOST_MASK;
        USBHS_USBCMD = USBHS_USBCMD_ASP(3) | USBHS_USBCMD_ITC(0);

        /* setup interrupt */
       _bsp_int_init(INT_USBHS, BSP_USB_INT_LEVEL, 0, TRUE);

    }
    else {
        /* unknown controller */
        result = USBBSP_ERROR;
    }

    return result;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_get_usb_base
* Returned Value   : Address upon success, NULL upon failure
* Comments         :
*    This function returns the address of USB OTG module
*
*END*----------------------------------------------------------------------*/
void *_bsp_get_usb_base(uint8_t dev_num)
{
    if (dev_num == 0) {
        return (void *)USB0_BASE_PTR;
    }
    else if (dev_num == 1) {
        return (void *)USBHS_BASE_PTR;
    }

    return NULL;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_get_usb_vector
* Returned Value   : Vector Number upon success, 0 upon failure
* Comments         :
*    This function returns the vector number of the VUSBHS Host Module
*
*END*----------------------------------------------------------------------*/
uint8_t _bsp_get_usb_vector
(
    _mqx_uint dev_num
)
{
    if (dev_num == 0) {
        return INT_USB0;
    }
    else if (dev_num == 1) {
        return INT_USBHS;
    }

    return 0;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_get_usb_capability_register_base
* Returned Value   : Address upon success, NULL upon failure
* Comments         :
*    This function returns the address of the VUSBHS Capability Registers
*
*END*----------------------------------------------------------------------*/
void *_bsp_get_usb_capability_register_base(uint8_t dev_num)
{
    if (dev_num == 1) {
        return (void *) ((unsigned char *) USBHS_BASE_PTR + 0x100);
    }

    return NULL;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_get_usb_timer_register_base
* Returned Value   : Address upon success, NULL upon failure
* Comments         :
*    This function returns the address of the VUSBHS Timer Registers
*
*END*----------------------------------------------------------------------*/
void *_bsp_get_usb_timer_register_base(uint8_t dev_num)
{
    if (dev_num == 1) {
        return (void *) ((unsigned char *) USBHS_BASE_PTR + 0x80);
    }

    return NULL;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_get_usb_otg_csr
* Returned Value   : Address upon success, NULL upon failure
* Comments         :
*    This function returns the address of the U0CSR register
*
*END*----------------------------------------------------------------------*/
void *_bsp_get_usb_otg_csr(uint8_t dev_num)
{
    return NULL;
}
