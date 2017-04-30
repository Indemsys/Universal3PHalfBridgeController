
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
*   This file containts definitions for use with light weight message queues
*
*
*END************************************************************************/

#ifndef __lwmsq_h__
#define __lwmsq_h__ 1

#include <mqx_cnfg.h>

#if (! MQX_USE_LWMSGQ) && (! defined (MQX_DISABLE_CONFIG_CHECK))
#warning LWMSGQ component is currently disabled in MQX kernel. Please set MQX_USE_LWMSGQ to 1 in user_config.h and recompile kernel.
#endif

/*--------------------------------------------------------------------------*/
/*
 *                            MACRO DEFINITIONS
 */

/* control bits for lwmsgq_send */

/* Block writing task if msgq if full */
#define LWMSGQ_SEND_BLOCK_ON_FULL           (0x01)

/* Block the sending task once message has been sent */
#define LWMSGQ_SEND_BLOCK_ON_SEND           (0x02)

/* Block the reading task if msgq is empty */
#define LWMSGQ_RECEIVE_BLOCK_ON_EMPTY       (0x04)

/* Perform a timeout using the tick structure as the absolute time */
#define LWMSGQ_TIMEOUT_UNTIL                (0x08)

/* Perform a timeout using the tick structure as the relative time */
#define LWMSGQ_TIMEOUT_FOR                  (0x10)

/* Error messages */
#define LWMSGQ_INVALID                      (MSG_ERROR_BASE | 0x80)
#define LWMSGQ_FULL                         (MSG_ERROR_BASE | 0x81)
#define LWMSGQ_EMPTY                        (MSG_ERROR_BASE | 0x82)
#define LWMSGQ_TIMEOUT                      (MSG_ERROR_BASE | 0x83)
#define LWMSGQ_DEINIT                       (MSG_ERROR_BASE | 0x84)

/* Helpful macros */

#define LWMSGQ_SIZE(lwq) (((LWMSGQ_STRUCT_PTR)(lwq))->CURRENT_SIZE)


/* Return whether the queue is empty */
#define LWMSGQ_IS_EMPTY(lwq) (((LWMSGQ_STRUCT_PTR)(lwq))->CURRENT_SIZE == 0)


/* Return whether the queue is full */
#define LWMSGQ_IS_FULL(q) \
   (((LWMSGQ_STRUCT_PTR)(q))->CURRENT_SIZE >= ((LWMSGQ_STRUCT_PTR)(q))->MAX_SIZE)

/*--------------------------------------------------------------------------*/
/*
 *                            DATATYPE DECLARATIONS
 */

/*---------------------------------------------------------------------*/

/* LWMSGQ STRUCTURE */

/*!
 * \brief This structure is used to store a circular long word queue.
 *
 * The structure must be the LAST if it is included into another data structure,
 * as the queue falls off of the end of this structure.
 */
typedef struct lwmsgq_struct
{
    /*! \brief Queue data structures. */
    QUEUE_ELEMENT_STRUCT LINK;

    /*! \brief A Queue of task descriptors waiting to write. */
    QUEUE_STRUCT WAITING_WRITERS;

    /*! \brief A Queue of task descriptors waiting to read. */
    QUEUE_STRUCT WAITING_READERS;

    /*! \brief The validity check field. */
    _mqx_uint    VALID;

    /*! \brief The size of the message chunk in the queue in _mqx_max_type's. */
    _mqx_uint    MSG_SIZE;

    /*!
     * \brief The maximum number of msgs for the queue, as specified in queue's
     * initialization.
     */
    _mqx_uint     MAX_SIZE;

    /*! \brief The current number of messages in the queue. */
    _mqx_uint    CURRENT_SIZE;

    /*! \brief Next message location to write to. */
    _mqx_max_type_ptr MSG_WRITE_LOC;

    /*! \brief Next message location to read from. */
    _mqx_max_type_ptr MSG_READ_LOC;

    /*! \brief Starting location of messages. */
    _mqx_max_type_ptr MSG_START_LOC;

    /*! \brief Location past end of messages. */
    _mqx_max_type_ptr MSG_END_LOC;

} LWMSGQ_STRUCT, * LWMSGQ_STRUCT_PTR;

/*--------------------------------------------------------------------------*/
/*
 * FUNCTION PROTOTYPES
 */

#ifdef __cplusplus
extern "C" {
#endif

_mqx_uint _lwmsgq_init(void *, _mqx_uint, _mqx_uint);
_mqx_uint _lwmsgq_deinit(void *);
_mqx_uint _lwmsgq_send(void *, _mqx_max_type_ptr, _mqx_uint);
_mqx_uint _lwmsgq_receive(void *, _mqx_max_type_ptr, _mqx_uint, _mqx_uint, MQX_TICK_STRUCT_PTR);

#if MQX_ENABLE_USER_MODE
_mqx_uint _usr_lwmsgq_init(void *, _mqx_uint, _mqx_uint);
_mqx_uint _usr_lwmsgq_deinit(void *);
_mqx_uint _usr_lwmsgq_send(void *, _mqx_max_type_ptr, _mqx_uint);
_mqx_uint _usr_lwmsgq_receive(void *, _mqx_max_type_ptr, _mqx_uint, _mqx_uint, MQX_TICK_STRUCT_PTR);

_mqx_uint _lwmsgq_usr_check(void *);
#endif /* MQX_ENABLE_USER_MODE */

#ifdef __cplusplus
}
#endif

#endif /* __lwmsq_h__ */
/* EOF */
