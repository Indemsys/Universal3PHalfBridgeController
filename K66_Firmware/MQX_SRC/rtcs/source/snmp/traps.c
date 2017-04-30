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
*   
*
*
*END************************************************************************/
#include <string.h>
#include <rtcsrtos.h>
#include <rtcs.h>
#include <rtcs_prv.h>
#include "tcpip.h"
#include "ip_prv.h"
#include "snmpcfg.h"
#include "asn1.h"
#include "snmp.h"

#if RTCSCFG_ENABLE_SNMP && RTCSCFG_ENABLE_IP4

/* This node is defined in mib.c */
extern RTCSMIB_NODE MIBNODE_enterprises;
extern RTCSMIB_NODE MIBNODE_snmpModules;

/* These nodes are defined as part of MIB-II (RFC 1213) and is found in 1213.c */
extern RTCSMIB_NODE MIBNODE_snmp;
extern RTCSMIB_NODE MIBNODE_sysUpTime;
extern RTCSMIB_NODE MIBNODE_ifIndex;

/* The following are defined in this file */
extern RTCSMIB_NODE MIBNODE_snmpTrapEnterprise;
extern RTCSMIB_NODE MIBNODE_snmpMIBObjects;
extern RTCSMIB_NODE MIBNODE_snmpTrapOID;
extern RTCSMIB_NODE MIBNODE_snmpTraps;
extern RTCSMIB_NODE MIBNODE_snmpTrap;
extern RTCSMIB_NODE MIBNODE_snmpMIB;

/* functions from 1213.c */
extern bool MIB_find_ifEntry(uint32_t, void *, void **);
extern uint32_t MIB_get_ifIndex (void *interface);
extern uint32_t MIB_get_sysUpTime(void *);

RTCSMIB_VALUE MIBVALUE_snmpTrapEnterprise = {
    RTCSMIB_NODETYPE_INT_CONST,
    NULL
};

RTCSMIB_VALUE MIBVALUE_snmpMIBObjects = {
    0,
    NULL
};

RTCSMIB_VALUE MIBVALUE_snmpTrapOID = {
    RTCSMIB_NODETYPE_INT_CONST,
    NULL
};

RTCSMIB_VALUE MIBVALUE_snmpTraps = {
    0,
    NULL
};

RTCSMIB_VALUE MIBVALUE_snmpTrap = {
    0,
    NULL
};

RTCSMIB_VALUE MIBVALUE_snmpMIB = {
    0,
    NULL
};

RTCSMIB_NODE MIBNODE_snmpTrapEnterprise = {
   3,

   NULL,
   NULL,
   (RTCSMIB_NODE_PTR)&MIBNODE_snmpTrap,

   0,
   NULL,
   MIB_instance_zero, ASN1_TYPE_OBJECT,
   (RTCSMIB_VALUE_PTR)&MIBVALUE_snmpTrapEnterprise,
   NULL
};

RTCSMIB_NODE MIBNODE_snmpMIBObjects = {
   1,

   NULL,
   (RTCSMIB_NODE_PTR)&MIBNODE_snmpTrap,
   (RTCSMIB_NODE_PTR)&MIBNODE_snmpMIB,

   0,
   NULL,
   NULL, 0, 
   (RTCSMIB_VALUE_PTR)&MIBVALUE_snmpMIBObjects, 
   NULL
};

RTCSMIB_NODE MIBNODE_snmpTrapOID = {
   1,

   (RTCSMIB_NODE_PTR)&MIBNODE_snmpTrapEnterprise,
   NULL,
   (RTCSMIB_NODE_PTR)&MIBNODE_snmpTrap,

   0,
   NULL,
   MIB_instance_zero, ASN1_TYPE_OBJECT,
   (RTCSMIB_VALUE_PTR)&MIBVALUE_snmpTrapOID,
   NULL
};

RTCSMIB_NODE MIBNODE_snmpTraps = {
   5,

   NULL,
   NULL,
   (RTCSMIB_NODE_PTR)&MIBNODE_snmpMIBObjects,

   0,
   NULL,
   NULL, 0, 
   (RTCSMIB_VALUE_PTR)&MIBVALUE_snmpTraps, 
   NULL
};

RTCSMIB_NODE MIBNODE_snmpTrap = {
   4,

   (RTCSMIB_NODE_PTR)&MIBNODE_snmpTraps,
   (RTCSMIB_NODE_PTR)&MIBNODE_snmpTrapOID,
   (RTCSMIB_NODE_PTR)&MIBNODE_snmpMIBObjects,

   0,
   NULL,
   NULL, 0, 
   (RTCSMIB_VALUE_PTR)&MIBVALUE_snmpTrap, 
   NULL
};

RTCSMIB_NODE MIBNODE_snmpMIB = {
   1,

   NULL,
   (RTCSMIB_NODE_PTR)&MIBNODE_snmpMIBObjects,
   (RTCSMIB_NODE_PTR)&MIBNODE_snmpModules,

   0,
   NULL,
   NULL, 0, 
   (RTCSMIB_VALUE_PTR)&MIBVALUE_snmpMIB, 
   NULL
};


bool SNMP_trap_select_community
   (
      char   *community
   )
{ /* Body */
   SNMP_PARSE_PTR   snmpcfg = SNMP_get_data();
   uint32_t          i;
   bool          result = FALSE;
   
   /* Check to see SNMP is initialized */
   if(snmpcfg == NULL)
      return (result);

   for(i=0; i < SNMPCFG_NUM_COMMUNITY; i++) {
      if(!strcmp(community, snmpcfg->community[i]))
      {
         snmpcfg->currentcommunity = i;
         result = TRUE;
         break;
      } /* Endif */               
   } /* Endfor */
        
   return (result);
        
} /* Endbody */


void SNMP_trap_warmStart
   (
      void
   )
{ /* Body */
   SNMP_PARSE_PTR    snmpcfg = SNMP_get_data();
   uint32_t           traplen, oidlen;
   RTCSMIB_NODE_PTR  node;
   uint32_t           i;
   unsigned char         *ipsrcp;
   _ip_address       ipsrc, ipaddr;
   sockaddr_in       addr;

   unsigned char outbuf[SNMPCFG_BUFFER_SIZE];
   struct {
      uint32_t     outlen;
      unsigned char   *outbuf;
   } snmp;

   traplen = 0;
   snmp.outlen = sizeof(outbuf);
   snmp.outbuf = outbuf + sizeof(outbuf);

   /* complete variable-bindings */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);
   /* time-stamp */
   i = MIB_get_sysUpTime(NULL);
   ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, traplen, ASN1_TYPE_TimeTicks, i);
   /* specific-trap */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);
   /* generic-trap */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 1);
   /* skip agent-addr */
   traplen     += 4;
   snmp.outlen -= 4;
   snmp.outbuf -= 4;
   ipsrcp = snmp.outbuf;
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_IpAddress, 4);

   /* enterprise */
   node = &MIBNODE_snmp;
   oidlen = 0;
   while (node) {
      ASN1_BKWRITE_ID(&snmp, oidlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, oidlen, ASN1_TYPE_OBJECT, oidlen);
   traplen += oidlen;

   /* complete Trap-PDU */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_PDU_TRAP, traplen);
   /* community */
   ASN1_BKWRITE_TYPELEN_BYTES(&snmp, traplen, ASN1_TYPE_OCTET, 
                              snmpcfg->community[snmpcfg->currentcommunity], 
                              snmpcfg->communitylen[snmpcfg->currentcommunity]);   
   /* version, 0=v1 */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);
   /* complete Message */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);

   addr.sin_family = AF_INET;
   addr.sin_port   = IPPORT_SNMPTRAP;
   for (i = 0; i < SNMPCFG_MAX_TRAP_RECEIVERS; i++) {
      if (snmpcfg->trap_receiver_list[i] != 0) {
         ipaddr = snmpcfg->trap_receiver_list[i];
         ipsrc = IP_route_find(ipaddr, 0);
         (void) mqx_htonl(ipsrcp, ipsrc);
         addr.sin_addr.s_addr = ipaddr;
         sendto(snmpcfg->trapsock, snmp.outbuf, traplen, 0, (sockaddr*)&addr, sizeof(addr));
      } /* Endif */
   } /* Endfor */

} /* Endbody */


void SNMP_trap_coldStart
   (
      void
   )
{ /* Body */
   SNMP_PARSE_PTR    snmpcfg = SNMP_get_data();
   uint32_t           traplen, oidlen;
   RTCSMIB_NODE_PTR  node;
   uint32_t           i;
   unsigned char         *ipsrcp;
   _ip_address       ipsrc, ipaddr;
   sockaddr_in       addr;

   unsigned char outbuf[SNMPCFG_BUFFER_SIZE];
   struct {
      uint32_t     outlen;
      unsigned char   *outbuf;
   } snmp;

   traplen = 0;
   snmp.outlen = sizeof(outbuf);
   snmp.outbuf = outbuf + sizeof(outbuf);

   /* complete variable-bindings */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);
   /* time-stamp */
   i = MIB_get_sysUpTime(NULL);
   ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, traplen, ASN1_TYPE_TimeTicks, i);
   /* specific-trap */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);
   /* generic-trap */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);
   /* skip agent-addr */
   traplen     += 4;
   snmp.outlen -= 4;
   snmp.outbuf -= 4;
   ipsrcp = snmp.outbuf;
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_IpAddress, 4);

   /* enterprise */
   node = &MIBNODE_snmp;
   oidlen = 0;
   while (node) {
      ASN1_BKWRITE_ID(&snmp, oidlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, oidlen, ASN1_TYPE_OBJECT, oidlen);
   traplen += oidlen;

   /* complete Trap-PDU */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_PDU_TRAP, traplen);
   /* community */  
   ASN1_BKWRITE_TYPELEN_BYTES(&snmp, traplen, ASN1_TYPE_OCTET, 
                              snmpcfg->community[snmpcfg->currentcommunity], 
                              snmpcfg->communitylen[snmpcfg->currentcommunity]);      
   /* version, 0=v1 */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);
   /* complete Message */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);

   addr.sin_family = AF_INET;
   addr.sin_port   = IPPORT_SNMPTRAP;
   for (i = 0; i < SNMPCFG_MAX_TRAP_RECEIVERS; i++) {
      if (snmpcfg->trap_receiver_list[i] != 0) {
         ipaddr = snmpcfg->trap_receiver_list[i];
         ipsrc = IP_route_find(ipaddr, 0);
         (void) mqx_htonl(ipsrcp, ipsrc);
         addr.sin_addr.s_addr = ipaddr;
         sendto(snmpcfg->trapsock, snmp.outbuf, traplen, 0, (sockaddr*)&addr, sizeof(addr));
      } /* Endif */
   } /* Endfor */

} /* Endbody */


void SNMP_trap_authenticationFailure
   (
      void
   )
{ /* Body */
   SNMP_PARSE_PTR    snmpcfg = SNMP_get_data();
   uint32_t           traplen, oidlen;
   RTCSMIB_NODE_PTR  node;
   uint32_t           i;
   unsigned char         *ipsrcp;
   _ip_address       ipsrc, ipaddr;
   sockaddr_in       addr;

   unsigned char outbuf[SNMPCFG_BUFFER_SIZE];
   struct {
      uint32_t     outlen;
      unsigned char   *outbuf;
   } snmp;

   traplen = 0;
   snmp.outlen = sizeof(outbuf);
   snmp.outbuf = outbuf + sizeof(outbuf);

   /* complete variable-bindings */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);
   /* time-stamp */
   i = MIB_get_sysUpTime(NULL);
   ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, traplen, ASN1_TYPE_TimeTicks, i);
   /* specific-trap */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);
   /* generic-trap */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 4);
   /* skip agent-addr */
   traplen     += 4;
   snmp.outlen -= 4;
   snmp.outbuf -= 4;
   ipsrcp = snmp.outbuf;
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_IpAddress, 4);

   /* enterprise */
   node = &MIBNODE_snmp;
   oidlen = 0;
   while (node) {
      ASN1_BKWRITE_ID(&snmp, oidlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, oidlen, ASN1_TYPE_OBJECT, oidlen);
   traplen += oidlen;

   /* complete Trap-PDU */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_PDU_TRAP, traplen);
   /* community */
   ASN1_BKWRITE_TYPELEN_BYTES(&snmp, traplen, ASN1_TYPE_OCTET, 
                              snmpcfg->community[snmpcfg->currentcommunity], 
                              snmpcfg->communitylen[snmpcfg->currentcommunity]);   
   /* version, 0=v1 */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);
   /* complete Message */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);

   addr.sin_family = AF_INET;
   addr.sin_port   = IPPORT_SNMPTRAP;
   for (i = 0; i < SNMPCFG_MAX_TRAP_RECEIVERS; i++) {
      if (snmpcfg->trap_receiver_list[i] != 0) {
         ipaddr = snmpcfg->trap_receiver_list[i];
         ipsrc = IP_route_find(ipaddr, 0);
         (void) mqx_htonl(ipsrcp, ipsrc);
         addr.sin_addr.s_addr = ipaddr;
         sendto(snmpcfg->trapsock, snmp.outbuf, traplen, 0, (sockaddr*)&addr, sizeof(addr));
      } /* Endif */
   } /* Endfor */

} /* Endbody */

void SNMP_trap_linkDown
   (
      void    *ihandle
   )
{ /* Body */
   SNMP_PARSE_PTR    snmpcfg = SNMP_get_data();
   uint32_t           traplen, oidlen, varlen;
   RTCSMIB_NODE_PTR  node = NULL;
   void             *instance;
   uint32_t           i, temp;
   unsigned char         *ipsrcp;
   _ip_address       ipsrc, ipaddr;
   sockaddr_in       addr;
   
   RTCSMIB_VALUE_PTR mib_val;

   unsigned char outbuf[SNMPCFG_BUFFER_SIZE];
   struct {
      uint32_t     outlen;
      unsigned char   *outbuf;
   } snmp;

   traplen = 0;
   snmp.outlen = sizeof(outbuf);
   snmp.outbuf = outbuf + sizeof(outbuf);

   /* VarBind for ifIndex */

   varlen = 0;
   node = &MIBNODE_ifIndex;
   mib_val = (void *)node->GET;
   temp = MIB_get_ifIndex(ihandle);
   MIB_find_ifEntry(RTCSMIB_OP_GET, &temp, &instance);

   { /* Scope */
      int32_t   sinteger = 0;
      uint32_t  uinteger = 0;
#if SNMPCFG_HAVE_INT64
      int64_t   sinteger64 = 0;
      uint64_t  uinteger64 = 0;
#endif

    switch (mib_val->TYPE) {
    case RTCSMIB_NODETYPE_INT_CONST:
        sinteger = (int32_t)(mib_val->PARAM);
        break; 
    case RTCSMIB_NODETYPE_INT_PTR:   
        sinteger = *(int32_t *)(mib_val->PARAM);
        break;
    case RTCSMIB_NODETYPE_INT_FN: 
        sinteger = ((RTCSMIB_INT_FN_PTR)mib_val->PARAM)(instance);      
        break;
    case RTCSMIB_NODETYPE_UINT_CONST:
        uinteger = (uint32_t)(mib_val->PARAM); 
        break; 
    case RTCSMIB_NODETYPE_UINT_PTR: 
        uinteger = *(uint32_t *)(mib_val->PARAM); 
        break;
    case RTCSMIB_NODETYPE_UINT_FN:      
        uinteger = ((RTCSMIB_UINT_FN_PTR)mib_val->PARAM)(instance);
        break;
#if SNMPCFG_HAVE_INT64
      case RTCSMIB_NODETYPE_INT64_CONST:  sinteger64 = node->GET.INT64_CONST; break;
      case RTCSMIB_NODETYPE_INT64_PTR:    sinteger64 = *node->GET.INT64_PTR; break;
      case RTCSMIB_NODETYPE_INT64_FN:     sinteger64 = node->GET.INT64_FN(instance); break;
      case RTCSMIB_NODETYPE_UINT64_CONST: uinteger64 = node->GET.UINT64_CONST; break;
      case RTCSMIB_NODETYPE_UINT64_PTR:   uinteger64 = *node->GET.UINT64_PTR; break;
      case RTCSMIB_NODETYPE_UINT64_FN:    uinteger64 = node->GET.UINT64_FN(instance); break;
#endif
      } /* Endswitch */

      switch (mib_val->TYPE) {
      case RTCSMIB_NODETYPE_INT_CONST:
      case RTCSMIB_NODETYPE_INT_PTR:
      case RTCSMIB_NODETYPE_INT_FN:
         ASN1_BKWRITE_TYPELEN_BIGINT(&snmp, varlen, node->ASN1_TYPE, sinteger);
         break;
      case RTCSMIB_NODETYPE_UINT_CONST:
      case RTCSMIB_NODETYPE_UINT_PTR:
      case RTCSMIB_NODETYPE_UINT_FN:
         ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, node->ASN1_TYPE, uinteger);
         break;
#if SNMPCFG_HAVE_INT64
      case RTCSMIB_NODETYPE_INT64_CONST:
      case RTCSMIB_NODETYPE_INT64_PTR:
      case RTCSMIB_NODETYPE_INT64_FN:
         ASN1_BKWRITE_TYPELEN_BIGINT(&snmp, varlen, node->ASN1_TYPE, sinteger64);
         break;
      case RTCSMIB_NODETYPE_UINT64_CONST:
      case RTCSMIB_NODETYPE_UINT64_PTR:
      case RTCSMIB_NODETYPE_UINT64_FN:
         ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, node->ASN1_TYPE, uinteger64);
         break;
#endif
      } /* Endswitch */
   } /* Endscope */

   oidlen = 0;

   { /* Scope */
      struct {
         uint32_t ifIndex;
      }      *index = (void *)&temp;
      ASN1_BKWRITE_ID(&snmp, oidlen, index->ifIndex);
   } /* Endscope */
   while (node) {
      ASN1_BKWRITE_ID(&snmp, oidlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, oidlen, ASN1_TYPE_OBJECT, oidlen);
   traplen += oidlen + varlen;
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, oidlen+varlen);

   /* complete variable-bindings */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);
   /* time-stamp */
   i = MIB_get_sysUpTime(NULL);
   ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, traplen, ASN1_TYPE_TimeTicks, i);
   /* specific-trap */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);
   /* generic-trap */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 2);
   /* skip agent-addr */
   traplen     += 4;
   snmp.outlen -= 4;
   snmp.outbuf -= 4;
   ipsrcp = snmp.outbuf;
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_IpAddress, 4);

   /* enterprise */
   node = &MIBNODE_snmp;
   oidlen = 0;
   while (node) {
      ASN1_BKWRITE_ID(&snmp, oidlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, oidlen, ASN1_TYPE_OBJECT, oidlen);
   traplen += oidlen;

   /* complete Trap-PDU */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_PDU_TRAP, traplen);
   /* community */
   ASN1_BKWRITE_TYPELEN_BYTES(&snmp, traplen, ASN1_TYPE_OCTET, 
                              snmpcfg->community[snmpcfg->currentcommunity], 
                              snmpcfg->communitylen[snmpcfg->currentcommunity]);   
   /* version, 0=v1 */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);
   /* complete Message */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);

   addr.sin_family = AF_INET;
   addr.sin_port   = IPPORT_SNMPTRAP;
   for (i=0; i<SNMPCFG_MAX_TRAP_RECEIVERS; i++) {
      if (snmpcfg->trap_receiver_list[i] != 0) {
         ipaddr = snmpcfg->trap_receiver_list[i];
         ipsrc = IP_route_find(ipaddr, 0);
         (void) mqx_htonl(ipsrcp, ipsrc);
         addr.sin_addr.s_addr = ipaddr;
         sendto(snmpcfg->trapsock, snmp.outbuf, traplen, 0, (sockaddr*)&addr, sizeof(addr));
      } /* Endif */
   } /* Endfor */

} /* Endbody */


void SNMP_trap_myLinkDown
   (
     void    *ihandle
   )
{ /* Body */
   SNMP_PARSE_PTR    snmpcfg = SNMP_get_data();
   uint32_t           traplen, oidlen, varlen;
   RTCSMIB_NODE_PTR  node;
   void             *instance;
   uint32_t           i, temp;
   unsigned char         *ipsrcp;
   _ip_address       ipsrc, ipaddr;
   sockaddr_in       addr;

   unsigned char outbuf[SNMPCFG_BUFFER_SIZE];
   struct {
      uint32_t     outlen;
      unsigned char   *outbuf;
   } snmp;

   RTCSMIB_VALUE_PTR    mib_val;    
   traplen = 0;
   snmp.outlen = sizeof(outbuf);
   snmp.outbuf = outbuf + sizeof(outbuf);

   /* VarBind for ifIndex */
   varlen = 0;
   node = &MIBNODE_ifIndex;
   mib_val = (void *)node->GET;
   temp = MIB_get_ifIndex(ihandle);
   MIB_find_ifEntry(RTCSMIB_OP_GET, &temp, &instance);

   { /* Scope */
      int32_t   sinteger = 0;
      uint32_t  uinteger = 0;
#if SNMPCFG_HAVE_INT64
      int64_t   sinteger64 = 0;
      uint64_t  uinteger64 = 0;
#endif

    switch (mib_val->TYPE) {
    case RTCSMIB_NODETYPE_INT_CONST:
        sinteger = (int32_t)(mib_val->PARAM); 
        break;    
    case RTCSMIB_NODETYPE_INT_PTR:       
        sinteger = *(int32_t *)(mib_val->PARAM); 
        break;
    case RTCSMIB_NODETYPE_INT_FN:       
        sinteger = ((RTCSMIB_INT_FN_PTR)mib_val->PARAM)(instance);
        break;
    case RTCSMIB_NODETYPE_UINT_CONST:
        uinteger = (uint32_t)(mib_val->PARAM);
        break;
    case RTCSMIB_NODETYPE_UINT_PTR:   
        uinteger = *(uint32_t *)(mib_val->PARAM);
        break;
    case RTCSMIB_NODETYPE_UINT_FN:      
        uinteger = ((RTCSMIB_UINT_FN_PTR)mib_val->PARAM)(instance);
        break;
#if SNMPCFG_HAVE_INT64
      case RTCSMIB_NODETYPE_INT64_CONST:  sinteger64 = node->GET.INT64_CONST; break;
      case RTCSMIB_NODETYPE_INT64_PTR:    sinteger64 = *node->GET.INT64_PTR; break;
      case RTCSMIB_NODETYPE_INT64_FN:     sinteger64 = node->GET.INT64_FN(instance); break;
      case RTCSMIB_NODETYPE_UINT64_CONST: uinteger64 = node->GET.UINT64_CONST; break;
      case RTCSMIB_NODETYPE_UINT64_PTR:   uinteger64 = *node->GET.UINT64_PTR; break;
      case RTCSMIB_NODETYPE_UINT64_FN:    uinteger64 = node->GET.UINT64_FN(instance); break;
#endif
      } /* Endswitch */

      switch (mib_val->TYPE) {
      case RTCSMIB_NODETYPE_INT_CONST:
      case RTCSMIB_NODETYPE_INT_PTR:
      case RTCSMIB_NODETYPE_INT_FN:
         ASN1_BKWRITE_TYPELEN_BIGINT(&snmp, varlen, node->ASN1_TYPE, sinteger);
         break;
      case RTCSMIB_NODETYPE_UINT_CONST:
      case RTCSMIB_NODETYPE_UINT_PTR:
      case RTCSMIB_NODETYPE_UINT_FN:
         ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, node->ASN1_TYPE, uinteger);
         break;
#if SNMPCFG_HAVE_INT64
      case RTCSMIB_NODETYPE_INT64_CONST:
      case RTCSMIB_NODETYPE_INT64_PTR:
      case RTCSMIB_NODETYPE_INT64_FN:
         ASN1_BKWRITE_TYPELEN_BIGINT(&snmp, varlen, node->ASN1_TYPE, sinteger64);
         break;
      case RTCSMIB_NODETYPE_UINT64_CONST:
      case RTCSMIB_NODETYPE_UINT64_PTR:
      case RTCSMIB_NODETYPE_UINT64_FN:
         ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, node->ASN1_TYPE, uinteger64);
         break;
#endif
      } /* Endswitch */
   } /* Endscope */

   oidlen = 0;

   { /* Scope */
      struct {
         uint32_t ifIndex;
      }      *index = (void *)&temp;//index_ifIndex;
      ASN1_BKWRITE_ID(&snmp, oidlen, index->ifIndex);
   } /* Endscope */
   while (node) {
      ASN1_BKWRITE_ID(&snmp, oidlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, oidlen, ASN1_TYPE_OBJECT, oidlen);
   traplen += oidlen + varlen;
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, oidlen+varlen);

   /* complete variable-bindings */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);
   /* time-stamp */
   i = MIB_get_sysUpTime(NULL);
   ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, traplen, ASN1_TYPE_TimeTicks, i);
   /* specific-trap */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 2);
   /* generic-trap */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 6);
   /* skip agent-addr */
   traplen     += 4;
   snmp.outlen -= 4;
   snmp.outbuf -= 4;
   ipsrcp = snmp.outbuf;
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_IpAddress, 4);

   /* enterprise */
   node = &MIBNODE_snmpTrapEnterprise;
   oidlen = 0;
   while (node) {
      ASN1_BKWRITE_ID(&snmp, oidlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, oidlen, ASN1_TYPE_OBJECT, oidlen);
   traplen += oidlen;

   /* complete Trap-PDU */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_PDU_TRAP, traplen);
   /* community */
   ASN1_BKWRITE_TYPELEN_BYTES(&snmp, traplen, ASN1_TYPE_OCTET, 
                              snmpcfg->community[snmpcfg->currentcommunity], 
                              snmpcfg->communitylen[snmpcfg->currentcommunity]);   
   /* version, 0=v1 */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);
   /* complete Message */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);

   addr.sin_family = AF_INET;
   addr.sin_port   = IPPORT_SNMPTRAP;
   for (i=0; i<SNMPCFG_MAX_TRAP_RECEIVERS; i++) {
      if (snmpcfg->trap_receiver_list[i] != 0) {
         ipaddr = snmpcfg->trap_receiver_list[i];
         ipsrc = IP_route_find(ipaddr, 0);
         (void) mqx_htonl(ipsrcp, ipsrc);
         addr.sin_addr.s_addr = ipaddr;
         sendto(snmpcfg->trapsock, snmp.outbuf, traplen, 0, (sockaddr*)&addr, sizeof(addr));
      } /* Endif */
   } /* Endfor */

} /* Endbody */


void SNMP_trap_linkUp
   (
      void   *ihandle
   )
{ /* Body */
   SNMP_PARSE_PTR    snmpcfg = SNMP_get_data();
   uint32_t           traplen, oidlen, varlen;
   RTCSMIB_NODE_PTR  node;
   void             *instance;
   uint32_t           i, temp;
   unsigned char         *ipsrcp;
   _ip_address       ipsrc, ipaddr;
   sockaddr_in       addr;
   RTCSMIB_VALUE_PTR mib_val;

   unsigned char outbuf[SNMPCFG_BUFFER_SIZE];
   struct {
      uint32_t     outlen;
      unsigned char   *outbuf;
   } snmp;

   traplen = 0;
   snmp.outlen = sizeof(outbuf);
   snmp.outbuf = outbuf + sizeof(outbuf);

   /* VarBind for ifIndex */
   varlen = 0;
   node = &MIBNODE_ifIndex;
   mib_val = (void *)node->GET;
   temp = MIB_get_ifIndex(ihandle);
   MIB_find_ifEntry(RTCSMIB_OP_GET, &temp, &instance);

   { /* Scope */
      int32_t   sinteger = 0;
      uint32_t  uinteger = 0;
#if SNMPCFG_HAVE_INT64
      int64_t   sinteger64 = 0;
      uint64_t  uinteger64 = 0;
#endif

    switch (mib_val->TYPE) {
    case RTCSMIB_NODETYPE_INT_CONST:
        sinteger = (int32_t)(mib_val->TYPE); 
        break;
    case RTCSMIB_NODETYPE_INT_PTR:    
        sinteger = *(int32_t *)(mib_val->TYPE); 
        break;
    case RTCSMIB_NODETYPE_INT_FN: 
        sinteger = ((RTCSMIB_INT_FN_PTR)mib_val->PARAM)(instance);      
        break;
    case RTCSMIB_NODETYPE_UINT_CONST:
        uinteger = (uint32_t)(mib_val->TYPE); 
        break; 
    case RTCSMIB_NODETYPE_UINT_PTR:  
        uinteger = *(uint32_t *)(mib_val->TYPE); 
        break;
    case RTCSMIB_NODETYPE_UINT_FN: 
        uinteger = ((RTCSMIB_UINT_FN_PTR)mib_val->PARAM)(instance);    
        break;
#if SNMPCFG_HAVE_INT64
      case RTCSMIB_NODETYPE_INT64_CONST:  sinteger64 = node->GET.INT64_CONST; break;
      case RTCSMIB_NODETYPE_INT64_PTR:    sinteger64 = *node->GET.INT64_PTR; break;
      case RTCSMIB_NODETYPE_INT64_FN:     sinteger64 = node->GET.INT64_FN(instance); break;
      case RTCSMIB_NODETYPE_UINT64_CONST: uinteger64 = node->GET.UINT64_CONST; break;
      case RTCSMIB_NODETYPE_UINT64_PTR:   uinteger64 = *node->GET.UINT64_PTR; break;
      case RTCSMIB_NODETYPE_UINT64_FN:    uinteger64 = node->GET.UINT64_FN(instance); break;
#endif
      } /* Endswitch */

      switch (mib_val->TYPE) {
      case RTCSMIB_NODETYPE_INT_CONST:
      case RTCSMIB_NODETYPE_INT_PTR:
      case RTCSMIB_NODETYPE_INT_FN:
         ASN1_BKWRITE_TYPELEN_BIGINT(&snmp, varlen, node->ASN1_TYPE, sinteger);
         break;
      case RTCSMIB_NODETYPE_UINT_CONST:
      case RTCSMIB_NODETYPE_UINT_PTR:
      case RTCSMIB_NODETYPE_UINT_FN:
         ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, node->ASN1_TYPE, uinteger);
         break;
#if SNMPCFG_HAVE_INT64
      case RTCSMIB_NODETYPE_INT64_CONST:
      case RTCSMIB_NODETYPE_INT64_PTR:
      case RTCSMIB_NODETYPE_INT64_FN:
         ASN1_BKWRITE_TYPELEN_BIGINT(&snmp, varlen, node->ASN1_TYPE, sinteger64);
         break;
      case RTCSMIB_NODETYPE_UINT64_CONST:
      case RTCSMIB_NODETYPE_UINT64_PTR:
      case RTCSMIB_NODETYPE_UINT64_FN:
         ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, node->ASN1_TYPE, uinteger64);
         break;
#endif
      } /* Endswitch */
   } /* Endscope */

   oidlen = 0;

   { /* Scope */
      struct {
         uint32_t ifIndex;
      }      *index = (void *)&temp;//index_ifIndex;
      ASN1_BKWRITE_ID(&snmp, oidlen, index->ifIndex);
   } /* Endscope */
   while (node) {
      ASN1_BKWRITE_ID(&snmp, oidlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, oidlen, ASN1_TYPE_OBJECT, oidlen);
   traplen += oidlen + varlen;
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, oidlen+varlen);

   /* complete variable-bindings */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);
   /* time-stamp */
   i = MIB_get_sysUpTime(NULL);
   ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, traplen, ASN1_TYPE_TimeTicks, i);
   /* specific-trap */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);
   /* generic-trap */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 3);
   /* skip agent-addr */
   traplen     += 4;
   snmp.outlen -= 4;
   snmp.outbuf -= 4;
   ipsrcp = snmp.outbuf;
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_IpAddress, 4);

   /* enterprise */
   node = &MIBNODE_snmp;
   oidlen = 0;
   while (node) {
      ASN1_BKWRITE_ID(&snmp, oidlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, oidlen, ASN1_TYPE_OBJECT, oidlen);
   traplen += oidlen;

   /* complete Trap-PDU */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_PDU_TRAP, traplen);
   /* community */
   ASN1_BKWRITE_TYPELEN_BYTES(&snmp, traplen, ASN1_TYPE_OCTET, 
                              snmpcfg->community[snmpcfg->currentcommunity], 
                              snmpcfg->communitylen[snmpcfg->currentcommunity]);   
   /* version, 0=v1 */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);
   /* complete Message */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);

   addr.sin_family = AF_INET;
   addr.sin_port   = IPPORT_SNMPTRAP;
   for (i=0; i<SNMPCFG_MAX_TRAP_RECEIVERS; i++) {
      if (snmpcfg->trap_receiver_list[i] != 0) {
         ipaddr = snmpcfg->trap_receiver_list[i];
         ipsrc = IP_route_find(ipaddr, 0);
         (void) mqx_htonl(ipsrcp, ipsrc);
         addr.sin_addr.s_addr = ipaddr;
         sendto(snmpcfg->trapsock, snmp.outbuf, traplen, 0, (sockaddr*)&addr, sizeof(addr));
      } /* Endif */
   } /* Endfor */

} /* Endbody */

void SNMPv2_trap_warmStart
   (
      void
   )
{ /* Body */
   SNMP_PARSE_PTR    snmpcfg = SNMP_get_data();
   uint32_t           traplen, oidlen, varlen;
   RTCSMIB_NODE_PTR  node;
   //void             *instance;
   uint32_t           i;
   sockaddr_in       addr;

   unsigned char outbuf[SNMPCFG_BUFFER_SIZE];
   struct {
      uint32_t     outlen;
      unsigned char   *outbuf;
   } snmp;

   traplen = 0;
   snmp.outlen = sizeof(outbuf);
   snmp.outbuf = outbuf + sizeof(outbuf);

   /* VarBind for snmpTrapOID */

   varlen = 0;
   node = &MIBNODE_snmpTraps;
   ASN1_BKWRITE_ID(&snmp, varlen, 2);
   while (node) {
      ASN1_BKWRITE_ID(&snmp, varlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, varlen, ASN1_TYPE_OBJECT, varlen);

   oidlen = 0;
   node = &MIBNODE_snmpTrapOID;
   ASN1_BKWRITE_ID(&snmp, oidlen, 0);
   while (node) {
      ASN1_BKWRITE_ID(&snmp, oidlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, oidlen, ASN1_TYPE_OBJECT, oidlen);

   traplen += oidlen + varlen;
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, oidlen+varlen);

   /* VarBind for sysUpTime */

   varlen = 0;
   i = MIB_get_sysUpTime(NULL);
   ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, ASN1_TYPE_TimeTicks, i);

   oidlen = 0;
   node = &MIBNODE_sysUpTime;
   ASN1_BKWRITE_ID(&snmp, oidlen, 0);
   while (node) {
      ASN1_BKWRITE_ID(&snmp, oidlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, oidlen, ASN1_TYPE_OBJECT, oidlen);

   traplen += oidlen + varlen;
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, oidlen+varlen);

   /* complete variable-bindings */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);
   /* error-index */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);
   /* error-status */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);
   /* request-id */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);

   /* complete SNMPv2-Trap-PDU */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_PDU_TRAPV2, traplen);
   /* community */
   ASN1_BKWRITE_TYPELEN_BYTES(&snmp, traplen, ASN1_TYPE_OCTET, 
                              snmpcfg->community[snmpcfg->currentcommunity], 
                              snmpcfg->communitylen[snmpcfg->currentcommunity]);   
   /* version, 1=v2 */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 1);
   /* complete Message */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);

   addr.sin_family = AF_INET;
   addr.sin_port   = IPPORT_SNMPTRAP;
   for (i = 0; i < SNMPCFG_MAX_TRAP_RECEIVERS; i++) {
      if (snmpcfg->trap_receiver_list[i] != 0) {
         addr.sin_addr.s_addr = snmpcfg->trap_receiver_list[i];
         sendto(snmpcfg->trapsock, snmp.outbuf, traplen, 0, (sockaddr*)&addr, sizeof(addr));
      } /* Endif */
   } /* Endfor */

} /* Endbody */


void SNMPv2_trap_coldStart
   (
      void
   )
{ /* Body */
   SNMP_PARSE_PTR    snmpcfg = SNMP_get_data();
   uint32_t           traplen, oidlen, varlen;
   RTCSMIB_NODE_PTR  node;
   //void             *instance;
   uint32_t           i;
   sockaddr_in       addr;

   unsigned char outbuf[SNMPCFG_BUFFER_SIZE];
   struct {
      uint32_t     outlen;
      unsigned char   *outbuf;
   } snmp;

   traplen = 0;
   snmp.outlen = sizeof(outbuf);
   snmp.outbuf = outbuf + sizeof(outbuf);

   /* VarBind for snmpTrapOID */

   varlen = 0;
   node = &MIBNODE_snmpTraps;
   ASN1_BKWRITE_ID(&snmp, varlen, 1);
   while (node) {
      ASN1_BKWRITE_ID(&snmp, varlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, varlen, ASN1_TYPE_OBJECT, varlen);

   oidlen = 0;
   node = &MIBNODE_snmpTrapOID;
   ASN1_BKWRITE_ID(&snmp, oidlen, 0);
   while (node) {
      ASN1_BKWRITE_ID(&snmp, oidlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, oidlen, ASN1_TYPE_OBJECT, oidlen);

   traplen += oidlen + varlen;
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, oidlen+varlen);

   /* VarBind for sysUpTime */

   varlen = 0;
   i = MIB_get_sysUpTime(NULL);
   ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, ASN1_TYPE_TimeTicks, i);

   oidlen = 0;
   node = &MIBNODE_sysUpTime;
   ASN1_BKWRITE_ID(&snmp, oidlen, 0);
   while (node) {
      ASN1_BKWRITE_ID(&snmp, oidlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, oidlen, ASN1_TYPE_OBJECT, oidlen);

   traplen += oidlen + varlen;
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, oidlen+varlen);

   /* complete variable-bindings */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);
   /* error-index */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);
   /* error-status */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);
   /* request-id */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);

   /* complete SNMPv2-Trap-PDU */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_PDU_TRAPV2, traplen);
   /* community */
   ASN1_BKWRITE_TYPELEN_BYTES(&snmp, traplen, ASN1_TYPE_OCTET, 
                              snmpcfg->community[snmpcfg->currentcommunity], 
                              snmpcfg->communitylen[snmpcfg->currentcommunity]);   
   /* version, 1=v2 */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 1);
   /* complete Message */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);

   addr.sin_family = AF_INET;
   addr.sin_port   = IPPORT_SNMPTRAP;
   for (i = 0; i < SNMPCFG_MAX_TRAP_RECEIVERS; i++) {
      if (snmpcfg->trap_receiver_list[i] != 0) {
         addr.sin_addr.s_addr = snmpcfg->trap_receiver_list[i];
         sendto(snmpcfg->trapsock, snmp.outbuf, traplen, 0, (sockaddr*)&addr, sizeof(addr));
      } /* Endif */
   } /* Endfor */

} /* Endbody */


void SNMPv2_trap_authenticationFailure
   (
      void
   )
{ /* Body */
   SNMP_PARSE_PTR    snmpcfg = SNMP_get_data();
   uint32_t           traplen, oidlen, varlen;
   RTCSMIB_NODE_PTR  node;
   //void             *instance;
   uint32_t           i;
   sockaddr_in       addr;

   unsigned char outbuf[SNMPCFG_BUFFER_SIZE];
   struct {
      uint32_t     outlen;
      unsigned char   *outbuf;
   } snmp;

   traplen = 0;
   snmp.outlen = sizeof(outbuf);
   snmp.outbuf = outbuf + sizeof(outbuf);

   /* VarBind for snmpTrapOID */

   varlen = 0;
   node = &MIBNODE_snmpTraps;
   ASN1_BKWRITE_ID(&snmp, varlen, 5);
   while (node) {
      ASN1_BKWRITE_ID(&snmp, varlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, varlen, ASN1_TYPE_OBJECT, varlen);

   oidlen = 0;
   node = &MIBNODE_snmpTrapOID;
   ASN1_BKWRITE_ID(&snmp, oidlen, 0);
   while (node) {
      ASN1_BKWRITE_ID(&snmp, oidlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, oidlen, ASN1_TYPE_OBJECT, oidlen);

   traplen += oidlen + varlen;
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, oidlen+varlen);

   /* VarBind for sysUpTime */

   varlen = 0;
   i = MIB_get_sysUpTime(NULL);
   ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, ASN1_TYPE_TimeTicks, i);

   oidlen = 0;
   node = &MIBNODE_sysUpTime;
   ASN1_BKWRITE_ID(&snmp, oidlen, 0);
   while (node) {
      ASN1_BKWRITE_ID(&snmp, oidlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, oidlen, ASN1_TYPE_OBJECT, oidlen);

   traplen += oidlen + varlen;
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, oidlen+varlen);

   /* complete variable-bindings */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);
   /* error-index */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);
   /* error-status */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);
   /* request-id */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);

   /* complete SNMPv2-Trap-PDU */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_PDU_TRAPV2, traplen);
   /* community */
   ASN1_BKWRITE_TYPELEN_BYTES(&snmp, traplen, ASN1_TYPE_OCTET, 
                              snmpcfg->community[snmpcfg->currentcommunity], 
                              snmpcfg->communitylen[snmpcfg->currentcommunity]);   
   /* version, 1=v2 */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 1);
   /* complete Message */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);

   addr.sin_family = AF_INET;
   addr.sin_port   = IPPORT_SNMPTRAP;
   for (i = 0; i < SNMPCFG_MAX_TRAP_RECEIVERS; i++) {
      if (snmpcfg->trap_receiver_list[i] != 0) {
         addr.sin_addr.s_addr = snmpcfg->trap_receiver_list[i];
         sendto(snmpcfg->trapsock, snmp.outbuf, traplen, 0, (sockaddr*)&addr, sizeof(addr));
      } /* Endif */
   } /* Endfor */

} /* Endbody */


void SNMPv2_trap_linkDown
   (
      void   *ihandle
   )
{ /* Body */
   SNMP_PARSE_PTR    snmpcfg = SNMP_get_data();
   uint32_t          traplen, oidlen, varlen;
   RTCSMIB_NODE_PTR  node;
   void              *instance;
   uint32_t          i, temp;
   sockaddr_in       addr;
   
   RTCSMIB_VALUE_PTR mib_val;

   unsigned char outbuf[SNMPCFG_BUFFER_SIZE];
   struct {
      uint32_t     outlen;
      unsigned char   *outbuf;
   } snmp;

   traplen = 0;
   snmp.outlen = sizeof(outbuf);
   snmp.outbuf = outbuf + sizeof(outbuf);

   /* VarBind for snmpTrapOID */
   
   varlen = 0;
   node = &MIBNODE_snmpTraps;
   ASN1_BKWRITE_ID(&snmp, varlen, 3);
   while (node) {
      ASN1_BKWRITE_ID(&snmp, varlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, varlen, ASN1_TYPE_OBJECT, varlen);

   oidlen = 0;
   node = &MIBNODE_snmpTrapOID;
   ASN1_BKWRITE_ID(&snmp, oidlen, 0);
   while (node) {
      ASN1_BKWRITE_ID(&snmp, oidlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, oidlen, ASN1_TYPE_OBJECT, oidlen);

   traplen += oidlen + varlen;
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, oidlen+varlen);

   /* VarBind for ifIndex */
   
   varlen = 0;
   node = &MIBNODE_ifIndex;
   mib_val = (void *)node->GET;
   temp = MIB_get_ifIndex(ihandle);
   MIB_find_ifEntry(RTCSMIB_OP_GET, &temp, &instance);

   { /* Scope */
      int32_t   sinteger = 0;
      uint32_t  uinteger = 0;
#if SNMPCFG_HAVE_INT64
      int64_t   sinteger64 = 0;
      uint64_t  uinteger64 = 0;
#endif

    switch (mib_val->TYPE) {
    case RTCSMIB_NODETYPE_INT_CONST:
        sinteger = (int32_t)(mib_val->TYPE);
        break; 
    case RTCSMIB_NODETYPE_INT_PTR:    
        sinteger = *(int32_t *)(mib_val->TYPE);
        break;
    case RTCSMIB_NODETYPE_INT_FN:       
        sinteger = ((RTCSMIB_INT_FN_PTR)mib_val->PARAM)(instance);  
        break;
    case RTCSMIB_NODETYPE_UINT_CONST:
        uinteger = (uint32_t)(mib_val->TYPE);
        break;   
    case RTCSMIB_NODETYPE_UINT_PTR:
        uinteger = *(uint32_t *)(mib_val->TYPE);
        break;
    case RTCSMIB_NODETYPE_UINT_FN:      
        uinteger = ((RTCSMIB_UINT_FN_PTR)mib_val->PARAM)(instance);
        break;
#if SNMPCFG_HAVE_INT64
      case RTCSMIB_NODETYPE_INT64_CONST:  sinteger64 = node->GET.INT64_CONST; break;
      case RTCSMIB_NODETYPE_INT64_PTR:    sinteger64 = *node->GET.INT64_PTR; break;
      case RTCSMIB_NODETYPE_INT64_FN:     sinteger64 = node->GET.INT64_FN(instance); break;
      case RTCSMIB_NODETYPE_UINT64_CONST: uinteger64 = node->GET.UINT64_CONST; break;
      case RTCSMIB_NODETYPE_UINT64_PTR:   uinteger64 = *node->GET.UINT64_PTR; break;
      case RTCSMIB_NODETYPE_UINT64_FN:    uinteger64 = node->GET.UINT64_FN(instance); break;
#endif
      } /* Endswitch */

      switch (mib_val->TYPE) {
      case RTCSMIB_NODETYPE_INT_CONST:
      case RTCSMIB_NODETYPE_INT_PTR:
      case RTCSMIB_NODETYPE_INT_FN:
         ASN1_BKWRITE_TYPELEN_BIGINT(&snmp, varlen, node->ASN1_TYPE, sinteger);
         break;
      case RTCSMIB_NODETYPE_UINT_CONST:
      case RTCSMIB_NODETYPE_UINT_PTR:
      case RTCSMIB_NODETYPE_UINT_FN:
         ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, node->ASN1_TYPE, uinteger);
         break;
#if SNMPCFG_HAVE_INT64
      case RTCSMIB_NODETYPE_INT64_CONST:
      case RTCSMIB_NODETYPE_INT64_PTR:
      case RTCSMIB_NODETYPE_INT64_FN:
         ASN1_BKWRITE_TYPELEN_BIGINT(&snmp, varlen, node->ASN1_TYPE, sinteger64);
         break;
      case RTCSMIB_NODETYPE_UINT64_CONST:
      case RTCSMIB_NODETYPE_UINT64_PTR:
      case RTCSMIB_NODETYPE_UINT64_FN:
         ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, node->ASN1_TYPE, uinteger64);
         break;
#endif
      } /* Endswitch */
   } /* Endscope */

   oidlen = 0;

   { /* Scope */
      struct {
         uint32_t ifIndex;
      }      *index = (void *)&temp;
      ASN1_BKWRITE_ID(&snmp, oidlen, index->ifIndex);
   } /* Endscope */
   
   while (node) {
      ASN1_BKWRITE_ID(&snmp, oidlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, oidlen, ASN1_TYPE_OBJECT, oidlen);

   traplen += oidlen + varlen;
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, oidlen+varlen);

   varlen = 0;
   /* time-stamp */
   i = MIB_get_sysUpTime(NULL);
   ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, ASN1_TYPE_TimeTicks, i);
   
   oidlen = 0;
   node = &MIBNODE_sysUpTime;
   ASN1_BKWRITE_ID(&snmp, oidlen, 0);
   while (node) {
      ASN1_BKWRITE_ID(&snmp, oidlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, oidlen, ASN1_TYPE_OBJECT, oidlen);

   traplen += oidlen + varlen;
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, oidlen+varlen);

   /* complete variable-bindings */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);

   /* error-index */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);
   /* error-status */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);
   /* request-id */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);

   /* complete SANMPv2-Trap-PDU */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_PDU_TRAPV2, traplen);

   /* community */
   ASN1_BKWRITE_TYPELEN_BYTES(&snmp, traplen, ASN1_TYPE_OCTET, 
                              snmpcfg->community[snmpcfg->currentcommunity], 
                              snmpcfg->communitylen[snmpcfg->currentcommunity]);   
   /* version, 1=v2 */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 1);
   /* complete Message */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);

   addr.sin_family = AF_INET;
   addr.sin_port   = IPPORT_SNMPTRAP;
   for (i=0; i<SNMPCFG_MAX_TRAP_RECEIVERS; i++) {
      if (snmpcfg->trap_receiver_list[i] != 0) {
         addr.sin_addr.s_addr = snmpcfg->trap_receiver_list[i];
         sendto(snmpcfg->trapsock, snmp.outbuf, traplen, 0, (sockaddr*)&addr, sizeof(addr));
      } /* Endif */
   } /* Endfor */

} /* Endbody */


void SNMPv2_trap_linkUp
   (
      void     *ihandle
   )
{ /* Body */
   SNMP_PARSE_PTR    snmpcfg = SNMP_get_data();
   uint32_t          traplen, oidlen, varlen;
   RTCSMIB_NODE_PTR  node;
   void              *instance;
   uint32_t          i, temp;
   sockaddr_in       addr;

   RTCSMIB_VALUE_PTR mib_val;
    
   unsigned char outbuf[SNMPCFG_BUFFER_SIZE];
   struct {
      uint32_t     outlen;
      unsigned char   *outbuf;
   } snmp;

   traplen = 0;
   snmp.outlen = sizeof(outbuf);
   snmp.outbuf = outbuf + sizeof(outbuf);

   /* VarBind for snmpTrapOID */

   varlen = 0;
   node = &MIBNODE_snmpTraps;
   ASN1_BKWRITE_ID(&snmp, varlen, 4);
   while (node) {
      ASN1_BKWRITE_ID(&snmp, varlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, varlen, ASN1_TYPE_OBJECT, varlen);

   oidlen = 0;
   node = &MIBNODE_snmpTrapOID;
   ASN1_BKWRITE_ID(&snmp, oidlen, 0);
   while (node) {
      ASN1_BKWRITE_ID(&snmp, oidlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, oidlen, ASN1_TYPE_OBJECT, oidlen);

   traplen += oidlen + varlen;
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, oidlen+varlen);

   /* VarBind for ifIndex */
   varlen = 0;
   node = &MIBNODE_ifIndex;
   mib_val = (void *)node->GET;
   temp = MIB_get_ifIndex(ihandle);
   MIB_find_ifEntry(RTCSMIB_OP_GET, &temp, &instance);

   { /* Scope */
      int32_t   sinteger = 0;
      uint32_t  uinteger = 0;
#if SNMPCFG_HAVE_INT64
      int64_t   sinteger64 = 0;
      uint64_t  uinteger64 = 0;
#endif

    switch (mib_val->TYPE) {
    case RTCSMIB_NODETYPE_INT_CONST:
        sinteger = (int32_t)(mib_val->PARAM);
        break;
    case RTCSMIB_NODETYPE_INT_PTR:    
        sinteger = *(int32_t *)(mib_val->PARAM);
        break;
    case RTCSMIB_NODETYPE_INT_FN:       
        sinteger = ((RTCSMIB_INT_FN_PTR)mib_val->PARAM)(instance);  
        break;
    case RTCSMIB_NODETYPE_UINT_CONST:
        uinteger = (uint32_t)(mib_val->PARAM);
        break; 
    case RTCSMIB_NODETYPE_UINT_PTR:  
        uinteger = *(uint32_t *)(mib_val->PARAM);
        break;
    case RTCSMIB_NODETYPE_UINT_FN:      
        uinteger = ((RTCSMIB_UINT_FN_PTR)mib_val->PARAM)(instance);
        break;
#if SNMPCFG_HAVE_INT64
      case RTCSMIB_NODETYPE_INT64_CONST:  sinteger64 = node->GET.INT64_CONST; break;
      case RTCSMIB_NODETYPE_INT64_PTR:    sinteger64 = *node->GET.INT64_PTR; break;
      case RTCSMIB_NODETYPE_INT64_FN:     sinteger64 = node->GET.INT64_FN(instance); break;
      case RTCSMIB_NODETYPE_UINT64_CONST: uinteger64 = node->GET.UINT64_CONST; break;
      case RTCSMIB_NODETYPE_UINT64_PTR:   uinteger64 = *node->GET.UINT64_PTR; break;
      case RTCSMIB_NODETYPE_UINT64_FN:    uinteger64 = node->GET.UINT64_FN(instance); break;
#endif
      } /* Endswitch */

      switch (mib_val->TYPE) {
      case RTCSMIB_NODETYPE_INT_CONST:
      case RTCSMIB_NODETYPE_INT_PTR:
      case RTCSMIB_NODETYPE_INT_FN:
         ASN1_BKWRITE_TYPELEN_BIGINT(&snmp, varlen, node->ASN1_TYPE, sinteger);
         break;
      case RTCSMIB_NODETYPE_UINT_CONST:
      case RTCSMIB_NODETYPE_UINT_PTR:
      case RTCSMIB_NODETYPE_UINT_FN:
         ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, node->ASN1_TYPE, uinteger);
         break;
#if SNMPCFG_HAVE_INT64
      case RTCSMIB_NODETYPE_INT64_CONST:
      case RTCSMIB_NODETYPE_INT64_PTR:
      case RTCSMIB_NODETYPE_INT64_FN:
         ASN1_BKWRITE_TYPELEN_BIGINT(&snmp, varlen, node->ASN1_TYPE, sinteger64);
         break;
      case RTCSMIB_NODETYPE_UINT64_CONST:
      case RTCSMIB_NODETYPE_UINT64_PTR:
      case RTCSMIB_NODETYPE_UINT64_FN:
         ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, node->ASN1_TYPE, uinteger64);
         break;
#endif
      } /* Endswitch */
   } /* Endscope */

   oidlen = 0;
   
   { /* Scope */
      struct {
         uint32_t ifIndex;
      }      *index = (void *)&temp;
      ASN1_BKWRITE_ID(&snmp, oidlen, index->ifIndex);
   } /* Endscope */
   
   while (node) {
      ASN1_BKWRITE_ID(&snmp, oidlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, oidlen, ASN1_TYPE_OBJECT, oidlen);
   
   traplen += oidlen + varlen;
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, oidlen+varlen);

   varlen = 0;
   /* time-stamp */
   i = MIB_get_sysUpTime(NULL);
   ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, ASN1_TYPE_TimeTicks, i);

   oidlen = 0;
   node = &MIBNODE_sysUpTime;
   ASN1_BKWRITE_ID(&snmp, oidlen, 0);
   while(node) {
      ASN1_BKWRITE_ID(&snmp, oidlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, oidlen, ASN1_TYPE_OBJECT, oidlen);
   
   traplen += oidlen + varlen;
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);
   
   /* complete variable-bindings */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);
   
   /* error-index */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);

   /* error-status */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);
   
   /* request-id */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);

   /* complete SNMPv2_Trap-PDU */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_PDU_TRAPV2, traplen);
   
   /* community */
   ASN1_BKWRITE_TYPELEN_BYTES(&snmp, traplen, ASN1_TYPE_OCTET, 
                              snmpcfg->community[snmpcfg->currentcommunity], 
                              snmpcfg->communitylen[snmpcfg->currentcommunity]);   
   /* version, 1=v2 */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 1);
   /* complete Message */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);

   addr.sin_family = AF_INET;
   addr.sin_port   = IPPORT_SNMPTRAP;
   for (i=0; i<SNMPCFG_MAX_TRAP_RECEIVERS; i++) {
      if (snmpcfg->trap_receiver_list[i] != 0) {
         addr.sin_addr.s_addr = snmpcfg->trap_receiver_list[i];
         sendto(snmpcfg->trapsock, snmp.outbuf, traplen, 0, (sockaddr*)&addr, sizeof(addr));
      } /* Endif */
   } /* Endfor */

} /* Endbody */


void SNMPv2_trap_userSpec
   (
      RTCSMIB_NODE_PTR trap_node
   )
{ /* Body */
   SNMP_PARSE_PTR    snmpcfg = SNMP_get_data();
   uint32_t          traplen, oidlen, varlen;
   RTCSMIB_NODE_PTR  node;
   uint32_t          uinteger;
   int32_t           sinteger, i;
   sockaddr_in       addr;

   RTCSMIB_VALUE_PTR mib_val;

   unsigned char outbuf[SNMPCFG_BUFFER_SIZE];
   struct {
      uint32_t     outlen;
      unsigned char   *outbuf;
   } snmp;
      
   traplen = 0;
   snmp.outlen = sizeof(outbuf);
   snmp.outbuf = outbuf + sizeof(outbuf);

   /* VarBind for snmpTrapOID */
    varlen = 0;
    mib_val = (void *)trap_node->GET;

    switch (mib_val->TYPE) {
        case RTCSMIB_NODETYPE_INT_CONST:
            sinteger = (int32_t)(mib_val->PARAM);
            ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, ASN1_TYPE_INTEGER, sinteger);
            break;
        case RTCSMIB_NODETYPE_INT_PTR:
            sinteger = *(int32_t *)(mib_val->PARAM);
            ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, ASN1_TYPE_INTEGER, sinteger);
            break;
        case RTCSMIB_NODETYPE_INT_FN:
            sinteger = ((RTCSMIB_INT_FN_PTR)mib_val->PARAM)(NULL);
            ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, ASN1_TYPE_INTEGER, sinteger)
            break;
        case RTCSMIB_NODETYPE_UINT_CONST:
            uinteger = (uint32_t)(mib_val->PARAM);
            ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, ASN1_TYPE_INTEGER, uinteger);
            break;
        case RTCSMIB_NODETYPE_UINT_PTR:
            uinteger = *(uint32_t *)(mib_val->PARAM);
            ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, ASN1_TYPE_INTEGER, uinteger);
            break;
        case RTCSMIB_NODETYPE_UINT_FN:
            uinteger = ((RTCSMIB_UINT_FN_PTR)mib_val->PARAM)(NULL);
            ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, ASN1_TYPE_INTEGER, uinteger);
            break;
        case RTCSMIB_NODETYPE_DISPSTR_PTR:
            ASN1_BKWRITE_TYPELEN_BYTES(&snmp, traplen, ASN1_TYPE_OCTET, 
                mib_val->PARAM, strlen(mib_val->PARAM));
            break;
        case RTCSMIB_NODETYPE_DISPSTR_FN:
            ASN1_BKWRITE_TYPELEN_BYTES(&snmp, traplen, ASN1_TYPE_OCTET, 
                ((RTCSMIB_DISPSTR_FN_PTR)mib_val->PARAM)(NULL),
                strlen( (const char *)((RTCSMIB_DISPSTR_FN_PTR)mib_val->PARAM)(NULL) ) ) ;
            break;                      
        default:
            ASN1_BKWRITE_TYPELEN_BYTES(&snmp, traplen, ASN1_TYPE_OCTET, 
                "Not supported", 13 ) ;
            break;
    } 
   
   oidlen = 0;
   node = trap_node;
   ASN1_BKWRITE_ID(&snmp, oidlen, 0);
   while (node) {
      ASN1_BKWRITE_ID(&snmp, oidlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, oidlen, ASN1_TYPE_OBJECT, oidlen);

   traplen += oidlen + varlen;
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);

   /* VarBind for snmpTrapOID2 */

   varlen = 0;
   node = &MIBNODE_snmpTraps;
   ASN1_BKWRITE_ID(&snmp, varlen, 6);
   while (node) {
      ASN1_BKWRITE_ID(&snmp, varlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, varlen, ASN1_TYPE_OBJECT, varlen);

   oidlen = 0;
   node = &MIBNODE_snmpTrapOID;
   ASN1_BKWRITE_ID(&snmp, oidlen, 1);
   while (node) {
      ASN1_BKWRITE_ID(&snmp, oidlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, oidlen, ASN1_TYPE_OBJECT, oidlen);

   traplen += oidlen + varlen;
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, oidlen+varlen);

   /* VarBind for sysUpTime */

   varlen = 0;
   sinteger = MIB_get_sysUpTime(NULL);
   ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, ASN1_TYPE_TimeTicks, sinteger);

   oidlen = 0;
   node = &MIBNODE_sysUpTime;
   ASN1_BKWRITE_ID(&snmp, oidlen, 0);
   while (node) {
      ASN1_BKWRITE_ID(&snmp, oidlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, oidlen, ASN1_TYPE_OBJECT, oidlen);

   traplen += oidlen + varlen;
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, oidlen+varlen);

   /* complete variable-bindings */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);
   /* error-index */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);
   /* error-status */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);
   /* request-id */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);

   /* complete SNMPv2-Trap-PDU */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_PDU_TRAPV2, traplen);
   /* community */
   ASN1_BKWRITE_TYPELEN_BYTES(&snmp, traplen, ASN1_TYPE_OCTET, 
                              snmpcfg->community[snmpcfg->currentcommunity], 
                              snmpcfg->communitylen[snmpcfg->currentcommunity]);   
   /* version, 1=v2 */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 1);
   /* complete Message */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);

   addr.sin_family = AF_INET;
   addr.sin_port   = IPPORT_SNMPTRAP;
   for (i = 0; i < SNMPCFG_MAX_TRAP_RECEIVERS; i++) {
      if (snmpcfg->trap_receiver_list[i] != 0) {
         addr.sin_addr.s_addr = snmpcfg->trap_receiver_list[i];
         sendto(snmpcfg->trapsock, snmp.outbuf, traplen, 0, (sockaddr*)&addr, sizeof(addr));
      } /* Endif */
   } /* Endfor */

} /* Endbody */

void SNMP_trap_userSpec
   (
      RTCSMIB_NODE_PTR trap_node, uint32_t spec_trap, RTCSMIB_NODE_PTR enterprises
   )
{ /* Body */
   SNMP_PARSE_PTR    snmpcfg = SNMP_get_data();
   uint32_t           traplen, oidlen, varlen;
   RTCSMIB_NODE_PTR  node;
   uint32_t           i;
   unsigned char         *ipsrcp;
   _ip_address       ipsrc, ipaddr;
   sockaddr_in       addr;
      uint32_t        uinteger;
   int32_t            sinteger;
   
   RTCSMIB_VALUE_PTR mib_val;
   
   unsigned char outbuf[SNMPCFG_BUFFER_SIZE];
   struct {
      uint32_t     outlen;
      unsigned char   *outbuf;
   } snmp;

   traplen = 0;
   oidlen = 0;
   snmp.outlen = sizeof(outbuf);
   snmp.outbuf = outbuf + sizeof(outbuf);

   /* complete variable-bindings */
    varlen  = 0;
    mib_val = (void *)trap_node->GET;

    switch (mib_val->TYPE) {
        case RTCSMIB_NODETYPE_INT_CONST:
            sinteger = (int32_t)(mib_val->PARAM);
            ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, ASN1_TYPE_INTEGER, sinteger);
            break;
        case RTCSMIB_NODETYPE_INT_PTR:
            sinteger = *(int32_t *)(mib_val->PARAM);
            ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, ASN1_TYPE_INTEGER, sinteger);
            break;
        case RTCSMIB_NODETYPE_INT_FN:
            sinteger = ((RTCSMIB_INT_FN_PTR)mib_val->PARAM)(NULL);
            ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, ASN1_TYPE_INTEGER, sinteger)
            break;
        case RTCSMIB_NODETYPE_UINT_CONST:
            uinteger = (uint32_t)(mib_val->PARAM);
            ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, ASN1_TYPE_INTEGER, uinteger);
            break;
        case RTCSMIB_NODETYPE_UINT_PTR:
            uinteger = *(uint32_t *)(mib_val->PARAM);
            ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, ASN1_TYPE_INTEGER, uinteger);
            break;
        case RTCSMIB_NODETYPE_UINT_FN:
            uinteger = ((RTCSMIB_UINT_FN_PTR)mib_val->PARAM)(NULL);
            ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, varlen, ASN1_TYPE_INTEGER, uinteger);
            break;
        case RTCSMIB_NODETYPE_DISPSTR_PTR:
            ASN1_BKWRITE_TYPELEN_BYTES(&snmp, traplen, ASN1_TYPE_OCTET, 
                mib_val->PARAM, strlen(mib_val->PARAM));
            break;
        case RTCSMIB_NODETYPE_DISPSTR_FN:
            ASN1_BKWRITE_TYPELEN_BYTES(&snmp, traplen, ASN1_TYPE_OCTET, 
                ((RTCSMIB_DISPSTR_FN_PTR)mib_val->PARAM)(NULL),
                strlen( (const char *)((RTCSMIB_DISPSTR_FN_PTR)mib_val->PARAM)(NULL) ) ) ;
            break;                      
        default:
            ASN1_BKWRITE_TYPELEN_BYTES(&snmp, traplen, ASN1_TYPE_OCTET, 
                "Not supported", 13 ) ;
            break;
    } 

   oidlen = 0;
   node = trap_node;
   ASN1_BKWRITE_ID(&snmp, oidlen, 0);
   while (node) {
      ASN1_BKWRITE_ID(&snmp, oidlen, node->ID);
      node = node->PARENT;
   } 
   ASN1_BKWRITE_TYPELEN(&snmp, oidlen, ASN1_TYPE_OBJECT, oidlen);
    
    traplen += oidlen + varlen;
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);   
   /* variable-bindings items */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);
   
   /* time-stamp */
   i = MIB_get_sysUpTime(NULL);
   ASN1_BKWRITE_TYPELEN_BIGUINT(&snmp, traplen, ASN1_TYPE_TimeTicks, i);
   /* specific-trap */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, spec_trap);
   /* generic-trap */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 6);
   /* skip agent-addr */
   traplen     += 4;
   snmp.outlen -= 4;
   snmp.outbuf -= 4;
   ipsrcp = snmp.outbuf;
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_IpAddress, 4);

   /* enterprise */
   node = enterprises;
   oidlen = 0;
   while (node) {
      ASN1_BKWRITE_ID(&snmp, oidlen, node->ID);
      node = node->PARENT;
   } /* Endwhile */
   ASN1_BKWRITE_TYPELEN(&snmp, oidlen, ASN1_TYPE_OBJECT, oidlen);
   traplen += oidlen;

   /* complete Trap-PDU */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_PDU_TRAP, traplen);
   /* community */
   ASN1_BKWRITE_TYPELEN_BYTES(&snmp, traplen, ASN1_TYPE_OCTET, 
                              snmpcfg->community[snmpcfg->currentcommunity], 
                              snmpcfg->communitylen[snmpcfg->currentcommunity]);   
   /* version, 0=v1 */
   ASN1_BKWRITE_TYPELEN_INT(&snmp, traplen, 0);
   /* complete Message */
   ASN1_BKWRITE_TYPELEN(&snmp, traplen, ASN1_TYPE_SEQUENCE, traplen);

   addr.sin_family = AF_INET;
   addr.sin_port   = IPPORT_SNMPTRAP;
   for (i = 0; i < SNMPCFG_MAX_TRAP_RECEIVERS; i++) {
      if (snmpcfg->trap_receiver_list[i] != 0) {
         ipaddr = snmpcfg->trap_receiver_list[i];
         ipsrc = IP_route_find(ipaddr, 0);
         (void) mqx_htonl(ipsrcp, ipsrc);
         addr.sin_addr.s_addr = ipaddr;
         sendto(snmpcfg->trapsock, snmp.outbuf, traplen, 0, (sockaddr*)&addr, sizeof(addr));
      } /* Endif */
   } /* Endfor */

} /* Endbody */

#endif /* RTCSCFG_ENABLE_SNMP && RTCSCFG_ENABLE_IP4 */
 
/* EOF */
