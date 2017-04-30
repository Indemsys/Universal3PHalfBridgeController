
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
*   This file contains the queue functions.
*
*
*END************************************************************************/

#ifndef __queue_h__
#define __queue_h__ 1

/*--------------------------------------------------------------------------*/
/*                     DATA STRUCTURE DEFINITIONS                           */



/*--------------------------------------------------------------------------*/
/* QUEUE ELEMENT STRUCTURE */
/*!
 * \brief Header for a queue element.
 *
 * This structure is required in each queue element.
 * The address of this structure is used to enqueue, dequeue elements.
 *
 * \see _queue_dequeue
 * \see _queue_enqueue
 * \see _queue_init
 * \see QUEUE_STRUCT
 */
typedef struct queue_element_struct
{

    /*! \brief Pointer to the next element in the queue. */
    struct queue_element_struct      *NEXT;

    /*! \brief Pointer to the previous element in the queue. */
    struct queue_element_struct      *PREV;

} QUEUE_ELEMENT_STRUCT, * QUEUE_ELEMENT_STRUCT_PTR;


/*--------------------------------------------------------------------------*/
/* QUEUE STRUCTURE */
/*!
 * \brief Queue of any type of element that has a header of type
 * QUEUE_ELEMENT_STRUCT.
 *
 * This structure represents a generic queue head structure. Each queue element
 * is made up of a data structure consisting of a NEXT pointer followed by a
 * PREV pointer. Thus any type of element may be queued onto this queue.
 *
 * \see _queue_init
 * \see QUEUE_ELEMENT_STRUCT
 */
typedef struct queue_struct
{
    /*!
     * \brief Pointer to the next element in the queue. If there are no elements
     * in the queue, the field is a pointer to the structure itself.
     */
    struct queue_element_struct      *NEXT;

    /*
     * \brief Pointer to the last element in the queue. If there are no elements
     * in the queue, the field is a pointer to the structure itself.
     */
    struct queue_element_struct      *PREV;

    /*! \brief Number of elements in the queue. */
    uint16_t                           SIZE;

    /*!
     * \brief Maximum number of elements that the queue can hold. If the field
     * is 0, the number is unlimited.
     */
    uint16_t                           MAX;

} QUEUE_STRUCT, * QUEUE_STRUCT_PTR;



/*--------------------------------------------------------------------------*/
/*                       EXTERNAL DECLARATIONS                              */

#ifdef __cplusplus
extern "C" {
#endif

extern QUEUE_ELEMENT_STRUCT_PTR _queue_dequeue  (QUEUE_STRUCT_PTR);
extern bool                  _queue_enqueue  (QUEUE_STRUCT_PTR,
                                                 QUEUE_ELEMENT_STRUCT_PTR);
extern _mqx_uint                _queue_get_size (QUEUE_STRUCT_PTR);
extern void                     _queue_init     (QUEUE_STRUCT_PTR, uint16_t);
extern bool                  _queue_insert   (QUEUE_STRUCT_PTR,
                                                 QUEUE_ELEMENT_STRUCT_PTR,
                                                 QUEUE_ELEMENT_STRUCT_PTR);
extern bool                  _queue_is_empty (QUEUE_STRUCT_PTR);
extern QUEUE_ELEMENT_STRUCT_PTR _queue_head     (QUEUE_STRUCT_PTR);
extern QUEUE_ELEMENT_STRUCT_PTR _queue_next     (QUEUE_STRUCT_PTR,
                                                 QUEUE_ELEMENT_STRUCT_PTR);
extern void                     _queue_unlink   (QUEUE_STRUCT_PTR,
                                                 QUEUE_ELEMENT_STRUCT_PTR);
extern _mqx_uint                _queue_test     (QUEUE_STRUCT_PTR,
                                                 void   **);

#ifdef __cplusplus
}
#endif

#endif  /* __queue_h__ */
/* EOF */
