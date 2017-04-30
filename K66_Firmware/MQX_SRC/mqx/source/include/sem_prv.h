
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
*   This include file is used to define constants and data types for the
*   semaphore component.
*
*
*END************************************************************************/
#ifndef __sem_prv_h__
#define __sem_prv_h__ 1

/*--------------------------------------------------------------------------*/
/*                        CONSTANT DEFINITIONS                              */

/* Used to mark a block of memory as belonging to the semaphore component */
#define SEM_VALID                     ((_mqx_uint)(0x73656d20))   /* "sem " */

#define SEM_MAX_WAITING_TASKS         ((uint16_t)0)  /* Unlimited */

/* Flag bits for the connection */
#define SEM_WANT_SEMAPHORE            (0x1001)
#define SEM_AVAILABLE                 (0x1002)


/*--------------------------------------------------------------------------*/
/*                      DATA STRUCTURE DEFINITIONS                          */

/* SEM COMPONENT STRUCTURE */
/*!
 * \cond DOXYGEN_PRIVATE
 *  
 * \brief This is the base semaphore structure pointed to by the kernel data
 * structure COMPONENT field for semaphores
 */
typedef struct sem_component_struct
{

   /*! \brief The maximum number of semaphores allowed. */
   _mqx_uint        MAXIMUM_NUMBER;

   /*! \brief The number of semaphores to grow by when table full. */
   _mqx_uint        GROW_NUMBER;

   /*! \brief A validation stamp allowing for checking of memory overwrite. */
   _mqx_uint        VALID;

   /*! \brief The handle to the name table for semaphores. */
   void            *NAME_TABLE_HANDLE;

} SEM_COMPONENT_STRUCT, * SEM_COMPONENT_STRUCT_PTR;
/*! \endcond */

/* SEM STRUCTURE */
/*!
 * \cond DOXYGEN_PRIVATE
 *  
 * \brief This is the structure of an instance of a semaphore. 
 * 
 * The address is kept in the semaphore name table, associated with the name.
 */
typedef struct sem_struct
{

   /*! 
    * \brief This semaphore will be destroyed when all waiting tasks relinquish
    * their semaphore. Other tasks may not wait any more.
    */
   bool         DELAYED_DESTROY;

   /*! 
    * \brief Policy settings. Specifies whether this semaphore has 
    * PRIORITY_QUEUEING, and/or PRIORITY_INHERITANCE.
    */
   _mqx_uint       POLICY;

   /*! 
    * \brief This is the queue of waiting tasks.  
    * 
    * This queue may be sorted in order of task priority it INHERITANCE is 
    * enabled. What is queued is the address of the handle provided to the user 
    * (SEM_COMPONENT_STRUCT).
    */
   QUEUE_STRUCT    WAITING_TASKS;

   /*! \brief The queue of tasks; each of which owns a semaphore. */
   QUEUE_STRUCT    OWNING_TASKS;

   /*! \brief The current count of the semaphore. */
   _mqx_uint       COUNT;

   /*! 
    * \brief The maximum count allowed for this semaphore.
    *     
    * Only used when SEM_STRICT is TRUE.
    */
   _mqx_uint       MAX_COUNT;

   /*! \brief A validation stamp for the semaphore. */
   _mqx_uint       VALID;
   
   /*! \brief The string name of the semaphore, includes NULL. */
   char            NAME[NAME_MAX_NAME_SIZE];

} SEM_STRUCT, * SEM_STRUCT_PTR;
/*! \endcond */

/* SEM CONNECTION STRUCTURE */
/*!
 * \cond DOXYGEN_PRIVATE   
 * 
 * \brief This is the struture whose address is returned to the user as a 
 * semaphore handle.
 */
typedef struct sem_connection_struct
{

   /*! \brief Pointer to the next item in WAITING TASK queue of the semaphore. */ 
   void           *NEXT;
   /*! \brief Pointer to the previous item in WAITING TASK queue of the semaphore. */
   void           *PREV;

   /*! \brief A validation stamp for the data structure. */
   _mqx_uint       VALID;

   /*! \brief The number of times this task has been priority boosted. */
   _mqx_uint       BOOSTED;

   /* A reserved field */
   /* Start SPR P171-0009-01     */
   /* _mqx_uint        RESERVED; */
   /* End SPR P171-0009-01       */

   /*! 
    * \brief This field is only used for STRICT semaphores.
    *     
    * A count of the number of semaphores owned by this task.
    * \n This is incremented by _sem_wait, and decremented by _sem_post.
    * \n A semaphore may be posted only if a wait has previously succeeded.
    */
   /* Start SPR P171-0010-01 */
   /* int        POST_STATE; */
   _mqx_int        POST_STATE;
   /* End SPR P171-0010-01   */
   
   /*! \brief The address of the task descriptor that owns this connection. */
   TD_STRUCT_PTR   TD_PTR;

   /*! 
    * \brief The address of the semaphore structure associated with this
    * connection. 
    */
   SEM_STRUCT_PTR  SEM_PTR;

    
} SEM_CONNECTION_STRUCT, * SEM_CONNECTION_STRUCT_PTR;
/*! \endcond */

/* ANSI C prototypes */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TAD_COMPILE__
extern void      _sem_insert_priority_internal(QUEUE_STRUCT_PTR, 
   SEM_CONNECTION_STRUCT_PTR);
extern void      _sem_cleanup(TD_STRUCT_PTR);
extern _mqx_uint _sem_wait_internal(void *, MQX_TICK_STRUCT_PTR, bool);
#endif

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
