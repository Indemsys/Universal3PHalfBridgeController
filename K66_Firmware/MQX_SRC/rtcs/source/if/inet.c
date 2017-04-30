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
*   This file contains some Internet address manipulation
*   functions.
*
*
*END************************************************************************/

#include <ctype.h>
#include <rtcs.h>


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : inet_aton
* Returned Value  : -1 on success, 0 on error
* Comments  :  Converts a dotted-decimal into a binary in_addr
*
*END*-----------------------------------------------------------------*/

int32_t inet_aton
   (
      const char   *name,
         /* [IN] dotted decimal IP address */
      in_addr      *ipaddr_ptr
         /* [OUT] binary IP address */
   )
{ /* Body */
   bool     ipok = FALSE;
   uint32_t     dots;
   uint32_t     byte;
   _ip_address addr;

   addr = 0;
   dots = 0;
   for (;;) {

      if (!isdigit((int) *name)) break;

      byte = 0;
      while (isdigit((int) *name)) {
         byte *= 10;
         byte += *name - '0';
         if (byte > 255) break;
         name++;
      } /* Endwhile */
      if (byte > 255) break;
      addr <<= 8;
      addr += byte;

      if (*name == '.') {
         dots++;
         if (dots > 3) break;
         name++;
         continue;
      } /* Endif */

      if ((*name == '\0') && (dots == 3)) {
         ipok = TRUE;
      } /* Endif */

      break;

   } /* Endfor */

   if (!ipok) {
      return 0;
   } /* Endif */

   if (ipaddr_ptr) {
      ipaddr_ptr->s_addr = addr;
   } /* Endif */

   return -1;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : inet_addr
* Returned Value  : a binary IP address, or -1 on error
* Comments  :  Converts a dotted-decimal into a binary in_addr
*
*END*-----------------------------------------------------------------*/

_ip_address inet_addr
   (
      const char   *name
         /* [IN] dotted decimal IP address */
   )
{ /* Body */
   in_addr  ipaddr;
   int32_t   ipok;

   ipok = inet_aton(name, &ipaddr);
   if (!ipok) {
      return INADDR_BROADCAST;
   } /* Endif */
   return ipaddr.s_addr;

} /* Endbody */


/* EOF */
