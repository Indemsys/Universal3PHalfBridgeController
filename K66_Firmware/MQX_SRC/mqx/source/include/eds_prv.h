
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
* See license agreement file for full license terms including other restrictions.
*****************************************************************************
*
* Comments:
*
*   IPC Embedded Debug Server private header file
*
*
*END************************************************************************/
#ifndef __eds_prv_h__
#define __eds_prv_h__

#include "message.h"
#include "msg_prv.h"
#include "ipc.h"
#include "ipc_prv.h"
#include "timer.h"

/*--------------------------------------------------------------------------*/
/*                        CONSTANT DEFINITIONS                              */

/* 
 * How long to wait for all mult-proc configuration messages to
 * return.
 */
#define EDS_MSG_WAIT_TIMEOUT  (5000) /* Msec */

/* server operations */
#define EDS_IDENTIFY                 0x00000001
#define EDS_LITTLE_ENDIAN_IDENTIFY   0x01000000
#define EDS_READ                     0x00000002
#define EDS_WRITE                    0x00000003
#define EDS_CONFIG_MULTIPROC         0x00000004
#define EDS_CONFIG_MULTIPROC_END     0x00000005
#define EDS_CONFIG_REQUEST_TIMEOUT   0x00000006
#define EDS_IDENTIFY_MULTIPROC       0x00000008
#define EDS_CONFIG_MULTIPROC_RESPOND 0x00000064


#define EDS_BIG_ENDIAN      0
#define EDS_LITTLE_ENDIAN   0xFFFFFFFF

/* error codes */
#define EDS_OK              0
#define EDS_INVALID_OP      1
#define EDS_INVALID_SIZE    2

/* Usefull sizes */
#define IPC_COMMAND_SIZE \
   (sizeof(MESSAGE_HEADER_STRUCT) + (2*sizeof(uint32_t)) + sizeof(_task_id))
#define EDS_COMMAND_SIZE  \
   (IPC_COMMAND_SIZE + sizeof(EDS_OP_STRUCT))
#define EDS_DATA_SIZE \
   (IPC_MAX_PARAMETERS - (sizeof(EDS_OP_STRUCT)/sizeof(uint32_t)))

/* Validates the EDS component */
#define EDS_VALID    ((_mqx_uint)0x65647376)   /* "edsv" */

/* EDS States */
#define EDS_IDLE                 0
#define EDS_CONFIG_IN_PROGRESS   1

/*
** MACROS
*/


/*--------------------------------------------------------------------------*/
/*                      DATA STRUCTURE DEFINITIONS                          */

/*!
 * \cond DOXYGEN_PRIVATE 
 * 
 * \brief EDS operation structure.
 * 
 * This structure is always BIG Endian. */
typedef struct eds_op_struct 
{
   /*! \brief Server operation. */
   uint32_t  OPERATION;   
   /*! \brief Read/write memory address. */
   uint32_t  ADDRESS;     
   /*! \brief Extra address field. */
   uint32_t  ADDRESS2;    
   /*! \brief Size of buffer. */
   uint32_t  SIZE;        
   /*! \brief Processor type. */
   uint32_t  PROCESSOR;   
   /*! \brief Endian of processor. */
   uint32_t  ENDIAN;      
   /*! \brief Error code. */
   uint32_t  EDS_ERROR;   
} EDS_OP_STRUCT, * EDS_OP_STRUCT_PTR;
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief EDS Message struct.
 *  
 * The EDS Message struct overlays the Parameters field of the IPC Message struct.
 */
typedef struct eds_msg_struct
{
   /*! \brief The EDS command to process, always in BIG endian format. */
   EDS_OP_STRUCT          OP_COMMAND;

   /*! \brief The maximum number of datums to send/receive. */
   uint32_t                DATA[EDS_DATA_SIZE];

} EDS_MSG_STRUCT, * EDS_MSG_STRUCT_PTR;
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief EDS component structure.
 */ 
typedef struct eds_component_struct
{
   /*! \brief Lightweight semaphore. */
   LWSEM_STRUCT           SEM;

   /*! \brief Pointer to saved messages. */
   IPC_MESSAGE_STRUCT_PTR SAVED_MSG_PTR;
   /*! \brief Valid flag. */
   _mqx_uint              VALID;
   /*! \brief state flag. */
   _mqx_uint              STATE;

   /*! \brief Timer ID. */
   _timer_id              TIMER_ID;
   /*! \brief Host queue ID. */
   _queue_id              HOST_QID;
   /*! \brief My queue ID. */
   _queue_id              MY_QID;
   /*! \brief Processor number whose responses are expected. */
   _processor_number      RESPONSES_EXPECTED;
   /*! \brief Host processor number. */
   _processor_number      HOST_PNUM;
   /*! \brief Queue number. */
   _queue_number          HOST_QNUM;

   /*! \brief Timeout time. */
   MQX_TICK_STRUCT        TIMEOUT;

} EDS_COMPONENT_STRUCT, * EDS_COMPONENT_STRUCT_PTR;
/*! \endcond */

/*--------------------------------------------------------------------------*/
/*                      PROTOTYPES                                          */


#ifdef __cplusplus
extern "C" {
#endif

extern _mqx_uint _eds_ipc_handler(void *);
extern void      _eds_ipc_return_message(IPC_MESSAGE_STRUCT_PTR);
extern void      _eds_timeout(_timer_id,void *,MQX_TICK_STRUCT_PTR);
 
#ifdef __cplusplus
}
#endif

#endif

/* EOF */
