#ifndef __iptunnel_h__
#define __iptunnel_h__
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
*   This file contains the defines, externs and data
*   structure definitions required by IP over IP encapsulation.
*
*
*END************************************************************************/

#include "ip_prv.h"


/*********************************************************
**
** IPIP CONSTANTS
*/

#define IPIP_INITIAL_COUNT  4
#define IPIP_GROW_COUNT     4


/*********************************************************
**
** IPIP Structures
*/

/*
** IPIP_TUNNEL
**    Contains information about an IP over IP tunnel
*/
typedef struct ipip_tunnel {
   struct ipip_tunnel      *NEXT;       /* Points to next node in list */
   _ip_address    INNER_SOURCE;           /* Embedded header source */
   _ip_address    INNER_SOURCE_NETMASK;   /* Embedded header source mask */
   _ip_address    INNER_DEST;             /* Embedded header destination */
   _ip_address    INNER_DEST_NETMASK;     /* Embedded header dest mask */
   _ip_address    OUTER_SOURCE;           /* Source for outer header */
   _ip_address    OUTER_DEST;             /* Destination for outer header */
   uint32_t        FLAGS;                  /* Behaviour flags */
} IPIP_TUNNEL, * IPIP_TUNNEL_PTR;


/*
** IPIP_PARM
**    Used to pass information from the application to RTCS
*/
typedef struct  ipip_parm {
   TCPIP_PARM     COMMON;                 /* Common to all parm structs */
   IPIP_TUNNEL    TUNNEL;
} IPIP_PARM, * IPIP_PARM_PTR;


/*
** IPIP_CFG_STRUCT
**    Contains the configuration information for the IP Tunnel device
*/
typedef struct ipip_cfg_struct {
   IPIP_TUNNEL_PTR   NEXT;             /* Linked list of tunnels */
   _rtcs_part        IPIP_PART;        /* Tunnel information part */
   IP_IF_PTR         IPIP_IF;          /* IPIP interface pointer */
   uint32_t           TUNNELS;          /* Number of tunnels */
} IPIP_CFG_STRUCT, * IPIP_CFG_STRUCT_PTR;



#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t IPIP_init (void);

extern void IPIP_service (RTCSPCB_PTR pcb_ptr, void *dummy);

extern void IPIP_insert (IPIP_PARM_PTR);
extern void IPIP_delete (IPIP_PARM_PTR);

extern uint32_t IPIP_open  (IP_IF_PTR);
extern uint32_t IPIP_close (IP_IF_PTR);

extern uint32_t IPIP_send (IP_IF_PTR, RTCSPCB_PTR, _ip_address, _ip_address, void *);

extern void IPIP_send_internal (RTCSPCB_PTR);

extern uint32_t IPIP_MTU (void *);

#ifdef __cplusplus
}
#endif


#endif

/* EOF */
