
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
*   Log component.
*
*
*END************************************************************************/

#ifndef __lwlog_h__
#define __lwlog_h__ 1

#include <mqx_cnfg.h>

#if (! MQX_USE_LWLOGS) && (! defined (MQX_DISABLE_CONFIG_CHECK))
#error LWLOG component is currently disabled in MQX kernel. Please set MQX_USE_LWLOGS to 1 in user_config.h and recompile kernel.
#endif

#include "log.h"

/*--------------------------------------------------------------------------*/
/*                        CONSTANT DEFINITIONS                              */

/* Maximum number of data elements allowed in each entry (not including the header */
#define LWLOG_MAXIMUM_DATA_ENTRIES (7)

/*--------------------------------------------------------------------------*/
/*                        DATATYPE DECLARATIONS                             */

/*!
 * \brief Entry in kernel log or a lightweight log.
 *
 * \see _lwlog_read
 */
typedef struct lwlog_entry_struct
{

   /*! \brief Sequence number for the entry */
    _mqx_uint           SEQUENCE_NUMBER;

   /* The time stamp units are configurable */
/*! \cond */
#if MQX_LWLOG_TIME_STAMP_IN_TICKS == 0
/*! \endcond */
   /*!
    * \brief The time at which the entry was written <b> if MQX is not configured at
    * compile time to timestamp in ticks. </b>
    */
    uint32_t             SECONDS;
   /*!
    * \brief The time at which the entry was written <b> if MQX is not configured at
    * compile time to timestamp in ticks. </b>
    */
    uint32_t             MILLISECONDS;
   /*!
    * \brief The time at which the entry was written <b> if MQX is not configured at
    * compile time to timestamp in ticks. </b>
    */
     uint32_t            MICROSECONDS;
/*! \cond */
#else
/*! \endcond */
   /*!
    * \brief The time in tick time at which the entry was written <b> if MQX is
    * configured at compile time to timestamp in ticks. </b>
    */
    MQX_TICK_STRUCT     TIMESTAMP;
/*! \cond */
#endif
/*! \endcond */
    /*! \brief Data for the entry. */
    _mqx_max_type       DATA[LWLOG_MAXIMUM_DATA_ENTRIES];

    /*! \brief Pointer to the next lightweight-log entry.*/
    struct lwlog_entry_struct      *NEXT_PTR;

} LWLOG_ENTRY_STRUCT, * LWLOG_ENTRY_STRUCT_PTR;

/*--------------------------------------------------------------------------*/
/*                           EXTERNAL DECLARATIONS                          */

#ifdef __cplusplus
extern "C" {
#endif

extern _mem_size _lwlog_calculate_size(_mqx_uint);
#if !MQX_LITE_VERSION_NUMBER
extern _mqx_uint _lwlog_create(_mqx_uint, _mqx_uint, _mqx_uint);
#endif /* MQX_LITE_VERSION_NUMBER */
extern _mqx_uint _lwlog_create_at(_mqx_uint, _mqx_uint, _mqx_uint, void *);
extern _mqx_uint _lwlog_create_component(void);
extern _mqx_uint _lwlog_destroy(_mqx_uint);
extern _mqx_uint _lwlog_disable(_mqx_uint);
extern _mqx_uint _lwlog_enable(_mqx_uint);
extern _mqx_uint _lwlog_read(_mqx_uint, _mqx_uint, LWLOG_ENTRY_STRUCT_PTR);
extern _mqx_uint _lwlog_reset(_mqx_uint);
extern _mqx_uint _lwlog_test(_mqx_uint *);
extern _mqx_uint _lwlog_write(_mqx_uint, _mqx_max_type, _mqx_max_type, _mqx_max_type, _mqx_max_type,
   _mqx_max_type, _mqx_max_type, _mqx_max_type);

#ifdef __cplusplus
}
#endif

#endif /* __lwlog_h__ */
/* EOF */
