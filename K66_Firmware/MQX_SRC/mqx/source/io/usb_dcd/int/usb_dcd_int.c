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
*   This file contains the USB DCD interrupt I/O driver functions.
*
*
*END************************************************************************/


#include <mqx_inc.h>
#include <fio.h>
#include <fio_prv.h>
#include <io.h>
#include <io_prv.h>
#include "usb_dcd.h"
#include "usb_dcd_int_prv.h"


/*FUNCTION****************************************************************
* 
* Function Name    : _io_usb_dcd_int_install
* Returned Value   : MQX error code
* Comments         :
*    Install the USB DCD device.
*
*END**********************************************************************/

_mqx_uint _io_usb_dcd_int_install
   (
      /* [IN] A string that identifies the device for fopen */
      char              *identifier,
  
      /* [IN] The I/O init function */
      _mqx_uint (_CODE_PTR_ init)(void *, char *),
      
      /* [IN] The I/O de-init function */
      _mqx_uint (_CODE_PTR_ deinit)(void *, void *),
      
      /* [IN] The input function */
      _mqx_int  (_CODE_PTR_ recv)(void *, char *, _mqx_int),
     
      /* [IN] The output function */
      _mqx_int  (_CODE_PTR_ xmit)(void *, char *, _mqx_int),

      /* [IN] The I/O ioctl function */
      _mqx_int  (_CODE_PTR_ ioctl)(void *, _mqx_uint, _mqx_uint_ptr),

      /* [IN] The I/O init data pointer */
      void                 *init_data_ptr
   )
{ /* Body */
   IO_USB_DCD_INT_DEVICE_STRUCT_PTR int_io_dev_ptr;

   int_io_dev_ptr = (IO_USB_DCD_INT_DEVICE_STRUCT_PTR)_mem_alloc_system_zero ((_mem_size)sizeof (IO_USB_DCD_INT_DEVICE_STRUCT));

   if (int_io_dev_ptr == NULL) 
   {
      return MQX_OUT_OF_MEMORY;
   }
   _mem_set_type (int_io_dev_ptr, MEM_TYPE_IO_USB_DCD_INT_DEVICE_STRUCT);            

   int_io_dev_ptr->DEV_INIT          = init;
   int_io_dev_ptr->DEV_DEINIT        = deinit;
   int_io_dev_ptr->DEV_READ          = recv;
   int_io_dev_ptr->DEV_WRITE         = xmit;
   int_io_dev_ptr->DEV_IOCTL         = ioctl;
   int_io_dev_ptr->DEV_INIT_DATA_PTR = init_data_ptr;
   
   return (_io_dev_install_ext(identifier,
      _io_usb_dcd_int_open, _io_usb_dcd_int_close,
      _io_usb_dcd_int_read, 
      _io_usb_dcd_int_write, 
      _io_usb_dcd_int_ioctl, _io_usb_dcd_int_uninstall,
      (void *)int_io_dev_ptr));

} /* Endbody */


/*FUNCTION****************************************************************
*
* Function Name    : _io_usb_dcd_int_uninstall
* Returned Value   : MQX error code
* Comments         :
*    UnInstall interrupt USB DCD device.
*
*END**********************************************************************/

_mqx_int _io_usb_dcd_int_uninstall
   (
      /* [IN] The IO device structure for the device */
      IO_DEVICE_STRUCT_PTR   io_dev_ptr
   )
{ /* Body */
   IO_USB_DCD_INT_DEVICE_STRUCT_PTR int_io_dev_ptr = io_dev_ptr->DRIVER_INIT_PTR;

   if (int_io_dev_ptr->COUNT == 0) 
   {
      if (int_io_dev_ptr->DEV_DEINIT) 
      {
         (*int_io_dev_ptr->DEV_DEINIT)(int_io_dev_ptr, int_io_dev_ptr->DEV_INFO_PTR);
      }
      _mem_free (int_io_dev_ptr);
      io_dev_ptr->DRIVER_INIT_PTR = NULL;
      return IO_OK;
   } else {
      return IO_ERROR_DEVICE_BUSY;
   }
} /* Endbody */


/*FUNCTION****************************************************************
* 
* Function Name    : _io_usb_dcd_int_open
* Returned Value   : MQX error code
* Comments         :
*    This routine initializes the USB DCD I/O channel. It acquires
*    memory, then stores information into it about the channel.
*    It then calls the hardware interface function to initialize the channel.
* 
*END**********************************************************************/

_mqx_int _io_usb_dcd_int_open
   (
      /* [IN] the file handle for the device being opened */
      FILE_DEVICE_STRUCT_PTR         fd_ptr,
       
      /* [IN] the remaining portion of the name of the device */
      char                       *open_name_ptr,

      /* [IN] the flags to be used during operation:
      ** echo, translation, xon/xoff.
      */
      char                       *flags
   )
{ /* Body */
   IO_DEVICE_STRUCT_PTR                  io_dev_ptr;
   IO_USB_DCD_INT_DEVICE_STRUCT_PTR      int_io_dev_ptr;
   _mqx_int                              result = MQX_OK;

   io_dev_ptr     = (IO_DEVICE_STRUCT_PTR)fd_ptr->DEV_PTR;
   int_io_dev_ptr = (void *)(io_dev_ptr->DRIVER_INIT_PTR);

   _int_disable ();
   if (int_io_dev_ptr->COUNT)
   {
      /* Device is already opened */
      _int_enable ();
      return MQX_IO_OPERATION_NOT_AVAILABLE;
   }
   int_io_dev_ptr->COUNT = 1;
   _int_enable ();
      
   int_io_dev_ptr->FLAGS = (_mqx_uint)flags;
   fd_ptr->FLAGS         = (_mqx_uint)flags;
            
   result = (*int_io_dev_ptr->DEV_INIT)(int_io_dev_ptr, open_name_ptr);
   if (result != MQX_OK) 
   {
      int_io_dev_ptr->COUNT = 0;
   }
   return result;
} /* Endbody */



/*FUNCTION****************************************************************
* 
* Function Name    : _io_usb_dcd_int_close
* Returned Value   : MQX error code
* Comments         :
*    This routine closes the USB DCD module
* 
*END**********************************************************************/

_mqx_int _io_usb_dcd_int_close
   (
      /* [IN] the file handle for the device being closed */
      FILE_DEVICE_STRUCT_PTR    fd_ptr
   )
{ /* Body */
   IO_DEVICE_STRUCT_PTR              io_dev_ptr;
   IO_USB_DCD_INT_DEVICE_STRUCT_PTR  int_io_dev_ptr;
   _mqx_int                          result = MQX_OK;

   io_dev_ptr     = (IO_DEVICE_STRUCT_PTR)fd_ptr->DEV_PTR;
   int_io_dev_ptr = (void *)io_dev_ptr->DRIVER_INIT_PTR;

   _int_disable();
   if (--int_io_dev_ptr->COUNT == 0) 
   {
      if (int_io_dev_ptr->DEV_DEINIT) 
      {
         result = (*int_io_dev_ptr->DEV_DEINIT)(int_io_dev_ptr, int_io_dev_ptr->DEV_INFO_PTR);
      }
   }
   _int_enable();
      
   return result;
} /* Endbody */


  
/*FUNCTION****************************************************************
* 
* Function Name    : _io_usb_dcd_int_read
* Returned Value   : 
* Comments         :
*    This routine calls the appropriate write routine.
*    
*
*END*********************************************************************/

_mqx_int _io_usb_dcd_int_read
   (
      /* [IN] the handle returned from _fopen */
      FILE_DEVICE_STRUCT_PTR          fd_ptr,

      /* [IN] where the characters are to be stored */
      char                        *data_ptr,

      /* [IN] the number of bytes to read */
      _mqx_int                        n
   )
{  /* Body */
   IO_DEVICE_STRUCT_PTR                   io_dev_ptr;
   IO_USB_DCD_INT_DEVICE_STRUCT_PTR       int_io_dev_ptr;
   
   io_dev_ptr     = (IO_DEVICE_STRUCT_PTR)fd_ptr->DEV_PTR;
   int_io_dev_ptr = (void *)io_dev_ptr->DRIVER_INIT_PTR;

   return (*int_io_dev_ptr->DEV_READ)(int_io_dev_ptr, data_ptr, n);
}  /* Endbody */



/*FUNCTION****************************************************************
* 
* Function Name    : _io_usb_dcd_int_write
* Returned Value   : 
* Comments         :
*    This routine calls the appropriate write routine.
*
*END**********************************************************************/

_mqx_int _io_usb_dcd_int_write
   (
      /* [IN] the handle returned from _fopen */
      FILE_DEVICE_STRUCT_PTR    fd_ptr,

      /* [IN] where the bytes are stored */
      char                  *data_ptr,

      /* [IN] the number of bytes to output */
      _mqx_int                  n
   )
{  /* Body */
   IO_DEVICE_STRUCT_PTR         io_dev_ptr;
   IO_USB_DCD_INT_DEVICE_STRUCT_PTR int_io_dev_ptr;
   
   io_dev_ptr     = (IO_DEVICE_STRUCT_PTR)fd_ptr->DEV_PTR;
   int_io_dev_ptr = (void *)io_dev_ptr->DRIVER_INIT_PTR;
               
   return (*int_io_dev_ptr->DEV_WRITE)(int_io_dev_ptr, data_ptr, n);
} /* Endbody */
   
   
   
/*FUNCTION*****************************************************************
* 
* Function Name    : _io_usb_dcd_int_ioctl
* Returned Value   : MQX error code
* Comments         :
*    Returns result of USB DCD ioctl operation.
*
*END*********************************************************************/

_mqx_int _io_usb_dcd_int_ioctl
   (
      /* [IN] the file handle for the device */
      FILE_DEVICE_STRUCT_PTR    fd_ptr,

      /* [IN] the ioctl command */
      _mqx_uint                 cmd,

      /* [IN] the ioctl parameters */
      void                     *input_param_ptr
   )
{ /* Body */
   IO_DEVICE_STRUCT_PTR         			io_dev_ptr;
   IO_USB_DCD_INT_DEVICE_STRUCT_PTR 	int_io_dev_ptr;
   _mqx_int                     			result = MQX_OK;
   _mqx_uint_ptr                			param_ptr = (_mqx_uint_ptr)input_param_ptr;

   io_dev_ptr     = (IO_DEVICE_STRUCT_PTR)fd_ptr->DEV_PTR;
   int_io_dev_ptr = (void *)io_dev_ptr->DRIVER_INIT_PTR;

    switch (cmd) {
        case IO_IOCTL_DEVICE_IDENTIFY :
            /* return the device identify */
            param_ptr[0] = IO_DEV_TYPE_PHYS_USB_DCD_INTERRUPT;
            param_ptr[1] = 0;
            param_ptr[2] = IO_DEV_ATTR_INTERRUPT | IO_DEV_ATTR_READ | IO_DEV_ATTR_WRITE;
            result = MQX_OK;    
            break;
        default:
            if (int_io_dev_ptr->DEV_IOCTL != NULL) {
                result = (*int_io_dev_ptr->DEV_IOCTL)(int_io_dev_ptr->DEV_INFO_PTR, cmd, param_ptr);
            }
    }
   
   return result;
} /* Endbody */

/* EOF */
