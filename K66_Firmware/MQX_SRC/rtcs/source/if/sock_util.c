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
*   This file contains utility functions for managing
*   socket structures.
*
*
*END************************************************************************/

#include <rtcs.h>
#include "rtcs_prv.h"
#include "tcpip.h"
#include "tcp_prv.h"
#include "udp_prv.h"
#include "ip_prv.h"
#include "socket.h"
#include <string.h>

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_Get_sock_struct
* Returned Value  : none
* Comments        : Get a socket structure, initialize it and return
*                     address of socket.
*
*END*-----------------------------------------------------------------*/

SOCKET_STRUCT_PTR   SOCK_Get_sock_struct
   (
      RTCS_SOCKET_CALL_STRUCT_PTR   type,
      _rtcs_taskid                  owner

   )
{ /* Body */
  RTCS_DATA_PTR              RTCS_data_ptr;
  SOCKET_CONFIG_STRUCT_PTR   socket_cfg_ptr;
  SOCKET_STRUCT_PTR          socket_ptr;

  RTCS_data_ptr = RTCS_get_data();
  socket_cfg_ptr = RTCS_data_ptr->SOCKET_CFG;
  
  socket_ptr = NULL;
  if(SOCK_PROTOCOL_exists_in_system(type))
  {
    socket_ptr = RTCS_part_alloc_zero(RTCS_data_ptr->RTCS_socket_partition);
  }

   if ( socket_ptr != NULL ) {

      RTCS_mutex_lock(&socket_cfg_ptr->SOCK_MUTEX);
      socket_cfg_ptr->CURRENT_SOCKETS++;
      
#if RTCSCFG_BACKWARD_COMPATIBILITY_RTCSSELECT==1
      #if RTCSCFG_SOCKET_OWNERSHIP
      SOCK_Add_owner(socket_ptr, owner);
      #endif
#endif
      
      /*
      ** link in this socket into the linked list of active sockets
      */
      if ( socket_cfg_ptr->SOCKET_HEAD != NULL ) {

         ((SOCKET_STRUCT_PTR)socket_cfg_ptr->SOCKET_TAIL)->NEXT =
            (void *)socket_ptr;
         socket_ptr->PREV = (SOCKET_STRUCT_PTR)socket_cfg_ptr->SOCKET_TAIL;

      } else {
         socket_cfg_ptr->SOCKET_HEAD = (void *)socket_ptr;
         socket_ptr->PREV = NULL;

      } /* Endif */

      socket_ptr->NEXT = NULL;
      socket_cfg_ptr->SOCKET_TAIL = (void *)socket_ptr;
      RTCS_mutex_unlock(&socket_cfg_ptr->SOCK_MUTEX);

      socket_ptr->VALID = SOCKET_VALID;

      /*
      ** set the default socket options
      */
      socket_ptr->CONNECT_TIMEOUT  = DEFAULT_CONNECT_TIMEOUT;
      socket_ptr->RETRANSMISSION_TIMEOUT = DEFAULT_RETRANSMISSION_TIMEOUT;
      socket_ptr->SEND_TIMEOUT     = DEFAULT_SEND_TIMEOUT;
      socket_ptr->RECEIVE_TIMEOUT  = DEFAULT_RECEIVE_TIMEOUT;
      socket_ptr->RECEIVE_PUSH     = DEFAULT_PUSH;
      if((uint32_t)type==SOCK_DGRAM)
      {
        socket_ptr->SEND_NOWAIT = DEFAULT_UDP_SEND_NOWAIT;
        socket_ptr->RBSIZE = DEFAULT_UDP_RBSIZE;
      }
      else
      {
        socket_ptr->SEND_NOWAIT = DEFAULT_TCP_SEND_NOWAIT;
        socket_ptr->RBSIZE = DEFAULT_RBSIZE;
      }      
      socket_ptr->SEND_PUSH        = DEFAULT_PUSH;
      socket_ptr->RECEIVE_NOWAIT   = DEFAULT_RECEIVE_NOWAIT;
      socket_ptr->TBSIZE           = DEFAULT_TBSIZE;
      socket_ptr->MAXRTO           = DEFAULT_MAXRTO;
      socket_ptr->MAXRCV_WND       = DEFAULT_MAXRCV_WND;
      socket_ptr->KEEPALIVE        = DEFAULT_KEEPALIVE;
      socket_ptr->NOWAIT           = DEFAULT_NOWAIT;
      socket_ptr->NO_NAGLE_ALGORITHM  = DEFAULT_NO_NAGLE_ALGORITHM;
      socket_ptr->NOSWRBUF         = DEFAULT_NOSWRBUF;
      socket_ptr->TIMEWAIT_TIMEOUT = DEFAULT_TIMEWAIT_TIMEOUT;
      socket_ptr->DELAY_ACK = DEFAULT_DELAY_ACK;
#if RTCSCFG_ENABLE_IP6
      socket_ptr->LINK_OPTIONS.TX.HOP_LIMIT_MULTICAST = DEFAULT_IP6_MULTICAST_HOPS;
#endif

      socket_ptr->PROTOCOL     = type;

   } /* Endif */

   return(socket_ptr);

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_Free_sock_struct
* Returned Value  : none
* Comments        : Delink and release a socket structure.
*
*END*-----------------------------------------------------------------*/

void SOCK_Free_sock_struct
   (
      SOCKET_STRUCT_PTR          socket_ptr
   )
{ /* Body */
   SOCKET_CONFIG_STRUCT_PTR   socket_cfg_ptr = RTCS_getcfg(SOCKET);
   
   if (socket_ptr->VALID == 0)
   {
      return;
   }
#if RTCSCFG_BACKWARD_COMPATIBILITY_RTCSSELECT==1
   #if RTCSCFG_SOCKET_OWNERSHIP //FSL AB
       SOCKET_OWNER_STRUCT_PTR  owner_ptr;
       SOCKET_OWNER_STRUCT_PTR  free_ptr;
   #endif
#endif

   socket_ptr->VALID = 0;

#if RTCSCFG_BACKWARD_COMPATIBILITY_RTCSSELECT==1
#if RTCSCFG_SOCKET_OWNERSHIP
   owner_ptr = socket_ptr->OWNERS.NEXT;
   while (owner_ptr != NULL) {
      free_ptr = owner_ptr;
      owner_ptr = owner_ptr->NEXT;
      _mem_free(free_ptr);
   } /* Endwhile */
#endif
#endif

   RTCS_mutex_lock(&socket_cfg_ptr->SOCK_MUTEX);

   /*
   ** Fix up the head/next pointer of our predecessor.
   */
   if ( socket_ptr->PREV == NULL ) {
      socket_cfg_ptr->SOCKET_HEAD = socket_ptr->NEXT;
   } else {
      (socket_ptr->PREV)->NEXT = socket_ptr->NEXT;
   } /* Endif */

   /*
   ** Fix up the tail/prev pointer of our successor.
   */
   if ( socket_ptr->NEXT == NULL ) {
      socket_cfg_ptr->SOCKET_TAIL = socket_ptr->PREV;
   } else {
      (socket_ptr->NEXT)->PREV = socket_ptr->PREV;
   } /* Endif */

   /*
   ** Release the socket structure memory.
   */
   RTCS_part_free(socket_ptr);

   socket_cfg_ptr->CURRENT_SOCKETS--;
   RTCS_mutex_unlock(&socket_cfg_ptr->SOCK_MUTEX);

} /* Endbody */

/**
 * @brief Call IP_route_find()/IP6_route_find()
 * 
 * Call IP_route_find/IP6_route_find for the <tt>struct sockaddr</tt> given in @p saddr_ptr.
 * IP_route_find() is called when @c sockaddr is @c sockaddr_in (@c AF_INET family).
 * IP_route_find6() is called when @c sockaddr is @c sockadddr_in6 (@c AF_INET6 family).
 * 
 * @param[in] saddr_ptr pointer to <tt>struct sockaddr</tt>
 *                   
 * @return _ip_address result of IP_route_find(), for @c AF_INET family, typecasted to <tt>void*</tt>.
 * @return (in6_addr*) result of IP6_route_find(), for @c AF_INET6 family, typecasted to <tt>void*</tt>.
 * @return @c NULL otherwise.
 */
void * SOCKADDR_route_find(const sockaddr * saddr_ptr)
{
  void * retval = NULL; 
  /* for AF_INET, retval is _ip_address */
  /* for AF_INET6, retval is (in6_addr*) */
#if RTCSCFG_ENABLE_IP4
  #if RTCSCFG_ENABLE_IP6
  if(AF_INET == saddr_ptr->sa_family) 
  {
  #endif
    retval = (void*)IP_route_find(SOCKADDR_get_ipaddr4(saddr_ptr), 0);
  #if RTCSCFG_ENABLE_IP6
  }
  #endif
#endif
#if RTCSCFG_ENABLE_IP6
  #if RTCSCFG_ENABLE_IP4
  else if(AF_INET6 == saddr_ptr->sa_family)
  {
  #endif
    retval = IP6_route_find(SOCKADDR_get_ipaddr6(saddr_ptr));
  #if RTCSCFG_ENABLE_IP4
  }
  #endif
#endif
  return retval;
}

/**
 * @brief Memcpy of sockaddr, limited to size.
 *
 * If any of the function parameters is given as a @c NULL pointer, the function will only return. 
 * 
 * @param[in] sock_struct_ptr @c SOCKET_STRUCT_PTR to a socket to determine address family AF_INET/AF_INET6.
 * @param[out] name Pointer to destination <tt>struct sockaddr</tt>. 
 * @param[in] addrfrom Pointer to source <tt>struct sockaddr</tt>. 
 * @param[out] namelen Points to size of data to be copied from source to destination in bytes.
 *  
 * @return None.
 */
void SOCKADDR_return_addr(const SOCKET_STRUCT_PTR sock_struct_ptr, sockaddr * name, const sockaddr * addrfrom, uint16_t * namelen)
{
  uint16_t to_copy = 0;
  
  /* any of the input arguments can be given as NULL. if so, only return. */
  if(sock_struct_ptr && name && addrfrom && namelen)
  {
    if(sock_struct_ptr->AF == AF_INET)
    {
      to_copy = sizeof(sockaddr_in);
    }
    else if(sock_struct_ptr->AF == AF_INET6)
    {
      to_copy = sizeof(sockaddr_in6);
    }
    if(*namelen < to_copy)
    {
      to_copy = *namelen;
    }
    memcpy(name, addrfrom, to_copy);
    *namelen = to_copy;
  }  
}

/**
 * @brief Check validity of struct sockaddr.
 *
 * Check if socket and struct sockaddr have the same protocol family. 
 * 
 * @param[in] sock @c SOCKET_STRUCT_PTR to a socket to determine address family AF_INET/AF_INET6.
 * @param[in] localaddr Pointer to <tt>struct sockaddr</tt> to be checked. 
 *  
 * @return Zero (RTCS_OK) if valid.
 * @return Specific error code in case of error.
 */
uint32_t SOCKADDR_check_valid(uint32_t sock, const sockaddr* localaddr)
{
    uint32_t retval = 0;
    SOCKET_STRUCT_PTR sock_struct_ptr = (SOCKET_STRUCT_PTR)sock;
    
    if((NULL == localaddr) || (NULL == sock_struct_ptr))
    {
      retval = RTCSERR_SOCK_INVALID_PARAMETER;
      goto RETURN;
    }

    if(localaddr->sa_family != sock_struct_ptr->AF)
    {
      retval = RTCSERR_SOCK_INVALID_AF;
    }
    
    RETURN:
    return(retval);
}

/**
 * @brief Check size of struct sockaddr.
 *
 * Check if addrlen has sufficient size for sockaddr_in/sockaddr_in6 depending on family. 
 *  
 * @param[in] localaddr Pointer to <tt>struct sockaddr</tt> to be checked. 
 * @param[in] addrlen Length in bytes to be checked. 
 *  
 * @return Zero (RTCS_OK) if struct sockaddr has sufficient size.
 * @return Specific error code otherwise.
 */
uint32_t SOCKADDR_check_addr(const sockaddr* localaddr, uint16_t addrlen)
{
    uint32_t retval = 0;
    if(NULL == localaddr)
    {
      retval = RTCSERR_SOCK_INVALID_PARAMETER;
      goto RETURN;
    }
    
    if (localaddr->sa_family == AF_INET) 
    {
        if (addrlen < sizeof(sockaddr_in)) 
        {
            retval = RTCSERR_SOCK_SHORT_ADDRESS;
        }
    }
    else if (localaddr->sa_family == AF_INET6) 
    {
        if (addrlen < sizeof(struct sockaddr_in6)) 
        {
            retval = RTCSERR_SOCK_SHORT_ADDRESS;
        }
    }
    RETURN:
    return(retval);
}

/**
 * @brief Check validity of socket data structure.
 *
 * Check if it it looks like a valid socket at the given pointer.
 *  
 * @param[in] sock_struct_ptr Pointer to <tt>SOCKET_STRUCT</tt> to be checked. 
 *  
 * @return TRUE if SOCKET_STRUCT is a valid socket.
 * @return FALSE otherwise.
 */
bool SOCK_check_valid(const uint32_t sock)
{
  SOCKET_STRUCT_PTR sock_struct_ptr = (SOCKET_STRUCT_PTR) sock;
  bool b_is_valid = FALSE;
  
  if((NULL == sock_struct_ptr) || ((SOCKET_STRUCT_PTR)RTCS_SOCKET_ERROR == sock_struct_ptr))
  {
    goto exit;
  }
  
  if(sock_struct_ptr->VALID != SOCKET_VALID)
  {
    goto exit;
  }
  
  if(
    #if RTCSCFG_ENABLE_IP4
      (AF_INET == sock_struct_ptr->AF)
      #if RTCSCFG_ENABLE_IP6
      ||
      #endif
    #endif
    #if RTCSCFG_ENABLE_IP6
      (AF_INET6 == sock_struct_ptr->AF)
    #endif
    )
  {
    b_is_valid = TRUE;
  }
  else
  {
    RTCS_setsockerror(sock, RTCSERR_SOCK_INVALID_AF);
  }
exit:
  return b_is_valid;
}

bool SOCK_disallow_recv(const uint32_t disallow_mask)
{
  bool b_disallow = FALSE;
  
  if(disallow_mask & ((1 << SHUT_RD)|(1 << SHUT_RDWR)))
  {
    b_disallow = TRUE;
  }
  
  return b_disallow;
}

bool SOCK_disallow_send(const uint32_t disallow_mask)
{
  bool b_disallow = FALSE;
  
  if(disallow_mask & ((1 << SHUT_WR)|(1 << SHUT_RDWR)))
  {
    b_disallow = TRUE;
  }
  
  return b_disallow;
}

uint32_t SOCK_set_disallow_mask(const uint32_t sock, uint32_t disallow_mask)
{
  uint32_t new_mask = 0;
  uint32_t action = 0;
  
  struct socket_struct *sock_ptr = (struct socket_struct*)sock;
  
  if(sock_ptr->disallow_mask == disallow_mask)
  {
    goto EXIT;
  }
    
  switch(sock_ptr->disallow_mask)
  {
    case 0:
      new_mask = disallow_mask; /* SHUT_RD or SHUT_RDWR or SHUT_WR */
      action = disallow_mask;
    break;
    
    case(1 << SHUT_RD):
      new_mask = (1 << SHUT_RDWR);
      action = (1 << SHUT_WR);
    break;
    
    case(1 << SHUT_WR):
      new_mask = (1 << SHUT_RDWR);
      action = (1 << SHUT_RD);
    break;
    
    case(1 << SHUT_RDWR): /* fall through */
    default:
      new_mask = (1 << SHUT_RDWR);
    break;
  }
  
  sock_ptr->disallow_mask = new_mask;
  EXIT:
  return action;
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : sock_exists_in_system
* Parameters      :
*
*     uint32_t      sock       [IN] pointer to SOCKET_STRUCT
*     SOCKET_CONFIG_STRUCT_PTR        socket_cfg_ptr  [IN] pointer to socket configuration structure in system
*
* Comments  :  returns TRUE if given sock is found in the system sockets. returns FALSE otherwise.
*   
*   Expected to be called internally in this module - from TCP/IP task.
*
*END*-----------------------------------------------------------------*/
bool SOCK_exists_in_system(const uint32_t sock)
{
  SOCKET_CONFIG_STRUCT_PTR socket_cfg_ptr = RTCS_getcfg(SOCKET);
  bool b_is_found = FALSE;
  SOCKET_STRUCT_PTR socket_ptr;
  
  if(FALSE == SOCK_check_valid(sock))
  {
    return FALSE;
  }
  
  for (socket_ptr = socket_cfg_ptr->SOCKET_HEAD; socket_ptr; socket_ptr = socket_ptr->NEXT)
  {
    if((uint32_t)socket_ptr == sock)
    {
      b_is_found = TRUE;
      break;
    }
  }
  return b_is_found;
}

bool TCB_exists_in_system(struct tcb_struct *tcb)
{
  TCP_CFG_STRUCT_PTR  tcp_cfg;
  struct tcb_struct *tcb2;
  bool b_is_found;
    
  b_is_found = FALSE;
  if(NULL != tcb)
  {
    tcp_cfg = RTCS_getcfg(TCP);
  
    tcb2 = tcp_cfg->TCBhead;
    while(NULL != tcb2)
    {
      if(tcb2 == tcb) 
      {
        b_is_found = TRUE;
        break;
      }
      tcb2 = tcb2->next;
    }  
  }
  return b_is_found;
}

bool UCB_exists_in_system(struct ucb_struct *ucb)
{
  UDP_CFG_STRUCT_PTR   UDP_cfg_ptr;
  bool b_is_found;
  struct ucb_struct *ucb2;
  
  b_is_found = FALSE;
  UDP_cfg_ptr = RTCS_getcfg(UDP);
  if(ucb != NULL)
  {
    /* ucb shall be found in one of the lists, otherwise it's not a valid UCB */
    RTCS_LIST_SEARCH(UDP_cfg_ptr->BOUND_UCB_HEAD, ucb2)
    {
      if(ucb == ucb2)
      {
        b_is_found = TRUE;
        goto RETURN;
      }
    }
    RTCS_LIST_SEARCH(UDP_cfg_ptr->GROUND_UCB_HEAD, ucb2)
    {
      if(ucb == ucb2)
      {
        b_is_found = TRUE;
        goto RETURN;
      }
    }
    RTCS_LIST_SEARCH(UDP_cfg_ptr->LBOUND_UCB_HEAD, ucb2)
    {
      if(ucb == ucb2)
      {
        b_is_found = TRUE;
        goto RETURN;
      }
    }
    RTCS_LIST_SEARCH(UDP_cfg_ptr->OPEN_UCB_HEAD, ucb2)
    {
      if(ucb == ucb2)
      {
        b_is_found = TRUE;
        goto RETURN;
      }
    }
  }
  RETURN:
  return b_is_found;
}

bool SOCK_PROTOCOL_exists_in_system(RTCS_SOCKET_CALL_STRUCT_PTR type)
{
  bool b_is_found;
  
  b_is_found = FALSE;
  #if RTCSCFG_ENABLE_TCP || RTCSCFG_ENABLE_UDP
  if(
    #if RTCSCFG_ENABLE_TCP
      ((uint32_t)type == SOCK_STREAM)
      #if RTCSCFG_ENABLE_UDP
      ||
      #endif      
    #endif
    #if RTCSCFG_ENABLE_UDP
      ((uint32_t)type == SOCK_DGRAM)
    #endif
    )
  {
    b_is_found = TRUE;
  }
#endif
  return b_is_found;
}

#if RTCSCFG_BACKWARD_COMPATIBILITY_RTCSSELECT==1
/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_Remove_owner
* Returned Value  : none
* Comments        :
*
*END*-----------------------------------------------------------------*/

bool SOCK_Remove_owner
   (
      SOCKET_STRUCT_PTR    socket_ptr,
      _rtcs_taskid         task_ptr
   )
{ /* Body */
#if RTCSCFG_SOCKET_OWNERSHIP
   SOCKET_OWNER_STRUCT_PTR owner_ptr;
   uint32_t                 i;

   owner_ptr = &socket_ptr->OWNERS;

   while (owner_ptr != NULL) {
      for (i=0;i<SOCKET_NUMOWNERS;i++) {
         if (owner_ptr->TASK[i] == task_ptr) {
            owner_ptr->TASK[i] = NULL;
            return TRUE;
         } /* Endif */
      } /* Endfor */
      owner_ptr = owner_ptr->NEXT;
   } /* Endwhile */
   return FALSE;
#else
   return TRUE;
#endif
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_Is_owner
* Returned Value  : none
* Comments        :
*
*END*-----------------------------------------------------------------*/

bool SOCK_Is_owner
   (
      SOCKET_STRUCT_PTR    socket_ptr,
      _rtcs_taskid         task_ptr
   )
{ /* Body */
#if RTCSCFG_SOCKET_OWNERSHIP
   SOCKET_OWNER_STRUCT_PTR owner_ptr;
   uint32_t                 i;

   owner_ptr = &socket_ptr->OWNERS;


   while (owner_ptr != NULL) {
      for (i=0;i<SOCKET_NUMOWNERS;i++) {
         if (owner_ptr->TASK[i] == task_ptr) {
            /* already here, just return */
            return TRUE;
         } /* Endif */
      } /* Endfor */
      owner_ptr = owner_ptr->NEXT;
   } /* Endwhile */
   return FALSE;
#else
   return TRUE;
#endif
} /* Endbody */



/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_Add_owner
* Returned Value  : none
* Comments        :
*
*END*-----------------------------------------------------------------*/

bool SOCK_Add_owner
   (
      SOCKET_STRUCT_PTR    socket_ptr,
      _rtcs_taskid         task_ptr
   )
{ /* Body */
#if RTCSCFG_SOCKET_OWNERSHIP
   SOCKET_OWNER_STRUCT_PTR owner_ptr, new_owner_ptr;
   uint32_t                 i;
   void                  **saved_ptr = NULL;

   owner_ptr = &socket_ptr->OWNERS;

   while (owner_ptr != NULL) {
      for (i=0;i<SOCKET_NUMOWNERS;i++) {
         if (owner_ptr->TASK[i] == task_ptr) {
            /* already here, just return */
            return TRUE;
         } else if ((owner_ptr->TASK[i] == 0) && (saved_ptr == NULL)) {
            saved_ptr = &owner_ptr->TASK[i];
         } /* Endif */
      } /* Endfor */
      owner_ptr = owner_ptr->NEXT;
   } /* Endwhile */

   if (saved_ptr != NULL) {
      *saved_ptr = task_ptr;
   } else {
      new_owner_ptr = RTCS_mem_alloc_zero(sizeof(SOCKET_OWNER_STRUCT));
      if (new_owner_ptr == NULL) {
         return FALSE;
      } /* Endif */

      _mem_set_type(new_owner_ptr, MEM_TYPE_SOCKET_OWNER_STRUCT);

      new_owner_ptr->TASK[0] = task_ptr;
      owner_ptr->NEXT = new_owner_ptr;
   } /* Endif */
   return TRUE;
#else
   return TRUE;
#endif
} /* Endbody */
#endif /* RTCSCFG_BACKWARD_COMPATIBILITY_RTCSSELECT */


/* EOF */
