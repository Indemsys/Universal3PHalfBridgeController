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
*   This file contains the utility functions for use by both the
*   DHCP Client and DHCP Server
*   For more information, refer to:
*   RFC 1541 (DHCP)
*   RPC 1533 (DHCP Options).
*
*
*END************************************************************************/

#include <rtcs.h>

#if RTCSCFG_ENABLE_IP4


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : DHCPCLNT_find_option
* Returned Value  : unsigned char
** Comments        : This function searches through the OPTIONS section
*                   of a DHCP message to find a specific option type.
*                   If it is found, a pointer to the specific option
*                   field is returned, If no option of the specified
*                   type exists, NULL is returned.
*
*END*-----------------------------------------------------------------*/

unsigned char *DHCPCLNT_find_option
   (
      unsigned char   *msgptr,
      uint32_t     msglen,
      unsigned char       option
   )
{ /* Body */



return( DHCP_find_option((msgptr + sizeof(DHCP_HEADER) + 4),
                         ((msglen - sizeof(DHCP_HEADER)) - 4),
                         option));

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : DHCP_find_option
* Returned Value  : unsigned char
** Comments        : This function searches through the OPTIONS section
*                   of a DHCP message to find a specific option type.
*                   If it is found, a pointer to the specific option
*                   field is returned, If no option of the specified
*                   type exists, NULL is returned.
*
*END*-----------------------------------------------------------------*/

unsigned char *DHCP_find_option
   (
      unsigned char   *msgptr,
      uint32_t     msglen,
      unsigned char       option
   )
{ /* Body */
   unsigned char optype;
   unsigned char oplen;

   for (;;) {

      /* Get the next option code */
      if (msglen == 0) {
         return NULL;
      } /* Endif */
      optype = mqx_ntohc(msgptr);
      msgptr++;
      msglen--;

      if (optype == DHCPOPT_END) {
         return NULL;
      } /* Endif */
      if (optype == DHCPOPT_PAD) {
         continue;
      } /* Endif */

      /* Get the option length */
      if (msglen == 0) {
         return NULL;
      } /* Endif */
      oplen = mqx_ntohc(msgptr);
      msgptr++;
      msglen--;

      if (msglen < oplen) {
         return NULL;
      } /* Endif */

      if (optype == option) {
         return msgptr-2;
      } /* Endif */

      msgptr += oplen;
      msglen -= oplen;

   } /* Endfor */
} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : DHCP_option_int8
* Returned Value  : TRUE if successful, FALSE otherwise
* Comments        : This function writes out an 8-bit option
*                   to the optptr buffer.
*
*END*-----------------------------------------------------------------*/

bool DHCP_option_int8
   (
      unsigned char   * *optptr,
      uint32_t      *optlen,
      unsigned char             opttype,
      unsigned char             optval
   )
{ /* Body */
   unsigned char *opt = *optptr;

   if ((*optlen) < 3) return FALSE;

   (void) mqx_htonc(opt, opttype); opt++;
   (void) mqx_htonc(opt, 1);       opt++;
   (void) mqx_htonc(opt, optval);  opt++;

   *optlen -= 3;
   *optptr = opt;
   return TRUE;

} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : DHCP_option_int16
* Returned Value  : TRUE if successful, FALSE otherwise
* Comments        : This function writes out a 16-bit option
*                   to the optptr buffer.
*
*END*-----------------------------------------------------------------*/

bool DHCP_option_int16
   (
      unsigned char   * *optptr,
      uint32_t      *optlen,
      unsigned char             opttype,
      uint16_t           optval
   )
{ /* Body */
   unsigned char *opt = *optptr;

   if ((*optlen) < 4) return FALSE;

   (void) mqx_htonc(opt, opttype); opt++;
   (void) mqx_htonc(opt, 2);       opt++;
   (void) mqx_htons(opt, optval);  opt += 2;

   *optlen -= 4;
   *optptr = opt;
   return TRUE;

} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : DHCP_option_int32
* Returned Value  : TRUE if successful, FALSE otherwise
* Comments        : This function writes out a 32-bit option
*                   to the optptr buffer.
*
*END*-----------------------------------------------------------------*/

bool DHCP_option_int32
   (
      unsigned char   * *optptr,
      uint32_t      *optlen,
      unsigned char             opttype,
      uint32_t           optval
   )
{ /* Body */
   unsigned char *opt = *optptr;

   if ((*optlen) < 6) return FALSE;

   (void) mqx_htonc(opt, opttype); opt++;
   (void) mqx_htonc(opt, 4);       opt++;
   (void) mqx_htonl(opt, optval);  opt += 4;

   *optlen -= 6;
   *optptr = opt;
   return TRUE;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : DHCP_option_variable
* Returned Value  : TRUE if successful, FALSE otherwise
* Comments        : This function writes out a variable size request option
*                   to the optptr buffer.
*
*END*-----------------------------------------------------------------*/

bool DHCP_option_variable
   (
      unsigned char   * *optptr,
      uint32_t      *optlen,
      unsigned char             opttype,
      unsigned char        *options,
      uint32_t           num_options
   )
{ /* Body */
   uint32_t   i;
   unsigned char *opt = *optptr;

   if ((*optlen) < (num_options + 2)) return FALSE;

   (void) mqx_htonc(opt, opttype); opt++;
   (void) mqx_htonc(opt, (unsigned char)num_options); opt++;

   for ( i = 0; i < num_options; i++ ) {
      (void) mqx_htonc(opt, options[i]); opt++;
   } /* Endfor */

   *optlen -= (num_options +  2);
   *optptr = opt;
   return TRUE;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : DHCP_option_addr
* Returned Value  : TRUE if successful, FALSE otherwise
* Comments        : This function writes out an IP address option
*                   to the optptr buffer.
*
*END*-----------------------------------------------------------------*/

bool DHCP_option_addr
   (
      unsigned char   * *optptr,
      uint32_t      *optlen,
      unsigned char             opttype,
      _ip_address       addr
   )
{ /* Body */
   unsigned char *opt = *optptr;

   if ((*optlen) < 6) return FALSE;

   (void) mqx_htonc(opt, opttype); opt++;
   (void) mqx_htonc(opt, 4);       opt++;
   (void) mqx_htonl(opt, addr);    opt += 4;

   *optlen -= 6;
   *optptr = opt;
   return TRUE;

} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : DHCP_option_addrlist
* Returned Value  : TRUE if successful, FALSE otherwise
* Comments        : This function writes out an IP address list option
*                   to the optptr buffer.
*
*END*-----------------------------------------------------------------*/

bool DHCP_option_addrlist
   (
      unsigned char   * *optptr,
      uint32_t      *optlen,
      unsigned char             opttype,
      _ip_address  *addrptr,
      uint32_t           addrcount
   )
{ /* Body */
   unsigned char   *opt = *optptr;
   _ip_address addr;

   if ((*optlen) < 4*addrcount+2) return FALSE;

   (void) mqx_htonc(opt, opttype);     opt++;
   (void) mqx_htonc(opt, 4*addrcount); opt++;
   while (addrcount--) {
      addr = *addrptr++;
      (void) mqx_htonl(opt, addr);     opt += 4;
   } /* Endwhile */

   *optlen -= 4*addrcount+2;
   *optptr = opt;
   return TRUE;

} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : DHCP_option_string
* Returned Value  : TRUE if successful, FALSE otherwise
* Comments        : This function writes out a variable-length option
*                   to the optptr buffer.
*
*END*-----------------------------------------------------------------*/

bool DHCP_option_string
   (
      unsigned char   * *optptr,
      uint32_t      *optlen,
      unsigned char             opttype,
      char          *optval
   )
{ /* Body */
   uint32_t   slen = 0;
   unsigned char *opt;

   opt = (unsigned char *)optval;
   while (*opt++) slen++;

   if ((*optlen) < slen+2) return FALSE;
   opt = *optptr;

   (void) mqx_htonc(opt, opttype); opt++;
   (void) mqx_htonc(opt, slen);    opt++;
   _mem_copy(optval, opt, slen); opt += slen;

   *optlen -= slen+2;
   *optptr = opt;
   return TRUE;

} /* Endbody */


#endif /* RTCSCFG_ENABLE_IP4 */


/* EOF */
