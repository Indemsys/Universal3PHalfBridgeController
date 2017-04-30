#ifndef __serinprv_h__
#define __serinprv_h__
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
*   This file includes the private definitions for the interrupt
*   driven serial I/O drivers.
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
** IO SERIAL INT DEVICE STRUCT
**
** This structure used to store information about a interrupt serial io device
** for the IO device table
*/
typedef struct io_serial_int_device_struct
{

   /* The I/O init function */
   _mqx_uint (_CODE_PTR_ DEV_INIT)(void *, char *);

   /* The enable interrupts function */
   _mqx_uint (_CODE_PTR_ DEV_ENABLE_INTS)(void *);

   /* The I/O deinit function */
   _mqx_uint (_CODE_PTR_ DEV_DEINIT)(void *, void *);

   /* The output function, used to write out the first character */
   void    (_CODE_PTR_ DEV_PUTC)(void *, char);

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

   /* The input queue */
   CHARQ_STRUCT_PTR    IN_QUEUE;

   /* The input waiting tasks */
   void               *IN_WAITING_TASKS;

   /* The output queue */
   CHARQ_STRUCT_PTR    OUT_QUEUE;

   /* The output waiting tasks */
   void               *OUT_WAITING_TASKS;

   /* Has output been started */
   bool             OUTPUT_ENABLED;
   
#if (PSP_MQX_CPU_IS_KINETIS || PSP_MQX_CPU_IS_VYBRID) 
   bool				TX_DMA_ONGOING;
#endif   
   /* Protocol flag information */
   _mqx_uint             HAVE_STOPPED_OUTPUT;
   _mqx_uint             HAVE_STOPPED_INPUT;
   _mqx_uint             MUST_STOP_INPUT;
   _mqx_uint             MUST_START_INPUT;
   _mqx_uint             INPUT_HIGH_WATER_MARK;
   _mqx_uint             INPUT_LOW_WATER_MARK;
   _mqx_uint             MUST_STOP_OUTPUT;
   
#if MQX_ENABLE_LOW_POWER
   
   /* Low power related state information */
   IO_SERIAL_LPM_STRUCT  LPM_INFO;
   
#endif

	LWSEM_STRUCT					 LWSEM;
} IO_SERIAL_INT_DEVICE_STRUCT, * IO_SERIAL_INT_DEVICE_STRUCT_PTR;


/*--------------------------------------------------------------------------*/
/*
**                            FUNCTION PROTOTYPES
*/

#ifdef __cplusplus
extern "C" {
#endif

/* Interrupt I/O prototypes */
extern _mqx_int  _io_serial_int_open(FILE_DEVICE_STRUCT_PTR, char *, 
   char *);
extern _mqx_int  _io_serial_int_close(FILE_DEVICE_STRUCT_PTR);
extern _mqx_int  _io_serial_int_read(FILE_DEVICE_STRUCT_PTR, char *, _mqx_int);
extern _mqx_int  _io_serial_int_write(FILE_DEVICE_STRUCT_PTR, char *, _mqx_int);
extern _mqx_int  _io_serial_int_ioctl(FILE_DEVICE_STRUCT_PTR, _mqx_uint, 
   void *);
extern _mqx_int _io_serial_int_uninstall(IO_DEVICE_STRUCT_PTR);

/* Callback Functions called by lower level interrupt I/O interrupt handlers */
extern bool _io_serial_int_addc(IO_SERIAL_INT_DEVICE_STRUCT_PTR, char);
extern _mqx_int  _io_serial_int_nextc(IO_SERIAL_INT_DEVICE_STRUCT_PTR);

/* Internal helper functions */
extern bool    _io_serial_int_putc_internal(IO_SERIAL_INT_DEVICE_STRUCT_PTR, 
   char, _mqx_uint);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
