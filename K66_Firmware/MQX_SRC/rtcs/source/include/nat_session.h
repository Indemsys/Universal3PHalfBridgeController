#ifndef __nat_session_h__
#define __nat_session_h__
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
*   Private Network Address Translator definitions.
*
*
*END************************************************************************/

 


/* 
** NAT_EVENT_STRUCT:
**    Used for timeouts.
**
*/
struct nat_session;

typedef struct nat_event_struct { 
   uint32_t                    TIME;    
   struct nat_session_struct        *SESSION_PTR;
   TCPIP_EVENT_PTR            EVENT;
   struct nat_event_struct          *NEXT;
   struct nat_event_struct          *PREV;
} NAT_EVENT_STRUCT, * NAT_EVENT_STRUCT_PTR;


/* 
** NAT_SESSION_EXT_STRUCT 
**    Holds ALG specific information attached to a session
*/
typedef struct nat_session_ext_struct {
   struct nat_session_ext_struct       *NEXT;
   uint32_t     ALG_TYPE;
} NAT_SESSION_EXT_STRUCT, * NAT_SESSION_EXT_STRUCT_PTR;

/* 
** NAT_SESSION_STRUCT: 
**    Holds information about a sessions. One of these is allocated for 
**    each session.
*/
typedef struct nat_session_struct {
   struct nat_session_struct           *NEXT_IN;       /* Used for IN tree           */
   struct nat_session_struct           *NEXT_OUT;      /* Used for OUT tree          */
   _ip_address                   PRV_HST;       /* private host IP            */
   _ip_address                   PUB_HST;       /* public host IP             */
   _ip_address                   NAT_HST;       /* replacement IP             */
   uint16_t                       PRV_PORT;      /* private host port          */
   uint16_t                       PUB_PORT;      /* public host port           */
   uint16_t                       NAT_PORT;      /* replacement port           */
   uint16_t                       RESERVED;      /* unused                     */
   uint32_t                       IP_PROT;       /* IP protocol                */
   NAT_EVENT_STRUCT              TIMEOUT;       /* to close sessions          */   
   uint32_t                       STATE;         /* holds state information    */
   NAT_SESSION_EXT_STRUCT_PTR    ALG_INFO_PTR;  /* ALG specifif information   */
   bool                       SNAT_OR_DNAT;  /* SNAT or DNAT session       */ 
} NAT_SESSION_STRUCT, * NAT_SESSION_STRUCT_PTR;



#endif
