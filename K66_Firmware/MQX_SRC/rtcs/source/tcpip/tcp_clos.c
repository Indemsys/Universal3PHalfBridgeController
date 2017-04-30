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
*   A TCB can be released (via TCP_Process_release) only after the upper layer
*   has provided a tcpCLOSE (or tcpABORT) command or if a tcpOPEN
*   command has failed.  In all other cases the TCB may be moved to
*   CLOSED state (via TCP_Close_TCB), but the TCB cannot be released.
*   The cases where the upper layer has closed are not easy to see (because
*   in the normal case the connection stays up until the other side is
*   also ready to close), but this occurs while in states:
*   FINWAIT_1, FINWAIT_2, CLOSING, TIME_WAIT, and LAST_ACK.
*
*
*END************************************************************************/

#include <rtcs.h>
#include "rtcs_prv.h"
#include "ticker.h"     /* for timer definitions */
#include "tcpip.h"      /* for error codes */
#include "tcp_prv.h"    /* for TCP internal definitions */
#include "socket.h"     /* for TCP internal definitions */

static int32_t tcp_process_abort(struct tcb_struct *tcb, struct tcp_cfg_struct *tcp_cfg);

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : TCP_Free_TCB
* Returned Values : None.
* Comments        :
*
*  Deallocate TCB; must only be done when TCB has been closed and released.
*
*END*-----------------------------------------------------------------*/

void TCP_Free_TCB
   (
      TCB_STRUCT_PTR       tcb,     /* IN/OUT - TCP context */
      TCP_CFG_STRUCT_PTR   tcp_cfg  /* IN/OUT - TCP layer data */
   )
{ /* Body */
   TCB_STRUCT_PTR    tmp;
   
   if(NULL==tcb) return;

   /*
   ** Free everything which wasn't freed in TCP_Close_TCB() ...
   */
   TCP_Truncate_receive_chunks(tcb, tcp_cfg, tcb->rcvlen); /* delete all chunk nodes */
   if ( tcb->rcvringbuf != NULL ) {

      _mem_free(tcb->rcvringbuf);
      tcb->rcvringbuf = NULL;

   } /* Endif */

   /*
   ** Deallocate TCB itself
   */
   if ( tcb != NULL ) {

      if ( tcb == tcp_cfg->TCBhead ) {
         tcp_cfg->TCBhead = tcb->next;
      } else {
         tmp = tcp_cfg->TCBhead;
         while ( tmp != NULL && tmp->next != tcb ) {
            tmp = tmp->next;
         } /* Endwhile */
         if ( tmp != NULL ) {
            tmp->next = tcb->next;
         } /* Endif */
      } /* Endif */
   } /* Endif */

   if (tcb->SOCKET) {
      ((SOCKET_STRUCT_PTR)tcb->SOCKET)->TCB_PTR = NULL;
   } /* Endif */
   tcb->VALID = TCB_INVALID_ID;

   tcp_cfg->CONN_COUNT--;
#if RTCSCFG_TCP_MAX_HALF_OPEN
    TCB_HALF_OPEN_DROP
#endif
   
   _mem_free(tcb);
   
   /* double check OPENS list. 
    * if the tcb is in OPENS list now, just return with error,
    * so that application is aware. 
    * normally it should not happen, as accept()/connect() request
    * if removed from OPENS list during ESTABLISHED and FINWAIT_1 states.
    */
    {
       TCP_PARM_PTR  req_ptr;     /* upper layer request */
       TCP_PARM_PTR  prev_ptr;    /* previous upper layer request */
       
       prev_ptr = NULL;
       req_ptr = tcp_cfg->OPENS;
       while (req_ptr != NULL &&
              tcb != req_ptr->TCB_PTR) 
       {
          prev_ptr = req_ptr;
          req_ptr = req_ptr->NEXT;
       }
       
       if (req_ptr != NULL) 
       {
          if (prev_ptr == NULL) 
          {
             tcp_cfg->OPENS = req_ptr->NEXT;
          } 
          else 
          {
             prev_ptr->NEXT = req_ptr->NEXT;
          } /* Endif */
          req_ptr->NEXT = NULL;
          req_ptr->TCB_PTR = tcb;
          RTCSCMD_complete(req_ptr, RTCSERR_TCP_CONN_RLSD);
       }
    }
} /* Endbody */



/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : TCP_Process_release
* Returned Values : None.
* Comments        :
*
*  Release the TCB so that it can no longer be used.  Called only
*     when in CLOSED state with no more data to receive.
*
*END*-----------------------------------------------------------------*/

void TCP_Process_release
   (
      TCB_STRUCT_PTR          tcb,     /* IN     - TCP context */
      TCP_CFG_STRUCT_PTR      tcp_cfg  /* IN/OUT - TCP layer data */
   )
{ /* Body */
   TCB_STRUCT_PTR      *search_ptr = &tcp_cfg->NOSOCK_TCB;
   TCB_STRUCT_PTR       temp_tcb;

   /*
   ** 1. Check if the TCB was spawned by a listening TCB, and if so remove it from
   **    the list of spawned listening TCBs.
   ** 2. If the TCB that is begin released is a listening TCB and it spawned
   **    other TCBs, they must also be freed.
   */
   while (*search_ptr) {
      if (*search_ptr == tcb) {
         /* The TCB we are releasing is a child of a listening TCB */
         *search_ptr = (*search_ptr)->NOSOCK_NEXT;
         break;
      } /* Endif */

      if (tcb && (*search_ptr)->LISTEN_TCB == tcb) {
         /*
         ** The TCB is a listening TCB and we have found one of its children.
         ** Delete the child. Recursion is only one level deep, because the
         ** child is not a listening TCB and has not spawned any TCBs of its own
         */
         temp_tcb = *search_ptr;
         *search_ptr = temp_tcb->NOSOCK_NEXT;
         TCP_Close_TCB(temp_tcb, RTCS_OK, tcp_cfg);
         TCP_Process_release(temp_tcb, tcp_cfg);
         continue;
      } /* Endif */

      search_ptr = &(*search_ptr)->NOSOCK_NEXT;
   } /* Endwhile */

   TCP_Free_TCB(tcb, tcp_cfg);

} /* Endbody */



/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : TCP_Close_TCB
* Returned Values : None.
* Comments        :
*
*  Enter CLOSED state and terminate all requests with error code errnum.
*
*  Will be followed by a TCP_Process_release at some time, when there
*     is no more data to receive.  Note that this call will prevent
*     and further data from being queued (and so should immediately
*     be followed by a check for ability to release).
*
*  This is the only routine which puts a TCB in the CLOSED state.
*
*END*-----------------------------------------------------------------*/

void TCP_Close_TCB
   (
      TCB_STRUCT_PTR       tcb,     /* IN/OUT - TCP context */
      int32_t               errnum,  /* IN     - error code to return */
      TCP_CFG_STRUCT_PTR   tcp_cfg  /* IN/OUT - TCP layer data */
   )
{ /* Body */
   TCP_PARM_PTR      req;
   SbufNode         *buf;
   bool           first = TRUE;  /* for returning Sends */
   uint32_t       state = tcb->state;
   
   /*
   ** This prevents new packets from being queued on this TCB;
   **   however there might still be some queued (but only if
   **   there are no outstanding Receive requests).
   */
   tcb->state = CLOSED;

   /*
   ** Cancel timers...
   */
   (void)TCP_Timer_stop(&tcb->sndxmittq.t, tcp_cfg->qhead);
   (void)TCP_Timer_stop(&tcb->sndacktq.t, tcp_cfg->qhead);
   (void)TCP_Timer_stop(&tcb->sndtq.t, tcp_cfg->qhead);
   (void)TCP_Timer_stop(&tcb->rcvtq.t, tcp_cfg->qhead);
   if(tcb->expire.PRIVATE)
   {
     TCPIP_Event_cancel(&tcb->expire);     
     if(tcb->blocking_close)
     {
       RTCSCMD_complete((struct tcb_parm *)tcb->expire.PRIVATE, RTCS_OK);
     }
     tcb->expire.PRIVATE = NULL;
   }
   if(tcb->ev_keepalive.PRIVATE)
   {
     TCPIP_Event_cancel(&tcb->ev_keepalive);
     tcb->ev_keepalive.PRIVATE = NULL;
   }

   /*
   ** Return open request if there is one.  This should
   ** only happen if we are closing on error.
   */
   if ( (errnum != RTCS_OK) && 
        ((state != SYN_RECEIVED) /* don't return open request if an error is closing a child of listening TCB.  
                                  * the child of a listening TCB is closed silently */
         || ( (state == SYN_RECEIVED) && (tcb->options & TCPO_ACTIVE)) 
            /* but return a special case when an active open request went from SYN_SENT to SYN_RECEIVED */
        ))
   {
      TCP_Return_open(tcb, errnum, tcp_cfg);
   } /* Endif */

   /*
   ** Terminate any Receive requests.  We can't receive any more
   **    packets in the CLOSED state, and presence of any Receives
   **    means that all data that could be (contiguously) in the
   **    first Receive is already there (see TCP_Process_data).
   **
   ** In this case the TCB could actually be released, but wait for
   **    user to close it.  Don't have to delete all of the RcvChunks
   **    or indicate no data left in ring since a TCP_Process_close or
   **    TCP_Process_receive will do this.
   */
   while ( tcb->rcvHead != NULL ) {

      TCP_Reply_receive(tcb, tcp_cfg, errnum);
      if ( tcb->rcvHead != NULL ) {
         TCP_Setup_receive(tcb->rcvHead, tcb, tcp_cfg);
      } /* Endif */

   } /* Endwhile */

   /*
   ** Terminate any Send requests
   */
   buf = tcb->sndbuf;
   while ( buf != NULL ) {

      tcb->sndbuf = buf->next;
      if ( tcb->sndbuf == NULL ) {
         tcb->sndbuftail = NULL;
      } /* Endif */

      req = buf->req;
      if ( req != NULL ) {

         /* report actual amount sent */
         if ( first ) {
            req->BUFF_LENG  = tcb->snduna - tcb->sndbufseq;
         } else {
            req->BUFF_LENG = 0;
         } /* Endif */
         RTCSCMD_complete(req, errnum);

      } /* Endif */
      first = FALSE;

      _mem_free(buf);
      buf = tcb->sndbuf;

   } /* Endwhile */

   /*
   ** Notify socket layer in case some task is blocked on select()
   ** waiting for data on this socket.
   */   
   SOCK_select_signal(tcb->SOCKET, tcb->state, errnum);

   /*  Delete send buffers (keep receive buffer in case there is
    *  pending data)
    */
   if ( tcb->sndringbuf != NULL ) {
      _mem_free(tcb->sndringbuf);
   } /* Endif */
   tcb->sndringbuf = NULL;

   if ( tcb->sndclk != NULL ) {
      _mem_free(tcb->sndclk);     /* xmit timestamps buffer */
   } /* Endif */
   tcb->sndclk = NULL;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : TCP_Process_effective_close
* Returned Values : None.
* Comments        :
*
*  Called when the connection is actually closed, i.e., when all data
*   to send has been sent and FIN can at last be sent.
*
*END*-----------------------------------------------------------------*/

void TCP_Process_effective_close
   (
      TCB_STRUCT_PTR       tcb,     /* IN/OUT - TCP context */
      TCP_CFG_STRUCT_PTR   tcp_cfg  /* IN     - TCP layer data */
   )
{ /* Body */
   RTCSLOG_FNE2(RTCSLOG_FN_TCP_Process_effective_close, tcb->state);
    
   switch ( tcb->state ) {

      case SYN_RECEIVED:
         /* no data has been sent  */

      case ESTABLISHED:
         tcb->state = FINWAIT_1;
         TCP_Must_send_ack(tcb, tcp_cfg);
         break;

      case CLOSE_WAIT:
         tcb->state = LAST_ACK;
         TCP_Must_send_ack(tcb, tcp_cfg);
         break;

      default:
         RTCS_log_error(ERROR_TCP,
                             RTCSERR_TCP_BAD_STATE_FOR_CLOSE,
                             (uint32_t)tcb->state,
                             (uint32_t)tcb, 0);

         break;

   } /* End Switch */

    SOCK_select_signal(tcb->SOCKET, tcb->state, RTCSERR_TCP_CONN_CLOSING);
   
    RTCSLOG_FNX1(RTCSLOG_FN_TCP_Process_effective_close);
} /* Endbody */

/*
 * @internal
 * linger timeout has expired.
 */
static bool tcb_linger_timeout_expire(TCPIP_EVENT_PTR event)
{
  struct tcb_struct     *tcb;
  struct tcp_cfg_struct *tcp_cfg;          /* TCP Layer constants */
  
  /* check if TCB is still valid. */
  tcb = event->PRIVATE;
  if(FALSE == TCB_exists_in_system(tcb))
  {
    return FALSE;
  }
  
  tcp_cfg = RTCS_getcfg(TCP);  
  if(  
      (tcb->SOCKET == 0) &&      /* socket struct for the TCB has beed freed. by TCP_Process_close(). */
      (tcb->VALID == TCB_VALID_ID)  /* tcb still valid */
    )
  {
    (void)tcp_process_abort(tcb, tcp_cfg);
  }
  return FALSE;
}

static bool tcb_blocking_linger_timeout_expire(TCPIP_EVENT_PTR event)
{
  struct tcb_parm *parms = event->PRIVATE;  
  struct tcb_struct *tcb = parms->TCB_PTR;
  
  event->PRIVATE = tcb;  
  (void)tcb_linger_timeout_expire(event);
  
  RTCSCMD_complete(parms, RTCS_OK);
  
  return FALSE;
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : TCP_Process_close
* Returned Values : None.
* Comments        :
*
*  Process TcpClose() requests, within TCP server.
*
*END*-----------------------------------------------------------------*/

void TCP_Process_close
   (
      TCP_PARM_PTR   req_ptr     /* IN/OUT - the open request */
   )
{ /* Body */
   TCP_CFG_STRUCT_PTR   tcp_cfg;
   TCB_STRUCT_PTR       tcb;
   int32_t               reply = RTCS_OK;

   tcp_cfg = RTCS_getcfg(TCP);
   tcb = req_ptr->TCB_PTR;

   if ( tcb == NULL ) { 
      RTCSCMD_complete(req_ptr, RTCSERR_TCP_CONN_RLSD);
      return;
   }

   if(FALSE == SOCK_exists_in_system(tcb->SOCKET))
   {
     RTCSCMD_complete(req_ptr, RTCSERR_SOCK_INVALID);
     return;
   }   

   if ( tcb->VALID != TCB_VALID_ID ) {
      reply = RTCSERR_TCP_CONN_RLSD;
      RTCS_log_error( ERROR_TCP,
                           RTCSERR_TCP_BAD_STATE_FOR_CLOSE,
                           (uint32_t)tcb,
                           (uint32_t)tcb->state,
                           1 );

   } else if ( tcb->state == CLOSED ) {

      /* This should be the standard case if the other side aborted. */
      TCP_Process_release(tcb, tcp_cfg);
      IF_TCP_STATS_ENABLED(tcp_cfg->STATS.ST_CONN_CLOSED++);

   } else if ( (tcb->status & TCPS_FINTOSEND) != 0 ) {

      /* Upper layer must have already given us a tcpCLOSE, so we
       *    can generate an error here. */
      reply = RTCSERR_TCP_CONN_CLOSING;

   } else {
      IF_TCP_STATS_ENABLED(tcp_cfg->STATS.ST_CONN_CLOSED++);
      switch(tcb->state)
      {
        case LISTEN:
        {
          uint32_t errnum;
          if(req_ptr->OPCODE == 0)
          {
            errnum = RTCSERR_TCP_CONN_ABORTED; /* called from closesocket(). same code as abort(). */
          }
          else
          {
            errnum = RTCSERR_SOCK_ESHUTDOWN; /* called from shutdownsocket(). accept() should return with this error code in listening socket struct. */
          }
          TCP_Close_TCB(tcb, errnum, tcp_cfg);
          TCP_Process_release(tcb, tcp_cfg);
          break;
        }
        case SYN_SENT:
          TCP_Close_TCB(tcb, RTCSERR_TCP_CONN_RLSD, tcp_cfg);
        case BOUND:
          TCP_Process_release(tcb, tcp_cfg);
          break;

        case SYN_RECEIVED:
          /* no data has been sent */
          
          /* if this is active open request (connect()) make sure we return it.
           * this should only happen if an active open request went from SYN_SENT to SYN_RECEIVED
           * and application closes the socket (perhaps from another task)
           */
          if (tcb->options & TCPO_ACTIVE)
          {
              TCP_Return_open(tcb, RTCSERR_TCP_CONN_RLSD, tcp_cfg);
          }

        case ESTABLISHED:
        case CLOSE_WAIT:

          TCP_Event(tcb,TCPS_FINTOSEND);

          /*  We can only effectively close if all data has been sent,
           *   allowing us to send a FIN immediately; otherwise closing
           *   is delayed (TCPS_FINTOSEND indicates that connection should
           *   be closed as soon as all data is sent)
           */
          if(tcb->sndnxt == (tcb->sndbufseq + tcb->sndlen))
          {
            TCP_Process_effective_close(tcb, tcp_cfg);
          }
          else
          {
            /*
            **  Have to kick ourselves into sending the outstanding
            **  data or we'll sit here forever if the other side is
            **  just waiting.  This would happen if we send w/o PUSH
            **  just before closing.
            */
            TCP_Must_send_ack(tcb, tcp_cfg);
          } /* Endif */

          break;

         default:
          /*  we shouldn't reach here, but jic...  */
          RTCS_log_error( ERROR_TCP,
                                 RTCSERR_TCP_BAD_STATE_FOR_CLOSE,
                                 (uint32_t)tcb,
                                 (uint32_t)tcb->state,
                                 2);

          reply = RTCSERR_TCP_NOT_CONN;
          break;

      } /* End Switch */

   } /* Endif */   
    
  if(
      tcb 
      && (tcb->VALID == TCB_VALID_ID) /* if TCP_Process_release()/TCP_Free_TCB() was not called. LISTEN/SYN_SENT/BOUND */      
      && (
           (RTCS_OK == reply)                      /* SYN_RECEIVED/ESTABLISHED/CLOSE_WAIT */
           || (RTCSERR_TCP_CONN_CLOSING == reply)  /* if shutdownsocket(SHUT_WR) has been called prior to closesocket(). or closesocket() is called repeatedly on the same socket. */
         )         
      && (NULL == tcb->expire.PRIVATE) /* timeout timer not running yet for the TCB. */
    )
  {    
    struct socket_struct *sock_ptr = (struct socket_struct *)tcb->SOCKET;
    
    /* check that TCP_Process_close() is called from closesocket(). If TCP_Process_close() is called from shutdownsocket(),
     * we don't start the expire timer. Start the expire timer only when TCP_Process_close() is called from closesocket().
     * req_ptr->OPCODE == 0 means it is called from closesocket().
    */
    if(sock_ptr && (req_ptr->OPCODE == 0))
    {
      /* The socket will be freed as soon as we return to closesocket() */   
      tcb->SOCKET = 0;
    
      if(0 == sock_ptr->so_linger.l_onoff)
      {
        tcb->expire.EVENT = tcb_linger_timeout_expire;
        tcb->expire.PRIVATE = tcb;
        tcb->expire.TIME = tcb->timewaitto;
        TCPIP_Event_add(&tcb->expire);
        /* unblock caller. */
      }
      else
      {
        /* graceful close monitored by linger timeout */
        tcb->expire.EVENT = tcb_blocking_linger_timeout_expire;
        tcb->expire.PRIVATE = req_ptr;
        tcb->expire.TIME = sock_ptr->so_linger.l_linger_ms;        
        TCPIP_Event_add(&tcb->expire);
        tcb->blocking_close = TRUE;
        return; /* the caller will be unblocked either by tcb_blocking_linger_timeout_expire() or by TCP_Close_TCB() */
      }
    } /* if(req_ptr->OPCODE == 0) */
  }

  RTCSCMD_complete(req_ptr, reply);
}

static int32_t tcp_process_abort(struct tcb_struct *tcb, struct tcp_cfg_struct *tcp_cfg)
{
  uint32_t reply = RTCS_OK;
  switch(tcb->state)
  {
    case LISTEN:
      TCP_Close_TCB(tcb, RTCSERR_TCP_CONN_ABORTED, tcp_cfg);
      TCP_Process_release(tcb, tcp_cfg);
      break;

    case SYN_RECEIVED:
    case ESTABLISHED:
    case FINWAIT_1:
    case FINWAIT_2:
    case CLOSE_WAIT:
      TCP_Send_reset(tcb, tcp_cfg);      /* send reset packet */
      /*FALLTHROUGH*/

    case SYN_SENT:
    case CLOSING:
    case LAST_ACK:
    case TIME_WAIT:
      TCP_Close_TCB(tcb, RTCSERR_TCP_CONN_ABORTED, tcp_cfg);
      /*FALLTHROUGH*/

    case BOUND:
    case CLOSED:
      TCP_Process_release(tcb, tcp_cfg);
      break;

    default:
      reply = RTCSERR_TCP_NOT_CONN;
      break;
  }
  return reply;
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : TCP_Process_abort
* Returned Values : None.
* Comments        :
*
*  Process TcpAbort() requests, within TCP server.
*
*END*-----------------------------------------------------------------*/

void TCP_Process_abort
   (
      TCP_PARM_PTR   req_ptr     /* IN/OUT - the open request */
   )
{ /* Body */
   TCP_CFG_STRUCT_PTR   tcp_cfg;
   TCB_STRUCT_PTR       tcb;
   int32_t               reply = RTCS_OK;

   tcp_cfg = RTCS_getcfg(TCP);
   tcb = req_ptr->TCB_PTR;

   if ( tcb == NULL ) { 
      RTCSCMD_complete(req_ptr, RTCSERR_TCP_CONN_RLSD);
      return;
   }

   if(FALSE == SOCK_exists_in_system(tcb->SOCKET))
   {
     RTCSCMD_complete(req_ptr, RTCSERR_SOCK_INVALID);
     return;
   }   

   IF_TCP_STATS_ENABLED(tcp_cfg->STATS.ST_CONN_ABORTS++);

   if ( tcb->VALID != TCB_VALID_ID ) {
      reply = RTCSERR_TCP_CONN_RLSD;

   } else {

    reply = tcp_process_abort(tcb, tcp_cfg);  

   } /* Endif */

   /* The socket will be freed as soon as we return to closesocket() */
   if(tcb && (tcb->VALID == TCB_VALID_ID)) 
   {
     tcb->SOCKET = 0;
   } /* Endif */

   RTCSCMD_complete(req_ptr, reply);

} /* Endbody */

void TCP_Process_shutdown
   (
      TCP_PARM_PTR   req_ptr     /* IN/OUT - the open request */
   )
{
  uint32_t error = RTCS_OK;
  TCP_CFG_STRUCT_PTR tcp_cfg;
  struct tcb_struct *tcb;
  struct socket_struct *sock_ptr;
  uint32_t disallow_action;
  
  tcp_cfg = RTCS_getcfg(TCP);
  sock_ptr = (struct socket_struct *)req_ptr->SOCKET;
  tcb = req_ptr->TCB_PTR;
  
  if(FALSE == SOCK_exists_in_system(tcb->SOCKET))
  {
    error = RTCSERR_SOCK_INVALID;
    goto EXIT;    
  }  
  
  disallow_action = SOCK_set_disallow_mask(req_ptr->SOCKET, req_ptr->disallow_mask);
    
  /* return recv() requests. copied from TCP_Close_TCB(). */
  if(SOCK_disallow_recv(disallow_action))
  {
    while(tcb->rcvHead != NULL)
    {
      TCP_Reply_receive(tcb, tcp_cfg, RTCSERR_SOCK_ESHUTDOWN);
      if(tcb->rcvHead != NULL)
      {
        TCP_Setup_receive(tcb->rcvHead, tcb, tcp_cfg);
      }
    }
  }
  
  /* SHUT_WR or SHUT_RDWR: */
  /* connected stream: Send queued data, wait for ACK, then send FIN. 
   * if we are first to send FIN, TCB state will be: FIN_WAIT_2 or TIME_WAIT (FIN received) */
  /* listening stream: destroy listening TCB. */
  if(
      ((SOCK_disallow_send(disallow_action))||(sock_ptr->STATE == SOCKSTATE_STREAM_LISTENING))
    )
  {
    struct tcb_parm parms = *req_ptr;
    error = RTCSCMD_internal(parms, TCP_Process_close);
  }
  EXIT:
  RTCSCMD_complete(req_ptr, error);
}

#if RTCSCFG_TCP_MAX_HALF_OPEN
/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : TCP_half_open_TCB_close
* Returned Values : void
* Comments        :
* This function can be called to recover from SYN attack.
*
*END*-----------------------------------------------------------------*/

void TCP_half_open_TCB_close
   (
      TCP_CFG_STRUCT_PTR   tcp_cfg  /* IN/OUT - TCP layer data */
   )
{ /* Body */
   TCB_STRUCT_PTR       tcb;
   uint32_t              curr_time;
   uint32_t              removed_tcb_count = 0;
   int                  i;

   curr_time = RTCS_time_get();
   /*
   ** Delete all the TCBs older than the default retransmission
   ** timeout (3 seconds).
   */
   for (i = 0; i < tcp_cfg->HALF_OPEN_TCB_COUNT; i++) {
      tcb = tcp_cfg->HALF_OPEN_TCB_LIST[i];
      if(RTCS_timer_get_interval(tcb->tcb_spawn_time, curr_time) > TCP_INITIAL_RTO_DEFAULT) {
         TCP_Send_reset(tcb, tcp_cfg);      /* send reset packet */
         TCP_Close_TCB(tcb, RTCSERR_TCP_CONN_ABORTED, tcp_cfg);
         TCP_Process_release(tcb, tcp_cfg);
         i--;   /* we removed a TCB  and replaced another TCB
                   from the end, so we need to check same spot again */
         removed_tcb_count++;
      } /* Endif */
   } /* Endfor */

   /*
   ** Remove one TCB from the half open list randomly. Remove the TCB
   ** with a reset. If this is not an attack the client will resend the SYN.
   */
   if (!removed_tcb_count && tcp_cfg->HALF_OPEN_TCB_COUNT) {
      i = RTCS_rand() % tcp_cfg->HALF_OPEN_TCB_COUNT;
      tcb = tcp_cfg->HALF_OPEN_TCB_LIST[i];
      TCP_Send_reset(tcb, tcp_cfg);      /* send reset packet */
      TCP_Close_TCB(tcb, RTCSERR_TCP_CONN_ABORTED, tcp_cfg);
      TCP_Process_release(tcb, tcp_cfg);
   } /* Endif */
} /* Endbody */
#endif

/* EOF */
