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
*   This file generates the lookup table for the CCITT-16
*   FCS.
*
*
*END************************************************************************/
#include <rtcs.h>

#if RTCSCFG_ENABLE_PPP && !PLATFORM_SDK_ENABLED

#if MQX_USE_IO_OLD
#include <fio.h>
#else
#include <stdio.h>
#endif
#define SIZE 256
#define POLY 0x8408

#if RCTSCFG_GENERATE_HASH_TABLES && RTCSCFG_ENABLE_IP4
/* Start CR 1679 */
int RTCS_PPP_SUPPORT_gen16 (void)
/* End CR 1679 */
{ /* Body */
   int count = 0, i;
   register int carry;
   unsigned short crc;

   printf("/*\n** The CCITT-16 Lookup Table\n*/\n\n"
          "static uint16_t fcstab[] = {");

   for (;;) {
      if (!(count%8)) printf("\n  ");
      crc = count;
      for (i=8;i--;) {
         carry = crc & 1;
         crc >>= 1;
         if (carry) crc ^= POLY;
      } /* Endfor */
      printf(" 0x%04X", crc);
      if (++count == SIZE) break;
      printf(",");
   } /* Endfor */
   printf("\n};\n");

   return 0;
} /* Endbody */
#endif

#endif // RTCSCFG_ENABLE_PPP
