/*
#*HEADER********************************************************************
#* 
#* Copyright 2008 Freescale Semiconductor, Inc.
#*
#* This software is owned or controlled by Freescale Semiconductor.
#* Use of this software is governed by the Freescale MQX RTOS License
#* distributed with this Material.
#* See the MQX_RTOS_LICENSE file distributed for more details.
#*
#* Brief License Summary:
#* This software is provided in source form for you to use free of charge,
#* but it is not open source software. You are allowed to use this software
#* but you cannot redistribute it or derivative works of it in source form.
#* The software may be used only in connection with a product containing
#* a Freescale microprocessor, microcontroller, or digital signal processor.
#* See license agreement file for full license terms including other restrictions.
#**************************************************************************
#*
#* Comments:
#*
#*   This assembler file contains functions for semihost.
#*   Thumb2 only
#*END***********************************************************************
*/

#include <asm_mac.h>

// thumb2 semihosting constant

ASM_EQUATE(SEMIHOSTING_SWI, 0xab)
ASM_EQUATE(SYS_OPEN,        0x01)
ASM_EQUATE(SYS_CLOSE,       0x02)
ASM_EQUATE(SYS_WRITEC,      0x03)
ASM_EQUATE(SYS_WRITE0,      0x04)
ASM_EQUATE(SYS_WRITE,       0x05)
ASM_EQUATE(SYS_READ,        0x06)
ASM_EQUATE(SYS_READC,       0x07)
ASM_EQUATE(SYS_ISERROR,     0x08)
ASM_EQUATE(SYS_ISTTY,       0x09)
ASM_EQUATE(SYS_SEEK,        0x0a)
ASM_EQUATE(SYS_FLEN,        0x0c)
ASM_EQUATE(SYS_TMPNAM,      0x0d)
ASM_EQUATE(SYS_REMOVE,      0x0e)
ASM_EQUATE(SYS_RENAME,      0x0f)
ASM_EQUATE(SYS_CLOCK,       0x10)
ASM_EQUATE(SYS_TIME,        0x11)
ASM_EQUATE(SYS_SYSTEM,      0x12)
ASM_EQUATE(SYS_ERRNO,       0x13)
ASM_EQUATE(SYS_GET_CMDLINE, 0x15)
ASM_EQUATE(SYS_HEAPINFO,    0x16)
ASM_EQUATE(SYS_ELAPSED,     0x30)
ASM_EQUATE(SYS_TICKFREQ,    0x31)
ASM_EQUATE(SYS_EXIT,        0x18)

// .thumb2 version only
 ASM_COMP_SPECIFIC_DIRECTIVES
 ASM_CODE_SECTION(.text)

 ASM_PUBLIC(_io_debug_semi_write_char)
 ASM_PUBLIC(_io_debug_semi_write_string)
 ASM_PUBLIC(_io_debug_semi_read_char)


//FUNCTION*----------------------------------------------------------------------
// 
// Function Name    : _io_debug_semi_write_char
// Returned Value   : none
// Comments         : write single character
//
//END*---------------------------------------------------------------------------

 ASM_PUBLIC_BEGIN(ASM_PREFIX(_io_debug_semi_write_char))
 ASM_PUBLIC_FUNC(ASM_PREFIX(_io_debug_semi_write_char))
ASM_LABEL(ASM_PREFIX(_io_debug_semi_write_char))
    push {lr}
    push {r1}
    mov r1, r0
    mov r0, #SYS_WRITEC
    bkpt SEMIHOSTING_SWI
    pop {r1}
    pop {pc}
 ASM_PUBLIC_END(ASM_PREFIX(_io_debug_semi_write_char))

//FUNCTION*----------------------------------------------------------------------
// 
// Function Name    : _io_debug_semi_write_string
// Returned Value   : none
// Comments         : write '\0' terminated data
//
//END*---------------------------------------------------------------------------

 ASM_PUBLIC_BEGIN(ASM_PREFIX(_io_debug_semi_write_string))
 ASM_PUBLIC_FUNC(ASM_PREFIX(_io_debug_semi_write_string))
ASM_LABEL(ASM_PREFIX(_io_debug_semi_write_string))
    push {lr}
    push {r1}
    mov r1, r0
    mov r0, #SYS_WRITE0
    bkpt SEMIHOSTING_SWI
    pop {r1}
    pop {pc}
 ASM_PUBLIC_END(ASM_PREFIX(_io_debug_semi_write_string))


//FUNCTION*----------------------------------------------------------------------
// 
// Function Name    : _io_debug_semi_read_char
// Returned Value   : char
// Comments         : read single character
//
//END*---------------------------------------------------------------------------

 ASM_PUBLIC_BEGIN(ASM_PREFIX(_io_debug_semi_read_char))
 ASM_PUBLIC_FUNC(ASM_PREFIX(_io_debug_semi_read_char))
ASM_LABEL(ASM_PREFIX(_io_debug_semi_read_char))
    push {lr}
//    push {r1}
    mov r1,#0
    mov r0,#SYS_READC
    bkpt SEMIHOSTING_SWI
//    pop {r1}
    pop {pc}
 ASM_PUBLIC_END(ASM_PREFIX(_io_debug_semi_read_char))

 ASM_END
