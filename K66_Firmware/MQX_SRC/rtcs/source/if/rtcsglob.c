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
*   Definitions of global variables required for RTCS.
*
*
*END************************************************************************/

#include <rtcs.h>

uint32_t  _RTCSQUEUE_base = RTCSCFG_QUEUE_BASE;

/*
** Number of RTCSPCBs to allocate
*/
uint32_t  _RTCSPCB_init = RTCSCFG_PCBS_INIT;
uint32_t  _RTCSPCB_grow = RTCSCFG_PCBS_GROW;
uint32_t  _RTCSPCB_max  = RTCSCFG_PCBS_MAX;

/*
** TCP/IP task priority and extra stack size
*/
uint32_t  _RTCSTASK_priority   = RTCSCFG_DEFAULT_RTCSTASK_PRIO;
uint32_t  _RTCSTASK_stacksize  = RTCSCFG_STACK_SIZE;
bool  _RTCS_initialized    = FALSE;

/*
** TCP-based RPC task priority and extra stack size
*/
uint32_t  _SVCTCPTASK_priority    = RTCSCFG_DEFAULT_RTCSTASK_PRIO+2;
uint32_t  _SVCTCPTASK_stacksize   = 0;

/*
** Indicates whether or not IP should forward packets between interfaces
*/
bool  _IP_forward = FALSE;

/*
** Indicates whether or not to bypass checksum checking
*/
bool  _TCP_bypass_rx = FALSE;
bool  _TCP_bypass_tx = FALSE;

/*
** The TCP retransmission timeout minimum
*/
uint32_t  _TCP_rto_min;     /* initialized by TCP_Init() */

/* 
** Use the dhcp broadcast flag, or dhcp unicast offers. 
*/
bool _DHCP_broadcast = FALSE;

/*
** Pool that RTCS should allocate memory from. If 0, susyem pool will be used
*/
_mem_pool_id _RTCS_mem_pool = 0;


uint32_t _RTCS_msgpool_init  = RTCSCFG_MSGPOOL_INIT;
uint32_t _RTCS_msgpool_grow  = RTCSCFG_MSGPOOL_GROW;
uint32_t _RTCS_msgpool_max   = RTCSCFG_MSGPOOL_MAX;


uint32_t _RTCS_socket_part_init  = RTCSCFG_SOCKET_PART_INIT;
uint32_t _RTCS_socket_part_grow  = RTCSCFG_SOCKET_PART_GROW;
uint32_t _RTCS_socket_part_max   = RTCSCFG_SOCKET_PART_MAX;

/* 
** Use following dhcp termination timeout. 
*/
uint32_t _RTCS_dhcp_term_timeout = 60;

/* EOF */
