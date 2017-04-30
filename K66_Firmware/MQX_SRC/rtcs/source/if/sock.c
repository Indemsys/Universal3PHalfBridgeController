/*HEADER**********************************************************************
*
* Copyright 2014 Freescale Semiconductor, Inc.
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
*   This file contains the implementation bind(), connect(), listen(), accept(),
*   getpeername(), getsockname(), recv(), send(), recvfrom(), sendto().
*
*
*END************************************************************************/

#include <rtcs.h>
#include <socket.h>
#include <rtcs_prv.h>
#include <udp.h>
#include <tcp.h>

/* RTCS log */
#define RTCS_ENTER(f,a)    RTCSLOG_API(RTCSLOG_TYPE_FNENTRY, RTCSLOG_FN_SOCK_ ## f, a)

#define RTCS_EXIT(f,a)     RTCSLOG_API(RTCSLOG_TYPE_FNEXIT,  RTCSLOG_FN_SOCK_ ## f, a); \
                           return a

uint32_t bind(uint32_t sock, const sockaddr *addr, uint16_t addrlen)
{
  struct socket_struct *socket_ptr;
  uint32_t error;
  
  RTCS_ENTER(sock, BIND);
  
  /* check input parameters in user API */
  if(!SOCK_check_valid(sock))
  {
    RTCS_EXIT(BIND, RTCSERR_SOCK_INVALID);
  }  
  
  error = SOCKADDR_check_valid(sock, addr);
  if(error)
  {
    goto EXIT;
  }

  error = SOCKADDR_check_addr(addr, addrlen);
  if(error)
  {
    goto EXIT;
  }
  
  socket_ptr = (struct socket_struct *)sock;
  if(
      (FALSE == SOCK_PROTOCOL_exists_in_system(socket_ptr->PROTOCOL)) 
      || (NULL == socket_ptr->PROTOCOL->SOCK_BIND)
    )
  {
    RTCS_setsockerror(sock, RTCSERR_SOCK_NOT_SUPPORTED);
    RTCS_EXIT(BIND, RTCSERR_SOCK_NOT_SUPPORTED);
  }
  
  /* input arguments look fine, proceed */
  error = socket_ptr->PROTOCOL->SOCK_BIND(sock, addr, addrlen);
EXIT:
  RTCS_EXIT(BIND, error);
}

uint32_t connect(uint32_t sock, const sockaddr *addr, uint16_t addrlen)
{
  struct socket_struct *socket_ptr;
  uint32_t error;
  
  RTCS_ENTER(sock, CONNECT);
  
  /* check input parameters in user API */
  if(!SOCK_check_valid(sock))
  {
    RTCS_EXIT(CONNECT, RTCSERR_SOCK_INVALID);
  }  
  
  error = SOCKADDR_check_valid(sock, addr);
  if(error)
  {
    goto EXIT;
  }

  error = SOCKADDR_check_addr(addr, addrlen);
  if(error)
  {
    goto EXIT;
  }
  
  socket_ptr = (struct socket_struct *)sock;
  if(
      (FALSE == SOCK_PROTOCOL_exists_in_system(socket_ptr->PROTOCOL))
      || (NULL == socket_ptr->PROTOCOL->SOCK_CONNECT)
    )
  {
    RTCS_setsockerror(sock, RTCSERR_SOCK_NOT_SUPPORTED);
    RTCS_EXIT(CONNECT, RTCSERR_SOCK_NOT_SUPPORTED);
  }
  
  /* input arguments look fine, proceed */
  error = socket_ptr->PROTOCOL->SOCK_CONNECT(sock, addr, addrlen);
EXIT:
  RTCS_EXIT(CONNECT, error);
}

int32_t recv(uint32_t sock, void *buffer, uint32_t buflen, uint32_t flags)
{
  struct socket_struct *socket_ptr;
  int32_t error;
  
  RTCS_ENTER(sock, RECV);
  
  /* check input parameters in user API */
  error = RTCS_ERROR;
  
  if(!SOCK_check_valid(sock))
  {
    goto RETURN;
  }
  
  /* check if recv() is disallowed by shutdownsocket() */
  socket_ptr = (struct socket_struct *)sock;
  if(SOCK_disallow_recv(socket_ptr->disallow_mask))
  {
    RTCS_setsockerror(sock, RTCSERR_SOCK_ESHUTDOWN);
    goto RETURN;
  }
  
  /* check if the protocol is right */  
  if(
      (FALSE == SOCK_PROTOCOL_exists_in_system(socket_ptr->PROTOCOL))
      || (NULL == socket_ptr->PROTOCOL->SOCK_RECV)
    )
  {
    RTCS_setsockerror(sock, RTCSERR_SOCK_NOT_SUPPORTED);
    goto RETURN;
  }
  
  /* input arguments look fine, proceed */
  error = socket_ptr->PROTOCOL->SOCK_RECV(sock, buffer, buflen, flags);
RETURN:
  RTCS_EXIT(RECV, error);
}

int32_t recvfrom(uint32_t sock, void *buffer, uint32_t buflen, uint32_t flags, sockaddr *sourceaddr, uint16_t *addrlen)
{
  struct socket_struct *socket_ptr;
  int32_t error;
  
  RTCS_ENTER(sock, RECVFROM);
  
  /* check input parameters in user API */
  error = RTCS_ERROR;
  
  if(!SOCK_check_valid(sock))
  {
    goto RETURN;
  }
  
  /* check if recv() is disallowed by shutdownsocket() */
  socket_ptr = (struct socket_struct *)sock;
  if(SOCK_disallow_recv(socket_ptr->disallow_mask))
  {
    RTCS_setsockerror(sock, RTCSERR_SOCK_ESHUTDOWN);
    goto RETURN;
  }
  
  if((NULL == buffer)  || (0 == buflen))
  {
    RTCS_setsockerror(sock, RTCSERR_SOCK_INVALID_PARAMETER);
    goto RETURN;    
  }
  
  /* check if the protocol is right */
  if(
      (FALSE == SOCK_PROTOCOL_exists_in_system(socket_ptr->PROTOCOL))
      || (NULL == socket_ptr->PROTOCOL->SOCK_RECVFROM)
    )
  {
    RTCS_setsockerror(sock, RTCSERR_SOCK_NOT_SUPPORTED);
    goto RETURN;
  }
  
  /* input arguments look fine, proceed */
  error = socket_ptr->PROTOCOL->SOCK_RECVFROM(sock, buffer, buflen, flags, sourceaddr, addrlen);
RETURN:
  RTCS_EXIT(RECVFROM, error);
}


int32_t send(uint32_t sock, void *buffer, uint32_t buflen, uint32_t flags)
{
  struct socket_struct *socket_ptr;
  int32_t error;
  
  RTCS_ENTER(sock, SEND);
  
  /* check input parameters in user API */
  error = RTCS_ERROR;
  
  if(!SOCK_check_valid(sock))
  {
    goto RETURN;
  }
  
  /* check if send() is disallowed by shutdownsocket() */
  socket_ptr = (struct socket_struct *)sock;
  if(SOCK_disallow_send(socket_ptr->disallow_mask))
  {
    RTCS_setsockerror(sock, RTCSERR_SOCK_ESHUTDOWN);
    goto RETURN;
  }
  
  /* check if the protocol is right */
  if(
      (FALSE == SOCK_PROTOCOL_exists_in_system(socket_ptr->PROTOCOL))
      || (NULL == socket_ptr->PROTOCOL->SOCK_SEND)
    )
  {
    RTCS_setsockerror(sock, RTCSERR_SOCK_NOT_SUPPORTED);
    goto RETURN;
  }
  
  /* input arguments look fine, proceed */
  error = socket_ptr->PROTOCOL->SOCK_SEND(sock, buffer, buflen, flags);
RETURN:
  RTCS_EXIT(SEND, error);
}

int32_t sendto(uint32_t sock, void *send_buffer, uint32_t buflen, uint32_t flags, sockaddr *destaddr, uint16_t addrlen)
{
  struct socket_struct *socket_ptr;
  int32_t error;
  uint32_t errcode;
  
  RTCS_ENTER(sock, SENDTO);
  
  /* check input parameters in user API */
  error = RTCS_ERROR;
  
  if(!SOCK_check_valid(sock))
  {
    goto RETURN;
  }   
  
  /* check if send() is disallowed by shutdownsocket() */
  socket_ptr = (struct socket_struct *)sock;
  if(SOCK_disallow_send(socket_ptr->disallow_mask))
  {
    RTCS_setsockerror(sock, RTCSERR_SOCK_ESHUTDOWN);
    goto RETURN;
  }
  
  if((NULL == send_buffer)  || (0 == buflen))
  {
    RTCS_setsockerror(sock, RTCSERR_SOCK_INVALID_PARAMETER);
    goto RETURN;    
  }
  
  /* destaddr should be given right */
  errcode = SOCKADDR_check_valid(sock, destaddr);
  if(errcode)
  {
    RTCS_setsockerror(sock, errcode);
    goto RETURN;
  }

  errcode = SOCKADDR_check_addr(destaddr, addrlen);
  if(errcode)
  {
    RTCS_setsockerror(sock, errcode);
    goto RETURN;
  }
  
  /* check if the protocol is right */
  if(
      (FALSE == SOCK_PROTOCOL_exists_in_system(socket_ptr->PROTOCOL))
      || (NULL == socket_ptr->PROTOCOL->SOCK_SENDTO)
    )
  {
    RTCS_setsockerror(sock, RTCSERR_SOCK_NOT_SUPPORTED);
    goto RETURN;
  }
  
  /* input arguments look fine, proceed */
  error = socket_ptr->PROTOCOL->SOCK_SENDTO(sock, send_buffer, buflen, flags, destaddr, addrlen);
RETURN:
  RTCS_EXIT(SENDTO, error);
}

uint32_t getsockname(uint32_t sock, struct sockaddr *addr, uint16_t *addrlen)
{
  struct socket_struct *socket_ptr;
  
  RTCS_ENTER(sock, GETSOCKNAME);
  
  /* check input parameters in user API */
  if(!SOCK_check_valid(sock))
  {
    RTCS_EXIT(GETSOCKNAME, RTCSERR_SOCK_INVALID);
  }
  
  if((NULL == addrlen) || (NULL == addr))
  {
    RTCS_EXIT(GETSOCKNAME, RTCSERR_INVALID_PARAMETER);
  }  
  
  socket_ptr = (struct socket_struct *)sock;
  if(
       (FALSE == SOCK_PROTOCOL_exists_in_system(socket_ptr->PROTOCOL)) 
        || (NULL == socket_ptr->PROTOCOL->SOCK_GETSOCKNAME)
    )
  {
    RTCS_setsockerror(sock, RTCSERR_SOCK_NOT_SUPPORTED);
    RTCS_EXIT(GETSOCKNAME, RTCSERR_SOCK_NOT_SUPPORTED);
  }
  
  /* input arguments look fine, proceed */
  RTCS_EXIT(GETSOCKNAME, socket_ptr->PROTOCOL->SOCK_GETSOCKNAME(sock, addr, addrlen));
}


uint32_t getpeername(uint32_t sock, struct sockaddr *addr, uint16_t *addrlen)
{
  struct socket_struct *socket_ptr;
  
  RTCS_ENTER(sock, GETPEERNAME);
  
  /* check input parameters in user API */
  if(!SOCK_check_valid(sock))
  {
    RTCS_EXIT(GETPEERNAME, RTCSERR_SOCK_INVALID);
  }
  
  if((NULL == addrlen) || (NULL == addr))
  {
    RTCS_EXIT(GETPEERNAME, RTCSERR_INVALID_PARAMETER);
  }  
  
  socket_ptr = (struct socket_struct *)sock;
  if(
      (FALSE == SOCK_PROTOCOL_exists_in_system(socket_ptr->PROTOCOL)) || 
      (NULL == socket_ptr->PROTOCOL->SOCK_GETPEERNAME)
    )
  {
    RTCS_setsockerror(sock, RTCSERR_SOCK_NOT_SUPPORTED);
    RTCS_EXIT(GETPEERNAME, RTCSERR_SOCK_NOT_SUPPORTED);
  }
  
  /* input arguments look fine, proceed */
  RTCS_EXIT(GETPEERNAME, socket_ptr->PROTOCOL->SOCK_GETPEERNAME(sock, addr, addrlen));
}


uint32_t listen(uint32_t sock, int32_t backlog)
{
  struct socket_struct *socket_ptr;
  int32_t error;

  RTCS_ENTER(sock, LISTEN);  
  
  /* check input parameters in user API */
  error = RTCS_ERROR;
  
  if(!SOCK_check_valid(sock))
  {
    goto RETURN;
  }
    
  socket_ptr = (struct socket_struct *)sock;
  /* check if the protocol is right */
  if(
      (FALSE == SOCK_PROTOCOL_exists_in_system(socket_ptr->PROTOCOL))
      || (NULL == socket_ptr->PROTOCOL->SOCK_LISTEN)
    )
  {
    RTCS_setsockerror(sock, RTCSERR_SOCK_NOT_SUPPORTED);
    goto RETURN;
  }
  
  /* input arguments look fine, proceed */
  error = socket_ptr->PROTOCOL->SOCK_LISTEN(sock, backlog);
RETURN:
  RTCS_EXIT(LISTEN, error);
}

uint32_t accept(uint32_t sock, sockaddr *addr, uint16_t *addrlen)
{
  struct socket_struct *socket_ptr;
  int32_t error;

  RTCS_ENTER(sock, ACCEPT);  
  
  /* check input parameters in user API */
  error = RTCS_ERROR;
  
  if(!SOCK_check_valid(sock))
  {
    goto RETURN;
  }
    
  socket_ptr = (struct socket_struct *)sock;
  /* check if the protocol is right */
  if(
      (FALSE == SOCK_PROTOCOL_exists_in_system(socket_ptr->PROTOCOL))
      || (NULL == socket_ptr->PROTOCOL->SOCK_ACCEPT)
    )
  {
    RTCS_setsockerror(sock, RTCSERR_SOCK_NOT_SUPPORTED);
    goto RETURN;
  }
  
  /* input arguments look fine, proceed */
  error = socket_ptr->PROTOCOL->SOCK_ACCEPT(sock, addr, addrlen);
RETURN:
  RTCS_EXIT(ACCEPT, error);
}

