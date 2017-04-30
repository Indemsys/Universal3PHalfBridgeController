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
*   This file generates the lookup table for the MD5 hash.
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
#include <math.h>
#define SIZE 64

#if RCTSCFG_GENERATE_HASH_TABLES && RTCSCFG_ENABLE_IP4
/* Start CR 1679 */
int RTCS_PPP_SUPPORT_genmd5 (void)
/* End CR 1679 */
{ /* Body */
   int count = 0;

   printf("/*\n** The MD5 Lookup Table\n*/\n\n"
          "static uint32_t mdtab[] = {");

   for (;;) {
      if (!(count%4)) printf("\n  ");
      printf(" 0x%08lXl", (long)ldexp(fabs(sin(++count)),32));
      if (count == SIZE) break;
      printf(",");
   } /* Endfor */
   printf("\n};\n");

   return 0;
} /* Endbody */
#endif

#endif // RTCSCFG_ENABLE_PPP
