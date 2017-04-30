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
*   This file contains the RTCS "netstat" shell command.
*
*
*END************************************************************************/

#include <ctype.h> 
#include <string.h> 
#include <mqx.h> 
#include "shell.h" 

#if SHELLCFG_USES_RTCS 
#include <rtcs.h> 
#include <enet.h> 
#include "sh_rtcs.h" 
#include <ipcfg.h> 


#define SHELL_NETSTAT_ENTRY(t,s,x) { s, (uint32_t) &(((t##_STATS_PTR)NULL)->x) } 

typedef struct stat_entry_struct { 
    char *      message; 
    uint32_t     offset; 
} STAT_ENTRY_STRUCT; 

#if BSPCFG_ENABLE_ENET_STATS  &&  (BSP_ENET_DEVICE_COUNT > 0) 
const STAT_ENTRY_STRUCT enet_stat_entries[] =
{ 
    SHELL_NETSTAT_ENTRY(ENET,"packets received",               COMMON.ST_RX_TOTAL), 
    SHELL_NETSTAT_ENTRY(ENET,"rx packets missed",              COMMON.ST_RX_MISSED), 
    SHELL_NETSTAT_ENTRY(ENET,"rx packets discarded",           COMMON.ST_RX_DISCARDED), 
    SHELL_NETSTAT_ENTRY(ENET,"rx packets with error",          COMMON.ST_RX_ERRORS), 
    SHELL_NETSTAT_ENTRY(ENET,"packets transmitted",            COMMON.ST_TX_TOTAL), 
    SHELL_NETSTAT_ENTRY(ENET,"tx packets missed",              COMMON.ST_TX_MISSED), 
    SHELL_NETSTAT_ENTRY(ENET,"tx packets discarded",           COMMON.ST_TX_DISCARDED), 
    SHELL_NETSTAT_ENTRY(ENET,"tx packets received with error", COMMON.ST_TX_ERRORS), 
    SHELL_NETSTAT_ENTRY(ENET,"rx align error",                 ST_RX_ALIGN), 
    SHELL_NETSTAT_ENTRY(ENET,"rx fcs error",                   ST_RX_FCS), 
    SHELL_NETSTAT_ENTRY(ENET,"rx runt error",                  ST_RX_RUNT), 
    SHELL_NETSTAT_ENTRY(ENET,"rx giant error",                 ST_RX_GIANT), 
    SHELL_NETSTAT_ENTRY(ENET,"rx latecoll error",              ST_RX_LATECOLL), 
    SHELL_NETSTAT_ENTRY(ENET,"rx overrun error",               ST_RX_OVERRUN), 
    SHELL_NETSTAT_ENTRY(ENET,"tx SQE",                         ST_TX_SQE), 
    SHELL_NETSTAT_ENTRY(ENET,"tx deferred",                    ST_TX_DEFERRED), 
    SHELL_NETSTAT_ENTRY(ENET,"tx late collision",              ST_TX_LATECOLL), 
    SHELL_NETSTAT_ENTRY(ENET,"tx excessive collision",         ST_TX_EXCESSCOLL), 
    SHELL_NETSTAT_ENTRY(ENET,"tx carrier",                     ST_TX_CARRIER), 
    SHELL_NETSTAT_ENTRY(ENET,"tx unerrun error",               ST_TX_UNDERRUN) 
}; 
#endif 
#if RTCSCFG_ENABLE_IP_STATS && RTCSCFG_ENABLE_IP4 
const STAT_ENTRY_STRUCT ip_stat_entries[] =
{ 
    SHELL_NETSTAT_ENTRY(IP, "packets received",                  COMMON.ST_RX_TOTAL), 
    SHELL_NETSTAT_ENTRY(IP, "packets delivered",                 ST_RX_DELIVERED), 
    SHELL_NETSTAT_ENTRY(IP, "packets forwarded",                 ST_RX_FORWARDED), 
    SHELL_NETSTAT_ENTRY(IP, "discarded for lack of resources",   COMMON.ST_RX_MISSED), 
    SHELL_NETSTAT_ENTRY(IP, "discarded due to internal errors",  COMMON.ST_RX_ERRORS), 
    SHELL_NETSTAT_ENTRY(IP, "discarded for other reasons:",      COMMON.ST_RX_DISCARDED), 
    SHELL_NETSTAT_ENTRY(IP, "  with header errors",              ST_RX_HDR_ERRORS), 
    SHELL_NETSTAT_ENTRY(IP, "  with an illegal destination",     ST_RX_ADDR_ERRORS), 
    SHELL_NETSTAT_ENTRY(IP, "  with unknown protocols",          ST_RX_NO_PROTO), 
    SHELL_NETSTAT_ENTRY(IP, "fragments received",                ST_RX_FRAG_RECVD), 
    SHELL_NETSTAT_ENTRY(IP, "fragments reassembled",             ST_RX_FRAG_REASMD), 
    SHELL_NETSTAT_ENTRY(IP, "fragments discarded",               ST_RX_FRAG_DISCARDED), 
    SHELL_NETSTAT_ENTRY(IP, "packets sent",                      COMMON.ST_TX_TOTAL), 
    SHELL_NETSTAT_ENTRY(IP, "unsent for lack of resources",      COMMON.ST_TX_MISSED), 
    SHELL_NETSTAT_ENTRY(IP, "unsent due to internal errors",     COMMON.ST_TX_ERRORS), 
    SHELL_NETSTAT_ENTRY(IP, "destinations found unreachable",    COMMON.ST_TX_DISCARDED), 
    SHELL_NETSTAT_ENTRY(IP, "packets fragmented",                ST_TX_FRAG_FRAGD), 
    SHELL_NETSTAT_ENTRY(IP, "fragments sent",                    ST_TX_FRAG_SENT), 
    SHELL_NETSTAT_ENTRY(IP, "fragmentation failures",            ST_TX_FRAG_DISCARDED) 
}; 
#endif 

#if RTCSCFG_ENABLE_IP6_STATS && RTCSCFG_ENABLE_IP6 
const STAT_ENTRY_STRUCT ip6_stat_entries[] =
{ 
    SHELL_NETSTAT_ENTRY(IP6, "packets received",                  COMMON.ST_RX_TOTAL), 
    SHELL_NETSTAT_ENTRY(IP6, "packets delivered",                 ST_RX_DELIVERED), 
    SHELL_NETSTAT_ENTRY(IP6, "packets forwarded",                 ST_RX_FORWARDED), 
    SHELL_NETSTAT_ENTRY(IP6, "discarded for lack of resources",   COMMON.ST_RX_MISSED), 
    SHELL_NETSTAT_ENTRY(IP6, "discarded due to internal errors",  COMMON.ST_RX_ERRORS), 
    SHELL_NETSTAT_ENTRY(IP6, "discarded for other reasons:",      COMMON.ST_RX_DISCARDED), 
    SHELL_NETSTAT_ENTRY(IP6, "  with header errors",              ST_RX_HDR_ERRORS), 
    SHELL_NETSTAT_ENTRY(IP6, "  with an illegal destination",     ST_RX_ADDR_ERRORS), 
    SHELL_NETSTAT_ENTRY(IP6, "  with unknown protocols",          ST_RX_NO_PROTO), 
    SHELL_NETSTAT_ENTRY(IP6, "fragments received",                ST_RX_FRAG_RECVD), 
    SHELL_NETSTAT_ENTRY(IP6, "fragments reassembled",             ST_RX_FRAG_REASMD), 
    SHELL_NETSTAT_ENTRY(IP6, "fragments discarded",               ST_RX_FRAG_DISCARDED), 
    SHELL_NETSTAT_ENTRY(IP6, "packets sent",                      COMMON.ST_TX_TOTAL), 
    SHELL_NETSTAT_ENTRY(IP6, "unsent for lack of resources",      COMMON.ST_TX_MISSED), 
    SHELL_NETSTAT_ENTRY(IP6, "unsent due to internal errors",     COMMON.ST_TX_ERRORS), 
    SHELL_NETSTAT_ENTRY(IP6, "destinations found unreachable",    COMMON.ST_TX_DISCARDED), 
    SHELL_NETSTAT_ENTRY(IP6, "packets fragmented",                ST_TX_FRAG_FRAGD), 
    SHELL_NETSTAT_ENTRY(IP6, "fragments sent",                    ST_TX_FRAG_SENT), 
    SHELL_NETSTAT_ENTRY(IP6, "fragmentation failures",            ST_TX_FRAG_DISCARDED) 
}; 
#endif 


#if RTCSCFG_ENABLE_ICMP_STATS && RTCSCFG_ENABLE_ICMP && RTCSCFG_ENABLE_IP4
const STAT_ENTRY_STRUCT icmp_stat_entries[] =
{ 
    SHELL_NETSTAT_ENTRY(ICMP, "[RX] packets received",                  COMMON.ST_RX_TOTAL), 
    SHELL_NETSTAT_ENTRY(ICMP, "discarded for lack of resources",   COMMON.ST_RX_MISSED), 
    SHELL_NETSTAT_ENTRY(ICMP, "discarded due to internal errors",  COMMON.ST_RX_ERRORS), 
    SHELL_NETSTAT_ENTRY(ICMP, "discarded for other reasons:",      COMMON.ST_RX_DISCARDED), 
    SHELL_NETSTAT_ENTRY(ICMP, "  with bad checksum",               ST_RX_BAD_CHECKSUM), 
    SHELL_NETSTAT_ENTRY(ICMP, "  with small dgram",                ST_RX_SMALL_DGRAM), 
    SHELL_NETSTAT_ENTRY(ICMP, "  unrecognized codes",              ST_RX_BAD_CODE), 
    SHELL_NETSTAT_ENTRY(ICMP, "  redirects from non-gateways",     ST_RX_RD_NOTGATE), 
    SHELL_NETSTAT_ENTRY(ICMP, "Destination Unreachables", ST_RX_DESTUNREACH), 
    SHELL_NETSTAT_ENTRY(ICMP, "Time Exceededs",           ST_RX_TIMEEXCEED), 
    SHELL_NETSTAT_ENTRY(ICMP, "Parameter Problems",       ST_RX_PARMPROB), 
    SHELL_NETSTAT_ENTRY(ICMP, "Source Quenches",          ST_RX_SRCQUENCH), 
    SHELL_NETSTAT_ENTRY(ICMP, "Redirects",                ST_RX_REDIRECT), 
    SHELL_NETSTAT_ENTRY(ICMP, "Echo Requests",            ST_RX_ECHO_REQ), 
    SHELL_NETSTAT_ENTRY(ICMP, "Echo Replies",             ST_RX_ECHO_REPLY), 
    SHELL_NETSTAT_ENTRY(ICMP, "Timestamp Requests",       ST_RX_TIME_REQ), 
    SHELL_NETSTAT_ENTRY(ICMP, "Timestamp Replies",        ST_RX_TIME_REPLY), 
    SHELL_NETSTAT_ENTRY(ICMP, "Information Requests",     ST_RX_INFO_REQ), 
    SHELL_NETSTAT_ENTRY(ICMP, "Information Replies",      ST_RX_INFO_REPLY), 
    SHELL_NETSTAT_ENTRY(ICMP, "Unknown",                  ST_RX_OTHER), 
    SHELL_NETSTAT_ENTRY(ICMP, "[TX] packets sent",                      COMMON.ST_TX_TOTAL), 
    SHELL_NETSTAT_ENTRY(ICMP, "discarded for lack of resources",   COMMON.ST_TX_MISSED), 
    SHELL_NETSTAT_ENTRY(ICMP, "discarded due to internal errors",  COMMON.ST_TX_ERRORS), 
    SHELL_NETSTAT_ENTRY(ICMP, "with illegal type or code",         COMMON.ST_TX_DISCARDED),
    SHELL_NETSTAT_ENTRY(ICMP, "Destination Unreachables", ST_TX_DESTUNREACH), 
    SHELL_NETSTAT_ENTRY(ICMP, "Time Exceededs",           ST_TX_TIMEEXCEED), 
    SHELL_NETSTAT_ENTRY(ICMP, "Parameter Problems",       ST_TX_PARMPROB), 
    SHELL_NETSTAT_ENTRY(ICMP, "Source Quenches",          ST_TX_SRCQUENCH), 
    SHELL_NETSTAT_ENTRY(ICMP, "Redirects",                ST_TX_REDIRECT), 
    SHELL_NETSTAT_ENTRY(ICMP, "Echo Requests",            ST_TX_ECHO_REQ), 
    SHELL_NETSTAT_ENTRY(ICMP, "Echo Replies",             ST_TX_ECHO_REPLY), 
    SHELL_NETSTAT_ENTRY(ICMP, "Timestamp Requests",       ST_TX_TIME_REQ), 
    SHELL_NETSTAT_ENTRY(ICMP, "Timestamp Replies",        ST_TX_TIME_REPLY), 
    SHELL_NETSTAT_ENTRY(ICMP, "Information Requests",     ST_TX_INFO_REQ), 
    SHELL_NETSTAT_ENTRY(ICMP, "Information Replies",      ST_TX_INFO_REPLY), 
    SHELL_NETSTAT_ENTRY(ICMP, "Unknown",                  ST_TX_OTHER),  
}; 
#endif 

#if RTCSCFG_ENABLE_ICMP6_STATS && RTCSCFG_ENABLE_IP6
const STAT_ENTRY_STRUCT icmp6_stat_entries[] =
{ 
    SHELL_NETSTAT_ENTRY(ICMP6, "[RX] packets received",                  COMMON.ST_RX_TOTAL), 
    SHELL_NETSTAT_ENTRY(ICMP6, "discarded for lack of resources",   COMMON.ST_RX_MISSED), 
    SHELL_NETSTAT_ENTRY(ICMP6, "discarded due to internal errors",  COMMON.ST_RX_ERRORS), 
    SHELL_NETSTAT_ENTRY(ICMP6, "discarded for other reasons:",      COMMON.ST_RX_DISCARDED), 
    SHELL_NETSTAT_ENTRY(ICMP6, "  with bad checksum",               ST_RX_BAD_CHECKSUM), 
    SHELL_NETSTAT_ENTRY(ICMP6, "  with small dgram",                ST_RX_SMALL_DGRAM), 
    SHELL_NETSTAT_ENTRY(ICMP6, "Destination Unreachables", ST_RX_DESTUNREACH), 
    SHELL_NETSTAT_ENTRY(ICMP6, "Time Exceededs",           ST_RX_TIMEEXCEED), 
    SHELL_NETSTAT_ENTRY(ICMP6, "Parameter Problems",       ST_RX_PARMPROB), 
    SHELL_NETSTAT_ENTRY(ICMP6, "Redirects",                ST_RX_REDIRECT), 
    SHELL_NETSTAT_ENTRY(ICMP6, "Echo Requests",            ST_RX_ECHO_REQ), 
    SHELL_NETSTAT_ENTRY(ICMP6, "Echo Replies",             ST_RX_ECHO_REPLY), 
    SHELL_NETSTAT_ENTRY(ICMP6, "Unknown",                  ST_RX_OTHER), 
    SHELL_NETSTAT_ENTRY(ICMP6, "[TX] packets sent",                      COMMON.ST_TX_TOTAL), 
    SHELL_NETSTAT_ENTRY(ICMP6, "discarded for lack of resources",   COMMON.ST_TX_MISSED), 
    SHELL_NETSTAT_ENTRY(ICMP6, "discarded due to internal errors",  COMMON.ST_TX_ERRORS), 
    SHELL_NETSTAT_ENTRY(ICMP6, "with illegal type or code",         COMMON.ST_TX_DISCARDED),
    SHELL_NETSTAT_ENTRY(ICMP6, "Destination Unreachables", ST_TX_DESTUNREACH), 
    SHELL_NETSTAT_ENTRY(ICMP6, "Time Exceededs",           ST_TX_TIMEEXCEED), 
    SHELL_NETSTAT_ENTRY(ICMP6, "Parameter Problems",       ST_TX_PARMPROB), 
    SHELL_NETSTAT_ENTRY(ICMP6, "Redirects",                ST_TX_REDIRECT), 
    SHELL_NETSTAT_ENTRY(ICMP6, "Echo Requests",            ST_TX_ECHO_REQ), 
    SHELL_NETSTAT_ENTRY(ICMP6, "Echo Replies",             ST_TX_ECHO_REPLY), 
    SHELL_NETSTAT_ENTRY(ICMP6, "Unknown",                  ST_TX_OTHER),  
}; 
#endif 

#if RTCSCFG_ENABLE_TCP_STATS && RTCSCFG_ENABLE_TCP
const STAT_ENTRY_STRUCT tcp_stat_entries[] =
{ 
    SHELL_NETSTAT_ENTRY(TCP, "packets received",                  COMMON.ST_RX_TOTAL), 
    SHELL_NETSTAT_ENTRY(TCP, "discarded for lack of resources",   COMMON.ST_RX_MISSED), 
    SHELL_NETSTAT_ENTRY(TCP, "discarded due to internal errors",  COMMON.ST_RX_ERRORS), 
    SHELL_NETSTAT_ENTRY(TCP, "discarded for other reasons:",      COMMON.ST_RX_DISCARDED), 
    SHELL_NETSTAT_ENTRY(TCP, "  with bad port",                   ST_RX_BAD_PORT), 
    SHELL_NETSTAT_ENTRY(TCP, "  with bad checksum",               ST_RX_BAD_CHECKSUM ), 
    SHELL_NETSTAT_ENTRY(TCP, "  with small header",               ST_RX_SMALL_HDR), 
    SHELL_NETSTAT_ENTRY(TCP, "  with small dgram",                ST_RX_SMALL_DGRAM ), 
    SHELL_NETSTAT_ENTRY(TCP, "  with small packet",               ST_RX_SMALL_PKT), 
    SHELL_NETSTAT_ENTRY(TCP, "  acks for unsent data",            ST_RX_BAD_ACK), 
    SHELL_NETSTAT_ENTRY(TCP, "  with data outside window",        ST_RX_BAD_DATA), 
    SHELL_NETSTAT_ENTRY(TCP, "  with data after close",           ST_RX_LATE_DATA), 
    SHELL_NETSTAT_ENTRY(TCP, "segments with data",                ST_RX_DATA), 
    SHELL_NETSTAT_ENTRY(TCP, "segments with duplicate data",      ST_RX_DATA_DUP), 
    SHELL_NETSTAT_ENTRY(TCP, "segments with only an ACK",         ST_RX_ACK), 
    SHELL_NETSTAT_ENTRY(TCP, "segments with a duplicate ACK",     ST_RX_ACK_DUP), 
    SHELL_NETSTAT_ENTRY(TCP, "segments with a RST",               ST_RX_RESET), 
    SHELL_NETSTAT_ENTRY(TCP, "window probes",                     ST_RX_PROBE), 
    SHELL_NETSTAT_ENTRY(TCP, "window updates",                    ST_RX_WINDOW), 
    SHELL_NETSTAT_ENTRY(TCP, "packets sent",                      COMMON.ST_TX_TOTAL), 
    SHELL_NETSTAT_ENTRY(TCP, "discarded for lack of resources",   COMMON.ST_TX_MISSED), 
    SHELL_NETSTAT_ENTRY(TCP, "discarded due to internal errors",  COMMON.ST_TX_ERRORS), 
    SHELL_NETSTAT_ENTRY(TCP, "with illegal destination port",     COMMON.ST_TX_DISCARDED), 
    SHELL_NETSTAT_ENTRY(TCP, "segments with data",                ST_TX_DATA), 
    SHELL_NETSTAT_ENTRY(TCP, "segments with retransmitted data",  ST_TX_DATA_DUP), 
    SHELL_NETSTAT_ENTRY(TCP, "segments with only an ACK",         ST_TX_ACK), 
    SHELL_NETSTAT_ENTRY(TCP, "segments with a delayed ACK",       ST_TX_ACK_DELAYED), 
    SHELL_NETSTAT_ENTRY(TCP, "segments with a RST",               ST_TX_RESET), 
    SHELL_NETSTAT_ENTRY(TCP, "window probes",                     ST_TX_PROBE), 
    SHELL_NETSTAT_ENTRY(TCP, "window updates",                    ST_TX_WINDOW), 
    SHELL_NETSTAT_ENTRY(TCP, "active opens",                      ST_CONN_ACTIVE), 
    SHELL_NETSTAT_ENTRY(TCP, "passive opens",                     ST_CONN_PASSIVE), 
    SHELL_NETSTAT_ENTRY(TCP, "connections currently established", ST_CONN_OPEN), 
    SHELL_NETSTAT_ENTRY(TCP, "connections gracefully closed",     ST_CONN_CLOSED), 
    SHELL_NETSTAT_ENTRY(TCP, "connections aborted",               ST_CONN_RESET), 
    SHELL_NETSTAT_ENTRY(TCP, "failed connection attempts",        ST_CONN_FAILED), 
}; 
#endif 
#if RTCSCFG_ENABLE_UDP_STATS && RTCSCFG_ENABLE_UDP
const STAT_ENTRY_STRUCT udp_stat_entries[] =
{ 
    SHELL_NETSTAT_ENTRY(UDP, "packets received",                  COMMON.ST_RX_TOTAL), 
    SHELL_NETSTAT_ENTRY(UDP, "discarded for lack of resources",   COMMON.ST_RX_MISSED), 
    SHELL_NETSTAT_ENTRY(UDP, "discarded due to internal errors",  COMMON.ST_RX_ERRORS), 
    SHELL_NETSTAT_ENTRY(UDP, "discarded for other reasons:",      COMMON.ST_RX_DISCARDED), 
    SHELL_NETSTAT_ENTRY(UDP, "  with bad port",                   ST_RX_BAD_PORT), 
    SHELL_NETSTAT_ENTRY(UDP, "  with bad checksum",               ST_RX_BAD_CHECKSUM ), 
    SHELL_NETSTAT_ENTRY(UDP, "  with small dgram",                ST_RX_SMALL_DGRAM), 
    SHELL_NETSTAT_ENTRY(UDP, "  with small packet",               ST_RX_SMALL_PKT), 
    SHELL_NETSTAT_ENTRY(UDP, "  with unknown ports",              ST_RX_NO_PORT), 
    SHELL_NETSTAT_ENTRY(UDP, "packets sent",                      COMMON.ST_TX_TOTAL), 
    SHELL_NETSTAT_ENTRY(UDP, "unsent for lack of resources",      COMMON.ST_TX_MISSED), 
    SHELL_NETSTAT_ENTRY(UDP, "unsent due to internal errors",     COMMON.ST_TX_ERRORS), 
    SHELL_NETSTAT_ENTRY(UDP, "with illegal destination port",     COMMON.ST_TX_DISCARDED) 
}; 
#endif 

/*****************************************************************************/
#if RTCSCFG_ENABLE_STATS
static uint32_t shell_netstat_print_stats( void *stats_ptr, const STAT_ENTRY_STRUCT * metadata, uint32_t elements) 
{ 
    uint32_t i,count=0; 
    uint32_t *stat_ptr; 
     
    for (i=0;i<elements;i++) { 
        stat_ptr = (uint32_t *) ((uint32_t) stats_ptr  + metadata[i].offset); 
        if (*stat_ptr) { 
            printf("   %ld %s\n", *stat_ptr, metadata[i].message); 
            count++; 
        } 
    } 
    if (count == 0) { 
        printf("   0 packets sent or received\n"); 
    } 
    return count; 
} 

/*****************************************************************************/
static uint32_t shell_netstat_check_pause(bool pause, uint32_t count) 
{ 
    if (pause && (count > 20))
    { 
        printf("---MORE---"); 
        getchar(); 
        printf("\n"); 
        count =0; 
    } 
    return count; 
} 
#endif
/************************************************************************
* NAME: Shell_netstat
* RETURNS: SHELL_EXIT_SUCCESS
* DESCRIPTION: The shell function to print RTCS netstats 
*************************************************************************/
int32_t Shell_netstat(int32_t argc, char *argv[] ) 
{ 
    bool     print_usage;
    bool     shorthelp = FALSE;
#if RTCSCFG_ENABLE_STATS
    bool     pause = FALSE; 
    uint32_t     count=0; 
#endif
    
    print_usage = Shell_check_help_request(argc, argv, &shorthelp ); 
    
    if (!print_usage)
    { 
#if RTCSCFG_ENABLE_STATS
        if (argc > 1)
        { 
            pause = strcmp(argv[1],"-p")==0; 
        } 
#endif
       
    #if BSPCFG_ENABLE_ENET_STATS  &&  (BSP_ENET_DEVICE_COUNT > 0)
        { 
            _enet_handle         handle; 
            ENET_STATS_PTR       enet = NULL; 

            handle = _mqx_get_io_component_handle(IO_ENET_COMPONENT); 
            while (handle)
            { 
                enet = ENET_get_stats(handle);
                if(enet)
                {
                    printf("\nENET (%x):\n",handle ); 
                    count += shell_netstat_print_stats(enet, enet_stat_entries , ELEMENTS_OF(enet_stat_entries)); 
                    count = shell_netstat_check_pause(pause, count); 
                    handle = ENET_get_next_device_handle(handle); 
                }
            } 
        } 
    #endif 

    #if RTCSCFG_ENABLE_IP_STATS && RTCSCFG_ENABLE_IP4 
        {
            IP_STATS_PTR       ip = IP_stats();
            if(ip)
            {
                printf("\nIPv4:\n"); 
                count += shell_netstat_print_stats(ip, ip_stat_entries, ELEMENTS_OF(ip_stat_entries) ); 
                count = shell_netstat_check_pause(pause,count); 
            }
        }
    #endif 

    #if RTCSCFG_ENABLE_IP6_STATS && RTCSCFG_ENABLE_IP6 
        {
            IP6_STATS_PTR       ip6 = IP6_stats();
            if(ip6)
            {
                printf("\nIPv6:\n"); 
                count += shell_netstat_print_stats(ip6, ip6_stat_entries, ELEMENTS_OF(ip6_stat_entries) ); 
                count = shell_netstat_check_pause(pause,count); 
            }
        }
    #endif 

    #if RTCSCFG_ENABLE_ICMP_STATS && RTCSCFG_ENABLE_ICMP && RTCSCFG_ENABLE_IP4
        { 
            ICMP_STATS_PTR       icmp = ICMP_stats(); 
            if(icmp)
            {
                printf("\nICMPv4:\n"); 
                count += shell_netstat_print_stats(icmp, icmp_stat_entries, ELEMENTS_OF(icmp_stat_entries) ); 
                count = shell_netstat_check_pause(pause,count); 
            }
        } 
    #endif 

    #if RTCSCFG_ENABLE_ICMP6_STATS && RTCSCFG_ENABLE_IP6
        { 
            ICMP6_STATS_PTR       icmp6 = ICMP6_stats(); 
            if(icmp6)
            {
                printf("\nICMPv6:\n"); 
                count += shell_netstat_print_stats(icmp6, icmp6_stat_entries, ELEMENTS_OF(icmp6_stat_entries) ); 
                count = shell_netstat_check_pause(pause,count); 
            }
        } 
    #endif 

    #if RTCSCFG_ENABLE_UDP_STATS && RTCSCFG_ENABLE_UDP
        { 
            UDP_STATS_PTR       udp = UDP_stats();
            if(udp)
            {
                printf("\nUDP:\n"); 
                count += shell_netstat_print_stats(udp, udp_stat_entries, ELEMENTS_OF(udp_stat_entries) ); 
                count = shell_netstat_check_pause(pause, count); 
            }
        } 
    #endif 

    #if RTCSCFG_ENABLE_TCP_STATS && RTCSCFG_ENABLE_TCP
        { 
            TCP_STATS_PTR       tcp = TCP_stats();
            if(tcp)
            {
                printf("\nTCP:\n"); 
                count += shell_netstat_print_stats(tcp, tcp_stat_entries, ELEMENTS_OF(tcp_stat_entries) ); 
                count = shell_netstat_check_pause(pause, count); 
            }
        } 
    #endif 

    #if RTCSCFG_ENABLE_IP6
        /*
         *   Neighbor Cache (ENET0):
         *       [0] fe80::d983:1777:dce7:68a8 = 00:e0:4c:68:23:43 (host)
         *
         *   IPv6 Prefix List (ENET0):
         *       [0] fe80::/64
         */
        { 
            _rtcs_if_handle                 ihandle;
            int                             i;
            uint32_t                        dev_num;
            RTCS6_IF_NEIGHBOR_CACHE_ENTRY   neighbor_cache_entry;
            RTCS6_IF_PREFIX_LIST_ENTRY      prefix_list_entry;
            char                            addr_str[RTCS_IP6_ADDR_STR_SIZE];

            for(dev_num=0; dev_num < IPCFG_DEVICE_COUNT; dev_num++)
            {
                ihandle = ipcfg_get_ihandle(dev_num);
                if (ihandle != NULL)
                {
                    /* Print IPv6 Neighbor Cache. */
                    for(i=0; RTCS6_if_get_neighbor_cache_entry(ihandle, i, &neighbor_cache_entry) == TRUE; i++)
                    {
                        if(i==0)
                        {
                             printf("\nIPv6 Neighbor Cache (Eth%d):\n", dev_num);
                        }
                        printf("   [%d] %s = %02x:%02x:%02x:%02x:%02x:%02x (%s) \n", i, 
                                inet_ntop(AF_INET6, &neighbor_cache_entry.ip_addr, addr_str, sizeof(addr_str)), 
                                neighbor_cache_entry.ll_addr[0], neighbor_cache_entry.ll_addr[1], neighbor_cache_entry.ll_addr[2], neighbor_cache_entry.ll_addr[3], neighbor_cache_entry.ll_addr[4], neighbor_cache_entry.ll_addr[5],
                                (neighbor_cache_entry.is_router == TRUE)? "router" : "host" );
                    }

                    /* Print IPv6 Prefix List. */
                    for(i=0; RTCS6_if_get_prefix_list_entry(ihandle, i, &prefix_list_entry) == TRUE; i++)
                    {
                        if(i==0)
                        {
                             printf("\nIPv6 Prefix List (Eth%d):\n", dev_num);
                        }
                        printf("   [%d] %s/%d\n", i, 
                                inet_ntop(AF_INET6, &prefix_list_entry.prefix, addr_str, sizeof(addr_str)), 
                                prefix_list_entry.prefix_length );
                    }                    
                }
            }
        } 
    #endif 
    } 

    if (print_usage)
    { 
        if (shorthelp)
        { 
            printf("%s [-p]\n", argv[0]); 
        } 
        else
        { 
            printf("Usage: %s [-p]\n", argv[0]); 
            printf("   -p Pause printing by pages.\n");
        } 
    } 
    return SHELL_EXIT_SUCCESS; 
} 

#endif /* SHELLCFG_USES_RTCS */ 

