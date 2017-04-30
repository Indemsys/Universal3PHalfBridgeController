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
*   Challenge Handshake Authentication Protocol.
*
*
*END************************************************************************/
#include <rtcs.h>

#if  RTCSCFG_ENABLE_IP4 && RTCSCFG_ENABLE_PPP && !PLATFORM_SDK_ENABLED

#include <string.h>
#include <ppp.h>
#include "ppp_prv.h"


static void CHAP_timeout (void *, PCB_PTR, bool);
static void CHAP_up      (_ppp_handle);
static void CHAP_fail    (_ppp_handle);
static void CHAP_close   (void *, PCB_PTR, bool);


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  CHAP_init
* Returned Value  :  void
* Comments        :
*     Called by PPP_initialize.  Initializes the CHAP state.
*
*END*-----------------------------------------------------------------*/

void CHAP_init
   (
      _ppp_handle  handle
            /* [IN] - the PPP state structure */
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   PPP_CFG_PTR ppp_ptr = handle;

   ppp_ptr->CHAP_STATE.CLIENT_STATE = CHAP_STATE_CLOSED;
   ppp_ptr->CHAP_STATE.SERVER_STATE = CHAP_STATE_CLOSED;

#endif /* RTCSCFG_ENABLE_IP4 */

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  CHAP_timeout
* Returned Value  :  void
* Comments        :
*     Called by PPP_tx_task.  Timeout expired.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static void CHAP_timeout
   (
      void          *chap_data,
            /* [IN/OUT] - CHAP state */
      PCB_PTR        pcb,
            /* [IN] - expired packet */
      bool        hard
            /* [IN] - TRUE if this is a hard timeout (TO- event) */
   )
{ /* Body */
   CHAP_DATA_PTR  chap_ptr = chap_data;
   uint16_t i;

   /* Increment the identifier */
   chap_ptr->CURID = (chap_ptr->CURID + 1) & 0xFF;
   pcb->FRAG[0].FRAGMENT[3] = chap_ptr->CURID;

   /* Generate a new challenge */
   for (i=0; i<CHAP_CHALLENGE_LEN; i++) {
      pcb->FRAG[0].FRAGMENT[i+7]= chap_ptr->MD5[i] = RTCS_rand() & 0xFF;
   } /* Endfor */

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  CHAP_open
* Returned Value  :  void
* Comments        :
*     Called by LCP_up.  Starts the CHAP client.
*
*END*-----------------------------------------------------------------*/

void CHAP_open
   (
      _ppp_handle    handle
            /* [IN] - the PPP state structure */
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   PPP_CFG_PTR ppp_ptr = handle;

   ppp_ptr->CHAP_STATE.CLIENT_STATE = CHAP_STATE_INITIAL;

#endif /* RTCSCFG_ENABLE_IP4 */

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  CHAP_challenge
* Returned Value  :  void
* Comments        :
*     Called by LCP_up.  Sends a Challenge packet.
*
*END*-----------------------------------------------------------------*/

void CHAP_challenge
   (
      _ppp_handle    handle
            /* [IN] - the PPP state structure */
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   PPP_CFG_PTR    ppp_ptr = handle;
   CHAP_DATA_PTR  chap_ptr = &ppp_ptr->CHAP_STATE;
   PCB_PTR        pcb;
   unsigned char      *outp;
   uint16_t        len;
   uint16_t        i, idlen;

   chap_ptr->SERVER_STATE = CHAP_STATE_INITIAL;
   chap_ptr->CURID = RTCS_rand() & 0xFF;

   /* Acquire a message buffer */
   /* Start CR 2207 */
   idlen = strlen(PPP_SECRET(ppp_ptr,_PPP_CHAP_LNAME));
   /* End CR 2207 */
   len = CHAP_HDR_LEN + 1 + CHAP_CHALLENGE_LEN + idlen;
   pcb = PPP_pcballoc(PPP_PROT_CHAP, len);
   if (pcb == NULL) {
      return;
   } /* Endif */

   /* Build a Challenge packet */
   outp = pcb->FRAG[0].FRAGMENT + 2;
   *outp++ = CHAP_CODE_CHALLENGE;
   *outp++ = chap_ptr->CURID;
   *outp++ = (len >> 8) & 0xFF;
   *outp++ =  len       & 0xFF;
   *outp++ = CHAP_CHALLENGE_LEN;
   for (i=0; i<CHAP_CHALLENGE_LEN; i++) {
      *outp++ = chap_ptr->MD5[i] = RTCS_rand() & 0xFF;
   } /* Endfor */
   /* Start CR 2207 */
   PPP_memcopy(PPP_SECRET(ppp_ptr,_PPP_CHAP_LNAME), outp, idlen);
   /* End CR 2207 */
   /* Send the PCB */
   PPP_send_rexmit(ppp_ptr, PPP_PROT_CHAP, pcb, 0, CHAP_timeout, chap_ptr);

#endif /* RTCSCFG_ENABLE_IP4 */
   

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  CHAP_input
* Returned Value  :  void
* Comments        :
*     Called by PPP_rx_task.  Parses a CHAP packet.
* Side effects    :
*     Consumes (sends or frees) the packet.
*
*END*-----------------------------------------------------------------*/

void CHAP_input
   (
      PCB_PTR        pcb,
            /* [IN] - CHAP packet */
      _ppp_handle    handle
            /* [IN] - the PPP state structure */
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   PPP_CFG_PTR    ppp_ptr = handle;
   CHAP_DATA_PTR  chap_ptr = &ppp_ptr->CHAP_STATE;
   PPP_SECRET_PTR secret;
   unsigned char      *inp = pcb->FRAG[0].FRAGMENT + 2;
   unsigned char      *valp = NULL;
   unsigned char      *idp = NULL;
   unsigned char          code, id;
   unsigned char          vallen = 0;
   unsigned char          idlen = 0;
   unsigned char          namelen;
   uint16_t        len;
   bool        delay = FALSE;

   /*
   ** Parse header (code, id and length).
   ** If packet too short, drop it.
   */
   if (pcb->FRAG[0].LENGTH < 2 + CHAP_HDR_LEN) {
      chap_ptr->ST_CHAP_SHORT++;
      PCB_free(pcb);
      return;
   } /* Endif */

   code = *inp++;
   id = *inp++;
   len = *inp++ << 8;
   len += *inp++;

   if ((len < CHAP_HDR_LEN) || (len > pcb->FRAG[0].LENGTH - 2)) {
      chap_ptr->ST_CHAP_SHORT++;
      PCB_free(pcb);
      return;
   } /* Endif */
   len -= CHAP_HDR_LEN;

   /*
   ** For Challenge and Response packets, also parse the
   ** value and name fields.
   */
   if ((code == CHAP_CODE_CHALLENGE) || (code == CHAP_CODE_RESPONSE)) {

      if (len < 1) {
         chap_ptr->ST_CHAP_SHORT++;
         PCB_free(pcb);
         return;
      } /* Endif */
      valp = inp;
      vallen = *valp++;
      len--;

      if (len < vallen) {
         chap_ptr->ST_CHAP_SHORT++;
         PCB_free(pcb);
         return;
      } /* Endif */
      idp = valp + vallen;
      idlen = len - vallen;

   } /* Endif */

   switch (code) {

   case CHAP_CODE_CHALLENGE:

      if (chap_ptr->CLIENT_STATE != CHAP_STATE_INITIAL) {
         chap_ptr->ST_CHAP_NOAUTH++;
         PCB_free(pcb);
         break;
      } /* Endif */

      /* OK -- Generate a Response packet */
      /* Start CR 2207 */
      secret = PPP_SECRET(ppp_ptr,_PPP_CHAP_LSECRETS);
      /* End CR 2207 */
      inp = pcb->FRAG[0].FRAGMENT + 2;
      *inp++ = CHAP_CODE_RESPONSE;
      inp++;                           /* Keep same ID */
      /* Start CR 2207 */
      namelen = strlen(PPP_SECRET(ppp_ptr,_PPP_CHAP_LNAME));
      /* End CR 2207 */
      len = CHAP_HDR_LEN + 1 + 16 + namelen;
      *inp++ = (len >> 8) & 0xFF;
      *inp++ =  len       & 0xFF;
      *inp++ = 16;
      for (;;) {
         if ((secret->PPP_ID_LENGTH == 0)
          && (secret->PPP_PW_LENGTH == 0)) {
            /* Couldn't find a secret for this peer */
            chap_ptr->ST_CHAP_NOPW++;
            PPP_MD5(inp, (uint32_t)1, &id,       /* id */
                         (uint32_t)vallen, valp, /* challenge */
                         (uint32_t)0);
            break;
         } /* Endif */
         if ((secret->PPP_ID_LENGTH == idlen)
          && (memcmp(secret->PPP_ID_PTR, idp, idlen) == 0)) {
            /* Found a secret; send response */
            PPP_MD5(inp, (uint32_t)1, &id,                /* id */
                         (uint32_t)secret->PPP_PW_LENGTH, /* secret */
                         secret->PPP_PW_PTR,
                         (uint32_t)vallen, valp,          /* challenge */
                         (uint32_t)0);
            break;
         } /* Endif */
         secret++;
      } /* Endfor */
      inp += 16;
      /* Start CR 2207 */
      PPP_memcopy(PPP_SECRET(ppp_ptr,_PPP_CHAP_LNAME), inp, namelen);
      /* End CR 2207 */
      pcb->FRAG[0].LENGTH = len + 2;
      PPP_send_one(ppp_ptr, PPP_PROT_CHAP, pcb);
      break;

   case CHAP_CODE_RESPONSE:

      switch (chap_ptr->SERVER_STATE) {
      case CHAP_STATE_INITIAL:

         /* Check the ID against our last challenge */
         if (id != chap_ptr->CURID) {
            chap_ptr->ST_CHAP_ID++;
            PCB_free(pcb);
            break;
         } /* Endif */

         /* Stop the challenge, whether or not the response is correct */
         PPP_send_stop(ppp_ptr, PPP_PROT_CHAP);

         chap_ptr->SERVER_STATE = CHAP_STATE_FAILURE;

         if (vallen != 16) {
            delay = TRUE;
            CHAP_fail(handle);

         } else {
            /* Search for name in the secrets table */
            /* Start CR 2207 */
            secret = PPP_SECRET(ppp_ptr,_PPP_CHAP_RSECRETS);
            /* End CR 2207 */
            for (;;) {
               if ((secret->PPP_ID_LENGTH == 0)
                && (secret->PPP_PW_LENGTH == 0)) {
                     /* Couldn't find a secret for this peer */
                  delay = TRUE;
                  CHAP_fail(handle);
                  break;
               } /* Endif */
               if ((secret->PPP_ID_LENGTH == idlen)
                && (memcmp(secret->PPP_ID_PTR, idp, idlen) == 0)) {
                     /* Found a secret; compute hash */
                  unsigned char digest[16];

                  PPP_MD5(digest, (uint32_t)1, &id,                /* id */
                                  (uint32_t)secret->PPP_PW_LENGTH, /* secret */
                                  secret->PPP_PW_PTR,
                                  (uint32_t)CHAP_CHALLENGE_LEN,    /* challenge */
                                  chap_ptr->MD5,
                                  (uint32_t)0);

                  if (memcmp(digest, valp, vallen) == 0) {
                     chap_ptr->SERVER_STATE = CHAP_STATE_SUCCESS;
                     CHAP_up(handle);
                  } else {
                     delay = TRUE;
                     CHAP_fail(handle);
                  } /* Endif */
                  break;
               } /* Endif */
               secret++;
            } /* Endfor */
         } /* Endif */

         /* fall through */
      case CHAP_STATE_SUCCESS:
      case CHAP_STATE_FAILURE:
         /* Build a Success or Failure reply */
         inp = pcb->FRAG[0].FRAGMENT + 2;
         *inp++ = chap_ptr->SERVER_STATE;
         inp++;                              /* Keep same ID */
         *inp++ = 0;
         *inp++ = 4;
         pcb->FRAG[0].LENGTH = 6;
         if (delay) {
            PPP_send_rexmit(ppp_ptr, PPP_PROT_CHAP, pcb, 1, CHAP_close, handle);
         } else {
            PPP_send_one(ppp_ptr, PPP_PROT_CHAP, pcb);
         } /* Endif */
         break;

      default:
         chap_ptr->ST_CHAP_NOCHAL++;
         PCB_free(pcb);
         break;
      } /* Endswitch */
      break;

   case CHAP_CODE_SUCCESS:
       /* Authentication success */
       chap_ptr->CLIENT_STATE = CHAP_STATE_SUCCESS;
       CHAP_up(handle);
   case CHAP_CODE_FAILURE:
       /* Authentication failure */
      if (chap_ptr->CLIENT_STATE != CHAP_STATE_SUCCESS)
      {
          /* Change client state when authenticate failure */
          chap_ptr->CLIENT_STATE = CHAP_STATE_FAILURE;
          if (chap_ptr->CLIENT_STATE != CHAP_STATE_INITIAL)
          {
             chap_ptr->ST_CHAP_NOAUTH++;
          }
          CHAP_fail(handle);
      } /* Endif */
      PCB_free(pcb);
      break;

   default:
      chap_ptr->ST_CHAP_CODE++;
      PCB_free(pcb);
      break;
   } /* Endswitch */
   
#else
    
    PCB_free(pcb);
    
#endif /* RTCSCFG_ENABLE_IP4 */

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  CHAP_up
* Returned Value  :  void
* Comments        :
*     Called by CHAP_input once the peer has been authenticated.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static void CHAP_up
   (
      _ppp_handle   handle
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
* Function Name   :  CHAP_fail
* Returned Value  :  void
* Comments        :
*     Called by CHAP_Input if the peer's credentials are rejected.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static void CHAP_fail
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
* Function Name   :  CHAP_close
* Returned Value  :  void
* Comments        :
*     Called by the Tx task.  Terminates the link after a
*     Failure packet is sent.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static void CHAP_close
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

#endif // RTCSCFG_ENABLE_PPP
