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
*   This file contains the private defines, externs and
*   data structure definitions required by the Ethernet
*   packet driver.
*
*
*END************************************************************************/
#ifndef __enetprv_h__
#define __enetprv_h__


#ifdef __cplusplus
extern "C" {
#endif


/***************************************
**
** Code macros
**
*/
#define ENET_ALIGN(x,pow)        (((x) + ((pow)-1)) & ~((pow)-1))

//#define ENET_lock_context()      _int_disable()
#define ENET_lock_context(enet_ptr)      \
    {   \
        _lwsem_wait(&enet_ptr->CONTEXT_LOCK);    \
    }


//#define ENET_unlock_context()    _int_enable()
#define ENET_unlock_context(enet_ptr)   \
    {   \
        _lwsem_post(&enet_ptr->CONTEXT_LOCK);    \
    }

#define QADD(head,tail,pcb)      \
   if ((head) == NULL) {         \
      (head) = (pcb);            \
   } else {                      \
      (tail)->PRIVATE = (pcb);   \
   }                             \
   (tail) = (pcb);               \
   (pcb)->PRIVATE = NULL

#define QGET(head,tail,pcb)      \
   (pcb) = (head);               \
   if (head) {                   \
      (head) = (head)->PRIVATE;  \
      if ((head) == NULL) {      \
         (tail) = NULL;          \
      }                          \
   }             


/***************************************
**
** Data structures
**
*/


/* Joined multicast groups */
typedef struct enet_mcb_struct {
   _enet_address        GROUP;
   uint32_t              HASH;
   struct enet_mcb_struct          *NEXT;
} ENET_MCB_STRUCT, * ENET_MCB_STRUCT_PTR;

/* Registered protocol numbers */
typedef struct enet_ecb_struct {
   uint16_t              TYPE;
   void (_CODE_PTR_     SERVICE)(PCB_PTR, void *);
   ENET_MCB_STRUCT_PTR  MCB_HEAD;
   void                *PRIVATE;
   struct enet_ecb_struct          *NEXT;
} ENET_ECB_STRUCT, * ENET_ECB_STRUCT_PTR;

/* The Ethernet state structure */
struct enet_context_struct;

typedef struct enet_context_struct {
   struct enet_context_struct *  NEXT;
   ENET_ECB_STRUCT_PTR           ECB_HEAD;
   const ENET_PARAM_STRUCT *     PARAM_PTR;
   _enet_address                 ADDRESS;
   unsigned char                         PHY_ADDRESS;         // Note, keep adjacent to ADDRESS for alignment
   uint16_t                       MaxRxFrameSize;
   uint16_t                       MaxTxFrameSize;
   void                         *MAC_CONTEXT_PTR;   
#if BSPCFG_ENABLE_ENET_STATS
   ENET_STATS                    STATS;      
#endif
   LWSEM_STRUCT                  CONTEXT_LOCK;
} ENET_CONTEXT_STRUCT, * ENET_CONTEXT_STRUCT_PTR;


/***************************************
**
** Prototypes
**
*/

extern ENET_ECB_STRUCT_PTR ENET_find_receiver(ENET_CONTEXT_STRUCT_PTR, ENET_HEADER_PTR, uint32_t *);

extern uint16_t ENET_max_framesize(uint16_t, uint16_t, bool );

extern void    ENET_Enqueue_Buffer( void **q, void *b) ; 
extern void   *ENET_Dequeue_Buffer( void **q);

static inline uint32_t enet_send(ENET_CONTEXT_STRUCT_PTR,
      PCB_PTR, uint32_t, uint32_t, uint32_t, uint32_t *);

static inline uint32_t enet_send(ENET_CONTEXT_STRUCT_PTR  enet_ptr,
         /* [IN] the Ethernet state structure */
      PCB_PTR              packet,
         /* [IN] the packet to send */
      uint32_t              size,
         /* [IN] total size of the packet */
      uint32_t              frags,
         /* [IN] total fragments in the packet */
      uint32_t              flags,
         /* [IN] optional flags, zero = default */
      uint32_t *error
        )
{
  *error = (*enet_ptr->PARAM_PTR->ENET_IF->MAC_IF->SEND)((_enet_handle)enet_ptr, packet, size, frags, flags);
  return *error;
}

#ifdef __cplusplus
}
#endif

#endif /* __enetprv_h__ */
/* EOF */
