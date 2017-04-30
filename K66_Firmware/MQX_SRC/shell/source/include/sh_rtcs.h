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
*   This file contains the RTCS shell.
*
*
*END************************************************************************/

#ifndef __sh_rtcs_h__
#define __sh_rtcs_h__

#include "ppp.h"

#define MAX_HOSTNAMESIZE     64
#define SHELL_IPCONFIG_DHCPCLN6_WAIT (20)

#define eaddrassign(p,x)   ((p)[0] = (x)[0], \
                            (p)[1] = (x)[1], \
                            (p)[2] = (x)[2], \
                            (p)[3] = (x)[3], \
                            (p)[4] = (x)[4], \
                            (p)[5] = (x)[5]  \
                           )

#define eaddriszero(p)   ( ((p)[0] == 0) && \
                           ((p)[1] == 0) && \
                           ((p)[2] == 0) && \
                           ((p)[3] == 0) && \
                           ((p)[4] == 0) && \
                           ((p)[5] == 0)    \
                          )

/*
** Function prototypes 
*/
#ifdef __cplusplus
extern "C" {
#endif

#if RTCSCFG_ENABLE_PPP
typedef struct shell_ppp_link { 
   char*           device_name;             /* system device name, like "ittyd:" */
  _ppp_handle      PPP_HANDLE;              /* PPP connection handle. */
  _rtcs_if_handle  PPP_IF_HANDLE;           /* PPP interface. */
  _ip_address      PPP_LOCAL_ADDRESS;       /* Address will be assigned by remote host for client mode. */
  _ip_address      PPP_REMOTE_ADDRESS;      /* Address will be assigned by remote host for client mode. */
  _ip_address      PPP_GATE_ADDR;
   char*           LOCAL_NAME;              /* Board as client login name. */
   char*           LOCAL_PASSWORD;          /* Board as client password.  */
   char*           REMOTE_NAME;             /* Board as server connect login name. */
   char*           REMOTE_PASSWORD;         /* Board as server connecr password. */
   PPP_SECRET      rsecrets[2];             /* Save here PAP information. */
   LWSEM_STRUCT    PPP_SEM;                 /* PPP semafor. */
   uint32_t         listen_flag;             /* Set it to "1" if we start PPP like server or "0" if we start PPP like client. */
   _task_id        hstid;                   /* Handshake task ID */
   _task_id        conntid;                 /* Checking connection task ID */
} SHELL_PPP_LINK, * SHELL_PPP_LINK_PTR;

#define PPP_PRINT_PREFIX   "PPP: "
#if (__CODEWARRIOR__)
   static void PPP_PRINTF(MQX_FILE_PTR _file, const char *_format, ...)
   {
         va_list argptr;
         va_start(argptr, _format);
         fprintf(_file, PPP_PRINT_PREFIX);
         vfprintf(_file, _format, argptr);
         fprintf(_file, "\n");
         va_end(argptr);
   }
#else
   #define PPP_PRINTF( _file, _format, ... )        \
       do {fprintf( _file, "%s"_format "%s", PPP_PRINT_PREFIX, ##__VA_ARGS__, "\n" );} while( 0 )
#endif

#endif


extern bool Shell_parse_ip_address( char *arg, _ip_address  *ipaddr_ptr);
extern bool Shell_parse_netmask( char *arg, _ip_address  *ipaddr_ptr);

extern int32_t Shell_netstat(int32_t argc, char *argv[] );
extern int32_t Shell_ping(int32_t argc, char *argv[] ); 
extern int32_t Shell_telnetcln(int32_t argc, char *argv[] );
extern int32_t Shell_FTP_client(int32_t argc, char *argv[] );
extern int32_t Shell_tftpcln(int32_t argc, char *argv[] );
extern int32_t Shell_get_host_by_name(int32_t argc, char *argv[] ); 
extern int32_t Shell_getname(int32_t argc, char *argv[] );
extern int32_t Shell_telnetsrv(int32_t argc, char *argv[]);
extern int32_t Shell_echosrv(int32_t argc, char *argv[] );
extern int32_t Shell_echo(int32_t argc, char *argv[] );
extern int32_t Shell_SNMPd(int32_t argc, char *argv[] );
extern int32_t Shell_ftpsrv(int32_t argc, char *argv[] );
extern int32_t Shell_tftpsrv(int32_t argc, char *argv[]);
extern int32_t Shell_type(int32_t argc, char *argv[] );
extern int32_t Shell_walkroute(int32_t argc, char *argv[] );
extern int32_t Shell_gate(int32_t argc, char *argv[] ); 
extern int32_t Shell_getroute(int32_t argc, char *argv[] );
extern int32_t Shell_smtp (int32_t argc, char *argv[]);

extern int32_t Shell_arpdisp(int32_t argc, char *argv[] );
extern int32_t Shell_arpadd(int32_t argc, char *argv[] );
extern int32_t Shell_arpdel(int32_t argc, char *argv[] );
extern int32_t Shell_sendto(int32_t argc, char *argv[] );

extern int32_t Shell_nat(int32_t argc, char *argv[] );
extern int32_t Shell_dnat(int32_t argc, char *argv[] );
extern int32_t Shell_natinfo(int32_t argc, char *argv[] );
extern int32_t Shell_natinit(int32_t argc, char *argv[] );
extern int32_t Shell_dnat_delet_rule(int32_t argc, char *argv[] );
extern int32_t Shell_dhcptest(int32_t argc, char *argv[] );

extern int32_t Shell_Clock_server_start(int32_t argc, char *argv[] );

extern int32_t Shell_ipconfig(int32_t argc, char *argv[] );
extern int32_t Shell_iwconfig(int32_t argc, char *argv[] );
extern int32_t Shell_llmnrsrv(int32_t argc, char *argv[] );

/* PPP related shell commands. */
extern int32_t Shell_ppp(int32_t argc, char *argv[] );


#ifdef __cplusplus
}
#endif

#endif
/*EOF*/
