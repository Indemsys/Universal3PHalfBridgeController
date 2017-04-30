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
*   Example using RTCS Library.
*
*
*END************************************************************************/

#include <ctype.h>
#include <string.h>
#include <mqx.h>
#include "shell.h"
#if SHELLCFG_USES_RTCS
#include <rtcs.h>
#include "sh_rtcs.h" 


bool Shell_parse_ip_address( char *arg, _ip_address  *ipaddr_ptr)
{
   uint32_t      num[4] =  { 0 };
   uint32_t      i, index = 0;
   uint32_t      temp = 0;
   
   if (ipaddr_ptr == NULL) return FALSE;
   
   for (i=0; arg[i] && (i<16) && (index<4); i++)  {
      if (isdigit((int) arg[i]))  {
         num[index] = num[index]*10 + arg[i]-'0';
      } else if (arg[i] == '.') {
         index++; 
      } else  {
         return FALSE;
      }
   }
   
   if ((arg[i] == '\0') && (index==3))  {
      for (i=0;i<4;i++)  {
         if (num[i] > 255)  {
            return FALSE;
         } else  {
            temp = (temp << 8) | num[i];
         }
      } 
   } 

   *ipaddr_ptr = temp;
   return TRUE;
}

bool Shell_parse_netmask( char *arg, _ip_address  *ipaddr_ptr)
{
   uint32_t  i, mask;
   bool  ones = FALSE;
   
   if ( ipaddr_ptr == NULL) return FALSE;
   
   if (!Shell_parse_ip_address(arg, ipaddr_ptr))  return FALSE;

   
   for (i=0;i<32;i++)  {
      mask = *ipaddr_ptr & (1<<i); 
      if (!ones)  {
         if (mask)  {
            ones = TRUE;
         }
      } else {
         if  (! mask)  {
            return FALSE;   
         }  
      } 
   }   
   return TRUE;
}

#endif /* SHELLCFG_USES_RTCS */
