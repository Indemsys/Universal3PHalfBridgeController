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
*   This file contains the implementation of the MD5
*   message digest algorithm described in RFC 1321.
*
*
*END************************************************************************/
#include <rtcs.h>

#if RTCSCFG_ENABLE_PPP && !PLATFORM_SDK_ENABLED

#include <stdarg.h>
#include <ppp.h>
#include "ppp_prv.h"
#include "md5.h"



/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPP_MD5
* Returned Value  :  void
* Comments        :
*     Computes the MD5 hash of a sequence of bytes.
*
*     The prototype is:
*        PPP_MD5(digest, len1, msg1, len2, msg2, ..., lenk, msgk, 0);
*
* Limitations     :
*     This function returns an incorrect hash if the complete byte
*     sequence exceeds 512MB in length.
*
*END*-----------------------------------------------------------------*/

void PPP_MD5
   (
      unsigned char *digest,
            /* [OUT] - the MD5 message digest */
      ...
            /* [IN] - (uint32_t, unsigned char *) pairs */
   )
{ /* Body */
   va_list   ap;
   uint32_t   block[16];
   uint32_t   context[4];
   uint32_t   temp;
   uint32_t   bcount = 0, wcount = 0;
   uint32_t   lenmsg = 0;
   uint32_t   lenfrag;
   unsigned char *msgfrag;

   va_start(ap, digest);

   context[0] = 0x67452301l;
   context[1] = 0xEFCDAB89l;
   context[2] = 0x98BADCFEl;
   context[3] = 0x10325476l;

   PPP_memzero(block, sizeof(block));

      /* Loop for each message fragment */
   for (;;) {

         /* Get the next message fragment */
      lenfrag = va_arg(ap, uint32_t);
      if (lenfrag == 0) break;
      lenmsg += lenfrag;
      msgfrag = va_arg(ap, unsigned char *);

         /* Copy bytes from the fragment to the block */
      while (lenfrag--) {
         temp = *msgfrag++ & 0xFF;
         block[wcount] >>= 8;
         block[wcount] |= temp << 24;
         if (++bcount == 4) {
            bcount = 0;
            if (++wcount == 16) {
               wcount = 0;
               PPP_MD5_block(context, block, mdtab);
            } /* Endif */
         } /* Endif */
      } /* Endwhile */

   } /* Endfor */

      /* Now append 0x80 */
   block[wcount] >>= 8;
   block[wcount] |= 0x80000000l;
   if (++bcount == 4) {
      bcount = 0;
      if (++wcount == 16) {
         wcount = 0;
         PPP_MD5_block(context, block, mdtab);
      } /* Endif */
   } /* Endif */

      /* Pad with 0 until wcount==14 */
   if (bcount > 0) {
      block[wcount++] >>= 8*(4-bcount);
      /* bcount henceforth ignored */
   } /* Endif */
   if (wcount > 14) {
      while (wcount < 16) block[wcount++] = 0;
      PPP_MD5_block(context, block, mdtab);
      wcount = 14;   /* PPP_MD5_task_block zeroes block */
   } /* Endif */
   while (wcount < 14) block[wcount++] = 0;

      /* Finally, append the pre-pad length */
   lenmsg <<= 3;     /* length must be in bits */
   block[14] = lenmsg;
   block[15] = 0;
   PPP_MD5_block(context, block, mdtab);

      /* Write the hash out to digest while destroying context */
   for (wcount = 0; wcount < 4; wcount++) {
      for (bcount = 0; bcount < 4; bcount++) {
         *digest++ = context[wcount] & 0xFF;
         context[wcount] >>= 8;
      } /* Endfor */
   } /* Endfor */

   va_end(ap);

} /* Endbody */

#endif // RTCSCFG_ENABLE_PPP
