#ifndef __serplprv_h__
#define __serplprv_h__
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
*   This file includes the private definitions for the polled serial I/O
*   drivers.
*
*
*END************************************************************************/

/*--------------------------------------------------------------------------*/
/*
**                            CONSTANT DEFINITIONS
*/

/*
** Xon/Xoff protocol characters
*/
#define CNTL_S   ((char) 0x13)  /* Control S == XOFF.   */
#define CNTL_Q   ((char) 0x11)  /* Control Q == XON.    */

/*--------------------------------------------------------------------------*/
/*
**                            DATATYPE DECLARATIONS
*/


/*---------------------------------------------------------------------
**
** IO SERIAL POLLED DEVICE STRUCT
**
** This structure used to store information about a polled serial io device
** for the IO device table.
*/
typedef struct io_serial_polled_device_struct
{

   /* The I/O init function */
   _mqx_uint (_CODE_PTR_ DEV_INIT)(void *, void **, char *);

   /* The I/O deinit function */
   _mqx_uint (_CODE_PTR_ DEV_DEINIT)(void *, void *);

   /* The input function */
   char    (_CODE_PTR_ DEV_GETC)(void *);

   /* The output function */
   void    (_CODE_PTR_ DEV_PUTC)(void *, char);

   /* The status function, (character available) */
   bool (_CODE_PTR_ DEV_STATUS)(void *);

   /* The ioctl function, (change bauds etc) */
   _mqx_uint (_CODE_PTR_ DEV_IOCTL)(void *, _mqx_uint, void *);

   /* The I/O channel initialization data */
   void               *DEV_INIT_DATA_PTR;

   /* Device specific information */
   void               *DEV_INFO_PTR;

   /* The queue size to use */
   _mqx_uint             QUEUE_SIZE;

   /* Open count for number of accessing file descriptors */
   _mqx_uint             COUNT;

   /* Open flags for this channel */
   _mqx_uint             FLAGS;

   /* The Character Queue Ring buffer, for input buffering */
   CHARQ_STRUCT_PTR    CHARQ;

#if MQX_ENABLE_LOW_POWER

   /* Low power related state information */
   IO_SERIAL_LPM_STRUCT LPM_INFO;

#endif

} IO_SERIAL_POLLED_DEVICE_STRUCT, * IO_SERIAL_POLLED_DEVICE_STRUCT_PTR;

/*--------------------------------------------------------------------------*/
/*
**                            FUNCTION PROTOTYPES
*/

#ifdef __cplusplus
extern "C" {
#endif

/*
** Polled I/O prototypes
*/
extern _mqx_int _io_serial_polled_open(MQX_FILE_PTR, char *, char *);
extern _mqx_int _io_serial_polled_close(MQX_FILE_PTR);
extern _mqx_int _io_serial_polled_read(MQX_FILE_PTR, char *, _mqx_int);
extern _mqx_int _io_serial_polled_write(MQX_FILE_PTR, char *, _mqx_int);
extern _mqx_int _io_serial_polled_ioctl(MQX_FILE_PTR, _mqx_uint, void *);
extern _mqx_int _io_serial_polled_uninstall(IO_DEVICE_STRUCT_PTR);

#ifdef __cplusplus
}
#endif

#endif

