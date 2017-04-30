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
* See license agreement file for full license terms including other
* restrictions.
*****************************************************************************
*
* Comments:
*
*   This file contains the "C" implementation of
*   PPP_MD5_task_block.
*
*
*END************************************************************************/
#include <rtcs.h>

#if RTCSCFG_ENABLE_PPP && !PLATFORM_SDK_ENABLED

#include <stdarg.h>
#include <ppp.h>
#include "ppp_prv.h"

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPP_MD5_block
* Returned Value  :  void
* Comments        :
*     Computes an incremental MD5 hash of a 16 word block.
*
*END*-----------------------------------------------------------------*/

#if defined(__DCC__) && defined(__ppc)
asm uint32_t rotl (uint32_t num, uint32_t bits)
{
% reg num; reg bits
   rotlw r3, num, bits
}
#else
#define rotl(num, bits) ((num << bits) | (num >> (32 - bits)))
#endif

void PPP_MD5_block
   (
      uint32_t *context,
            /* [IN/OUT] - the incremental hash */

      register uint32_t *block,
            /* [IN] - the 16 word block */

      register const uint32_t  *ctab
            /* [IN] - the sine function */
   )
{ /* Body */

#define F(x,y,z)  ((y&x)|(z&~x))
#define G(x,y,z)  ((x&z)|(y&~z))
#define H(x,y,z)  (x^y^z)
#define I(x,y,z)  (y^(x|~z))

#define P1(f,k,s) t=a+f(b,c,d)+block[k]+*ctab++; a=rotl(t,s)+b
#define P2(f,k,s) t=d+f(a,b,c)+block[k]+*ctab++; d=rotl(t,s)+a
#define P3(f,k,s) t=c+f(d,a,b)+block[k]+*ctab++; c=rotl(t,s)+d
#define P4(f,k,s) t=b+f(c,d,a)+block[k]+*ctab++; b=rotl(t,s)+c

   register uint32_t  a = context[0];
   register uint32_t  b = context[1];
   register uint32_t  c = context[2];
   register uint32_t  d = context[3];
   register uint32_t  t;

   P1(F,  0,  7); P2(F,  1, 12); P3(F,  2, 17); P4(F,  3, 22);
   P1(F,  4,  7); P2(F,  5, 12); P3(F,  6, 17); P4(F,  7, 22);
   P1(F,  8,  7); P2(F,  9, 12); P3(F, 10, 17); P4(F, 11, 22);
   P1(F, 12,  7); P2(F, 13, 12); P3(F, 14, 17); P4(F, 15, 22);

   P1(G,  1,  5); P2(G,  6,  9); P3(G, 11, 14); P4(G,  0, 20);
   P1(G,  5,  5); P2(G, 10,  9); P3(G, 15, 14); P4(G,  4, 20);
   P1(G,  9,  5); P2(G, 14,  9); P3(G,  3, 14); P4(G,  8, 20);
   P1(G, 13,  5); P2(G,  2,  9); P3(G,  7, 14); P4(G, 12, 20);

   P1(H,  5,  4); P2(H,  8, 11); P3(H, 11, 16); P4(H, 14, 23);
   P1(H,  1,  4); P2(H,  4, 11); P3(H,  7, 16); P4(H, 10, 23);
   P1(H, 13,  4); P2(H,  0, 11); P3(H,  3, 16); P4(H,  6, 23);
   P1(H,  9,  4); P2(H, 12, 11); P3(H, 15, 16); P4(H,  2, 23);

   P1(I,  0,  6); P2(I,  7, 10); P3(I, 14, 15); P4(I,  5, 21);
   P1(I, 12,  6); P2(I,  3, 10); P3(I, 10, 15); P4(I,  1, 21);
   P1(I,  8,  6); P2(I, 15, 10); P3(I,  6, 15); P4(I, 13, 21);
   P1(I,  4,  6); P2(I, 11, 10); P3(I,  2, 15); P4(I,  9, 21);

   context[0] += a;
   context[1] += b;
   context[2] += c;
   context[3] += d;

   PPP_memzero(block, sizeof(uint32_t[16]));

} /* Endbody */

#endif // RTCSCFG_ENABLE_PPP
