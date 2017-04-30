#ifndef __socket_h__
#define __socket_h__
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
*   Constants and prototype declarations which are internal
*   to the SOCKET module.
*
*
*END************************************************************************/
/** @file */
#include "rtcstime.h"

/*
** Connectionless-mode socket states
*/
#define SOCKSTATE_DGRAM_GROUND      0
#define SOCKSTATE_DGRAM_BOUND       1
#define SOCKSTATE_DGRAM_OPEN        2

/*
** Connection-mode socket states
*/
#define SOCKSTATE_STREAM_GROUND     0
#define SOCKSTATE_STREAM_BOUND      1
#define SOCKSTATE_STREAM_LISTENING  2
#define SOCKSTATE_STREAM_CONNECTED  3


#define RTCS_SIGNAL_READ      0
#define RTCS_SIGNAL_WRITE     1
#define RTCS_SIGNAL_EXCEPT    2


/*
**  socket-specific structures
*/
/* Select functions */
typedef struct sock_select_parm {
   TCPIP_PARM                          COMMON;
   struct sock_select_parm             *NEXT;
   struct sock_select_parm            **PREV;
   TCPIP_EVENT                         EXPIRE;
   _rtcs_taskid                        owner;
   rtcs_fd_set *restrict                    readfds;
   rtcs_fd_set *restrict                    writefds;
   rtcs_fd_set *restrict                    exceptfds;
   uint32_t                            sock_count;
   uint32_t                            timeout;
   uint32_t                            sock;
} SOCK_SELECT_PARM, * SOCK_SELECT_PARM_PTR;

#ifdef __cplusplus
extern "C" {
#endif

#define SOCK_select_signal(a, b, err)    _SOCK_select_signal(a, b, RTCS_SIGNAL_READ, err)

extern void    _SOCK_select_signal   (uint32_t, uint32_t, uint32_t, uint32_t);

typedef struct socket_config_struct {

   bool                       INITIALIZED;
   uint32_t                       CURRENT_SOCKETS;
   void                         *SOCKET_HEAD;
   void                         *SOCKET_TAIL;
   struct sock_select_parm       *SELECT_HEAD;
   void                         *SELECT_TIME_HEAD;    /* not used */
   _rtcs_mutex                   SOCK_MUTEX;

} SOCKET_CONFIG_STRUCT, * SOCKET_CONFIG_STRUCT_PTR;


typedef struct rtcs_setsockopt_parm
{
  TCPIP_PARM COMMON;
  uint32_t socket;
  uint32_t level;
  uint32_t option_name;
  const void * option_value;
  socklen_t option_len;
} RTCS_SETSOCKOPT_PARM, * RTCS_SETSOCKOPT_PARM_PTR;

typedef struct rtcs_getsockopt_parm
{
  TCPIP_PARM COMMON;
  uint32_t socket;
  uint32_t level;
  uint32_t option_name;
  void *restrict option_value;
  socklen_t *restrict option_len;
} RTCS_GETSOCKOPT_PARM, * RTCS_GETSOCKOPT_PARM_PTR;

typedef struct rtcs_ioctlsocket_parm
{
  TCPIP_PARM COMMON;
  uint32_t sock;
  int32_t  cmd;
  uint32_t *argp;
} rtcs_ioctlsocket_parm;


/*------------------------------------------------------
** extern statements for socket procedures          *
*****************************************************/

extern SOCKET_STRUCT_PTR   SOCK_Get_sock_struct
(
   RTCS_SOCKET_CALL_STRUCT_PTR   type,
   _rtcs_taskid                  owner
);
extern void SOCK_Free_sock_struct
(
   SOCKET_STRUCT_PTR          socket_ptr
);

#if RTCSCFG_BACKWARD_COMPATIBILITY_RTCSSELECT==1
extern bool SOCK_Remove_owner( SOCKET_STRUCT_PTR, void *);
extern bool SOCK_Is_owner( SOCKET_STRUCT_PTR, void *);
extern bool SOCK_Add_owner( SOCKET_STRUCT_PTR, void *);
#endif

bool SOCK_check_valid(const uint32_t);
bool SOCK_disallow_recv(const uint32_t);
bool SOCK_disallow_send(const uint32_t);
uint32_t SOCK_set_disallow_mask(const uint32_t, uint32_t);
bool SOCK_exists_in_system(const uint32_t);
bool TCB_exists_in_system(struct tcb_struct *);
bool UCB_exists_in_system(struct ucb_struct *);
bool SOCK_PROTOCOL_exists_in_system( struct rtcs_socket_call_struct *);

/*------------------------------------------------------
** function to support generic code for IP4/IP6/IP4+IP6 in tcp_udp layer
*****************************************************/
void * SOCKADDR_route_find(const sockaddr * saddr_ptr);
void SOCKADDR_return_addr(const SOCKET_STRUCT_PTR sock_struct_ptr, sockaddr * name, const sockaddr * addrfrom, uint16_t * namelen);
uint32_t SOCKADDR_check_valid(uint32_t sock, const sockaddr * localaddr);
uint32_t SOCKADDR_check_addr(const sockaddr * localaddr, uint16_t addrlen);

/*------------------------------------------------------
** inline function to support generic code for IP4/IP6/IP4+IP6 in tcp_udp layer
*****************************************************/
static inline uint16_t SOCKADDR_get_port(const sockaddr * saddr_ptr);
static inline void SOCKADDR_set_port(sockaddr * saddr_ptr, const uint16_t port);
static inline void SOCKADDR_copy(const sockaddr * from_ptr, sockaddr * to_ptr);
static inline void SOCKADDR_init(const void * from_ptr, const uint16_t port, sockaddr * to_ptr);
static inline void SOCKADDR_init_no_port(const void * from_ptr, sockaddr * to_ptr);
static inline bool SOCKADDR_ip_and_port_are_equal(const sockaddr * saddr1_ptr, const sockaddr * saddr2_ptr);
#if RTCSCFG_ENABLE_IP6
static inline in6_addr * SOCKADDR_get_ipaddr6(const sockaddr * saddr_ptr);
#endif
#if RTCSCFG_ENABLE_IP4
static inline _ip_address SOCKADDR_get_ipaddr4(const sockaddr * saddr_ptr);
#endif
static inline bool SOCKADDR_ip_is_zero(const sockaddr * saddr_ptr);
static inline void SOCKADDR_zero_ip(sockaddr * saddr_ptr);
static inline void SOCKADDR_zero_ip_and_port(sockaddr * saddr_ptr);
static inline bool SOCKADDR_ip_are_equal(const sockaddr * saddr1_ptr, const sockaddr * saddr2_ptr);
static inline bool SOCKADDR_ip_is_multicast(const sockaddr * saddr_ptr);
static inline bool SOCKADDR_ip_is_broadcast(const sockaddr * saddr_ptr);

static inline uint32_t SOCKADDR_get_if_scope_id(const sockaddr * saddr_ptr);
static inline void SOCKADDR_set_if_scope_id(sockaddr * saddr_ptr, const uint32_t if_scope_id);

/**
 * @brief Get port.
 *
 * Read a port number from <tt>struct sockaddr</tt>. Depending on @c sa_family member of this structure, the pointer is type casted
 * to either <tt>sockaddr_in*</tt> or <tt>sockaddr_in6*</tt> and value of port is read from the data structure.
 * @param[in] saddr_ptr pointer to <tt>struct sockaddr</tt>
 * @return port
 * @see SOCKADDR_set_port
 * @warning @c sa_family member of <tt>struct sockaddr</tt> shall be set to one of <tt>AF_INET, AF_INET6</tt>.
 */
static inline uint16_t SOCKADDR_get_port(const sockaddr * saddr_ptr)
{
  uint16_t port = 0;

#if RTCSCFG_ENABLE_IP4
  #if RTCSCFG_ENABLE_IP6
  if(AF_INET == saddr_ptr->sa_family)
  {
  #endif
    port = ((sockaddr_in*)(saddr_ptr))->sin_port;
  #if RTCSCFG_ENABLE_IP6
  }
  #endif
#endif
#if RTCSCFG_ENABLE_IP6
  #if RTCSCFG_ENABLE_IP4
  else if(AF_INET6 == saddr_ptr->sa_family)
  {
  #endif
    port = ((sockaddr_in6*)(saddr_ptr))->sin6_port;
  #if RTCSCFG_ENABLE_IP4
  }
  #endif
#endif
  return port;
}

/**
 * @brief Set port.
 *
 * Write a port number to <tt>struct sockaddr</tt>. Depending on @c sa_family member of this structure, the pointer is type casted
 * to either <tt>sockaddr_in*</tt> or <tt>sockaddr_in6*</tt> and value of port is written to the data structure.
 * @param[in,out] saddr_ptr pointer to <tt>struct sockaddr</tt>
 * @param[in] port port number
 * @return none
 * @see SOCKADDR_get_port
 * @warning @c sa_family member of <tt>struct sockaddr</tt> shall be set to one of <tt>AF_INET, AF_INET6</tt>.
 */
static inline void SOCKADDR_set_port(sockaddr * saddr_ptr, const uint16_t port)
{
#if RTCSCFG_ENABLE_IP4
  #if RTCSCFG_ENABLE_IP6
  if(AF_INET == saddr_ptr->sa_family)
  {
  #endif
    ((sockaddr_in*)(saddr_ptr))->sin_port = port;
  #if RTCSCFG_ENABLE_IP6
  }
  #endif
#endif
#if RTCSCFG_ENABLE_IP6
  #if RTCSCFG_ENABLE_IP4
  else if(AF_INET6 == saddr_ptr->sa_family)
  {
  #endif
    ((sockaddr_in6*)(saddr_ptr))->sin6_port = port;
  #if RTCSCFG_ENABLE_IP4
  }
  #endif
#endif
  return;
}

/**
 * @brief Copy struct sockaddr from -> to.
 *
 * Copy <tt>struct sockaddr</tt> from source to destination. Depending on @c sa_family member of this structure, the pointer is type casted
 * to either <tt>sockaddr_in*</tt> or <tt>sockaddr_in6*</tt> and the content of source data structure is copied onto destination data structure.
 * @param[in] from_ptr pointer to source <tt>struct sockaddr</tt>
 * @param[out] to_ptr pointer to destination <tt>struct sockaddr</tt>
 * @return none
 * @warning @c sa_family member of <tt>struct sockaddr</tt> at @a from_ptr shall be set to one of <tt>AF_INET, AF_INET6</tt>.
 */
static inline void SOCKADDR_copy(const sockaddr * from_ptr, sockaddr * to_ptr)
{
#if RTCSCFG_ENABLE_IP4
  #if RTCSCFG_ENABLE_IP6
  if(AF_INET == from_ptr->sa_family)
  {
  #endif
    memcpy(to_ptr, from_ptr, sizeof(sockaddr_in));
  #if RTCSCFG_ENABLE_IP6
  }
  #endif
#endif
#if RTCSCFG_ENABLE_IP6
  #if RTCSCFG_ENABLE_IP4
  else if(AF_INET6 == from_ptr->sa_family)  
  {
  #endif
    memcpy(to_ptr, from_ptr, sizeof(sockaddr_in6));
  #if RTCSCFG_ENABLE_IP4
  }
  #endif
#endif
}

/**
 * @brief Test struct sockaddr for zero ip address.
 *
 * Test if given <tt>struct sockaddr</tt> ip address equal zero. Depending on @c sa_family member of this structure, the pointer is type casted
 * to either <tt>sockaddr_in*</tt> or <tt>sockaddr_in6*</tt> and the ip4/ip6 address member of this structure is compared with <tt>INADDR_ANY/in6addr_any</tt>.
 * @param[in] saddr_ptr pointer to <tt>struct sockaddr</tt>
 * @return @c TRUE if ip address member of <tt>struct sockaddr</tt> equals INADDR_ANY/in6_addr_any.
 * @return @c FALSE otherwise.
 * @see SOCKADDR_zero_ip
 * @warning @c sa_family member of <tt>struct sockaddr</tt> shall be set to one of <tt>AF_INET, AF_INET6</tt>.
 */
static inline bool SOCKADDR_ip_is_zero(const sockaddr * saddr_ptr)
{
  bool b_is_zero = TRUE;
#if RTCSCFG_ENABLE_IP4
  #if RTCSCFG_ENABLE_IP6
  if(AF_INET == saddr_ptr->sa_family)  
  {
  #endif
    b_is_zero = !(((sockaddr_in*)saddr_ptr)->sin_addr.s_addr);
  #if RTCSCFG_ENABLE_IP6
  }
  #endif
#endif
#if RTCSCFG_ENABLE_IP6
  #if RTCSCFG_ENABLE_IP4
  else if(AF_INET6 == saddr_ptr->sa_family)
  {
  #endif
    b_is_zero = IN6_ARE_ADDR_EQUAL(&(((sockaddr_in6*)saddr_ptr)->sin6_addr), &in6addr_any);
  #if RTCSCFG_ENABLE_IP4
  }
  #endif
#endif
  return b_is_zero;
}

/**
 * @brief Set ip address in struct sockaddr to zero.
 *
 * Set zero to ip address member of given <tt>struct sockaddr</tt>. Depending on @c sa_family member of this structure, the pointer is type casted
 * to either <tt>sockaddr_in*</tt> or <tt>sockaddr_in6*</tt> and the ip4/ip6 address member of this structure is set to <tt>INADDR_ANY/in6addr_any</tt>.
 * Other structure members, such as @c sa_family and <tt>sin_port/sin6_port</tt>, are not affected.
 * @param[in,out] saddr_ptr pointer to <tt>struct sockaddr</tt>
 * @see SOCKADDR_ip_is_zero
 * @return none
 * @warning @c sa_family member of <tt>struct sockaddr</tt> shall be set to one of <tt>AF_INET, AF_INET6</tt>.
 */
static inline void SOCKADDR_zero_ip(sockaddr * saddr_ptr)
{
#if RTCSCFG_ENABLE_IP4
  #if RTCSCFG_ENABLE_IP6
  if(AF_INET == saddr_ptr->sa_family)  
  {
  #endif
    (((sockaddr_in*)saddr_ptr)->sin_addr.s_addr) = 0;
  #if RTCSCFG_ENABLE_IP6
  }
  #endif
#endif
#if RTCSCFG_ENABLE_IP6
  #if RTCSCFG_ENABLE_IP4
  else if(AF_INET6 == saddr_ptr->sa_family)
  {
  #endif
    IN6_ADDR_COPY((in6_addr*)(&(in6addr_any)),&(((sockaddr_in6*)saddr_ptr)->sin6_addr));
  #if RTCSCFG_ENABLE_IP4
  }
  #endif
#endif
}

/**
 * @brief Set ip address and port in struct sockaddr to zero.
 *
 * Set zero to ip address member of given <tt>struct sockaddr</tt>. Depending on @c sa_family member of this structure, the pointer is type casted
 * to either <tt>sockaddr_in*</tt> or <tt>sockaddr_in6*</tt> and the ip4/ip6 address member of this structure is set to <tt>INADDR_ANY/in6addr_any</tt>.
 * Port member of the structure is set to zero. * @c sa_family member of the structure is not affected.
 * @param[in,out] saddr_ptr pointer to <tt>struct sockaddr</tt>
 * @see SOCKADDR_ip_is_zero
 * @see SOCKADDR_zero_ip
 * @return none
 * @warning @c sa_family member of <tt>struct sockaddr</tt> shall be set to one of <tt>AF_INET, AF_INET6</tt>.
 */
static inline void SOCKADDR_zero_ip_and_port(sockaddr * saddr_ptr)
{
#if RTCSCFG_ENABLE_IP4
  #if RTCSCFG_ENABLE_IP6
  if(AF_INET == saddr_ptr->sa_family)  
  {
  #endif
    (((sockaddr_in*)saddr_ptr)->sin_addr.s_addr) = 0;
    (((sockaddr_in*)saddr_ptr)->sin_port) = 0;
  #if RTCSCFG_ENABLE_IP6
  }
  #endif
#endif
#if RTCSCFG_ENABLE_IP6
  #if RTCSCFG_ENABLE_IP4
  else if(AF_INET6 == saddr_ptr->sa_family)
  {
  #endif
    IN6_ADDR_COPY((in6_addr*)(&(in6addr_any)),&(((sockaddr_in6*)saddr_ptr)->sin6_addr));
    (((sockaddr_in6*)saddr_ptr)->sin6_port) = 0;
  #if RTCSCFG_ENABLE_IP4
  }
  #endif
#endif
}

/**
 * @brief Test if two struct sockaddr are equal in ip address.
 *
 * Test if given <tt>struct sockaddr</tt> ip addresses are equal. Depending on @c sa_family member of @p saddr2_ptr, the pointers are type casted
 * to either <tt>sockaddr_in*</tt> or <tt>sockaddr_in6*</tt> and the ip4/ip6 address member of one structure is compared with ip4/ip6 address member of the other structure.
 * This function does not consider <tt>sin_port/sin6_port</tt>, only @c s_addr or @c sin6_addr members are compared.
 * @param[in] saddr1_ptr pointer to <tt>struct sockaddr</tt>
 * @param[in] saddr2_ptr pointer to <tt>struct sockaddr</tt>
 * @return @c TRUE if ip address members of two <tt>struct sockaddr</tt> are equal.
 * @return @c FALSE otherwise.
 * @see SOCKADDR_ip_and_port_are_equal
 * @warning @c sa_family member of <tt>struct sockaddr</tt> shall be set to one of <tt>AF_INET, AF_INET6</tt>.
 */
static inline bool SOCKADDR_ip_are_equal(const sockaddr * saddr1_ptr, const sockaddr * saddr2_ptr)
{
  bool b_equal = FALSE;
#if RTCSCFG_ENABLE_IP4
  #if RTCSCFG_ENABLE_IP6
  if(AF_INET == saddr2_ptr->sa_family)  
  {
  #endif
    b_equal = ((sockaddr_in*)saddr1_ptr)->sin_addr.s_addr == ((sockaddr_in*)saddr2_ptr)->sin_addr.s_addr;
  #if RTCSCFG_ENABLE_IP6
  }
  #endif
#endif
#if RTCSCFG_ENABLE_IP6
  #if RTCSCFG_ENABLE_IP4
  else if(AF_INET6 == saddr2_ptr->sa_family)  
  {
  #endif
    b_equal = IN6_ARE_ADDR_EQUAL(&(((sockaddr_in6*)saddr1_ptr)->sin6_addr), &(((sockaddr_in6*)saddr2_ptr)->sin6_addr));
  #if RTCSCFG_ENABLE_IP4
  }
  #endif
#endif
  return b_equal;
}

/**
 * @brief Test if two struct sockaddr are equal in ip address and port.
 *
 * Test if given <tt>struct sockaddr</tt> ip addresses and ports are equal. Depending on @c sa_family member of @p saddr2_ptr structure, the pointers are type casted
 * to either <tt>sockaddr_in*</tt> or <tt>sockaddr_in6*</tt> and the ip4/ip6 address and port members of one structure are compared with ip4/ip6 address and port members of the other structure.
 * This function does consider <tt>sin_port/sin6_port</tt>.
 * @param[in] saddr1_ptr pointer to <tt>struct sockaddr</tt>
 * @param[in] saddr2_ptr pointer to <tt>struct sockaddr</tt>
 * @return @c TRUE if ip addresses and ports of two <tt>struct sockaddr</tt> are equal.
 * @return @c FALSE otherwise.
 * @see SOCKADDR_ip_are_equal
 * @warning @c sa_family member of <tt>struct sockaddr</tt> shall be set to one of <tt>AF_INET, AF_INET6</tt>.
 */
static inline bool SOCKADDR_ip_and_port_are_equal(const sockaddr * saddr1_ptr, const sockaddr * saddr2_ptr)
{
  bool b_equal = FALSE;
#if RTCSCFG_ENABLE_IP4
  #if RTCSCFG_ENABLE_IP6
  if(AF_INET == saddr2_ptr->sa_family)  
  {
  #endif
    b_equal = ((sockaddr_in*)saddr1_ptr)->sin_addr.s_addr == ((sockaddr_in*)saddr2_ptr)->sin_addr.s_addr;
    b_equal = b_equal && ((sockaddr_in*)saddr1_ptr)->sin_port == ((sockaddr_in*)saddr2_ptr)->sin_port;
  #if RTCSCFG_ENABLE_IP6
  }
  #endif
#endif
#if RTCSCFG_ENABLE_IP6
  #if RTCSCFG_ENABLE_IP4
  else if(AF_INET6 == saddr2_ptr->sa_family)  
  {
  #endif
    b_equal = IN6_ARE_ADDR_EQUAL(&(((sockaddr_in6*)saddr1_ptr)->sin6_addr), &(((sockaddr_in6*)saddr2_ptr)->sin6_addr));
    b_equal = b_equal && ((sockaddr_in6*)saddr1_ptr)->sin6_port == ((sockaddr_in6*)saddr2_ptr)->sin6_port;
  #if RTCSCFG_ENABLE_IP4
  }
  #endif
#endif
  return b_equal;
}

/**
 * @brief Initialize ip address and port.
 *
 * Initialize struct sockaddr with port and ip4/ip6 address. <br>
 * @p port is directly written to @c sin_port or @c sin6_port member of <tt>struct sockaddr</tt> at @p to_ptr. <br>
 * For ip address, @p from_ptr is type cased depending on the @c sa_family of data structure at @p to_ptr:<br>
 * if <tt>sa_family == AF_INET</tt> , @p from_ptr is type casted to @c _ip_address.<br>
 * if <tt>sa_family == AF_INET6</tt> , @p from_ptr is type casted to <tt>(in6_addr*)</tt>.<br>
 * and then @c s_addr or @c sin6_addr ip address is initialized according to the resulting type.
 * @param[in] from_ptr _ip_address for AF_INET structure, in6_addr* for AF_INET6 structure. Init value for ip address member of @p to_ptr.
 * @param[in] port Init value for port member of @p to_ptr.
 * @param[in,out] to_ptr pointer to struct sockaddr to be initialized.
 * @return none
 * @see SOCKADDR_init_no_port
 * @warning @c sa_family member of <tt>struct sockaddr</tt> shall be set to one of <tt>AF_INET, AF_INET6</tt>.
 */
static inline void SOCKADDR_init(const void * from_ptr, const uint16_t port, sockaddr * to_ptr)
{
/*
 * family == AF_INET, from_ptr is _ip_address
 * family == AF_INET6, from_ptr is (in6_addr *)
 */
#if RTCSCFG_ENABLE_IP4
  #if RTCSCFG_ENABLE_IP6
  uint16_t family = to_ptr->sa_family;
  
  if(AF_INET == family) 
  {
  #endif
    sockaddr_in * addr = (sockaddr_in*)to_ptr;
    addr->sin_addr.s_addr = (_ip_address)from_ptr;
    addr->sin_port = port;
  #if RTCSCFG_ENABLE_IP6
  }
  #endif
#endif
#if RTCSCFG_ENABLE_IP6
  #if RTCSCFG_ENABLE_IP4
  else if(AF_INET6 == family)
  {
  #endif
    sockaddr_in6 * addr = (sockaddr_in6*)to_ptr;
    /* from->to */
    IN6_ADDR_COPY((in6_addr*)from_ptr, &addr->sin6_addr);
    addr->sin6_port = port;
  #if RTCSCFG_ENABLE_IP4
  }
  #endif
#endif
  return;
}

/**
 * @brief Initialize ip address.
 *
 * Initialize struct sockaddr with ip4/ip6 address. <br>
 * For ip address, @p from_ptr is type casted depending on the @c sa_family of data structure at @p to_ptr:<br>
 * if <tt>sa_family == AF_INET</tt> , @p from_ptr is type casted to @c _ip_address.<br>
 * if <tt>sa_family == AF_INET6</tt> , @p from_ptr is type casted to <tt>(in6_addr*)</tt>.<br>
 * and then @c s_addr or @c sin6_addr ip address is initialized according to the resulting type.
 * @param[in] from_ptr _ip_address for AF_INET structure, in6_addr* for AF_INET6 structure. Init value for ip address member of @p to_ptr.
 * @param[in,out] to_ptr pointer to struct sockaddr to be initialized.
 * @return none
 * @see SOCKADDR_init
 * @warning @c sa_family member of <tt>struct sockaddr</tt> shall be set to one of <tt>AF_INET, AF_INET6</tt>.
 */
static inline void SOCKADDR_init_no_port(const void * from_ptr, sockaddr * to_ptr)
{
#if RTCSCFG_ENABLE_IP4
  #if RTCSCFG_ENABLE_IP6
  uint16_t family = to_ptr->sa_family;
  
  if(AF_INET == family) 
  {
  #endif
    sockaddr_in * addr = (sockaddr_in*)to_ptr;
    addr->sin_addr.s_addr = (_ip_address)from_ptr;
  #if RTCSCFG_ENABLE_IP6
  }
  #endif
#endif
#if RTCSCFG_ENABLE_IP6
  #if RTCSCFG_ENABLE_IP4
  else if(AF_INET6 == family)
  {
  #endif
    sockaddr_in6 * addr = (sockaddr_in6*)to_ptr;
    /* from->to */
    IN6_ADDR_COPY((in6_addr*)from_ptr, &addr->sin6_addr);
  #if RTCSCFG_ENABLE_IP4
  }
  #endif
#endif
  return;
}

/**
 * @brief Test if struct sockaddr ip address is a multicast address.
 *
 * Test if given <tt>struct sockaddr</tt> ip address is a multicast address. Depending on @c sa_family member of @p saddr_ptr, the pointer is type casted
 * to either <tt>sockaddr_in*</tt> or <tt>sockaddr_in6*</tt> and the ip4/ip6 address member of one structure is checked for being multicast.
 * @param[in] saddr_ptr pointer to <tt>struct sockaddr</tt>
 * @return @c TRUE if ip address member of <tt>struct sockaddr</tt> is a multicast address.
 * @return @c FALSE otherwise.
 * @see SOCKADDR_ip_is_broadcast
 * @warning @c sa_family member of <tt>struct sockaddr</tt> shall be set to one of <tt>AF_INET, AF_INET6</tt>.
 */
static inline bool SOCKADDR_ip_is_multicast(const sockaddr * saddr_ptr)
{
  bool b_is_multicast = FALSE;
#if RTCSCFG_ENABLE_IP4
  #if RTCSCFG_ENABLE_IP6
  if(AF_INET == saddr_ptr->sa_family)
  {
  #endif
    b_is_multicast = IN_MULTICAST(((sockaddr_in*)saddr_ptr)->sin_addr.s_addr);
  #if RTCSCFG_ENABLE_IP6
  }
  #endif
#endif
#if RTCSCFG_ENABLE_IP6
  #if RTCSCFG_ENABLE_IP4
  else if(AF_INET6 == saddr_ptr->sa_family)
  {
  #endif
    b_is_multicast = IN6_IS_ADDR_MULTICAST(&((sockaddr_in6*)saddr_ptr)->sin6_addr);
  #if RTCSCFG_ENABLE_IP4
  }
  #endif
#endif
  return b_is_multicast;
}

/**
 * @brief Test if struct sockaddr ip address is a broadcast address.
 *
 * Test if given <tt>struct sockaddr</tt> ip address is a broadcast address. Depending on @c sa_family member of @p saddr_ptr, the pointer is type casted
 * to either <tt>sockaddr_in*</tt> or <tt>sockaddr_in6*</tt>. AF_INET address is checked for broadcast. For AF_INET6, this function returs @c FALSE as there
 * is no broadcast in IPv6. 
 * @param[in] saddr_ptr pointer to <tt>struct sockaddr</tt>
 * @return @c TRUE if <tt>struct sockaddr</tt> is an @c AF_INET broadcast address.
 * @return @c FALSE otherwise.
 * @see SOCKADDR_ip_is_multicast
 * @warning @c sa_family member of <tt>struct sockaddr</tt> shall be set to one of <tt>AF_INET, AF_INET6</tt>.
 */
static inline bool SOCKADDR_ip_is_broadcast(const sockaddr * saddr_ptr)
{
  bool b_is_broadcast = FALSE;
#if RTCSCFG_ENABLE_IP4
  #if RTCSCFG_ENABLE_IP6
  if(AF_INET == saddr_ptr->sa_family)
  {
  #endif
    b_is_broadcast = (INADDR_BROADCAST == (((sockaddr_in*)saddr_ptr)->sin_addr.s_addr));
  #if RTCSCFG_ENABLE_IP6
  }
  #endif
#endif
  return b_is_broadcast;
}

#if RTCSCFG_ENABLE_IP6
/**
 * @brief Get ip6 address.
 *
 * Given <tt>struct sockaddr</tt> is type casted to <tt>sockaddr_in6*</tt> and pointer to ip6 address member is returned.  
 * @param[in] saddr_ptr pointer to <tt>struct sockaddr</tt>
 * @return pointer to <tt>(in6_addr)</tt> in <tt>struct sockaddr</tt> data structure.
 * @see SOCKADDR_get_ipaddr4
 * @warning @c sa_family member of <tt>struct sockaddr</tt> shall be set to @c AF_INET6.
 */
static inline in6_addr * SOCKADDR_get_ipaddr6(const sockaddr * saddr_ptr)
{
  return &((sockaddr_in6*)saddr_ptr)->sin6_addr;
}
#endif

#if RTCSCFG_ENABLE_IP4
/**
 * @brief Get ip4 address.
 *
 * Given <tt>struct sockaddr</tt> is type casted to <tt>sockaddr_in*</tt> and @c _ip_address member is returned.
 * @param[in] saddr_ptr pointer to <tt>struct sockaddr</tt>
 * @return @c _ip_address read from the <tt>struct sockaddr</tt>.
 * @see SOCKADDR_get_ipaddr6
 * @warning @c sa_family member of <tt>struct sockaddr</tt> shall be set to @c AF_INET.
 */
static inline _ip_address SOCKADDR_get_ipaddr4(const sockaddr * saddr_ptr)
{
  return ((sockaddr_in*)saddr_ptr)->sin_addr.s_addr;
}
#endif

/**
 * @brief Get interface scope_id. IPv6 specific.
 *
 * Read an interface scope_id from <tt>struct sockaddr</tt>. Depending on @c sa_family member of this structure, the pointer is type casted
 * to <tt>sockaddr_in6*</tt> and value of if_scope_id is read from the data structure.
 * @param[in] saddr_ptr pointer to <tt>struct sockaddr</tt>
 * @return if_scope_id
 * @see SOCKADDR_set_if_scope_id
 * @warning @c sa_family member of <tt>struct sockaddr</tt> shall be set to @c AF_INET6. If set to any different value, this function will return zero.
 */
static inline uint32_t SOCKADDR_get_if_scope_id(const sockaddr * saddr_ptr)
{
  uint32_t if_scope_id = 0;
#if RTCSCFG_ENABLE_IP6
  #if RTCSCFG_ENABLE_IP4
  if(AF_INET6 == saddr_ptr->sa_family)
  {
  #endif
    if_scope_id = (((sockaddr_in6*)saddr_ptr)->sin6_scope_id);
  #if RTCSCFG_ENABLE_IP4
  }
  #endif
#endif
  return if_scope_id;
}

/**
 * @brief Set interface scope_id. IPv6 specific.
 *
 * Write given interface scope_id to <tt>struct sockaddr</tt>. Depending on @c sa_family member of this structure, the pointer is type casted
 * to <tt>sockaddr_in6*</tt> and value of if_scope_id is written to the data structure.
 * @param[in,out] saddr_ptr pointer to <tt>struct sockaddr</tt>
 * @param[in] if_scope_id Interface scope_id.
 * @return none
 * @see SOCKADDR_get_if_scope_id
 * @warning @c sa_family member of <tt>struct sockaddr</tt> shall be set to @c AF_INET6. If set to any different value, this function only returns.
 */
static inline void SOCKADDR_set_if_scope_id(sockaddr * saddr_ptr, const uint32_t if_scope_id)
{  
#if RTCSCFG_ENABLE_IP6
  #if RTCSCFG_ENABLE_IP4
  if(AF_INET6 == saddr_ptr->sa_family)
  {
  #endif
    (((sockaddr_in6*)saddr_ptr)->sin6_scope_id) = if_scope_id;
  #if RTCSCFG_ENABLE_IP4
  }
  #endif
#endif
  return;
}

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
