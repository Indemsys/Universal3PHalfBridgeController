#ifndef __fsm_h__
#define __fsm_h__
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
*   This file contains the defines and data structure
*   definitions required by the PPP Finite State Machine.
*
*
*END************************************************************************/

/*
** fsm.h - {Link, IP} Control Protocol Finite State Machine definitions.
**
** Copyright (c) 1989 Carnegie Mellon University.
** All rights reserved.
**
** Redistribution and use in source and binary forms are permitted
** provided that the above copyright notice and this paragraph are
** duplicated in all such forms and that any documentation,
** advertising materials, and other materials related to such
** distribution and use acknowledge that the software was developed
** by Carnegie Mellon University.  The name of the
** University may not be used to endorse or promote products derived
** from this software without specific prior written permission.
** THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
** WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
** Packet header = Code, id, length.
*/

#define CP_HDR_LEN   4


/*
**  CP (LCP, IPCP, etc.) codes.
*/

#define CP_CODE_CONF_REQ  1     /* Configuration Request */
#define CP_CODE_CONF_ACK  2     /* Configuration Ack */
#define CP_CODE_CONF_NAK  3     /* Configuration Nak */
#define CP_CODE_CONF_REJ  4     /* Configuration Reject */
#define CP_CODE_TERM_REQ  5     /* Termination Request */
#define CP_CODE_TERM_ACK  6     /* Termination Ack */
#define CP_CODE_CODE_REJ  7     /* Code Reject */
#define CP_CODE_ECHO_REQ  9     /* Echo request */
#define CP_CODE_ECHO_REPLY  10  /* Echo reply */


/*
** The FSM call table.  Contains a list a functions to call when various
** events occur in the FSM.
*/
struct pppfsm_cfg;

typedef struct pppfsm_call {
   uint16_t             PROTOCOL;       /* Protocol number */
   bool (_CODE_PTR_ linkup)  (struct pppfsm_cfg *); /* Link open */
   void    (_CODE_PTR_ linkdown)(struct pppfsm_cfg *); /* Link no longer open */
   void    (_CODE_PTR_ resetreq)(struct pppfsm_cfg *); /* Reset configuration options */
   uint32_t (_CODE_PTR_ buildreq)(struct pppfsm_cfg *, unsigned char *, uint32_t); /* Build ConfReq */
   uint32_t (_CODE_PTR_ recvreq) (struct pppfsm_cfg *, bool); /* Convert ConfReq to ConfAck/Nak/Rej */
   bool (_CODE_PTR_ recvack) (struct pppfsm_cfg *); /* TRUE if ConfAck is PPP_OK */
   bool (_CODE_PTR_ recvnak) (struct pppfsm_cfg *); /* TRUE if ConfNak is PPP_OK */
   bool (_CODE_PTR_ recvrej) (struct pppfsm_cfg *); /* TRUE if ConfRej is PPP_OK */
   bool (_CODE_PTR_ testcode)(struct pppfsm_cfg *); /* TRUE if CodeRej is catastrophic */
   bool (_CODE_PTR_ recvcode)(struct pppfsm_cfg *); /* TRUE if code is PPP_OK */
} PPPFSM_CALL, * PPPFSM_CALL_PTR;

/*
** The FSM Configuration structure.
*/

typedef struct pppfsm_cfg {

   /* Statistics counters */
   uint32_t           ST_CP_SHORT;
   uint32_t           ST_CP_DOWN;
   uint32_t           ST_CP_NOBUFS;
   uint32_t           ST_CP_BAD_ACK;
   uint32_t           ST_CP_BAD_NAK;
   uint32_t           ST_CP_BAD_REJ;

   _ppp_handle       HANDLE;     /* the PPP state structure */
   PPPFSM_CALL_PTR   CALL;       /* The protocol-specific call table */
   void             *PRIVATE;    /* Parameter for protocol-specific functions */
   uint32_t           STATE;      /* Current state */
   _ppp_mutex        MUTEX;      /* Mutual exclusion for the STATE field */
   LWEVENT_STRUCT    CONN_EVENT;    /* Used for checking if cable is disconnected or not */

   unsigned char             CURID;      /* ID of the next ConfReq */
   unsigned char             REQID;      /* ID of the last ConfReq */
   uint32_t           NAKS;       /* Number of consecutive NAKs */

   PCB_PTR           PACKET;     /* Current packet */
   unsigned char             CODE;       /* Temporary location for the code, */
   unsigned char             ID;         /*    id and length fields for the  */
   uint16_t           LENGTH;     /*    current packet                */
   unsigned char         *DATA;       /* Current position in the current packet */

   uint32_t           OPTIONS;    /* Options passed to PPPFSM_open() */

} PPPFSM_CFG, * PPPFSM_CFG_PTR;


/*
** Prototypes
*/

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t PPPFSM_init      (PPPFSM_CFG_PTR, _ppp_handle, PPPFSM_CALL_PTR, void *);
extern uint32_t PPPFSM_destroy   (PPPFSM_CFG_PTR);
extern void    PPPFSM_open      (PPPFSM_CFG_PTR, uint32_t);
extern void    PPPFSM_close     (PPPFSM_CFG_PTR);
extern void    PPPFSM_lowerup   (PPPFSM_CFG_PTR);
extern void    PPPFSM_lowerdown (PPPFSM_CFG_PTR);

extern void    PPPFSM_input (PCB_PTR, void *);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
