
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
*   watchdog component.
*
*
*END************************************************************************/
#ifndef __watchdog_prv_h__
#define __watchdog_prv_h__ 1

/*--------------------------------------------------------------------------*/
/*                        CONSTANT DEFINITIONS                              */

/* The number of tasks in the alarm table */
#define WATCHDOG_TABLE_SIZE   (16)

/* The watchdog validation number */
#define WATCHDOG_VALID        (_mqx_uint)(0x77646f67)     /* "wdog" */

/*--------------------------------------------------------------------------*/
/*                       DATATYPE DEFINITIONS                               */


/*  WATCHDOG ALARM TABLE STRUCTURE */
/*!
 * \cond DOXYGEN_PRIVATE
 * 
 * \brief Watchdog alarm table structure. 
 */
typedef struct watchdog_alarm_table_struct
{
   /*! \brief The next table if required. */
   struct watchdog_alarm_table_struct      *NEXT_TABLE_PTR;

   /*! \brief The tasks being monitored. */
   TD_STRUCT_PTR                            TD_PTRS[WATCHDOG_TABLE_SIZE];

} WATCHDOG_ALARM_TABLE_STRUCT, * WATCHDOG_ALARM_TABLE_STRUCT_PTR;
/*! \endcond */


/* WATCHDOG COMPONENT STRUCTURE */
/*!
 * \cond DOXYGEN_PRIVATE
 * 
 * \brief Watchdog component structure. 
 */
typedef struct watchdog_component_struct
{
   /*! \brief The table of alarms. */
   WATCHDOG_ALARM_TABLE_STRUCT ALARM_ENTRIES;

   /*! \brief Watchdog validation stamp. */
   _mqx_uint                    VALID;
   
   /*! \brief The function to call when the watchdog expires. */
   WATCHDOG_ERROR_FPTR          ERROR_FUNCTION;
   
   /*! \brief The old timer interrupt handler. */
   INT_ISR_FPTR                 TIMER_INTERRUPT_HANDLER;

   /*! \brief The interrupt vector. */
   _mqx_uint                    INTERRUPT_VECTOR;

} WATCHDOG_COMPONENT_STRUCT, * WATCHDOG_COMPONENT_STRUCT_PTR;
/*! \endcond */

/*--------------------------------------------------------------------------*/
/*                       PROTOTYPE DEFINITIONS                              */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TAD_COMPILE__
extern void     _watchdog_cleanup(TD_STRUCT_PTR);
extern void     _watchdog_isr(void *);
extern bool  _watchdog_start_internal(MQX_TICK_STRUCT_PTR);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __watchdog_prv_h__ */
/* EOF */

