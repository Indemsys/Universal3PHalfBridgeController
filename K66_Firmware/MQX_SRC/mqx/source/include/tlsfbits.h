/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
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
*   This file contains functions of the Two Level Segregate Fit, 
*   based on public domain licensed implementation from http://tlsf.baisoku.org/ .
*
*
*END************************************************************************/

#ifndef INCLUDED_tlsfbits
#define INCLUDED_tlsfbits

#include <mqx_cpudef.h>

#if !defined(__cplusplus)
#define tlsf_decl inline static
#else
#define tlsf_decl static
#endif

/*
** Architecture-specific bit manipulation routines.
**
** TLSF achieves O(1) cost for malloc and free operations by limiting
** the search for a free block to a free list of guaranteed size
** adequate to fulfill the request, combined with efficient free list
** queries using bitmasks and architecture-specific bit-manipulation
** routines.
**
** Most modern processors provide instructions to count leading zeroes
** in a word, find the lowest and highest set bit, etc. These
** specific implementations will be used when available, falling back
** to a reasonably efficient generic implementation.
**
** NOTE: TLSF spec relies on ffs/fls returning value 0..31.
** ffs/fls return 1-32 by default, returning 0 for error.
*/

#if (PSP_MQX_CPU_IS_ARM_CORTEX_M4 || PSP_MQX_CPU_IS_ARM_CORTEX_A5)
/* IAR ARM compiler*/
extern unsigned int tlsf_clz(unsigned int x);

#define tlsf_ffs(_word_) (31 - tlsf_clz((_word_) & (~(_word_) + 1)))

/* CLZ returns 32 if input is 0 */ 
#define tlsf_fls(_word_) (31 - tlsf_clz(_word_)) 

#else /* PSP_MQX_CPU_IS_ARM_CORTEX_M0P and others */
/* Fall back to generic implementation. */

tlsf_decl int tlsf_fls_generic(unsigned int word)
{
	int bit = 32;

	if (!word) bit -= 1;
	if (!(word & 0xffff0000)) { word <<= 16; bit -= 16; }
	if (!(word & 0xff000000)) { word <<= 8; bit -= 8; }
	if (!(word & 0xf0000000)) { word <<= 4; bit -= 4; }
	if (!(word & 0xc0000000)) { word <<= 2; bit -= 2; }
	if (!(word & 0x80000000)) { word <<= 1; bit -= 1; }

	return bit;
}

/* Implement ffs in terms of fls. */
tlsf_decl int tlsf_ffs(unsigned int word)
{
	return tlsf_fls_generic(word & (~word + 1)) - 1;
}

tlsf_decl int tlsf_fls(unsigned int word)
{
	return tlsf_fls_generic(word) - 1;
}

#endif

#define tlsf_fls_sizet tlsf_fls
#undef tlsf_decl

#endif
