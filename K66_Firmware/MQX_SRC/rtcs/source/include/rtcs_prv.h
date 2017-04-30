/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
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
*   This file contains the include files for the
*   RTCS Communication Library.
*
*
*END************************************************************************/

#ifndef __rtcs_prv_h__
#define __rtcs_prv_h__

#include "rtcspcb.h"

/*
** RTCS queue numbers
*/
#define TCPIP_QUEUE     _RTCSQUEUE_base


/*
** Maximum number of protocols that can be logged
*/
#define RTCSLOG_PROT_MAX   16


/*
** Macro definitions for manipulating common data structures
*/

   /*
   ** RTCS_QUEUE: a singly-linked FIFO list
   **
   ** - requires a head and a tail
   ** - each element requires a field called NEXT
   */

#define RTCS_QUEUE_INIT(head,tail)              \
      (head) = NULL

#define RTCS_QUEUE_PEEK(head,tail,elem)         \
      (elem) = (head)

#define RTCS_QUEUE_INS(head,tail,elem)          \
      (elem)->NEXT = NULL;                      \
      if (head) {                               \
         (tail)->NEXT = (elem);                 \
         (tail) = (elem);                       \
      } else {                                  \
         (head) = (elem);                       \
         (tail) = (elem);                       \
      }

#define RTCS_QUEUE_DEL(head,tail)               \
      if (head) {                               \
         (head) = (head)->NEXT;                 \
      }

#define RTCS_QUEUE_DEL_NONEMPTY(head,tail)      \
      (head) = (head)->NEXT

#define RTCS_QUEUE_DELALL(head,tail,elem,temp)  \
      for ((elem)=(head);                       \
           (elem)?(temp)=(elem)->NEXT:0,        \
           (elem);                              \
           (elem)=(temp))

   /*
   ** RTCS_LIST: a singly-linked list
   **
   ** - requires a head
   ** - each element requires a field called NEXT
   */

#define RTCS_LIST_INIT(head)                    \
      (head) = NULL

#define RTCS_LIST_INS(head,elem)                \
      (elem)->NEXT = (head);                    \
      (head) = (elem)

#define RTCS_LIST_INS_END(head,elem,tempp)      \
      for ((tempp)=&(head);                     \
            *(tempp);                           \
            (tempp)=&(*(tempp))->NEXT) ;        \
      *(tempp) = (elem);                        \
      (elem)->NEXT = NULL

#define RTCS_LIST_PEEK(head,elem)               \
      (elem) = (head)

#define RTCS_LIST_SEARCH(head,elem)             \
      for ((elem)=(head);                       \
           (elem);                              \
           (elem)=(elem)->NEXT)

#define RTCS_LIST_DEL(head,elemp)               \
      *(elemp) = (*(elemp))->NEXT

#define RTCS_LIST_SEARCHMOD(head,elemp)         \
      for ((elemp)=&(head);                     \
           *(elemp);                            \
           (elemp)=&(*(elemp))->NEXT)

#define RTCS_LIST_SEARCHMOD_NOHEAD(head,elemp)  \
      for ((elemp)=&(head)->NEXT;               \
           *(elemp);                            \
           (elemp)=&(*(elemp))->NEXT)

   /*
   ** RTCS_DLIST: a doubly-linked list
   **
   ** - requires a head
   ** - each element requires fields called NEXT and PREV
   */

#define RTCS_DLIST_INIT(head)                   \
      (head) = NULL

#define RTCS_DLIST_INS(head,elem)               \
      (elem)->NEXT = (head);                    \
      (elem)->PREV = &(head);                   \
      if (head) {                               \
         (head)->PREV = &(elem)->NEXT;          \
      }                                         \
      (head) = (elem)

#define RTCS_DLIST_PEEK(head,elem)              \
      (elem) = (head)

#define RTCS_DLIST_SEARCH(head,elem)            \
      for ((elem)=(head);                       \
           (elem);                              \
           (elem)=(elem)->NEXT)

#define RTCS_DLIST_SEARCH_REST(head,elem)       \
      for ((elem)=(elem)->NEXT;                 \
           (elem);                              \
           (elem)=(elem)->NEXT)

#define RTCS_DLIST_DEL(head,elem)               \
      if ((elem)->NEXT) {                       \
         (elem)->NEXT->PREV = (elem)->PREV;     \
      }                                         \
      *(elem)->PREV = (elem)->NEXT


/*
** Macro definitions for RTCS
*/

#define RTCSCMD_issue(parm,cmd)      RTCS_cmd_issue(&(parm).COMMON, (void(_CODE_PTR_)(void *))(cmd))
#define RTCSCMD_internal(parm,cmd)   RTCS_cmd_internal(&(parm).COMMON, (void(_CODE_PTR_)(void *))(cmd))
#define RTCSCMD_smartissue(parm,cmd) RTCS_cmd_smartissue(&(parm).COMMON, (void(_CODE_PTR_)(void *))(cmd))
#define RTCSCMD_service(pcb,cmd)     RTCS_cmd_service(pcb, (void(_CODE_PTR_)(void *))(cmd))
#define RTCSCMD_complete(parm,err)   RTCS_cmd_complete(&(parm)->COMMON, err)

#define RTCS_getcfg(prot)        ((RTCS_DATA_PTR)RTCS_get_data())-> prot ## _CFG
#define RTCS_setcfg(prot,ws2812B_DMA_cfg)    ((RTCS_DATA_PTR)RTCS_get_data())-> prot ## _CFG = (ws2812B_DMA_cfg)

typedef struct rtcs_data {
   uint32_t        VERSION;

   void          *TCPIP_CFG;
   void          *SOCKET_CFG;
   void          *IP_CFG;
   void          *ICMP_CFG;
   void          *UDP_CFG;
   void          *TCP_CFG;

   _pool_id       TCPIP_msg_pool;
   _queue_id      TCPIP_qid;
   _rtcs_part     RTCS_PCB_partition;
   _rtcs_part     RTCS_socket_partition;

#if RTCSCFG_ENABLE_IGMP && RTCSCFG_ENABLE_IP4
   void          *IGMP_CFG;
#endif
#if RTCSCFG_ENABLE_RIP && RTCSCFG_ENABLE_IP4 
   void          *RIP_CFG;
#endif
#if RTCSCFG_ENABLE_OSPF
   void          *OSPF_CFG;
#endif

   _rtcs_taskid   TCPIP_TASKID;

#if RTCSCFG_LOG_SOCKET_API || RTCSCFG_LOG_PCB || RTCSCFG_LOGGING
   uint32_t        RTCS_LOG_CONTROL;
   uint32_t        RTCS_LOG_PROTCOUNT;
   uint16_t        RTCS_LOG_PCB[RTCSLOG_PROT_MAX];
   uint16_t        RTCS_LOG_TIMER[RTCSLOG_PROT_MAX];
#endif

#if RTCSCFG_ENABLE_NAT
   void          *NAT_CFG;
#endif
#if RTCSCFG_ENABLE_IPIP && RTCSCFG_ENABLE_IP4
   void          *IPIP_CFG;
#endif
#if RTCSCFG_ENABLE_IP6 
    void          *IP6_CFG;
    void          *ICMP6_CFG;
#endif

} RTCS_DATA, * RTCS_DATA_PTR;

/*
** externs for RTCS utility procedures
*/

#ifdef __cplusplus
extern "C" {
#endif

extern void RTCS_seterror
(
   RTCS_ERROR_STRUCT_PTR   errptr,
   uint32_t                 errcode,
   uint32_t                 errparm
);

extern void RTCS_setsockerror
(
   uint32_t     sock,
   uint32_t     error
);

extern uint32_t RTCS_cmd_issue      (TCPIP_PARM_PTR, void(_CODE_PTR_)(void *));
extern uint32_t RTCS_cmd_internal   (TCPIP_PARM_PTR, void(_CODE_PTR_)(void *));
extern uint32_t RTCS_cmd_smartissue (TCPIP_PARM_PTR, void(_CODE_PTR_)(void *));
extern bool RTCS_cmd_service    (RTCSPCB_PTR,    void(_CODE_PTR_)(void *));
extern void    RTCS_cmd_complete   (TCPIP_PARM_PTR, uint32_t);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
