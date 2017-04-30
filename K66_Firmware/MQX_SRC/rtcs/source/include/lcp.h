#ifndef __lcp_h__
#define __lcp_h__
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
*   definitions required by LCP.
*
*
*END************************************************************************/

/*
** lcp.h - Link Control Protocol definitions.
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
** LCP-specific packet types
*/

#define LCP_CODE_PROT_REJ  8       /* Protocol Reject */
#define LCP_CODE_ECHO_REQ  9       /* Echo Request */
#define LCP_CODE_ECHO_REP  10      /* Echo Reply */
#define LCP_CODE_DISC_REQ  11      /* Discard Request */


/*
** LCP Configuration Options
*/

#define LCP_CI_MRU      1     /* Maximum Receive Unit */
#define LCP_CI_ACCM     2     /* Async Control Character Map */
#define LCP_CI_AP       3     /* Authentication Protocol */
#define LCP_CI_QP       4     /* Quality Protocol */
#define LCP_CI_MAGIC    5     /* Magic Number */
#define LCP_CI_PFC      7     /* Protocol Field Compression */
#define LCP_CI_ACFC     8     /* Address and Control Field Compression */
#define LCP_CI_FCS      9     /* Frame Check Sequence Method */


/*
** Prototypes
*/

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t LCP_init    (_ppp_handle);
extern uint32_t LCP_destroy (_ppp_handle);

extern void LCP_sendprotrej (PCB_PTR, PPPFSM_CFG_PTR);

#ifdef __cplusplus
}
#endif

#define LCP_input PPPFSM_input

#endif
/* EOF */
