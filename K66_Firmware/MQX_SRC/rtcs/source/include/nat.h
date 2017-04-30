#ifndef __nat_h__
#define __nat_h__
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
*   Network Address Translator definitions.
*
*
*END************************************************************************/


#if RTCS_VERSION < 0x00029600L
#error "This version of NAT requires RTCS 2.96.00 or higher"
#endif


/*************************************
**
** NAT constants
*/
#define NAT_DEFAULT_TIMEOUT_TCP     (15 * 60 * 1000)  /* 15 minutes */
#define NAT_DEFAULT_TIMEOUT_UDP     ( 5 * 60 * 1000)  /*  5 minutes */
#define NAT_DEFAULT_TIMEOUT_FIN     ( 2 * 60 * 1000)  /*  2 minutes */
/*
 * ICMP specific NAT extension
 * NAT Default ICMP session timeout. It should be small in order
 * to minimized the system resources, each PING creates a new NAT
 * session
 */
#define NAT_DEFAULT_TIMEOUT_ICMP    ( 1 * 60 * 1000)  /*  1 minutes */

// IANA Port Ranges
#define NAT_WELL_KNOWN_PORT_MIN     0
#define NAT_WELL_KNOWN_PORT_MAX     1023
#define NAT_REGISTERED_PORT_MIN     1024
#define NAT_REGISTERED_PORT_MAX     49151
#define NAT_DYNAMIC_PORT_MIN        49152
#define NAT_DYNAMIC_PORT_MAX        65535

#define NAT_DEFAULT_PORT_MIN        10000
#define NAT_DEFAULT_PORT_MAX        20000

#define SNAT 0 
#define DNAT 1 

#define NAT_MAX_PRIVATE_NETWORKS    16    // maximum number of supported private networks


/**************************************
**
** NAT setsockopt 
*/


/* 
** NAT Timeout structure (for NAT configuration)
*/
typedef struct nat_timeouts_struct {
   uint32_t  timeout_tcp;
   uint32_t  timeout_fin;
   uint32_t  timeout_udp;
   uint32_t  timeout_icmp;     // NAT ICMP session timeout extension 
} nat_timeouts, NAT_TIMEOUTS_STRUCT, * NAT_TIMEOUTS_STRUCT_PTR;


/* 
** NAT Port structure (for NAT configuration)
*/
typedef struct nat_ports_struct {
   uint16_t  port_min;
   uint16_t  port_max;
} nat_ports, NAT_PORTS_STRUCT, * NAT_PORTS_STRUCT_PTR;


/* 
** NAT_STATS:
**    A structure used for statistics 
*/
typedef struct nat_stats {
   uint32_t  ST_SESSIONS;            /* total amount of sessions created to date */
   uint32_t  ST_SESSIONS_OPEN;       /* amount of sessions currently open */
   uint32_t  ST_SESSIONS_OPEN_MAX;   /* max amount of sessions simult. open */   
   uint32_t  ST_PACKETS_TOTAL;       /* amount of packets processed by NAT */
   uint32_t  ST_PACKETS_BYPASS;      /* amount of unmodified packets */
   uint32_t  ST_PACKETS_PUB_PRV;     /* packets from public to private */
   uint32_t  ST_PACKETS_PUB_PRV_ERR; /* packets from public to private with errs */
   uint32_t  ST_PACKETS_PRV_PUB;     /* packets from private to public */
   uint32_t  ST_PACKETS_PRV_PUB_ERR; /* packets from private to public with errs */

   uint32_t  ST_PACKETS_FRAGMENTS;
   uint32_t  ST_PACKETS_REASSEMBLED;
   uint32_t  ST_PACKETS_REASSSEMBLY_ERR;

   uint32_t  ST_SESSIONS_SNAT;       /* total amount of SNAT sessions created to date */
   uint32_t  ST_SESSIONS_DNAT;       /* total amount of DNAT sessions created to date */

} NAT_STATS, * NAT_STATS_PTR;


typedef struct nat_network_struct {
   uint32_t                 NUM_PRV_NETS;              /* Number of initialized private networks */
   _ip_address             PRV_NET[NAT_MAX_PRIVATE_NETWORKS];  /* List of private networks      */
   _ip_address             PRV_MASK[NAT_MAX_PRIVATE_NETWORKS]; /* Corresponding network masks   */
} NAT_NETWORK_STRUCT, * NAT_NETWORK_STRUCT_PTR;

typedef uint32_t (_CODE_PTR_ NAT_ALG)(void *);

/*
** NAT_alg_table:
**    NULL terminated table which specifies enabled ALGs 
*/

#define NAT_ALG_TFTP    NAT_ALG_tftp
#define NAT_ALG_FTP     NAT_ALG_ftp
#define NAT_ALG_ENDLIST NULL
extern NAT_ALG NAT_alg_table[];

/***************************************
**
** Function definitions
**
*/

#ifdef __cplusplus
extern "C" {
#endif
  
uint32_t SOL_NAT_getsockopt  (uint32_t, void *, uint32_t *);
uint32_t SOL_NAT_setsockopt  (uint32_t, const void *, uint32_t);

extern uint32_t NAT_init
(
   _ip_address,         /* [IN] private network IP */
   _ip_address          /* [IN] private network netmask */
);

extern uint32_t NAT_add_network
(
   _ip_address    private_network,
   _ip_address    private_netmask
);

extern uint32_t NAT_remove_network
(
   _ip_address    private_network,
   _ip_address    private_netmask
);

extern uint32_t NAT_close
(  
   void
);

extern NAT_STATS_PTR NAT_stats
(
   void
);

extern NAT_NETWORK_STRUCT_PTR NAT_networks
(
   void
);

extern void *NAT_find_next_session
(
   void    *session,
   uint32_t  tree
);

extern uint32_t NAT_ALG_tftp(void *);
extern uint32_t NAT_ALG_ftp(void *);

/***************************************
**
** Function definitions for DNAT
**
*/
extern uint32_t DNAT_add_rule
(
   uint32_t,       /* [IN] */    
   uint16_t,       /* [IN] */
   uint16_t,       /* [IN] */
   uint16_t,       /* [IN] */
   _ip_address,   /* [IN] */
   uint16_t        /* [IN] */
);

extern uint32_t DNAT_delete_rule
(
      uint32_t
);

extern uint32_t  DNAT_get_next_rule
(
   uint32_t *,    /* [IN/OUT] */   
   uint16_t *,    /* [OUT */
   uint16_t *,    /* [OUT] */
   uint16_t *,    /* [OUT] */
   _ip_address *,/* [OUT]   */
   uint16_t *     /* [OUT] */
);

#ifdef __cplusplus
}
#endif

#endif

/* EOF */
