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
*   This include file is used to provide information needed by
*   applications using the USB DCD I/O functions.
*
*
*END************************************************************************/

#ifndef _usb_dcd_h_
#define _usb_dcd_h_ 1

#include <ioctl.h>


/*--------------------------------------------------------------------------*/
/*
**                            CONSTANT DEFINITIONS
*/

/* 
** USB DCD Sequence 
*/
enum 
{
   VBUS_DETECTION = 0,
   USB_DCD_ACTIVE = 1,
   USB_DCD_SEQ_STAT0 = 2,
   USB_DCD_SEQ_STAT1 = 3,
   USB_DCD_SEQ_RES0 = 4,
   USB_DCD_SEQ_RES1 = 5,
   USB_DCD_ISR_SIGNAL = 6,
   USB_DCD_TO = 7,
   USB_DCD_ERR = 8 ,
   USB_DCD_DEBUG = 9
};

/* 
** USB Charger Type 
*/
enum
{
	 STANDARD_HOST = 1,
	 CHARGING_HOST = 2,
	 DEDICATED_CHARGER = 3	
};

/* 
** USB DCD state  
*/
enum
{
	 USB_DCD_DISABLE = 0,
	 USB_DCD_ENABLE,
	 USB_DCD_SEQ_COMPLETE	 
};

/* 
** USB DCD METHOD
*/
enum
{
	 USB_DCD_POLLED = 0,
	 USB_DCD_INT
};

/* 
** USB DCD Registers reset value 
*/
#define USBDCD_CLOCK_SPEED_RESET_VALUE    (48000) /* 48 Mhz*/
#define USBDCD_TSEQ_INIT_RESET_VALUE      (0x10)
#define USB_DCD_TDCD_DBNC_RESET_VALUE		(0x0A)
#define USB_DCD_TVDPSRC_ON_RESET_VALUE		(0x28)
#define USB_DCD_TVDPSRC_CON_RESET_VALUE	(0x28)
#define USB_DCD_CHECK_DM_RESET_VALUE		(0x01)
#define USB_DCD_LEVEL_RESET_VALUE			(1)

/* 
** IOCTL calls specific to USB_DC 
*/
#define IO_IOCTL_USB_DCD_SET_TSEQ_INIT                _IO(IO_TYPE_USB_DCD,0x01)
#define IO_IOCTL_USB_DCD_GET_TSEQ_INIT                _IO(IO_TYPE_USB_DCD,0x02)
#define IO_IOCTL_USB_DCD_GET_TUNITCON                 _IO(IO_TYPE_USB_DCD,0x03)
#define IO_IOCTL_USB_DCD_SET_TDCD_DBNC                _IO(IO_TYPE_USB_DCD,0x04)
#define IO_IOCTL_USB_DCD_GET_TDCD_DBNC                _IO(IO_TYPE_USB_DCD,0x05)
#define IO_IOCTL_USB_DCD_SET_TVDPSRC_ON					_IO(IO_TYPE_USB_DCD,0x06)
#define IO_IOCTL_USB_DCD_GET_TVDPSRC_ON					_IO(IO_TYPE_USB_DCD,0x07)
#define IO_IOCTL_USB_DCD_SET_TVDPSRC_CON					_IO(IO_TYPE_USB_DCD,0x08)	
#define IO_IOCTL_USB_DCD_GET_TVDPSRC_CON					_IO(IO_TYPE_USB_DCD,0x09)
#define IO_IOCTL_USB_DCD_SET_CHECK_DM   					_IO(IO_TYPE_USB_DCD,0x0A)
#define IO_IOCTL_USB_DCD_GET_CHECK_DM   					_IO(IO_TYPE_USB_DCD,0x0B)
#define IO_IOCTL_USB_DCD_SET_CLOCK_SPEED     			_IO(IO_TYPE_USB_DCD,0x0C)
#define IO_IOCTL_USB_DCD_GET_CLOCK_SPEED     			_IO(IO_TYPE_USB_DCD,0x0D)
#define IO_IOCTL_USB_DCD_GET_STATUS				  			_IO(IO_TYPE_USB_DCD,0x0E)
#define IO_IOCTL_USB_DCD_RESET    				  			_IO(IO_TYPE_USB_DCD,0x0F)
#define IO_IOCTL_USB_DCD_START    				  			_IO(IO_TYPE_USB_DCD,0x10)
#define IO_IOCTL_USB_DCD_GET_STATE  			  			_IO(IO_TYPE_USB_DCD,0x11)
#define IO_IOCTL_USB_DCD_GET_CHARGER_TYPE				   _IO(IO_TYPE_USB_DCD,0x12)  


/* 
** USB DCD ERROR CODE
*/

#define USB_DCD_OK												0x00
#define USB_DCD_ERROR_CHANNEL_INVALID  					0x01
#define USB_DCD_INIT_INVALID									0x02
#define USB_DCD_CLOCK_SPEED_INVALID							0x03
#define USB_DCD_ERROR_INVALID_PARAMETER					0x04
#define USB_DCD_ERROR_INVALID_CMD							0x05
#define USB_DCD_ERROR_INVALID_INT_VECTOR  				0x06

/*--------------------------------------------------------------------------*/
/*
**                            FUNCTION PROTOTYPES
*/

#ifdef __cplusplus
extern "C" {
#endif

extern _mqx_uint _io_usb_dcd_polled_install(
      char *,
      _mqx_uint (_CODE_PTR_)(void *, void **, char *),
      _mqx_uint (_CODE_PTR_)(void *, void *),
      _mqx_int (_CODE_PTR_)(void *, char *, _mqx_int),
      _mqx_int (_CODE_PTR_)(void *, char *, _mqx_int),
      _mqx_int (_CODE_PTR_)(void *, _mqx_uint, _mqx_uint_ptr),
      void *);

extern _mqx_uint _io_usb_dcd_int_install(
      char *,
      _mqx_uint (_CODE_PTR_)(void *, char *),
      _mqx_uint (_CODE_PTR_)(void *, void *),
      _mqx_int (_CODE_PTR_)(void *, char *, _mqx_int),
      _mqx_int (_CODE_PTR_)(void *, char *, _mqx_int),
      _mqx_int (_CODE_PTR_)(void *, _mqx_uint, _mqx_uint_ptr),
      void *);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
