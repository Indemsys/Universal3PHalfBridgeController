#ifndef __ipcp_h__
#define __ipcp_h__
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
*   definitions required by IPCP.
*
*
*END************************************************************************/

/*
** ipcp.h - IP Control Protocol definitions.
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

#include <ppp.h>
#include "pppfsm.h"

#include "ip_prv.h"


/***************************************
**
** Constants
**
*/

/*
** PPP Protocol Numbers
*/

#define PPP_PROT_IP     0x0021
#define PPP_PROT_IPCP   0x8021


/*
** IPCP Configuration Options
*/

#define IPCP_CI_ADDR    3     /* IP Address */
#define IPCP_CI_DNS     129   /* Primary DNS server */


/***************************************
**
** Macros
**
*/

/*
** PPP error code -> RTCS error code conversion
*/

#define PPP_TO_RTCS_ERROR(x) ((x)?((x)|0x2000):(x))


/***************************************
**
** Type definitions
**
*/

/*
** The IPCP Options state used during negotiation
*/

typedef struct ipcp_neg {
   unsigned    NEG_ADDRS : 1;    /* Not used */
   unsigned    NEG_CP    : 1;    /* Not used */
   unsigned    NEG_ADDR  : 1;
   unsigned    NEG_DNS   : 1;
   unsigned              : 0;
   _ip_address ADDR;
   _ip_address DNS;
} IPCP_NEG, * IPCP_NEG_PTR;

typedef struct ipcp_init {

   unsigned    NEG_LOCAL_DNS      : 1;
   unsigned    NEG_REMOTE_DNS     : 1;

   unsigned    ACCEPT_LOCAL_ADDR  : 1;
   unsigned    ACCEPT_REMOTE_ADDR : 1;
   unsigned    ACCEPT_LOCAL_DNS   : 1;
   unsigned    ACCEPT_REMOTE_DNS  : 1;

   unsigned    DEFAULT_NETMASK    : 1;
   unsigned    DEFAULT_ROUTE      : 1;

   unsigned                       : 0;

   _ip_address LOCAL_ADDR;
   _ip_address REMOTE_ADDR;
   _ip_address NETMASK;
   _ip_address LOCAL_DNS;
   _ip_address REMOTE_DNS;

} IPCP_INIT, * IPCP_INIT_PTR;

typedef struct ipcp_opt {
   _ip_address ADDR;    /* Negotiated IP address */
   _ip_address DNS;     /* Negotiated DNS server address */
} IPCP_OPT, * IPCP_OPT_PTR;

/*
** The IPCP Configuration structure
*/

typedef struct ipcp_cfg_struct {
   _ppp_handle       HANDLE;        /* PPP channel */
   void (_CODE_PTR_  CALLUP)(void *);    /* previous PPP_LINK_UP callback */
   void             *PARAMUP;             /* previous PPP_LINK_UP parameter */
   void (_CODE_PTR_  CALLDOWN)(void *);  /* previous PPP_LINK_DOWN callback */
   void             *PARAMDOWN;           /* previous PPP_LINK_DOWN parameter */
   void (_CODE_PTR_  IP_UP)   (void *);  /* application UP callback */
   void (_CODE_PTR_  IP_DOWN) (void *);  /* application DOWN callback */
   void             *IP_PARAM;            /* parameter to IP_UP and IP_DOWN */
   IPCP_INIT         INIT;          /* Initial values provided by application */
   IPCP_NEG          NEG;           /* Options under negotiation */
   IPCP_OPT          LOPT;          /* Negotiated local options */
   IPCP_OPT          POPT;          /* Negotiated peer options */
   IPIF_PARM         BIND_PARMS;    /* Parameters to IPIF_[un]bind */
   IPIF_PARM         GATE_PARMS;    /* Parameters to IPIF_gate_{add,remove} */
   PPPFSM_CFG        FSM;           /* IPCP automaton state */
} IPCP_CFG_STRUCT, * IPCP_CFG_STRUCT_PTR;


/***************************************
**
** Function-specific type definitions
**
*/

/* IPCP_bind() */
typedef struct tcpip_parm_ipcp {
   TCPIP_PARM              COMMON;
   _rtcs_if_handle         handle;  /* [IN] the RTCS interface state structure */
   IPCP_DATA_STRUCT_PTR    data;    /* [IN] IPCP parameters */
} TCPIP_PARM_IPCP;


/***************************************
**
** Global variables
**
*/

extern PPPFSM_CALL IPCP_FSM_CALL;


/***************************************
**
** Prototypes
**
*/

#ifdef __cplusplus
extern "C" {
#endif


uint32_t IPCP_init    (IP_IF_PTR);
uint32_t IPCP_destroy (IP_IF_PTR);
uint32_t IPCP_send    (IP_IF_PTR, RTCSPCB_PTR, _ip_address, _ip_address, void *);
void IPCP_recv(PCB_PTR, void *);

void IPCP_bind(TCPIP_PARM_IPCP *);

#ifdef __cplusplus
}
#endif


#endif
/* EOF */
