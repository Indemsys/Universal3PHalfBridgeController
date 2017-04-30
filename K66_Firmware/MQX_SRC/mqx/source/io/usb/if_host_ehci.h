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
*   The file exports USB KHCI host initial structure
*
*
*END************************************************************************/
#ifndef __if_host_ehci_h__
#define __if_host_ehci_h__ 1

#include <mqx.h> //pointer types

#ifdef __cplusplus
extern "C" {
#endif

extern const struct usb_host_callback_functions_struct _usb_ehci_host_callback_table;

enum _bsp_ehci_speed {
    BSP_EHCI_FS,
    BSP_EHCI_HS
};

typedef struct usb_ehci_host_init_struct {
    void     *BASE_PTR;
    void     *CAP_BASE_PTR;
    void     *TIMER_BASE_PTR;
    enum _bsp_ehci_speed SPEED; /* full speed, low speed, high speed... */
    _mqx_uint VECTOR;
    _mqx_uint PRIORITY;
    uint32_t   FRAME_LIST_SIZE;
} USB_EHCI_HOST_INIT_STRUCT, * USB_EHCI_HOST_INIT_STRUCT_PTR;

#ifdef __cplusplus
}
#endif

#endif
