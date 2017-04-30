#ifndef __pcb_mqxa_h__
#define __pcb_mqxa_h__
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
*   This file contains the definitions for the PCB device driver that
*   sends and receives packets over a asynchrnous serial device.  The
*   packets are in MQX IPC async packet format.
*
*
*END************************************************************************/


/*--------------------------------------------------------------------------*/
/*
**                          CONSTANT DECLARATIONS
*/

/*
** Initialization errors
*/
#define IO_PCB_MQXA_DEVICE_ALREADY_OPEN         (IO_PCB_ERROR_BASE|0x90)
#define IO_PCB_MQXA_INCORRECT_SERIAL_DEVICE     (IO_PCB_ERROR_BASE|0x91)

/*
**             PACKET STRUCTURE CONTROL FIELD BIT DEFINITIONS
*/

#define IO_PCB_MQXA_HDR_LITTLE_ENDIAN           (0x40)
#define IO_PCB_MQXA_DATA_LITTLE_ENDIAN          (0x20)

#define IO_PCB_MQXA_HDR_BIG_ENDIAN              (0x00)
#define IO_PCB_MQXA_DATA_BIG_ENDIAN             (0x00)

/*
** ENDIAN
** Indicates in control field of packet the endianness of the packet
*/
#define IO_PCB_MQXA_HDR_ENDIAN_MASK             (0x40)
#define IO_PCB_MQXA_DATA_ENDIAN_MASK            (0x20)

#if PSP_ENDIAN == MQX_LITTLE_ENDIAN
#define IO_PCB_MQXA_HDR_ENDIAN   IO_PCB_MQXA_HDR_LITTLE_ENDIAN
#define IO_PCB_MQXA_DATA_ENDIAN  IO_PCB_MQXA_DATA_LITTLE_ENDIAN
#else
#define IO_PCB_MQXA_HDR_ENDIAN   IO_PCB_MQXA_HDR_BIG_ENDIAN
#define IO_PCB_MQXA_DATA_ENDIAN  IO_PCB_MQXA_DATA_BIG_ENDIAN
#endif

#define IO_PCB_MQX_MUST_CONVERT_HDR_ENDIAN(ctrl) \
   (((ctrl) & IO_PCB_MQXA_HDR_ENDIAN_MASK) != IO_PCB_MQXA_HDR_ENDIAN)

#define IO_PCB_MQX_MUST_CONVERT_DATA_ENDIAN(ctrl) \
   (((ctrl) & IO_PCB_MQXA_DATA_ENDIAN_MASK) != IO_PCB_MQXA_DATA_ENDIAN)

/*--------------------------------------------------------------------------*/
/*
**                          DATATYPE DECLARATIONS
*/

/*
** IO_PCB_MQXA_PACKET_HEADER_STRUCT
** This structure defines what a packet header looks like for this
** MQX protocol.
*/
typedef struct io_pcb_mqxa_packet_header_struct
{
   uint16_t LENGTH;
#ifdef MQX_USE_32BIT_MESSAGE_QIDS
   uint16_t PADD;
   uint32_t ADDR1;
   uint32_t ADDR2;
#else
   uint16_t ADDR;
   uint16_t ADDR2;
#endif
   unsigned char CONTROL;
#ifdef MQX_USE_32BIT_MESSAGE_QIDS
   unsigned char RESERVED[3];
#else
   unsigned char RESERVED[1];
#endif
} IO_PCB_MQXA_PACKET_HEADER_STRUCT, * IO_PCB_MQXA_PACKET_HEADER_STRUCT_PTR;

/*
** IO_PCB_MQXA_INIT_STRUCT
** This structure contains the initialization information for the
** async_serial protocol
**
*/
typedef struct io_pcb_mqxa_init_struct
{

   /* The serial interrupt device to use */
   char      *IO_PORT_NAME;

   /* What baud rate to use */
   uint32_t    BAUD_RATE;

   /* Is the IO port polled? */
   bool    IS_POLLED;

   /* Maximum size of input packet */
   _mem_size  INPUT_MAX_LENGTH;

   /* What priority the tasks are to run at */
   _mqx_uint  INPUT_TASK_PRIORITY;
   _mqx_uint  OUTPUT_TASK_PRIORITY;

} IO_PCB_MQXA_INIT_STRUCT, * IO_PCB_MQXA_INIT_STRUCT_PTR;

/*--------------------------------------------------------------------------*/
/*
**                          C PROTOTYPES
*/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TAD_COMPILE__
extern _mqx_uint _io_pcb_mqxa_install(char *, void *);
#endif

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
