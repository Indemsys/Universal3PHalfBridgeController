/*HEADER**********************************************************************
*
* Copyright 2010 Freescale Semiconductor, Inc.
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
*   This file contains the definitions for functions that provide
*   mathematics for working on 64 bit, 96 bit and 128 bit numbers.
*   (Needed by the time conversion functions) The operations on the numbers 
*   are performed uing the 16-bit array representation of the numbers.
*
*
*END************************************************************************/
   
#ifndef __psp_math_h__
#define __psp_math_h__

/*-----------------------------------------------------------------------*/
/*
** PSP 64 BIT UNION
**
** The representation of a 64 bit number
**
*/
typedef union psp_64_bit_union
{
   uint64_t  LLW;
   uint32_t  LW[2];
   uint16_t  W[4];
   uint8_t   B[8];
} PSP_64_BIT_UNION, * PSP_64_BIT_UNION_PTR;

/*-----------------------------------------------------------------------*/
/*
** PSP 96 BIT UNION
**
** The representation of a 96 bit number
**
*/
typedef union psp_96_bit_union
{
   uint32_t         LW[3];
   uint16_t         W[6];
   uint8_t          B[12];
   PSP_TICK_STRUCT TICKS;
} PSP_96_BIT_UNION, * PSP_96_BIT_UNION_PTR;

/*-----------------------------------------------------------------------*/
/*
** PSP 128 BIT UNION
**
** The representation of a 128 bit number
**
*/
typedef union psp_128_bit_union
{
   uint64_t  LLW[2];
   uint32_t  LW[4];
   uint16_t  W[8];
   uint8_t   B[16];
} PSP_128_BIT_UNION, * PSP_128_BIT_UNION_PTR;


/*--------------------------------------------------------------------------*/
/*
**                  PROTOTYPES OF PSP FUNCTIONS
*/

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t _psp_add_element_to_array(uint32_t *, uint32_t, uint32_t, uint32_t *);
extern uint32_t _psp_div_128_by_32(PSP_128_BIT_UNION_PTR, uint32_t, PSP_128_BIT_UNION_PTR);
extern uint32_t _psp_mul_128_by_32(PSP_128_BIT_UNION_PTR, uint32_t, PSP_128_BIT_UNION_PTR);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
