#ifndef __pcbmqxav_h__
#define __pcbmqxav_h__
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
*   This file contains the private definitions for the MQX packet format
*   PCB packet drivers operating on serial async drivers.
*
*
*END************************************************************************/


/*--------------------------------------------------------------------------*/
/*
**                          CONSTANT DECLARATIONS
*/

#define IO_PCB_MQXA_STACK_SIZE   (750 * sizeof(_mem_size))

/* IO PROTOCOL DEFINITIONS */

#define AP_SYNC            (0x7e)

#define AP_STATE_SYNC         (1)
#define AP_STATE_SYNC_SKIP    (2)
#define AP_STATE_READING      (3)
#define AP_STATE_DONE         (4)
#define AP_STATE_CS0          (5)
#define AP_STATE_CS1          (6)

/* OUTPUT STATES */
#define AP_CRC_NONE           (0)
#define AP_CRC_0              (1)
#define AP_CRC_1              (2)

/*
** The following macro is used to calculate the checksums
*/
#if (PSP_MEMORY_ADDRESSING_CAPABILITY != 8)
   #define AP_CHECKSUM(c, cs0, cs1) \
      cs0 = (cs0 + (c)) & 0xFF; cs1 = ((cs1) + (cs0)) & 0xFF
#else
   #define AP_CHECKSUM(c, cs0, cs1) \
      cs0 += (c); cs1 += (cs0)
#endif

#define IO_PCB_MQXA_CNTL_OFFSET \
   (FIELD_OFFSET(IO_PCB_MQXA_PACKET_HEADER_STRUCT, CONTROL))

#define PKTIZE(data_ptr) ((IO_PCB_MQXA_PACKET_HEADER_STRUCT_PTR)(data_ptr))

#define MQXA_MSG_CONTROL_OFFSET  (6)
#define MQXA_MSG_LENGTH_OFFSET   (0)

#if (PSP_ENDIAN == MQX_BIG_ENDIAN)
#define GET_LENGTH(data_ptr) \
   ( IO_PCB_MQX_MUST_CONVERT_HDR_ENDIAN( ((unsigned char *)(data_ptr))[MQXA_MSG_CONTROL_OFFSET] ) \
     ? \
     ((((unsigned char *)(data_ptr))[MQXA_MSG_LENGTH_OFFSET]) & 0xFF) | \
     (((((unsigned char *)(data_ptr))[MQXA_MSG_LENGTH_OFFSET+1]) & 0xFF) << 8) \
     : \
     ((((unsigned char *)(data_ptr))[MQXA_MSG_LENGTH_OFFSET+1]) & 0xFF) | \
     (((((unsigned char *)(data_ptr))[MQXA_MSG_LENGTH_OFFSET]) & 0xFF) << 8) \
   )
#else
#define GET_LENGTH(data_ptr) \
   ( IO_PCB_MQX_MUST_CONVERT_HDR_ENDIAN( ((unsigned char *)(data_ptr))[MQXA_MSG_CONTROL_OFFSET] ) \
     ? \
     ((((unsigned char *)(data_ptr))[MQXA_MSG_LENGTH_OFFSET+1]) & 0xFF) | \
     (((((unsigned char *)(data_ptr))[MQXA_MSG_LENGTH_OFFSET]) & 0xFF) << 8) \
     : \
     ((((unsigned char *)(data_ptr))[MQXA_MSG_LENGTH_OFFSET]) & 0xFF) | \
     (((((unsigned char *)(data_ptr))[MQXA_MSG_LENGTH_OFFSET+1]) & 0xFF) << 8) \
   )
#endif

/*--------------------------------------------------------------------------*/
/*
**                          DATATYPE DECLARATIONS
*/

/*
** IO_PCB_MQXA_INFO_STRUCT
** This structure contains standard Bspio protocol information
**
*/
typedef struct io_pcb_mqxa_info_struct
{

   /* Serial ASYNC IO port to use */
   MQX_FILE_PTR    FD;

   /* Tasks created */
   _task_id        INPUT_TASK;
   _task_id        OUTPUT_TASK;
   
   /*  INPUT DEFINITIONS */
   LWSEM_STRUCT    READ_LWSEM;
   QUEUE_STRUCT    READ_QUEUE;
   
   /* Start CR 398 */
   MQX_FILE_PTR    CALLBACK_FD;/* CALLBACK File descriptor to use */
   /* End CR */
    void (_CODE_PTR_ READ_CALLBACK_FUNCTION)(FILE_DEVICE_STRUCT_PTR, 
      IO_PCB_STRUCT_PTR);
   _io_pcb_pool_id READ_PCB_POOL;
 
   /* OUTPUT DEFINITIONS */
   LWSEM_STRUCT    WRITE_LWSEM;
   QUEUE_STRUCT    WRITE_QUEUE;

   /* STATISTICAL INFORMATION */
   _mqx_uint       RX_PACKETS;
   _mqx_uint       RX_PACKETS_TOO_LONG;
   _mqx_uint       RX_PACKETS_BAD_CRC;
   _mqx_uint       RX_PACKETS_NO_MEMORY;
   _mqx_uint       TX_PACKETS;

   /* A copy of the initialization info */
   IO_PCB_MQXA_INIT_STRUCT INIT;

} IO_PCB_MQXA_INFO_STRUCT, * IO_PCB_MQXA_INFO_STRUCT_PTR;

/*--------------------------------------------------------------------------*/
/*
**                          C PROTOTYPES
*/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TAD_COMPILER__
extern _mqx_int _io_pcb_mqxa_open(FILE_DEVICE_STRUCT_PTR, char *, char *);
extern _mqx_int _io_pcb_mqxa_close(FILE_DEVICE_STRUCT_PTR);
extern _mqx_int _io_pcb_mqxa_read(FILE_DEVICE_STRUCT_PTR, char *, _mqx_int);
extern _mqx_int _io_pcb_mqxa_write(FILE_DEVICE_STRUCT_PTR, char*, _mqx_int);
extern _mqx_int _io_pcb_mqxa_ioctl(FILE_DEVICE_STRUCT_PTR, _mqx_uint, void *);
extern _mqx_int _io_pcb_mqxa_uninstall(IO_DEVICE_STRUCT_PTR);

extern void _io_pcb_mqxa_read_task(uint32_t);
extern void _io_pcb_mqxa_write_task(uint32_t);
#endif

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
