
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
*   This file contains the definitions private to the router used to route
*   IPC messages.
*
*
*END************************************************************************/
#ifndef __ipcrtprv_h__
#define __ipcrtprv_h__ 1

/*--------------------------------------------------------------------------*/
/*
 *                          CONSTANT DECLARATIONS
 */

/*--------------------------------------------------------------------------*/
/*
 *                    TYPEDEFS FOR _CODE_PTR_ FUNCTIONS
 */ 
typedef bool (_CODE_PTR_ IPC_MSGROUTER_FPTR)( _processor_number, void *, bool);
 
/*--------------------------------------------------------------------------*/
/*
 *                          DATATYPE DECLARATIONS
 */

/* IPC MSG ROUTING STRUCT */
/*!
 * \cond DOXYGEN_PRIVATE
 *  
 * \brief This structure contains info for a particular route.
 */
typedef struct ipc_msg_routing_struct
{

   /*! \brief Used to link all routing structures together. */
   QUEUE_ELEMENT_STRUCT  LINK;

   /*! \brief The minimum processor number in the range. */
   _processor_number     MIN_PROC_NUMBER;

   /*! \brief The maximum processor number in the range. */
   _processor_number     MAX_PROC_NUMBER;

   /*! \brief The queue to use if the processor number is in the above range. */
   _queue_number         QUEUE;

} IPC_MSG_ROUTING_STRUCT, * IPC_MSG_ROUTING_STRUCT_PTR;
/*! \endcond */


/* IPC MSG ROUTING COMPONENT STRUCT */
/*!
 * \cond DOXYGEN_PRIVATE
 *  
 * \brief The structure kept in the kernel data by the message routing component.
 */
typedef struct ipc_msg_routing_component_struct 
{
   /*! \brief Linked list of routes installed. */
   QUEUE_STRUCT         ROUTING_LIST;
   /*! \brief Message router function. */
   IPC_MSGROUTER_FPTR 	MSG_ROUTER;
   
} IPC_MSG_ROUTING_COMPONENT_STRUCT, * IPC_MSG_ROUTING_COMPONENT_STRUCT_PTR;
/*! \endcond */

/*--------------------------------------------------------------------------*/
/*
 *                          C PROTOTYPES
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TAD_COMPILE__
extern _mqx_uint _ipc_msg_route_init_internal( const IPC_ROUTING_STRUCT * route_ptr );
/*! \private */
extern bool _ipc_msg_route_internal(_processor_number, void *, bool);
#endif

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
