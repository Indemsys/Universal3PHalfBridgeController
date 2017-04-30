#ifndef __arp_prv_h__
#define __arp_prv_h__
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
*   Address Resolution Protocol definitions.
*
*
*END************************************************************************/


/***************************************
**
** Constants
**
*/

/*
** ARP operation codes
*/
#define ARPOP_REQUEST   1
#define ARPOP_REPLY     2

/*
** ARP protocol types
*/
#define ARPPROT_IP      0x800


#if (ARP_ENTRY_MAX_QUEUED_PCBS>1)
#define ARP_ENTRY_QUEUE_SIZE(e)    (e->QUEUED_PCBS)
#define ARP_ENTRY_DEQUEUE_PCB(e)    _arp_entry_dequeue_pcb(e)
#define ARP_ENTRY_ENQUEUE_PCB(e,p)  _arp_entry_enqueue_pcb(e,p)
#define ARP_ENTRY_QUEUE_PEEK(e)     _arp_entry_queue_peek(e)
#define ARP_ENTRY_QUEUE_INIT(e)     _arp_entry_queue_init(e)
#define ARP_ENTRY_QUEUE_FULL(e)     (e->QUEUED_PCBS == ARP_ENTRY_MAX_QUEUED_PCBS)
#else
#define ARP_ENTRY_QUEUE_SIZE(e)  ((e->WAITING != NULL)?1:0)
#define ARP_ENTRY_ENQUEUE_PCB(e,p)     (e->WAITING=p)
#define ARP_ENTRY_DEQUEUE_PCB(e)    e->WAITING; e->WAITING=NULL
#define ARP_ENTRY_QUEUE_PEEK(e)       (e->WAITING)
#define ARP_ENTRY_QUEUE_FULL(e)     (e->WAITING != NULL)
#define ARP_ENTRY_QUEUE_INIT(e)     (e->WAITING = NULL)

#endif
// END CR 2233


#if RTCSCFG_ENABLE_ARP_STATS
#define IF_ARP_STATS_ENABLED(x) x
#else
#define IF_ARP_STATS_ENABLED(x)
#endif

/***************************************
**
** Type definitions
**
*/

/*
** Contents of an ARP packet
*/
typedef struct arp_packet
{
   unsigned char    LINKTYPE[2];
   unsigned char    PROTTYPE[2];
   unsigned char    LINKLEN[1];
   unsigned char    PROTLEN[1];
   unsigned char    OPCODE[2];
   unsigned char    LINKSRC[6];
   unsigned char    PROTSRC[4];
   unsigned char    LINKDEST[6];
   unsigned char    PROTDEST[4];
} ARP_PACKET, * ARP_PACKET_PTR;


typedef struct arp_entry {

   _ip_address             PADDR;
   unsigned char                   LADDR[6];   /* ignored when COMPLETE == FALSE */
   unsigned char                   RESERVED[2];

   IP_IF_PTR               HEAD;
   struct arp_entry       *PREV;
   struct arp_entry       *NEXT;

   bool                 COMPLETE;
   bool                 AGED;       /* ignored when COMPLETE == FALSE */
   bool                 HIT;        /* ignored when COMPLETE == FALSE */

// START CR 2233
#if (ARP_ENTRY_MAX_QUEUED_PCBS==1)
   /* There is at most one queued PCB on any incomplete entry */
   RTCSPCB_PTR             WAITING;

#else
   /* There may be multiple queued PCB on any incomplete entry */
   RTCSPCB_PTR             WAITING[ARP_ENTRY_MAX_QUEUED_PCBS];
   uint32_t                 QUEUED_PCBS;
#endif
// END CR 2233

   _ip_address             WAITADDR;   /* ignored when RESEND_TIMER inactive */
   RTCS_LINKOPT_STRUCT     WAITOPT;    /* ignored when RESEND_TIMER inactive */

   uint32_t                 RESEND_INTERVAL;
   TCPIP_EVENT             RESEND_TIMER;
   TCPIP_EVENT             AGE_TIMER;
   TCPIP_EVENT             EXPIRE_TIMER;

} ARP_ENTRY, * ARP_ENTRY_PTR;


/*
** ARP Configuration
**    This information is persistent for the ARP layer.
*/
typedef struct arp_cfg
{
#if RTCSCFG_ENABLE_ARP_STATS
    ARP_STATS       STATS;
#endif
    ARP_ENTRY_PTR   CACHE;
    ARP_ENTRY_PTR   CACHE_FREE;
    void *          CACHE_PTR;
    unsigned char   LADDR[6];
    unsigned char   RESERVED[2];
    ARP_PACKET      ARP_REQUEST;
} ARP_CFG, * ARP_CFG_PTR;


/***************************************
**
** Global variables
**
*/

#ifdef __cplusplus
extern "C" {
#endif

#if (ARP_ENTRY_MAX_QUEUED_PCBS!=1)
extern RTCSPCB_PTR   _arp_entry_dequeue_pcb(ARP_ENTRY_PTR e);
extern bool       _arp_entry_enqueue_pcb(ARP_ENTRY_PTR e,RTCSPCB_PTR p);
extern RTCSPCB_PTR   _arp_entry_queue_peek(ARP_ENTRY_PTR e);
extern bool       _arp_entry_queue_init(ARP_ENTRY_PTR e);
#endif

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
