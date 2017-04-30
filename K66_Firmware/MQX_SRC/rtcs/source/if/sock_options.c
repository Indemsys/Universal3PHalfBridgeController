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
*   This file contains the implementation of getsockopt() and setsockopt()
*
*
*END************************************************************************/

#include <rtcs.h>
#include <socket.h>
#include <rtcs_prv.h>
#include <udp.h>
#include "tcpip.h"
#include <stdint.h>

#define RTCS_ENTER(f,a) RTCSLOG_API(RTCSLOG_TYPE_FNENTRY, RTCSLOG_FN_SOCKET_ ## f, a)

#define RTCS_EXIT(f,a)  RTCSLOG_API(RTCSLOG_TYPE_FNEXIT,  RTCSLOG_FN_SOCKET_ ## f, a); \
                        return a

static bool sockopt_check_level(uint32_t level, SOCKET_STRUCT_PTR socket_ptr);
static bool sockopt_check_option(const void * option_value, const socklen_t *restrict option_len);
static uint32_t sockopt_common_prologue(uint32_t socket, uint32_t * level, uint32_t option_name,
                     const void *restrict option_value, const socklen_t *restrict option_len);

static uint32_t setsockopt_cmd_issue(uint32_t socket, uint32_t level, uint32_t option_name,
                     const void * option_value, socklen_t option_len);
static void setsockopt_internal(RTCS_SETSOCKOPT_PARM_PTR parms);

static uint32_t getsockopt_cmd_issue(uint32_t socket, uint32_t level, uint32_t option_name,
                     void *restrict option_value, socklen_t *restrict option_len);
static void getsockopt_internal(RTCS_GETSOCKOPT_PARM_PTR);
static int32_t sock_data_avail_internal(SOCKET_STRUCT_PTR sock_ptr);

/**
 * @brief Set the socket options.
 *
 * @param[in] socket
 * @param[in] level SOL_SOCKET, SOL_TCP, SOL_UDP, SOL_IGMP, SOL_IP, SOL_LINK, SOL_IP6
 * @param[in] option_name Specifies a single option to set.
 * @param[in] option_value
 * @param[in] option_len
 * @return RTCS_OK upon successful completion.
 * @return specific error code upon failure.
 * @see getsockopt
 */
int32_t setsockopt(uint32_t socket, uint32_t level, uint32_t option_name,
                     const void * option_value, socklen_t option_len)
{
  uint32_t error;

  RTCS_ENTER(SETSOCKOPT, sock);

  error = sockopt_common_prologue(socket, &level, option_name, option_value, &option_len);
  if(error)
  {
    goto exit;
  }

  switch(level)
  {
    case SOL_SOCKET:
    case SOL_UDP:
    case SOL_TCP:
      error = setsockopt_cmd_issue(socket, level, option_name, option_value, option_len);
    break;

    case SOL_IP:
      error = setsockopt_legacy(socket, ((uint32_t)&SOL_IP_CALL), option_name, option_value, option_len);
    break;
#if RTCSCFG_ENABLE_IGMP && RTCSCFG_ENABLE_IP4
    case SOL_IGMP:
      error = setsockopt_legacy(socket, ((uint32_t)&SOL_IGMP_CALL), option_name, option_value, option_len);
    break;
#endif
    case SOL_LINK:
      error = setsockopt_legacy(socket, ((uint32_t)&SOL_LINK_CALL), option_name, option_value, option_len);
    break;

#if RTCSCFG_ENABLE_IP6
    case SOL_IP6:
      error = setsockopt_legacy(socket, ((uint32_t)&SOL_IP6_CALL), option_name, option_value, option_len);
    break;
#endif

    default:
      error = (uint32_t)RTCS_ERROR;
    break;
  }

exit:
  RTCS_EXIT(SETSOCKOPT, error);
}

/**
 * @brief Get the socket options.
 *
 * @param[in] socket
 * @param[in] level SOL_SOCKET, SOL_TCP, SOL_UDP, SOL_IGMP, SOL_IP, SOL_LINK, SOL_IP6
 * @param[in] option_name Specifies a single option to get.
 * @param[in] option_value
 * @param[in] option_len
 * @return RTCS_OK upon successful completion.
 * @return specific error code upon failure.
 * @see setsockopt
 */
int32_t getsockopt(uint32_t socket, uint32_t level, uint32_t option_name,
                     void *restrict option_value, socklen_t *restrict option_len)
{
  uint32_t error;

  RTCS_ENTER(GETSOCKOPT, sock);

  error = sockopt_common_prologue(socket, &level, option_name, option_value, option_len);
  if(error)
  {
    goto exit;
  }

  switch(level)
  {
    case SOL_SOCKET:
    case SOL_UDP:
    case SOL_TCP:
      error = getsockopt_cmd_issue(socket, level, option_name, option_value, option_len);
    break;

    case SOL_IP:
      error = getsockopt_legacy(socket, ((uint32_t)&SOL_IP_CALL), option_name, option_value, option_len);
    break;

#if RTCSCFG_ENABLE_IGMP && RTCSCFG_ENABLE_IP4
    case SOL_IGMP:
      error = getsockopt_legacy(socket, ((uint32_t)&SOL_IGMP_CALL), option_name, option_value, option_len);
    break;
#endif

    case SOL_LINK:
      error = getsockopt_legacy(socket, ((uint32_t)&SOL_LINK_CALL), option_name, option_value, option_len);
    break;

#if RTCSCFG_ENABLE_IP6
    case SOL_IP6:
      error = getsockopt_legacy(socket, ((uint32_t)&SOL_IP6_CALL), option_name, option_value, option_len);
    break;
#endif

    default:
      error = (uint32_t)RTCS_ERROR;
    break;
  }
exit:
  RTCS_EXIT(GETSOCKOPT, error);
}

/**
 * @brief Check the @p level argument.
 *
 * For the given protocol level, this function will check if the appropriate protocol exists in the socket structure.
 * @param[in] level
 * @param[in] socket_ptr
 * @return TRUE if option can be get/set.
 * @return FALSE if protocol (specified by the level) does not exist in the socket structure.
 */
static bool sockopt_check_level(uint32_t level, SOCKET_STRUCT_PTR socket_ptr)
{
  bool b_level_ok = TRUE;

  /* check that for a SOL_UDP option the socket is datagram socket */
  if((SOL_UDP == level) && ((uint32_t)socket_ptr->PROTOCOL != SOCK_DGRAM))
  {
    b_level_ok = FALSE;
    goto exit;
  }

  /* check that for a SOL_TCP option the socket is stream socket. */
  if((SOL_TCP == level) && ((uint32_t)socket_ptr->PROTOCOL != SOCK_STREAM))
  {
    b_level_ok = FALSE;
  }
exit:
  return b_level_ok;
}

/**
 * @brief Check the option_value and option_len arguments.
 *
 * Basic checks for NULL pointers and zero option length.
 * @param[in] option_value
 * @param[in] option_len
 * @return TRUE if option_value and option_len are ok.
 * @return FALSE if option_value/option_len pointers are NULL or option_len points to a zero length.
 */
static bool sockopt_check_option(const void * option_value, const socklen_t *restrict option_len)
{
  bool b_option_ok = TRUE;

  if((NULL == option_value) || (NULL == option_len) || (!*option_len))
  {
    b_option_ok = FALSE;
  }

  return b_option_ok;
}

/**
 * @brief Issue set option command to TCP/IP task.
 *
 * @param[in] socket
 * @param[in] level
 * @param[in] option_name
 * @param[in] option_value
 * @param[in] option_len
 * @return RTCS_OK if successful.
 * @return specific error code on failure.
 */
static uint32_t setsockopt_cmd_issue(uint32_t socket, uint32_t level, uint32_t option_name,
                     const void * option_value, socklen_t option_len)
{
  RTCS_SETSOCKOPT_PARM parms = {0};
  socklen_t option_len_required = 0;

  /* SO_LINGER must have size of struct linger */
  if((SOL_SOCKET == level) && (SO_LINGER == option_name))
  {
    option_len_required = sizeof(struct linger);
  }
  else
  {
    /* for UDP/TCP, option shall have 32-bits */
    option_len_required = sizeof(uint32_t);
  }

  if(option_len < option_len_required)
  {
    return RTCSERR_SOCK_SHORT_OPTION;
  }

  parms.socket = socket;
  parms.level = level;
  parms.option_name = option_name;
  parms.option_value = option_value;
  parms.option_len = option_len;

  return RTCSCMD_issue(parms, setsockopt_internal);
}

/**
 * @brief Issue get option command to TCP/IP task.
 *
 * @param[in] socket
 * @param[in] level
 * @param[in] option_name
 * @param[in] option_value
 * @param[in] option_len
 * @return RTCS_OK if successful.
 * @return specific error code on failure.
 */
static uint32_t getsockopt_cmd_issue(uint32_t socket, uint32_t level, uint32_t option_name,
                     void *restrict option_value, socklen_t *restrict option_len)
{
  RTCS_GETSOCKOPT_PARM parms = {0};
  socklen_t option_len_required = 0;

  /* SO_LINGER must have size of struct linger */
  if((SOL_SOCKET == level) && (SO_LINGER == option_name))
  {
    option_len_required = sizeof(struct linger);
  }
  else
  {
    /* for UDP/TCP, option shall have 32-bits */
    option_len_required = sizeof(uint32_t);
  }

  if(*option_len < option_len_required)
  {
    return RTCSERR_SOCK_SHORT_OPTION;
  }

  parms.socket = socket;
  parms.level = level;
  parms.option_name = option_name;
  parms.option_value = option_value;
  parms.option_len = option_len;

  return RTCSCMD_issue(parms, getsockopt_internal);
}

/**
 * @brief Error check prologue for getsockopt()/setsockopt().
 *
 * @param[in] socket
 * @param[in] level pointer to SOL_SOCKET, SOL_TCP, SOL_UDP, SOL_IGMP, SOL_IP, SOL_LINK, SOL_IP6.
 *                  given as pointer.
 * @param[in] option_name Specifies a single option to error checked.
 * @param[in] option_value
 * @param[in] option_len
 * @return RTCS_OK upon successful completion.
 * @return specific error code upon failure.
 * @see getsockopt
 * @see setsockopt
 */
static uint32_t sockopt_common_prologue(uint32_t socket, uint32_t * level, uint32_t option_name,
                     const void *restrict option_value, const socklen_t *restrict option_len)
{
  SOCKET_STRUCT_PTR socket_ptr = (SOCKET_STRUCT_PTR)socket;
  RTCS_ASSERT(level);

  /* check input parameters in user API */
  if(!SOCK_check_valid(socket))
  {
    return RTCSERR_SOCK_INVALID;
  }

  if(!sockopt_check_option(option_value, option_len))
  {
    return RTCSERR_INVALID_PARAMETER;
  }

  /* Note: if !RTCSCFG_ENABLE_UDP, socket_ptr->PROTOCOL is not SOCK_DGRAM. */
  /* Note: if !RTCSCFG_ENABLE_TCP, socket_ptr->PROTOCOL is not SOCK_STREAM. */

  /* check that for a SOL_UDP option there exists UCB (is datagram socket) */
  /* check that for a SOL_TCP option there exists TCB (is stream socket) */
  if(!sockopt_check_level(*level, socket_ptr))
  {
    return RTCSERR_SOCK_INVALID_OPTION;
  }

  return RTCS_OK;
}

static uint32_t socket_set_opt(struct socket_struct *socket_ptr, uint32_t option_name, const void *option_value)
{
  uint32_t error = RTCS_OK;

  switch(option_name)
  {
    case SO_LINGER:
      socket_ptr->so_linger.l_onoff = ((const struct linger *)option_value)->l_onoff;
      socket_ptr->so_linger.l_linger_ms = ((const struct linger *)option_value)->l_linger_ms;
      break;

    case SO_KEEPALIVE:
      socket_ptr->KEEPALIVE = *(const int32_t *)option_value;
      break;

    case SO_SNDTIMEO:
      socket_ptr->SEND_TIMEOUT = *(const uint32_t *)option_value;
      break;

    case SO_RCVTIMEO:
      socket_ptr->RECEIVE_TIMEOUT = *(const uint32_t *)option_value;
      break;

    case SO_ERROR: /* = OPT_SOCKET_ERROR */
    case OPT_SOCKET_TYPE:
    case SO_RCVNUM:
      error = RTCSERR_SOCK_OPTION_IS_READ_ONLY;
      break;

    case SO_EXCEPTION:
      /* if there is any select() blocked on this socket in exceptfds array, unblock it.
       * set exception along with pending exception flag.
       */
      socket_ptr->exceptsignal = *(const int32_t*)option_value;
      socket_ptr->exceptpending = TRUE;
      _SOCK_select_signal((uint32_t)socket_ptr, RTCS_OK, RTCS_SIGNAL_EXCEPT, RTCS_OK);
      break;

    default:
      error = RTCSERR_SOCK_INVALID_OPTION;
      break;
  }

  return error;
}

static uint32_t tcp_set_opt(struct socket_struct *socket_ptr, uint32_t option_name, uint32_t option_value)
{
  uint32_t error;

  if(socket_ptr->TCB_PTR)
  {
    struct tcb_struct *tcb = (struct tcb_struct*)socket_ptr->TCB_PTR;
    error = RTCS_OK;

    switch(option_name)
    {
      case TCP_KEEPIDLE:
        /* TCP_KEEPIDLE given in seconds */
        tcb->keepidle_ms = option_value<INT32_MAX/1000?1000*option_value:INT32_MAX;
        break;

      case TCP_KEEPINTVL:
        /* TCP_KEEPINTVL given in seconds */
        tcb->keepintvl_ms = option_value<INT32_MAX/1000?1000*option_value:INT32_MAX;
        break;

      case TCP_KEEPCNT:
        tcb->keepcnt = option_value;
        break;

      case OPT_DELAY_ACK:
        socket_ptr->DELAY_ACK = option_value;
        break;

      case OPT_TCPSECUREDRAFT_0:
        socket_ptr->TCPSECUREDRAFT_0 = option_value;
        break;

      case OPT_TIMEWAIT_TIMEOUT:
        socket_ptr->TIMEWAIT_TIMEOUT = option_value;
        break;

      case OPT_NOSWRBUF:
        socket_ptr->NOSWRBUF = option_value;
        break;

      case OPT_NO_NAGLE_ALGORITHM:
        socket_ptr->NO_NAGLE_ALGORITHM = option_value;
        break;

      case OPT_NOWAIT:
        socket_ptr->NOWAIT = option_value;
        break;

      case OPT_KEEPALIVE:
        /* backward compatible socket option.
         * all of this can be achieved by using standard BSD socket options:
         * SO_KEEPALIVE, TCP_KEEPIDLE, TCP_KEEPINTVL and TCP_KEEPCNT
         */
        socket_ptr->KEEPALIVE = option_value; /* enable */
        tcb->keepidle_ms = option_value<INT32_MAX/60000?60000*option_value:INT32_MAX; /* option_value given in minutes */
        tcb->keepintvl_ms = tcb->sndto_2 + tcb->sndto_1;
        if (0 == tcb->keepintvl_ms)
        {
            tcb->keepintvl_ms = DEFAULT_SEND_TIMEOUT;
        }
        tcb->keepcnt = 2;
        break;

      case OPT_MAXRCV_WND:
        socket_ptr->MAXRCV_WND = option_value;
        break;

      case OPT_MAXRTO:
        socket_ptr->MAXRTO = option_value;
        break;

      case OPT_RBSIZE:
        socket_ptr->RBSIZE = option_value;
        break;

      case OPT_TBSIZE:
        socket_ptr->TBSIZE = option_value;
        break;

      case OPT_SEND_PUSH:
        socket_ptr->SEND_PUSH = option_value;
        break;

      case OPT_RECEIVE_PUSH:
        socket_ptr->RECEIVE_PUSH = option_value;
        break;

      case OPT_RECEIVE_TIMEOUT:
        socket_ptr->RECEIVE_TIMEOUT = option_value;
        break;

      case OPT_SEND_TIMEOUT:
        socket_ptr->SEND_TIMEOUT = option_value;
        break;

      case OPT_RETRANSMISSION_TIMEOUT:
        socket_ptr->RETRANSMISSION_TIMEOUT = option_value;
        break;

      case OPT_CONNECT_TIMEOUT:
        socket_ptr->CONNECT_TIMEOUT = option_value;
        break;

      case OPT_SEND_NOWAIT:
        socket_ptr->SEND_NOWAIT = option_value;
        break;

      case OPT_RECEIVE_NOWAIT:
        socket_ptr->RECEIVE_NOWAIT = option_value;
        break;

      default:
        error = RTCS_SOCKET_ERROR;
        break;
    }
  }
  else if(((uint32_t)socket_ptr->PROTOCOL) == SOCK_DGRAM)
  {
    error = RTCSERR_SOCK_NOT_SUPPORTED;
  }
  else
  {
    error = RTCSERR_TCP_CONN_RLSD;
  }
  return error;
}

static uint32_t udp_set_opt(struct socket_struct *socket_ptr, uint32_t option_name, uint32_t option_value)
{
  uint32_t error;

  if(socket_ptr->UCB_PTR)
  {
    struct ucb_struct *ucb = (struct ucb_struct*)socket_ptr->UCB_PTR;
    error = RTCS_OK;

    switch(option_name)
    {
      case OPT_SEND_NOWAIT:
        socket_ptr->SEND_NOWAIT = option_value;
        break;

      case OPT_RECEIVE_NOWAIT:
        socket_ptr->RECEIVE_NOWAIT = option_value;
        break;

      case OPT_CHECKSUM_BYPASS:
        ucb->BYPASS_TX = (option_value>0);
        break;

      case OPT_RBSIZE:
        socket_ptr->RBSIZE = option_value;
        break;

      default:
        error = RTCS_SOCKET_ERROR;
        break;
    }
  }
  else if(((uint32_t)socket_ptr->PROTOCOL) == SOCK_STREAM)
  {
    error = RTCSERR_SOCK_NOT_SUPPORTED;
  }
  else
  {
    error = RTCSERR_UDP_UCB_CLOSE;
  }
  return error;
}




static uint32_t socket_get_opt(struct socket_struct *socket_ptr, uint32_t option_name, void *option_value, socklen_t *option_len)
{
  uint32_t error;

  error = RTCS_OK;
  *option_len = 0;

  switch(option_name)
  {
    case SO_LINGER:
      ((struct linger *)option_value)->l_onoff = socket_ptr->so_linger.l_onoff;
      ((struct linger *)option_value)->l_linger_ms = socket_ptr->so_linger.l_linger_ms;
      *option_len = sizeof(struct linger);
      break;

    case SO_KEEPALIVE:
      *(int32_t *)option_value = socket_ptr->KEEPALIVE;
      break;

    case SO_SNDTIMEO:
      *(uint32_t *)option_value = socket_ptr->SEND_TIMEOUT;
      break;

    case SO_RCVTIMEO:
      *(uint32_t *)option_value = socket_ptr->RECEIVE_TIMEOUT;;
      break;

    case SO_ERROR: /* = OPT_SOCKET_ERROR */
      *(uint32_t *)option_value = socket_ptr->ERROR_CODE;
      socket_ptr->ERROR_CODE = 0;
      break;

    case OPT_SOCKET_TYPE:
      *(uint32_t *)option_value = *(uint32_t *)&socket_ptr->PROTOCOL;
      break;

    case SO_RCVNUM:
      {
        int32_t data_avail;

        data_avail = sock_data_avail_internal(socket_ptr);
        if(data_avail < 0)
        {
          error = RTCS_SOCKET_ERROR;
          break;
        }

        *(uint32_t *)option_value = data_avail;
      }
      break;

    case SO_EXCEPTION:
      *(uint32_t *)option_value = socket_ptr->exceptsignal;
      socket_ptr->exceptpending = 0;
      break;

    default:
      error = RTCSERR_SOCK_INVALID_OPTION;
      break;
  }

  if((*option_len == 0) && (RTCS_OK == error))
  {
    *option_len = sizeof(uint32_t);
  }

  return error;
}

static uint32_t tcp_get_opt(struct socket_struct *socket_ptr, uint32_t option_name, uint32_t * option_value, socklen_t * option_len)
{
  uint32_t error;

  *option_len = 0;

  if(socket_ptr->TCB_PTR)
  {
    struct tcb_struct *tcb = (struct tcb_struct*)socket_ptr->TCB_PTR;
    error = RTCS_OK;

    switch(option_name)
    {
      case TCP_KEEPIDLE:
        /* TCP_KEEPIDLE given in seconds */
        *option_value = tcb->keepidle_ms/1000;
        break;

      case TCP_KEEPINTVL:
        /* TCP_KEEPINTVL given in seconds */
        *option_value = tcb->keepintvl_ms/1000;
        break;

      case TCP_KEEPCNT:
        *option_value = tcb->keepcnt;
        break;

      case OPT_DELAY_ACK:
        *option_value = socket_ptr->DELAY_ACK;
        break;

      case OPT_TCPSECUREDRAFT_0:
        *option_value = socket_ptr->TCPSECUREDRAFT_0;
        break;

      case OPT_TIMEWAIT_TIMEOUT:
        *option_value = socket_ptr->TIMEWAIT_TIMEOUT;
        break;

      case OPT_NOSWRBUF:
        *option_value = socket_ptr->NOSWRBUF;
        break;

      case OPT_NO_NAGLE_ALGORITHM:
        *option_value = socket_ptr->NO_NAGLE_ALGORITHM;
        break;

      case OPT_NOWAIT:
        *option_value = socket_ptr->NOWAIT;
        break;

      case OPT_KEEPALIVE:
        *option_value = socket_ptr->KEEPALIVE;
        /* configure KEEPALIVE, TCP_KEEPCNT, TCP_KEEPIDLE, TCP_KEEPINTVL */
        /* read TCP_KEEPIDLE if SO_KEEPALIVE is on */
        break;

      case OPT_MAXRCV_WND:
        *option_value = socket_ptr->MAXRCV_WND;
        break;

      case OPT_MAXRTO:
        *option_value = socket_ptr->MAXRTO;
        break;

      case OPT_RBSIZE:
        *option_value = socket_ptr->RBSIZE;
        break;

      case OPT_TBSIZE:
        *option_value = socket_ptr->TBSIZE;
        break;

      case OPT_SEND_PUSH:
        *option_value = socket_ptr->SEND_PUSH;
        break;

      case OPT_RECEIVE_PUSH:
        *option_value = socket_ptr->RECEIVE_PUSH;
        break;

      case OPT_RECEIVE_TIMEOUT:
        *option_value = socket_ptr->RECEIVE_TIMEOUT;
        break;

      case OPT_SEND_TIMEOUT:
        *option_value = socket_ptr->SEND_TIMEOUT;
        break;

      case OPT_RETRANSMISSION_TIMEOUT:
        *option_value = socket_ptr->RETRANSMISSION_TIMEOUT;
        break;

      case OPT_CONNECT_TIMEOUT:
        *option_value = socket_ptr->CONNECT_TIMEOUT;
        break;

      case OPT_SEND_NOWAIT:
        *option_value = socket_ptr->SEND_NOWAIT;
        break;

      case OPT_RECEIVE_NOWAIT:
        *option_value = socket_ptr->RECEIVE_NOWAIT;
        break;

      default:
        error = RTCS_SOCKET_ERROR;
        break;
    }
  }
  else if(((uint32_t)socket_ptr->PROTOCOL) == SOCK_DGRAM)
  {
    error = RTCSERR_SOCK_NOT_SUPPORTED;
  }
  else
  {
    error = RTCSERR_TCP_CONN_RLSD;
  }

  if((*option_len == 0) && (RTCS_OK == error))
  {
    *option_len = sizeof(uint32_t);
  }

  return error;
}

static uint32_t udp_get_opt(struct socket_struct *socket_ptr, uint32_t option_name, uint32_t *option_value, socklen_t *option_len)
{
  uint32_t error;

  error = RTCS_OK;
  *option_len = 0;

  if(socket_ptr->UCB_PTR)
  {
    struct ucb_struct *ucb = (struct ucb_struct*)socket_ptr->UCB_PTR;
    error = RTCS_OK;

    switch(option_name)
    {
      case OPT_SEND_NOWAIT:
        *option_value = socket_ptr->SEND_NOWAIT;
        break;

      case OPT_RECEIVE_NOWAIT:
        *option_value = socket_ptr->RECEIVE_NOWAIT;
        break;

      case OPT_CHECKSUM_BYPASS:
        *option_value = ucb->BYPASS_TX;
        break;

      case OPT_RBSIZE:
        *option_value = socket_ptr->RBSIZE;
        break;

      default:
        error = RTCS_SOCKET_ERROR;
        break;
    }
  }
  else if(((uint32_t)socket_ptr->PROTOCOL) == SOCK_STREAM)
  {
    error = RTCSERR_SOCK_NOT_SUPPORTED;
  }
  else
  {
    error = RTCSERR_UDP_UCB_CLOSE;
  }

  if((*option_len == 0) && (RTCS_OK == error))
  {
    *option_len = sizeof(uint32_t);
  }

  return error;
}

/**
 * @brief Set the socket options for SOL_SOCKET, SOL_TCP and SOL_UDP levels.
 *
 * Called by TCP/IP task. Unblocks the caller task when command is serviced.
 * @param[in] parms
 * @return none
 */
static void setsockopt_internal(RTCS_SETSOCKOPT_PARM_PTR parms)
{
  uint32_t sock;
  uint32_t error = RTCS_SOCKET_ERROR;
  SOCKET_STRUCT_PTR socket_ptr;

  RTCS_ASSERT(parms);
  sock = parms->socket;

  RTCS_ASSERT(sock);
  RTCS_ASSERT(sock != RTCS_SOCKET_ERROR);
  socket_ptr = (SOCKET_STRUCT_PTR)sock;

  if(FALSE == SOCK_exists_in_system(sock))
  {
    error = RTCSERR_SOCK_INVALID;
    goto EXIT;
  }

  switch(parms->level)
  {
    case SOL_SOCKET:
      error = socket_set_opt(socket_ptr, parms->option_name, parms->option_value);
      break;

    case SOL_TCP:
      error = tcp_set_opt(socket_ptr, parms->option_name, *(const uint32_t *)parms->option_value);
      break;

    case SOL_UDP:
      error = udp_set_opt(socket_ptr, parms->option_name, *(const uint32_t *)parms->option_value);
      break;

    default:
      error = RTCSERR_SOCK_INVALID_OPTION;
      break;
  }

  EXIT:
  RTCSCMD_complete(parms, error);
}

/**
 * @brief Get the socket options for SOL_SOCKET, SOL_TCP and SOL_UDP levels.
 *
 * Called by TCP/IP task. Unblocks the caller task when command is serviced.
 * @param[in] parms
 * @return none
 */
static void getsockopt_internal(RTCS_GETSOCKOPT_PARM_PTR parms)
{
  uint32_t sock;
  SOCKET_STRUCT_PTR socket_ptr;
  uint32_t error = RTCS_OK;

  RTCS_ASSERT(parms);
  sock = parms->socket;

  RTCS_ASSERT(sock);
  RTCS_ASSERT(sock != RTCS_SOCKET_ERROR);
  socket_ptr = (SOCKET_STRUCT_PTR)sock;

  if(FALSE == SOCK_exists_in_system(sock))
  {
    error = RTCSERR_SOCK_INVALID;
    goto EXIT;
  }

  RTCS_ASSERT(parms->option_len);

  switch(parms->level)
  {
    case SOL_SOCKET:
      error = socket_get_opt(socket_ptr, parms->option_name, parms->option_value, parms->option_len);
      break;

    case SOL_TCP:
      error = tcp_get_opt(socket_ptr, parms->option_name, (uint32_t *)parms->option_value, parms->option_len);
      break;

    case SOL_UDP:
      error = udp_get_opt(socket_ptr, parms->option_name, (uint32_t *)parms->option_value, parms->option_len);
      break;

    default:
      error = RTCSERR_SOCK_INVALID_OPTION;
      break;
  }

  EXIT:
  RTCSCMD_complete(parms, error);
}

/**
 * @brief Compute data received by a socket but not yet given to upper layer.
 *
 * Called by TCP/IP task.
 * @return -1 in case of error
 * @return size of data available in bytes
 */
static int32_t sock_data_avail_internal(SOCKET_STRUCT_PTR sock_ptr)
{
  int32_t n = -1;

  if(sock_ptr->TCB_PTR)
  {
    TCB_STRUCT_PTR tcb = sock_ptr->TCB_PTR;

    /* === copied code from tcp_rcv.c, function TCP_Setup_receive  */
    n = tcb->rcvnxt - tcb->rcvbufseq;   /* Seq #'s acked but not yet given to upper layer */
    /* END: === copied code from tcp_rcv.c, function TCP_Setup_receive  */
  }
  else if(sock_ptr->UCB_PTR)
  {
    /* UDP datagrams received but not yet given to upper layer */
    struct udp_rx_dgram_header * dgram_item;

    /* loop through all UDP datagrams enqueued at UDP layer and sum their data payload */
    dgram_item = sock_ptr->UCB_PTR->PHEAD;
    n = 0;
    while(1)
    {
      if(dgram_item)
      {
        n += dgram_item->size;
        dgram_item = dgram_item->NEXT;
      }
      else
      {
        break;
      }
    }
  }

  return n;
}
