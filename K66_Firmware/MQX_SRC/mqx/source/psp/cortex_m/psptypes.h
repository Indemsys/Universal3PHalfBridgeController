/*HEADER**********************************************************************
*
* Copyright 2010-2011 Freescale Semiconductor, Inc.
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
* See license agreement file for full license terms including other
* restrictions.
*****************************************************************************
*
* Comments:
*   This file contains the definitions of the basic MQX types.
*
*
*END************************************************************************/
#ifndef __psptypes_h__
#define __psptypes_h__

#include <stdint.h>
#include <stdbool.h>

/*--------------------------------------------------------------------------*/
/*
**                            STANDARD TYPES
*/

/*
**  The following typedefs allow us to minimize portability problems
**  due to the various C compilers (even for the same processor) not
**  agreeing on the sizes of "int"s and "short int"s and "longs".
*/

#define _CODE_PTR_ *

/* IEEE single precision floating point number (32 bits, 8 exponent bits) */
typedef float          ieee_single;

/* IEEE double precision floating point number (64 bits, 11 exponent bits) */
typedef double         ieee_double;

#ifndef USE_32BIT_TYPES
/* Type for the CPU's natural size */
typedef uint32_t  _mqx_uint, * _mqx_uint_ptr;
typedef int32_t   _mqx_int, * _mqx_int_ptr;

/* How big a data pointer is on this processor */
typedef uint32_t  _psp_data_addr, * _psp_data_addr_ptr;

/* How big a code pointer is on this processor */
typedef uint32_t  _psp_code_addr, * _psp_code_addr_ptr;

/* Maximum type */
typedef uint32_t  _mqx_max_type, * _mqx_max_type_ptr;

/* _mem_size is equated to the a type that can hold the maximum data address */
typedef uint32_t _mem_size, * _mem_size_ptr;

/* Used for file sizes. */
typedef uint64_t       _file_size;
typedef int64_t        _file_offset;

#else
#define _mqx_uint      uint32_t
#define _mqx_uint_ptr  uint32_t *
#define _mqx_int       int32_t
#define _mqx_int_ptr   int32_t *
#define _psp_data_addr uint32_t
#define _psp_code_addr uint32_t
#define _mqx_max_type  uint32_t
#define _mem_size      uint32_t
#endif

/*--------------------------------------------------------------------------*/
/*
**                         DATATYPE VALUE RANGES
*/

#define MAX_CHAR      (0x7F)
#define MAX_UCHAR     (0xFF)
#define MAX_INT_8     (0x7F)
#define MAX_UINT_8    (0xFF)
#define MAX_INT_16    (0x7FFF)
#define MAX_UINT_16   (0xFFFF)
#define MAX_INT_32    (0x7FFFFFFFL)
#define MAX_UINT_32   (0xFFFFFFFFUL)
#define MAX_INT_64    (0x7FFFFFFFFFFFFFFFLL)
#define MAX_UINT_64   (0xFFFFFFFFFFFFFFFFULL)

#define MIN_FLOAT     (8.43E-37)
#define MAX_FLOAT     (3.37E+38)

#define MIN_DOUBLE    (2.225074E-308)
#define MAX_DOUBLE    (1.797693E+308)

#define MAX_MQX_UINT         (MAX_UINT_32)
#define MAX_MQX_INT          (MAX_INT_32)
#define MAX_FILE_SIZE        (MAX_UINT_32)
#define MAX_MEM_SIZE         (MAX_UINT_32)
#define MAX_MQX_MAX_TYPE     (MAX_UINT_32)
#define MQX_INT_SIZE_IN_BITS (32)


#endif /* __psptypes_h__ */
/* EOF */
