#ifndef __serial_h__
#define __serial_h__ 1
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
*   This include file is used to provide information needed by
*   applications using the serial I/O functions.
*
*
*END************************************************************************/

#include "ioctl.h"

/*--------------------------------------------------------------------------*/
/*
**                            CONSTANT DEFINITIONS
*/

/* Incoming and outgoing data not processed */
#define IO_SERIAL_RAW_IO             (0)

/* Perform xon/xoff processing */
#define IO_SERIAL_XON_XOFF           (0x01)

/*
** Perform translation :
**    outgoing \n to CR\LF
**    incoming CR to \n
**    incoming backspace erases previous character
*/
#define IO_SERIAL_TRANSLATION        (0x02)

/* echo incoming characters */
#define IO_SERIAL_ECHO               (0x04)

/* Perform hardware flow control processing */
#define IO_SERIAL_HW_FLOW_CONTROL    (0x08)

/*  */
#define IO_SERIAL_NON_BLOCKING       (0x10)

/* RS485 flags */
#define IO_SERIAL_HW_485_FLOW_CONTROL (0x20)

/* Multidrop flag */
#define IO_SERIAL_MULTI_DROP          (0x40)

/* Serial I/O IOCTL commands */
#define IO_IOCTL_SERIAL_GET_FLAGS        _IO(IO_TYPE_SERIAL,0x01)
#define IO_IOCTL_SERIAL_SET_FLAGS        _IO(IO_TYPE_SERIAL,0x02)
#define IO_IOCTL_SERIAL_GET_BAUD         _IO(IO_TYPE_SERIAL,0x03)
#define IO_IOCTL_SERIAL_SET_BAUD         _IO(IO_TYPE_SERIAL,0x04)
#define IO_IOCTL_SERIAL_GET_STATS        _IO(IO_TYPE_SERIAL,0x05)
#define IO_IOCTL_SERIAL_CLEAR_STATS      _IO(IO_TYPE_SERIAL,0x06)
#define IO_IOCTL_SERIAL_TRANSMIT_DONE    _IO(IO_TYPE_SERIAL,0x07)
#define IO_IOCTL_SERIAL_GET_CONFIG       _IO(IO_TYPE_SERIAL,0x08)

#define IO_IOCTL_SERIAL_GET_HW_SIGNAL    _IO(IO_TYPE_SERIAL,0x09)
#define IO_IOCTL_SERIAL_SET_HW_SIGNAL    _IO(IO_TYPE_SERIAL,0x0A)
#define IO_IOCTL_SERIAL_CLEAR_HW_SIGNAL  _IO(IO_TYPE_SERIAL,0x0B)
/* Standard HW signal names used with GET/SET/CLEAR HW SIGNAL */
#define IO_SERIAL_CTS                    (1)
#define IO_SERIAL_RTS                    (2)
#define IO_SERIAL_DTR                    (4)
#define IO_SERIAL_DSR                    (8)
#define IO_SERIAL_DCD                    (0x10)
#define IO_SERIAL_RI                     (0x20)
#define IO_SERIAL_BRK                    (0x40)

#define IO_IOCTL_SERIAL_SET_DATA_BITS    _IO(IO_TYPE_SERIAL,0x0C)
#define IO_IOCTL_SERIAL_GET_DATA_BITS    _IO(IO_TYPE_SERIAL,0x0D)
/* Value used with SET DATA BITS is just the integer number of bits */

#define IO_IOCTL_SERIAL_SET_STOP_BITS    _IO(IO_TYPE_SERIAL,0x0E)
#define IO_IOCTL_SERIAL_GET_STOP_BITS    _IO(IO_TYPE_SERIAL,0x0F)
/* Standard names used with SET STOP BITS */
#define IO_SERIAL_STOP_BITS_1            (1)
#define IO_SERIAL_STOP_BITS_1_5          (2)
#define IO_SERIAL_STOP_BITS_2            (3)

#define IO_IOCTL_SERIAL_SET_PARITY       _IO(IO_TYPE_SERIAL,0x10)
#define IO_IOCTL_SERIAL_GET_PARITY       _IO(IO_TYPE_SERIAL,0x11)
/* Standard parity names used with SET PARITY */
#define IO_SERIAL_PARITY_NONE            (1)
#define IO_SERIAL_PARITY_ODD             (2)
#define IO_SERIAL_PARITY_EVEN            (3)
#define IO_SERIAL_PARITY_FORCE           (4)
#define IO_SERIAL_PARITY_MARK            (5)
#define IO_SERIAL_PARITY_SPACE           (6)
#define IO_SERIAL_PARITY_MULTI_DATA      (7)
#define IO_SERIAL_PARITY_MULTI_ADDRESS   (8)

#define IO_IOCTL_SERIAL_START_BREAK      _IO(IO_TYPE_SERIAL,0x12)
#define IO_IOCTL_SERIAL_STOP_BREAK       _IO(IO_TYPE_SERIAL,0x13)
#define IO_IOCTL_SERIAL_TX_DRAINED       _IO(IO_TYPE_SERIAL,0x14)

#define IO_IOCTL_SERIAL_CAN_TRANSMIT     _IO(IO_TYPE_SERIAL,0x15)
#define IO_IOCTL_SERIAL_CAN_RECEIVE      _IO(IO_TYPE_SERIAL,0x16)

#define IO_IOCTL_SERIAL_DISABLE_RX       _IO(IO_TYPE_SERIAL,0x17)
#define IO_IOCTL_SERIAL_WAIT_FOR_TC      _IO(IO_TYPE_SERIAL,0x18)

#define IO_IOCTL_SERIAL_SET_RTL          _IO(IO_TYPE_SERIAL,0x19)
#define IO_IOCTL_SERIAL_GET_RTL          _IO(IO_TYPE_SERIAL,0x1A)
#define IO_IOCTL_SERIAL_SET_IRDA_TX      _IO(IO_TYPE_SERIAL,0x1B)
#define IO_IOCTL_SERIAL_SET_IRDA_RX      _IO(IO_TYPE_SERIAL,0x1C)
#define IO_IOCTL_SERIAL_SET_ADDRESS_DEVICE      _IO(IO_TYPE_SERIAL, 0x1D)
#define IO_IOCTL_SERIAL_GET_ADDRESS_DEVICE      _IO(IO_TYPE_SERIAL, 0x1E)

/*--------------------------------------------------------------------------*/
/*
**                      FUNCTION PROTOTYPES
*/

#ifdef __cplusplus
extern "C" {
#endif

extern void    _io_serial_default_init(void);

extern _mqx_uint _io_serial_polled_install(
      char *,
      _mqx_uint (_CODE_PTR_)(void *, void **, char *),
      _mqx_uint (_CODE_PTR_)(void *, void *),
      char    (_CODE_PTR_)(void *),
      void    (_CODE_PTR_)(void *, char),
      bool (_CODE_PTR_)(void *),
      _mqx_uint (_CODE_PTR_)(void *, _mqx_uint, void *),
      void *, _mqx_uint);

extern _mqx_uint _io_serial_int_install(
      char *,
      _mqx_uint (_CODE_PTR_)(void *, char *),
      _mqx_uint (_CODE_PTR_)(void *),
      _mqx_uint (_CODE_PTR_)(void *, void *),
      void     (_CODE_PTR_)(void *, char),
      _mqx_uint (_CODE_PTR_)(void *, _mqx_uint, void *),
      void *, _mqx_uint);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
