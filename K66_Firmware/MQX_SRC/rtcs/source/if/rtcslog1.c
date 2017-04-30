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
*   This file contains the RTCS logging control functions.
*
*
*END************************************************************************/

#include <rtcs.h>
#include "rtcs_prv.h"

#if RTCSCFG_LOGGING
#if RTCSCFG_LOG_SOCKET_API || RTCSCFG_LOG_PCB || RTCSCFG_LOGGING
/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCSLOG_enable
* Returned Value  : void
* Comments  :  Enables logging of certain RTCS events
*
*END*-----------------------------------------------------------------*/

void RTCSLOG_enable
   (
      uint32_t     logtype
         /* [IN] the type of log entry to enable */
   )
{ /* Body */
   RTCS_DATA_PTR rtcs = RTCS_get_data();

   if (rtcs) {
      rtcs->RTCS_LOG_CONTROL |= logtype;
      RTCSLOG_enable_prot(logtype, RTCS_LOGCTRL_IFTYPE (IPIFTYPE_ETHERNET) );
      RTCSLOG_enable_prot(logtype, RTCS_LOGCTRL_IFTYPE (IPIFTYPE_PPP)      );
      RTCSLOG_enable_prot(logtype, RTCS_LOGCTRL_IFTYPE (IPIFTYPE_LOOPBACK) );
      RTCSLOG_enable_prot(logtype, RTCS_LOGCTRL_ARP    (IPIFTYPE_ETHERNET) );
      RTCSLOG_enable_prot(logtype, RTCS_LOGCTRL_PROTO  (IPPROTO_ICMP)      );
      RTCSLOG_enable_prot(logtype, RTCS_LOGCTRL_PROTO  (IPPROTO_IGMP)      );
      RTCSLOG_enable_prot(logtype, RTCS_LOGCTRL_PROTO  (IPPROTO_IP)        );
      RTCSLOG_enable_prot(logtype, RTCS_LOGCTRL_PROTO  (IPPROTO_TCP)       );
      RTCSLOG_enable_prot(logtype, RTCS_LOGCTRL_PROTO  (IPPROTO_UDP)       );
      RTCSLOG_enable_prot(logtype, RTCS_LOGCTRL_PROTO  (IPPROTO_OSPF)      );
      RTCSLOG_enable_prot(logtype, RTCS_LOGCTRL_PORT   (IPPORT_BOOTPC)     );
      RTCSLOG_enable_prot(logtype, RTCS_LOGCTRL_PORT   (IPPORT_RIP)        );
   }
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCSLOG_disable
* Returned Value  : void
* Comments  :  Disables logging of certain RTCS events
*
*END*-----------------------------------------------------------------*/

void RTCSLOG_disable
   (
      uint32_t     logtype
         /* [IN] the type of log entry to disable */
   )
{ /* Body */
   RTCS_DATA_PTR rtcs = RTCS_get_data();
   uint32_t       i;

   if (rtcs) {
      rtcs->RTCS_LOG_CONTROL &= ~logtype;

      if (logtype & RTCS_LOGCTRL_PCB) {
         for (i=0; i<RTCSLOG_PROT_MAX; i++) {
            rtcs->RTCS_LOG_PCB[i] = 0;
         } /* Endfor */
      } /* Endif */

      if (logtype & RTCS_LOGCTRL_TIMER) {
         for (i=0; i<RTCSLOG_PROT_MAX; i++) {
            rtcs->RTCS_LOG_TIMER[i] = 0;
         } /* Endfor */
      } /* Endif */
   }
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCSLOG_enable_prot
* Returned Value  : void
* Comments  :  Enables logging of certain RTCS events for a specific
*              protocol
*
*END*-----------------------------------------------------------------*/

void RTCSLOG_enable_prot
   (
      uint32_t     logtype,
         /* [IN] the type of log entry to enable */
      uint32_t     protocol
         /* [IN] the protocol to enable */
   )
{ /* Body */
   RTCS_DATA_PTR rtcs = RTCS_get_data();
   uint32_t       i, j;

   rtcs->RTCS_LOG_CONTROL |= logtype;

   if (logtype & RTCS_LOGCTRL_PCB) {
      j = RTCSLOG_PROT_MAX;
      for (i = RTCSLOG_PROT_MAX; i > 0; ) {
         i--;
         if (rtcs->RTCS_LOG_PCB[i] == protocol) {
            j = i;
            break;
         } else if (rtcs->RTCS_LOG_PCB[i] == 0) {
            j = i;
         } /* Endif */
      } /* Endfor */
      if (j < RTCSLOG_PROT_MAX) {
         rtcs->RTCS_LOG_PCB[j] = protocol;
      } /* Endif */
   } /* Endif */

   if (logtype & RTCS_LOGCTRL_TIMER) {
      j = RTCSLOG_PROT_MAX;
      for (i = RTCSLOG_PROT_MAX; i > 0; ) {
         i--;
         if (rtcs->RTCS_LOG_TIMER[i] == protocol) {
            j = i;
            break;
         } else if (rtcs->RTCS_LOG_TIMER[i] == 0) {
            j = i;
         } /* Endif */
      } /* Endfor */
      if (j < RTCSLOG_PROT_MAX) {
         rtcs->RTCS_LOG_TIMER[j] = protocol;
      } /* Endif */
   } /* Endif */

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCSLOG_disable_prot
* Returned Value  : void
* Comments  :  Disables logging of certain RTCS events for a specific
*              protocol
*
*END*-----------------------------------------------------------------*/

void RTCSLOG_disable_prot
   (
      uint32_t     logtype,
         /* [IN] the type of log entry to disable */
      uint32_t     protocol
         /* [IN] the protocol to disable */
   )
{ /* Body */
   RTCS_DATA_PTR rtcs = RTCS_get_data();
   uint32_t       i, j;

   rtcs->RTCS_LOG_CONTROL &= ~logtype | (RTCS_LOGCTRL_PCB | RTCS_LOGCTRL_TIMER);

   if (logtype & RTCS_LOGCTRL_PCB) {
      j = RTCSLOG_PROT_MAX;
      for (i = 0; i < RTCSLOG_PROT_MAX; i++) {
         if (rtcs->RTCS_LOG_PCB[i] == protocol) {
            rtcs->RTCS_LOG_PCB[i] = 0;
         } else if (rtcs->RTCS_LOG_PCB[i] != 0) {
            j = i;
         } /* Endif */
      } /* Endfor */
      if (j < RTCSLOG_PROT_MAX) {
         rtcs->RTCS_LOG_CONTROL &= ~RTCS_LOGCTRL_PCB;
      } /* Endif */
   } /* Endif */

   if (logtype & RTCS_LOGCTRL_TIMER) {
      j = RTCSLOG_PROT_MAX;
      for (i = 0; i < RTCSLOG_PROT_MAX; i++) {
         if (rtcs->RTCS_LOG_TIMER[i] == protocol) {
            rtcs->RTCS_LOG_TIMER[i] = 0;
         } else if (rtcs->RTCS_LOG_TIMER[i] != 0) {
            j = i;
         } /* Endif */
      } /* Endfor */
      if (j < RTCSLOG_PROT_MAX) {
         rtcs->RTCS_LOG_CONTROL &= ~RTCS_LOGCTRL_TIMER;
      } /* Endif */
   } /* Endif */

} /* Endbody */

#endif
#endif

/* EOF */
