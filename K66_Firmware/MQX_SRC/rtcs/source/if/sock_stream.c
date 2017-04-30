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
*   This file contains the socket interface functions for
*   (PF_INET,SOCK_STREAM) sockets.
*
*
*END************************************************************************/

#include <rtcs.h>
#include "rtcs_prv.h"
#include "tcpip.h"
#include "socket.h"

#if RTCSCFG_ENABLE_TCP

#define RTCS_ENTER(f,a)    RTCSLOG_API(RTCSLOG_TYPE_FNENTRY, RTCSLOG_FN_STREAM_ ## f, a)

#define RTCS_EXIT(f,a)     RTCSLOG_API(RTCSLOG_TYPE_FNEXIT,  RTCSLOG_FN_STREAM_ ## f, a); \
                           return a

#define RTCS_EXIT2(f,a,e)  RTCSLOG_API(RTCSLOG_TYPE_FNEXIT,  RTCSLOG_FN_STREAM_ ## f, a); \
                           return e



static uint32_t SOCK_STREAM_socket      (uint32_t);
static uint32_t SOCK_STREAM_bind      (uint32_t, const sockaddr *, uint16_t);
static uint32_t SOCK_STREAM_connect     (uint32_t, const sockaddr *, uint16_t);
static uint32_t SOCK_STREAM_listen      (uint32_t, int32_t);
static uint32_t SOCK_STREAM_accept      (uint32_t, sockaddr *, uint16_t *);
static uint32_t SOCK_STREAM_getsockname (uint32_t, sockaddr *, uint16_t *);
static uint32_t SOCK_STREAM_getpeername (uint32_t, sockaddr *, uint16_t *);
static int32_t SOCK_STREAM_recv        (uint32_t, void *, uint32_t, uint32_t);
static int32_t SOCK_STREAM_send        (uint32_t, void *, uint32_t, uint32_t);



const RTCS_SOCKET_CALL_STRUCT SOCK_STREAM_CALL = {
   SOCK_STREAM_socket,
   SOCK_STREAM_bind,
   SOCK_STREAM_connect,
   SOCK_STREAM_listen,
   SOCK_STREAM_accept,
   SOCK_STREAM_getsockname,
   SOCK_STREAM_getpeername,
   SOCK_STREAM_recv,
   NULL,
   NULL,
   SOCK_STREAM_send,
   NULL,
   NULL,
   NULL,
   NULL
};

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_STREAM_socket
* Returned Value  : error code
* Comments  :  Creates an unbound socket.
*
*END*-----------------------------------------------------------------*/

static uint32_t  SOCK_STREAM_socket
   (
      uint32_t     sock
         /* [IN] socket handle */
   )
{ /* Body */
   TCP_PARM    parms;
   uint32_t     error;

   parms.SOCKET = sock;

   error = RTCSCMD_issue(parms, TCP_Process_create);
   if (error) return error;

   ((SOCKET_STRUCT_PTR)sock)->STATE   = SOCKSTATE_STREAM_GROUND;
   ((SOCKET_STRUCT_PTR)sock)->TCB_PTR = parms.TCB_PTR;
   return RTCS_OK;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name     : SOCK_STREAM_bind
* Returned Value    : error code
* Comments          : Binds the local endpoint of a socket for IPv4 and IPv6 families.
*END*-----------------------------------------------------------------*/
static uint32_t  SOCK_STREAM_bind
   (
        uint32_t                    sock,
         /* [IN] socket handle */
        const sockaddr     *localaddr,
         /* [IN] IPv4 or IPv6 local address to which to bind the socket */
        uint16_t                    addrlen
         /* [IN] length of the address, in bytes */
   )
{
  TCP_PARM    parms;
  uint32_t    error = 0;
  sockaddr    laddr = {0};
  SOCKET_STRUCT_PTR sock_struct_ptr = (SOCKET_STRUCT_PTR)sock;

  RTCS_ENTER(BIND, sock);

  if(sock_struct_ptr->STATE != SOCKSTATE_STREAM_GROUND) 
  {
    RTCS_EXIT(BIND, RTCSERR_SOCK_IS_BOUND);
  }
  
  if(!sock_struct_ptr->TCB_PTR) 
  {
    RTCS_EXIT(BIND, RTCSERR_TCP_CONN_RLSD);
  }

  parms.TCB_PTR = sock_struct_ptr->TCB_PTR;
  SOCKADDR_copy(localaddr, &laddr); /* from->to */
  parms.saddr_ptr = &laddr;  
    
  /* Call main bind function */
  error = RTCSCMD_issue(parms, TCP_Process_bind);
  if (error) 
  {
    RTCS_EXIT(BIND, error);
  }
  
  sock_struct_ptr->STATE = SOCKSTATE_STREAM_BOUND;
  RTCS_EXIT(BIND, RTCS_OK);
}


/*FUNCTION*-------------------------------------------------------------
*
* Function Name     : SOCK_STREAM_connect
* Returned Value    : error code
* Comments          : Binds the remote endpoint of a socket. IPv4 and IPv6 support.
*END*-----------------------------------------------------------------*/
static uint32_t  SOCK_STREAM_connect 
   (
      uint32_t                   sock,
         /* [IN] socket handle */
      const sockaddr       *peeraddr,
         /* [IN] remote IPv4 or IPv6 address to which to bind the socket */
      uint16_t                   addrlen
         /* [IN] length of the address, in bytes */
   )
{
  TCP_PARM parms;
  TCP_OPTIONS_STRUCT  options;
  SOCKET_STRUCT_PTR   socket_ptr = (SOCKET_STRUCT_PTR)sock;
  uint32_t            error = 0;
            
  struct sockaddr param_addr;

  RTCS_ENTER(CONNECT, sock);

  switch(socket_ptr->STATE) 
  {
    case SOCKSTATE_STREAM_LISTENING: RTCS_EXIT(CONNECT, RTCSERR_SOCK_IS_LISTENING);
    case SOCKSTATE_STREAM_CONNECTED: RTCS_EXIT(CONNECT, RTCSERR_SOCK_IS_CONNECTED);
  }
  
  if (!socket_ptr->TCB_PTR)
  {
      RTCS_EXIT(CONNECT, RTCSERR_TCP_CONN_RLSD);
  } /* Endif */

  /* Bind if socket was not binded yet */
  if(socket_ptr->STATE == SOCKSTATE_STREAM_GROUND) 
  {
    struct sockaddr localaddr = {0};
    localaddr.sa_family = peeraddr->sa_family;
    error = SOCK_STREAM_bind(sock, &localaddr, sizeof(localaddr));
    
    if(error)
    {
      RTCS_EXIT(CONNECT, error);
    }
  }
    
  /*
  **  Setup the known 'connect' options
  */
  SOCKADDR_copy(peeraddr , &param_addr);
  parms.saddr_ptr = &param_addr;
  
  /*  
   *  Chose and add scope_id here. 
   *  If peeraddr scope_id is not NULL we will use it.  
   *  If it is NULL, we will use scope_id of local binded address
   */
  if(SOCKADDR_get_if_scope_id(&param_addr) == 0)
  {
    /* Here if_scope_id is if nember that was binded with local address */
    SOCKADDR_set_if_scope_id(&param_addr, SOCKADDR_get_if_scope_id(&socket_ptr->TCB_PTR->laddr));
  }
    

  options.utimeout   = socket_ptr->SEND_TIMEOUT;
  options.timeout    = socket_ptr->CONNECT_TIMEOUT;
  options.rto        = socket_ptr->RETRANSMISSION_TIMEOUT;
  options.active     = TRUE;
  options.tbsize     = socket_ptr->TBSIZE;
  options.rbsize     = socket_ptr->RBSIZE;
  options.maxrto     = socket_ptr->MAXRTO;
  options.maxrcvwnd  = socket_ptr->MAXRCV_WND;
  options.so_keepalive  = socket_ptr->KEEPALIVE;
  options.nowait     = socket_ptr->NOWAIT;
  options.nonagle    = socket_ptr->NO_NAGLE_ALGORITHM;
  options.noswrbuf   = socket_ptr->NOSWRBUF;
  options.timewaitto = socket_ptr->TIMEWAIT_TIMEOUT; 
  options.tcpsecuredraft0 = socket_ptr->TCPSECUREDRAFT_0; 
  options.delayackto = socket_ptr->DELAY_ACK;

  /*
  **  Setup the default 'connect' options
  */
  options.tos        = 0;
  options.precede    = 0xFF;
  options.ttl        = 0;

  parms.TCB_PTR     = socket_ptr->TCB_PTR;
  parms.OPTIONS_PTR = &options;
   
  error = RTCSCMD_issue(parms, TCP_Process_open);
  if (error) {
     RTCS_EXIT(CONNECT, error);
  }

  socket_ptr->STATE = SOCKSTATE_STREAM_CONNECTED;
  RTCS_EXIT(CONNECT, RTCS_OK);

} /* Endbody */

/**
 * @internal
 * Changes backlog queue length for the given TCB. Called from context of TCP/IP task.
 * Called in response to a user API call to listen() with a socket that has been already marked as listening.
 */
static void sock_stream_change_backlog(struct tcb_parm *req_ptr)
{  
  struct tcb_struct *tcb;
  struct tcp_options_struct *coptions; /* list of TCP options */
  
  RTCS_ASSERT(req_ptr != NULL);
  tcb = req_ptr->TCB_PTR;  
  if(NULL == tcb)
  {
    RTCSCMD_complete(req_ptr, RTCSERR_TCP_CONN_RLSD);
  }
  
  coptions = req_ptr->OPTIONS_PTR;
  RTCS_ASSERT(coptions != NULL);
  
  /* change listen backlog to new value */
  tcb->backlog = coptions->backlog;
  
  RTCSCMD_complete(req_ptr, RTCS_OK);
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_STREAM_listen
* Returned Value  : error code
* Comments  :  Enables remote connections on a socket.
*
*END*-----------------------------------------------------------------*/
static uint32_t  SOCK_STREAM_listen
   (
      uint32_t     sock,
         /* [IN] socket handle */
      int32_t      backlog
         /* [IN] backlog size, hint for the RTCS to determine maximum number of fully established connections pending for accept() */
   )
{
  TCP_PARM             parms;
  TCP_OPTIONS_STRUCT   options;
  SOCKET_STRUCT_PTR    socket_ptr = (SOCKET_STRUCT_PTR)sock;
  uint32_t             error = 0;
  struct sockaddr      param_addr = {0};
  
  RTCS_ENTER(LISTEN, sock);
  
  if(!socket_ptr->TCB_PTR) 
  {
    RTCS_EXIT(LISTEN, RTCSERR_TCP_CONN_RLSD);
  }
    
  param_addr.sa_family = socket_ptr->AF;  
  backlog = backlog < 0 ? 0: (backlog + 1);
  #if RTCSCFG_SOMAXCONN
  if(backlog > RTCSCFG_SOMAXCONN)
  {
    backlog = RTCSCFG_SOMAXCONN;
  }
  #endif
  options.backlog = backlog;
  
  parms.TCB_PTR     = socket_ptr->TCB_PTR;
  parms.OPTIONS_PTR = &options;
  parms.SOCKET      = sock;

  switch (socket_ptr->STATE) 
  {
    case SOCKSTATE_STREAM_GROUND:    RTCS_EXIT(LISTEN, RTCSERR_SOCK_NOT_BOUND);
    case SOCKSTATE_STREAM_CONNECTED: RTCS_EXIT(LISTEN, RTCSERR_SOCK_IS_CONNECTED);
    case SOCKSTATE_STREAM_LISTENING: 
    {
      /* if the sock is already marked as listening, only change backlog queue length for it. */
      error = RTCSCMD_issue(parms, sock_stream_change_backlog);
      goto exit;
    }
  }
  
  /* add scope_id of listen interface here */
  SOCKADDR_set_if_scope_id(&param_addr, SOCKADDR_get_if_scope_id(&socket_ptr->TCB_PTR->laddr));
  parms.saddr_ptr = &param_addr;
   
  options.utimeout   = socket_ptr->SEND_TIMEOUT;
  options.timeout    = socket_ptr->CONNECT_TIMEOUT;
  options.rto        = socket_ptr->RETRANSMISSION_TIMEOUT;
  options.active     = FALSE;
  options.tbsize     = socket_ptr->TBSIZE;
  options.rbsize     = socket_ptr->RBSIZE;
  options.maxrto     = socket_ptr->MAXRTO;
  options.maxrcvwnd  = socket_ptr->MAXRCV_WND;
  options.so_keepalive  = socket_ptr->KEEPALIVE;
  options.nowait     = socket_ptr->NOWAIT;
  options.nonagle    = socket_ptr->NO_NAGLE_ALGORITHM;
  options.noswrbuf   = socket_ptr->NOSWRBUF;
  options.timewaitto = socket_ptr->TIMEWAIT_TIMEOUT;
  options.delayackto = socket_ptr->DELAY_ACK;  

  /*
  **  Setup the default 'connect' options
  */
  options.tos        = 0;
  options.precede    = 0xFF;
  options.ttl        = 0;

  error = RTCSCMD_issue(parms, TCP_Process_open);

exit:
  if(error) 
  {
    RTCS_EXIT(LISTEN, error);
  }

  /* mark sock as a listening for connections */
  socket_ptr->STATE = SOCKSTATE_STREAM_LISTENING;
   
  RTCS_EXIT(LISTEN, RTCS_OK);
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_STREAM_accept
* Returned Value  : socket
* Comments  :  Wait for a connection on a socket, support IPv4 and IPv6.
*
*END*-----------------------------------------------------------------*/
static uint32_t  SOCK_STREAM_accept
   (
             uint32_t                sock,
         /* [IN] socket handle */
      struct sockaddr      *peeraddr,
         /* [OUT] remote address IPv6 to which the socket is connected */
             uint16_t           *addrlen
         /* [IN/OUT] length of the address IPv6, in bytes */
   )
{ /* Body */
  TCP_PARM             parms;
  SOCKET_STRUCT_PTR    socket_ptr = (SOCKET_STRUCT_PTR)sock;
  SOCKET_STRUCT_PTR    new_socket_ptr;
  uint32_t              error;
  uint32_t              newsock;

  RTCS_ENTER(ACCEPT, sock);

  switch (socket_ptr->STATE) 
  {
    case SOCKSTATE_STREAM_GROUND:
            RTCS_setsockerror(sock, RTCSERR_SOCK_NOT_LISTENING);
            RTCS_EXIT2(ACCEPT, RTCSERR_SOCK_NOT_LISTENING, RTCS_SOCKET_ERROR);
    case SOCKSTATE_STREAM_BOUND:
            RTCS_setsockerror(sock, RTCSERR_SOCK_NOT_LISTENING);
            RTCS_EXIT2(ACCEPT, RTCSERR_SOCK_NOT_LISTENING, RTCS_SOCKET_ERROR);
    case SOCKSTATE_STREAM_CONNECTED:
            RTCS_setsockerror(sock, RTCSERR_SOCK_IS_CONNECTED);
            RTCS_EXIT2(ACCEPT, RTCSERR_SOCK_IS_CONNECTED, RTCS_SOCKET_ERROR);
  }

   /*
   ** Allocate a new socket structure for the established TCB
   */
   new_socket_ptr = SOCK_Get_sock_struct((RTCS_SOCKET_CALL_STRUCT_PTR)SOCK_STREAM,
                                         RTCS_task_getid());
   newsock = (uint32_t)new_socket_ptr;

   if (new_socket_ptr == NULL) {
      RTCS_setsockerror(sock, RTCSERR_OUT_OF_SOCKETS);
      RTCS_EXIT2(ACCEPT, RTCSERR_OUT_OF_SOCKETS, RTCS_SOCKET_ERROR);
   } /* Endif */

   parms.TCB_PTR = socket_ptr->TCB_PTR;
   new_socket_ptr->AF = socket_ptr->AF;
   parms.SOCKET = newsock;

   error = RTCSCMD_issue(parms, TCP_Process_accept);
   if (error) {
      SOCK_Free_sock_struct(new_socket_ptr);
      if(error == RTCSERR_TCP_CONN_ABORTED)
      {  /* this error when we were blocked on accept() and another task calls closesocket(). 
          * as the socket struct does not exist (closesocket() destroys it), upper layer must not access the socket struct.
          * so we set RTCS_errno = RTCSERR_SOCK_CLOSED, so the application is aware.
          */
        RTCS_set_errno(RTCSERR_SOCK_CLOSED);
      }
      else if(error == RTCSERR_TCP_CONN_RLSD)
      {  /* this error when we call accept() on a socket, after it has been shutdown by shutdownsocket().
          * as the socket struct still exist, the upper layer shall close the socket by closesocket().
          * so we set RTCS_errno = RTCSERR_SOCK_ESHUTDOWN, so the application knows the socket struct shall be closed.
          */
        /* shutdownsocket() followed by accept(). */
        RTCS_set_errno(RTCSERR_SOCK_ESHUTDOWN);
        RTCS_setsockerror(sock, error);
      }
      else
      { /* one of the errors here is RTCSERR_SOCK_ESHUTDOWN, when we were blocked on accept() and another task calls shutdownsocket(). 
         * so we set RTCS_errno = RTCSERR_SOCK_ESHUTDOWN, so the application knows the socket struct shall be closed.
         */
        /* accept() followed by shutdownsocket(). */
        RTCS_set_errno(error);
        RTCS_setsockerror(sock, error);
      }
      RTCS_EXIT2(ACCEPT, error, RTCS_SOCKET_ERROR);
   }

   new_socket_ptr->STATE = SOCKSTATE_STREAM_CONNECTED;
   
   error = SOCK_STREAM_getpeername(newsock, peeraddr, addrlen);

   /*
   ** If there is an error, it is because the TCB was freed (by a RST).
   ** We will free the socket and return a socket error
   */
   if (error) {
      SOCK_Free_sock_struct(new_socket_ptr);
      RTCS_setsockerror(sock, error);
      RTCS_set_errno(error); /* set RTCS_errno */
      RTCS_EXIT2(ACCEPT, error, RTCS_SOCKET_ERROR);
   }

   /*
   ** inherit the socket options from listen socket
   */
   new_socket_ptr->CONNECT_TIMEOUT        = socket_ptr->CONNECT_TIMEOUT;
   new_socket_ptr->RETRANSMISSION_TIMEOUT = socket_ptr->RETRANSMISSION_TIMEOUT;
   new_socket_ptr->SEND_TIMEOUT           = socket_ptr->SEND_TIMEOUT;
   new_socket_ptr->RECEIVE_TIMEOUT        = socket_ptr->RECEIVE_TIMEOUT;
   new_socket_ptr->RECEIVE_PUSH           = socket_ptr->RECEIVE_PUSH;
   new_socket_ptr->SEND_NOWAIT            = socket_ptr->SEND_NOWAIT;
   new_socket_ptr->SEND_PUSH              = socket_ptr->SEND_PUSH;
   new_socket_ptr->RECEIVE_NOWAIT         = socket_ptr->RECEIVE_NOWAIT;
   new_socket_ptr->TBSIZE                 = socket_ptr->TBSIZE;
   new_socket_ptr->RBSIZE                 = socket_ptr->RBSIZE;
   new_socket_ptr->MAXRTO                 = socket_ptr->MAXRTO;
   new_socket_ptr->MAXRCV_WND             = socket_ptr->MAXRCV_WND;
   new_socket_ptr->KEEPALIVE              = socket_ptr->KEEPALIVE;
   new_socket_ptr->NOWAIT                 = socket_ptr->NOWAIT;
   new_socket_ptr->NO_NAGLE_ALGORITHM     = socket_ptr->NO_NAGLE_ALGORITHM;
   new_socket_ptr->NOSWRBUF               = socket_ptr->NOSWRBUF;
   new_socket_ptr->TIMEWAIT_TIMEOUT       = socket_ptr->TIMEWAIT_TIMEOUT;
   new_socket_ptr->AF                     = socket_ptr->AF;
   new_socket_ptr->LINK_OPTIONS           = socket_ptr->LINK_OPTIONS;   

   RTCS_EXIT2(ACCEPT, RTCS_OK, newsock);
}


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_STREAM_send
* Returned Value  : number of bytes sent or RTCS_ERROR
* Comments  :  Send data to a connected socket.
*
*END*-----------------------------------------------------------------*/

static int32_t  SOCK_STREAM_send
   (
      uint32_t        sock,
         /* [IN] socket handle */
      void          *buffer,
         /* [IN] data to transmit */
      uint32_t        buflen,
         /* [IN] length of the buffer, in bytes */
      uint32_t        flags
         /* [IN] flags to underlying protocols */
   )
{ /* Body */
            TCP_PARM             parms;
   register SOCKET_STRUCT_PTR    socket_ptr = (SOCKET_STRUCT_PTR)sock;
            uint32_t              error;

  RTCS_ENTER(SEND, sock);

  if (socket_ptr->STATE != SOCKSTATE_STREAM_CONNECTED) 
  {
    RTCS_setsockerror(sock, RTCSERR_SOCK_NOT_CONNECTED);
    RTCS_EXIT2(SEND, RTCSERR_SOCK_NOT_CONNECTED, RTCS_ERROR);
  }

  if(socket_ptr->TCB_PTR == NULL)
  {
    RTCS_setsockerror(sock, RTCSERR_TCP_CONN_RLSD);
    RTCS_EXIT2(SEND, RTCSERR_TCP_CONN_RLSD, RTCS_ERROR);
  }
   
   /* at the moment we support zero, MSG_DONTWAIT and MSG_WAITACK for SOCK_STREAM_send */
   if (flags == 0)
   {
     /* read SEND_NOWAIT default from socket options */
      if(socket_ptr->SEND_NOWAIT)  flags |= TCPO_NOWAIT;
   }
   else if(flags & MSG_DONTWAIT)
   {
     flags = TCPO_NOWAIT; /* note this clears the MSG_DONTWAIT from the flag. */
   }
   else if(flags & MSG_WAITACK)
   {
     flags = TCPO_WAIT; /* note this clears the MSG_WAITACK from the flag. */     
   }
   else
   {
      /* for stream socket, supported flags are zero or MSG_DONTWAIT or MSG_WAITACK */
      /* if flags is different, means application tries to pass some options. */
      /* return error in this case */
      /* options for stream socket should be configured in SOCKET_STRUCT */
      RTCS_setsockerror(sock, RTCSERR_SOCK_INVALID_OPTION);
      RTCS_EXIT2(SEND, RTCSERR_SOCK_INVALID_OPTION, RTCS_ERROR);
   }
   /* Endif */

   if(socket_ptr->SEND_PUSH)    flags |= TCPO_PUSH;

   parms.TCB_PTR    = socket_ptr->TCB_PTR;
   parms.SOCKET     = sock;
   parms.BUFFER_PTR = buffer;
   parms.BUFF_LENG  = buflen;
   parms.TIMEOUT    = socket_ptr->SEND_TIMEOUT;
   parms.OPTIONS    = flags;

    if((flags & (TCPO_NOWAIT|TCPO_WAIT)) != 0) /* TCPO_NOWAIT or TCPO_WAIT */
    {   
       error = RTCSCMD_issue(parms, TCP_Process_send);
       if (error)
       {
            RTCS_setsockerror(sock, error);
            RTCS_EXIT2(SEND, error, RTCS_ERROR);
       } 
     
       RTCS_EXIT2(SEND, RTCS_OK, parms.BUFF_LENG);
    }   
    else /* Default */
    {
        uint32_t result_length = 0;
        rtcs_fd_set wfds;
        do
        {
            error = RTCSCMD_issue(parms, TCP_Process_send);
            if (error)
            {
                RTCS_setsockerror(sock, error);
                RTCS_EXIT2(SEND, error, RTCS_ERROR);
            } 
             
            if(parms.BUFF_LENG >0)
            {  
                result_length += parms.BUFF_LENG;
            }
            else
            {
              RTCS_FD_ZERO(&wfds);
              RTCS_FD_SET(sock,&wfds);
              select(1,NULL,&wfds,NULL,1);
            }
            
            parms.BUFFER_PTR = (void *) ((uint32_t)(parms.BUFFER_PTR) + parms.BUFF_LENG);
            parms.BUFF_LENG  = buflen - result_length;
        }
        while(parms.BUFF_LENG > 0);
        
        RTCS_EXIT2(SEND, RTCS_OK, result_length);
    }

} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_STREAM_recv
* Returned Value  : number of bytes received or RTCS_ERROR
* Comments  :  Receive data from a connected socket.
*
*END*-----------------------------------------------------------------*/

static int32_t  SOCK_STREAM_recv
   (
      uint32_t        sock,
         /* [IN] socket handle */
      void          *buffer,
         /* [IN] buffer for received data */
      uint32_t        buflen,
         /* [IN] length of the buffer, in bytes */
      uint32_t        flags
         /* [IN] flags to underlying protocols */
   )
{ /* Body */
            TCP_PARM             parms;
   register SOCKET_STRUCT_PTR    socket_ptr = (SOCKET_STRUCT_PTR)sock;
            uint32_t              error;

  RTCS_ENTER(RECV, sock);

  if (socket_ptr->STATE != SOCKSTATE_STREAM_CONNECTED) 
  {
    RTCS_setsockerror(sock, RTCSERR_SOCK_NOT_CONNECTED);
    RTCS_EXIT2(RECV, RTCSERR_SOCK_NOT_CONNECTED, RTCS_ERROR);
  }

  if (!socket_ptr->TCB_PTR) 
  {
    RTCS_setsockerror(sock, RTCSERR_TCP_CONN_RLSD);
    RTCS_EXIT2(RECV, RTCSERR_TCP_CONN_RLSD, RTCS_ERROR);
  }

  /* at the moment we support zero, MSG_DONTWAIT and MSG_WAITALL for SOCK_STREAM_recv */
   if (flags == 0)
   {
     /* read RECEIVE_NOWAIT from socket option */
      if(socket_ptr->RECEIVE_NOWAIT)
      {
        flags |= TCPO_NOWAIT; /* non-blocking */
      }
      else 
      {
        flags |= TCPO_WAIT;  /* blocking */
      }      
   }
   else if(flags & MSG_DONTWAIT)
   {
     flags = TCPO_NOWAIT; /* note this clears the MSG_DONTWAIT from the flag. */
   }
   else if(flags & MSG_WAITALL)
   {
     flags = TCPO_WAIT; /* note this clears the MSG_WAITALL from the flag. */
     goto no_push; /* bypass RECEIVE_PUSH socket option => the read request behaves as if RECEIVE_PUSH was set to FALSE */
   }
   else
   {
      /* for stream socket receive, supported flags are zero or MSG_DONTWAIT or MSG_WAITALL */
      /* if flags is different, means application tries to pass some options. */
      /* return error in this case */
      /* options for stream socket should be configured in SOCKET_STRUCT */
      RTCS_setsockerror(sock, RTCSERR_SOCK_INVALID_OPTION);
      RTCS_EXIT2(RECV, RTCSERR_SOCK_INVALID_OPTION, RTCS_ERROR);
   }
   
  if(socket_ptr->RECEIVE_PUSH)    
  {
    flags |= TCPO_PUSH; /* return on received TCP push flag */
  }

no_push:

  parms.TCB_PTR    = socket_ptr->TCB_PTR;
  parms.SOCKET     = sock;
  parms.BUFFER_PTR = buffer;
  parms.BUFF_LENG  = buflen;
  parms.TIMEOUT    = socket_ptr->RECEIVE_TIMEOUT;
  parms.OPTIONS    = flags;

  error = RTCSCMD_issue(parms, TCP_Process_receive);
  if (error) {
    RTCS_setsockerror(sock, error);
    
    /* If data was copied to the userbuf, but not all that
       the recv() asked for, and a timer was started that has
       now timed out, we need to return with the count, and not
       RTCS_ERROR 
       
     * The count is prepared by the TCP/IP task in the parms.OPCODE
     * OPCODE = tcb->rcvnxt - prev_rcvbufseq
     * where prev_rcvbufseq is set to tcb->rcvbufseq by TCP_Process_receive()
     */
    if (error == RTCSERR_TCP_TIMED_OUT) {
       RTCS_EXIT2(RECV, RTCS_OK, parms.OPCODE);
    }    
    
    RTCS_EXIT2(RECV, error, RTCS_ERROR);
  }

  RTCS_EXIT2(RECV, RTCS_OK, parms.BUFF_LENG);
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_STREAM_getsockname
* Returned Value  : error code
* Comments  :  Retrieve the address of the local endpoint of a
*              bound socket. Support IPv6 and IPv4.
*
*END*-----------------------------------------------------------------*/

static uint32_t  SOCK_STREAM_getsockname
   (
      uint32_t              sock,
         /* [IN] socket handle */
      sockaddr     *name,
         /* [OUT] address of local endpoint */
      uint16_t         *namelen
         /* [IN/OUT] length of the address, in bytes */
   )
{
  RTCS_ENTER(GETSOCKNAME, sock);

  if(((SOCKET_STRUCT_PTR)sock)->STATE == SOCKSTATE_STREAM_GROUND) 
  {
    RTCS_EXIT(GETSOCKNAME, RTCSERR_SOCK_NOT_BOUND);
  }

  if(!((SOCKET_STRUCT_PTR)sock)->TCB_PTR) 
  {
    RTCS_EXIT(GETSOCKNAME, RTCSERR_TCP_CONN_RLSD);
  }
  
  SOCKADDR_return_addr((SOCKET_STRUCT_PTR)sock, name, &((SOCKET_STRUCT_PTR)sock)->TCB_PTR->laddr, namelen);

  RTCS_EXIT(GETSOCKNAME, RTCS_OK);
}


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_STREAM_getpeername
* Returned Value  : error code
* Comments  :  Retrieve the address of the remote endpoint of a
*              connected socket. Support IPv4 and IPv6
*
*END*-----------------------------------------------------------------*/

static uint32_t  SOCK_STREAM_getpeername
   (
      uint32_t                     sock,
         /* [IN] socket handle */
      struct sockaddr    *name,
         /* [IN] address of remote endpoint */
      uint16_t                *namelen
         /* [IN/OUT] length of the address, in bytes */
   )
{
  RTCS_ENTER(GETPEERNAME, sock);

  if (((SOCKET_STRUCT_PTR)sock)->STATE != SOCKSTATE_STREAM_CONNECTED) 
  {
    RTCS_EXIT(GETPEERNAME, RTCSERR_SOCK_NOT_CONNECTED);
  }

  if (!((SOCKET_STRUCT_PTR)sock)->TCB_PTR) 
  {
    RTCS_EXIT(GETPEERNAME, RTCSERR_TCP_CONN_RLSD);
  }
  
  SOCKADDR_return_addr((SOCKET_STRUCT_PTR)sock, name, &((SOCKET_STRUCT_PTR)sock)->TCB_PTR->raddr, namelen);

  RTCS_EXIT(GETPEERNAME, RTCS_OK);
}

#else

const RTCS_SOCKET_CALL_STRUCT SOCK_STREAM_CALL; /* for SOCK_STREAM, TBD fix other way.*/

#endif

/* EOF */
