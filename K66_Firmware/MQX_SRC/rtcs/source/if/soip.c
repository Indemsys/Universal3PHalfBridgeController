/*HEADER**********************************************************************
*
* Copyright 2008, 2014 Freescale Semiconductor, Inc.
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
*   This file contains the implementation of getsockopt()
*   and setsockopt() at the SOL_IP level.
*
*
*END************************************************************************/

#include <rtcs.h>
#include "rtcs_prv.h"
#include "tcpip.h"
#include "socket.h"

#define RTCS_ENTER(f,a)
#define RTCS_EXIT(f,a)  return a

uint32_t SOL_IP_getsockopt  (uint32_t, uint32_t, uint32_t, void *, uint32_t *);
uint32_t SOL_IP_setsockopt  (uint32_t, uint32_t, uint32_t, const void *, uint32_t);

const RTCS_SOCKOPT_CALL_STRUCT SOL_IP_CALL = {
   SOL_IP_getsockopt,
   SOL_IP_setsockopt
};


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOL_IP_getsockopt
* Returned Value  : error code
* Comments  :  Obtain the current value for a socket option.
*
*END*-----------------------------------------------------------------*/

uint32_t  SOL_IP_getsockopt
   (
      uint32_t        sock,
         /* [IN] socket handle */
      uint32_t        level,
         /* [IN] protocol level for the option */
      uint32_t        optname,
         /* [IN] name of the option */
      void          *optval,
         /* [IN] buffer for the current value of the option */
      uint32_t   *optlen
         /* [IN/OUT] length of the option value, in bytes */
   )
{ /* Body */
   register SOCKET_STRUCT_PTR    socket_ptr = (SOCKET_STRUCT_PTR)sock;
   uint32_t                       error = RTCS_OK;

   RTCS_ENTER(GETSOCKOPT, sock);

   switch (optname) {

      case RTCS_SO_IP_RX_DEST:
         if (*optlen < sizeof(_ip_address)) {
            RTCS_EXIT(GETSOCKOPT, RTCSERR_SOCK_SHORT_OPTION);
         } /* Endif */

         *(_ip_address *)optval = socket_ptr->LINK_OPTIONS.RX.DEST;
         *optlen = sizeof(_ip_address);
         break;

      case RTCS_SO_IP_RX_TTL:
         if (!*optlen) {
            RTCS_EXIT(GETSOCKOPT, RTCSERR_SOCK_SHORT_OPTION);
         } /* Endif */

         *(unsigned char *)optval = socket_ptr->LINK_OPTIONS.RX.TTL;
         *optlen = sizeof(unsigned char);
         break;

      case RTCS_SO_IP_TX_TTL:
         if (!*optlen) {
            RTCS_EXIT(GETSOCKOPT, RTCSERR_SOCK_SHORT_OPTION);
         } /* Endif */

         *(unsigned char *)optval = socket_ptr->LINK_OPTIONS.TX.TTL;
         *optlen = sizeof(unsigned char);
         break;

      case RTCS_SO_IP_LOCAL_ADDR:
#if RTCSCFG_ENABLE_IP4
         if (*optlen < sizeof(_ip_address)) {
            RTCS_EXIT(GETSOCKOPT, RTCSERR_SOCK_SHORT_OPTION);
         } /* Endif */

         if ( socket_ptr->TCB_PTR != NULL)  {
            *(_ip_address *)optval = SOCKADDR_get_ipaddr4(&socket_ptr->TCB_PTR->laddr);
         } else  
         {
            *(_ip_address *)optval = 0;
         }
         *optlen = sizeof(_ip_address);
#else
         *(uint32_t*)optval = 0;
         *optlen = 0;
#endif
         break;

      case RTCS_SO_IP_RX_TOS:
         if (!*optlen) {
            RTCS_EXIT(GETSOCKOPT, RTCSERR_SOCK_SHORT_OPTION);
         } /* Endif */

         *(unsigned char *)optval = socket_ptr->LINK_OPTIONS.RX.TOS;
         *optlen = sizeof(unsigned char);
         break;

      case RTCS_SO_IP_TX_TOS:
         if (!*optlen) {
            RTCS_EXIT(GETSOCKOPT, RTCSERR_SOCK_SHORT_OPTION);
         } /* Endif */

         *(unsigned char *)optval = socket_ptr->LINK_OPTIONS.TX.TOS;
         *optlen = sizeof(unsigned char);
         break;         
         
       default:
         error = RTCSERR_SOCK_INVALID_OPTION;
      } /* Endswitch */

   RTCS_EXIT(GETSOCKOPT, error);

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOL_IP_setsockopt
* Returned Value  : error code
* Comments  :  Modify the current value for a socket option.
*
*END*-----------------------------------------------------------------*/

uint32_t  SOL_IP_setsockopt
   (
      uint32_t        sock,
         /* [IN] socket handle */
      uint32_t        level,
         /* [IN] protocol level for the option */
      uint32_t        optname,
         /* [IN] name of the option */
      const void          *optval,
         /* [IN] new value for the option */
      uint32_t        optlen
         /* [IN] length of the option value, in bytes */
   )
{ /* Body */
/* Start CR 1146  */
   register SOCKET_STRUCT_PTR    socket_ptr = (SOCKET_STRUCT_PTR)sock;
   uint32_t     error;

   RTCS_ENTER(SETSOCKOPT, sock);

   switch (optname) {

      case RTCS_SO_IP_TX_TTL:
         if (!optlen) {
            error =  RTCSERR_SOCK_SHORT_OPTION;
         } else  {
            socket_ptr->LINK_OPTIONS.TX.TTL = *(unsigned char *)optval;
            /* Propogate the option to the TCB */
            if (socket_ptr->TCB_PTR != NULL)  {
               socket_ptr->TCB_PTR->TX.TTL = socket_ptr->LINK_OPTIONS.TX.TTL;
            }
            error = RTCS_OK;
         } /* Endif */
         break;
      case RTCS_SO_IP_TX_TOS:
         if (!optlen) {
            error =  RTCSERR_SOCK_SHORT_OPTION;
         } else  {
            socket_ptr->LINK_OPTIONS.TX.TOS = *(unsigned char *)optval;
            /* Propogate the option to the TCB */
            if (socket_ptr->TCB_PTR != NULL)  {
               socket_ptr->TCB_PTR->TX.TOS = socket_ptr->LINK_OPTIONS.TX.TOS;
            }
            error = RTCS_OK;
         } /* Endif */
         break;
         
       default:
         error = RTCSERR_SOCK_INVALID_OPTION;
      } /* Endswitch */
/* End CR 1146 */

   RTCS_EXIT(SETSOCKOPT, error);
} /* Endbody */

/* EOF */
