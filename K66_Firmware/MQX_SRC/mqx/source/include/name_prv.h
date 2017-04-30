
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
*   name component.
*
*
*END************************************************************************/
#ifndef __name_prv_h__
#define __name_prv_h__ 1

/*--------------------------------------------------------------------------*/
/*                        CONSTANT DEFINITIONS                              */

/* Queue FIFO */
#define NAME_TASK_QUEUE_POLICY  (0)

#define NAME_VALID              (_mqx_uint)(0x6e616d65)   /* "name" */

/*--------------------------------------------------------------------------*/
/*                      DATA STRUCTURE DEFINITIONS                          */

/* NAME STRUCT */
/*!
 * \cond DOXYGEN_PRIVATE
 *   
 * \brief An individual pair of name/number.
 */
typedef struct name_struct
{
   /*! \brief The number associated with the name. */
   _mqx_max_type NUMBER;

   /*! \brief The string name stored in the table, includes null. */
   char      NAME[NAME_MAX_NAME_SIZE];

} NAME_STRUCT, * NAME_STRUCT_PTR;
/*! \endcond */

/* NAME COMPONENT STRUCT */
/*!
 *\cond DOXYGEN_PRIVATE 
 * 
 * \brief This structure is used to store information required for name retrieval.
 * 
 * An initial structure is created and when pool growth is required, duplicate 
 * copies of this structure are created and linked via the NEXT_TABLE field.
 */
typedef struct name_component_struct
{

   /*! \brief The maximum number of names allowed in the entire pool. */
   _mqx_uint      MAX_NUMBER;

   /*! \brief The total number of names allocated in the entire pool so far. */
   _mqx_uint      TOTAL_NUMBER;

   /*! \brief The number of names allowed in this block. */   
   _mqx_uint      NUMBER_IN_BLOCK;

   /*! \brief The number of names allowed to be created in the next block. */
   _mqx_uint      GROW_NUMBER;

   /*! \brief The number of names used in the name component. */
   _mqx_uint      NUMBER;

   /*! \brief Lightweight semaphore for protecting the name component. */
   LWSEM_STRUCT   SEM;

   /*! \brief A validation stamp to verify handle correctness. */
   _mqx_uint      VALID;

   /*! \brief The address of the next block of names. */
   struct  name_component_struct      *NEXT_TABLE;

   /*! \brief A variable sized array of name/number pairs. */
   NAME_STRUCT    NAMES[1];
   
} NAME_COMPONENT_STRUCT, * NAME_COMPONENT_STRUCT_PTR;
/*! \endcond */

/*--------------------------------------------------------------------------*/
/*                           EXTERNAL DECLARATIONS                          */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TAD_COMPILE__
extern _mqx_uint _name_add_internal(void *, char *, _mqx_max_type);
extern _mqx_uint _name_add_internal_by_index(void *, char *, _mqx_max_type, 
   _mqx_uint);
extern _mqx_uint _name_create_handle_internal(void **, _mqx_uint, _mqx_uint, 
   _mqx_uint, _mqx_uint);
extern _mqx_uint _name_destroy_handle_internal(void *);
extern _mqx_uint _name_delete_internal(void *, char *);
extern _mqx_uint _name_delete_internal_by_index(void *, _mqx_uint);
extern _mqx_uint _name_find_internal(void *, char *, _mqx_max_type_ptr);
extern _mqx_uint _name_find_name_internal(void *, _mqx_max_type, char *);
extern _mqx_uint _name_find_internal_by_index(void *, _mqx_uint, _mqx_max_type_ptr);
extern _mqx_uint _name_init_internal(void **, _mqx_uint, _mqx_uint, _mqx_uint,
   _mqx_uint);
extern _mqx_uint _name_test_internal(NAME_COMPONENT_STRUCT_PTR, void **, 
   void   **);
#endif

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
