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
*   {Link, IP} Control Protocol Finite State Machine.
*
*
*END************************************************************************/

#include <rtcs.h>

#if  RTCSCFG_ENABLE_IP4 && RTCSCFG_ENABLE_PPP && !PLATFORM_SDK_ENABLED

/*
** fsm.c - {Link, IP} Control Protocol Finite State Machine.
**
** Copyright (c) 1989 Carnegie Mellon University.
** All rights reserved.
**
** Redistribution and use in source and binary forms are permitted
** provided that the above copyright notice and this paragraph are
** duplicated in all such forms and that any documentation,
** advertising materials, and other materials related to such
** distribution and use acknowledge that the software was developed
** by Carnegie Mellon University.  The name of the
** University may not be used to endorse or promote products derived
** from this software without specific prior written permission.
** THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
** WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <ppp.h>
#include "ppp_prv.h"

static void PPPFSM_buildheader(PPPFSM_CFG_PTR, PCB_PTR, unsigned char, bool, uint32_t);

static void PPPFSM_timeout(void *, PCB_PTR, bool);

static void PPPFSM_recvconfreq(PPPFSM_CFG_PTR, _ppp_handle);
static void PPPFSM_recvconfack(PPPFSM_CFG_PTR);
static void PPPFSM_recvconfnak(PPPFSM_CFG_PTR, bool(_CODE_PTR_)(PPPFSM_CFG_PTR));
static void PPPFSM_recvtermreq(PPPFSM_CFG_PTR);
static void PPPFSM_recvtermack(PPPFSM_CFG_PTR);
static void PPPFSM_recvcoderej(PPPFSM_CFG_PTR);

static void PPPFSM_sendconfrep(PPPFSM_CFG_PTR, uint32_t);
static void PPPFSM_sendtermack(PPPFSM_CFG_PTR, bool);
static void PPPFSM_sendcoderej(PPPFSM_CFG_PTR);

static void PPPFSM_sendconfreq(PPPFSM_CFG_PTR, PCB_PTR);
static void PPPFSM_sendtermreq(PPPFSM_CFG_PTR, PCB_PTR);
static void PPP_send_echo     (PPPFSM_CFG_PTR, PCB_PTR);

static void PPPFSM_recvechoreply(PPPFSM_CFG_PTR);

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPPFSM_init
* Returned Value  :  error code
* Comments        :
*     Called by application.  Initializes the Finite State Machine.
*
*END*-----------------------------------------------------------------*/

uint32_t PPPFSM_init
   (
      PPPFSM_CFG_PTR    fsm,
            /* [IN/OUT] - State Machine */
      _ppp_handle       handle,
            /* [IN] - the PPP state structure */
      PPPFSM_CALL_PTR   call_ptr,
            /* [IN] - Protocol-specific call table */
      void             *param
            /* [IN] - Parameter for protocol-specific calls */
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4


   PPP_memzero(fsm, sizeof(PPPFSM_CFG));
   fsm->HANDLE  = handle;
   fsm->CALL    = call_ptr;
   fsm->PRIVATE = param;
   fsm->STATE   = PPP_STATE_INITIAL;
   fsm->CURID   = RTCS_rand() & 0xFF;

   if (_lwevent_create(&(fsm->CONN_EVENT), 0)) {
      return RTCSERR_PPP_INIT_MUTEX_FAILED;
   }

   /* Initialize the mutex */
   if (PPP_mutex_init(&fsm->MUTEX)) {
      return RTCSERR_PPP_INIT_MUTEX_FAILED;
   } /* Endif */

   return PPP_OK;

#else

    return RTCSERR_IP_IS_DISABLED;

#endif /* RTCSCFG_ENABLE_IP4  */

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPPFSM_destroy
* Returned Value  :  error code
* Comments        :
*     Called by application.  Destroys the Finite State Machine.
*     Assumes the Finite State Machine has been initialized.
*
*END*-----------------------------------------------------------------*/

uint32_t PPPFSM_destroy
   (
      PPPFSM_CFG_PTR    fsm
            /* [IN/OUT] - State Machine */
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   uint32_t error = PPP_OK;

   PPP_mutex_lock(&fsm->MUTEX);
   if (fsm->STATE > PPP_STATE_CLOSED) {
      error = RTCSERR_PPP_FSM_ACTIVE;
   } else {
      fsm->STATE = PPP_STATE_INITIAL;
   } /* Endif */
   PPP_mutex_unlock(&fsm->MUTEX);
   if (!error) {
      PPP_mutex_destroy(&fsm->MUTEX);
      _lwevent_destroy(&(fsm->CONN_EVENT));
   } /* Endif */
   return error;

#else

    return RTCSERR_IP_IS_DISABLED;

#endif /* RTCSCFG_ENABLE_IP4  */

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPPFSM_lowerup
* Returned Value  :  void
* Comments        :
*    Lower layer is up.
*
*END*-----------------------------------------------------------------*/

void PPPFSM_lowerup
   (
      PPPFSM_CFG_PTR    fsm
         /* [IN/OUT] - State Machine */
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   PPP_mutex_lock(&fsm->MUTEX);

   switch(fsm->STATE) {
   case PPP_STATE_INITIAL:
      fsm->STATE = PPP_STATE_CLOSED;
      break;

   case PPP_STATE_STARTING:
      if (fsm->OPTIONS & PPP_OPT_SILENT) {
         fsm->STATE = PPP_STATE_STOPPED;
      } else {
         PPPFSM_sendconfreq(fsm, NULL);
         fsm->STATE = PPP_STATE_REQ_SENT;
      }/* Endif */
      break;
   }  /* Endswitch */

   PPP_mutex_unlock(&fsm->MUTEX);

#endif /* RTCSCFG_ENABLE_IP4 */

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPPFSM_lowerdown
* Returned Value  :  void
* Comments        :
*    Cancel all timeouts and inform upper layers.  Lower layer is
*    down.
*
*END*-----------------------------------------------------------------*/

void PPPFSM_lowerdown
   (
      PPPFSM_CFG_PTR    fsm
         /* [IN/OUT] - State Machine */
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   PPP_CFG_PTR    ppp_ptr = fsm->HANDLE;

   PPP_mutex_lock(&fsm->MUTEX);

   switch(fsm->STATE)  {
   case PPP_STATE_CLOSED:
      fsm->STATE = PPP_STATE_INITIAL;
      break;

   case PPP_STATE_STOPPED:
      fsm->STATE = PPP_STATE_STARTING;
      break;

   case PPP_STATE_CLOSING:
      fsm->STATE = PPP_STATE_INITIAL;
      PPP_send_stop(ppp_ptr, fsm->CALL->PROTOCOL);
      break;

   case PPP_STATE_STOPPING:
   case PPP_STATE_REQ_SENT:
   case PPP_STATE_ACK_RCVD:
   case PPP_STATE_ACK_SENT:
      fsm->STATE = PPP_STATE_STARTING;
      PPP_send_stop(ppp_ptr, fsm->CALL->PROTOCOL);
      break;

   case PPP_STATE_OPENED:
      if (fsm->CALL->linkdown) {
         fsm->CALL->linkdown(fsm);
      } /* Endif */
      fsm->STATE = PPP_STATE_STARTING;
      break;
   } /* Endswitch */

   PPP_mutex_unlock(&fsm->MUTEX);

#endif /* RTCSCFG_ENABLE_IP4 */

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPPFSM_open
* Returned Value  :  void
* Comments        :
*     Called by application.  Begin negotiating the link.
*
*END*-----------------------------------------------------------------*/

void PPPFSM_open
   (
      PPPFSM_CFG_PTR    fsm,
            /* [IN/OUT] - State Machine */
      uint32_t           options
            /* [IN] - Startup options */
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   PPP_mutex_lock(&fsm->MUTEX);
   fsm->OPTIONS = options;

   switch (fsm->STATE) {
   case PPP_STATE_INITIAL:
      fsm->STATE = PPP_STATE_STARTING;
      break;

   case PPP_STATE_CLOSED:
      if (options & PPP_OPT_SILENT) {
         fsm->STATE = PPP_STATE_STOPPED;
      } else {
         /* Send an initial Configure-Request */
         PPPFSM_sendconfreq(fsm, NULL);
         fsm->STATE = PPP_STATE_REQ_SENT;
      } /* Endif */
      break;

   case PPP_STATE_CLOSING:
      fsm->STATE = PPP_STATE_STOPPING;
      /* fall through */
   case PPP_STATE_STOPPING:
   case PPP_STATE_STOPPED:
      if (options & PPP_OPT_RESTART) {
         PPPFSM_sendconfreq(fsm, NULL);
         fsm->STATE = PPP_STATE_REQ_SENT;
      } /* Endif */
      break;

   case PPP_STATE_OPENED:
      if (options & PPP_OPT_RESTART) {
         if (fsm->CALL->linkdown) {
            fsm->CALL->linkdown(fsm);
         } /* Endif */
         PPPFSM_sendconfreq(fsm, NULL);
         fsm->STATE = PPP_STATE_REQ_SENT;
      } /* Endif */
      break;
   } /* Endswitch */

   PPP_mutex_unlock(&fsm->MUTEX);

#endif /* RTCSCFG_ENABLE_IP4 */

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPPFSM_close
* Returned Value  :  void
* Comments        :
*     Called by application.  Begin closing the link.
*
*END*-----------------------------------------------------------------*/

void PPPFSM_close
   (
      PPPFSM_CFG_PTR    fsm
            /* [IN/OUT] - State Machine */
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   PPP_mutex_lock(&fsm->MUTEX);
   switch (fsm->STATE) {
   case PPP_STATE_STARTING:
      fsm->STATE = PPP_STATE_INITIAL;
      break;
   case PPP_STATE_STOPPED:
      fsm->STATE = PPP_STATE_CLOSED;
      break;
   case PPP_STATE_STOPPING:
      fsm->STATE = PPP_STATE_CLOSING;
      break;

   case PPP_STATE_OPENED:
      if (fsm->CALL->linkdown) {
         fsm->CALL->linkdown(fsm);
      } /* Endif */
      /* fall through */
   case PPP_STATE_REQ_SENT:
   case PPP_STATE_ACK_RCVD:
   case PPP_STATE_ACK_SENT:
      PPPFSM_sendtermreq(fsm, NULL);
      fsm->STATE = PPP_STATE_CLOSING;
      break;
   } /* Endswitch */
   PPP_mutex_unlock(&fsm->MUTEX);

#endif /* RTCSCFG_ENABLE_IP4 */

} /* Endbody */

#if RTCSCFG_ENABLE_IP4
static void PPP_send_echo(PPPFSM_CFG_PTR fsm, PCB_PTR packet)
{
    PPP_CFG_PTR    ppp_ptr = fsm->HANDLE;

    /* Allocate a packet if necessary */
    if (!packet) {
        packet = PPP_pcballoc(fsm->CALL->PROTOCOL, CP_HDR_LEN);
        if (!packet) {
            ++fsm->ST_CP_NOBUFS;    /* Nonfatal error */
            return;
        } /* Endif */
    } /* Endif */

    fsm->REQID = fsm->CURID;
    PPPFSM_buildheader(fsm, packet, CP_CODE_ECHO_REQ, FALSE, 8);

    PPP_send_one(ppp_ptr, fsm->CALL->PROTOCOL, packet);
}


uint32_t PPP_check_connection(_ppp_handle handle)
{
    PPP_CFG_PTR         ppp_ptr = handle;
    PPPFSM_CFG_PTR      fsm = &(ppp_ptr->LCP_FSM);
    _mqx_uint           err = MQX_OK;
    int i = 0;

    while (i < PPP_CHECK_CONN_RETRY)
    {
        /* Send a LCP_echo_request packet */
        PPP_send_echo(&(ppp_ptr->LCP_FSM), NULL);

        /* Wait a LCP_echo_reply package or timeout */
        err = _lwevent_wait_ticks(&(fsm->CONN_EVENT), 0x01, TRUE, PPP_CHECK_CONN_TIMEOUT / BSP_ALARM_RESOLUTION);

        _lwevent_clear(&(fsm->CONN_EVENT), 0x01);

        if (err == MQX_OK)
            return PPP_CONNECTED;
        i++;
    }
    return PPP_DISCONNECTED;
}

static void PPPFSM_recvechoreply
   (
      PPPFSM_CFG_PTR    fsm
   )
{
    /* Verify the packet */
    if (fsm->ID != fsm->REQID)
    {
        PCB_free(fsm->PACKET);
        return;
    }

    _lwevent_set(&(fsm->CONN_EVENT), 0x01);
    PCB_free(fsm->PACKET);
}
#endif


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPPFSM_timeout
* Returned Value  :  void
* Comments        :
*     Called by PPP_tx_task.  Timeout expired.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static void PPPFSM_timeout
   (
      void          *fsm_ptr,
            /* [IN/OUT] - State Machine */
      PCB_PTR        packet,
            /* [IN] - expired packet */
      bool        hard
            /* [IN] - TRUE if this is a hard timeout (TO- event) */
   )
{ /* Body */
   PPPFSM_CFG_PTR    fsm = fsm_ptr;

   PPP_mutex_lock(&fsm->MUTEX);
   switch (fsm->STATE) {
   case PPP_STATE_CLOSING:
   case PPP_STATE_STOPPING:
      if (hard) {
         /* We've waited for an ack long enough */
         fsm->STATE -= 2;  /* Stopping/Closing -> Stopped/Closed */
      } /* Endif */
      break;

   case PPP_STATE_REQ_SENT:
   case PPP_STATE_ACK_RCVD:
   case PPP_STATE_ACK_SENT:
      if (hard) {
         fsm->STATE = PPP_STATE_STOPPED;
      } else if (fsm->STATE == PPP_STATE_ACK_RCVD) {
         fsm->STATE = PPP_STATE_REQ_SENT;
      } /* Endif */
      break;
   } /* Endswitch */
   PPP_mutex_unlock(&fsm->MUTEX);

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPPFSM_input
* Returned Value  :  void
* Comments        :
*     Called by PPP_rx_task.  Parses a xCP packet.
* Side effects    :
*     Consumes (sends or frees) the packet.
*
*END*-----------------------------------------------------------------*/

void PPPFSM_input
   (
      PCB_PTR        pcb,
            /* [IN] - xCP packet */
      void          *fsm_data
            /* [IN/OUT] - State Machine */
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   PPPFSM_CFG_PTR fsm = fsm_data;
   uint32_t    *stat;
   unsigned char      *inp;
   uint16_t        pcblen, len;

   /*
   ** Parse header (code, id and length).
   ** If packet is too short, drop it.
   */

   stat = NULL;
   fsm->PACKET = pcb;
   inp    = pcb->FRAG[0].FRAGMENT + 2; /* skip the protocol field */
   pcblen = pcb->FRAG[0].LENGTH - 2;

   PPP_mutex_lock(&fsm->MUTEX);
   if (pcblen < CP_HDR_LEN) {
      stat = &fsm->ST_CP_SHORT;
   } else {
      fsm->CODE = *inp++;
      fsm->ID = *inp++;
      len = *inp++ << 8;
      len += *inp++;
      if (len < CP_HDR_LEN) {
         stat = &fsm->ST_CP_SHORT;
      } else if (len > pcblen) {
         stat = &fsm->ST_CP_SHORT;
      } else if (fsm->STATE < PPP_STATE_CLOSED) {
         stat = &fsm->ST_CP_DOWN;
      } /* Endif */
   } /* Endif */

   if (stat) {
      PPP_mutex_unlock(&fsm->MUTEX);
      ++*stat;
      PCB_free(pcb);
      return;
   } /* Endif */

   fsm->LENGTH = len - CP_HDR_LEN;
   fsm->DATA   = inp;
   /*
   ** Action depends on code.
   */
   switch (fsm->CODE) {
   case CP_CODE_CONF_REQ:
      PPPFSM_recvconfreq(fsm, fsm->HANDLE);
      break;

   case CP_CODE_CONF_ACK:
      PPPFSM_recvconfack(fsm);
      break;

   case CP_CODE_CONF_NAK:
      PPPFSM_recvconfnak(fsm, fsm->CALL->recvnak);
      break;

   case CP_CODE_CONF_REJ:
      PPPFSM_recvconfnak(fsm, fsm->CALL->recvrej);
      break;

   case CP_CODE_TERM_REQ:
      PPPFSM_recvtermreq(fsm);
      break;

   case CP_CODE_TERM_ACK:
      PPPFSM_recvtermack(fsm);
      break;

   case CP_CODE_CODE_REJ:
      PPPFSM_recvcoderej(fsm);
      break;

   case CP_CODE_ECHO_REPLY:
      PPPFSM_recvechoreply(fsm);
      break;

   default:
      if (!fsm->CALL->recvcode || !fsm->CALL->recvcode(fsm)) {
         PPPFSM_sendcoderej(fsm);
      } /* Endif */
      break;
   } /* Endswitch */
   PPP_mutex_unlock(&fsm->MUTEX);

#else

    PCB_free(pcb);

#endif /* RTCSCFG_ENABLE_IP4   */

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPPFSM_buildheader
* Returned Value  :  void
* Comments        :
*     Called by PPPFSM_send*.  Builds the CP header before sending
*     a packet.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static void PPPFSM_buildheader
   (
      PPPFSM_CFG_PTR    fsm,
            /* [IN] - State Machine */
      PCB_PTR           pcb,
            /* [IN] - packet */
      unsigned char             code,
            /* [IN] - packet code, 0 means use fsm->CODE */
      bool           keepid,
            /* [IN] - packet id, FALSE mean use fsm->ID */
      uint32_t           size
            /* [IN] - length of packet data */
   )
{ /* Body */
   unsigned char *outp = pcb->FRAG[0].FRAGMENT + 2;

   if (!code) code = fsm->CODE;
   *outp++ = code;

   if (!keepid) {
      *outp = fsm->CURID;
      fsm->CURID = (fsm->CURID + 1) & 0xFF;
   } /* Endif */
   outp++;

   size += CP_HDR_LEN;
   *outp++ = (size >> 8) & 0xFF;
   *outp++ =  size       & 0xFF;

   pcb->FRAG[0].LENGTH = size + 2;

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPPFSM_recvconfreq
* Returned Value  :  void
* Comments        :
*     Called by PPPFSM_Input.  Parses a Configure-Request packet.
* Side effects    :
*     Assumes fsm->PACKET is valid, and consumes (sends or frees) it.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static void PPPFSM_recvconfreq
   (
      PPPFSM_CFG_PTR    fsm,
            /* [IN/OUT] - State Machine */
      _ppp_handle       handle
            /* [IN] - the PPP state structure */
   )
{ /* Body */
   PPP_CFG_PTR    ppp_ptr = handle;
   uint32_t        size;

   switch (fsm->STATE) {
   case PPP_STATE_CLOSED:
      PPPFSM_sendtermack(fsm, FALSE);
      return;
   case PPP_STATE_CLOSING:
   case PPP_STATE_STOPPING:
      PCB_free(fsm->PACKET);
      return;

   case PPP_STATE_OPENED:
      /* Go down and restart negotiation */
      if (fsm->CALL->linkdown) {
         fsm->CALL->linkdown(fsm);
      } /* Endif */
      PPPFSM_sendconfreq(fsm, NULL);
      fsm->STATE = PPP_STATE_REQ_SENT;
      break;

   case PPP_STATE_STOPPED:
      /* Negotiation started by our peer */
      PPPFSM_sendconfreq(fsm, NULL);
      fsm->STATE = PPP_STATE_REQ_SENT;
      break;
   } /* Endswitch */

   /*
   ** Pass the requested configuration options
   ** to protocol-specific code for checking.
   **
   ** !! Packet consumed here !!
   */
   if (fsm->CALL->recvreq) {    /* Check CI */
      size = fsm->CALL->recvreq(fsm, fsm->NAKS >= _PPP_MAX_CONF_NAKS);
      PPPFSM_buildheader(fsm, fsm->PACKET, 0, TRUE, size);
      PPP_send_one(ppp_ptr, fsm->CALL->PROTOCOL, fsm->PACKET);
   } else if (fsm->LENGTH) {
      PPPFSM_sendconfrep(fsm, CP_CODE_CONF_REJ);
   } else {
      PPPFSM_sendconfrep(fsm, CP_CODE_CONF_ACK);
   } /* Endif */

   if (fsm->CODE == CP_CODE_CONF_ACK) {
      if (fsm->STATE == PPP_STATE_ACK_RCVD) {
         /* Inform upper layers */
         if (fsm->CALL->linkup) {
            if (!fsm->CALL->linkup(fsm)) {
               PPPFSM_sendtermreq(fsm, NULL);
               fsm->STATE = PPP_STATE_CLOSING;
               return;
            } /* Endif */
         } /* Endif */
         fsm->STATE = PPP_STATE_OPENED;
         PPP_send_stop(ppp_ptr, fsm->CALL->PROTOCOL);
      } else {
         fsm->STATE = PPP_STATE_ACK_SENT;
      } /* Endif */
      fsm->NAKS = 0;

   } else { /* We sent a NAK or a REJ */
      if (fsm->STATE != PPP_STATE_ACK_RCVD) {
         fsm->STATE = PPP_STATE_REQ_SENT;
      } /* Endif */
      if (fsm->CODE == CP_CODE_CONF_NAK ) {
         ++fsm->NAKS;
      } /* Endif */
   } /* Endif */
} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPPFSM_recvconfack
* Returned Value  :  void
* Comments        :
*     Called by PPPFSM_input.  Parses a Configure-Ack packet.
* Side effects    :
*     Assumes fsm->PACKET is valid, and consumes (sends or frees) it.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static void PPPFSM_recvconfack
   (
      PPPFSM_CFG_PTR    fsm
            /* [IN/OUT] - State Machine */
   )
{ /* Body */

   /* Verify the packet */
   if (fsm->ID != fsm->REQID) {
      PCB_free(fsm->PACKET);
      return;
   } else if (fsm->CALL->recvack) {
      if (!fsm->CALL->recvack(fsm)) {
         ++fsm->ST_CP_BAD_ACK;
         PCB_free(fsm->PACKET);
         return;
      } /* Endif */
   } else if (fsm->LENGTH) {
      ++fsm->ST_CP_BAD_ACK;
      PCB_free(fsm->PACKET);
      return;
   } /* Endif */

   /*
   ** !! Packet consumed here !!
   */
   switch (fsm->STATE) {
   case PPP_STATE_CLOSED:
   case PPP_STATE_STOPPED:

      PPPFSM_sendtermack(fsm, FALSE);
      break;

   case PPP_STATE_REQ_SENT:
      /* Restart retransmissions */
      fsm->STATE = PPP_STATE_ACK_RCVD;
      PCB_free(fsm->PACKET);
      PPP_send_restart(fsm->HANDLE, fsm->CALL->PROTOCOL);
      break;

   case PPP_STATE_ACK_RCVD:
      PPPFSM_sendconfreq(fsm, fsm->PACKET);
      fsm->STATE = PPP_STATE_REQ_SENT;
      break;

   case PPP_STATE_ACK_SENT:
      /* Inform upper layers */
      if (fsm->CALL->linkup) {
         if (!fsm->CALL->linkup(fsm)) {
            PPPFSM_sendtermreq(fsm, fsm->PACKET);
            fsm->STATE = PPP_STATE_CLOSING;
            return;
         } /* Endif */
      } /* Endif */
      fsm->STATE = PPP_STATE_OPENED;
      PCB_free(fsm->PACKET);
      PPP_send_stop(fsm->HANDLE, fsm->CALL->PROTOCOL);
      break;

   case PPP_STATE_OPENED:
      /* Restart negotiation */
      if (fsm->CALL->linkdown) {
         fsm->CALL->linkdown(fsm);
      } /* Endif */
      PPPFSM_sendconfreq(fsm, fsm->PACKET);
      fsm->STATE = PPP_STATE_REQ_SENT;
      break;

   default:
      PCB_free(fsm->PACKET);
      break;
   } /* Endswitch */

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPPFSM_recvconfnak
* Returned Value  :  void
* Comments        :
*     Called by PPPFSM_input.  Parses a Configure-Nak or
*     Configure-Reject packet.
* Side effects    :
*     Assumes fsm->PACKET is valid, and consumes (sends or frees) it.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static void PPPFSM_recvconfnak
   (
      PPPFSM_CFG_PTR    fsm,
            /* [IN/OUT] - State Machine */
      bool (_CODE_PTR_ call)(PPPFSM_CFG_PTR)
            /* [IN] - Protocol-specific callback for this packet */
   )
{ /* Body */

   /* Verify the packet */
   if (fsm->ID != fsm->REQID) {
      PCB_free(fsm->PACKET);
      return;

   } else if (!call || !call(fsm)) {
      if (fsm->CODE == CP_CODE_CONF_NAK) {
         ++fsm->ST_CP_BAD_NAK;
      } else {
         ++fsm->ST_CP_BAD_REJ;
      } /* Endif */
      PCB_free(fsm->PACKET);
      return;
   } /* Endif */

   /*
   ** !! Packet consumed here !!
   */
   switch (fsm->STATE) {
   case PPP_STATE_CLOSED:
   case PPP_STATE_STOPPED:
      PPPFSM_sendtermack(fsm, FALSE);
      break;

   case PPP_STATE_REQ_SENT:
   case PPP_STATE_ACK_SENT:
      /* They didn't agree -- try another request */
      PPPFSM_sendconfreq(fsm, fsm->PACKET);
      break;

   case PPP_STATE_ACK_RCVD:
      PPPFSM_sendconfreq(fsm, fsm->PACKET);
      fsm->STATE = PPP_STATE_REQ_SENT;
      break;

   case PPP_STATE_OPENED:
      /* Restart negotiation */
      if (fsm->CALL->linkdown) {
         fsm->CALL->linkdown(fsm);
      } /* Endif */
      PPPFSM_sendconfreq(fsm, fsm->PACKET);
      fsm->STATE = PPP_STATE_REQ_SENT;
      break;

   default:
      PCB_free(fsm->PACKET);
      break;
   } /* Endswitch */

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPPFSM_recvtermreq
* Returned Value  :  void
* Comments        :
*     Called by PPPFSM_input.  Parses a Terminate-Request packet.
* Side effects    :
*     Assumes fsm->PACKET is valid, and consumes (sends) it.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static void PPPFSM_recvtermreq
   (
      PPPFSM_CFG_PTR    fsm
            /* [IN/OUT] - State Machine */
   )
{ /* Body */

   /*
   ** !! Packet consumed here !!
   */
   switch (fsm->STATE) {
   case PPP_STATE_ACK_RCVD:
   case PPP_STATE_ACK_SENT:
      fsm->STATE = PPP_STATE_REQ_SENT;
      PPPFSM_sendtermack(fsm, FALSE);
      break;

   case PPP_STATE_OPENED:
      /* Restart negotiation */
      if (fsm->CALL->linkdown) {
         fsm->CALL->linkdown(fsm);
      } /* Endif */
      PPPFSM_sendtermack(fsm, TRUE);
      fsm->STATE = PPP_STATE_STOPPING;
      break;

   default:
      PPPFSM_sendtermack(fsm, FALSE);
      break;
   } /* Endswitch */

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPPFSM_recvtermack
* Returned Value  :  void
* Comments        :
*     Called by PPPFSM_input.  Parses a Terminate-Ack packet.
* Side effects    :
*     Assumes fsm->PACKET is valid, and consumes (sends or frees) it.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static void PPPFSM_recvtermack
   (
      PPPFSM_CFG_PTR    fsm
            /* [IN/OUT] - State Machine */
   )
{ /* Body */

   switch (fsm->STATE) {
   case PPP_STATE_CLOSING:
   case PPP_STATE_STOPPING:
      PCB_free(fsm->PACKET);
      PPP_send_stop(fsm->HANDLE, fsm->CALL->PROTOCOL);
      fsm->STATE -= 2;  /* Stopping/Closing -> Stopped/Closed */
      break;

   case PPP_STATE_ACK_RCVD:
      PCB_free(fsm->PACKET);
      fsm->STATE = PPP_STATE_REQ_SENT;
      break;

   case PPP_STATE_OPENED:
      if (fsm->CALL->linkdown) {
         fsm->CALL->linkdown(fsm);
      } /* Endif */
      PPPFSM_sendconfreq(fsm, fsm->PACKET);
      fsm->STATE = PPP_STATE_REQ_SENT;
      break;

   default:
      PCB_free(fsm->PACKET);
      break;
   } /* Endswitch */

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPPFSM_recvcoderej
* Returned Value  :  void
* Comments        :
*     Called by PPPFSM_input.  Parses a Code-Reject packet.
* Side effects    :
*     Assumes fsm->PACKET is valid, and consumes (sends or frees) it.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static void PPPFSM_recvcoderej
   (
      PPPFSM_CFG_PTR    fsm
            /* [IN/OUT] - State Machine */
   )
{ /* Body */

   if (fsm->LENGTH < CP_HDR_LEN) {
      ++fsm->ST_CP_SHORT;
      PCB_free(fsm->PACKET);
      return;
   } /* Endif */

   /*
   ** Check if Code-Reject is catastrophic
   **
   ** !! Packet consumed here !!
   */
   if (!fsm->CALL->testcode || !fsm->CALL->testcode(fsm)) {
      if (fsm->STATE == PPP_STATE_ACK_RCVD) {
         fsm->STATE = PPP_STATE_REQ_SENT;
      } /* Endif */
      PCB_free(fsm->PACKET);

   } else {

      switch (fsm->STATE) {
      case PPP_STATE_CLOSING:
         PCB_free(fsm->PACKET);
         PPP_send_stop(fsm->HANDLE, fsm->CALL->PROTOCOL);
         fsm->STATE = PPP_STATE_CLOSED;
         break;

      case PPP_STATE_STOPPING:
      case PPP_STATE_REQ_SENT:
      case PPP_STATE_ACK_RCVD:
      case PPP_STATE_ACK_SENT:
         PCB_free(fsm->PACKET);
         PPP_send_stop(fsm->HANDLE, fsm->CALL->PROTOCOL);
         fsm->STATE = PPP_STATE_STOPPED;
         break;

      case PPP_STATE_OPENED:
         if (fsm->CALL->linkdown) {
            fsm->CALL->linkdown(fsm);
         } /* Endif */
         PPPFSM_sendtermreq(fsm, fsm->PACKET);
         fsm->STATE = PPP_STATE_STOPPING;
         break;

      default:
         PCB_free(fsm->PACKET);
         break;
      } /* Endswitch */
   } /* Endif */

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPPFSM_sendconfrep
* Returned Value  :  void
* Comments        :
*     Called by PPPFSM_input.  Builds a 'code' packet from
*     the packet in fsm, and sends it.
* Side effects    :
*     Assumes fsm->PACKET is valid, and consumes (sends) it.
*     Sets fsm->CODE to 'code'.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static void PPPFSM_sendconfrep
   (
      PPPFSM_CFG_PTR    fsm,
            /* [IN/OUT] - State Machine */
      uint32_t           code
            /* [IN] - packet code for reply */
   )
{ /* Body */

   fsm->CODE = fsm->PACKET->FRAG[0].FRAGMENT[2] = code;

   PPP_send_one(fsm->HANDLE, fsm->CALL->PROTOCOL, fsm->PACKET);

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPPFSM_sendtermack
* Returned Value  :  void
* Comments        :
*     Called by PPPFSM_input.  Builds a Terminate-Ack packet from
*     the packet in fsm, and sends it.
* Side effects    :
*     Assumes fsm->PACKET is valid, and consumes (sends) it.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static void PPPFSM_sendtermack
   (
      PPPFSM_CFG_PTR    fsm,
            /* [IN/OUT] - State Machine */
      bool           delay
            /* [IN] - whether or not to Zero-Restart-Counter */
   )
{ /* Body */

   PPPFSM_buildheader(fsm, fsm->PACKET, CP_CODE_TERM_ACK, TRUE, 0);

   if (delay) {
      PPP_send_rexmit(fsm->HANDLE, fsm->CALL->PROTOCOL, fsm->PACKET,
                      1, PPPFSM_timeout, fsm);
   } else {
      PPP_send_one(fsm->HANDLE, fsm->CALL->PROTOCOL, fsm->PACKET);
   } /* Endif */

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPPFSM_sendcoderej
* Returned Value  :  void
* Comments        :
*     Called by PPPFSM_input.  Builds a Code-Reject packet from
*     the packet in fsm, and sends it.
* Side effects    :
*     Assumes fsm->PACKET is valid, and consumes (sends) it.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static void PPPFSM_sendcoderej
   (
      PPPFSM_CFG_PTR    fsm
            /* [IN/OUT] - State Machine */
   )
{ /* Body */
   PPP_CFG_PTR ppp_ptr = fsm->HANDLE;
   unsigned char   *packet, *src, *dst;
   uint16_t     length = fsm->PACKET->FRAG[0].LENGTH - 2;
   uint16_t     size = ppp_ptr->SEND_OPTIONS->MRU - CP_HDR_LEN;

   /* Truncate the packet if necessary, so as not to exceed the MTU */
   if (length > size) {
      length = size;
   } /* Endif */

   /* Make room for a Code-Reject header */
   size = length;

   /* Copy the received packet to the data portion of the Code-Reject */
   packet = fsm->PACKET->FRAG[0].FRAGMENT + 2;
   src = packet + length;
   dst = src + CP_HDR_LEN;
   while (length--) *--dst = *--src;

   /* Build a Code-Reject header */
   PPPFSM_buildheader(fsm, fsm->PACKET, CP_CODE_CODE_REJ, FALSE, size);

   PPP_send_one(ppp_ptr, fsm->CALL->PROTOCOL, fsm->PACKET);

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPPFSM_sendconfreq
* Returned Value  :  void
* Comments        :
*     Called by PPPFSM_input.  Builds a Configure-Request packet
*     and sends it.
* Side effects    :
*     Starts the retransmission timer with the new
*        Configure-Request packet.
*     If 'packet' is non-NULL, assumes it's valid, and consumes
*        (sends) it.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static void PPPFSM_sendconfreq
   (
      PPPFSM_CFG_PTR    fsm,
            /* [IN/OUT] - State Machine */
      PCB_PTR           packet
            /* [IN] - free packet */
   )
{ /* Body */
   PPP_CFG_PTR    ppp_ptr = fsm->HANDLE;
   unsigned char      *outp;
   uint32_t        size;

   if (fsm->STATE <= PPP_STATE_STOPPING || fsm->STATE >= PPP_STATE_OPENED) {
      /* Not currently negotiating -- reset options */
      if (fsm->CALL->resetreq) {
         fsm->CALL->resetreq(fsm);
      } /* Endif */
      fsm->NAKS = 0;
   } /* Endif */

   /* Stop the retransmission timer if necessary */
   if (fsm->STATE > PPP_STATE_STOPPED && fsm->STATE < PPP_STATE_OPENED) {
      PPP_send_stop(ppp_ptr, fsm->CALL->PROTOCOL);
   } /* Endif */

   /* Allocate a packet if necessary */
   if (!packet) {
      packet = PPP_pcballoc(fsm->CALL->PROTOCOL, ppp_ptr->SEND_OPTIONS->MRU);
      if (!packet) {
         ++fsm->ST_CP_NOBUFS;    /* Nonfatal error */
         return;
      } /* Endif */
   } /* Endif */

   /* Build the request packet */
   outp = packet->FRAG[0].FRAGMENT + 2;
   size = packet->FRAG[0].LENGTH - 2 - CP_HDR_LEN;

   if (fsm->CALL->buildreq) {
      size = fsm->CALL->buildreq(fsm, outp + CP_HDR_LEN, size);
   } /* Endif */

   fsm->REQID = fsm->CURID;
   PPPFSM_buildheader(fsm, packet, CP_CODE_CONF_REQ, FALSE, size);

   /* Send the request */
   PPP_send_rexmit(ppp_ptr, fsm->CALL->PROTOCOL, packet,
                   _PPP_MAX_CONF_RETRIES, PPPFSM_timeout, fsm);

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPPFSM_sendtermreq
* Returned Value  :  void
* Comments        :
*     Called by PPPFSM_input.  Builds a Terminate-Request packet from
*     the packet in fsm, and sends it.
* Side effects    :
*     Starts the retransmission timer with the Terminate-Request
*        packet.
*     If 'packet' is non-NULL, assumes it's valid, and consumes
*        (sends) it.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static void PPPFSM_sendtermreq
   (
      PPPFSM_CFG_PTR    fsm,
            /* [IN/OUT] - State Machine */
      PCB_PTR           packet
            /* [IN] - free packet */
   )
{ /* Body */
   PPP_CFG_PTR    ppp_ptr = fsm->HANDLE;

   /* Stop the retransmission timer if necessary */
   if (fsm->STATE > PPP_STATE_STOPPED && fsm->STATE < PPP_STATE_OPENED) {
      PPP_send_stop(ppp_ptr, fsm->CALL->PROTOCOL);
   } /* Endif */

   /* Allocate a packet if necessary */
   if (!packet) {
      packet = PPP_pcballoc(fsm->CALL->PROTOCOL, CP_HDR_LEN);
      if (!packet) {
         ++fsm->ST_CP_NOBUFS;    /* Nonfatal error */
         return;
      } /* Endif */
   } /* Endif */

   PPPFSM_buildheader(fsm, packet, CP_CODE_TERM_REQ, FALSE, 0);

   PPP_send_rexmit(ppp_ptr, fsm->CALL->PROTOCOL, packet,
                   _PPP_MAX_TERM_RETRIES, PPPFSM_timeout, fsm);

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

#endif // RTCSCFG_ENABLE_PPP

