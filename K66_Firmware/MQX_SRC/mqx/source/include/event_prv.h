
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
*   This include file is used to define constants and data types private
*   to the event component.
*
*
*END************************************************************************/
#ifndef __event_prv_h__
#define __event_prv_h__ 1

/*--------------------------------------------------------------------------*/
/*                        CONSTANT DEFINITIONS                              */

/* Used to mark a block of memory as belonging to an event group */
#define EVENT_VALID                    ((_mqx_uint)(0x65766E74))   /* "evnt" */

/* Used to indicate that an event occurred */
#define EVENT_OCCURRED                 (2)
#define EVENT_WANTS_ALL                (1)

#define EVENT_MAX_WAITING_TASKS        ((uint16_t)0)

/* IPC Message types for remote event access */
#define IPC_EVENT_OPEN                 (1)
#define IPC_EVENT_SET                  (2)

/*--------------------------------------------------------------------------*/
/*                      DATA STRUCTURE DEFINITIONS                          */

 
/* EVENT COMPONENT STRUCTURE */
/*!
 * \cond DOXYGEN_PRIVATE
 * 
 * \brief This is the base structure pointed to by the kernel data structure 
 * COMPONENT field for events.
 */
typedef struct event_component_struct
{

   /*! \brief The maximum number of event instances allowed. */
   _mqx_uint        MAXIMUM_NUMBER;

   /*! \brief The number of event instances to grow by when table full. */
   _mqx_uint        GROW_NUMBER;

   /*! \brief A validation stamp allowing for checking of memory overwrite. */
   _mqx_uint        VALID;

   /*! \brief The handle to the naming table for events. */
   void           *NAME_TABLE_HANDLE;

} EVENT_COMPONENT_STRUCT, * EVENT_COMPONENT_STRUCT_PTR;
/*! \endcond */

/* EVENT STRUCTURE */
/*! 
 * \cond DOXYGEN_PRIVATE
 * 
 * \brief This is the structure of an instance of an event. 
 * 
 * The address is kept in the event name table, associated with the name.
 */
typedef struct event_struct
{

   /*! 
    * \brief This is the queue of waiting tasks.
    *
    * What is queued is the address of the handle provided to the user 
    * (EVENT_COMPONENT_STRUCT).
    */
   QUEUE_STRUCT    WAITING_TASKS;

   /*! \brief This is a validation stamp for the event. */
   _mqx_uint       VALID;
   
   /*! \brief The actual event bits. */
   _mqx_uint       EVENT;

   /*! \brief Event type. */
   bool         AUTO_CLEAR;
   
   /*! \brief The string name of the event, includes null. */
   char            NAME[NAME_MAX_NAME_SIZE];

} EVENT_STRUCT, * EVENT_STRUCT_PTR;
/*! \endcond */

 
/* EVENT CONNECTION STRUCTURE */
/*!     
 * \cond DOXYGEN_PRIVATE
 * 
 * \brief This is the structure whose address is returned to the user
 * as an event handle.
 */
typedef struct event_connection_struct
{
   /*! 
    * \brief Pointer to the next structure. It is used to link the connection
    * struct onto the WAITING TASK queue of the event. 
    */     
   void             *NEXT;
   /*! 
    * \brief Pointer to the previous structure. It is used to link the connection
    * struct onto the WAITING TASK queue of the event. 
    */
   void             *PREV;

   /*! \brief This is a validation stamp for the data structure. */
   _mqx_uint         VALID;

   /*! 
    * \brief Is this event on a remote procssor, if non-zero it is the processor 
    * number of the remote processor.
    */
   _mqx_uint         REMOTE_CPU;

   /*! \brief The bit mask of bits to wait for. */
   _mqx_uint         MASK;
   
   /*! 
    * \brief A flag indicating whether all bits are required, or whether an event 
    * has been set.
    */
   _mqx_uint         FLAGS;
   
   /*! \brief The address of the task descriptor that owns this connection. */
   TD_STRUCT_PTR     TD_PTR;

   /*! \brief The address of the event structure associated with this connection. */
   EVENT_STRUCT_PTR  EVENT_PTR;

    
} EVENT_CONNECTION_STRUCT, * EVENT_CONNECTION_STRUCT_PTR;
/*! \endcond */

/* ANSI c prototypes */
#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TAD_COMPILE__
extern void      _event_cleanup(TD_STRUCT_PTR);
extern _mqx_uint _event_create_internal(char *, EVENT_STRUCT_PTR *);
extern _mqx_uint _event_create_fast_internal(_mqx_uint, EVENT_STRUCT_PTR *);
extern _mqx_uint _event_wait_internal(void *, _mqx_uint, MQX_TICK_STRUCT_PTR, bool, bool);
extern _mqx_uint _event_wait_any_internal(void *, _mqx_uint, MQX_TICK_STRUCT_PTR, bool);  
#endif

#ifdef __cplusplus
}
#endif

#endif /* __event_prv_h__ */
/* EOF */
