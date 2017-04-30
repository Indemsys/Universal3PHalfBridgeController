#ifndef __ppp_prv_h__
#define __ppp_prv_h__
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
*   This file contains the private defines, externs and data
*   structure definitions required by the PPP Library.
*
*
*END************************************************************************/

#include "pppfsm.h"
#include "lcp.h"
#include "pap.h"
#include "chap.h"
#include "ccp.h"


/*
** Assorted PPP constants
*/

#define PPP_VALID       0x1A0A80CFl
#define PPP_PROT_CP     0x00FD
#define PPP_PROT_CCP    0x80FD
#define PPP_PROT_LCP    0xC021
#define PPP_PROT_PAP    0xC023
#define PPP_PROT_CHAP   0xC223

/*
** Control packet types
*/

#define PPPCMD_SEND     0
#define PPPCMD_RESTART  1
#define PPPCMD_STOP     2
#define PPPCMD_SHUTDOWN 3

/*
** Constants for the Message Pool
*/

#define PPP_MESSAGE_INITCOUNT    8
#define PPP_MESSAGE_GROWTH       4
#define PPP_MESSAGE_LIMIT        32

/*
 * Constants for checking cable is disconnected or not
 */
#define PPP_CHECK_CONN_TIMEOUT   500
#define PPP_CHECK_CONN_RETRY     4

struct ppp_opt;
struct ppp_cfg;

typedef struct ppp_message {
   MESSAGE_HEADER_STRUCT   HEADER;
   uint16_t                 COMMAND;
   uint16_t                 PROTOCOL;
   PCB_PTR                 PCB;
   void (_CODE_PTR_        ORIG_FREE)(PCB_PTR);
   uint32_t                 TIMEOUT;
   uint32_t                 RETRY;
   void (_CODE_PTR_        CALL)(void *, PCB_PTR, bool);
   void                   *PARAM;
   int32_t                  DELTA;
   struct ppp_message         *NEXT;
} PPP_MESSAGE, * PPP_MESSAGE_PTR;

/*
** The PPP Options structure.  This structure contains all the
** negotiated options for an open PPP link.
*/

typedef struct ppp_opt {
   uint32_t           ACCM;
   uint32_t           MRU;
   uint16_t           AP;
   uint16_t           QP;            /* Not implemented */
   void (_CODE_PTR_  AP_Start)(_ppp_handle);
   uint32_t           MAGIC;
   bool           PFC;
   bool           ACFC;
   CP_CALL_PTR       CP;
} PPP_OPT, * PPP_OPT_PTR;

extern PPP_OPT PPP_DEFAULT_OPTIONS;
extern PPP_OPT PPP_DEFAULT_OPTIONS_PPPOE;


/*
** The LCP Options state used during negotiation
*/

typedef struct lcp_neg {
   unsigned NEG_MRU   : 1;
   unsigned NEG_ACCM  : 1;
   unsigned NEG_AP    : 1;
   unsigned NEG_QP    : 1;    /* Not used */
   unsigned NEG_MAGIC : 1;    /* Not used */
   unsigned NEG_PFC   : 1;
   unsigned NEG_ACFC  : 1;
   unsigned NEG_FCS   : 1;    /* Not used */
   unsigned           : 0;
   uint16_t  MRU;
   uint32_t  ACCM;       /* LCP only negotiates 32 bits of the ACCM */
   uint16_t  AP;
} LCP_NEG, * LCP_NEG_PTR;

/*
** The LCP Configuration structure
*/

typedef struct lcp_cfg {

   /* Statistics counters */
   uint32_t        ST_LCP_RX_REJECT;
   uint32_t        ST_LCP_RX_ECHO;
   uint32_t        ST_LCP_RX_REPLY;
   uint32_t        ST_LCP_RX_DISCARD;
   uint32_t        ST_LCP_TX_REJECT;
   uint32_t        ST_LCP_TX_ECHO;
   uint32_t        ST_LCP_TX_REPLY;
   uint32_t        ST_LCP_TX_DISCARD;

   _iopcb_handle  DEVICE;
   PPP_OPT        SEND_OPT;
   PPP_OPT        RECV_OPT;
   LCP_NEG        RECV_NEG;
/* Start CR 2207 */
#if PPP_SECRETS_NOT_SHARED
    _ppp_handle    HANDLE;
#endif
/* End CR 2207 */
} LCP_CFG, * LCP_CFG_PTR;


/*
** The PPP Callback structure.  An array of these structures allows
** PPP to inform the application when certain events occur.
*/

typedef struct ppp_call_internal {
   void (_CODE_PTR_  CALLBACK)();
   void             *PARAM;
} PPP_CALL_INTERNAL, * PPP_CALL_INTERNAL_PTR;


/*
** The Protocol Callback structure.  A linked list of these structures allows
** PPP_rx_task to forward incoming packets to upper layer protocols.
*/

typedef struct prot_call {
   uint16_t                 PROTOCOL;
   void (_CODE_PTR_        CALLBACK)(PCB_PTR, void *);
   void                   *PARAM;
   struct prot_call       *NEXT;
} PROT_CALL, * PROT_CALL_PTR;


/*
** The PPP Configuration structure.
**
** MUTEX provides mutual exclusion on LINK_STATE, RECV_OPTIONS and
** SEND_OPTIONS, since they are read by both the Rx and Tx tasks,
** and written by the Rx task and any task that calls PPP_open or PPP_close.
**
** MUTEX is also used for mutual exclusion on the PROT_CALLS linked
** list.  We don't have to use the same mutex, but it saves space.
*/

typedef struct ppp_cfg {

   /* Statistics counters */
   uint32_t                 ST_RX_RECEIVED;
   uint32_t                 ST_RX_ABORTED;
   uint32_t                 ST_RX_DISCARDED;
   uint32_t                 ST_RX_RUNT;
   uint32_t                 ST_RX_LONG;
   uint32_t                 ST_RX_BAD_ADDR;
   uint32_t                 ST_RX_BAD_CTRL;
   uint32_t                 ST_RX_BAD_FCS;
   uint32_t                 ST_TX_SENT;
   uint32_t                 ST_TX_DISCARDED;
   uint32_t                 ST_TX_BAD_FCS;

   uint32_t                 VALID;
   bool                 LINK_STATE;              /* TRUE if PPP link is up */
   bool                 LINK_AUTH;               /* TRUE if the peer is authentic */
   _ppp_mutex              MUTEX;
   LCP_CFG                 LCP_STATE;
   PPPFSM_CFG              LCP_FSM;
   PPP_CALL_INTERNAL       LCP_CALL[PPP_CALL_MAX];
   char*                   DEVICE_NAME;             /* Name of low-level communication device */
   _iopcb_handle           DEVICE;                  /* for _iopcb_read/_iopcb_write */
   MQX_FILE_PTR            IOPCB_DEVICE;            /* Low level device handle - used by HDLC layer */
   _queue_id               MSG_QUEUE;               /* for Tx task */
   _queue_id               RESERVED1;
   _pool_id                MSG_POOL;                /* for Tx task */

   PROT_CALL_PTR           PROT_CALLS;
   PPP_OPT volatile       *RECV_OPTIONS;
   PPP_OPT volatile       *SEND_OPTIONS;

   PAP_DATA                PAP_STATE;
   CHAP_DATA               CHAP_STATE;

   CCP_CFG                 CCP_STATE;
   PPPFSM_CFG              CCP_FSM;

   _rtcs_taskid            RX_TASKID;
   _rtcs_taskid            TX_TASKID;
   /* new fix */
   volatile   bool      STOP_RX;
   /***********/
/* Start CR 2207 */
#if PPP_SECRETS_NOT_SHARED
   PPP_SECRET_PTR   _PPP_PAP_LSECRET;

   /* The array of secrets to use to authenticate the peer with PAP */
   PPP_SECRET_PTR   _PPP_PAP_RSECRETS;

   /* The name of the local host (required for CHAP) */
   char   *_PPP_CHAP_LNAME;

   /* The array of secrets to use if the peer requests CHAP authentication */
   PPP_SECRET_PTR   _PPP_CHAP_LSECRETS;

   /* The array of secrets to use to authenticate the peer with CHAP */
   PPP_SECRET_PTR   _PPP_CHAP_RSECRETS;
#endif
/* End CR 2207 */
   _rtcs_if_handle  IF_HANDLE;
} PPP_CFG, * PPP_CFG_PTR;

#ifdef __cplusplus
extern "C" {
#endif

extern PCB_PTR PPP_pcballoc (uint16_t, uint32_t);

extern uint32_t PPP_send_one      (PPP_CFG_PTR, uint16_t, PCB_PTR);
extern uint32_t PPP_send_rexmit   (PPP_CFG_PTR, uint16_t, PCB_PTR,
                                  uint32_t, void (_CODE_PTR_)(void *, PCB_PTR, bool), void *);
extern uint32_t PPP_send_restart  (PPP_CFG_PTR, uint16_t);
extern uint32_t PPP_send_stop     (PPP_CFG_PTR, uint16_t);
extern uint32_t PPP_send_shutdown (PPP_CFG_PTR, void *);

extern void PPP_init_fail(PPP_CFG_PTR ppp_ptr, int stage);

extern void PPP_up(_ppp_handle);
extern void PPP_MD5_block(uint32_t *, uint32_t *, const uint32_t *);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
