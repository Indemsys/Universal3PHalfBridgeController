/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
* Copyright 2008 Embedded Access Inc.
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
*   This file contains macros used by the IAR ARM assembler
*
*
*END************************************************************************/

#ifndef __asm_mac_h__
#define __asm_mac_h__   1

#define ASM_PREFIX(x) x

#define ASM_LABEL(value)      value
#define ASM_EXTERN(value)     EXTERN  value
#define ASM_ALIGN(value)      ALIGNROM  value
#define ASM_PUBLIC(label)     PUBLIC label
#define ASM_SET(label, value) ASM_LABEL(label) SET value
#define ASM_CONST16(value)    DC16 value
#define ASM_CONST32(value)    DC32 value
#define ASM_LABEL_CONST32(label,value) ASM_LABEL(label) ASM_CONST32(value)
#define ASM_DATA_SECTION(label)    SECTION label : DATA (4)
#define ASM_CODE_SECTION(label)    SECTION label : CODE (4)
#define ASM_CODE_SECTION_NOOPT(label)    SECTION label : CODE (4) : ROOT  
#define ASM_END               END
#define ASM_COMP_SPECIFIC_DIRECTIVES
#define ASM_EQUATE(label, value) label   EQU  value
#define ASM_CODE        CODE
#define ASM_DATA        DATA

/* CFI annotations for public symbols called from C code */

  CFI Names CFINames0
  CFI StackFrame CFA R13 DATA
  CFI Resource R0:32, R1:32, R2:32, R3:32, R4:32, R5:32, R6:32, R7:32
  CFI Resource R8:32, R9:32, R10:32, R11:32, R12:32, R13:32, R14:32
  CFI EndNames         CFINames0

  CFI Common CFICommon0 Using         CFINames0
  CFI CodeAlign 2
  CFI DataAlign 4
  CFI ReturnAddress R14 CODE
  CFI CFA R13+0
  CFI R0 SameValue
  CFI R1 SameValue
  CFI R2 SameValue
  CFI R3 SameValue
  CFI R4 SameValue
  CFI R5 SameValue
  CFI R6 SameValue
  CFI R7 SameValue
  CFI R8 SameValue
  CFI R9 SameValue
  CFI R10 SameValue
  CFI R11 SameValue
  CFI R12 SameValue
  CFI R14 SameValue
  CFI EndCommon CFICommon0

/* Note that these macros should NOT be on the beggining of line when used
   in assembler code. Prepend allways by at least one space. 
   (was not an issue in EWARM 6.40.x, space seems to be needed in 6.50.x) */
#define ASM_PUBLIC_BEGIN(name) CFI Block CFIBlock_##name Using CFICommon0
#define ASM_PUBLIC_FUNC(name)  CFI Function name
#define ASM_PUBLIC_END(name)   CFI EndBlock CFIBlock_##name

#endif /* __asm_mac_h__ */
