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
*   This file contains the RTCS logging functions.
*
*
*END************************************************************************/

#include <rtcs.h>
#include "rtcs_prv.h"

#if RTCSCFG_LOGGING
#if RTCSCFG_LOG_SOCKET_API || RTCSCFG_LOG_PCB || RTCSCFG_LOGGING
/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_log
* Returned Value  : void
* Comments  :  Logs an RTCS event
*
*END*-----------------------------------------------------------------*/

void RTCS_log
   (
      uint32_t     logtype,    /* [IN] specifies the type of log entry */
         
      uint32_t     p1,         /* [IN] first parameter */
      uint32_t     p2,         /* [IN] second parameter */
      uint32_t     p3,         /* [IN] third parameter */
      uint32_t     p4,         /* [IN] fourth parameter */
      uint32_t     p5          /* [IN] fifth parameter */
   )
{ /* Body */
   RTCS_DATA_PTR rtcs = RTCS_get_data();
   uint32_t       i;

   switch (logtype) {
   case RTCSLOG_TYPE_FNENTRY:
   case RTCSLOG_TYPE_FNEXIT:
      if (!(rtcs->RTCS_LOG_CONTROL & RTCS_LOGCTRL_FUNCTION)) {
         return;
      } /* Endif */
      break;

   case RTCSLOG_TYPE_PCB:
      if (!(rtcs->RTCS_LOG_CONTROL & RTCS_LOGCTRL_PCB)) {
         return;
      } /* Endif */
      for (i = 0; i < RTCSLOG_PROT_MAX; i++) {
         if (rtcs->RTCS_LOG_PCB[i] == (p1 & 0xFFFF)) {
            break;
         } /* Endif */
      } /* Endfor */
      if (i == RTCSLOG_PROT_MAX) {
         return;
      } /* Endif */
      break;

   case RTCSLOG_TYPE_TIMER:
      if (!(rtcs->RTCS_LOG_CONTROL & RTCS_LOGCTRL_TIMER)) {
         return;
      } /* Endif */
      for (i = 0; i < RTCSLOG_PROT_MAX; i++) {
         if (rtcs->RTCS_LOG_TIMER[i] == (p1 & 0xFFFF)) {
            break;
         } /* Endif */
      } /* Endfor */
      if (i == RTCSLOG_PROT_MAX) {
         return;
      } /* Endif */
      break;
    case RTCSLOG_TYPE_ERROR:
        break;
   default:
      return;
   } /* Endswitch */

   RTCS_log_internal(logtype,p1,p2,p3,p4,p5);

} /* Endbody */
#endif
#endif
/* EOF */
