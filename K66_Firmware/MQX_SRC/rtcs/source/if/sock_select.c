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
* $FileName: sock_select.c$
* Comments:
*
*   This file contains the select() implementation.
*
*END************************************************************************/

#include <rtcs.h>
#include "rtcs_prv.h"
#include "tcpip.h"
#include "tcp_prv.h"    /* for TCP internal definitions */
#include "udp_prv.h"    /* for UDP internal definitions */
#include "socket.h"

/* RTCS log */
#define RTCS_ENTER(f,a)    RTCSLOG_API(RTCSLOG_TYPE_FNENTRY, RTCSLOG_FN_SOCK_ ## f, a)

#define RTCS_EXIT(f,a)     RTCSLOG_API(RTCSLOG_TYPE_FNEXIT,  RTCSLOG_FN_SOCK_ ## f, a); \
                           return a

/* definitions local for this source file */
#define SOCK_ACTIVITY_NONE              0
#define SOCK_ACTIVITY_ON_READ_FDS       1
#define SOCK_ACTIVITY_ON_WRITE_FDS      2
#define SOCK_ACTIVITY_ON_READWRITE_FDS  (SOCK_ACTIVITY_ON_READ_FDS | SOCK_ACTIVITY_ON_WRITE_FDS)
#define SOCK_ACTIVITY_ON_EXCEPT_FDS     4

#define SOCK_ACT_TYPE_CLOSE 1
#define SOCK_ACT_TYPE_RECV 2
#define SOCK_ACT_TYPE_SEND 3
#define SOCK_ACT_TYPE_EXCEPT 4
#define SOCK_ACT_TYPE_NONE 0

#define CHECK_RX_AND_SHUTDOWN 1
#define CHECK_TX_AND_SHUTDOWN 2
#define CHECK_EXCEPT_AND_SHUTDOWN 3

/* forward declarations for static functions */
static void SOCK_poll_fds(SOCK_SELECT_PARM_PTR);
static uint32_t SOCK_select_activity (SOCKET_STRUCT_PTR, int32_t);
static void SOCK_select_block    (SOCK_SELECT_PARM_PTR);
static bool SOCK_select_expire   (TCPIP_EVENT_PTR);
static void SOCK_select_unblock  (SOCK_SELECT_PARM_PTR, uint32_t, uint32_t);

#if RTCSCFG_BACKWARD_COMPATIBILITY_RTCSSELECT==1
static void SOCK_build_fds(SOCK_SELECT_PARM_PTR);
#endif

static uint32_t sock_fds_poll(rtcs_fd_set * fds_ptr, uint32_t size, int32_t checkwhat, int32_t *error);
static uint32_t sock_fds_check(rtcs_fd_set *fds_ptr);
static uint32_t sock_fds_signal(rtcs_fd_set *fds_ptr, const uint32_t sock, uint32_t size);

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_select_activity
* Returned Value  : SOCK_ACT_TYPE_NONE, SOCK_ACT_TYPE_RECV, SOCK_ACT_TYPE_CLOSE, SOCK_ACT_TYPE_SEND
* Comments        : Determine whether there is any activity on a socket.
*
*END*-----------------------------------------------------------------*/

static uint32_t SOCK_select_activity
   (
      SOCKET_STRUCT_PTR    socket_ptr,
      int32_t checkwhat                          // CHECK_RX_AND_SHUTDOWN,CHECK_TX_AND_SHUTDOWN,CHECK_EXCEPT_AND_SHUTDOWN 
   )
{
  TCB_STRUCT_PTR tcb_ptr;
  uint32_t       activity = SOCK_ACT_TYPE_NONE;

  /* Check closing and connected stream sockets for data */
  if(socket_ptr->TCB_PTR)
  {
    tcb_ptr = socket_ptr->TCB_PTR;
    
    if(tcb_ptr->state == CLOSED)
    {
      activity = SOCK_ACT_TYPE_CLOSE;
    }
    else
    {
      if(CHECK_RX_AND_SHUTDOWN == checkwhat)
      {
        /*
        ** Check that there is data in the receive ring or
        ** that the socket has been closed by the peer
        */
        
        if((tcb_ptr->conn_pending) || (GT32(tcb_ptr->rcvnxt, tcb_ptr->rcvbufseq)))
        {
          activity = SOCK_ACT_TYPE_RECV;
        }      
      }
      else
      {
        if(!tcb_ptr->sndlen) // TODO - remake for partialy empty send buffers
        {    
          activity = SOCK_ACT_TYPE_SEND;
        }
      }
    }    
   /* Check datagram sockets for data */
  } 
  else if (socket_ptr->UCB_PTR)
  {
    if(socket_ptr->disallow_mask)
    {
      /* shutdownsocket() must have been called. */
      activity = SOCK_ACT_TYPE_CLOSE;
    }
    else
    {
      if(CHECK_RX_AND_SHUTDOWN == checkwhat)
      {
        /*
        ** Check that there is queued data
        */
        if(socket_ptr->UCB_PTR->PHEAD)
        {
          activity = SOCK_ACT_TYPE_RECV;
        }
      }
      else
      {
        activity = SOCK_ACT_TYPE_SEND; /* for now assume we can always UDP send. */
      }
    }
  /* TCB=UCB=NULL is a TCP connection reset by peer */
  }
  else
  {
    activity = SOCK_ACT_TYPE_CLOSE;
  }
  
  if((activity != SOCK_ACT_TYPE_CLOSE) && (CHECK_EXCEPT_AND_SHUTDOWN == checkwhat))
  {
    if(socket_ptr->exceptpending != 0)
    {
      activity = SOCK_ACT_TYPE_EXCEPT;
    }
    else
    {
      /* CHECK_EXCEPT_AND_SHUTDOWN we are interested only in SOCK_ACT_TYPE_CLOSE or SOCK_ACT_TYPE_EXCEPT.
       * so if neither was detected, say no activity to exceptfds.
       */
      activity = SOCK_ACT_TYPE_NONE;
    }
  }

  return activity;
}


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_select_block
* Returned Values :
* Comments  :
*     Enqueues a select() call on the select queue.
*
*END*-----------------------------------------------------------------*/

static void SOCK_select_block
   (
      SOCK_SELECT_PARM_PTR  parms
   )
{ /* Body */
   SOCKET_CONFIG_STRUCT_PTR   socket_cfg_ptr = RTCS_getcfg(SOCKET);

   parms->NEXT = socket_cfg_ptr->SELECT_HEAD;
   if (parms->NEXT) {
      parms->NEXT->PREV = &parms->NEXT;
   } /* Endif */
   socket_cfg_ptr->SELECT_HEAD = parms;
   parms->PREV = &socket_cfg_ptr->SELECT_HEAD;

   if (parms->timeout) {
      parms->EXPIRE.TIME    = parms->timeout;
      parms->EXPIRE.EVENT   = SOCK_select_expire;
      parms->EXPIRE.PRIVATE = parms;
      TCPIP_Event_add(&parms->EXPIRE);
   } /* Endif */

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_select_expire
* Returned Values :
* Comments  :
*     Called by the Timer.  Expire a select call.
*
*END*-----------------------------------------------------------------*/

static bool SOCK_select_expire
   (
      TCPIP_EVENT_PTR   event
   )
{ /* Body */
   SOCK_SELECT_PARM_PTR  parms = event->PRIVATE;

   if (parms->NEXT) {
      parms->NEXT->PREV = parms->PREV;
   } /* Endif */
   *parms->PREV = parms->NEXT;

  parms->sock = 0;
  if(NULL != parms->readfds)
  {
    RTCS_FD_ZERO(parms->readfds);
  }
  if(NULL != parms->writefds)
  {
    RTCS_FD_ZERO(parms->writefds);
  }
  RTCSCMD_complete(parms, RTCS_OK);

  return FALSE;
} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_select_unblock
* Returned Values :
* Comments  :
*     Dequeues a select() call from the select queue.
*
*END*-----------------------------------------------------------------*/

static void SOCK_select_unblock
   (
      SOCK_SELECT_PARM_PTR  parms,
      uint32_t  state,      // TCP state or UDP socket flag
      uint32_t  errnum
   )
{

   if (parms->NEXT) {
      parms->NEXT->PREV = parms->PREV;
   } 

   *parms->PREV = parms->NEXT;

   if (parms->timeout)
   {
      TCPIP_Event_cancel(&parms->EXPIRE);
   } 

   /* Check TCP state and UDP socket flag */
   if ( (state == CLOSED) || (state == UDP_SOCKET_CLOSE) )
   {
     if((errnum == RTCSERR_TCP_CONN_RESET)||(errnum == RTCSERR_SOCK_ESHUTDOWN))
     {
       RTCSCMD_complete(parms, RTCSERR_SOCK_ESHUTDOWN);
     }
     else
     {
       RTCSCMD_complete(parms, RTCSERR_SOCK_CLOSED);
     }
   }
   else
   {
      RTCSCMD_complete(parms, RTCS_OK);
   }
}

/**
 * @internal
 * @brief Find socket in rtcs_fd_set.  
 * If socket is found, rtcs_fd_set is cleared and the socket handle is written to the rtcs_fd_set.
 * Called internally in TCP/IP task context.
 *
 * @return zero if socket is not in the rtcs_fd_set.
 * @return one otherwise.
 */
static uint32_t sock_fds_signal(rtcs_fd_set *fds_ptr, const uint32_t sock, uint32_t size)
{
  int32_t  i;
  uint32_t sockcnt = 0;
  
  if(!fds_ptr)
  {
    goto exit;
  }
  
  for(i=0; (i < RTCSCFG_FD_SETSIZE) && size ;i++)
  {
    if(fds_ptr->fd_array[i])
    {
      if(fds_ptr->fd_array[i] == sock)
      {
        sockcnt++;
        RTCS_FD_ZERO(fds_ptr);
        RTCS_FD_SET(sock, fds_ptr);
        break;
      }
      size--; /* decrease with each non-zero socket handle */
    }          
  }
  
exit:
  return sockcnt;
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_select_signal
* Returned Values :
* Comments  :
*     Unblocks all select() calls blocked on a specific socket.
*
*END*-----------------------------------------------------------------*/

void _SOCK_select_signal
   (
      uint32_t  sock,
      uint32_t  state,      // TCP state or UDP socket flag
      uint32_t  flag,       // direction flag - RTCS_SIGNAL_WRITE, RTCS_SIGNAL_READ, RTCS_SIGNAL_EXCEPT
      uint32_t  errnum
   )
{ /* Body */
   SOCKET_CONFIG_STRUCT_PTR   socket_cfg_ptr = RTCS_getcfg(SOCKET);
   SOCK_SELECT_PARM_PTR       parms;
   SOCK_SELECT_PARM_PTR       nextparms;
   uint32_t                   poll_act_mask = SOCK_ACTIVITY_NONE;
   uint32_t                   sockcnt = 0;

   if (!sock)
   {
      return;
   } /* Endif */

   for (parms = socket_cfg_ptr->SELECT_HEAD; parms; parms = nextparms)
   {
      nextparms = parms->NEXT;
      poll_act_mask = SOCK_ACTIVITY_NONE;

      if(flag == RTCS_SIGNAL_WRITE)
      {
        sockcnt = sock_fds_signal(parms->writefds, sock, parms->sock_count);
        if(sockcnt)
        {
          poll_act_mask = SOCK_ACTIVITY_ON_WRITE_FDS;
          parms->sock = sockcnt;
        }
      }
      
      if(RTCS_SIGNAL_READ == flag)
      {
        sockcnt = sock_fds_signal(parms->readfds, sock, parms->sock_count);
        if(sockcnt)
        {
          poll_act_mask |= SOCK_ACTIVITY_ON_READ_FDS;
          parms->sock += sockcnt;
        }
      }
      
      if(RTCS_SIGNAL_EXCEPT == flag)
      {
        sockcnt = sock_fds_signal(parms->exceptfds, sock, parms->sock_count);
        if(sockcnt)
        {
          poll_act_mask |= SOCK_ACTIVITY_ON_EXCEPT_FDS;
          parms->sock += sockcnt;
        }
      }
      
      if(poll_act_mask != SOCK_ACTIVITY_NONE)
      {
        if((NULL != parms->writefds)&&((SOCK_ACTIVITY_ON_WRITE_FDS & poll_act_mask) == 0))
        {
          RTCS_FD_ZERO(parms->writefds);
        }
        if((NULL != parms->readfds)&&((SOCK_ACTIVITY_ON_READ_FDS & poll_act_mask) == 0))
        { 
          RTCS_FD_ZERO(parms->readfds);
        }
        if((NULL != parms->exceptfds)&&((SOCK_ACTIVITY_ON_EXCEPT_FDS & poll_act_mask) == 0))
        { 
          RTCS_FD_ZERO(parms->exceptfds);
        }
        SOCK_select_unblock(parms, state, errnum);
      }
   } /* Endfor */
} /* Endbody */


#if RTCSCFG_BACKWARD_COMPATIBILITY_RTCSSELECT==1

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_selectset
* Returned Value  : socket handle
* Comments  :  Wait for data or connection requests on any socket
*      in a specified set.
*
*END*-----------------------------------------------------------------*/
uint32_t RTCS_selectset(void *sockset, uint32_t size, uint32_t timeout)
{
  int32_t i;
  rtcs_fd_set rfds;
  /* fd_set wfds; */
  int32_t retval;
  
  RTCS_FD_ZERO(&rfds);
  /* FD_ZERO(&wfds); */
  for(i=0;i<size;i++)
  {
    RTCS_FD_SET(*(((uint32_t*)sockset)+i),&rfds);
    /* FD_SET(*(((uint32_t*)sockset)+i),&wfds); */
  }
  /* retval = select(size,&rfds,&wfds,NULL,timeout); */
  /* with select() we can (if we wish) monitor only receive activity: example on the line below */
  retval = select(size,&rfds,NULL,NULL,timeout);
  if(RTCS_ERROR == retval)
  {
    return RTCS_SOCKET_ERROR;
  }
  else if(0 == retval)
  {
    return 0; /* timeout expired */
  }
  else 
  { /* at least of of the rfds/wfds should be valid. take priority on read activity. */
    for(i=0;i<size;i++)
    {
      if(RTCS_FD_ISSET(*(((uint32_t*)sockset)+i),&rfds))
      {
        return *(((uint32_t*)sockset)+i);
      }
    }
    return 0;
  }
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_selectall
* Returned Value  : socket handle
* Comments  :  Wait for data or connection requests on any socket
*      owned by this task.
*
*END*-----------------------------------------------------------------*/
uint32_t RTCS_selectall
   (
      uint32_t     timeout
         /* [IN] specifies the maximum amount of time to wait for data */
   )
{ /* Body */
   SOCK_SELECT_PARM   parms;
   uint32_t           error;
   rtcs_fd_set rfds;

   RTCS_FD_ZERO(&rfds);

   parms.owner   = RTCS_task_getid();
   parms.timeout = timeout;
   parms.readfds = &rfds;
   /* create socket set from all existing sockets */
   error = RTCSCMD_issue(parms, SOCK_build_fds);
   if (error) return RTCS_SOCKET_ERROR;
   
   return RTCS_selectset(rfds.fd_array, rfds.fd_count,timeout);
  
} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_build_fds
* Parameters      :
*
*     SOCK_SELECT_PARM_PTR      p_param       [IN] pointer to SOCKSELECT_PARM
*
* Comments  : 
*   Cycle through all sockets in the system and build fd_set from the socket handles.
*   This is only to support RTCS_selectall() for backward compatibility.
*   Expected to be called by TCP/IP task.
*
*END*-----------------------------------------------------------------*/
static void SOCK_build_fds(SOCK_SELECT_PARM_PTR p_param)
{
  SOCKET_CONFIG_STRUCT_PTR   socket_cfg_ptr = RTCS_getcfg(SOCKET);
  SOCKET_STRUCT_PTR          socket_ptr;
  
  /* cycle through sockets looking for one owned by this task */
  for (socket_ptr = socket_cfg_ptr->SOCKET_HEAD;
      socket_ptr;
      socket_ptr = socket_ptr->NEXT) 
  {
    /* check owner only if backward compatibility is configured. */
    /* this is specific for compatibility with RTCSCFG_SOCKET_OWNERSHIP */
    /* of RTCS_selectall() function. */
    if(SOCK_Is_owner(socket_ptr, p_param->owner))
    {
      RTCS_FD_SET((uint32_t)socket_ptr,p_param->readfds);
    }
  } /* Endfor */
  RTCSCMD_complete(p_param, RTCS_OK);
}
#endif /* RTCSCFG_BACKWARD_COMPATIBILITY_RTCSSELECT==1 */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : FD_SET
* Parameters      :
*
*     uint32_t      sock       [IN] pointer to SOCKET_STRUCT
*     fd_set        *p_fd_set  [IN] pointer to fd_set
*
* Comments  :  Add socket handle to the array pointed to by p_fd_set.
*   Insert sock to first non-zero array member.
*
*END*-----------------------------------------------------------------*/
void RTCS_FD_SET(const uint32_t sock, rtcs_fd_set * const p_fd_set)
{
  int32_t i;
  if(!SOCK_check_valid(sock) || (NULL == p_fd_set))
  {
    return;
  }
  /* if there is an empty place in the fd_set */  
  if(p_fd_set->fd_count<RTCSCFG_FD_SETSIZE)
  {
    /* search for the 1st empty place */
    for(i=0;i<RTCSCFG_FD_SETSIZE;i++)
    {
      if(0==p_fd_set->fd_array[i]) /* empty place found */ 
      {
        p_fd_set->fd_array[i] = sock;
        p_fd_set->fd_count++;
        break;
      }
    }
  }
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : FD_CLR
* Parameters      :
*
*     uint32_t      sock       [IN] pointer to SOCKET_STRUCT
*     fd_set        *p_fd_set  [IN] pointer to fd_set
*
* Comments  :  Remove socket handle from the array pointed to by p_fd_set.
*  Clear the array member which equals to sock.
*
*END*-----------------------------------------------------------------*/
void RTCS_FD_CLR(const uint32_t sock, rtcs_fd_set * const p_fd_set)
{
  int32_t i;
  if(!SOCK_check_valid(sock) || (NULL == p_fd_set))
  {
    return;
  }
  /* if there is at least one socket in the fd_set */  
  if(p_fd_set->fd_count>0)
  {
    /* search for the one that needs to be cleared */
    for(i=0;i<RTCSCFG_FD_SETSIZE;i++)
    {
      if(sock==p_fd_set->fd_array[i])
      {
        p_fd_set->fd_array[i] = 0;
        p_fd_set->fd_count--;
        break;
      }
    }
  }
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : FD_ZERO
* Parameters      :
*
*     fd_set        *p_fd_set  [IN] pointer to fd_set
*
* Comments  :  Clear out the array
*
*END*-----------------------------------------------------------------*/
void RTCS_FD_ZERO(rtcs_fd_set * const p_fd_set)
{
  int32_t i;
  if(NULL == p_fd_set) return;
  //for(i=0;i<p_fd_set->fd_count;i++)
  for(i=0;i<(sizeof(p_fd_set->fd_array)/sizeof(p_fd_set->fd_array[0]));i++)
  {
    p_fd_set->fd_array[i] = 0;
  }
  p_fd_set->fd_count=0;
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : FD_ISSET
* Parameters      :
*
*     uint32_t      sock       [IN] pointer to SOCKET_STRUCT
*     fd_set        *p_fd_set  [IN] pointer to fd_set
*
* Comments  :  check if sock is the member of fd_set array
*
*END*-----------------------------------------------------------------*/
bool RTCS_FD_ISSET(const uint32_t sock, const rtcs_fd_set * const p_fd_set)
{
  int32_t i;
  if(NULL == p_fd_set) return FALSE;
  if(SOCK_check_valid(sock))
  {
    for(i=0;i<RTCSCFG_FD_SETSIZE;i++)
    {
      if(sock==(p_fd_set->fd_array[i]))
      {
        return TRUE;
      }
    }
  }
  return FALSE;
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : select
* Parameters      :
*
*     int32_t      nfds       [IN] The first nfds socket handles 
*                                  are checked in each set; i.e., 
*                                  the sockets from 0 through nfds-1 
*                                  in the descriptor sets are examined.
*     fd_set *     readfds   [IN/OUT]
*       IN - array of pointers to SOCKET_STRUCT to examine for receive activity
*       OUT - readfds contains the socket handles where activity 
*           has been detected
*     fd_set *     writefds  [IN/OUT]
*       IN - array of pointers to SOCKET_STRUCT to examine for transmit activity
*       OUT - writefds contains the socket handles where activity 
*           has been detected
*     uint32_t     timeout_ms [IN]
*       if timeout_ms is zero, select() may block indefinitely.
*       if timeout_ms is 0xFFFFFFFF, select() only polls the socket descriptors
*         and returns when the actual status is determined.
*       other values determine time limit in milliseconds that the select
*         would block at max
*       

* Comments  :  Due to the restrict keyword the fd_sets must not overlap;
*  readfds, writefds shall not point to the same fd_set structure 
*  in memory.
*  
*  The select() function returns the number of ready sockets that are
*  contained in the descriptor sets, or RTCS_ERROR if an error occurred
*  (RTCS_errno is set appropriately).If the time limit expires, select() returns 0.  
*  
*  Any of readfds, writefds may be given as null pointers if
*  no descriptors are of interest.
*  
*  select() function modifies the content of readfds/writefds.
*
*  SPECIAL NOTES:
*  datagram socket in writefds is reported as always writeable.
*
*END*-----------------------------------------------------------------*/
int32_t select(int32_t nfds, 
                rtcs_fd_set *restrict readfds,
                rtcs_fd_set *restrict writefds,
                rtcs_fd_set *restrict exceptfds,
                uint32_t timeout_ms)
{
  SOCK_SELECT_PARM   parms;
  uint32_t error;
  
  RTCS_ENTER(SELECT, nfds);
  
  /* socket descriptors have to be checked by TCP/IP task */
  parms.readfds = readfds;
  parms.writefds = writefds;
  parms.exceptfds = exceptfds;
  /* greater value from nfds and RTCSCFG_FD_SETSIZE */
  if(nfds<0)
  {
    RTCS_EXIT(SELECT, RTCSERR_SOCK_EINVAL);
  }
  
  if(RTCSCFG_FD_SETSIZE>nfds)
  {
    parms.sock_count = nfds; 
  }
  else
  {
    parms.sock_count = RTCSCFG_FD_SETSIZE;
  }
  
  parms.timeout    = timeout_ms;
  
  parms.sock = 0; /* will be used by SOCK_poll_fds to inform caller about activity */
  if(RTCS_OK != (error = RTCSCMD_issue(parms, SOCK_poll_fds)))
  {
    RTCS_set_errno(error); /* set RTCS_errno */
    RTCS_EXIT(SELECT, RTCS_ERROR);
  }
  RTCS_EXIT(SELECT, parms.sock);
}

/**
 * @internal
 * @brief Check sockets in given rtcs_fd_set.
 *
 * Called internally in TCP/IP task context.
 *
 * @return zero if sockets are OK.
 * @return RTCSERR_SOCK_EBADF on failure.
 */
static uint32_t sock_fds_check(rtcs_fd_set *fds_ptr)
{  
  uint32_t ret_code = RTCS_OK;
  int32_t i = 0;
  uint32_t nonzero_sockcnt = 0;
  
  nonzero_sockcnt = 0;
  for(i=0; i<sizeof(fds_ptr->fd_array)/sizeof(fds_ptr->fd_array[0]); i++)
  {
    if(0 != fds_ptr->fd_array[i])
    {
      nonzero_sockcnt++;
      if(FALSE == SOCK_exists_in_system(fds_ptr->fd_array[i]))
      {
        RTCS_FD_CLR(fds_ptr->fd_array[i], fds_ptr);
        ret_code = RTCSERR_SOCK_EBADF;
      }  
    }
  }
  /* catch fd_set will all zeroes or a missing sock */
  if((nonzero_sockcnt==0)||(nonzero_sockcnt < fds_ptr->fd_count))
  {
    ret_code = RTCSERR_SOCK_EBADF;
  }
  
  return ret_code;
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_correct_fds
* Parameters      :
*
*     SOCK_SELECT_PARM_PTR      p_param       [IN] pointer to SOCKSELECT_PARM
*
* Comments  :
*   Cycle through all sockets in the system and build fd_set from the socket handles.

*   Expected to be called by TCP/IP task.
*
*   Expecting p_param->readfds and p_param->writefds as input fds.
*   Content of both will be checked against invalid values (0, RTCS_SOCKET_ERR
*   and that the sockets really exist in the system).
*   
*   Non-existing socket handles are cleared out in the input arrays.
*   
*   EBADF will be returned if an invalid socket is found in either fd_set.
*
*END*-----------------------------------------------------------------*/
static uint32_t SOCK_correct_fds(SOCK_SELECT_PARM_PTR p_param)
{
  uint32_t error = RTCS_OK;
  
  if(NULL != p_param->readfds)
  {
    error = sock_fds_check(p_param->readfds);
    goto exit;
  }  

  if(NULL != p_param->writefds)
  {
    error = sock_fds_check(p_param->writefds);
  }
  
exit:
  return error;
}

/**
 * @internal
 * @brief Poll sockets in given rtcs_fd_set for activity.
 *
 * Called internally in TCP/IP task context.
 *
 * @return number of sockets with activity.
 * @return One. If any socket in the given set has reset or shutdown request.
 */
static uint32_t sock_fds_poll(rtcs_fd_set * fds_ptr, uint32_t size, int32_t checkwhat, int32_t *error)
{
  uint32_t    *sock_ptr;
  uint32_t     activity_type = SOCK_ACT_TYPE_NONE;
  uint32_t     closed_sock = 0;
  uint32_t     sockcnt = 0; /* num of sockets with activity. */
  rtcs_fd_set  temp_fds; /* temp storage for socket handles to output */
  int32_t      i;

  sock_ptr = fds_ptr->fd_array;
  RTCS_FD_ZERO(&temp_fds);
  for (i = 0; (i < RTCSCFG_FD_SETSIZE) && size; i++) 
  {
    if(*(sock_ptr+i) != 0) /* may be NULL (by incorrect user input or by SOCK_correct_fds) */
    {
      activity_type = SOCK_select_activity((SOCKET_STRUCT_PTR) *(sock_ptr+i), checkwhat);
      if(SOCK_ACT_TYPE_NONE != activity_type)
      {
        if(SOCK_ACT_TYPE_CLOSE == activity_type)
        {
          closed_sock = *(sock_ptr+i);
          sockcnt = 1; /* number of sockets with activity */
          break;
        }
        RTCS_FD_SET(*(sock_ptr+i),&temp_fds);
        sockcnt++; /* number of sockets with activity */
      }
      size--; /* decrease with each non-zero socket */
    }
  }

  if(sockcnt) /* activity has been detected */
  {
    RTCS_FD_ZERO(fds_ptr);
    if(SOCK_ACT_TYPE_CLOSE == activity_type)
    {
      *error = RTCSERR_SOCK_ESHUTDOWN;
      RTCS_FD_SET(closed_sock, fds_ptr);
    }
    else
    {
      for(i=0;i<temp_fds.fd_count;i++)
      {
        RTCS_FD_SET(temp_fds.fd_array[i], fds_ptr);
      }
    }
  }
  
  return sockcnt;
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOCK_poll_fds
* Parameters      :
*
*     uint32_t      sock       [IN] pointer to SOCKET_STRUCT
*     fd_set        *p_fd_set  [IN] pointer to fd_set
*
* Comments  :  Poll socket descriptors for activity.
*   If no activity is detected during poll, block the caller
*   until activity is signalled by TCP/UDP processes.
*   (may block if SOCK_select_block followed by return is called)
*   
*   Expected to be called by TCP/IP task.
*
*END*-----------------------------------------------------------------*/
static void SOCK_poll_fds(SOCK_SELECT_PARM_PTR p_parms)
{
    uint32_t   poll_act_mask = SOCK_ACTIVITY_NONE;
    int32_t    error = RTCS_OK; 

    /*
     * run auto correction function for input socket fd_sets.
     * it will clear out (zero) all socket handles that are invalid
     * invalidity is checked by function FD_ISSET
     * it means the handle is not RTCS_SOCKET_ERROR and that the socket exists in the system
     */
    if(RTCS_OK != SOCK_correct_fds(p_parms)) 
    {
      RTCSCMD_complete(p_parms, RTCSERR_SOCK_EBADF);
      return;
    }
        
    /* poll socket descriptors for a read activity */
    if(NULL != p_parms->readfds)
    {
      uint32_t sockcnt = 0;      
      sockcnt = sock_fds_poll(p_parms->readfds, p_parms->sock_count, CHECK_RX_AND_SHUTDOWN /* for read */, &error);      
      if(sockcnt)
      {
        p_parms->sock = sockcnt; /* num of sockets with activity return to upper layer */
        poll_act_mask = SOCK_ACTIVITY_ON_READ_FDS; /* save info that read activity was detected */     
      }
    }
    
    /* Poll for write activity as RTCS_selectall and RTCS_selectset wait for Rx activity only */
//#if RTCSCFG_BACKWARD_COMPATIBILITY_RTCSSELECT==0
    /* Poll socket descriptors for a write activity.
     * If backward compatibility with RTCS_selectset() and RTCS_selectall() is on,
     * these non-BSD implementation, write activity was not checked here in poll function,
     * so only _SOCK_select_signal() was used.
     * particularly because of listening stream socket. as after socket(), the socket is writeable
     * tranmsit buffer is empty, this code would return writeable activity to listening socket
     * so in the call sequnce: socket()/listen(), RTCS_selectall or RTCS_selectset, accept(), the RTCS_select
     * would return with the listening socket handle.
     * and subsequent accept() would block or fail
     */
    if(NULL != p_parms->writefds)
    {
      uint32_t sockcnt = 0;      
      sockcnt = sock_fds_poll(p_parms->writefds, p_parms->sock_count, CHECK_TX_AND_SHUTDOWN /* for write */, &error);      
      if(sockcnt)
      {
        p_parms->sock += sockcnt; /* num of sockets with activity return to upper layer */
        poll_act_mask |= SOCK_ACTIVITY_ON_WRITE_FDS; /* save info that write activity was detected */     
      }      
    }
//#endif
    
    if(NULL != p_parms->exceptfds)
    {
      uint32_t sockcnt = 0;
      sockcnt = sock_fds_poll(p_parms->exceptfds, p_parms->sock_count, CHECK_EXCEPT_AND_SHUTDOWN, &error);
      if(sockcnt)
      {
        p_parms->sock += sockcnt; /* num of sockets with activity return to upper layer */
        poll_act_mask |= SOCK_ACTIVITY_ON_EXCEPT_FDS;
      }
    }
    
    /* if there was activity detected in either array, clear fd_sets where no activity was detected.
     * because select() will return and so we need to prepare return fd_set arrays for the upper layer.
     */
    if(poll_act_mask != SOCK_ACTIVITY_NONE)
    {
      if((NULL != p_parms->writefds)&&((SOCK_ACTIVITY_ON_WRITE_FDS & poll_act_mask) == 0))
      {
        RTCS_FD_ZERO(p_parms->writefds);
      }
      if((NULL != p_parms->readfds)&&((SOCK_ACTIVITY_ON_READ_FDS & poll_act_mask) == 0))
      { 
        RTCS_FD_ZERO(p_parms->readfds);
      }
      if((NULL != p_parms->exceptfds)&&((SOCK_ACTIVITY_ON_EXCEPT_FDS & poll_act_mask) == 0))
      { 
        RTCS_FD_ZERO(p_parms->exceptfds);
      }
    }

    /*
     * if NO poll activity has been detected
     * if NOT in poll only mode
     */
    if((0 == p_parms->sock)&&(p_parms->timeout != 0xFFFFFFFF))
    {
      /* put this select() to select queue. */
      /* unblock via _SOCK_select_signal() or via SOCK_select_expire() */
      SOCK_select_block(p_parms);
      return;
    }
    
    RTCSCMD_complete(p_parms, error);
}
/* EOF */
