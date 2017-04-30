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
*   This file contains the implementation of the User
*   Datagram Protocol.  For more details, refer to RFC768.
*
*
*END************************************************************************/

#include <rtcs.h>
#include "rtcs_prv.h"
#include "tcpip.h"
#include "ip_prv.h"
#include "udp_prv.h"
#include "icmp_prv.h"
#include "socket.h"
#include <string.h> /* memcpy */

static void udp_set_fromaddr(struct sockaddr *fromaddr, RTCSPCB_PTR pcb_ptr, uint16_t af)
{
  fromaddr->sa_family = af;
  
  #if RTCSCFG_ENABLE_IP4
    #if RTCSCFG_ENABLE_IP6
    if(af == AF_INET)      
    {
    #endif
      SOCKADDR_init((void *)IP_source(pcb_ptr), UDP_source(pcb_ptr), fromaddr);
    #if RTCSCFG_ENABLE_IP6
    }
    #endif
  #endif
  #if RTCSCFG_ENABLE_IP6
    #if RTCSCFG_ENABLE_IP4
    else if(af == AF_INET6)      
    {
    #endif      
      SOCKADDR_init((in6_addr *)(((IP6_HEADER_PTR)(pcb_ptr->NETWORK_LAYER))->SOURCE), UDP_source(pcb_ptr), fromaddr);
      /* here we get scope_id */
      ((sockaddr_in6*)fromaddr)->sin6_scope_id = RTCS6_if_get_scope_id((_rtcs_if_handle)pcb_ptr->IFSRC);
    #if RTCSCFG_ENABLE_IP4
    }
    #endif
  #endif
}

static uint32_t udp_get_rx_item_size(RTCSPCB_PTR pcb_ptr)
{
  return sizeof(struct udp_rx_dgram_header) + RTCSPCB_SIZE(pcb_ptr);
}

static void udp_set_rx_dgram_header_and_data(struct udp_rx_dgram_header * dgram_item, RTCSPCB_PTR pcb_ptr, uint16_t af)
{
  /* move information from PCB to UDP RX queue and free PCB */
  
  /* sockaddr we received the dgram from */
  udp_set_fromaddr(&dgram_item->fromaddr, pcb_ptr, af);
  
  /* link opts */
  dgram_item->rx_linkopts = pcb_ptr->LINK_OPTIONS.RX;
  
  /* dgram itself: size and payload */
  dgram_item->size = RTCSPCB_SIZE(pcb_ptr);
  RTCSPCB_memcopy(pcb_ptr, dgram_item->dgram, 0, dgram_item->size);
  
  /* free PCB */
  RTCSLOG_PCB_FREE(pcb_ptr, RTCS_OK);
  RTCSPCB_free(pcb_ptr);
}

static void udp_return_req2socket_from_rx_queue(UDP_PARM_PTR parms, struct udp_rx_dgram_header * dgram_item)
{
  uint32_t dgram_size = 0;  
  
  dgram_size = dgram_item->size;  
    
  /* limit for the upper layer recv buffer */  
  if(parms->udpword > dgram_size)
  {
    parms->udpword = dgram_size;
  }
  
  if(parms->saddr_ptr)
  {
    *parms->saddr_ptr = dgram_item->fromaddr;
  }
  memcpy(parms->udpptr, dgram_item->dgram, dgram_size);
  ((SOCKET_STRUCT_PTR)parms->ucb->SOCKET)->LINK_OPTIONS.RX = dgram_item->rx_linkopts;
}

static void udp_return_req2socket_from_pcb(UDP_PARM_PTR parms, RTCSPCB_PTR pcb_ptr)
{
  uint32_t dgram_size = 0;
    
  dgram_size = RTCSPCB_SIZE(pcb_ptr);
      
  /* limit for the upper layer recv buffer */  
  if(parms->udpword > dgram_size)
  {
    parms->udpword = dgram_size;
  }
  
  if(parms->saddr_ptr)
  {
    uint16_t af = ((SOCKET_STRUCT_PTR)parms->ucb->SOCKET)->AF;
    udp_set_fromaddr(parms->saddr_ptr, pcb_ptr, af);
  }
  RTCSPCB_memcopy(pcb_ptr, parms->udpptr, 0, parms->udpword);    
  ((SOCKET_STRUCT_PTR)parms->ucb->SOCKET)->LINK_OPTIONS.RX = pcb_ptr->LINK_OPTIONS.RX;
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : UDP_init
* Returned Values : uint32_t
* Comments        :
*     Initialize the UDP layer.
*
*END*-----------------------------------------------------------------*/

uint32_t UDP_init
   (
      void
   )
{ /* Body */
   UDP_CFG_STRUCT_PTR   UDP_cfg_ptr;
   uint32_t              status = RTCS_OK;

   UDP_cfg_ptr = RTCS_mem_alloc_zero(sizeof(UDP_CFG_STRUCT));
   if (UDP_cfg_ptr == NULL)  {
      return RTCSERR_OUT_OF_MEMORY;
   }
   _mem_set_type(UDP_cfg_ptr, MEM_TYPE_UDP_CFG_STRUCT);

   RTCS_LIST_INIT(UDP_cfg_ptr->BOUND_UCB_HEAD);
   RTCS_LIST_INIT(UDP_cfg_ptr->LBOUND_UCB_HEAD);
   RTCS_LIST_INIT(UDP_cfg_ptr->GROUND_UCB_HEAD);
   RTCS_LIST_INIT(UDP_cfg_ptr->OPEN_UCB_HEAD);
   UDP_cfg_ptr->LAST_PORT = IPPORT_USERRESERVED;

   RTCS_setcfg(UDP, UDP_cfg_ptr);

#if RTCSCFG_ENABLE_IP4
    IP_open(IPPROTO_UDP, UDP_service, (void*)AF_INET, &status);
    if(status != RTCS_OK)
    {
        return status;
    }
#endif /*RTCSCFG_ENABLE_IP4*/
   /* Initialisation of the UDP IPv6 service */
#if RTCSCFG_ENABLE_IP6
   IP6_open(IPPROTO_UDP, UDP_service, (void*)AF_INET6, &status);
#endif

   return status;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : UDP_openbind_internal
* Returned Values : error code
* Comments        :
*     Opens and binds a new UCB.
*
*END*-----------------------------------------------------------------*/
#if RTCSCFG_ENABLE_IP4

uint32_t UDP_openbind_internal
   (
      uint16_t              localport,  /* [IN] local UDP port */
      void (_CODE_PTR_     service)(RTCSPCB_PTR, UCB_STRUCT_PTR),
      UCB_STRUCT_PTR  *ucb         /* [OUT] new UCB */
   )
{ /* Body */
   UDP_PARM     parms = {0};
   uint32_t     error;
   sockaddr     ladr = {0};
   
   parms.udpword = AF_INET;    
   error = RTCSCMD_internal(parms, UDP_open);
   if (error) {
      return error;
   }   
   parms.udpservice = service;
   ladr.sa_family = AF_INET;
   ((sockaddr_in*)&ladr)->sin_port = localport;
   parms.saddr_ptr = &ladr;
   parms.udpword    = 0;
   error = RTCSCMD_internal(parms, UDP_bind);
   if (error) {
      RTCSCMD_internal(parms, UDP_close);
      return error;
   } /* Endif */

   *ucb = parms.ucb;
   return RTCS_OK;

} /* Endbody */

#endif

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : UDP_close_internal
* Returned Values : error code
* Comments        :
*     Closes a UCB.
*
*END*-----------------------------------------------------------------*/

uint32_t UDP_close_internal
   (
      UCB_STRUCT_PTR    ucb         /* [IN] old UCB */
   )
{ /* Body */
   UDP_PARM    parms;
   uint32_t     error;

   parms.ucb = ucb;
   error = RTCSCMD_internal(parms, UDP_close);
   if (error) {
      return error;
   } /* Endif */

   return RTCS_OK;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : UDP_open
* Parameters      :
*
*     UCB_STRUCT_PTR    ucb         [OUT] unbound UCB
*    _CODE_PTR_         udpservice  not used
*     _ip_address       ipaddress   not used
*     uint16_t           udpport     not used
*     uint16_t           udpflags    not used
*     pointer           udpptr      not used
*     uint32_t           udpword     not used
*
* Comments        :
*     Creates an unbound UCB.
*
*END*-----------------------------------------------------------------*/

void UDP_open
   (
      UDP_PARM_PTR      parms
   )
{ /* Body */
   UDP_CFG_STRUCT_PTR   UDP_cfg_ptr;
   UCB_STRUCT_PTR       ucb;
   uint16_t af;

   UDP_cfg_ptr = RTCS_getcfg(UDP);

   ucb = RTCS_mem_alloc_zero(sizeof(UCB_STRUCT));
   if (!ucb) {
      RTCSCMD_complete(parms, RTCSERR_UDP_UCB_ALLOC);
      return;
   } /* Endif */

   _mem_set_type(ucb, MEM_TYPE_UCB_STRUCT);

   RTCS_QUEUE_INIT(ucb->PHEAD, ucb->PTAIL);
   RTCS_QUEUE_INIT(ucb->RHEAD, ucb->RTAIL);

   /* Initialize the list of joined multicast groups */
   RTCS_LIST_INIT(ucb->MCB_PTR);
   ucb->IGMP_LEAVEALL = NULL;

   /* Add the new UCB into the chain of ground UCBs */
   RTCS_LIST_INS(UDP_cfg_ptr->GROUND_UCB_HEAD, ucb);
   
   af = parms->udpword;
   RTCS_ASSERT((af==AF_INET)||(af==AF_INET6));
   ucb->LADDR.sa_family = af;
   ucb->RADDR.sa_family = af;
   ucb->BYPASS_TX = DEFAULT_CHECKSUM_BYPASS;
   parms->ucb = ucb;
   RTCSCMD_complete(parms, RTCS_OK);
   return;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : UDP_bind
* Parameters      :
*
*     UCB_STRUCT_PTR    ucb         [IN] unbound UCB
*    _CODE_PTR_         udpservice  [IN] service function for incoming datagrams
*     _ip_address       ipaddress   [IN] local IP address
*     uint16_t           udpport     [IN] local port
*     uint16_t           udpflags    not used
*     pointer           udpptr      not used
*     uint32_t           udpword     [IN] socket handle
*
* Comments        :
*     Binds a UCB to a local port.
*
*END*-----------------------------------------------------------------*/
void UDP_bind
   (
      UDP_PARM_PTR      parms
   )
{ /* Body */
   UDP_CFG_STRUCT_PTR   UDP_cfg_ptr;
   UCB_STRUCT_PTR       ucb, search_ptr, * tmp_ptr, * list_ptr;
   uint16_t             port;
   bool                 b_ip_is_zero;;
   
   ucb = parms->ucb;
   if(FALSE == UCB_exists_in_system(ucb))
   {
     RTCSCMD_complete(parms, RTCSERR_UDP_UCB_CLOSE);
     return;
   }
   UDP_cfg_ptr = RTCS_getcfg(UDP);
   /* this function can be called from nosock UCB, that is, ucb->SOCKET can be zero. */

   /* Initialize the UCB */
   port = SOCKADDR_get_port(parms->saddr_ptr);
   b_ip_is_zero = SOCKADDR_ip_is_zero(parms->saddr_ptr);

   if (port) {

      /*
      ** If port is specified, make sure its not in use with same local
      ** IP addr. We can only have one UCB per (IPADDR/port) combination,
      ** including the case were the IPADDR is INADDR_ANY.
      */
      if (!b_ip_is_zero) {
         RTCS_LIST_SEARCH(UDP_cfg_ptr->LBOUND_UCB_HEAD, search_ptr) {
            if(SOCKADDR_ip_and_port_are_equal(&search_ptr->LADDR, parms->saddr_ptr))
            {
               RTCSCMD_complete(parms, RTCSERR_UDP_PORT_OPEN);
               return;
            } /* Endif */
         } /* End SEARCH */

         RTCS_LIST_SEARCH(UDP_cfg_ptr->OPEN_UCB_HEAD, search_ptr) {
            if(SOCKADDR_ip_and_port_are_equal(&search_ptr->LADDR, parms->saddr_ptr))               
            {
               RTCSCMD_complete(parms, RTCSERR_UDP_PORT_OPEN);
               return;
            } /* Endif */
         } /* End SEARCH */
      } else {
         RTCS_LIST_SEARCH(UDP_cfg_ptr->BOUND_UCB_HEAD, search_ptr) {
            if((SOCKADDR_get_port(&search_ptr->LADDR) == port) && (search_ptr->LADDR.sa_family == parms->saddr_ptr->sa_family)) {
               RTCSCMD_complete(parms, RTCSERR_UDP_PORT_OPEN);
               return;
            } /* Endif */
         } /* End SEARCH */
      } /* Endif */
   }
   else
   {
      port = UDP_cfg_ptr->LAST_PORT;
      for (;;)
      {
         /* Get next port */
         if (--port < IPPORT_RESERVED)
         {
            port = IPPORT_USERRESERVED;
         } /* Endif */
         /* If we tested them all, return an error */
         if (port == UDP_cfg_ptr->LAST_PORT)
         {
            RTCSCMD_complete(parms, RTCSERR_UDP_PORT_ALLOC);
            return;
         } /* Endif */
         /*
         ** Check if it's in use
         */
         RTCS_LIST_SEARCH(UDP_cfg_ptr->BOUND_UCB_HEAD, search_ptr)
         {
            if (SOCKADDR_get_port(&search_ptr->LADDR) == port)
            {
               break;
            } /* Endif */
         } /* End SEARCH */
         if (!search_ptr)
         {
            RTCS_LIST_SEARCH(UDP_cfg_ptr->LBOUND_UCB_HEAD, search_ptr)
            {
               if (SOCKADDR_get_port(&search_ptr->LADDR) == port)
               {
                  break;
               } /* Endif */
            } /* End SEARCH */
         } /* Endif */
         if (!search_ptr)
         {
            RTCS_LIST_SEARCH(UDP_cfg_ptr->OPEN_UCB_HEAD, search_ptr)
            {
               if (SOCKADDR_get_port(&search_ptr->LADDR) == port)
               {
                  break;
               } /* Endif */
            } /* End SEARCH */
         } /* Endif */
         /* If it's not in use, keep it */
         if (!search_ptr)
         {
            break;
         } /* Endif */
      } /* Endfor */

      UDP_cfg_ptr->LAST_PORT = port;
      SOCKADDR_set_port(parms->saddr_ptr, port); /* port was computed from LAST_PORT */

   } /* Endif */
   
   ucb->PCOUNT       = 0;     
   ucb->KEEP_IPADDR  = !b_ip_is_zero;
   ucb->SERVICE      = parms->udpservice;
   SOCKADDR_copy(parms->saddr_ptr, &ucb->LADDR);//from->to
   
   /* This is the list the UCB will be moved to */
   if (b_ip_is_zero) {
      list_ptr = &(UDP_cfg_ptr->BOUND_UCB_HEAD);
   } else {
      list_ptr = &(UDP_cfg_ptr->LBOUND_UCB_HEAD);
   } /* Endif */
   /* Move the UCB fromt the GROUND list to the BOUND list */
   RTCS_LIST_SEARCHMOD(UDP_cfg_ptr->GROUND_UCB_HEAD, tmp_ptr) {
      if (*tmp_ptr == ucb) {
         RTCS_LIST_DEL(UDP_cfg_ptr->GROUND_UCB_HEAD,tmp_ptr);
         break;
      } /* Endif */
   } /* End SEARCH */
   RTCS_LIST_INS_END((*list_ptr), ucb, tmp_ptr);
   RTCSCMD_complete(parms, RTCS_OK);
   return;
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : UDP_connect
* Parameters      :
*
*     UCB_STRUCT_PTR    ucb         [IN] bound or open UCB
*    _CODE_PTR_         udpservice  not used
*     in6_addr          ipv6address [IN] peer IPv6 address
*     uint16_t           udpport     [IN] peer port
*     uint16_t           udpflags    not used
*     pointer           udpptr      not used
*     uint32_t           udpword     [IN] socket level error
*
* Comments        :
*     Connects a UCB to a peer port.
*
*END*-----------------------------------------------------------------*/
void UDP_connect
   (
      UDP_PARM_PTR      parms
   )
{
  UDP_CFG_STRUCT_PTR   UDP_cfg_ptr;
  UCB_STRUCT_PTR       ucb, search_ptr, * tmp_ptr;
  UCB_STRUCT_PTR       *ins_ptr, * del_ptr;
  uint32_t             error = RTCS_OK;

  void *               temp_ptr;
  uint16_t raddr_port;
  uint16_t remote_port;
  
  ucb = parms->ucb;
  if(FALSE == UCB_exists_in_system(ucb))
  {
    RTCSCMD_complete(parms, RTCSERR_UDP_UCB_CLOSE);
    return;
  }

  remote_port = SOCKADDR_get_port(parms->saddr_ptr);
  raddr_port = SOCKADDR_get_port(&ucb->RADDR);
   

  UDP_cfg_ptr = RTCS_getcfg(UDP);

  if(!remote_port || SOCKADDR_ip_is_zero(parms->saddr_ptr))
  {
    /* Connect requires both a port AND and IP address.  */
    error = RTCSERR_UDP_PORT_ZERO;
  }
  else
  {
    /* Bind a local address if none is specified.
     * This is not needed for multicast or broadcast foreign address. 
     */
    if(
       SOCKADDR_ip_is_zero(&ucb->LADDR) &&
       !(SOCKADDR_ip_is_multicast(parms->saddr_ptr) || SOCKADDR_ip_is_broadcast(parms->saddr_ptr))
      )
    {
      temp_ptr = SOCKADDR_route_find(parms->saddr_ptr);
      if(NULL == temp_ptr)
      {        
        error = RTCSERR_IP_UNREACH;
      }
      else
      {       
        /* initialize ip address, do not touch port */
        SOCKADDR_init_no_port(temp_ptr, &ucb->LADDR);
        /*
        ** Make sure this connect request does not put two UCB's into the
        ** same configuration.
        */
        RTCS_LIST_SEARCH(UDP_cfg_ptr->OPEN_UCB_HEAD, search_ptr)
        {
          if  (
                search_ptr != ucb &&
                SOCKADDR_ip_and_port_are_equal(&search_ptr->LADDR, &ucb->LADDR)
              )
                {
                  error = RTCSERR_UDP_PORT_OPEN;
                  SOCKADDR_zero_ip(&ucb->LADDR);
                  break;
                } /* Endif */
        } /* End SEARCH */
      } /* Endif */
    }
  }  /*Endif */

   /*
   ** Check for errors and location of UCB. If TRUE, UCB must be moved.
   ** This is an evaluates to TRUE if one of the following is true:
   ** A. There is an error, and there is a remote port  OR
   ** B. There is no error, and there is no remote port
   */
   if(!error == !raddr_port)
   {
        if(raddr_port)
        {
            if(ucb->KEEP_IPADDR)
            {
                ins_ptr = &(UDP_cfg_ptr->LBOUND_UCB_HEAD);
            }
            else
            {
                ins_ptr = &(UDP_cfg_ptr->BOUND_UCB_HEAD);
            }
            del_ptr = &(UDP_cfg_ptr->OPEN_UCB_HEAD);
        }
        else
        {
            ins_ptr = &(UDP_cfg_ptr->OPEN_UCB_HEAD);
            if(ucb->KEEP_IPADDR)
            {
                del_ptr = &(UDP_cfg_ptr->LBOUND_UCB_HEAD);
            }
            else
            {
                del_ptr = &(UDP_cfg_ptr->BOUND_UCB_HEAD);
            }
        }
        /* Move the UCB from one list to the other */
        RTCS_LIST_SEARCHMOD(*del_ptr, tmp_ptr)
        {
            if(*tmp_ptr == ucb)
            {
                RTCS_LIST_DEL(*del_ptr, tmp_ptr);
                break;
            } /* Endif */
        } /* End SEARCH */
        RTCS_LIST_INS_END(*ins_ptr, ucb, tmp_ptr);
   } /* Endif */

   /* If there was no error, set the peer endpoint */
   if(!error)
   {
      /* from->to */
      SOCKADDR_copy(parms->saddr_ptr, &ucb->RADDR);
   }
   else
   {
        /* Disconnect the UCB. It is in bound state now */
        if(!ucb->KEEP_IPADDR)
        {
            SOCKADDR_zero_ip(&ucb->LADDR);
        }
        /* clear ip address and port of ucb->RADDR, keep sa_family */
        SOCKADDR_zero_ip(&ucb->RADDR);
        SOCKADDR_set_port(&ucb->RADDR, 0);        
   } /* Endif */
   
 #if RTCSCFG_ENABLE_IP6
  /* add scope_id here */
  if(ucb->LADDR.sa_family == AF_INET6)
  {
    ((sockaddr_in6*)&ucb->LADDR)->sin6_scope_id = ((sockaddr_in6*)(parms->saddr_ptr))->sin6_scope_id;
  }
 #endif
  RTCSCMD_complete(parms, error);
  return;
}

/* receive timeout expires - remove the receive request
 * from the receive queue and unblock the caller task with zero bytes
 * received. 
 */
static bool UDP_receive_expire(TCPIP_EVENT_PTR event)
{
  UDP_PARM_PTR parms;
  UDP_PARM_PTR act_ptr;
  UDP_PARM_PTR prev_ptr;
  
  UCB_STRUCT_PTR ucb;

  parms = event->PRIVATE;
  ucb = parms->ucb;
  if(FALSE == UCB_exists_in_system(ucb))
  {
    return FALSE;
  }
    
  if(parms == ucb->RHEAD)
  {
    RTCS_QUEUE_DEL_NONEMPTY(ucb->RHEAD, ucb->RTAIL);
  }
  else
  {
    act_ptr = ucb->RHEAD->NEXT;
    prev_ptr = ucb->RHEAD;
    while(act_ptr)
    {
      if(parms == act_ptr)
      {
        prev_ptr->NEXT = act_ptr->NEXT;
        break;
      }
      prev_ptr = act_ptr;
      act_ptr = act_ptr->NEXT;
    }  
  }
    
  /* Expectation: SOCK_DGRAM_recvfrom() uses all zeroes sockaddr for default source address */
  parms->udpword   = 0;
  RTCSCMD_complete(parms, RTCS_OK);

  return FALSE; /* don't repeat */
} 

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : UDP_receive
* Parameters      :
*
*     UCB_STRUCT_PTR    ucb         [IN] UCB
*    _CODE_PTR_         udpservice  not used
*     _ip_address       ipaddress   [OUT] source IP address
*     uint16_t           udpport     [OUT] source UDP port
*     uint16_t           udpflags    [IN] flags
*     pointer           udpptr      [IN/OUT] data buffer
*     uint32_t           udpword     [IN/OUT] length of buffer
*
* Comments        :
*     Processes a receive request from the application.
*
*END*-----------------------------------------------------------------*/
void UDP_receive
   (
      UDP_PARM_PTR      parms
   )
{
  bool nonblock_rx;
  uint32_t timeout_ms;
  struct ucb_struct   *ucb;
  struct udp_rx_dgram_header * dgram_item;
  
  ucb = parms->ucb;
  if(FALSE == UCB_exists_in_system(ucb))
  {
   RTCSCMD_complete(parms, RTCSERR_UDP_UCB_CLOSE);
   return;
  }
  /* this function cannot be called from nosock UCB. test it. */
  if(FALSE == SOCK_exists_in_system(ucb->SOCKET))
  {
   RTCSCMD_complete(parms, RTCSERR_SOCK_INVALID);
   return;
  }
  
  nonblock_rx = (((uint32_t)TRUE == ((SOCKET_STRUCT_PTR)ucb->SOCKET)->RECEIVE_NOWAIT)
                           || (parms->udpflags & RTCS_MSG_PEEK));
  timeout_ms = ((SOCKET_STRUCT_PTR)parms->ucb->SOCKET)->RECEIVE_TIMEOUT;
  /*
  ** Search for a waiting packet on incoming queue
  */
  RTCS_QUEUE_PEEK(ucb->PHEAD, ucb->PTAIL, dgram_item);
  if(dgram_item) 
  {    
    /*
    ** Return request to Socket Layer with packet
    */
    udp_return_req2socket_from_rx_queue(parms, dgram_item);
    
    if (!(parms->udpflags & RTCS_MSG_PEEK)) 
    {
      RTCS_QUEUE_DEL_NONEMPTY(ucb->PHEAD, ucb->PTAIL);
      ucb->PCOUNT -= dgram_item->size;
      _mem_free(dgram_item);
    } 

    RTCSCMD_complete(parms, RTCS_OK);
    return;

  } 
  else if (nonblock_rx) 
  {

    /*
      ** Return request to Socket Layer with zero bytes
      ** expectation: *((sockaddr_in*)parms->saddr_ptr) is all zeroes as passed from SOCK_DGRAM_recvfrom
      */
    parms->udpword = 0;
    RTCSCMD_complete(parms, RTCS_OK);
    return;
  } 
  else 
  {
    if(timeout_ms)
    {
      parms->EXPIRE.TIME = timeout_ms;
      parms->EXPIRE.EVENT = UDP_receive_expire;
      parms->EXPIRE.PRIVATE = parms;
      TCPIP_Event_add(&parms->EXPIRE);
    }
    else
    {
      parms->EXPIRE.PRIVATE = 0;
    }
    /*
    ** Otherwise, enqueue the request
    */
    RTCS_QUEUE_INS(ucb->RHEAD, ucb->RTAIL, parms);

  } /* Endif */

} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : UDP_send
* Parameters      :
*
*     UCB_STRUCT_PTR    ucb         [IN] UCB
*    _CODE_PTR_         udpservice  not used
*     _ip_address       ipaddress   [IN] destination IP address
*     uint16_t           udpport     [IN] destination UDP port
*     uint16_t           udpflags    [IN] flags
*     pointer           udpptr      [IN] data to send
*     uint32_t           udpword     [IN] length of data to send
*
* Comments        :
*     Sends a UDP packet.
*
*END*-----------------------------------------------------------------*/
void UDP_send
   (
      UDP_PARM_PTR      parms
   )
{ /* Body */
   IF_UDP_STATS_ENABLED(UDP_CFG_STRUCT_PTR   UDP_cfg_ptr);
   RTCSPCB_PTR          pcb;
   register bool     nonblock;
   uint32_t              error;
   struct ucb_struct   *ucb;
   uint16_t port = 0;
   
   ucb = parms->ucb;
   if(FALSE == UCB_exists_in_system(ucb))
   {
     RTCSCMD_complete(parms, RTCSERR_UDP_UCB_CLOSE);
     return;
   }
   /* this function cannot be called from nosock UCB. test it. */
   if(FALSE == SOCK_exists_in_system(ucb->SOCKET))
   {
     RTCSCMD_complete(parms, RTCSERR_SOCK_INVALID);
     return;
   }

   port = SOCKADDR_get_port(parms->saddr_ptr);
   
   IF_UDP_STATS_ENABLED(UDP_cfg_ptr = RTCS_getcfg(UDP));

   if (port == 0) {
      IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.COMMON.ST_TX_TOTAL++);
      IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.COMMON.ST_TX_DISCARDED++);
      RTCSCMD_complete(parms, RTCSERR_UDP_PORT_ZERO);
      return;
   } /* Endif */

   pcb = RTCSPCB_alloc_send();
   if (pcb == NULL) {
      IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.COMMON.ST_TX_TOTAL++);
      IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.COMMON.ST_TX_MISSED++);
      RTCSCMD_complete(parms, RTCSERR_PCB_ALLOC);
      return;
   } /* Endif */
   if (parms->udpflags & RTCS_MSG_O_NONBLOCK) {
      nonblock = parms->udpflags & RTCS_MSG_S_NONBLOCK;
   } else {
      nonblock = ((SOCKET_STRUCT_PTR)ucb->SOCKET)->SEND_NOWAIT == (uint32_t)TRUE;
   } /* Endif */

   if (nonblock) {
      unsigned char *udpbuffer;

      udpbuffer = RTCS_mem_alloc_system(parms->udpword);
      if (!udpbuffer) {
         IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.COMMON.ST_TX_TOTAL++);
         IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.COMMON.ST_TX_MISSED++);
         RTCSLOG_PCB_FREE(pcb, RTCSERR_PCB_ALLOC);
         RTCSPCB_free(pcb);
         RTCSCMD_complete(parms, RTCSERR_PCB_ALLOC);
         return;
      } /* Endif */

       _mem_set_type(udpbuffer, MEM_TYPE_UDP_TX_BUFFER);


      error = RTCSPCB_append_fragment_autofree(pcb, parms->udpword, udpbuffer);
      if (error) {
         IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.COMMON.ST_TX_TOTAL++);
         IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.COMMON.ST_TX_MISSED++);
         IF_UDP_STATS_ENABLED(RTCS_seterror(&UDP_cfg_ptr->STATS.ERR_TX, error, (uint32_t)pcb));
         _mem_free(udpbuffer);
         RTCSLOG_PCB_FREE(pcb, error);
         RTCSPCB_free(pcb);
         RTCSCMD_complete(parms, error);
         return;
      } /* Endif */
      
      memcpy(udpbuffer, parms->udpptr, parms->udpword);

   } else {

      error = RTCSPCB_append_fragment(pcb, parms->udpword, parms->udpptr);
      if (error) {
         IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.COMMON.ST_TX_TOTAL++);
         IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.COMMON.ST_TX_MISSED++);
         IF_UDP_STATS_ENABLED(RTCS_seterror(&UDP_cfg_ptr->STATS.ERR_TX, error, (uint32_t)pcb));
         RTCSLOG_PCB_FREE(pcb, error);
         RTCSPCB_free(pcb);
         RTCSCMD_complete(parms, error);
         return;
      } /* Endif */

      pcb->UDP_REQUEST = parms;

   } /* Endif */

   pcb->LINK_OPTIONS.TX = ((SOCKET_STRUCT_PTR)ucb->SOCKET)->LINK_OPTIONS.TX;

   parms->COMMON.ERROR = RTCS_OK;   

#if RTCSCFG_ENABLE_IP6
   if((((SOCKET_STRUCT_PTR)(ucb->SOCKET))->AF) == AF_INET6)
   {
     if(((sockaddr_in6*)(parms->saddr_ptr))->sin6_scope_id)
        {
         /* parms->if_scope_id has value of scope_id from destination address.
          * If it is not NULL we will use it for calculation what interface we will be used for send packet.
          */
            pcb->IFSRC = ip6_if_get_by_scope_id(((sockaddr_in6*)(parms->saddr_ptr))->sin6_scope_id);
        }
        else if(((sockaddr_in6*)&ucb->LADDR)->sin6_scope_id)
        {/* parms->ucb->IF_SCOPE_ID has value of scope_id from local address.
          * If it is not NULL we will use it for calculation what interface we will be used for send packet.
          */
            pcb->IFSRC = ip6_if_get_by_scope_id(((sockaddr_in6*)&(ucb->LADDR))->sin6_scope_id);
        }
        else
        {   /*
             * if scope_id was not defined( is NULL) let IP level calculate send interface
             */
            pcb->IFSRC = NULL;
        }
   }
#endif /* RTCSCFG_ENABLE_IP6 */

    error = UDP_send_internal(ucb, &ucb->LADDR, parms->saddr_ptr, pcb, parms->udpflags);

    if(nonblock)
    {
      RTCSCMD_complete(parms, error);
    }
    else if(error)
    {
      parms->COMMON.ERROR = error;  /* Assumes application is lower priority */
    } 
} 

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : UDP_send_internal
* Returned Values : error code
* Comments        :
*     Sends a UDP packet.
*
*END*-----------------------------------------------------------------*/
uint32_t UDP_send_internal
   (
      UCB_STRUCT_PTR    ucb,        /* [IN] UDP layer context        */
      sockaddr *        srcaddr,    /* [IN] source IPv4/IPv6 address      */
      sockaddr *        destaddr,   /* [IN] destination IPv4/IPv6 address and port */      
      RTCSPCB_PTR       pcb_ptr,    /* [IN] packet to send           */
      uint32_t          flags       /* [IN] optional flags           */
   )
{ 
    IF_UDP_STATS_ENABLED(UDP_CFG_STRUCT_PTR   UDP_cfg_ptr);
    UDP_HEADER_PTR       packet;
    register bool        nochksum;
    uint32_t             error;
    
#if RTCSCFG_ENABLE_IP4
    #if RTCSCFG_ENABLE_IP6
    uint16_t             af;
    #endif
#endif
    
    /* This function can be called from nosock UCB, that is, ucb->SOCKET can be zero.
     * Thus, test only UCB.
     */    
    if(FALSE == UCB_exists_in_system(ucb))
    {
      return RTCSERR_UDP_UCB_CLOSE;
    }
    
#if RTCSCFG_ENABLE_IP4
    #if RTCSCFG_ENABLE_IP6
    af = ucb->LADDR.sa_family;
    #endif
#endif
#if BSPCFG_ENET_HW_TX_PROTOCOL_CHECKSUM
    IP_IF_PTR            if_ptr = NULL;
#endif
    uint16_t             destport = SOCKADDR_get_port(destaddr);
    uint16_t             srcport = SOCKADDR_get_port(srcaddr);

    IF_UDP_STATS_ENABLED(UDP_cfg_ptr = RTCS_getcfg(UDP));
    IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.COMMON.ST_TX_TOTAL++);

    error = RTCSPCB_insert_header(pcb_ptr, sizeof(UDP_HEADER));
    if(error) 
    {
      IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.COMMON.ST_TX_MISSED++);
      IF_UDP_STATS_ENABLED(RTCS_seterror(&UDP_cfg_ptr->STATS.ERR_TX, error, (uint32_t)pcb_ptr));
      RTCSLOG_PCB_FREE(pcb_ptr, error);
      RTCSPCB_free(pcb_ptr);
      return error;
    }

    RTCSLOG_PCB_WRITE(pcb_ptr, RTCS_LOGCTRL_PROTO(IPPROTO_UDP), 0);

    packet = (UDP_HEADER_PTR)RTCSPCB_DATA(pcb_ptr);
    RTCSPCB_DATA_TRANSPORT(pcb_ptr) = RTCSPCB_DATA(pcb_ptr);
    (void) mqx_htons(packet->SRC_PORT, srcport);
    (void) mqx_htons(packet->DEST_PORT, destport);
    (void) mqx_htons(packet->LENGTH, RTCSPCB_SIZE(pcb_ptr));
    (void) mqx_htons(packet->CHECKSUM, 0);

    if (flags & RTCS_MSG_O_NOCHKSUM) {
      nochksum = flags & RTCS_MSG_S_NOCHKSUM;
    } else {
      nochksum = ucb->BYPASS_TX;
    }

#if BSPCFG_ENET_HW_TX_PROTOCOL_CHECKSUM

  #if RTCSCFG_ENABLE_IP4
    #if RTCSCFG_ENABLE_IP6
    if(AF_INET == af)    
    {
    #endif
      _ip_address         if_addr;
      if_addr = IP_route_find(SOCKADDR_get_ipaddr4(destaddr), 1);
      if_ptr = IP_find_if(if_addr);
    #if RTCSCFG_ENABLE_IP6
    }
    #endif
  #endif
  #if RTCSCFG_ENABLE_IP6
    #if RTCSCFG_ENABLE_IP4
    if(AF_INET6 == af)    
    {
    #endif
      if_ptr = pcb_ptr->IFSRC; /* Detect output interface.*/
      if(if_ptr == NULL)
      {
        in6_addr            *if_addr;
        if_addr = IP6_route_find(SOCKADDR_get_ipaddr6(destaddr));
        if(if_addr)
        {
          if_ptr = ip6_if_get_by_addr(if_addr);
        }
      }
    #if RTCSCFG_ENABLE_IP4
    }
    #endif
  #endif
  
  if(if_ptr 
        && (if_ptr->FEATURES & IP_IF_FEATURE_HW_TX_PROTOCOL_CHECKSUM)
#if RTCSCFG_LINKOPT_8023
        && (pcb_ptr->LINK_OPTIONS.TX.OPT_8023 == 0)
#endif
        && (
#if RTCSCFG_ENABLE_IP4       
       (
         #if RTCSCFG_ENABLE_IP6
         (af == AF_INET)&&
         #endif
         (IP_will_fragment(if_ptr, RTCSPCB_SIZE(pcb_ptr)) == FALSE)
       )
       #if RTCSCFG_ENABLE_IP6
       ||
       #endif
#endif
#if RTCSCFG_ENABLE_IP6
       (
         #if RTCSCFG_ENABLE_IP4
         (af == AF_INET6)&&
         #endif
         (IP6_will_fragment(if_ptr, RTCSPCB_SIZE(pcb_ptr)) == FALSE)
       )
#endif
           )
    )
    {
      pcb_ptr->TYPE |= RTCSPCB_TYPE_HW_PROTOCOL_CHECKSUM;
    }
    else
#endif
    {
        if (!nochksum)
        {
            pcb_ptr->IP_SUM     = IP_Sum_PCB(RTCSPCB_SIZE(pcb_ptr), pcb_ptr);
            pcb_ptr->IP_SUM_PTR = packet->CHECKSUM;
        }
    }
  
  
  #if RTCSCFG_ENABLE_IP4
    #if RTCSCFG_ENABLE_IP6    
    if(AF_INET == af)
    {
    #endif
      return IP_send(pcb_ptr, IPPROTO_UDP, SOCKADDR_get_ipaddr4(srcaddr), SOCKADDR_get_ipaddr4(destaddr), flags);
    #if RTCSCFG_ENABLE_IP6
    }
    #endif
  #endif
  #if RTCSCFG_ENABLE_IP6
    #if RTCSCFG_ENABLE_IP4
    else if(AF_INET6 == af)    
    {
    #endif
      /*
            Here I am checking is local IP is  NULL or not
            It is necessary for allow IPsend change NULL local ipv6 to correct local IPv6
        */
      if(SOCKADDR_ip_is_zero(srcaddr))
      {
        return IP6_send(pcb_ptr, IPPROTO_UDP, NULL, SOCKADDR_get_ipaddr6(destaddr), pcb_ptr->IFSRC);
      }
      else
      {
        return IP6_send(pcb_ptr, IPPROTO_UDP, SOCKADDR_get_ipaddr6(srcaddr), SOCKADDR_get_ipaddr6(destaddr), pcb_ptr->IFSRC);
      }
    #if RTCSCFG_ENABLE_IP4
    }
    return 0;
    #endif
  #endif
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : UDP_send_IF
* Returned Values : uint32_t (error code)
* Comments        :
*     Sends a UDP packet through a specific IP interface.
*
*END*-----------------------------------------------------------------*/

uint32_t UDP_send_IF
   (
      UCB_STRUCT_PTR    ucb,           /* [IN/OUT] UDP layer context   */
      void             *dest_if,       /* [IN]     dest interface      */
      uint16_t           dest_port,     /* [IN]     dest port           */
      RTCSPCB_PTR       pcb_ptr        /* [IN/OUT] packet to send      */
   )
{ /* Body */
   IF_UDP_STATS_ENABLED(UDP_CFG_STRUCT_PTR   UDP_cfg_ptr);
   UDP_HEADER_PTR packet;
   uint32_t        error;   
   uint16_t  src_port;
   
   if(FALSE == UCB_exists_in_system(ucb))
   {
     return RTCSERR_UDP_UCB_CLOSE;
   }
   
   src_port = SOCKADDR_get_port(&ucb->LADDR);
   IF_UDP_STATS_ENABLED(UDP_cfg_ptr = RTCS_getcfg(UDP));

   IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.COMMON.ST_TX_TOTAL++);

   if (dest_port == 0) {
      IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.COMMON.ST_TX_DISCARDED++);
      RTCSLOG_PCB_FREE(pcb_ptr, RTCSERR_UDP_PORT_ZERO);
      RTCSPCB_free(pcb_ptr);
      return RTCSERR_UDP_PORT_ZERO;
   } /* Endif */

   error = RTCSPCB_insert_header(pcb_ptr, sizeof(UDP_HEADER));
   if (error) {
      IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.COMMON.ST_TX_MISSED++);
      IF_UDP_STATS_ENABLED(RTCS_seterror(&UDP_cfg_ptr->STATS.ERR_TX, error, (uint32_t)pcb_ptr));
      RTCSLOG_PCB_FREE(pcb_ptr, error);
      RTCSPCB_free(pcb_ptr);
      return error;
   } /* Endif */

   RTCSLOG_PCB_WRITE(pcb_ptr, RTCS_LOGCTRL_PROTO(IPPROTO_UDP), 0);

   packet = (UDP_HEADER_PTR)RTCSPCB_DATA(pcb_ptr);
   RTCSPCB_DATA_TRANSPORT(pcb_ptr) = RTCSPCB_DATA(pcb_ptr);
   (void) mqx_htons(packet->SRC_PORT, src_port);
   (void) mqx_htons(packet->DEST_PORT, dest_port);
   (void) mqx_htons(packet->LENGTH, RTCSPCB_SIZE(pcb_ptr));
   (void) mqx_htons(packet->CHECKSUM, 0);

   pcb_ptr->IP_SUM     = IP_Sum_PCB(RTCSPCB_SIZE(pcb_ptr), pcb_ptr);
   pcb_ptr->IP_SUM_PTR = packet->CHECKSUM;

   return IP_send_IF(pcb_ptr, IPPROTO_UDP, dest_if);

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : UDP_service
* Returned Values : void
* Comments        :
*     Process incoming UDP packets for IPv4 and IPv6 family.  Called from IP_service/IP6_service.
*
*END*-----------------------------------------------------------------*/
void UDP_service
   (
      RTCSPCB_PTR    pcb_ptr,          /* [IN/OUT] incoming packet */
      void          *this_af           /* [IN]     address family        */
   )
{ /* Body */
  UDP_CFG_STRUCT_PTR   UDP_cfg_ptr;
  UDP_HEADER_PTR       packet;
  uint16_t             port, src_port, len, sum = 0;
  UCB_STRUCT_PTR       ucb_ptr = NULL;
  UCB_STRUCT_PTR *     search_ptr;
  uint32_t             af = (uint32_t)this_af;

  sockaddr srcaddr = {0};
  sockaddr destaddr = {0};
#if RTCSCFG_ENABLE_IP6  
  IP6_HEADER_PTR          ip6_packet;
#endif
  
  uint32_t              error;

  UDP_cfg_ptr = RTCS_getcfg(UDP);

  IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.COMMON.ST_RX_TOTAL++);
  packet = (UDP_HEADER_PTR)RTCSPCB_DATA(pcb_ptr);
  RTCSPCB_DATA_TRANSPORT(pcb_ptr) = RTCSPCB_DATA(pcb_ptr);
  port = mqx_ntohs(packet->DEST_PORT);
  len = mqx_ntohs(packet->LENGTH);

  /*
  ** Make sure that
  **    sizeof(UDP_HEADER) <= pktlen <= RTCSPCB_SIZE(pcb)
  **
  ** Note: If RTCSPCB_SIZE(pcb) is too small, then pktlen is
  **       undefined.  However, if the following conditions pass,
  **       then RTCSPCB_SIZE(pcb) >= sizeof(UDP_HEADER), and
  **       therefore, pktlen is valid.
  */
  if(
      len < sizeof(UDP_HEADER)
    )
  {
    IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.COMMON.ST_RX_DISCARDED++);
    IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.ST_RX_SMALL_DGRAM++);
    RTCSLOG_PCB_FREE(pcb_ptr, RTCSERR_UDP_BAD_HEADER);
    RTCSPCB_free(pcb_ptr);
    return;
  } /* Endif */

  if(
      RTCSPCB_SIZE(pcb_ptr) < len
    )
  {
    IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.COMMON.ST_RX_DISCARDED++);
    IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.ST_RX_SMALL_PKT++);
    RTCSLOG_PCB_FREE(pcb_ptr, RTCSERR_UDP_BAD_HEADER);
    RTCSPCB_free(pcb_ptr);
    return;
  } /* Endif */

  RTCSLOG_PCB_READ(pcb_ptr, RTCS_LOGCTRL_PROTO(IPPROTO_UDP), 0);

   /*
   ** Port zero is an illegal destination
   */
   if(
        port == 0
     )
   {
      IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.COMMON.ST_RX_DISCARDED++);
      IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.ST_RX_BAD_PORT++);
      RTCSLOG_PCB_FREE(pcb_ptr, RTCSERR_UDP_BAD_HEADER);
      RTCSPCB_free(pcb_ptr);
      return;
   } /* Endif */

   /*
   ** Find a listening UCB, and move it to the front.
   ** There are multiple lists to search. If the UNICAST bit in the pcb is not
   ** set, we will only check the BOUND list, because this is the only list
   ** that can receive Multicast or Broadcast packets. No ucb that is bound to
   ** a specific local IP can receive a multicast or broadcast packet.
   */

  /* extract source port number */
  src_port = mqx_ntohs(packet->SRC_PORT);
  srcaddr.sa_family = af;
  destaddr.sa_family = af;

#if RTCSCFG_ENABLE_IP4
  #if RTCSCFG_ENABLE_IP6  
  if(af == AF_INET) 
  {
  #endif
    SOCKADDR_init((void*)IP_source(pcb_ptr), src_port, &srcaddr);
    SOCKADDR_init((void*)IP_dest(pcb_ptr), port, &destaddr);
  #if RTCSCFG_ENABLE_IP6
  }
  #endif
#endif
#if RTCSCFG_ENABLE_IP6
  #if RTCSCFG_ENABLE_IP4  
  else if(af == AF_INET6) 
  {
  #endif
   
    /*convert packet to IP6_HEADER_PTR type*/
    ip6_packet = (IP6_HEADER_PTR)RTCSPCB_DATA_NETWORK(pcb_ptr);
    SOCKADDR_init((in6_addr *)ip6_packet->SOURCE, src_port, &srcaddr);
    SOCKADDR_init((in6_addr *)ip6_packet->DEST, port, &destaddr);
  #if RTCSCFG_ENABLE_IP4
  }
  #endif
#endif

   /*
   ** Only search this list if the packet might be unicast.
   ** The search criterion is matching host PORT/IP and matching peer PORT/IP
   */
   if(
        pcb_ptr->TYPE & RTCSPCB_TYPE_UNICAST
     )
   {
      RTCS_LIST_PEEK(UDP_cfg_ptr->OPEN_UCB_HEAD, ucb_ptr);
      if(
            ucb_ptr &&(
                        (!SOCKADDR_ip_and_port_are_equal(&ucb_ptr->RADDR, &srcaddr))    ||
                        (!SOCKADDR_ip_and_port_are_equal(&ucb_ptr->LADDR, &destaddr))   ||
                        ((ucb_ptr->LADDR).sa_family != af)
                      )
        )
      {
            ucb_ptr = NULL;
            RTCS_LIST_SEARCHMOD_NOHEAD(UDP_cfg_ptr->OPEN_UCB_HEAD, search_ptr)
            {
                if(
                  ((search_ptr)&&(*search_ptr)) && (
                    SOCKADDR_ip_and_port_are_equal(&(*search_ptr)->RADDR, &srcaddr) &&
                    SOCKADDR_ip_and_port_are_equal(&(*search_ptr)->LADDR, &destaddr) &&
                    ((*search_ptr)->LADDR.sa_family == af))
                  )
                {
                    ucb_ptr = *search_ptr;
                    RTCS_LIST_DEL(UDP_cfg_ptr->OPEN_UCB_HEAD, search_ptr);
                    RTCS_LIST_INS(UDP_cfg_ptr->OPEN_UCB_HEAD, ucb_ptr);
                    break;
                } /* Endif */
            } /* End SEARCH */
      } /* Endif */
   } /* Endif */
   
  #if RTCSCFG_UDP_ENABLE_LBOUND_MULTICAST && RTCSCFG_ENABLE_IGMP && RTCSCFG_ENABLE_IP4
   /*
   ** If no OPEN ucb matches, search the LBOUND ucb list,
   ** allowing multicast.  The local port must match the
   ** packet destination port.
   */
   #if RTCSCFG_ENABLE_IP6
   if(af == AF_INET)
   {
   #endif
     if (!ucb_ptr && UDP_cfg_ptr->LBOUND_UCB_HEAD)  {
        RTCS_LIST_PEEK(UDP_cfg_ptr->LBOUND_UCB_HEAD, ucb_ptr);
        if (ucb_ptr &&
          ((SOCKADDR_get_port(&ucb_ptr->LADDR) != port) || 
           ((SOCKADDR_ip_is_multicast(&destaddr)) &&
           (!IGMP_is_member(&ucb_ptr->MCB_PTR, pcb_ptr->IFSRC, SOCKADDR_get_ipaddr4(&destaddr)))) ||
           (ucb_ptr->LADDR.sa_family != AF_INET)))
        {
           ucb_ptr = NULL;
           RTCS_LIST_SEARCHMOD_NOHEAD(UDP_cfg_ptr->LBOUND_UCB_HEAD, search_ptr)  {
              if (((SOCKADDR_get_port(&(*search_ptr)->LADDR) == port) && ((*search_ptr)->LADDR.sa_family == AF_INET) && (!SOCKADDR_ip_is_multicast(&destaddr))) ||
                  ((SOCKADDR_get_port(&(*search_ptr)->LADDR) == port) && (SOCKADDR_ip_is_multicast(&destaddr)) && (IGMP_is_member(&(*search_ptr)->MCB_PTR, pcb_ptr->IFSRC, SOCKADDR_get_ipaddr4(&destaddr)))))
              {
                 ucb_ptr = *search_ptr;
                 RTCS_LIST_DEL(UDP_cfg_ptr->LBOUND_UCB_HEAD, search_ptr);
                 RTCS_LIST_INS(UDP_cfg_ptr->LBOUND_UCB_HEAD, ucb_ptr);
                 break;
              } /* Endif */
           } /* End SEARCH */
        } /* Endif */
     } /* Endif */
   #if RTCSCFG_ENABLE_IP6     
     goto bound_ucb_head;
   }
   #endif
  #endif

   /*
   ** If no OPEN ucb matches, search the LBOUND ucb list.
   ** Only search this list if the packet could be unicast.
   ** The search criterion is a matching local IP/PORT
   */
#if RTCSCFG_ENABLE_IP6 || (RTCSCFG_ENABLE_IP4 && (!RTCSCFG_ENABLE_IGMP || (!RTCSCFG_UDP_ENABLE_LBOUND_MULTICAST && RTCSCFG_ENABLE_IGMP)))
   if(
        !ucb_ptr && UDP_cfg_ptr->LBOUND_UCB_HEAD &&
        (pcb_ptr->TYPE & RTCSPCB_TYPE_UNICAST)
     )
   {
        RTCS_LIST_PEEK(UDP_cfg_ptr->LBOUND_UCB_HEAD, ucb_ptr);
        if(
            ucb_ptr &&
            ((!SOCKADDR_ip_and_port_are_equal(&ucb_ptr->LADDR, &destaddr)) ||
            (ucb_ptr->LADDR).sa_family != af)
         )
        {
            ucb_ptr = NULL;
            RTCS_LIST_SEARCHMOD_NOHEAD(UDP_cfg_ptr->LBOUND_UCB_HEAD, search_ptr)
            {
                if(
                    (SOCKADDR_ip_and_port_are_equal(&(*search_ptr)->LADDR, &destaddr)) &&
                    ((*search_ptr)->LADDR).sa_family == af
                  )
                {
                    ucb_ptr = *search_ptr;
                    RTCS_LIST_DEL(UDP_cfg_ptr->LBOUND_UCB_HEAD, search_ptr);
                    RTCS_LIST_INS(UDP_cfg_ptr->LBOUND_UCB_HEAD, ucb_ptr);
                    break;
                } /* Endif */
            } /* End SEARCH */
        }/* Endif */
    } /* Endif */
#endif

#if RTCSCFG_UDP_ENABLE_LBOUND_MULTICAST && RTCSCFG_ENABLE_IGMP && RTCSCFG_ENABLE_IP4 && RTCSCFG_ENABLE_IP6
bound_ucb_head:
#endif
   /*
   ** If no LBOUND ucb matches, search the BOUND ucb list
   ** Always search this list, regardless of packet type.
   ** The local port must match the packet destination port
   */
   if(!ucb_ptr && UDP_cfg_ptr->BOUND_UCB_HEAD)
   {
        RTCS_LIST_PEEK(UDP_cfg_ptr->BOUND_UCB_HEAD, ucb_ptr);
        if((ucb_ptr && (SOCKADDR_get_port(&ucb_ptr->LADDR) != port)) || ((ucb_ptr->LADDR).sa_family != af))
        {
            ucb_ptr = NULL;
            RTCS_LIST_SEARCHMOD_NOHEAD(UDP_cfg_ptr->BOUND_UCB_HEAD, search_ptr)
            {
                if((SOCKADDR_get_port(&(*search_ptr)->LADDR) == port) && (((*search_ptr)->LADDR).sa_family == af))
                {
                    ucb_ptr = *search_ptr;
                    RTCS_LIST_DEL(UDP_cfg_ptr->BOUND_UCB_HEAD, search_ptr);
                    RTCS_LIST_INS(UDP_cfg_ptr->BOUND_UCB_HEAD, ucb_ptr);
                    break;
                } /* Endif */
            } /* End SEARCH */
        }/* Endif */
        
        if(ucb_ptr && SOCKADDR_ip_is_multicast(&destaddr))
        {
        #if RTCSCFG_ENABLE_IP4
          #if RTCSCFG_ENABLE_IP6
          if(af == AF_INET)
          {
          #endif
            #if RTCSCFG_ENABLE_IGMP && RTCSCFG_ENABLE_IP4
              if (!IGMP_is_member(&ucb_ptr->MCB_PTR, pcb_ptr->IFSRC, SOCKADDR_get_ipaddr4(&destaddr))) 
              {
                ucb_ptr = NULL;
              }
            #else
              ucb_ptr = NULL;
            #endif
          #if RTCSCFG_ENABLE_IP6
          }
          #endif
        #endif
        #if RTCSCFG_ENABLE_IP6
          #if RTCSCFG_ENABLE_IP4
          else if(af == AF_INET6)
          {
          #endif
            if(!ip6_multicast_find_socket_entry(((SOCKET_STRUCT_PTR)ucb_ptr->SOCKET), pcb_ptr->IFSRC, SOCKADDR_get_ipaddr6(&destaddr) ))
            {            
              ucb_ptr = NULL;
            }
          #if RTCSCFG_ENABLE_IP4
          }
          #endif
        #endif
        }
   }

#if RTCSCFG_ENABLE_IP6
  #if RTCSCFG_ENABLE_IP4
  if(af == AF_INET6)
  {
  #endif
    /* Lets check scope_id there binded socket and scope _id of input packet*/
    error = RTCS6_if_get_scope_id((_rtcs_if_handle)pcb_ptr->IFSRC); 

    /*
     * If scope_id of input packet is different from UCB scope_id set UCB to NULL and
     * UCB to NULL, to discard packet and generate error.
     */
    if(((sockaddr_in6*)&ucb_ptr->LADDR)->sin6_scope_id)
    {
        if(((sockaddr_in6*)&ucb_ptr->LADDR)->sin6_scope_id != error)
        {
            ucb_ptr = NULL;
        }
    }
  #if RTCSCFG_ENABLE_IP4
  }
  #endif
#endif
   /*
   ** If no UCB was found on the destination port, discard packet and generate
   ** a ICMP error packet
   */
   if(!ucb_ptr)
   {
        IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.COMMON.ST_RX_DISCARDED++);
        IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.ST_RX_NO_PORT++);
        #if RTCSCFG_ENABLE_IP4
            #if RTCSCFG_ENABLE_IP6
            if(af == AF_INET)
            {
            #endif
              #if RTCSCFG_ENABLE_ICMP
              ICMP_send_error(ICMPTYPE_DESTUNREACH, ICMPCODE_DU_PORT_UNREACH, 0, pcb_ptr, -1);
              #endif
            #if RTCSCFG_ENABLE_IP6
            }
            #endif
        #endif
        #if RTCSCFG_ENABLE_IP6
            #if RTCSCFG_ENABLE_IP4
            else if(af == AF_INET6)
            {
            #endif
              ICMP6_send_error (ICMP6_TYPE_DEST_UNREACH, ICMP6_CODE_DU_PORT_UNREACH,
                                0   /* [IN] error parameter */,
                                pcb_ptr   /* [IN] the packet which caused the error */);
            #if RTCSCFG_ENABLE_IP4
            }
            #endif
        #endif
        RTCSLOG_PCB_FREE(pcb_ptr, RTCS_OK);
        RTCSPCB_free(pcb_ptr);
        return;
   } /* Endif */
    
#if BSPCFG_ENET_HW_RX_PROTOCOL_CHECKSUM
    /* HW checksum offload.*/
    if((pcb_ptr->TYPE & RTCSPCB_TYPE_HW_PROTOCOL_CHECKSUM)==0)
#endif
    {
        /* Verify the checksum if present. */
        if(mqx_ntohs(packet->CHECKSUM) && !ucb_ptr->BYPASS_RX)
        {
        #if RTCSCFG_ENABLE_IP4
            #if RTCSCFG_ENABLE_IP6
            if(af == AF_INET)
            {
            #endif
              sum = IP_Sum_pseudo(len, pcb_ptr, -1);
            #if RTCSCFG_ENABLE_IP6
            }
            #endif
        #endif
        #if RTCSCFG_ENABLE_IP6
            #if RTCSCFG_ENABLE_IP4
            else if(af == AF_INET6)
            {
            #endif
              /*IP6 use different method for calculation checksum*/
              sum = IP6_Sum_pseudo(RTCSPCB_SIZE(pcb_ptr), pcb_ptr);
            #if RTCSCFG_ENABLE_IP4
            }
            #endif
        #endif
            sum = IP_Sum_PCB(sum, pcb_ptr);

            if (sum != 0xFFFF)
            {
                IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.COMMON.ST_RX_DISCARDED++);
                IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.ST_RX_BAD_CHECKSUM++);
                RTCSLOG_PCB_FREE(pcb_ptr, RTCSERR_UDP_BAD_CHECKSUM);
                RTCSPCB_free(pcb_ptr);
                return;
            } /* Endif */
        } /* Endif */
    }

   /*
   ** Although RTCSPCB_shrink() always shrinks the first fragment,
   ** this is OK, because if RTCSPCB_SIZE(pcb_ptr) != len, then the
   ** datagram must have originated from another host, in which
   ** case the PCB only has one fragment.
   **
   ** If the packet originated on this host (and was looped back),
   ** the datagram could have several fragments, but in this case,
   ** RTCSPCB_SIZE(pcb_ptr) == len, so we'll always shrink zero bytes.
   */
   error = RTCSPCB_shrink(pcb_ptr, RTCSPCB_SIZE(pcb_ptr) - len);
   if(error)
   {
        IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.COMMON.ST_RX_ERRORS++);
        IF_UDP_STATS_ENABLED(RTCS_seterror(&UDP_cfg_ptr->STATS.ERR_RX, error, (uint32_t)pcb_ptr));
        RTCSLOG_PCB_FREE(pcb_ptr, error);
        RTCSPCB_free(pcb_ptr);
        return;
   } /* Endif */
   error = RTCSPCB_next(pcb_ptr, sizeof(UDP_HEADER));
   if(error)
   {
        IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.COMMON.ST_RX_ERRORS++);
        IF_UDP_STATS_ENABLED(RTCS_seterror(&UDP_cfg_ptr->STATS.ERR_RX, error, (uint32_t)pcb_ptr));
        RTCSLOG_PCB_FREE(pcb_ptr, error);
        RTCSPCB_free(pcb_ptr);
        return;
   } /* Endif */

   ucb_ptr->SERVICE(pcb_ptr, ucb_ptr);

}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : UDP_process
* Returned Values : void
* Comments        :
*     Process valid incoming UDP packets for IPv4 or IPv6 family. Called from UDP_service.
*
*END*-----------------------------------------------------------------*/
void UDP_process
   (
      RTCSPCB_PTR    pcb_ptr,          /* [IN/OUT] incoming packet */
      UCB_STRUCT_PTR ucb_ptr           /* [IN]     target UCB      */
   )
{ /* Body */
   IF_UDP_STATS_ENABLED(UDP_CFG_STRUCT_PTR   UDP_cfg_ptr);
   UDP_PARM_PTR         req_ptr;

   IF_UDP_STATS_ENABLED(UDP_cfg_ptr = RTCS_getcfg(UDP));
  
  /* this function cannot be called from nosock UCB. test it. */
   if((FALSE == UCB_exists_in_system(ucb_ptr)) || (FALSE == SOCK_exists_in_system(ucb_ptr->SOCKET)))
   {
     IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.COMMON.ST_RX_MISSED++);
     RTCSLOG_PCB_FREE(pcb_ptr, RTCS_OK);
     RTCSPCB_free(pcb_ptr);
     return;
   }

   /*
   ** If there is a pending request, unblock it
   */
   RTCS_QUEUE_PEEK(ucb_ptr->RHEAD, ucb_ptr->RTAIL, req_ptr);
   if(req_ptr)
   {
      RTCS_QUEUE_DEL_NONEMPTY(ucb_ptr->RHEAD, ucb_ptr->RTAIL);
      udp_return_req2socket_from_pcb(req_ptr, pcb_ptr);
      
      RTCSLOG_PCB_FREE(pcb_ptr, RTCS_OK);
      RTCSPCB_free(pcb_ptr);
      if(req_ptr->EXPIRE.PRIVATE)
      {
        TCPIP_Event_cancel(&req_ptr->EXPIRE);
      }
      RTCSCMD_complete(req_ptr, RTCS_OK);
      return;
   }
   else if ((ucb_ptr->PCOUNT + RTCSPCB_SIZE(pcb_ptr)) >= ((struct socket_struct *)(ucb_ptr->SOCKET))->RBSIZE)
   {
       IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.COMMON.ST_RX_MISSED++);
       RTCSLOG_PCB_FREE(pcb_ptr, RTCS_OK);
       RTCSPCB_free(pcb_ptr);
       return;
   /*
   ** Otherwise, enqueue the packet
   */
   }
   else
   {
      struct udp_rx_dgram_header * dgram_item = RTCS_mem_alloc_system(udp_get_rx_item_size(pcb_ptr));
      if(NULL == dgram_item)
      {
        IF_UDP_STATS_ENABLED(UDP_cfg_ptr->STATS.COMMON.ST_RX_MISSED++);
        RTCSLOG_PCB_FREE(pcb_ptr, RTCS_OK);
        RTCSPCB_free(pcb_ptr);
      }
      else
      {
        _mem_set_type(dgram_item, MEM_TYPE_UDP_RX_BUFFER);
      
        /* store information for socket layer and free PCB. */
        udp_set_rx_dgram_header_and_data(dgram_item, pcb_ptr, ucb_ptr->LADDR.sa_family);
        RTCS_QUEUE_INS(ucb_ptr->PHEAD, ucb_ptr->PTAIL, dgram_item);
        ucb_ptr->PCOUNT += dgram_item->size;

        /*
        ** Notify socket layer in case some task is blocked on select()
        ** waiting for this data.
        */      
        SOCK_select_signal(ucb_ptr->SOCKET, UDP_SOCKET_ENQUEUED, RTCS_OK);
      }
   }
} /* Endbody */

static void udp_release_pending_receive_requests(struct ucb_struct *ucb)
{
  struct ucb_parm *req;
  struct ucb_parm *nextreq;
  RTCS_QUEUE_DELALL(ucb->RHEAD, ucb->RTAIL, req, nextreq) 
  {
    if(req->EXPIRE.TIME)
    {
      TCPIP_Event_cancel(&req->EXPIRE);
    }
    RTCSCMD_complete(req, RTCSERR_READ_ABORTED);
  } /* End DELALL */
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : UDP_close
* Parameters      :
*
*     UCB_STRUCT_PTR    ucb         [IN] UCB
*    _CODE_PTR_         udpservice  not used
*     _ip_address       ipaddress   not used
*     uint16_t           udpport     not used
*     uint16_t           udpflags    not used
*     pointer           udpptr      not used
*     uint32_t           udpword     not used
*
* Comments        :
*     Closes a UCB.
*
*END*-----------------------------------------------------------------*/

void UDP_close
   (
      UDP_PARM_PTR      parms
   )
{ /* Body */
    UDP_CFG_STRUCT_PTR   UDP_cfg_ptr;
    UCB_STRUCT_PTR       ucb, * tmp;
    UCB_STRUCT_PTR      *search_ptr;       /* Temporary UCB pointer */
    //RTCSPCB_PTR          pcb, nextpcb;     /* Temporary packet pointers */
    struct udp_rx_dgram_header * dgram_item;
    struct udp_rx_dgram_header * next_dgram_item;
    uint32_t             error;

    UDP_cfg_ptr = RTCS_getcfg(UDP);
    ucb = parms->ucb;
    
    /* this function can be called from nosock UCB, that is, ucb->SOCKET can be zero. 
     * test only UCB.
     */
    if(FALSE == UCB_exists_in_system(ucb))
    {
      RTCSCMD_complete(parms, RTCSERR_UDP_UCB_CLOSE);
      return;
    }

#if RTCSCFG_ENABLE_IP6
    { /* Leave all IPv6 multicast groups.*/
      int i;
      for(i = 0; i < RTCSCFG_IP6_MULTICAST_SOCKET_MAX; i++)
      {
        if (ucb->IP6_MULTICAST_PTR[i]!= NULL) 
        {
          ip6_multicast_leave_entry(ucb->IP6_MULTICAST_PTR[i]);
        }
      }
    }
#endif
   
    /*
    ** Remove the UCB from the chain of UCBs
    */
    tmp = NULL;
    if(!SOCKADDR_get_port(&ucb->LADDR))
    {
      tmp = &(UDP_cfg_ptr->GROUND_UCB_HEAD);
    }
    else if (SOCKADDR_ip_is_zero(&ucb->RADDR))
    {
      if(!SOCKADDR_ip_is_zero(&ucb->LADDR))
      {
        tmp = &(UDP_cfg_ptr->LBOUND_UCB_HEAD);
      }
      else
      {
        tmp = &(UDP_cfg_ptr->BOUND_UCB_HEAD);
      }     
      
      if(NULL == tmp) 
      {
        RTCSCMD_complete(parms, RTCSERR_UDP_UCB_CLOSE);
        return;
      }
    }
    else
    {
      tmp = &(UDP_cfg_ptr->OPEN_UCB_HEAD);
    }
        
    RTCS_LIST_SEARCHMOD(*tmp, search_ptr) {
      if (*search_ptr == ucb) {
        break;
      } /* Endif */
    } /* End SEARCH */

    if (!*search_ptr) {
      RTCSCMD_complete(parms, RTCSERR_UDP_UCB_CLOSE);
      return;
    } /* Endif */

    RTCS_LIST_DEL(tmp, search_ptr);

    /*
    ** Free all queued packets
    */
    RTCS_QUEUE_DELALL(ucb->PHEAD, ucb->PTAIL, dgram_item, next_dgram_item) {
      _mem_free(dgram_item);
    } /* End DELALL */

    /*
    ** Release pending receive requests from host
    */
    udp_release_pending_receive_requests(ucb);

    /*
    ** Notify socket layer in case some task is blocked on select()
    ** waiting for data on this socket.
    */
    SOCK_select_signal(ucb->SOCKET, UDP_SOCKET_CLOSE, RTCS_OK);

    /*
    ** Leave all joined multicast groups
    */
    if (ucb->IGMP_LEAVEALL) {
      ucb->IGMP_LEAVEALL(&ucb->MCB_PTR);
    } /* Endif */

    /*
    ** Free the UCB memory
    */
    error = _mem_free(ucb);
    if (error) {
      RTCSCMD_complete(parms, RTCSERR_UDP_UCB_FREE);
      return;
    } /* Endif */

    RTCSCMD_complete(parms, RTCS_OK);
    return;
}

void UDP_shutdown
   (
      UDP_PARM_PTR      parms
   )
{
  struct ucb_struct *ucb;
  uint32_t disallow_action = 0;
  uint32_t error = RTCS_OK;
  
  ucb = parms->ucb;
    
  if(FALSE == UCB_exists_in_system(ucb))
  {
    goto EXIT; /* UDP_close must have released UCB_PTR. just return in this case. */
  }
  
  /* this function cannot be called for nosock ucb */
  if(FALSE == SOCK_exists_in_system(ucb->SOCKET))
  {
    error = RTCSERR_SOCK_INVALID;
    goto EXIT;    
  }
  
  disallow_action = SOCK_set_disallow_mask(ucb->SOCKET, parms->udpword);
  
  /*
    ** Release pending receive requests from host.
    ** in udpword we have bool value from upper layer if this shutdownsocket() has SHUT_RD or SHUT_RDWR flag.
    */
  if(SOCK_disallow_recv(disallow_action))
  {
    udp_release_pending_receive_requests(ucb);
  }
  
  SOCK_select_signal(ucb->SOCKET, UDP_SOCKET_CLOSE, RTCSERR_SOCK_ESHUTDOWN);
  EXIT:
  RTCSCMD_complete(parms, error);
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : UDP_source
* Returned Value  : source UDP port
* Comments        :
*     Return source UDP port of packet.
*
*END*-----------------------------------------------------------------*/

uint16_t UDP_source
   (
      RTCSPCB_PTR    pcb      /* [IN] packet to find source of */
   )
{
    uint16_t port = 0;
    unsigned char *srcptr = ((UDP_HEADER_PTR)RTCSPCB_DATA_TRANSPORT(pcb))->SRC_PORT;
    port = mqx_ntohs(srcptr);
    RTCS_ASSERT(port);
    return port;
}
/* EOF */
