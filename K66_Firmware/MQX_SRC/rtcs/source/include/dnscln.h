#ifndef __dnscln_h__
#define __dnscln_h__
/*HEADER**********************************************************************
*
* Copyright 2013 Freescale Semiconductor, Inc.
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
*   Definitions for use by the DNS Client.
*
*
*END************************************************************************/

/* DNS Resource Record types. RFC1035: */
typedef enum dns_rr_type
{
    DNS_RR_TYPE_A       = 0x0001,   /* IPv4 host address. */
    DNS_RR_TYPE_NS      = 0x0002,   /* An authoritative name server. */
    DNS_RR_TYPE_CNAME   = 0x0005,   /* Canonical name for an alias.*/
    DNS_RR_TYPE_NULL    = 0x000A,   /* Null Resource Record. */
    DNS_RR_TYPE_WKS     = 0x000B,   /* Service Description. */
    DNS_RR_TYPE_PTR     = 0x000C,   /* Domain name pointer, that maps an IP address to a host name for reverse lookups. */
    DNS_RR_TYPE_HINFO   = 0x000D,   /* Host information. */
    DNS_RR_TYPE_MINFO   = 0x000E,   /* Mailbox or mail list information. */
    DNS_RR_TYPE_MX      = 0x000F,   /* Mail Exchange. */
    DNS_RR_TYPE_TXT     = 0x0010,   /* Text Strings. */
    DNS_RR_TYPE_AAAA    = 0x001C    /* IPv6 host address. */
    /* Add new DNS Resource Records here.*/ 
} DNS_RR_TYPE;

#define DNSCLN_SERVER_PORT          (53)

/************************************************************************
* DNS Resorce Record, returned by DNSCLN_query().
*************************************************************************/
typedef struct dnscln_record_struct
{
    struct dnscln_record_struct *next;          /* Pointer to the next DNSCLN_RECORD_STRUCT.*/
    char                        *name;          /* Pointer to a host name string (null-terminated). */
    DNS_RR_TYPE                 type;           /* Type of DNS Resource Record. */
    uint32_t                    ttl;            /* Time to Live of DNS Resource Record, in seconds.*/
    uint16_t                    data_length;    /* Length of DNS Resource Record data, in bytes.*/
    char                        *data;          /* Pointer to DNS Resource Record data in network byte order. Its size is defined by data_length.*/
} DNSCLN_RECORD_STRUCT, *DNSCLN_RECORD_STRUCT_PTR;

/************************************************************************
* DNS Client Input Parameters 
*************************************************************************/
typedef struct dnscln_param_struct
{
    sockaddr        dns_server;         /* Socket address of the remote DNS server to connect to. */
    const char      *name_to_resolve;   /* Host name to resolve (null-terminated string). */
    DNS_RR_TYPE     type;               /* DNS Resource Record Type that is queried. */
} DNSCLN_PARAM_STRUCT, *DNSCLN_PARAM_STRUCT_PTR;

#ifdef __cplusplus
extern "C" {
#endif

extern void DNSCLN_record_list_free(DNSCLN_RECORD_STRUCT *record_list);
extern DNSCLN_RECORD_STRUCT_PTR DNSCLN_query(DNSCLN_PARAM_STRUCT *params);
extern bool DNSCLN_get_dns_addr(_rtcs_if_handle ihandle /*Optional.*/, uint32_t n, sockaddr *dns_server);

#ifdef __cplusplus
}
#endif

#endif /* __dnscln_h__ */

