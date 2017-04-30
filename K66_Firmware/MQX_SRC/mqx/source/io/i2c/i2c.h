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
*   applications using the I2C I/O functions.
*
*
*END************************************************************************/

#ifndef _i2c_h_
#define _i2c_h_ 1

#include <ioctl.h>


/*--------------------------------------------------------------------------*/
/*
**                            CONSTANT DEFINITIONS
*/

/* 
** I2C internal states
*/
enum 
{
   I2C_STATE_READY = 0,
   I2C_STATE_REPEATED_START,
   I2C_STATE_TRANSMIT,
   I2C_STATE_RECEIVE,
   I2C_STATE_ADDRESSED_AS_SLAVE_RX,
   I2C_STATE_ADDRESSED_AS_SLAVE_TX,
   I2C_STATE_LOST_ARBITRATION,
   I2C_STATE_FINISHED
};

/* 
** IOCTL calls specific to I2C 
*/
#define IO_IOCTL_I2C_SET_BAUD                         _IO(IO_TYPE_I2C,0x01)
#define IO_IOCTL_I2C_GET_BAUD                         _IO(IO_TYPE_I2C,0x02)
#define IO_IOCTL_I2C_SET_MASTER_MODE                  _IO(IO_TYPE_I2C,0x03)
#define IO_IOCTL_I2C_SET_SLAVE_MODE                   _IO(IO_TYPE_I2C,0x04)
#define IO_IOCTL_I2C_GET_MODE                         _IO(IO_TYPE_I2C,0x05)
#define IO_IOCTL_I2C_SET_STATION_ADDRESS              _IO(IO_TYPE_I2C,0x06)
#define IO_IOCTL_I2C_GET_STATION_ADDRESS              _IO(IO_TYPE_I2C,0x07)
#define IO_IOCTL_I2C_SET_DESTINATION_ADDRESS          _IO(IO_TYPE_I2C,0x08)
#define IO_IOCTL_I2C_GET_DESTINATION_ADDRESS          _IO(IO_TYPE_I2C,0x09)
#define IO_IOCTL_I2C_SET_RX_REQUEST                   _IO(IO_TYPE_I2C,0x0A)
#define IO_IOCTL_I2C_REPEATED_START                   _IO(IO_TYPE_I2C,0x0B)
#define IO_IOCTL_I2C_STOP                             _IO(IO_TYPE_I2C,0x0C)
#define IO_IOCTL_I2C_GET_STATE                        _IO(IO_TYPE_I2C,0x0D)
#define IO_IOCTL_I2C_GET_STATISTICS                   _IO(IO_TYPE_I2C,0x0E)
#define IO_IOCTL_I2C_CLEAR_STATISTICS                 _IO(IO_TYPE_I2C,0x0F)
#define IO_IOCTL_I2C_DISABLE_DEVICE                   _IO(IO_TYPE_I2C,0x10)
#define IO_IOCTL_I2C_ENABLE_DEVICE                    _IO(IO_TYPE_I2C,0x11)
#define IO_IOCTL_I2C_GET_BUS_AVAILABILITY             _IO(IO_TYPE_I2C,0x12)

/* 
** I2C Bus Modes
*/
#define I2C_MODE_MASTER                               (0x00)
#define I2C_MODE_SLAVE                                (0x01)

/* 
** I2C Bus Operations
*/
#define I2C_OPERATION_WRITE                           (0x00)
#define I2C_OPERATION_READ                            (0x01)
#define I2C_OPERATION_STARTED                         (0x02)

/* 
** I2C Bus Availability
*/
#define I2C_BUS_IDLE                                  (0x00)
#define I2C_BUS_BUSY                                  (0x01)

/* 
** I2C Error Codes
*/
#define I2C_OK                                        (0x00)
#define I2C_ERROR_DEVICE_BUSY                         (I2C_ERROR_BASE | 0x01)
#define I2C_ERROR_CHANNEL_INVALID                     (I2C_ERROR_BASE | 0x02)
#define I2C_ERROR_INVALID_PARAMETER                   (I2C_ERROR_BASE | 0x03)
#define I2C_ERROR_LINE_TIMEOUT                        (I2C_ERROR_BASE | 0x04)

/*--------------------------------------------------------------------------*/
/*
**                    DATATYPE DECLARATIONS
*/
 
typedef struct i2c_statistics_struct 
{
   /* Number of I2C interrupts so far */
   uint32_t               INTERRUPTS;

   /* Number of valid bytes received (not dummy receives) */
   uint32_t               RX_PACKETS;

   /* Number of valid bytes transmitted (not dummy transmits) */
   uint32_t               TX_PACKETS;

   /* Number of times master lost arbitration */
   uint32_t               TX_LOST_ARBITRATIONS;

   /* Number of times master was addressed as slave */
   uint32_t               TX_ADDRESSED_AS_SLAVE;

   /* Number of not acknowledged (interrupted) transmits */
   uint32_t               TX_NAKS;
   
} I2C_STATISTICS_STRUCT, * I2C_STATISTICS_STRUCT_PTR;


/*--------------------------------------------------------------------------*/
/*
**                            FUNCTION PROTOTYPES
*/

#ifdef __cplusplus
extern "C" {
#endif

extern _mqx_uint _io_i2c_polled_install(
      char *,
      _mqx_uint (_CODE_PTR_)(void *, void **, char *),
      _mqx_uint (_CODE_PTR_)(void *, void *),
      _mqx_int (_CODE_PTR_)(void *, char *, _mqx_int),
      _mqx_int (_CODE_PTR_)(void *, char *, _mqx_int),
      _mqx_int (_CODE_PTR_)(void *, _mqx_uint, _mqx_uint_ptr),
      void *);

extern _mqx_uint _io_i2c_int_install(
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
