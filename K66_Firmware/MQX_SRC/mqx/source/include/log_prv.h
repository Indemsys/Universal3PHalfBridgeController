
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

#ifndef __log_prv_h__
#define __log_prv_h__ 1


/*--------------------------------------------------------------------------*/
/*                        CONSTANT DEFINITIONS                              */

#define LOG_VALID           ((_mqx_uint)(0x6c6f6720))   /* "log " */

/* Control bits in the control flags */
#define LOG_ENABLED         (0x1000)


/*--------------------------------------------------------------------------*/
/*                      DATA STRUCTURE DEFINITIONS                          */

/*  LOG HEADER STRUCT */
/*!
 * \cond DOXYGEN_PRIVATE
 *  
 * \brief This structure is stored at the front of each log to provide information
 * about the current state of the log.
 */
typedef struct log_header_struct
{
    /*! \brief Control flags for the log. */
    uint32_t       FLAGS;

    /*! \brief The sequence number for next write. */
    _mqx_uint     NUMBER;

    /*! \brief Where next log is to be written. */
    _mqx_uint_ptr LOG_WRITE;

    /*! \brief Where first log is to be read. */
    _mqx_uint_ptr LOG_READ;

    /*! \brief Where last log was written. */
    _mqx_uint_ptr LAST_LOG;

    /*! \brief Starting address of data. */
    _mqx_uint_ptr LOG_START;

    /*! \brief Ending address of data. */
    _mqx_uint_ptr LOG_END;

    /*! \brief The next log to read. */
    _mqx_uint_ptr LOG_NEXT;

    /*! \brief Current size of data. */
    _mqx_uint     SIZE;

    /*! \brief Maximum size of data. */
    _mqx_uint     MAX;

    /*! \brief Variable array of data. */
    _mqx_uint     DATA[1];

} LOG_HEADER_STRUCT, * LOG_HEADER_STRUCT_PTR;
/*! \endcond */


/*  LOG COMPONENT STRUCT */
/*!
 * \cond DOXYGEN_PRIVATE
 * 
 * \brief  This structure is used to store information required for log retrieval.
 * 
 * Its address is stored in the kernel component field of the kernel data structure.
 */
typedef struct log_component_struct
{
    /*! \brief A validation stamp to verify handle correctness. */
    _mqx_uint             VALID;

    /*! \brief The address of the log headers. */
    LOG_HEADER_STRUCT_PTR LOGS[LOG_MAXIMUM_NUMBER];

} LOG_COMPONENT_STRUCT, * LOG_COMPONENT_STRUCT_PTR;
/*! \endcond */

#endif
/* EOF */
