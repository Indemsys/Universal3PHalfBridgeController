
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
*   This file contains the private definitions for the inter-processor
*   communications component.
*
*
*END************************************************************************/
#ifndef __ipc_prv__
#define __ipc_prv__

/*--------------------------------------------------------------------------*/
/*
 *                          CONSTANT DECLARATIONS
 */

/*
 * The number of ipc messages in the ipc message pool
 * which is created at initialization time.
 */
#define IPC_NUM_MESSAGES        (8)
#define IPC_GROW_MESSAGES       (8)
#define IPC_LIMIT_MESSAGES      (0)

/*
 * 2.3X message types 
 */
#define IPC_MQX_CREATE          (0x01)
#define IPC_MQX_DESTROY         (0x02)
#define IPC_MQX_ACTIVATE        (0x03)
//#define IPC_MQX_TYPE_MASK       (0x03)
#define IPC_MQX_ABORT           (0x05)  
#define IPC_MQX_RESTART         (0x06)
#define IPC_MQX_TYPE_MASK       (0x07)

/* Helpful macros for packing and unpacking the message type */
#define IPC_GET_COMPONENT(t) \
   (((uint32_t)(t) >> 16) & 0x3FF)
#define IPC_SET_COMPONENT(t,c)  \
   ((uint32_t)(t) | (((uint32_t)(c) & 0x3FF) << 16))
#define IPC_SET_IO_COMPONENT(t,c)  \
   ((uint32_t)(t) | ((((uint32_t)(c) + MAX_KERNEL_COMPONENTS) & 0x3FF) << 16))

#define IPC_GET_NON_BLOCKING(t) \
   ((uint32_t)(t) & 0x80000000)
#define IPC_SET_NON_BLOCKING(t,c) \
   ((c) ? ((uint32_t)(t) | 0x80000000) : ((uint32_t)(t)))

#define IPC_GET_TYPE(t) \
   (((uint32_t)(t) >> 8) & 0xFF)
#define IPC_SET_TYPE(t,c) \
   ((uint32_t)(t) | ((uint32_t)(c) & 0xFF) << 8)

#define IPC_SET_MESSAGE_TYPE(comp,type,non_blocking,val) \
   ((val) | \
   (((uint32_t)(comp) & 0x3FF) << 16) | \
   (((uint32_t)(type) & 0xFF) << 8) | \
   ((non_blocking) ? 0x80000000 : 0x0))

#define IPC_SET_IO_MESSAGE_TYPE(comp,type,non_blocking,val) \
   ((val) | \
   ((((uint32_t)(comp) + MAX_KERNEL_COMPONENTS) & 0x3FF) << 16) | \
   (((uint32_t)(type) & 0xFF) << 8) | \
   ((non_blocking) ? 0x80000000 : 0x0))

/* The ipc_task message queue number */
#define IPC_MESSAGE_QUEUE_NUMBER  (1)

/* The most number of parameters that can be sent via the IPC */
#if IPC_DATA_SIZE < 3
#error INVALID IPC DATA SIZE
#endif
#define IPC_MAX_PARAMETERS        (IPC_DATA_SIZE-3)

/* The last parameter is a string to send to the target */
#define IPC_STRING_PARAMETER      (0x8000)

/*--------------------------------------------------------------------------*/
/*
 *                         DATATYPE DECLARATIONS
 */

/* IPC MESSAGE STRUCT */
/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief This structure defines the message sent to the ipc task.
 */
typedef struct ipc_message_struct
{
   /*! \brief MQX standard message header. */
   MESSAGE_HEADER_STRUCT HEADER;

   /* 
    * The high bit is set if the message is non-blocking
    * Bits 25 to 16 indicate the component number.
    * Bits 15 to 8  indicate the message type for the component.
    * Bits 1  to 0  are used for task creation, destruction and
    *   activation.
    */
   /*! \brief Type of message. */
   uint32_t               MESSAGE_TYPE;

   /*! \brief The number of parameters to follow. */
   uint32_t               NUMBER_OF_PARAMETERS;
   
   /*! \brief The task ID of the requesting task. */
   _task_id              REQUESTOR_ID;

   /*! \brief The maximum number of parameters to send. */
   uint32_t               PARAMETERS[IPC_MAX_PARAMETERS];
   
} IPC_MESSAGE_STRUCT, * IPC_MESSAGE_STRUCT_PTR;
/*! \endcond */

/* IPC MQX MESSAGE STRUCTURE */
/*!
 * \cond DOXYGEN_PRIVATE
 * 
 * \brief This structure defines the ipc messages for 2.3x compatibility.
 */
typedef struct ipc_mqx_message_struct
{

   /*! \brief MQX standard message header. */
   MESSAGE_HEADER_STRUCT HEADER;

   /*! \brief Type of message (see ipc_message_struct). */
   uint32_t               MESSAGE_TYPE;

   /*!
    * \brief The task template index on the processor. If 0, then use the task 
    * template embedded in the message.
    */
   uint32_t               TEMPLATE_INDEX;

   /*! \brief The task ID of the requesting task. */
   _task_id              REQUESTOR_ID;

   /*! \brief The task ID of the task to be destroyed. */
   _task_id              VICTIM_ID;

   /*! \brief The task creation parameter for the task being created. */
   uint32_t               CREATE_PARAMETER;

   /*! \brief The embedded task template. */
   TASK_TEMPLATE_STRUCT  TEMPLATE;

} IPC_MQX_MESSAGE_STRUCT, * IPC_MQX_MESSAGE_STRUCT_PTR;
/*! \endcond */


/* IPC COMPONENT STRUCT */
/*! 
 * \cond DOXYGEN_PRIVATE
 * 
 * \brief This structure contains the component information for the IPC task.
 */
typedef struct ipc_component_struct
{
   /*! \brief Handlers for ipc components. */
   IPC_HANDLER_FPTR IPC_COMPONENT_HANDLER[MAX_KERNEL_COMPONENTS];

   /*! \brief Handlers for ipc components. */
   IPC_HANDLER_FPTR IPC_IO_COMPONENT_HANDLER[MAX_IO_COMPONENTS];

} IPC_COMPONENT_STRUCT, * IPC_COMPONENT_STRUCT_PTR;
/*! \endcond */
/*--------------------------------------------------------------------------*/
/*
 *                          C PROTOTYPES
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TAD_COMPILE__
extern _mqx_uint _ipc_send_internal(bool, _processor_number, _mqx_uint,
   _mqx_uint, _mqx_uint, ...);      
extern _mqx_uint _event_ipc_handler(void *imsg_ptr);
#endif

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
