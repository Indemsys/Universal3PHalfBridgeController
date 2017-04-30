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
*   This file contains the various global PPP structures
*
*
*END************************************************************************/

#include <rtcs.h>

#if RTCSCFG_ENABLE_PPP && !PLATFORM_SDK_ENABLED

#include <ppp.h>
#include "ppp_prv.h"


/*
** PPP task priority and extra stack size
*/

uint32_t  _PPPTASK_priority   = (RTCSCFG_DEFAULT_RTCSTASK_PRIO+1);
uint32_t  _PPPTASK_stacksize  = 0;

/*
** Variables used by the retransmission timer
*/

uint32_t _PPP_MIN_XMIT_TIMEOUT = 3000;        /* RFC 1661 4.6 */
uint32_t _PPP_MAX_XMIT_TIMEOUT = 10000l;
uint32_t _PPP_MAX_CONF_RETRIES = 10;          /* RFC 1661 4.6 */
uint32_t _PPP_MAX_TERM_RETRIES = 2;           /* RFC 1661 4.6 */
uint32_t _PPP_MAX_CONF_NAKS    = 5;           /* RFC 1661 4.6 */


/*
** The default PPP link options
*/

PPP_OPT PPP_DEFAULT_OPTIONS = {
   /* ACCM     */   0xFFFFFFFF,
   /* MRU      */   DEFAULT_MRU,
   /* AP       */   0,
   /* QP       */   0,
   /* AP_Start */   NULL,
   /* MAGIC    */   0,
   /* PFC      */   FALSE,
   /* ACFC     */   FALSE,
   /* CP       */   NULL
};

PPP_OPT PPP_DEFAULT_OPTIONS_PPPOE = {
   /* ACCM     */   0xFFFFFFFF,
   /* MRU      */   1492,
   /* AP       */   0,
   /* QP       */   0,
   /* AP_Start */   NULL,
   /* MAGIC    */   0,
   /* PFC      */   FALSE,
   /* ACFC     */   FALSE,
   /* CP       */   NULL
};

/*
** The minimal ACCM
*/

uint32_t _PPP_ACCM = 0xFFFFFFFFl;


/*
** The secrets.  These are the secrets used by PAP and CHAP.
** an xSECRET is a void   *to a single secret; an xSECRETS
** points to an array of secrets.
**
** LSECRET(S) points to the secret(s) to use when authenticating
** ourself to the peer (local secret(s)); RSECRET(S) points to
** the secret(s) to use when authenticating the peer (remote
** secret(s)).
**
** If an LSECRET(S) pointer is NULL, LCP will not allow the
** peer to request that authentication protocol.  If all LSECRET(S)
** are NULL, LCP will reject negotiation of the authentication
** protocol.
**
** If any RSECRET(S) are non-NULL, LCP will require that the peer
** authenticate itself.  If the peer rejects negotiation of the
** authentication protocol, LCP will terminate the link immediately
** upon reaching the OPENED state.
*/

PPP_SECRET_PTR _PPP_PAP_LSECRET = NULL;
PPP_SECRET_PTR _PPP_PAP_RSECRETS = NULL;
char       *_PPP_CHAP_LNAME = "";
PPP_SECRET_PTR _PPP_CHAP_LSECRETS = NULL;
PPP_SECRET_PTR _PPP_CHAP_RSECRETS = NULL;

#endif // RTCSCFG_ENABLE_PPP

