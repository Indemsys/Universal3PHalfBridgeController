
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
*   This include file is used to define constants and data types
*   private to the mutex component.
*
*
*END************************************************************************/
#ifndef __mutex_prv_h__
#define __mutex_prv_h__ 1

/*--------------------------------------------------------------------------*/
/*
 *                      CONSTANT DECLARATIONS
 */

/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief This structure defines the mutex component data structure.
 */
typedef struct mutex_component_struct
{

   /*! \brief A queue of all created mutexes. */
   QUEUE_STRUCT MUTEXES;

   /*! \brief A validation field for mutexes. */
   _mqx_uint     VALID;

} MUTEX_COMPONENT_STRUCT, * MUTEX_COMPONENT_STRUCT_PTR;
/*! \endcond */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TAD_COMPILE__
extern void _mutex_cleanup(TD_STRUCT_PTR);
#endif

#ifdef __cplusplus
}
#endif

#endif  /* __mutex_prv_h__ */
/* EOF */
