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
*   This file contains the implementation of shutdownsocket() and closesocket()
*
*
*END************************************************************************/

#include <rtcs.h>
#include <socket.h>
#include <rtcs_prv.h>
#include <udp.h>
#include <tcp.h>

#define RTCS_ENTER(f,a)    RTCSLOG_API(RTCSLOG_TYPE_FNENTRY, RTCSLOG_FN_SOCK_ ## f, a)

#define RTCS_EXIT(f,a)     RTCSLOG_API(RTCSLOG_TYPE_FNEXIT,  RTCSLOG_FN_SOCK_ ## f, a); \
                           return a

static int32_t shutdownsocket_stream(struct socket_struct *socket_ptr, uint32_t disallow_mask)
{
  int32_t error = RTCS_OK;
  
  if(NULL == socket_ptr->TCB_PTR)
  {
    return error; /* RTCS_OK. */
  }
  
  struct tcb_parm parms;
  parms.TCB_PTR = socket_ptr->TCB_PTR;
  parms.SOCKET = (uint32_t)socket_ptr;
  parms.OPCODE = 1; /* 1 when TCP_Process_close() is called from TCP_Process_shutdown() */
  parms.disallow_mask = disallow_mask;
  error = RTCSCMD_issue(parms, TCP_Process_shutdown);  
  
  return error;
}

static int32_t shutdownsocket_dgram(struct socket_struct *socket_ptr, uint32_t disallow_mask)
{
  struct ucb_parm parms;
  parms.ucb = socket_ptr->UCB_PTR;
  parms.udpword = disallow_mask;
  return RTCSCMD_issue(parms, UDP_shutdown);
}

int32_t shutdownsocket(uint32_t sock, int32_t how)
{
  uint32_t  error;
  struct socket_struct *socket_ptr;
  uint32_t  disallow_mask;
  
  RTCS_ENTER(SHUTDOWNSOCKET, sock);
  error = RTCS_OK;
  disallow_mask = 0;
  
  /* check input parameters in user API */
  if(!SOCK_check_valid(sock))
  {
    RTCS_EXIT(SHUTDOWNSOCKET, RTCSERR_SOCK_INVALID);
  }
  
  /* set disallow_mask for the socket. this is used to disallow further send/receive requests from application. */
  socket_ptr = (struct socket_struct *)sock;
  if(socket_ptr->disallow_mask & (1 << SHUT_RDWR))
  {
    goto exit;
  }
  
  switch(how)
  {
    case SHUT_RD:
    case SHUT_WR:
    case SHUT_RDWR:
      disallow_mask = (1 << how);
    break;
    
    default:
      error = RTCSERR_INVALID_PARAMETER;
    break;
  }
  
  if(RTCS_OK != error)
  {
    goto exit;
  }
  
  if(((uint32_t)socket_ptr->PROTOCOL) == SOCK_STREAM)
  {
    error = shutdownsocket_stream(socket_ptr, disallow_mask);
  } 
  else if(((uint32_t)socket_ptr->PROTOCOL) == SOCK_DGRAM)
  {
    error = shutdownsocket_dgram(socket_ptr, disallow_mask);
  }
  else
  {
    error = RTCSERR_SOCK_INVALID;
  }
  
exit:
  RTCS_EXIT(SHUTDOWNSOCKET, error);
}

int32_t closesocket(uint32_t sock)
{
  uint32_t  error;
  struct socket_struct *socket_ptr;
  
  RTCS_ENTER(CLOSESOCKET, sock);  
  error = RTCS_OK;
  
  /* check input parameters in user API */
  _task_stop_preemption();
  if(!SOCK_check_valid(sock))
  {
    _task_start_preemption();
    RTCS_EXIT(CLOSESOCKET, RTCSERR_SOCK_INVALID);
  }
  
  socket_ptr = (struct socket_struct *)sock;
    
  if(((uint32_t)socket_ptr->PROTOCOL) == SOCK_DGRAM)
  {    
    struct ucb_parm parms;
    
    parms.ucb = socket_ptr->UCB_PTR;
    _task_start_preemption();
    error = RTCSCMD_issue(parms, UDP_close);
  }
  else if(((uint32_t)socket_ptr->PROTOCOL) == SOCK_STREAM)
  {
    struct tcb_parm parms;
    
    if(NULL == socket_ptr->TCB_PTR)
    {
      goto exit;
    }
    
    parms.TCB_PTR = socket_ptr->TCB_PTR;
    
    /* for stream socket, the behavior depends on the linger socket option. */
    if((0 != socket_ptr->so_linger.l_onoff) && (0 == socket_ptr->so_linger.l_linger_ms))
    {
      /* hard (sends RST to connected host). */
      _task_start_preemption();
      do
      {
          /* for socket close process, make sure RTCS_cmd_issue() sends the command to the TCP/IP task. */
          error = RTCSCMD_issue(parms, TCP_Process_abort);
      } while ((RTCSERR_OUT_OF_BUFFERS == error) || (RTCSERR_SEND_FAILED == error));
    }
    else 
    {      
      /* graceful close. */
      /* so_linger.l_onoff controls whether TCP_Process_close() blocks and for how long, or it does not block */
      /* flag for the TCP_Process_close(). if zero, it is called from here. otherwise it is called from shutdownsocket(). */
      parms.OPCODE = 0;
      _task_start_preemption();
      do
      {
          /* for socket close process, make sure RTCS_cmd_issue() sends the command to the TCP/IP task. */
          error = RTCSCMD_issue(parms, TCP_Process_close);
      } while ((RTCSERR_OUT_OF_BUFFERS == error) || (RTCSERR_SEND_FAILED == error));
    }
  }    
  else 
  {
    _task_start_preemption();
    RTCS_EXIT(CLOSESOCKET, RTCSERR_SOCK_INVALID);
  }
exit:
  SOCK_Free_sock_struct(socket_ptr);
  RTCS_EXIT(CLOSESOCKET, error);
}


