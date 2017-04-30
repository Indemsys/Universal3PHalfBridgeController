#ifndef __snmpcfg_h__
#define __snmpcfg_h__
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
*   This file contains the definitions for configuring
*   optional features in SNMP.
*   SNMP is delivered as follows:
*   SNMPCFG_BUFFER_SIZE            512
*   SNMPCFG_NUM_COMMUNITY           2
*   SNMPCFG_COMMUNITY_LIST         "public" , "private"
*   SNMPCFG_MAX_TRAP_RECEIVERS      6
*   SNMPCFG_SEND_V2_TRAPS           NOT DEFINE
*   SNMPCFG_SYSDESCR               "RTCS version <version>"
*   SNMPCFG_SYSSERVICES            8 (end-to-end)
*   SNMPCFG_HAVE_INT64             0
*
*
*END************************************************************************/

/*
** SNMP has two static buffers (one for receiving and one for generating
** responses), each of size SNMPCFG_BUFFER_SIZE.  RFCs 1157 and 1906 require
** this number to be at least 484.
*/
#ifndef SNMPCFG_BUFFER_SIZE
#define SNMPCFG_BUFFER_SIZE   512
#endif

/*
** The number of communities, used in both SNMPv1 and SNMPv2c.
*/
#ifndef SNMPCFG_NUM_COMMUNITY
#define SNMPCFG_NUM_COMMUNITY 2
#endif


/* 
** The string of communities, used in both SNMPv1 and SNMPv2c.
** Communities can be added to the list at compile time
*/
#ifndef SNMPCFG_COMMUNITY_LIST
#define SNMPCFG_COMMUNITY_LIST {"public", "private"} 
#endif

/* 
** Size of TRAP Receivers table - increase if sending traps to more than 6 nodes 
*/
#ifndef SNMPCFG_MAX_TRAP_RECEIVERS
#define SNMPCFG_MAX_TRAP_RECEIVERS 6
#endif

/* 
** Send v2 traps?  If not defined, send v1 traps instead 
*/
#define SNMPCFG_SEND_V2_TRAPS

/*
** The value of the system.sysDescr variable.
*/
#ifndef SNMPCFG_SYSDESCR
#define SNMPCFG_NUMTOSTR(x)   #x
#define SNMPCFG_NUMTOSTR2(x)  SNMPCFG_NUMTOSTR(x)
#define SNMPCFG_SYSDESCR      "RTCS version " SNMPCFG_NUMTOSTR2(RTCS_VERSION_MAJOR) \
                              "." SNMPCFG_NUMTOSTR2(RTCS_VERSION_MINOR) \
                              "." SNMPCFG_NUMTOSTR2(RTCS_VERSION_REV)
#endif

/*
** The value of the system.sysServices variable.
*/
#ifndef SNMPCFG_SYSSERVICES
#define SNMPCFG_SYSSERVICES   8
#endif

/*
** SNMPCFG_HAVE_INT64 enables support for 64-bit integers.
*/
#ifndef SNMPCFG_HAVE_INT64
#define SNMPCFG_HAVE_INT64    0
#endif

/* To traverse the MIB in the SNMP task instead
   of the RTCS task, uncomment the line below.  */
// #define TRAVERSE_MIB_IN_SNMP_TASK 1
#ifdef TRAVERSE_MIB_IN_SNMP_TASK
#warning Do not access RTCS OIDs when traversing MIB in snmp task
#endif

#endif
/* EOF */
