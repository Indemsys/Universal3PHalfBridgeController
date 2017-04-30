
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

#ifndef __log_h__
#define __log_h__ 1

#include <mqx_cnfg.h>

#if (!MQX_USE_LOGS) && (!defined __lwlog_h__) && (! defined (MQX_DISABLE_CONFIG_CHECK))
#error LOG component is currently disabled in MQX kernel. Please set MQX_USE_LOGS to 1 in user_config.h and recompile kernel.
#endif

/*--------------------------------------------------------------------------*/
/*                        CONSTANT DEFINITIONS                              */


/* Maximum number of logs allowed */
#define LOG_MAXIMUM_NUMBER (16)

/*
 * The kernel system logging uses log number 0
 */
#define LOG_KERNEL_LOG_NUMBER      (0)

/* Configuration flags */
#define LOG_OVERWRITE              (1)

/* Types of log reads */
#define LOG_READ_NEWEST            (1)
#define LOG_READ_OLDEST            (2)
#define LOG_READ_NEXT              (3)
#define LOG_READ_OLDEST_AND_DELETE (4)

/* Error codes */
#define LOG_INVALID                (LOG_ERROR_BASE|0x00)
#define LOG_EXISTS                 (LOG_ERROR_BASE|0x01)
#define LOG_DOES_NOT_EXIST         (LOG_ERROR_BASE|0x02)
#define LOG_FULL                   (LOG_ERROR_BASE|0x03)
#define LOG_ENTRY_NOT_AVAILABLE    (LOG_ERROR_BASE|0x04)
#define LOG_DISABLED               (LOG_ERROR_BASE|0x05)
#define LOG_INVALID_READ_TYPE      (LOG_ERROR_BASE|0x06)
#define LOG_INVALID_SIZE           (LOG_ERROR_BASE|0x07)

/*--------------------------------------------------------------------------*/
/*                        DATATYPE DECLARATIONS                             */

/*                        LOG_ENTRY_STRUCT                                  */
/*!
 * \brief Header of an entry in a user log.
 *
 * The length of the entry depends on the SIZE field.
 *
 * \see _log_read
 * \see _log_write
 */

typedef struct log_entry_struct
{

   /*! \brief The size of this entry in _mqx_uints. */
   _mqx_uint    SIZE;

   /*! \brief The sequence number for the entry. */
   _mqx_uint    SEQUENCE_NUMBER;

   /*! \brief The time (in seconds) at which MQX wrote the entry. */
   uint32_t      SECONDS;

   /*! \brief The time (in milliseconds) at which MQX wrote the entry. */
   uint16_t      MILLISECONDS;

   /*! \brief The time (in microseconds) at which MQX wrote the entry. */
   uint16_t      MICROSECONDS;

} LOG_ENTRY_STRUCT, * LOG_ENTRY_STRUCT_PTR;

/*--------------------------------------------------------------------------*/
/*                           EXTERNAL DECLARATIONS                          */

#ifdef __cplusplus
extern "C" {
#endif

extern _mqx_uint _log_create(_mqx_uint, _mqx_uint, uint32_t);
extern _mqx_uint _log_create_component(void);
extern _mqx_uint _log_destroy(_mqx_uint);
extern _mqx_uint _log_disable(_mqx_uint);
extern _mqx_uint _log_enable(_mqx_uint);
extern _mqx_uint _log_read(_mqx_uint, _mqx_uint, _mqx_uint,
   LOG_ENTRY_STRUCT_PTR);
extern _mqx_uint _log_reset(_mqx_uint);
extern _mqx_uint _log_test(_mqx_uint *);
extern _mqx_uint _log_write(_mqx_uint, _mqx_uint, ... );

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
