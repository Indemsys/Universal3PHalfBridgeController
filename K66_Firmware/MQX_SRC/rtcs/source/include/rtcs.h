#ifndef __rtcs_h__
#define __rtcs_h__
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
*   structure definitions required by application
*   programs in order to use the RTCS Communication Library.
*
*
*END************************************************************************/

#include <rtcsvers.h>
#include <rtcsrtos.h>
#include <rtcslog.h>
#include <rtcs_in.h>
#include <string.h>

#include <rtcs_err.h>       /* RTCS Error Definitions. */
#include <rtcs_sock.h>      /* RTCS Socket API. */
#include <rtcs_if.h>        /* RTCS Network Interface API. */
#include <rtcs_stats.h>     /* RTCS Statistics API. */
#include <rtcs_hosts.h>     /* RTCS Hosts (file). */
#include <rtcs_assert.h>    /* RTCS Assert.*/
#include <rtcs_ssl.h>       /* RTCS SSL adapter.*/
#include <rtcs_util.h>      /* Common functions. */
#include <dnscln.h>         /* DNS Client API. */
#include <dhcpcln6.h>       /* DHCPv6 Client.*/
#include <addrinfo.h>
#include <llmnrsrv.h>       /* LLMNR Server.*/
#if PLATFORM_SDK_ENABLED
    #include <fsl_enet_rtcs_adapter.h>  /* ENET KSDK adaptter.*/
#else
    #include <ethernet.h>   /* ENET driver APU.*/
    #include <enet.h>       /* ENET driver APU.*/
#endif
#include <ipcfg.h>          /* LLMNR Server.*/

/* IP address string length */
#ifdef RTCSCFG_ENABLE_IP6
#define RTCS_IP_ADDR_STR_SIZE RTCS_IP6_ADDR_STR_SIZE
#else
#define RTCS_IP_ADDR_STR_SIZE RTCS_IP4_ADDR_STR_SIZE
#endif

/*
** Protocols for the RTCS_protocol_table[]
*/
#define RTCSPROT_IGMP   IGMP_init
#define RTCSPROT_UDP    UDP_init
#define RTCSPROT_TCP    TCP_Init
#define RTCSPROT_RIP    RIP_init
#define RTCSPROT_OSPF   OSPF_init
#define RTCSPROT_IPIP   IPIP_init

/*
** Protocol and Address families
*/
#define AF_UNSPEC       0
#define AF_INET         1
#define PF_INET         AF_INET

#define AF_INET6         2          /* IPv6 */
#define PF_INET6         AF_INET6   /* Same as the address family. */

/*
 * Flag values for getaddrinfo()
 */
#define	AI_PASSIVE	   0x00000001 /* get address to use bind() */
#define	AI_CANONNAME	0x00000002 /* fill ai_canonname */
#define	AI_NUMERICHOST	0x00000004 /* prevent host name resolution */

/*
** Flags for logging
*/
#define RTCS_LOGCTRL_FUNCTION    0x00000001
#define RTCS_LOGCTRL_PCB         0x00000002
#define RTCS_LOGCTRL_TIMER       0x00000004
#define RTCS_LOGCTRL_ALL         0x00000007

#define RTCS_LOGCTRL_IFTYPE(p)   ((p) & 0x3FF)
#define RTCS_LOGCTRL_ARP(p)      ((p) & 0x3FF | 0x4000)
#define RTCS_LOGCTRL_PROTO(p)    ((p) & 0x3FF | 0x8000)
#define RTCS_LOGCTRL_PORT(p)     ((p) & 0x3FF | 0xC000)

/*
** Modes for the FTP client
*/
#define FTPMODE_DEFAULT 1
#define FTPMODE_PORT    2
#define FTPMODE_PASV    3
#define FTPDIR_RECV     0
#define FTPDIR_SEND     4

#define RTCS_BOOTFILE_LEN       (128)
#define RTCS_SNAME_LEN          (64)
#define RTCS_BOOTOPT_LEN        (64)

#define MEM_TYPE_RTCS_BASE                      ( (IO_RTCS_COMPONENT) << (MEM_TYPE_COMPONENT_SHIFT))
#define MEM_TYPE_RTCS_DATA                      (MEM_TYPE_RTCS_BASE+1)
#define MEM_TYPE_ARP_CFG                        (MEM_TYPE_RTCS_BASE+2)
#define MEM_TYPE_ARP_ENTRY                      (MEM_TYPE_RTCS_BASE+3)
#define MEM_TYPE_DHCP_CLNT_STRUCT               (MEM_TYPE_RTCS_BASE+4)
#define MEM_TYPE_DHCPSRV_OPTIONS_STRUCT         (MEM_TYPE_RTCS_BASE+5)
#define MEM_TYPE_ICMP_CFG_STRUCT                (MEM_TYPE_RTCS_BASE+8)
#define MEM_TYPE_ICMP_DATA                      (MEM_TYPE_RTCS_BASE+9)
#define MEM_TYPE_IP_CFG_STRUCT                  (MEM_TYPE_RTCS_BASE+10)
#define MEM_TYPE_IP_DATA                        (MEM_TYPE_RTCS_BASE+11)
#define MEM_TYPE_ICB                            (MEM_TYPE_RTCS_BASE+12)
#define MEM_TYPE_IP_IF                          (MEM_TYPE_RTCS_BASE+13)
#define MEM_TYPE_SOCKET_CONFIG_STRUCT           (MEM_TYPE_RTCS_BASE+14)
#define MEM_TYPE_RTCS_PARTITION                 (MEM_TYPE_RTCS_BASE+15)
#define MEM_TYPE_IO_SOCKET                      (MEM_TYPE_RTCS_BASE+16)
#define MEM_TYPE_SOCKET_OWNER_STRUCT            (MEM_TYPE_RTCS_BASE+17)
#define MEM_TYPE_TCP_CFG_STRUCT                 (MEM_TYPE_RTCS_BASE+18)
#define MEM_TYPE_TCB                            (MEM_TYPE_RTCS_BASE+19)
#define MEM_TYPE_TCP_TX_WINDOW                  (MEM_TYPE_RTCS_BASE+20)
#define MEM_TYPE_TCP_RX_WINDOW                  (MEM_TYPE_RTCS_BASE+21)
#define MEM_TYPE_TCP_SEND_CLOCK                 (MEM_TYPE_RTCS_BASE+22)
#define MEM_TYPE_UDP_CFG_STRUCT                 (MEM_TYPE_RTCS_BASE+23)
#define MEM_TYPE_UCB_STRUCT                     (MEM_TYPE_RTCS_BASE+24)
#define MEM_TYPE_UDP_TX_BUFFER                  (MEM_TYPE_RTCS_BASE+25)
#define MEM_TYPE_IO_TELNET                      (MEM_TYPE_RTCS_BASE+26)
#define MEM_TYPE_SbufNode                       (MEM_TYPE_RTCS_BASE+27)
#define MEM_TYPE_Rchunk                         (MEM_TYPE_RTCS_BASE+28)
#define MEM_TYPE_FTPc_CONTEXT_STRUCT            (MEM_TYPE_RTCS_BASE+29)
#define MEM_TYPE_DHCPSRV_ADDR_STRUCT            (MEM_TYPE_RTCS_BASE+30)
#define MEM_TYPE_DHCPSRV_CID                    (MEM_TYPE_RTCS_BASE+31)
#define MEM_TYPE_DNS_CONTROL_STRUCT             (MEM_TYPE_RTCS_BASE+32)
#define MEM_TYPE_DNS_UDP_MESSAGE                (MEM_TYPE_RTCS_BASE+33)
#define MEM_TYPE_FTPc_RX_BUFFER                 (MEM_TYPE_RTCS_BASE+34)
#define MEM_TYPE_FTPc_COMMAND_BUFFER            (MEM_TYPE_RTCS_BASE+35)

#define MEM_TYPE_TFTPSRV_SESSION_BUFFER         (MEM_TYPE_RTCS_BASE+36)
#define MEM_TYPE_TFTPSRV_SERVER_STRUCT          (MEM_TYPE_RTCS_BASE+39)

#define MEM_TYPE_HTTPSRV_STRUCT                 (MEM_TYPE_RTCS_BASE+40)
#define MEM_TYPE_HTTPSRV_PARAMS                 (MEM_TYPE_RTCS_BASE+41)
#define MEM_TYPE_HTTPSRV_SESSION_STRUCT         (MEM_TYPE_RTCS_BASE+42)
#define MEM_TYPE_HTTPSRV_URI                    (MEM_TYPE_RTCS_BASE+43)
#define MEM_TYPE_HTTPSRV_AUTH                   (MEM_TYPE_RTCS_BASE+44)

#define MEM_TYPE_ICMP6_CFG_STRUCT               (MEM_TYPE_RTCS_BASE+45)
#define MEM_TYPE_ND6_CFG                        (MEM_TYPE_RTCS_BASE+46)
#define MEM_TYPE_IP6_CFG_STRUCT                 (MEM_TYPE_RTCS_BASE+47)
#define MEM_TYPE_FTPSRV_STRUCT                  (MEM_TYPE_RTCS_BASE+48)
#define MEM_TYPE_ECHOSRV_DATA_BUFFER            (MEM_TYPE_RTCS_BASE+49)
#define MEM_TYPE_TFTPSRV_SESSION_STRUCT         (MEM_TYPE_RTCS_BASE+50)
#define MEM_TYPE_IO_SOCKET_STRUCT               (MEM_TYPE_RTCS_BASE+51)

#define MEM_TYPE_TELNETSRV_STRUCT               (MEM_TYPE_RTCS_BASE+52)
#define MEM_TYPE_TELNETSRV_PARAMS               (MEM_TYPE_RTCS_BASE+53)
#define MEM_TYPE_TELNETSRV_SESSION_STRUCT       (MEM_TYPE_RTCS_BASE+54)
#define MEM_TYPE_DHCPCLN6_CONTEXT_STRUCT        (MEM_TYPE_RTCS_BASE+55)
#define MEM_TYPE_DHCPCLN6_MESSAGE_STRUCT        (MEM_TYPE_RTCS_BASE+56)

#define MEM_TYPE_UDP_RX_BUFFER                  (MEM_TYPE_RTCS_BASE+57)


/* Helpful macros.*/
#define IPBYTES(a)            (((a)>>24)&0xFF),(((a)>>16)&0xFF),(((a)>> 8)&0xFF),((a)&0xFF)
#define IPADDR(a,b,c,d)       ((((uint32_t)(a)&0xFF)<<24)|(((uint32_t)(b)&0xFF)<<16)|(((uint32_t)(c)&0xFF)<<8)|((uint32_t)(d)&0xFF))

/*
** Configurable globals
*/
extern uint32_t _RTCSTASK_priority;     /* TCP/IP task priority  */
extern uint32_t _RTCSTASK_stacksize;    /* additional stack size */

extern uint32_t _RTCSQUEUE_base;        /* RTCS queue numbers to use  */
extern uint32_t _RTCSPCB_init;          /* maximum RTCSPCBs available */
extern uint32_t _RTCSPCB_grow;          /* maximum RTCSPCBs available */
extern uint32_t _RTCSPCB_max;           /* maximum RTCSPCBs available */

extern bool _IP_forward;            /* IP forwarding */

extern bool _TCP_bypass_rx;         /* TCP checksum bypass (recv) */
extern bool _TCP_bypass_tx;         /* TCP checksum bypass (send) */
extern uint32_t _TCP_rto_min;           /* minimum TCP resend timeout */
extern bool _RTCS_initialized;      /* RTCS initialized */
extern bool _DHCP_broadcast;        /* DHCP broadcast or unicast offers */

extern _mem_pool_id _RTCS_mem_pool;

extern uint32_t _RTCS_msgpool_init;
extern uint32_t _RTCS_msgpool_grow;
extern uint32_t _RTCS_msgpool_max;

extern uint32_t _RTCS_socket_part_init;
extern uint32_t _RTCS_socket_part_grow;
extern uint32_t _RTCS_socket_part_max;

extern uint32_t _RTCS_dhcp_term_timeout;

/*
** The protocol table
*/
extern uint32_t (_CODE_PTR_ const RTCS_protocol_table[])(void);

/*
** Required when booting with BOOTP
*/
typedef struct bootp_data_struct {
   _ip_address SADDR;
#if RTCSCFG_BOOTP_RETURN_YIADDR
   _ip_address CLIENTADDR;
#endif
   unsigned char       SNAME[RTCS_SNAME_LEN];
   unsigned char       BOOTFILE[RTCS_BOOTFILE_LEN];
   unsigned char       OPTIONS[RTCS_BOOTOPT_LEN];
} BOOTP_DATA_STRUCT, * BOOTP_DATA_STRUCT_PTR;

/*
** Required when using PPP
*/
typedef struct ipcp_data_struct {
   void (_CODE_PTR_  IP_UP)   (void *);
   void (_CODE_PTR_  IP_DOWN) (void *);
   void             *IP_PARAM;

   unsigned ACCEPT_LOCAL_ADDR  : 1;
   unsigned ACCEPT_REMOTE_ADDR : 1;
   unsigned DEFAULT_NETMASK    : 1; /* obsolete */
   unsigned DEFAULT_ROUTE      : 1;
   unsigned NEG_LOCAL_DNS      : 1;
   unsigned NEG_REMOTE_DNS     : 1;
   unsigned ACCEPT_LOCAL_DNS   : 1; /* ignored if NEG_LOCAL_DNS  == 0 */
   unsigned ACCEPT_REMOTE_DNS  : 1; /* ignored if NEG_REMOTE_DNS == 0 */
   unsigned                    : 0;

   _ip_address LOCAL_ADDR;
   _ip_address REMOTE_ADDR;
   _ip_address NETMASK;     /* obsolete */
   _ip_address LOCAL_DNS;   /* ignored if NEG_LOCAL_DNS   == 0 */
   _ip_address REMOTE_DNS;  /* ignored if NEG_REMOTE_DNS  == 0 */

} IPCP_DATA_STRUCT, * IPCP_DATA_STRUCT_PTR;


/*
** Initialization for the DHCP server
*/
typedef struct dhcpsrv_data_struct {
   _ip_address      SERVERID;
   uint32_t         LEASE;
   _ip_address      MASK;
   _ip_address      SADDR;
   unsigned char    SNAME[RTCS_SNAME_LEN];
   unsigned char    FILE[RTCS_BOOTFILE_LEN];
} DHCPSRV_DATA_STRUCT, * DHCPSRV_DATA_STRUCT_PTR;

/*
** Structure to synchronize between applications and
** the TCP/IP task.
*/
typedef struct tcpip_parm {
  _task_id      SYNC;      /* for synchronization with application */
   uint32_t     ERROR;      /* [OUT] error code */
} TCPIP_PARM, * TCPIP_PARM_PTR;

#ifdef __cplusplus
extern "C" {
#endif

/*
** Prototypes for the RTCS_protocol_table[]
*/

extern uint32_t IGMP_init  (void);
extern uint32_t UDP_init   (void);
extern uint32_t TCP_Init   (void);
extern uint32_t RIP_init   (void);
extern uint32_t OSPF_init  (void);
extern uint32_t IPIP_init  (void);

/*
** BSD prototypes
*/

extern int32_t      inet_aton (const char *, in_addr *);
extern _ip_address  inet_addr (const char *);

extern uint32_t DHCPSRV_init        (char *, uint32_t, uint32_t);
extern uint32_t SNTP_init           (char *, uint32_t, uint32_t, _ip_address, uint32_t);
extern uint32_t SNMP_init           (char *, uint32_t, uint32_t);

extern uint32_t SNMP_init_with_traps (char *, uint32_t, uint32_t, _ip_address *);

extern uint32_t EDS_init           (char *, uint32_t, uint32_t);

extern uint32_t EDS_stop           (void);
extern uint32_t SNMP_stop          (void);

extern void    RTCS_delay (uint32_t);

/* TBD move to separate file.*/
typedef struct ping_param_struct 
{
    sockaddr            addr;               /* [IN][OUT] Remote socket address to ping. */
    uint32_t            timeout;            /* [IN] Maximum time to wait for reply, in milliseconds. */
    uint16_t            id;                 /* [IN] ICMP identifier for the ping request (optional). */ 
    uint8_t             hop_limit;          /* [IN] IPv4 Time To Live (TTL) or IPv6 Hop Limit (optional). */ 
    void                *data_buffer;       /* [IN] Pointer to ping data buffer, sent as payload of the ICMP request (optional). */
    uint32_t            data_buffer_size;   /* [IN] Size of the ping data buffer (optional).  */ 
    uint32_t            round_trip_time;    /* [OUT] Round trip time, in milliseconds. */
}PING_PARAM_STRUCT, * PING_PARAM_STRUCT_PTR;


extern uint32_t RTCS_ping  (PING_PARAM_STRUCT_PTR params);
extern uint32_t SNTP_oneshot     (_ip_address, uint32_t);
extern void MIB1213_init (void);
extern void MIBMQX_init  (void);
extern void EDS_task          (uint32_t);

extern uint32_t RTCS_if_bind_BOOTP
(
   _rtcs_if_handle            ,
   BOOTP_DATA_STRUCT_PTR
);
extern uint32_t RTCS_if_bind_IPCP
(
   _rtcs_if_handle            ,
   IPCP_DATA_STRUCT_PTR
);

/*
** RTCS initialization prototypes
*/

extern uint32_t RTCS_create
(
   void
);

extern uint32_t RTCS_gate_add
(
   _ip_address      gateway  ,  /* [IN] the IP address of the gateway */
   _ip_address      network  ,  /* [IN] the IP (sub)network to route */
   _ip_address      netmask     /* [IN] the IP (sub)network mask to route */
);

extern uint32_t RTCS_gate_add_metric
(
   _ip_address      gateway  ,  /* [IN] the IP address of the gateway */
   _ip_address      network  ,  /* [IN] the IP (sub)network to route */
   _ip_address      netmask  ,  /* [IN] the IP (sub)network mask to route */
   uint16_t         metric      /* [IN] the metric of the gateway */
);

extern uint32_t RTCS_gate_remove
(
   _ip_address      gateway  ,  /* [IN] the IP address of the gateway */
   _ip_address      network  ,  /* [IN] the IP (sub)network to route */
   _ip_address      netmask     /* [IN] the IP (sub)network mask to route */
);

extern uint32_t RTCS_gate_remove_metric
(
   _ip_address      gateway  ,  /* [IN] the IP address of the gateway */
   _ip_address      network  ,  /* [IN] the IP (sub)network to route */
   _ip_address      netmask  ,  /* [IN] the IP (sub)network mask to route */
   uint16_t         metric      /* [IN] the metric of the gateway */
);

extern _ip_address RTCS_get_route
(
   _ip_address netaddr, /* [IN] the network address */
   _ip_address netmask  /* [IN] the network mask */
);
extern void RTCS_walk_route( void );

extern uint32_t RTCS_trap_target_add
(
   _ip_address       ip_addr     /* [IN] the IP address to send the trap to */
);

extern uint32_t RTCS_trap_target_remove
(
   _ip_address       ip_addr     /* [IN] removes this IP address from the trap list */
);


extern _ip_address  IPCP_get_local_addr( _rtcs_if_handle );
extern _ip_address  IPCP_get_peer_addr( _rtcs_if_handle );


extern bool RTCS_resolve_ip_address( char *arg, _ip_address  *ipaddr_ptr, char *ipname, uint32_t ipnamesize);


extern void  RTCS_log_error
(
   uint32_t  module,
   uint32_t  error,
   uint32_t  p1,
   uint32_t  p2,
   uint32_t  p3
);


#include <dhcp.h>

/*
** The DHCP server
*/

#define DHCPSRV_option_int8     DHCP_option_int8
#define DHCPSRV_option_int16    DHCP_option_int16
#define DHCPSRV_option_int32    DHCP_option_int32
#define DHCPSRV_option_addr     DHCP_option_addr
#define DHCPSRV_option_addrlist DHCP_option_addrlist
#define DHCPSRV_option_string   DHCP_option_string

extern uint32_t DHCPSRV_ippool_add
(
   _ip_address              ipstart,
   uint32_t                 ipnum,
   DHCPSRV_DATA_STRUCT_PTR  params,
   unsigned char            *optptr,
   uint32_t                 optlen
);

/*
** The IPIP device
*/

extern uint32_t RTCS_tunnel_add
(
   _ip_address  inner_source_addr,      /* [IN] Inner IP header source        */
   _ip_address  inner_source_netmask,   /* [IN] Inner IP header source mask   */
   _ip_address  inner_dest_addr,        /* [IN] Inner IP header dest          */
   _ip_address  inner_dest_netmask,     /* [IN] Inner IP header dest mask     */
   _ip_address  outer_source_addr,      /* [IN] Outer IP header source        */
   _ip_address  outer_dest_addr,        /* [IN] Outer IP header dest          */
   uint32_t flags                       /* [IN] Flags to set tunnel behaviour */
);

extern uint32_t RTCS_tunnel_remove
(
   _ip_address  inner_source_addr,      /* [IN] Inner IP header source        */
   _ip_address  inner_source_netmask,   /* [IN] Inner IP header source mask   */
   _ip_address  inner_dest_addr,        /* [IN] Inner IP header dest          */
   _ip_address  inner_dest_netmask,     /* [IN] Inner IP header dest mask     */
   _ip_address  outer_source_addr,      /* [IN] Outer IP header source        */
   _ip_address  outer_dest_addr,        /* [IN] Outer IP header dest          */
   uint32_t flags                       /* [IN] Flags to set tunnel behaviour */
);

/*
** IPv6 address definition (BSD-like)
*/
typedef struct addrinfo {
	uint16_t	    ai_flags	        ;	/* AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST */
	uint16_t  	    ai_family			;	/* AF_INET, AF_INET6						*/
	uint32_t	    ai_socktype			;	/* SOCK_STREAM, SOCK_DGRAM					*/
	uint16_t        ai_protocol			;	/* 0 or IPPROTO_xxx for IPv4 and IPv6 		*/
	unsigned int 	ai_addrlen			;	/* length of ai_addr 						*/
	char	        *ai_canonname		;	/* canonical name for hostname 				*/
	struct		    sockaddr *ai_addr	;	/* binary address 							*/
	struct		    addrinfo *ai_next	;	/* next structure in linked list 			*/
} addrinfo;

/* user api functions */

extern int32_t inet_pton
(
    int32_t         af, 	    /* af - address family (AF_INET or AF_INET6)				    */
    const char 	    *src, 	    /* textual presentation of an address,example:"192.0.2.12" 	    */
    void 		    *dst,       /* pointer to a in_addr or in6-addr binary form of an address   */
    unsigned int    sizeof_dst  /* size of dst buffer                                           */
);

extern char *inet_ntop
(
	int32_t 	    af, 		/* af - address family (AF_INET or AF_INET6)				*/
	const void 		*src, 		/* pointer to a binary form of an address					*/
	char 			*dst, 		/* pointer to a textual presentation of an address			*/
	socklen_t	    size		/* max. size of the textual presentation of an address		*/
);

extern unsigned int strlcpy
(
	char 				   *dst, 		/* pointer to destination buffer 					*/
	const char 			   *src, 		/* pointer to source string		 					*/
	unsigned int	       siz			/* size of the destination buffer					*/
);

extern void freeaddrinfo
(
	struct addrinfo 	   *ai			/* addr info structure 								*/
);

extern int32_t getnameinfo
(
	const struct sockaddr   *sa		,	/* socket address structure 						*/
	unsigned int            salen	, 	/* size of socket address structure 				*/
	char 				    *host	,	/* pointer to host name 							*/
	unsigned int            hostlen	, 	/* size of host 									*/
	char       			    *serv	, 	/* pointer to service 								*/
	unsigned int            servlen	, 	/* size of serv 									*/
	int                     flags		/* flags 											*/
	/*
        NI_NUMERICHOST Return the address in numeric form.
	*/
);

extern int32_t getaddrinfo
(
	const char 				*hostname, 	/* host name or IP or NULL							*/
	const char 				*servname,	/* service name or port number						*/
	const struct addrinfo 	*hints,		/* set flags										*/
	struct addrinfo 		**res		/* [OUT]list of address structures					*/
);

extern uint32_t RTCS_set_primary_dns_suffix(char *dns_suffix);
extern const char * RTCS_get_primary_dns_suffix(void);

#ifdef __cplusplus
}
#endif

#include <httpsrv.h>
#include <echosrv.h>

#endif
