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
*   Implementation of ECHO Client.
*
*
*END************************************************************************/
/** @file */ 
#include <rtcs.h>
#include <echocln.h>
#include <string.h>

#if RTCSCFG_ECHOCLN_DEBUG_MESSAGES==1
  #define ECHOCLN_DEBUG(x) x
#if MQX_USE_IO_OLD
  #include <fio.h>
#else
  #include <stdio.h>
#endif
#else
  #define ECHOCLN_DEBUG(x) 
#endif

/**
 * @brief Try to connect to a server.
 * 
 * Try to connect to a server, where server is specified by struct addrinfo *.
 * getaddrinfo() can be used to create struct addrinfo for a given server.
 * 
 * @param[in] addrinfo_ptr pointer to struct addrinfo
 *                   
 * @return RTCS_SOCKET_ERROR (failure)
 * @return socket handle (success).
 * @see ECHOCLN_process
 * @see getaddrinfo
 * @code
 * int32_t    i_result;
 * uint32_t   sock = RTCS_SOCKET_ERROR;
 * struct     addrinfo *result = NULL,
 *                   *ptr = NULL,
 *                   hints;
 *
 * memset( &hints, 0, sizeof(hints) );
 * hints.ai_family = AF_UNSPEC;
 * hints.ai_socktype = SOCK_STREAM;
 * hints.ai_protocol = IPPROTO_TCP;          
          
 * i_result = getaddrinfo("192.168.1.202", "7", &hints, &result);
 * if ( i_result != 0 ) 
 * {
 *   fprintf(stdout, "getaddrinfo failed with error: %d\n", i_result);
 *   goto exit;
 * }
 *
 * for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) 
 * {            
 *   sock = ECHOCLN_connect(ptr);
 *   if(sock == RTCS_SOCKET_ERROR)
 *   {
 *     continue;
 *   }
 *   break;
 * }
 * freeaddrinfo(result);
 *
 * if(sock == RTCS_SOCKET_ERROR) 
 * {
 *   fprintf(stdout, "Unable to connect to server!\n");
 *   goto exit;
 * }
 * @endcode
 */
uint32_t ECHOCLN_connect(const struct addrinfo * addrinfo_ptr)
{
  uint32_t   sock = RTCS_SOCKET_ERROR;
  int32_t    iResult;
  
  /* check input parameters */
  if(!addrinfo_ptr)
  {
    return sock;
  }
  
  sock = socket(addrinfo_ptr->ai_family, addrinfo_ptr->ai_socktype, addrinfo_ptr->ai_protocol);
  if(sock != RTCS_SOCKET_ERROR) 
  {
    /* Connect to server. */
    iResult = connect(sock, addrinfo_ptr->ai_addr, (uint16_t)addrinfo_ptr->ai_addrlen);
    if (iResult != RTCS_OK) 
    {
      shutdown(sock, FLAG_ABORT_CONNECTION );
      sock = RTCS_SOCKET_ERROR;
    }  
  }
  return sock;
}

/**
 * @brief Send echo data to a server and receive echo back.
 * 
 * One echo loop consits of: given data @p buffer is sent to the given @p sock.
 * Then inbound data is received from the socket and receive data is compared 
 * with the send data.
 * This loop is repeated @p count times.
 * 
 * @param[in] sock connected socket handle
 * @param[in] buffer pointer to a data for sending
 * @param[in] buflen length of send data in bytes
 * @param[in] count number of echo loops. If negative or zero, RTCSCFG_ECHOCLN_DEFAULT_LOOPCNT applies.
 * @param[in,out] time_ptr Non zero value on input means the function will measure time duration of @p count echo loops
 *                         and resulting time will be written into TIME_STRUCT pointed by @p time_ptr.
 *                         If NULL, time is not measured.
 *                   
 * @return RTCS_OK (success)
 * @return ECHOCLN_ERR_SOCKET (send() or recv() error)
 * @return ECHOCLN_ERR_DATA_COMPARE_FAIL (received and sent data are different)
 * @return ECHOCLN_ERR_OUT_OF_MEMORY (failed to allocate memory for receive data)
 * @return ECHOCLN_ERR_INVALID_PARAM (input parameters are invalid)
 *
 * @see ECHOCLN_connect
 * @see RTCSCFG_ECHOCLN_DEFAULT_BUFLEN
 * @see RTCSCFG_ECHOCLN_DEFAULT_LOOPCNT
 * @see RTCSCFG_ECHOCLN_DEBUG_MESSAGES
 * @code
 * int32_t i_result;
 * TIME_STRUCT diff_time;
 *
 * i_result = ECHOCLN_process(sock, buffer, buflen, loop_cnt, &diff_time);
 * if(RTCS_OK != i_result)
 * {
 *   fprintf(stdout, "ECHO client error %i after run time %i.%i\n", i_result, diff_time.SECONDS, diff_time.MILLISECONDS);
 * }
 * else
 * {
 *   fprintf(stdout, "exchanged %d echo packets, total time: %i.%i s\n", loop_cnt, diff_time.SECONDS, diff_time.MILLISECONDS);
 * }
 * shutdown(sock, 0);
 * @endcode
 */
int32_t ECHOCLN_process(uint32_t sock, char * buffer, uint32_t buflen, int32_t count, TIME_STRUCT_PTR time_ptr)
{
    char *    recvbuf;
    int32_t   iResult;
    uint32_t  recvbuflen = buflen;
    int32_t   to_send, sent, received;
    int32_t   total_packets = 0;
    
    TIME_STRUCT start_time, end_time;
    
    int32_t retval = ECHOCLN_ERR_SOCKET;
    
    /* check input parameters */
    if(count <= 0)
    {
      count = RTCSCFG_ECHOCLN_DEFAULT_LOOPCNT;
    }
    if(!sock || !buffer || (RTCS_SOCKET_ERROR == sock))
    {
      ECHOCLN_DEBUG(fprintf(stderr, "invalid input parameters\n"));
      return ECHOCLN_ERR_INVALID_PARAM;
    }
    
    
    recvbuf = RTCS_mem_alloc_system_zero(recvbuflen);
    if(NULL == recvbuf)
    {
      ECHOCLN_DEBUG(fprintf(stderr, "failed to allocate buffer for data compare\n"));
      retval = ECHOCLN_ERR_OUT_OF_MEMORY;
      goto cleanup;
    }
        
    to_send = buflen;    
    _time_get_elapsed(&start_time);
    for(;;)
    {
      /* outbound data */
      sent = 0;
      do
      {
        iResult = send(sock, buffer+sent, to_send-sent, 0);
        if(iResult == RTCS_ERROR) 
        {
          ECHOCLN_DEBUG(fprintf(stderr, "send failed with error: %d\n", RTCS_geterror(sock)));
          goto cleanup;            
        }
        sent += iResult;
      } while(sent < to_send);
      
      /* inbound data */
      received = 0;
      do
      {
        iResult = recv(sock, recvbuf+received, to_send-received, 0);
        if(iResult>0)
        {
          received += iResult;
        }
        else if ( iResult == 0 )
        {
          ECHOCLN_DEBUG(fprintf(stderr, "Connection closed\n"));
          goto cleanup;
        }
        else
        {
          ECHOCLN_DEBUG(fprintf(stderr, "recv failed with error: %d\n", RTCS_geterror(sock)));
          goto cleanup;
        }
      } while(received < to_send);

      total_packets++;
      if(total_packets >= count)
      {
        retval = RTCS_OK;
        goto cleanup;
      }
      
      /* compare inbound with outbound data */
      if(0 != memcmp(buffer, recvbuf, to_send))
      {
        ECHOCLN_DEBUG(fprintf(stderr, "Data compare corrupted\n"));
        retval = ECHOCLN_ERR_DATA_COMPARE_FAIL;
        goto cleanup;
      }
    } /* for(;;) */
    
    /* cleanup */    
cleanup:
    if(time_ptr)
    {
      _time_get_elapsed(&end_time);
      _time_diff(&start_time, &end_time, time_ptr);
    }
    if(recvbuf)
    {
      _mem_free(recvbuf); /* always release recvbuf as it is local for this function only */
    }
    return retval;
}
