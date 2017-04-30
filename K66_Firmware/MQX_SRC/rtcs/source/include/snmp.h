#ifndef __snmp_h__
#define __snmp_h__
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
*   This file contains definitions for using the SNMP
*   agent.
*
*
*END************************************************************************/


/***************************************
**
** Constants
**
*/
#include <rtcs.h>

/*
** SNMP error status values
*/

#define SNMP_ERROR_noError              0
#define SNMP_ERROR_tooBig               1
#define SNMP_ERROR_noSuchName           2
#define SNMP_ERROR_badValue             3
#define SNMP_ERROR_readOnly             4
#define SNMP_ERROR_genErr               5

#define SNMP_ERROR_noAccess             6
#define SNMP_ERROR_wrongType            7
#define SNMP_ERROR_wrongLength          8
#define SNMP_ERROR_wrongEncoding        9
#define SNMP_ERROR_wrongValue          10
#define SNMP_ERROR_noCreation          11
#define SNMP_ERROR_inconsistentValue   12
#define SNMP_ERROR_resourceUnavailable 13
#define SNMP_ERROR_commitFailed        14
#define SNMP_ERROR_undoFailed          15
#define SNMP_ERROR_authorizationError  16
#define SNMP_ERROR_notWritable         17
#define SNMP_ERROR_inconsistentName    18

/* decide whether an error-status uses the error-index */
#define SNMP_ERROR_USEINDEX(n)         ((n) > 1)

/* not real error codes -- used internally */
#define SNMP_ERROR_PARSE               0x7F
#define SNMP_ERROR_noSuchObject        ASN1_TYPE_ERR_NOOBJECT
#define SNMP_ERROR_noSuchInstance      ASN1_TYPE_ERR_NOINST
#define SNMP_ERROR_endOfMibView        ASN1_TYPE_ERR_ENDOFMIB

/*
** MIB access types
*/
#define RTCSMIB_ACCESS_READ   0x01
#define RTCSMIB_ACCESS_WRITE  0x02

/*
** MIB node types
*/
#define RTCSMIB_NODETYPE_INT_CONST     1
#define RTCSMIB_NODETYPE_INT_PTR       2
#define RTCSMIB_NODETYPE_INT_FN        3
#define RTCSMIB_NODETYPE_UINT_CONST    4
#define RTCSMIB_NODETYPE_UINT_PTR      5
#define RTCSMIB_NODETYPE_UINT_FN       6
#define RTCSMIB_NODETYPE_DISPSTR_PTR   7
#define RTCSMIB_NODETYPE_DISPSTR_FN    8
#define RTCSMIB_NODETYPE_OCTSTR_FN     9
#define RTCSMIB_NODETYPE_OID_PTR       10
#define RTCSMIB_NODETYPE_OID_FN        11

#if SNMPCFG_HAVE_INT64
#define RTCSMIB_NODETYPE_INT64_CONST   12
#define RTCSMIB_NODETYPE_INT64_PTR     13
#define RTCSMIB_NODETYPE_INT64_FN      14
#define RTCSMIB_NODETYPE_UINT64_CONST  15
#define RTCSMIB_NODETYPE_UINT64_PTR    16
#define RTCSMIB_NODETYPE_UINT64_FN     17
#endif

/*
** Used internally by the MIB instance parser
*/
#define RTCSMIB_OP_GET              0
#define RTCSMIB_OP_GETNEXT          1
#define RTCSMIB_OP_GETFIRST         2
#define RTCSMIB_OP_FINDNEXT         3
#define RTCSMIB_OP_SET              4

#if RTCSCFG_ENABLE_SNMP_STATS
#define IF_SNMP_STATS_ENABLED(x) x
#else
#define IF_SNMP_STATS_ENABLED(x)
#endif


/***************************************
**
** Type definitions
**
*/

/*
** SNMP statistics for the SNMP MIB
*/

#if RTCSCFG_ENABLE_SNMP_STATS
typedef struct snmp_stats {
   RTCS_STATS_STRUCT COMMON;
   
   /* These are all included in ST_RX_DISCARDED */
   uint32_t  ST_RX_BAD_PARSE;
   uint32_t  ST_RX_BAD_VERSION;
   uint32_t  ST_RX_BAD_COMMUNITY;
   uint32_t  ST_RX_BAD_PDU;

   /* Breakdown of all packets received and sent */
   uint32_t  ST_RX_VAR_GETS;
   uint32_t  ST_RX_VAR_SETS;

   uint32_t  ST_RX_GETREQ;
   uint32_t  ST_RX_GETNEXTREQ;
   uint32_t  ST_RX_RESPONSE;
   uint32_t  ST_RX_SETREQ;
   uint32_t  ST_RX_TRAP;
   uint32_t  ST_RX_GETBULKREQ;
   uint32_t  ST_RX_INFORM;
   uint32_t  ST_RX_V2TRAP;
   uint32_t  ST_RX_REPORT;
   uint32_t  ST_RX_OTHER;

   uint32_t  ST_TX_GETREQ;
   uint32_t  ST_TX_GETNEXTREQ;
   uint32_t  ST_TX_RESPONSE;
   uint32_t  ST_TX_SETREQ;
   uint32_t  ST_TX_TRAP;
   uint32_t  ST_TX_GETBULKREQ;
   uint32_t  ST_TX_INFORM;
   uint32_t  ST_TX_V2TRAP;
   uint32_t  ST_TX_REPORT;
   uint32_t  ST_TX_OTHER;

   uint32_t  ST_TX_TOOBIG;
   uint32_t  ST_TX_NOSUCHNAME;
   uint32_t  ST_TX_BADVALUE;
   uint32_t  ST_TX_READONLY;
   uint32_t  ST_TX_GENERR;
   uint32_t  ST_TX_OTHERERR;

} SNMP_STATS, * SNMP_STATS_PTR;
#endif
/*
** Internal state for the SNMP agent
*/

typedef struct snmp_parse {
#if RTCSCFG_ENABLE_SNMP_STATS
   SNMP_STATS  STATS;
#endif
   uint32_t     trapsock;
   char    *community[SNMPCFG_NUM_COMMUNITY];
   uint32_t     communitylen[SNMPCFG_NUM_COMMUNITY];
   uint32_t     currentcommunity;
   _ip_address trap_receiver_list[SNMPCFG_MAX_TRAP_RECEIVERS];

   unsigned char   *inbuf;
   uint32_t     inlen;
   unsigned char   *outbuf;
   uint32_t     outlen;

   uint32_t     errstat;
   uint32_t     errindex;
   unsigned char   *errstatp;
   unsigned char   *errindexp;

   uint32_t     version;
   uint32_t     pdutype;
   uint32_t     nonrep;
   uint32_t     reps;
} SNMP_PARSE, * SNMP_PARSE_PTR;

/*
** Internal state for the MIB walker
*/

typedef struct rtcsmib_walk {
   TCPIP_PARM  COMMON;

   uint8_t      pdutype;

   unsigned char   *inbuf;
   uint32_t     inlen;
   unsigned char   *outbuf;
   uint32_t     outlen;

   uint32_t     errstat;

   unsigned char   *oidp;
   uint32_t     oidlen;
   uint32_t     totlen;

} RTCSMIB_WALK, * RTCSMIB_WALK_PTR;

/* Interface to pass messages to trap server */
typedef struct trap_parm {
   TCPIP_PARM           COMMON;
   _ip_address          address;
} TRAP_PARM, * TRAP_PARM_PTR;

/*
** Structure of a single MIB node
*/

typedef struct rtcsmib_node {
   uint32_t                    ID;

   struct rtcsmib_node       *NEXT;
   struct rtcsmib_node       *CHILD;
   struct rtcsmib_node       *PARENT;

   uint32_t                    ACCESS;
   bool (_CODE_PTR_        FIND)(uint32_t, void *, void **);
   bool (_CODE_PTR_        PARSE)(RTCSMIB_WALK_PTR, uint32_t, bool (_CODE_PTR_)(uint32_t, void *, void **), bool *, void **);
   uint32_t                    ASN1_TYPE;

   struct rtcsmib_value      *GET;      

   uint32_t (_CODE_PTR_        SET)(void *, unsigned char *, uint32_t);

} RTCSMIB_NODE, * RTCSMIB_NODE_PTR;

typedef struct rtcsmib_value
{
    uint32_t     TYPE; /* ignored in nonleaf nodes */
    void       *PARAM;
} RTCSMIB_VALUE, * RTCSMIB_VALUE_PTR ;

/* callback functions to provide MIB values dynamically */
typedef uint32_t   (*RTCSMIB_UINT_FN_PTR)(void *);
typedef int32_t    (*RTCSMIB_INT_FN_PTR)(void *);
typedef unsigned char *(*RTCSMIB_DISPSTR_FN_PTR)(void *);
typedef unsigned char *(*RTCSMIB_OCTSTR_FN_PTR)(void *, uint32_t *);
typedef RTCSMIB_NODE_PTR (*RTCSMIB_OID_FN_PTR)(void *);    


/***************************************
**
** Prototypes
**
*/

/*
** Used by SNMP to make MIB requests:
*/

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t RTCSMIB_request
(
   uint8_t         pdutype,
   unsigned char      *invarp,
   uint32_t        invarlen,
   unsigned char      *outvarp,
   uint32_t        outvarlen,
   uint32_t       *writelen
);

/*
** Utilities for MIB implementations:
*/

extern void RTCSMIB_mib_add
(
   RTCSMIB_NODE_PTR  mib
);

extern int32_t RTCSMIB_int_read    (unsigned char *, uint32_t);
extern bool RTCSMIB_id_read8   (RTCSMIB_WALK_PTR, unsigned char *);
extern bool RTCSMIB_id_read32  (RTCSMIB_WALK_PTR, uint32_t *);
extern bool RTCSMIB_id_write8  (RTCSMIB_WALK_PTR, unsigned char);
extern bool RTCSMIB_id_write32 (RTCSMIB_WALK_PTR, uint32_t);

extern bool MIB_instance_zero
(
   RTCSMIB_WALK_PTR     mib,
   uint32_t              op,
   bool (_CODE_PTR_  find)(uint32_t, void *, void **),
   bool         *found,
   void               **instance
);

extern void       RTCS_insert_trap_receiver_internal(TRAP_PARM *);
extern void       RTCS_remove_trap_receiver_internal(TRAP_PARM *);
extern _mqx_uint  RTCS_count_trap_receivers_internal(void);

extern void SNMP_trap_warmStart(void);
extern void SNMP_trap_coldStart(void);
extern void SNMP_trap_authenticationFailure(void);
extern void SNMP_trap_userSpec(RTCSMIB_NODE_PTR trap_node , uint32_t spec_trap, RTCSMIB_NODE_PTR enterprises );
extern void SNMPv2_trap_warmStart(void);
extern void SNMPv2_trap_coldStart(void);
extern void SNMPv2_trap_authenticationFailure(void);
extern void SNMPv2_trap_userSpec(RTCSMIB_NODE_PTR trap_node);
extern bool SNMP_trap_select_community(char *);
extern void SNMP_trap_linkDown(void *);
extern void SNMP_trap_myLinkDown(void *);
extern void SNMP_trap_linkUp(void *);
extern void SNMPv2_trap_linkDown(void *);
extern void SNMPv2_trap_linkUp(void *);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
