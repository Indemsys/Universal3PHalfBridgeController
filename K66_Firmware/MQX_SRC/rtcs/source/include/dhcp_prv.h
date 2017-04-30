#ifndef __dhcp_prv_h__
#define __dhcp_prv_h__
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
*   Dynamic Host Configuration Protocol definitions.
*
*
*END************************************************************************/

#include "ip_prv.h"



/***************************************
**
** Constants
**
*/

#define DHCP_TIMEOUT_MIN       4000     /* 4 sec */
#define DHCP_TIMEOUT_MAX       64000    /* 64 sec */
#define DHCP_SAVED_ARRAY_SIZE  4
#define DHCP_SITE_SPEC_MIN     128
#define DHCP_SITE_SPEC_MAX     254

/* Internal DHCP Error codes - backward compatibility */

#define DHCP_PACKET_ERROR          RTCSERR_DHCP_PACKET_ERROR
#define DHCPCLNT_ERROR_DECLINED    RTCSERR_DHCPCLNT_ERROR_DECLINED
#define DHCPCLNT_XID_MISMATCH      RTCSERR_DHCPCLNT_XID_MISMATCH
#define DHCPCLNT_PACKET_SIZE_ERROR RTCSERR_DHCPCLNT_PACKET_SIZE_ERROR

/*
** This value can actually be 0x418937 before a rollover occurs, but the
** Diab compiler treats it as signed, so an overflow warning occurs unless
** you use 0x20C49B.
*/
#define DHCP_TCPIP_MAX_TIMED_EVENT_VALUE  0x20C49B

/*
** Standard port assignments
*/

#define DHCP_SERVER_PORT            67
#define DHCP_CLIENT_PORT            68


/* DHCP State definitions  */

#define DHCP_INITIALIZE    1
#define DHCP_BOUND         2
#define DHCP_RENEWING      3
#define DHCP_REBINDING     4
#define DHCP_REQUESTING    5
#define DHCP_DECLINING     6
#define DHCP_RELEASING     7
#define DHCP_INFORMED      8


/* 5 percent of total lease, there's no standard, use whatever's appropriate */
#define DHCP_RENEWALL_RANGE         0x14

/* The renewall and rebinding biases are suggested in RFC1541 */
#define DHCP_RENEWALL_BIAS(T)     (T/2)
#define DHCP_REBINDING_BIAS(T)    (T * 7/8)

/* Wait ten seconds before re-trying */
#define DHCP_TERMINATION_RETRY       10000
#define DHCP_OFFER_CHECK_EXPIRY_TIME 3000

/***************************************
**
** DHCP Macros
**
*/

#define DHCP_OPTION(type,len,val) \
            mqx_htonc(optptr, DHCPOPT_  ## type); optptr++; optlen++; \
            mqx_htonc(optptr, DHCPSIZE_ ## type); optptr++; optlen++; \
            hton ## len(optptr, val);                             \
            optptr += DHCPSIZE_ ## type;                          \
            optlen += DHCPSIZE_ ## type

#define DHCP_FILL(opt, cond) \
            opt_ptr = DHCP_find_option(parms->OPT_BUFF,                        \
                                       parms->OPT_BUFF_SIZE, DHCPOPT_ ## opt); \
            if (opt_ptr) { len = mqx_ntohc(opt_ptr + 1) + 2;                       \
               if (cond) { _mem_copy(opt_ptr, outp, len); outp += len;}        \
               for (i = 0; i < len; i++) {mqx_htonc(opt_ptr, 0); opt_ptr++;}}

#define DHCP_SET_RENEWALL_RANGE(l, r) \
            r = l / DHCP_RENEWALL_RANGE


#define DHCPCLNT_TIME_OVERFLOW_CHECK(r, t) if ( t > DHCP_TCPIP_MAX_TIMED_EVENT_VALUE ) { \
            t -= DHCP_TCPIP_MAX_TIMED_EVENT_VALUE;                                       \
            r = DHCP_TCPIP_MAX_TIMED_EVENT_VALUE * 1000;} else {                         \
            r = t * 1000;                                                                \
            t = 0;}

/***************************************
**
** Type definitions
**
*/

/***************************************
**
** Function-specific type definitions
**
*/

/* DHCP_open() */
typedef struct tcpip_parm_if_dhcp {

   TCPIP_PARM           COMMON;
   _rtcs_if_handle      HANDLE;
   uint32_t              FLAGS;

   unsigned char            *OPT_BUFF;
   uint32_t              OPT_BUFF_SIZE;
   DHCP_DATA_STRUCT_PTR OPTIONS;
   _ip_address          CLNT_IP_ADDR;
   _ip_address          CLNT_IP_MASK;
   _ip_address          SERVER_IP_ADDR;
   uint32_t              LEASE;
   uint32_t              TIMEOUT;

} TCPIP_PARM_IF_DHCP, * TCPIP_PARM_IF_DHCP_PTR;

/* DHCPCLNT_release() */
typedef struct tcpip_parm_if_dhcp_release {

   TCPIP_PARM           COMMON;
   _rtcs_if_handle      HANDLE;

} TCPIP_PARM_IF_DHCP_RELEASE, * TCPIP_PARM_IF_DHCP_RELEASE_PTR;

/* Kept for backwards compatibility, still use internally */
typedef struct dhcp_lease_struct 
{
   uint32_t     LEASE;
   _ip_address ServerIp;
   _ip_address ClientIPaddress;
   _ip_address Netmask;
   _ip_address SADDR;
   unsigned char       SNAME[64];
   unsigned char       BOOTFILE[128];

} DHCP_LEASE_STRUCT, * DHCP_LEASE_STRUCT_PTR;

typedef struct dhcp_stats {

   RTCS_STATS_STRUCT COMMON;

   RTCS_ERROR_STRUCT ERR_RX;
   RTCS_ERROR_STRUCT ERR_TX;

   uint32_t ST_TX_DISCOVER;
   uint32_t ST_TX_DECLINE;
   uint32_t ST_TX_REQUEST;
   uint32_t ST_RX_OFFER;
   uint32_t ST_RX_ACK;
   uint32_t ST_RX_NAK;

} DHCP_STATS, * DHCP_STATS_PTR;

typedef struct dhcp_clnt_struct  {

   IP_IF                 *HANDLE;  /* [IN] the RTCS interface state structure */
   UCB_STRUCT            *UCB;
   unsigned char                 *SERVERID_PTR;
   unsigned char                 *MSGTYPE_PTR;
   unsigned char                 *PACKET;
   unsigned char                 *REQUEST_OPT_PTR;
   DHCP_LEASE_STRUCT      DATA;    /* [IN/OUT] DHCP parameters for callback */
   DHCP_LEASE_STRUCT      NEW_DATA; /* [IN] DHCP parameters for initialization*/
   TCPIP_EVENT            RENEW;
   TCPIP_EVENT            RESEND;
   TCPIP_EVENT            TERMINATE;
   bool                BOUND;
   uint32_t                STATE;
   uint32_t                INIT_TIMEOUT;
   uint32_t                NEW_RENEW_TIME;
   uint32_t                RENEW_TIME;
   uint32_t                NEW_REBIND_TIME;
   uint32_t                REBIND_TIME;
   uint32_t                RETRY_TIMEOUT;
   uint32_t                TERMINATE_TIME;
   uint32_t                R_EVENTS_TIME_LEFT;
   uint32_t                S_EVENT_TIME_LEFT;
   uint32_t                SECS;
   uint32_t                TOTAL_PACKET_SIZE;
   uint32_t                CURRENT_PACKET_SIZE;
   uint32_t                FLAGS;
   int32_t                 (_CODE_PTR_ CHOICE_FUNC)(unsigned char *, uint32_t);
   void                   (_CODE_PTR_ BIND_FUNC)  (unsigned char *, uint32_t, _rtcs_if_handle);
   bool                (_CODE_PTR_ UNBIND_FUNC)(_rtcs_if_handle);
   bool                (_CODE_PTR_ FAILURE_FUNC)(_rtcs_if_handle);
   bool                (_CODE_PTR_ NAK_FUNC)  (unsigned char *, uint32_t, _rtcs_if_handle);
   DHCP_STATS             STATS;
   IPIF_PARM              PARMS_BIND;
   IPIF_PARM              NEW_PARMS_BIND;

} DHCP_CLNT_STRUCT, * DHCP_CLNT_STRUCT_PTR;

/***************************************
**
** Prototypes
**
*/

#ifdef __cplusplus
extern "C" {
#endif

void DHCPCLNT_init ( TCPIP_PARM_IF_DHCP *);
void DHCPCLNT_reinit ( TCPIP_PARM_IF_DHCP *);
void DHCPCLNT_service ( RTCSPCB *, UCB_STRUCT *);
bool DHCPCLNT_send ( TCPIP_EVENT *);

uint32_t DHCPCLNT_start ( TCPIP_PARM_IF_DHCP *, DHCP_CLNT_STRUCT *);
void DHCPCLNT_release_internal(TCPIP_PARM_IF_DHCP_RELEASE *);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
