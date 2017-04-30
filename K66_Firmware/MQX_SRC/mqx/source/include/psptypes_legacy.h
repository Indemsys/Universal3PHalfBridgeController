/*HEADER**********************************************************************
*
* Copyright 2013 Freescale Semiconductor, Inc.
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
*   This file contains the definitions of old MQX types that have been used
*   before the MQX 4.1.0 release. This file has been created for the backward
*   compatibility and is intended to be included into old MQX application source 
*   files (created in MQX releases up to the version 4.1.0) to avoid types 
*   conflicts in MQX 4.0.1 and above releases. Include this file into the main
*   application header file.
*
*
*END************************************************************************/
#ifndef __psptypes_legacy_h__
#define __psptypes_legacy_h__

#define _PTR_      *

typedef char _PTR_                    char_ptr;    /* signed character       */
typedef unsigned char  uchar, _PTR_   uchar_ptr;   /* unsigned character     */
typedef volatile char _PTR_                    vchar_ptr;    /* signed character       */
typedef volatile unsigned char  vuchar, _PTR_   vuchar_ptr;   /* unsigned character     */

typedef signed   char   int_8, _PTR_   int_8_ptr;   /* 8-bit signed integer   */
typedef unsigned char  uint_8, _PTR_  uint_8_ptr;  /* 8-bit signed integer   */
typedef volatile signed   char   vint_8, _PTR_   vint_8_ptr;   /* 8-bit volatile signed integer   */
typedef volatile unsigned char  vuint_8, _PTR_  vuint_8_ptr;  /* 8-bit volatile signed integer   */

typedef          short int_16, _PTR_  int_16_ptr;  /* 16-bit signed integer  */
typedef unsigned short uint_16, _PTR_ uint_16_ptr; /* 16-bit unsigned integer*/
typedef volatile          short vint_16, _PTR_  vint_16_ptr;  /* 16-bit volatile signed integer  */
typedef volatile unsigned short vuint_16, _PTR_ vuint_16_ptr; /* 16-bit volatile unsigned integer*/

typedef          long  int_32, _PTR_  int_32_ptr;  /* 32-bit signed integer  */
typedef unsigned long  uint_32, _PTR_ uint_32_ptr; /* 32-bit unsigned integer*/

typedef volatile          long  vint_32, _PTR_  vint_32_ptr;  /* 32-bit signed integer  */
typedef volatile unsigned long  vuint_32, _PTR_ vuint_32_ptr; /* 32-bit unsigned integer*/

typedef    long  long  int_64, _PTR_  int_64_ptr;       /* 64-bit signed   */
typedef unsigned long long  uint_64, _PTR_ uint_64_ptr; /* 64-bit unsigned */
typedef volatile   long  long  vint_64, _PTR_  vint_64_ptr;       /* 64-bit signed   */
typedef volatile unsigned long long  vuint_64, _PTR_ vuint_64_ptr; /* 64-bit unsigned */

typedef void _PTR_     pointer;  /* Machine representation of a pointer */

#define boolean     bool
#define  uint_8  uint8_t
#define   int_8   int8_t
#define uint_16 uint16_t
#define  int_16  int16_t
#define uint_32 uint32_t
#define  int_32  int32_t
#define uint_64 uint64_t
#define  int_64  int64_t

#endif /* __psptypes_legacy_h__ */
/* EOF */
