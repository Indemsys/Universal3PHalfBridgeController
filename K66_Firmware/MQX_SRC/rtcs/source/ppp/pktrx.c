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
*   This file contains the implementation of the PPP Rx
*   task.
*
*
*END************************************************************************/

#include <rtcs.h>

#if RTCSCFG_ENABLE_PPP && !PLATFORM_SDK_ENABLED

#include <ppp.h>
#include "ppp_prv.h"


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPP_findprot
* Returned Value  :  TRUE if protocol is registered
* Comments        :
*     Finds a protocol callback.  If found, the callback is
*     moved to the head of the list.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static bool PPP_findprot
   (
      PPP_CFG_PTR    ppp_ptr,
            /* [IN] - the PPP state structure */
      uint16_t        protocol
            /* [IN] - protocol */
   )
{ /* Body */
   PROT_CALL_PTR       *search_ptr;
   bool              found;

   for (search_ptr = &ppp_ptr->PROT_CALLS;;
        search_ptr = &(*search_ptr)->NEXT) {
      if (*search_ptr == NULL) {
         found = FALSE;
         break;
      } else if ((*search_ptr)->PROTOCOL == protocol) {
         found = TRUE;
         break;
      } /* Endif */
   } /* Endfor */

   if (found) {
      PROT_CALL_PTR prot_ptr = *search_ptr;
      *search_ptr = prot_ptr->NEXT;
      prot_ptr->NEXT = ppp_ptr->PROT_CALLS;
      ppp_ptr->PROT_CALLS = prot_ptr;
   } /* Endif */

   return found;

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPP_register
* Returned Value  :  error code
* Comments        :
*     Registers a callback function for a protocol.
*
*END*-----------------------------------------------------------------*/

uint32_t PPP_register
   (
      _ppp_handle         handle,
            /* [IN] - the PPP state structure */
      uint16_t             protocol,
            /* [IN] - protocol */
      void (_CODE_PTR_    callback)(PCB_PTR, void *),
            /* [IN] - callback function */
      void               *parameter
            /* [IN] - first parameter for callback function */
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   PPP_CFG_PTR    ppp_ptr = handle;
   PROT_CALL_PTR  prot_ptr;

   /* Do some error checking */
   if (ppp_ptr->VALID != PPP_VALID) {
      return RTCSERR_PPP_INVALID_HANDLE;
   } else if (!(protocol&1) || (protocol&0x100)) {
      return RTCSERR_PPP_INVALID_PROTOCOL;
   } /* Endif */

   /* Allocate a callback structure */
   prot_ptr = _mem_alloc_system_zero(sizeof(PROT_CALL));
   if (prot_ptr == NULL) {
      return RTCSERR_PPP_ALLOC_CALL_FAILED;
   } /* Endif */
   protocol &= 0xFFFF;
   prot_ptr->PROTOCOL = protocol;
   prot_ptr->CALLBACK = callback;
   prot_ptr->PARAM    = parameter;

   PPP_mutex_lock(&ppp_ptr->MUTEX);

   if (PPP_findprot(ppp_ptr, protocol)) {
      ppp_ptr->PROT_CALLS->CALLBACK = callback;
      ppp_ptr->PROT_CALLS->PARAM    = parameter;
   } else {
      prot_ptr->NEXT      = ppp_ptr->PROT_CALLS;
      ppp_ptr->PROT_CALLS = prot_ptr;
      prot_ptr = NULL;
   } /* Endif */

   PPP_mutex_unlock(&ppp_ptr->MUTEX);

   if (prot_ptr) {
      _mem_free(prot_ptr);
   } /* Endif */

   return PPP_OK;

#else

    return RTCSERR_IP_IS_DISABLED;    

#endif /* RTCSCFG_ENABLE_IP4 */      

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPP_unregister
* Returned Value  :  error code
* Comments        :
*     Unregisters a callback function for a protocol.
*
*END*-----------------------------------------------------------------*/

uint32_t PPP_unregister
   (
      _ppp_handle         handle,
            /* [IN] - the PPP state structure */
      uint16_t             protocol
            /* [IN] - protocol */
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   PPP_CFG_PTR    ppp_ptr = handle;
   PROT_CALL_PTR  prot_ptr;
   uint32_t        error;

   /* Do some error checking */
   if (ppp_ptr->VALID != PPP_VALID) {
      return RTCSERR_PPP_INVALID_HANDLE;
   } else if (!(protocol&1) || (protocol&0x100)) {
      return RTCSERR_PPP_INVALID_PROTOCOL;
   } else if (!(~protocol & 0xC000)) {
      /* Don't allow unregistering LCP */
      return RTCSERR_PPP_INVALID_PROTOCOL;
   } /* Endif */

   /* Remove structure from list */
   protocol &= 0xFFFF;
   prot_ptr = NULL;
   error = RTCSERR_PPP_PROT_NOT_FOUND;
   PPP_mutex_lock(&ppp_ptr->MUTEX);

   if (PPP_findprot(ppp_ptr, protocol)) {
      prot_ptr = ppp_ptr->PROT_CALLS;
      ppp_ptr->PROT_CALLS = prot_ptr->NEXT;
      error = PPP_OK;
   } /* Endif */

   PPP_mutex_unlock(&ppp_ptr->MUTEX);

   if (prot_ptr) {
      _mem_free(prot_ptr);
   } /* Endif */

   return error;

#else

    return RTCSERR_IP_IS_DISABLED;    

#endif /* RTCSCFG_ENABLE_IP4 */  

} /* Endbody */


/*TASK*-----------------------------------------------------------------
*
* Task Name       :   PPP_rx_task
* Comments        :
*     This task receives and forwards incoming packets.
*
*END*-----------------------------------------------------------------*/

void PPP_rx_task
   (
      void       *handle,
            /* [IN] - the PPP state structure */
      void       *creator
            /* [IN] - the task create information */
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   PPP_CFG_PTR          ppp_ptr = handle;
   PCB_PTR              pcb;
   bool              linkstate, linkauth;
   PPP_OPT              opt;
   uint16_t              protocol, len;
   void (_CODE_PTR_     callback)(PCB_PTR, void *);
   void                *param;


    ppp_ptr->RX_TASKID = RTCS_task_getid();
    RTCS_task_resume_creator(creator, PPP_OK);

      /* Wait for incoming packets */
    while (!ppp_ptr->STOP_RX) 
    {    
      /**********************************************************
      **
      **    Wait for a frame
      **
      ***********************************************************/

      /* Read a frame */
      /*Lets use flag for send pointer to ppp_ptr->STOP_RX */
      pcb = _iopcb_read(ppp_ptr->DEVICE, (uint32_t)&(ppp_ptr->STOP_RX));  

      if(NULL == pcb)  
      {  
         _time_delay(1);  
         continue;  
      }  
      len = pcb->FRAG[0].LENGTH;
      if (len < 2) 
      {
         PCB_free(pcb);
         ppp_ptr->ST_RX_RUNT++;
         continue;
      } /* Endif */
      protocol = mqx_ntohs(pcb->FRAG[0].FRAGMENT);

      /* Read the negotiated Receive options */
      PPP_mutex_lock(&ppp_ptr->MUTEX);
      linkstate = ppp_ptr->LINK_STATE;
      linkauth  = ppp_ptr->LINK_AUTH;
      opt = *ppp_ptr->RECV_OPTIONS;
      PPP_mutex_unlock(&ppp_ptr->MUTEX);

      /* Discard all non-LCP packets until the link is opened */
      if (protocol != PPP_PROT_LCP) {
         if (!linkstate) {
            PCB_free(pcb);
            ppp_ptr->ST_RX_DISCARDED++;
            continue;
         } /* Endif */
         /* and all non-AP packets until the link is authenticated */
         if (protocol != opt.AP) {
            if (!linkauth) {
               PCB_free(pcb);
               ppp_ptr->ST_RX_DISCARDED++;
               continue;
            } /* Endif */
         } /* Endif */
      } /* Endif */

      /* Decompress the packet if compression was negotiated */
      //if ((protocol == PPP_PROT_CP) && opt.CP) {
      //   pcb = opt.CP->CP_decomp(&ppp_ptr->CCP_STATE.RECV_DATA, pcb, ppp_ptr, &opt);
      //   protocol = mqx_ntohs(pcb->FRAG[0].FRAGMENT);
      //} /* Endif */

      /**********************************************************
      **
      **    Forward the packet to higher-level protocol
      **
      */

      /* Find out where to send the packet */
      ppp_ptr->ST_RX_RECEIVED++;


      /*
      ** We could put the known protocols in the PROT_CALLS
      ** list, but we don't because we always want them to
      ** work, even if someone tries to PPP_unregister() them.
      */
      switch (protocol) {

         /* Got an LCP packet */
      case PPP_PROT_LCP:
         LCP_input(pcb, &ppp_ptr->LCP_FSM);
         break;

         /* Got a CCP packet */
      //case PPP_PROT_CCP:
      //   CCP_input(pcb, &ppp_ptr->CCP_FSM);
      //   break;

         /* Got a PAP packet */
      case PPP_PROT_PAP:
         PAP_input(pcb, ppp_ptr);
         break;

         /* Got a CHAP packet */
      case PPP_PROT_CHAP:
         CHAP_input(pcb, ppp_ptr);
         break;

      default:
         callback = NULL;
         PPP_mutex_lock(&ppp_ptr->MUTEX);
         if (PPP_findprot(ppp_ptr, protocol)) {
            callback = ppp_ptr->PROT_CALLS->CALLBACK;
            param    = ppp_ptr->PROT_CALLS->PARAM;
         } /* Endif */
         PPP_mutex_unlock(&ppp_ptr->MUTEX);

         if (callback) {
            callback(pcb, param);
         } else {
            /* No callback found -- Send Protocol-Reject */
            LCP_sendprotrej(pcb, &ppp_ptr->LCP_FSM);
         } /* Endif */
         break;
      } /* Endswitch */

   } /* Endfor */
   /* We can start task again. (STOP_RX == FALSE) */
   ppp_ptr->STOP_RX = FALSE; 
#endif /* RTCSCFG_ENABLE_IP4 */


} /* Endbody */

#endif // RTCSCFG_ENABLE_PPP
