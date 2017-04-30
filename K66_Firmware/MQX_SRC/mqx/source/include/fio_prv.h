
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
*   This file includes the private definitions for the formatted I/O .
*
*
*END************************************************************************/
#ifndef __fio_prv_h__
#define __fio_prv_h__

/*--------------------------------------------------------------------------*/
/*
 *  Compiler Dependencies
 *
 *  Most compilers have adequate modf and strtod functions
 */
#ifdef NEED_MODF
#define  modf       _io_modf
#endif
#ifdef NEED_STRTOD
#define  strtod     _io_strtod
#endif

/*--------------------------------------------------------------------------*/
/*
 *                            CONSTANT DEFINITIONS
 */

/* 
 * Type definitions also used for sizing by doprint 
 * They are the maximum string size that a 32/64 bit number 
 * can be displayed as. 
 */
#define _MQX_IO_DIVISION_ADJUST_64 1000000000000000000LL
#define PRINT_OCTAL_64   (22L)
#define PRINT_DECIMAL_64 (20L)
#define PRINT_HEX_64     (16L)

#define _MQX_IO_DIVISION_ADJUST 1000000000L
#define PRINT_OCTAL   (11L)
#define PRINT_DECIMAL (10L)
#define PRINT_HEX     (8L)

#define PRINT_ADDRESS (8L)

/* Type definitions use in the control of scanline */

#define SCAN_ERROR    (-1)
#define SCAN_LLONG    (0)
#define SCAN_WLONG    (1)
#define SCAN_BLONG    (2)
#define SCAN_MLONG    (3)


/*--------------------------------------------------------------------------*/
/*
 *                            FUNCTION PROTOTYPES
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TAD_COMPILE__
extern _mqx_int _io_doprint(MQX_FILE_PTR, IO_PUTCHAR_FPTR, _mqx_int max_count, char *, va_list);
extern _mqx_int _io_sputc(_mqx_int, MQX_FILE_PTR);
extern _mqx_int _io_scanline(char *, char *, va_list);
#endif

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
