/*HEADER**********************************************************************
*
* Copyright 2008-2014 Freescale Semiconductor, Inc.
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
*   (PF_INET,SOCK_DGRAM) sockets.
*
*
*END************************************************************************/

#include <rtcs.h>
#include "rtcs_prv.h"

#if RTCSCFG_ENABLE_UDP

#include "tcpip.h"
#include "udp.h"
#include "socket.h"
#include <string.h>

#define RTCS_ENTER(f,a)    RTCSLOG_API(RTCSLOG_TYPE_FNENTRY, RTCSLOG_FN_DGRAM_ ## f, a)

#define RTCS_EXIT(f,a)     RTCSLOG_API(RTCSLOG_TYPE_FNEXIT,  RTCSLOG_FN_DGRAM_ ## f, a); \
                           return a

#define RTCS_EXIT2(f,a,e)  RTCSLOG_API(RTCSLOG_TYPE_FNEXIT,  RTCSLOG_FN_DGRAM_ ## f, a); \
                           return e


static uint32_t SOCK_DGRAM_socket      (uint32_t);
static uint32_t SOCK_DGRAM_bind        (uint32_t, const sockaddr *, uint16_t);
static uint32_t SOCK_DGRAM_connect  (uint32_t, const sockaddr *, uint16_t);
static uint32_t SOCK_DGRAM_getsockname (uint32_t, sockaddr *, uint16_t *);
static uint32_t SOCK_DGRAM_getpeername (uint32_t, sockaddr *, uint16_t *);
static int32_t SOCK_DGRAM_recv        (uint32_t, void *, uint32_t, uint32_t);
static int32_t SOCK_DGRAM_recvfrom    (uint32_t, void *, uint32_t, uint32_t, sockaddr *, uint16_t *);//IPv4+6
static int32_t SOCK_DGRAM_send        (uint32_t, void *, uint32_t, uint32_t);
static int32_t SOCK_DGRAM_sendto      (uint32_t, void *, uint32_t, uint32_t, sockaddr *, uint16_t);

const RTCS_SOCKET_CALL_STRUCT SOCK_DGRAM_CALL = {
   SOCK_DGRAM_socket,
   SOCK_DGRAM_bind,
   SOCK_DGRAM_connect,
   NULL,
   NULL,
   SOCK_DGRAM_getsockname,
   SOCK_DGRAM_getpeername,
   SOCK_DGRAM_recv,
   SOCK_DGRAM_recvfrom,
   NULL,
   SOCK_DGRAM_send,
   SOCK_DGRAM_sendto,
   NULL,
   NULL,
   NULL
};

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_DGRAM_socket
* Returned Value  : error code
* Comments  :  Creates an unbound socket.
*
*END*-----------------------------------------------------------------*/
static uint32_t  SOCK_DGRAM_socket
   (
      uint32_t     sock
         /* [IN] socket handle */
   )
{
    UDP_PARM    parms = {0};
    uint32_t    error;
    SOCKET_STRUCT_PTR sock_struct_ptr = (SOCKET_STRUCT_PTR)sock;
    uint16_t af = sock_struct_ptr->AF;
    
    parms.udpword = af;
    error = RTCSCMD_issue(parms, UDP_open);
    if (error) return error;

    parms.ucb->SOCKET = sock;
    sock_struct_ptr->STATE         = SOCKSTATE_DGRAM_GROUND;
    sock_struct_ptr->UCB_PTR       = parms.ucb;
    sock_struct_ptr->MCB_PTR       = &parms.ucb->MCB_PTR;
    sock_struct_ptr->IGMP_LEAVEALL = &parms.ucb->IGMP_LEAVEALL;
    return RTCS_OK;
}

/*FUNCTION*-------------------------------------------------------------
************************************************************************
*
* Function Name   : SOCK_DGRAM_bind
* Returned Value  : error code 
*
* Comments  :  Binds the local endpoint of a socket. Supports IPv6 and IPv4
*
************************************************************************
*END*-----------------------------------------------------------------*/
static uint32_t  SOCK_DGRAM_bind
   (
      uint32_t                    sock,
         /* [IN] socket handle */
      const sockaddr     *localaddr,
         /* [IN] local address to which to bind the socket */
      uint16_t                    addrlen
         /* [IN] length of the address, in bytes */
   )
{
    UDP_PARM    parms;
    uint32_t    error;
    sockaddr    laddr = {0};
    SOCKET_STRUCT_PTR sock_struct_ptr = (SOCKET_STRUCT_PTR)sock;

    RTCS_ENTER(BIND, sock);

    if(sock_struct_ptr->STATE != SOCKSTATE_DGRAM_GROUND) 
    {
      RTCS_EXIT(BIND, RTCSERR_SOCK_IS_BOUND);
    }

    parms.ucb = sock_struct_ptr->UCB_PTR;
    SOCKADDR_copy(localaddr, &laddr); /* from->to */
    parms.saddr_ptr  = &laddr;
    parms.udpservice = UDP_process;
    
    error = RTCSCMD_issue(parms, UDP_bind);
    
    if(error) 
    {
      RTCS_EXIT(BIND, error);
    }

    sock_struct_ptr->STATE = SOCKSTATE_DGRAM_BOUND;

    RTCS_EXIT(BIND, RTCS_OK);
}

/*FUNCTION*-------------------------------------------------------------
************************************************************************
*
* Function Name   : SOCK_DGRAM_connect
* Returned Value  : error code
* Comments  :  Binds the remote endpoint of a socket. Support IPv6 and IPv4.
*
*     1. If connect succeeds the socket is in OPEN state.
*     2. If connect fails, the socket is in BOUND or GROUND state, and the
*        local IP is the same as it was immediately after the bind() call.
*
***********************************************************************
*END*-----------------------------------------------------------------*/
static uint32_t  SOCK_DGRAM_connect
   (
      uint32_t                    sock,
         /* [IN] socket handle */
      const struct sockaddr     *peeraddr,
         /* [IN] remote address to which to bind the socket */
      uint16_t                    addrlen
         /* [IN] length of the address, in bytes */
   )
{
    UDP_PARM  parms = {0};
    uint32_t  error = RTCS_OK;
    SOCKET_STRUCT_PTR sock_struct_ptr = (SOCKET_STRUCT_PTR)sock;
    uint16_t  state = sock_struct_ptr->STATE;
    sockaddr  raddr = {0};
      
    RTCS_ENTER(CONNECT, sock);
     
    /* If socket is unbound, bind it  SOCK_DGRAM_bind */
    if(state == SOCKSTATE_DGRAM_GROUND) 
    {
      /* as we clear sockaddr to zeroes, it means INADDR_ANY, port 0, scope_id 0 */
      struct sockaddr localaddr = {0};        
      localaddr.sa_family = sock_struct_ptr->AF;
            
      error = SOCK_DGRAM_bind(sock, &localaddr, sizeof(localaddr));
      if (error)  
      {
        RTCS_EXIT(CONNECT, error);
      }
    }

     /*
     ** Set socket in BOUND state and proceed normaly. If connect
     ** fails, leave the socket in BOUND state. If connect
     ** succeeds, set state to OPEN.
     */
     sock_struct_ptr->STATE = SOCKSTATE_DGRAM_BOUND;
     parms.ucb = sock_struct_ptr->UCB_PTR;
     memcpy(&raddr, peeraddr, sizeof(sockaddr));
     parms.saddr_ptr = &raddr;

   #if RTCSCFG_ENABLE_IP6
    #if RTCSCFG_ENABLE_IP4
    if (peeraddr->sa_family == AF_INET6) 
    {
    #endif
      /*
       * choose and add scope_id here. 
       * If peeraddr(target addr) scope_id is NULL we well use local(binded) scope_id 
       */
      if(0 == ((const sockaddr_in6 *)peeraddr)->sin6_scope_id)
      {
        ((sockaddr_in6*)&raddr)->sin6_scope_id = ((sockaddr_in6*)(&(sock_struct_ptr->UCB_PTR->LADDR)))->sin6_scope_id;
      }
    #if RTCSCFG_ENABLE_IP4
      }
    #endif 
  #endif
    error = RTCSCMD_issue(parms, UDP_connect);
      
    if(error) 
    {
      RTCS_EXIT(CONNECT, error);
    }

     /* Success */
    sock_struct_ptr->STATE = SOCKSTATE_DGRAM_OPEN;
    RTCS_EXIT(CONNECT, RTCS_OK);
}

/**
 * @brief Retrieve the address of the local endpoint of a bound socket.
 * 
 * Retrieve the address of the local endpoint of a bound socket. Support IPv6 and IPv4. 
 * The funciton blocks but the command is immediately serviced and replied to.
 * This function is called by getsockname(), if sock type is a datagram socket.
 * 
 * @param sock [IN] socket handle
 * @param name [OUT] pointer to struct sockaddr, where the address of a local endpoint
 *             will be written to.
 * @param namelen [IN/OUT] pointer to length of the address, in bytes. On input, 
                   the length determines the size of sockaddr structure.
                   On output, the length indicates number of bytes that have been
                   written to name.
 *                   
 * @return RTCS_OK (success)
 *         specific error code (failure).
 * @see SOCK_DGRAM_bind
 * @see SOCK_DGRAM_getpeername
 * @see SOCK_DGRAM_socket
 * @code
 * uint32_t handle;
 * sockaddr_in local_sin;
 * uint32_t status;
 * uint16_t namelen;
 * ...
 * namelen = sizeof (sockaddr_in);
 * status = getsockname(handle, (struct sockaddr *)&local_sin, &namelen);
 * if (status != RTCS_OK)
 * {
 *   printf(“\nError, getsockname() failed with error code %lx”, status);
 * } else {
 *   printf(“\nLocal address family is %x”, local_sin.sin_family);
 *   printf(“\nLocal port is %d”, local_sin.sin_port);
 *   printf(“\nLocal IP address is %lx”, local_sin.sin_addr.s_addr);
 * }
 * @endcode
 */
static uint32_t  SOCK_DGRAM_getsockname
   (
      uint32_t              sock,
         /* [IN] socket handle */
      sockaddr        *name,
         /* [OUT] address of local endpoint */
      uint16_t         *namelen
         /* [IN/OUT] length of the address, in bytes */
   )
{
    SOCKET_STRUCT_PTR sock_struct_ptr = (SOCKET_STRUCT_PTR)sock;
    RTCS_ENTER(GETSOCKNAME, sock);

    if(sock_struct_ptr->STATE == SOCKSTATE_DGRAM_GROUND) 
    {
      RTCS_EXIT(GETSOCKNAME, RTCSERR_SOCK_NOT_BOUND);
    }
  
    SOCKADDR_return_addr(sock_struct_ptr, name, &sock_struct_ptr->UCB_PTR->LADDR, namelen);
    RTCS_EXIT(GETSOCKNAME, RTCS_OK);
} 


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_DGRAM_getpeername
* Returned Value  : error code
* Comments  :  Retrieve the address of the peer endpoint of a
*              connected socket.
*
*END*-----------------------------------------------------------------*/
static uint32_t  SOCK_DGRAM_getpeername
   (
      uint32_t              sock,
         /* [IN] socket handle */
      sockaddr     *name,
         /* [OUT] address of peer endpoint */
      uint16_t         *namelen
         /* [IN/OUT] length of the address, in bytes */
   )
{
    SOCKET_STRUCT_PTR sock_struct_ptr = (SOCKET_STRUCT_PTR)sock;
    RTCS_ENTER(GETPEERNAME, sock);

    if(sock_struct_ptr->STATE != SOCKSTATE_DGRAM_OPEN) 
    {
      RTCS_EXIT(GETPEERNAME, RTCSERR_SOCK_NOT_CONNECTED);
    }
    
    SOCKADDR_return_addr(sock_struct_ptr, name, &sock_struct_ptr->UCB_PTR->RADDR, namelen);   
    RTCS_EXIT(GETPEERNAME, RTCS_OK);
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_DGRAM_recv
* Returned Value  : number of bytes received or RTCS_ERROR
* Comments  :  Receives data from a socket.
*
*END*-----------------------------------------------------------------*/

static int32_t  SOCK_DGRAM_recv
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
{
    int32_t   len;

    RTCS_ENTER(RECV, sock);
    len = SOCK_DGRAM_recvfrom(sock, buffer, buflen, flags, NULL, NULL);
    RTCS_EXIT2(RECV, (len < 0) ? RTCS_ERROR : RTCS_OK, len);
}

/**
 * @brief Receive data from a datagram socket.
 * 
 * Provides RTCS with the buffer in which to place data that is incoming on the datagram
 * socket. Supports IPv6 and IPv4 family. 
 * If a remote endpoint has been specified with connect(), only datagrams from that source
 * will be received.
 * When the flags parameter is RTCS_MSG_PEEK, the same datagram is received the next time recv() or
 * recvfrom() is called.
 * If addrlen is NULL, the socket address is not written to sourceaddr. If sourceaddr is NULL and the value of
 * addrlen is not NULL, the result is unspecified.
 * If the function returns RTCS_ERROR, the application can call RTCS_geterror() to determine the reason
 * for the error.
 * This function blocks (unles nonblocking option has been set for the socket) until data is available 
 * or an error or a timeout occurs.
 *
 * 
 * @param sock [IN] socket handle.
 * @param buffer [IN] pointer to a buffer for received data.
 * @param buflen [IN] size of the buffer in bytes.
 * @param flags [IN] flags to underlying protocols. One of the following:
 *              RTCS_MSG_PEEK - receives a datagram bug does not consume it.
 *              Zero - ignore.
 * @param sourceaddr [IN/OUT] 
 * @param addrlen [IN/OUT] 
 * @return number of bytes received (success)
 *         RTCS_ERROR (failure).
 * @see SOCK_DGRAM_bind
 * @see RTCS_geterror
 * @see SOCK_DGRAM_sendto
 * @code 
 * uint32_t handle;
 * sockaddr_in remote_sin;
 * uint32_t count;
 * char my_buffer[500];
 * uint16_t remote_len = sizeof(remote_sin);
 * ...
 * count = recvfrom(handle, my_buffer, sizeof(my_buffer), 0, (struct sockaddr *) &remote_sin, &remote_len);
 * if (count == RTCS_ERROR)
 * {
 *   printf(“\nrecvfrom() failed with error %lx”,
 *   RTCS_geterror(handle));
 * } else {
 * printf(“\nReceived %ld bytes of data.”, count);
 * }
 * @endcode
 */
static int32_t  SOCK_DGRAM_recvfrom
   (
      uint32_t              sock,
         /* [IN] socket handle */
      void                *buffer,
         /* [IN] buffer for received data */
      uint32_t              buflen,
         /* [IN] length of the buffer, in bytes */
      uint32_t              flags,
         /* [IN] flags to underlying protocols */
               
      struct sockaddr  *sourceaddr,
         /* [OUT] address from which data was received */
      uint16_t         *addrlen
         /* [IN/OUT] length of the address, in bytes */
   )
{
    UDP_PARM          parms = {0};
    uint32_t          error;
    struct sockaddr   addr_from = {0};
    SOCKET_STRUCT_PTR sock_struct_ptr = (SOCKET_STRUCT_PTR)sock;

    RTCS_ENTER(RECVFROM, sock);

    if(sock_struct_ptr->STATE == SOCKSTATE_DGRAM_GROUND) 
    {
      RTCS_setsockerror(sock, RTCSERR_SOCK_NOT_BOUND);
      RTCS_EXIT2(RECVFROM, RTCSERR_SOCK_NOT_BOUND, RTCS_ERROR);
    }

    parms.ucb      = sock_struct_ptr->UCB_PTR;
    parms.udpptr   = buffer;
    parms.udpword  = buflen;
    parms.udpflags = flags;
    if(addrlen)
    {
      /* if addrlen pointer is NULL, nothing is to be written to sourceaddr. parms.saddr_ptr is NULL by default. 
       * if addrlen is a non NULL pointer, UDP code will indicate the remote peer address via *saddr_ptr
       */      
      parms.saddr_ptr = &addr_from;
    }
     
    error = RTCSCMD_issue(parms, UDP_receive);
      
    if(error) 
    {
      RTCS_setsockerror(sock, error);
      RTCS_EXIT2(RECVFROM, error, RTCS_ERROR);
    }
    
    /* write the address from which data was received */
    SOCKADDR_return_addr(sock_struct_ptr, sourceaddr, &addr_from, addrlen);     
    RTCS_EXIT2(RECVFROM, RTCS_OK, parms.udpword);
} 

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_DGRAM_send
* Returned Value  : number of bytes sent or RTCS_ERROR
* Comments  :  Send data to a connected socket.
*
*END*-----------------------------------------------------------------*/

static int32_t  SOCK_DGRAM_send
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
{
    int32_t len= -1 ; // it will generate error if AF != AF_INET or AF_INET6

    RTCS_ENTER(SEND, sock);
     
    len = SOCK_DGRAM_sendto(sock, buffer, buflen, flags, NULL, 0);
 
    RTCS_EXIT2(SEND, (len < 0) ? RTCS_ERROR : RTCS_OK, len);
}


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_DGRAM_sendto
* Returned Value  : number of bytes sent or RTCS_ERROR
* Comments  :  Send data to a socket.
*
*END*-----------------------------------------------------------------*/

static int32_t  SOCK_DGRAM_sendto
   (
      uint32_t              sock,
         /* [IN] socket handle */
      void                *send_buffer,
         /* [IN] data to transmit */
      uint32_t              buflen,
         /* [IN] length of the buffer, in bytes */
      uint32_t              flags,
         /* [IN] flags to underlying protocols */
      sockaddr     *destaddr,
         /* [IN] address to which to send data */
      uint16_t              addrlen
         /* [IN] length of the address, in bytes */
   )
{
    UDP_PARM    parms = {0};
    uint32_t    error = RTCS_OK;
    sockaddr    addr = {0};
    uint16_t    len = sizeof(addr);
    SOCKET_STRUCT_PTR sock_struct_ptr = (SOCKET_STRUCT_PTR)sock;

    RTCS_ENTER(SENDTO, sock);
    
    if(!destaddr) /* called from SOCK_DGRAM_send() */
    {
      error = SOCK_DGRAM_getpeername(sock, &addr, &len);
      if(error) 
      {
        RTCS_setsockerror(sock, RTCSERR_SOCK_NOT_CONNECTED);
        RTCS_EXIT2(SENDTO, RTCSERR_SOCK_NOT_CONNECTED, RTCS_ERROR);
      }
      destaddr = &addr;
    } 
    parms.saddr_ptr = destaddr;
     
    if(sock_struct_ptr->STATE == SOCKSTATE_DGRAM_GROUND) 
    {
      sockaddr localaddr = {0};
      localaddr.sa_family = sock_struct_ptr->AF;
      error = SOCK_DGRAM_bind(sock,(sockaddr *)&localaddr, sizeof(localaddr));
      if(error) 
      {
        RTCS_setsockerror(sock, error);
        RTCS_EXIT2(SENDTO, error, RTCS_ERROR);
      }
    }
     
    parms.ucb       = sock_struct_ptr->UCB_PTR;
    parms.udpptr    = send_buffer;
    parms.udpword   = buflen;
    parms.udpflags  = flags;

    error = RTCSCMD_issue(parms, UDP_send);

    if(error) 
    {
      RTCS_setsockerror(sock, error);
      RTCS_EXIT2(SENDTO, error, RTCS_ERROR);
    }

    RTCS_EXIT2(SENDTO, RTCS_OK, buflen);
}

#else

const RTCS_SOCKET_CALL_STRUCT SOCK_DGRAM_CALL; /* for SOCK_DGRAM, TBD fixed other way.*/

#endif 
/* EOF */
