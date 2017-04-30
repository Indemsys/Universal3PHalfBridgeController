
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
*   This file contains the private definitions for the IO PCB
*   interprocessor drivers.
*
*
*END************************************************************************/
#ifndef __ipc_pcbv_h__
#define __ipc_pcbv_h__ 1

/*--------------------------------------------------------------------------*/
/*
 *                          CONSTANT DECLARATIONS
 */

/* Stack sizes */
#define IPC_PCB_STACK_SIZE IPC_DEFAULT_STACK_SIZE

/*--------------------------------------------------------------------------*/
/*
 *                          DATATYPE DECLARATIONS
 */



/* IPC_PCB_INFO_STRUCT */
/*!
 * \brief This structure contains protocol information for the IPC over PCBs.
 */
typedef struct ipc_pcb_info_struct
{
   /*! \brief Queue headers for keeping track of driver. */
   QUEUE_ELEMENT_STRUCT QUEUE;

   /*! \brief The IO PCB device to use. */
   MQX_FILE_PTR         FD;

   /*! \brief PCB input pool. */
   _io_pcb_pool_id      PCB_INPUT_POOL;
   /*! \brief Message input pool. */
   _pool_id             MSG_INPUT_POOL;
 
   /*! \brief PCB output pool. */
   _io_pcb_pool_id      PCB_OUTPUT_POOL;
   /*! \brief Message output pool. */
   _queue_id            OUT_MSG_QID;
   
   /*! \brief Number of sent messages. */
   _mqx_uint            OUTPUT_MESSAGE_COUNT;
   /*! \brief Number of received messages. */
   _mqx_uint            INPUT_MESSAGE_COUNT;

} IPC_PCB_INFO_STRUCT, * IPC_PCB_INFO_STRUCT_PTR;

/*--------------------------------------------------------------------------*/
/*
 *                          C PROTOTYPES
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TAD_COMPILE__
extern _mqx_uint _ipc_pcb_free(IO_PCB_STRUCT_PTR);
extern IO_PCB_STRUCT_PTR _ipc_pcb_alloc(IO_PCB_STRUCT_PTR, void *);
extern void _ipc_pcb_output_notification(void *);
extern void _ipc_pcb_input_notification(MQX_FILE_PTR, IO_PCB_STRUCT_PTR);
#endif

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
