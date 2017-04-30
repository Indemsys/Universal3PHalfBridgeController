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

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_device_driver_install
*  Returned Value : None
*  Comments       :
*        Installs the device
*END*-----------------------------------------------------------------*/
_mqx_int _usb_device_driver_install
(
   /* [IN] address if the callback functions structure */
   USB_DEV_IF_STRUCT_PTR    usb_if
)
{ /* Body */
   USB_DEV_IF_STRUCT_PTR usb_c;
   _mqx_int      i;

   /* Check usb_if structure */
   if (usb_if == NULL) {
         return USBBSP_ERROR;
   }

   usb_c = _mqx_get_io_component_handle(IO_USB_COMPONENT);

   if (!usb_c) {
      usb_c = _mem_alloc_zero(USBCFG_MAX_DRIVERS * sizeof(USB_DEV_IF_STRUCT));

      if (!usb_c) {
         return USBBSP_ERROR;
      } /* Endif */
      /* TODO: change memory type */
      //_mem_set_type(usb_c, MEM_TYPE_USB_CALLBACK_STRUCT);
      _mem_transfer(usb_c, _task_get_id(), _mqx_get_system_task_id()); //this should return failure value anytime
      _mqx_set_io_component_handle(IO_USB_COMPONENT, (void *) usb_c);
      i = USBCFG_MAX_DRIVERS;
   }
   else {
      /* Find out if the interface has not been already installed */
      for (i = 0; i < USBCFG_MAX_DRIVERS; i++) {
         if (usb_c[i].DEV_INIT_PARAM == usb_if->DEV_INIT_PARAM) {
            /* The interface was already installed, skip search */
            break;
         }
      }
   }

   if (i == USBCFG_MAX_DRIVERS) {
      /* The interface was not installed yet, search for a free position. */
      for (i = 0; i < USBCFG_MAX_DRIVERS; i++) {
         if (usb_c[i].DEV_INIT_PARAM == NULL) {
            usb_c[i].DEV_IF = usb_if->DEV_IF;
            usb_c[i].DEV_INIT_PARAM = usb_if->DEV_INIT_PARAM;
            usb_c[i].DEV_HANDLE = NULL; /* not initialized yet */
            break;
         }
      }
   }
   else {
      /* The interface was already installed */
   }

   if (i == USBCFG_MAX_DRIVERS) {
      /* Not enough space to install interface */
      return USBBSP_ERROR;
   }

   return MQX_OK;

} /* EndBody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_dev_driver_uninstall
*  Returned Value : USB_OK or error code
*  Comments       :
*        Uninstalls the device
*END*-----------------------------------------------------------------*/
_mqx_int _usb_device_driver_uninstall
   (
        void
   )
{ /* Body */
   void   *callback_struct_table_ptr;
   callback_struct_table_ptr = _mqx_get_io_component_handle(IO_USB_COMPONENT);

   if (callback_struct_table_ptr)
   {
      _mem_free(callback_struct_table_ptr);
   }
   else
   {
      return USBBSP_ERROR;
   }
   /* Endif */
   _mqx_set_io_component_handle(IO_USB_COMPONENT, NULL);
   return MQX_OK;

} /* EndBody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_host_driver_install
*  Returned Value : None
*  Comments       :
*        Installs the host controller
*END*-----------------------------------------------------------------*/
_mqx_int _usb_host_driver_install
(
   /* [IN] address if the callback functions structure */
   USB_HOST_IF_STRUCT_PTR    usb_if
)
{ /* Body */
   USB_HOST_IF_STRUCT_PTR usb_c;
   _mqx_int      i;

   /* Check usb_if structure */
   if (usb_if == NULL) {
         return USBBSP_ERROR;
   }

   usb_c = _mqx_get_io_component_handle(IO_USB_COMPONENT);

   if (!usb_c) {
      usb_c = _mem_alloc_zero(USBCFG_MAX_DRIVERS * sizeof(USB_HOST_IF_STRUCT));

      if (!usb_c) {
         return USBBSP_ERROR;
      } /* Endif */
      /* TODO: change memory type */
      //_mem_set_type(usb_c, MEM_TYPE_USB_CALLBACK_STRUCT);
      _mem_transfer(usb_c, _task_get_id(), _mqx_get_system_task_id()); //this should return failure value anytime
      _mqx_set_io_component_handle(IO_USB_COMPONENT, (void *) usb_c);
      i = USBCFG_MAX_DRIVERS;
   }
   else {
      /* Find out if the interface has not been already installed */
      for (i = 0; i < USBCFG_MAX_DRIVERS; i++) {
         if (usb_c[i].HOST_INIT_PARAM == usb_if->HOST_INIT_PARAM) {
            /* The interface was already installed, skip search */
            break;
         }
      }
   }

   if (i == USBCFG_MAX_DRIVERS) {
      /* The interface was not installed yet, search for a free position. */
      for (i = 0; i < USBCFG_MAX_DRIVERS; i++) {
         if (usb_c[i].HOST_INIT_PARAM == NULL) {
            usb_c[i].HOST_IF = usb_if->HOST_IF;
            usb_c[i].HOST_INIT_PARAM = usb_if->HOST_INIT_PARAM;
            usb_c[i].HOST_HANDLE = NULL; /* not initialized yet */
            break;
         }
      }
   }
   else {
      /* The interface was already installed */
   }

   if (i == USBCFG_MAX_DRIVERS) {
      /* Not enough space to install interface */
      return USBBSP_ERROR;
   }

   return MQX_OK;

} /* EndBody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _usb_host_get_init
*  Returned Value : None
*  Comments       :
*        Gets the init parameters for the driver
*END*-----------------------------------------------------------------*/
void *_usb_host_get_init
(
   /* [IN] address if the callback functions structure */
   void      *host_handle
)
{ /* Body */
   USB_HOST_IF_STRUCT_PTR usb_c;
   void   *retval = NULL;
   int i;

   usb_c = _mqx_get_io_component_handle(IO_USB_COMPONENT);

   if (usb_c) {
      /* Find out if the interface has not been already installed */
      for (i = 0; i < USBCFG_MAX_DRIVERS; i++) {
         if (usb_c[i].HOST_HANDLE == host_handle) {
            retval = usb_c[i].HOST_INIT_PARAM;
            break;
         }
      }
   }

   return retval;

} /* EndBody */

/* EOF */
