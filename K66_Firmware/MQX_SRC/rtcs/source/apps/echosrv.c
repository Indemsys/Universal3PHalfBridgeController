/*HEADER**********************************************************************
*
* Copyright 2008, 2014 Freescale Semiconductor, Inc.
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
*   This file contains an implementation of an
*   RFC 862 Echo server.
*
*
*END************************************************************************/
#include <rtcs.h>

/* if RTCSCFG_ECHOSRV_DEBUG_MESSAGES is defined as TRUE, ECHOSRV
 *  prints status messages to stdout */
#if RTCSCFG_ECHOSRV_DEBUG_MESSAGES==1
  #define ECHOSRV_DEBUG(x) x
#if MQX_USE_IO_OLD
  #include <fio.h>
#else
  #include <stdio.h>
#endif
#else
  #define ECHOSRV_DEBUG(x) 
#endif

#define ECHOSRV_BUFFER_SIZE  RTCSCFG_ECHOSRV_DEFAULT_BUFLEN
#define ECHO_TASK_STACK      (2000)
static const char * const ECHO_TASK_NAME = "ECHO_server";

typedef struct echosrv_context
{
  uint32_t       udpsock[2];
  volatile uint32_t       listensock[2];
  uint32_t       clientsock[RTCSCFG_ECHOSRV_MAX_TCP_CLIENTS];
  int32_t        num_clients;
  int32_t        num_listensock;
  int32_t        num_udpsock;
  char *         buffer;
  volatile bool  b_run;
} ECHOSRV_CONTEXT, * ECHOSRV_CONTEXT_PTR;

/*
 * Parameters for server task
 */
typedef struct echosrv_server_task_param
{
    void*              handle;    /* [out] Server handle, non-zero if initialization was successful */
    ECHOSRV_PARAM_STRUCT *params; /* [in] Server parameters */
}ECHOSRV_SERVER_TASK_PARAM;

static void ECHOSRV_task(void*, void*);
static void ServerStop(ECHOSRV_CONTEXT_PTR echo_ptr);

/*
 * 3 static functions below are just for management of an array of uint32_t
 * socket handles
 */

/*
 * Add accepted client socket to the array of socket handles.
 * Scan starts from index zero and first non-zero array member is used.
 */
static void AddItem(const uint32_t sock, uint32_t * array_ptr, const int32_t size, int32_t * num_ptr)
{
  int32_t i = 0;
  if(NULL == array_ptr) return;
  if(NULL == num_ptr) return;
  for(i=0; i<size; i++)
  {
    if(0 == array_ptr[i])
    {
      array_ptr[i] = sock;
      (*num_ptr)++;
      break;
    }
  }
}

/*
 * Function RemoveItem() might put zero in the middle of the array.
 * We need to keep the array filled from index 0 without gaps of zero(s).
 */
static void NormalizeArray(uint32_t * array_ptr, const int32_t size, int32_t * num_ptr)
{
  int32_t i = 0;
  int32_t nonzero_cnt = 0;
    
  if((NULL != array_ptr)&&(NULL != num_ptr))
  {
    /*
     * get only non-zero array members.     
     */
    for(i=0; i<size; i++)
    {
      if(array_ptr[i])
      { 
        if(i > nonzero_cnt)
        {
          array_ptr[nonzero_cnt] = array_ptr[i];
          array_ptr[i] = 0; 
        }
        nonzero_cnt++;
      } 
    }    
    (*num_ptr) = nonzero_cnt;
  }
}

/*
 * Client has disconnected.
 * Clear it's socket handle from the clientsock[] array.
 * NormalizeArray() will take care that socket handles start at index 0
 * in the array and that there are no gaps (of zero values).
 */
static void RemoveItem(const uint32_t sock, uint32_t * array_ptr, const int32_t size, int32_t * num_ptr)
{
  int32_t i = 0;
  if(NULL == array_ptr) return;
  if(NULL == num_ptr) return;
  for(i=0; i<size; i++)
  {
    if(sock == array_ptr[i])
    {
      array_ptr[i] = 0;
      break;
    }
  }
  NormalizeArray(array_ptr, size, num_ptr);
}

static uint32_t InitEchoSocket(uint32_t af, uint32_t type,
                               ECHOSRV_CONTEXT_PTR pECHOSRV_CONTEXT,
                               ECHOSRV_PARAM_STRUCT * pECHOSRV_PARAM)
{
  sockaddr laddr;
  uint32_t        sock;
  uint32_t        error;

  /* create TCP or UDP socket and save it's handle */
  sock = socket(af, type, 0);
  if(type == SOCK_STREAM)
  {
    pECHOSRV_CONTEXT->listensock[pECHOSRV_CONTEXT->num_listensock++] = sock;
  }
  else
  {
    pECHOSRV_CONTEXT->udpsock[pECHOSRV_CONTEXT->num_udpsock++] = sock;
  }
  if(RTCS_SOCKET_ERROR == sock)
  {
    return RTCSERR_OUT_OF_SOCKETS;
  }
  _mem_zero(&laddr, sizeof(laddr));
#if RTCSCFG_ENABLE_IP4
  if(AF_INET == af)
  {
    ((sockaddr_in *)&laddr)->sin_port = pECHOSRV_PARAM->port;
    ((sockaddr_in *)&laddr)->sin_addr = pECHOSRV_PARAM->ipv4_address;
    ((sockaddr_in *)&laddr)->sin_family = AF_INET;
  }
 #if RTCSCFG_ENABLE_IP6
  else
 #endif
#endif

#if RTCSCFG_ENABLE_IP6
  if(AF_INET6 == af)
  {
    ((sockaddr_in6 *)&laddr)->sin6_port = pECHOSRV_PARAM->port;
    ((sockaddr_in6 *)&laddr)->sin6_family = AF_INET6;
    ((sockaddr_in6 *)&laddr)->sin6_scope_id = pECHOSRV_PARAM->ipv6_scope_id;
    ((sockaddr_in6 *)&laddr)->sin6_addr = pECHOSRV_PARAM->ipv6_address;
  }
#endif
  else
  {
    return RTCSERR_INVALID_PARAMETER;
  }

  /* bind on TCP or UDP */
  error = bind(sock, &laddr,sizeof(laddr));

  if(error)
  {
    closesocket(sock);
    return error;
  }

  if(SOCK_STREAM == type)
  {
    /* Listen on TCP port */
    error = listen(sock, 0);
    if(error)
    {
      closesocket(sock);
      return error;

    }
  }
  return RTCS_OK;
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name    : ECHOSRV_init
* Returned Value   : error code
* Comments  :  Start the port 7 echo server.
*
*END*-----------------------------------------------------------------*/
void* ECHOSRV_init(ECHOSRV_PARAM_STRUCT * params)
{
  ECHOSRV_SERVER_TASK_PARAM server_params = {0};

  if(NULL == params)
  {
    _task_set_error(RTCSERR_INVALID_PARAMETER);
    return 0;
  }

  server_params.params = params;

  /* Run server task. */
  if (RTCS_task_create((char*)ECHO_TASK_NAME, params->server_prio, 
                        ECHO_TASK_STACK, ECHOSRV_task, (void*)&server_params) 
      != RTCS_OK)
  {
    return(0);
  }
  return(server_params.handle);
}

static void echosrv_remove_client(ECHOSRV_CONTEXT_PTR echo_ptr, rtcs_fd_set * rfds_ptr, bool b_closesocket)
{
  int32_t num_clients_on_enter_loop = 0;
  int32_t i;
  uint32_t sock;
  
  num_clients_on_enter_loop = echo_ptr->num_clients;
  for(i=0; i<num_clients_on_enter_loop; i++)
  {
    sock = echo_ptr->clientsock[i];
    if(RTCS_FD_ISSET(sock, rfds_ptr))
    {
      if(b_closesocket)
      {
        closesocket(sock);
      }
      RemoveItem(sock, echo_ptr->clientsock, RTCSCFG_ECHOSRV_MAX_TCP_CLIENTS, &echo_ptr->num_clients);
    }
  }
}

/*TASK*-----------------------------------------------------------------
*
* Function Name    : ECHOSRV_task
* Returned Value   : none
* Comments  :  Simple Echo server.
*
*END*-----------------------------------------------------------------*/
static void ECHOSRV_task
   (
      void   *dummy,
      void   *creator
   )
{
  sockaddr raddr;
  uint32_t        sock;
  uint32_t        error;
  int32_t         length;
  uint16_t        rlen;

  sockaddr udpraddr;
  uint16_t udprlen;

  rtcs_fd_set        rfds;
  int32_t       retval;

  int32_t i;
  
  int32_t num_clients_on_enter_loop = 0;
  
  ECHOSRV_CONTEXT     echo_context =  { 0 };
  ECHOSRV_CONTEXT_PTR echo_ptr = &echo_context;
  
  ECHOSRV_SERVER_TASK_PARAM * server_task_params = (ECHOSRV_SERVER_TASK_PARAM *)dummy;
  server_task_params->handle = RTCS_task_getid(); /* TD of this task */
  
  /* Set up exit handler and context so that we can clean up if the Echo Server is terminated */
  _task_set_environment( _task_get_id(), (void *) echo_ptr );

  /* Initialize socket, bind and listen in case of a stream socket */
  if(server_task_params->params->af & AF_INET)
  {
    error = InitEchoSocket(AF_INET, SOCK_STREAM, echo_ptr, server_task_params->params);
    if(error)
    {
      RTCS_task_exit(creator, error);
    }
    error = InitEchoSocket(AF_INET, SOCK_DGRAM, echo_ptr, server_task_params->params);
    if(error)
    {
      RTCS_task_exit(creator, error);
    }
  }

  if(server_task_params->params->af & AF_INET6)
  {
    error = InitEchoSocket(AF_INET6, SOCK_STREAM, echo_ptr, server_task_params->params);
    if(error)
    {
      RTCS_task_exit(creator, error);
    }
    error = InitEchoSocket(AF_INET6, SOCK_DGRAM, echo_ptr, server_task_params->params);
    if(error)
    {
      RTCS_task_exit(creator, error);
    }
  }
  
  /* allocate buffer for data */
  echo_ptr->buffer = RTCS_mem_alloc_system(ECHOSRV_BUFFER_SIZE);
  if(NULL == echo_ptr->buffer)
  {
    RTCS_task_exit(creator, RTCSERR_OUT_OF_MEMORY);
  }
  _mem_set_type(echo_ptr->buffer, MEM_TYPE_ECHOSRV_DATA_BUFFER);
  
  echo_ptr->b_run = TRUE;
  RTCS_task_resume_creator(creator, RTCS_OK);

  for(;;)
  { 
    /*
     * echosrv run
     */
    
    /* add socket handles to fd_set to select on inbound activity */
    RTCS_FD_ZERO(&rfds);
    for(i=0; i<echo_ptr->num_listensock; i++)
    {
      RTCS_FD_SET(echo_ptr->listensock[i],&rfds);
    }
    for(i=0; i<echo_ptr->num_udpsock; i++)
    {
      RTCS_FD_SET(echo_ptr->udpsock[i],&rfds);
    }
    for(i=0;i<echo_ptr->num_clients;i++)
    {
      RTCS_FD_SET(echo_ptr->clientsock[i],&rfds);
    }

    /* 
     * select on all client sockets plus TCP listen socket and UDP socket.
     */
    retval = select(echo_ptr->num_listensock
                    + echo_ptr->num_udpsock
                    + echo_ptr->num_clients, &rfds, 0, 0, 0);

    if(RTCS_ERROR == retval)
    {
      uint32_t errorcode = RTCS_get_errno();      
      
      if(RTCSERR_SOCK_ESHUTDOWN == errorcode)
      {
        ECHOSRV_DEBUG(fputs("RTCSERR_SOCK_ESHUTDOWN!\n", stderr));
        
        /*
         * other task call shutdownsocket() on listening sockets
         * or remote host resets established connection.
         */
        
        /*
         * On listening sockets, we interpret this as an application command to stop server & sessions.
         */
        for(i=0; i<echo_ptr->num_listensock; i++)
        {
          if(RTCS_FD_ISSET(echo_ptr->listensock[i], &rfds))
          {
            ServerStop(echo_ptr);
          }
        }
        
        /*
         * a connected client has disconnected. (sends RST to us).
         * Check which one and remove it from the list of clients.
         * we shall also call closesocket().
         */
        echosrv_remove_client(echo_ptr, &rfds, TRUE /* remove and closesocket(). */);
      }
      else if(RTCSERR_SOCK_CLOSED == errorcode)
      {
        /* this code is set when other task calls closesocket(). */
        ECHOSRV_DEBUG(fputs("RTCSERR_SOCK_CLOSED!\n", stderr));
        /*
         * other task calls hard closesocket() (abort). 
         * Check which one and remove it from the list of clients.
         */
        echosrv_remove_client(echo_ptr, &rfds, FALSE /* remove only. socket is already closed. */);
      }
      else
      {
        ECHOSRV_DEBUG(fprintf(stderr, "\nselect() failed: errno 0x%x\nechosrv ceased\n",errorcode));
        return; /* return from task body -> exit handler is called */
      }
    }
    else if(0 == retval)
    {
      /* 
       * timeout on select() has occured. 
       * 
       */
    }
    else
    {
      /* handle stream listening sockets */
      for(i=0; i<echo_ptr->num_listensock; i++)
      {
        sock = echo_ptr->listensock[i];
        if(RTCS_FD_ISSET(sock,&rfds))
        {
          /* Connection requested; accept it;
           * check that we can accept one more. */
          if(echo_ptr->num_clients < RTCSCFG_ECHOSRV_MAX_TCP_CLIENTS)
          {
            rlen = sizeof(raddr);
            sock = accept(sock, &raddr, &rlen);
            if(RTCS_SOCKET_ERROR == sock)
            {
              uint32_t errorcode = RTCS_get_errno();
              if(errorcode == RTCSERR_SOCK_ESHUTDOWN)
              {
                ServerStop(echo_ptr);
              }
              _time_delay(5000);
            }
            else
            {
              AddItem(sock, echo_ptr->clientsock, RTCSCFG_ECHOSRV_MAX_TCP_CLIENTS, &echo_ptr->num_clients);
            }
          }
          else
          {
            ECHOSRV_DEBUG(fputs("Sorry, max no of clients have active connection!\n", stderr));
          }
        }
      }
      /* handle connected stream client sockets */
      num_clients_on_enter_loop = echo_ptr->num_clients;
      for(i=0; i<num_clients_on_enter_loop; i++)
      {
        sock = echo_ptr->clientsock[i];
        if(RTCS_FD_ISSET(sock,&rfds))
        {
          length = recv(sock, echo_ptr->buffer, ECHOSRV_BUFFER_SIZE, 0);
          if (length == RTCS_ERROR)
          {
            closesocket(sock);
            RemoveItem(sock, echo_ptr->clientsock, RTCSCFG_ECHOSRV_MAX_TCP_CLIENTS, &echo_ptr->num_clients);
          }
          else
          {
            send(sock, echo_ptr->buffer, length, 0);
          }
        }
      }
      /* handle datagram sockets */
      for(i=0; i<echo_ptr->num_udpsock; i++)
      {
        sock = echo_ptr->udpsock[i];
        if(RTCS_FD_ISSET(sock,&rfds))
        {
          udprlen = sizeof(udpraddr);
          length = recvfrom(sock, echo_ptr->buffer, ECHOSRV_BUFFER_SIZE, 0, &udpraddr, &udprlen);
          if(length == RTCS_ERROR)
          {
            ECHOSRV_DEBUG(printf("oops"));
          }
          else
          {
            sendto(sock, echo_ptr->buffer, length, 0, &udpraddr, udprlen);
          }
        }
      }
    }
  } /* for(;;) */
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name    : ECHOSRV_stop
* Returned Value   : error code
* Comments  :  Stop the ECHO task and release sockets and memory.
*              Called by user application.
*
*END*-----------------------------------------------------------------*/
uint32_t ECHOSRV_release(void* server_h)
{
  _task_id ECHOSRV_task_id;
  int32_t i;
  ECHOSRV_CONTEXT_PTR echo_context_ptr;

  ECHOSRV_task_id = RTCS_task_id_from_td(server_h);
  if(MQX_NULL_TASK_ID == ECHOSRV_task_id)
  {
    return RTCSERR_SERVER_NOT_RUNNING;
  }
  
  echo_context_ptr = (ECHOSRV_CONTEXT_PTR)_task_get_environment(ECHOSRV_task_id);
  
  for(i=0; i<echo_context_ptr->num_listensock; i++)
  {
    shutdownsocket(echo_context_ptr->listensock[i], SHUT_RDWR);
  }
  
  /* wait here until server task acknowledges this request. */
  while(echo_context_ptr->b_run)
  {
    _time_delay(1);
  }
  ECHOSRV_DEBUG(fprintf(stderr, "\nserver closed."));
  _task_destroy(ECHOSRV_task_id);
  
  return RTCS_OK;
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name    : ServerStop
* Returned Value   : none
* Comments  :  Blocks.
*
*END*-----------------------------------------------------------------*/
static void ServerStop(ECHOSRV_CONTEXT_PTR echo_ptr)
{
  int32_t i;
  uint32_t sock;
  
  for(i=0; i<echo_ptr->num_listensock; i++)
  {
    closesocket(echo_ptr->listensock[i]);
  }
  
  ECHOSRV_DEBUG(fputs("Listening socket(s) closed!\n", stderr));
  /* stop sessions */
  for(i=0; i<echo_ptr->num_clients; i++)
  {
    sock = echo_ptr->clientsock[i];
    closesocket(sock);
  }
  for(i=0; i<echo_ptr->num_udpsock; i++)
  {
    closesocket(echo_ptr->udpsock[i]);
  }
  _mem_free(echo_ptr->buffer);
  echo_ptr->b_run = FALSE; /* ECHOSRV_release() from app monitors this flag. */
  _task_block(); /* ECHOSRV_release() will destroy us. */
}
