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
*   This file contains the implementation for the
*   Password Authentication Protocol.
*
*
*END************************************************************************/

#include <rtcs.h>

#if  RTCSCFG_ENABLE_IP4 && RTCSCFG_ENABLE_PPP && !PLATFORM_SDK_ENABLED

#include <string.h>
#include <ppp.h>
#include "ppp_prv.h"


static void PAP_timeout (void *, PCB_PTR, bool);
static void PAP_up      (_ppp_handle);
static void PAP_fail    (_ppp_handle);
static void PAP_close   (void *, PCB_PTR, bool);


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PAP_init
* Returned Value  :  void
* Comments        :
*     Called by PPP_initialize.  Initializes the PAP state.
*
*END*-----------------------------------------------------------------*/

void PAP_init
   (
      _ppp_handle   handle
            /* [IN] - the PPP state structure */
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   PPP_CFG_PTR ppp_ptr = handle;

   ppp_ptr->PAP_STATE.CLIENT_STATE = PAP_STATE_CLOSED;
   ppp_ptr->PAP_STATE.SERVER_STATE = PAP_STATE_CLOSED;

#endif /* RTCSCFG_ENABLE_IP4 */

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PAP_timeout
* Returned Value  :  void
* Comments        :
*     Called by PPP_tx_task.  Timeout expired.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static void PAP_timeout
   (
      void          *pap_data,
            /* [IN/OUT] - PAP state */
      PCB_PTR        pcb,
            /* [IN] - expired packet */
      bool        hard
            /* [IN] - TRUE if this is a hard timeout (TO- event) */
   )
{ /* Body */
   PAP_DATA_PTR   pap_ptr = pap_data;

   /* Increment the identifier */
   pap_ptr->CURID = (pap_ptr->CURID + 1) & 0xFF;
   pcb->FRAG[0].FRAGMENT[3] = pap_ptr->CURID;

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PAP_send
* Returned Value  :  void
* Comments        :
*     Called by LCP_up.  Sends an Authenticate-Request packet.
*
*END*-----------------------------------------------------------------*/

void PAP_send
   (
      _ppp_handle    handle
            /* [IN] - the PPP state structure */
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   PPP_CFG_PTR    ppp_ptr = handle;
   PAP_DATA_PTR   pap_ptr = &ppp_ptr->PAP_STATE;
   PCB_PTR        pcb;
   unsigned char      *outp;
   uint16_t        len;

   pap_ptr->CLIENT_STATE = PAP_STATE_INITIAL;
   pap_ptr->CURID = RTCS_rand() & 0xFF;

   /* Acquire a PCB */
   len = PAP_HDR_LEN + 2
/* Start CR 2207 */
       + PPP_SECRET(ppp_ptr,_PPP_PAP_LSECRET->PPP_ID_LENGTH)
       + PPP_SECRET(ppp_ptr,_PPP_PAP_LSECRET->PPP_PW_LENGTH);
/* End CR 2207 */

   pcb = PPP_pcballoc(PPP_PROT_PAP, len);
   if (pcb == NULL) {
      return;
   } /* Endif */

   /* Build an Authenticate-Request packet */
   outp = pcb->FRAG[0].FRAGMENT + 2;
   *outp++ = PAP_CODE_AUTH_REQ;
   *outp++ = pap_ptr->CURID;
   *outp++ = (len >> 8) & 0xFF;
   *outp++ =  len       & 0xFF;
/* Start CR 2207 */
   *outp++ = len = PPP_SECRET(ppp_ptr,_PPP_PAP_LSECRET->PPP_ID_LENGTH);
   PPP_memcopy(PPP_SECRET(ppp_ptr,_PPP_PAP_LSECRET->PPP_ID_PTR), outp, len);
   outp += len;
   *outp++ = len = PPP_SECRET(ppp_ptr,_PPP_PAP_LSECRET->PPP_PW_LENGTH);
   PPP_memcopy(PPP_SECRET(ppp_ptr,_PPP_PAP_LSECRET->PPP_PW_PTR), outp, len);
/* End CR 2207 */

   /* Send the PCB */
   PPP_send_rexmit(ppp_ptr, PPP_PROT_PAP, pcb, 0, PAP_timeout, pap_ptr);

#endif /* RTCSCFG_ENABLE_IP4 */

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PAP_open
* Returned Value  :  void
* Comments        :
*     Called by LCP_up.  Starts the PAP server.
*
*END*-----------------------------------------------------------------*/

void PAP_open
   (
      _ppp_handle    handle
            /* [IN] - the PPP state structure */
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   PPP_CFG_PTR ppp_ptr = handle;

   ppp_ptr->PAP_STATE.SERVER_STATE = PAP_STATE_INITIAL;
   
#endif /* RTCSCFG_ENABLE_IP4 */

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PAP_input
* Returned Value  :  void
* Comments        :
*     Called by PPP_rx_task.  Parses a PAP packet.
* Side effects    :
*     Consumes (sends or frees) the packet.
*
*END*-----------------------------------------------------------------*/

void PAP_input
   (
      PCB_PTR        pcb,
            /* [IN] - PAP packet */
      _ppp_handle    handle
            /* [IN] - the PPP state structure */
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   PPP_CFG_PTR    ppp_ptr = handle;
   PAP_DATA_PTR   pap_ptr = &ppp_ptr->PAP_STATE;
   PPP_SECRET_PTR secret;
   unsigned char      *inp = pcb->FRAG[0].FRAGMENT + 2;
   unsigned char      *idp, *pwp;
   unsigned char          code, id;
   unsigned char          idlen, pwlen;
   uint16_t        len;
   bool        delay = FALSE;

   /*
   ** Parse header (code, id and length).
   ** If packet too short, drop it.
   */
   if (pcb->FRAG[0].LENGTH < 2 + PAP_HDR_LEN) {
      pap_ptr->ST_PAP_SHORT++;
      PCB_free(pcb);
      return;
   } /* Endif */

   code = *inp++;
   id = *inp++;
   len = *inp++ << 8;
   len += *inp++;

   if ((len < PAP_HDR_LEN) || (len > pcb->FRAG[0].LENGTH - 2)) {
      pap_ptr->ST_PAP_SHORT++;
      PCB_free(pcb);
      return;
   } /* Endif */
   len -= PAP_HDR_LEN;

   switch (code) {

   case PAP_CODE_AUTH_REQ:

      switch (pap_ptr->SERVER_STATE) {
      case PAP_STATE_INITIAL:

         /* Parse the peer id and password */
         idp = inp;
         idlen = *idp++;
         pwp = idp + idlen;
         pwlen = *pwp++;
         if (len < idlen + pwlen + 2) {
            pap_ptr->ST_PAP_SHORT++;
            PCB_free(pcb);
            break;
         } /* Endif */

         /* Match id/password pair against the secrets table */
/* Start CR 2207 */
         secret = PPP_SECRET(ppp_ptr,_PPP_PAP_RSECRETS);
/* End CR 2207 */
         for (;;) {
            if ((secret->PPP_ID_LENGTH == 0)
             && (secret->PPP_PW_LENGTH == 0)) {
               pap_ptr->SERVER_STATE = PAP_STATE_AUTH_NAK;
               PAP_fail(handle);
               delay = TRUE;
               break;
            } /* Endif */
            if ((secret->PPP_ID_LENGTH == idlen)
             && (secret->PPP_PW_LENGTH == pwlen)
             && (memcmp(secret->PPP_ID_PTR, idp, idlen) == 0)
             && (memcmp(secret->PPP_PW_PTR, pwp, pwlen) == 0)) {
               pap_ptr->SERVER_STATE = PAP_STATE_AUTH_ACK;
               PAP_up(handle);
               break;
            } /* Endif */
            secret++;
         } /* Endfor */

         /* fall through */
      case PAP_STATE_AUTH_ACK:
      case PAP_STATE_AUTH_NAK:
         /* Build an Auth-Ack or Auth-Nak reply */
         inp = pcb->FRAG[0].FRAGMENT + 2;
         *inp++ = pap_ptr->SERVER_STATE;
         inp++;                              /* Keep same ID */
         *inp++ = 0;
         *inp++ = 5;
         *inp++ = 0;
         pcb->FRAG[0].LENGTH = 7;
         if (delay) {
            PPP_send_rexmit(ppp_ptr, PPP_PROT_PAP, pcb, 1, PAP_close, handle);
         } else {
            PPP_send_one(ppp_ptr, PPP_PROT_PAP, pcb);
         } /* Endif */
         break;

      default:
         pap_ptr->ST_PAP_NOAUTH++;
         PCB_free(pcb);
         break;
      } /* Endswitch */
      break;

   case PAP_CODE_AUTH_ACK:
       pap_ptr->CLIENT_STATE = PAP_STATE_AUTH_ACK;
       PAP_up(handle);
   case PAP_CODE_AUTH_NAK:
      if (pap_ptr->CLIENT_STATE != PAP_STATE_AUTH_ACK) {
         pap_ptr->CLIENT_STATE = PAP_STATE_AUTH_NAK;
      } /* Endif */
      
      if (pap_ptr->CLIENT_STATE != PAP_STATE_INITIAL && pap_ptr->CLIENT_STATE != PAP_STATE_AUTH_ACK) {
         pap_ptr->ST_PAP_NOREQ++;
      } else if (id != pap_ptr->CURID) {
         pap_ptr->ST_PAP_ID++;
      } else if (len < 1) {
         pap_ptr->ST_PAP_SHORT++;
      } else if (--len < *inp++) {
         pap_ptr->ST_PAP_SHORT++;
      } else {
         /* All is well -- stop send Auth-Req's */
         PPP_send_stop(ppp_ptr, PPP_PROT_PAP);
      } /* Endif */
      PCB_free(pcb);
      break;

   default:
      pap_ptr->ST_PAP_CODE++;
      PCB_free(pcb);
      break;
   } /* Endswitch */

#else

    PCB_free(pcb);

#endif /* RTCSCFG_ENABLE_IP4 */

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PAP_up
* Returned Value  :  void
* Comments        :
*     Called by PAP_input once the peer has been authenticated.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static void PAP_up
   (
      _ppp_handle    handle
            /* [IN] - the PPP state structure */
   )
{ /* Body */
   PPP_CFG_PTR ppp_ptr = handle;

   PPP_mutex_lock(&ppp_ptr->MUTEX);
   ppp_ptr->LINK_AUTH = TRUE;
   PPP_mutex_unlock(&ppp_ptr->MUTEX);

   PPP_up(handle);

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PAP_fail
* Returned Value  :  void
* Comments        :
*     Called by PAP_input if the peer's credentials are rejected.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static void PAP_fail
   (
      _ppp_handle    handle
            /* [IN] - the PPP state structure */
   )
{ /* Body */
   PPP_CFG_PTR ppp_ptr = handle;
   PPP_CALL_INTERNAL_PTR call_ptr = &ppp_ptr->LCP_CALL[PPP_CALL_AUTH_FAIL];

   if (call_ptr->CALLBACK) {
      call_ptr->CALLBACK(call_ptr->PARAM);
   } /* Endif */

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PAP_close
* Returned Value  :  void
* Comments        :
*     Called by the Tx task.  Terminates the link after an
*     Authenticate-Nak packet is sent.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static void PAP_close
   (
      void          *handle,
            /* [IN] - the PPP state structure */
      PCB_PTR        pcb,
            /* [IN] - expired packet */
      bool        hard
            /* [IN] - TRUE if this is a hard timeout (TO- event) */
   )
{ /* Body */

   PPP_close(handle);

} /* Endbody */


#endif /* RTCSCFG_ENABLE_IP4 */

#endif //RTCSCFG_ENABLE_PPP
