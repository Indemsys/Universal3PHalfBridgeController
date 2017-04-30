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
*   Include this header file to use BSP-specific USB initialization code
*
*
*END************************************************************************/

#ifndef __usb_bsp_h__
#define __usb_bsp_h__ 1

#if MQX_USE_IO_OLD
#define USBBSP_ERROR IO_ERROR 
#else
#define USBBSP_ERROR -1
#endif
//#include "../../../../usb/host/source/rtos/mqx/mqx_host.h"

#include "if_host_khci.h"
#include "if_host_ehci.h"
#include "if_dev_khci.h"
#include "if_dev_ehci.h"

/* Forward declaration of structure */
//struct usb_host_callback_functions_struct;

/* NOTE!!!: since both structures below are used in the array of structures for USB component, they must have the same size */
typedef struct usb_host_if_struct
{
   const struct usb_host_callback_functions_struct *HOST_IF;
   void                         *HOST_INIT_PARAM;
   void                         *HOST_HANDLE;
} USB_HOST_IF_STRUCT, * USB_HOST_IF_STRUCT_PTR;

typedef struct usb_dev_if_struct
{
   const struct usb_dev_callback_functions_struct *DEV_IF;
   void                         *DEV_INIT_PARAM;
   void                         *DEV_HANDLE;
} USB_DEV_IF_STRUCT, * USB_DEV_IF_STRUCT_PTR;

/* TODO: move to processor USB header files */
extern struct usb_dev_if_struct _bsp_usb_dev_ehci0_if;
extern struct usb_dev_if_struct _bsp_usb_dev_ehci1_if;

extern struct usb_dev_if_struct _bsp_usb_dev_khci0_if;
extern struct usb_dev_if_struct _bsp_usb_dev_khci1_if;

extern struct usb_host_if_struct _bsp_usb_host_ehci0_if;
extern struct usb_host_if_struct _bsp_usb_host_ehci1_if;

extern struct usb_host_if_struct _bsp_usb_host_khci0_if;
extern struct usb_host_if_struct _bsp_usb_host_khci1_if;


extern const USB_KHCI_HOST_INIT_STRUCT _khci0_host_init_param;
extern const USB_KHCI_HOST_INIT_STRUCT _khci1_host_init_param;

extern const USB_EHCI_HOST_INIT_STRUCT _ehci0_host_init_param;
extern const USB_EHCI_HOST_INIT_STRUCT _ehci1_host_init_param;

extern const USB_EHCI_DEV_INIT_STRUCT _ehci0_dev_init_param;
extern const USB_EHCI_DEV_INIT_STRUCT _ehci1_dev_init_param;

extern const USB_KHCI_DEV_INIT_STRUCT _khci0_dev_init_param;
extern const USB_KHCI_DEV_INIT_STRUCT _khci1_dev_init_param;

#ifdef __cplusplus
extern "C" {
#endif

_mqx_int _usb_host_driver_install(struct usb_host_if_struct *);
_mqx_int _usb_device_driver_install(struct usb_dev_if_struct *);
_mqx_int _usb_device_driver_uninstall(void);
_mqx_int _bsp_usb_dev_init(struct usb_dev_if_struct *);
_mqx_int _bsp_usb_host_init(struct usb_host_if_struct *);
void   *_usb_host_get_init(void *host_handle);


#ifdef __cplusplus
}
#endif

#endif

