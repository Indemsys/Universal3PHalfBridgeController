/*HEADER**********************************************************************
*
* Copyright 2014 Freescale Semiconductor, Inc.
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
*   This file contains an implementation LLMNR Server/Responder (RFC4795).
*
*
*END************************************************************************/
#include <rtcs.h>

#if RTCSCFG_LLMNRSRV_DEBUG_MESSAGES==1
  #define LLMNRSRV_DEBUG(x) x
#if MQX_USE_IO_OLD
  #include <fio.h>
#else
  #include <stdio.h>
#endif
#else
  #define LLMNRSRV_DEBUG(x) 
#endif

#include "dns_prv.h"

/************************************************************************
*     Definitions
*************************************************************************/
#define LLMNRSRV_TASK_STACK     (2000)
#define LLMNRSRV_TASK_NAME      "LLMNR_server"
/* Size limits. */
#define LLMNRSRV_MAME_SIZE      (255)     /* RFC1035:To simplify implementations, the total length of a domain name (i.e.,
                                           * label octets and label length octets) is restricted to 255 octets or less.*/
#define LLMNRSRV_MESSAGE_SIZE   DNS_MESSAGE_SIZE    /* Messages carried by UDP are restricted to 512 bytes (not counting the IP
                                                    * or UDP headers). Longer messages (not supported) are truncated and the TC bit is set in
                                                    * the header.*/   

/* RFC 4795: The IPv4 link-scope multicast address a given responder listens to, and to which a
* sender sends queries, is 224.0.0.252.*/
#define LLMNRSRV_IP4_LINK_LOCAL_MULTICAST_ADDR   0xE00000FCL

/* RFC 4795: The IPv6 link-scope multicast address a given responder listens to,
* and to which a sender sends all queries, is FF02:0:0:0:0:0:1:3.*/
const struct in6_addr llmnr_ip6_linklocal_multicast_addr = {{{ 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03 }}};

/************************************************************************
*    [RFC 4795 2.1.1.]  LLMNR Header Format
*************************************************************************
   LLMNR queries and responses utilize the DNS header format defined in
   [RFC1035] with exceptions noted below:
        0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
      |                      ID                       |
      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
      |QR|   Opcode  | C|TC| T| Z| Z| Z| Z|   RCODE   |
      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
      |                    QDCOUNT                    |
      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
      |                    ANCOUNT                    |
      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
      |                    NSCOUNT                    |
      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
      |                    ARCOUNT                    |
      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+*/
typedef struct LLMNR_HEADER
{
    unsigned char ID[2];        /* A 16-bit identifier assigned by the program that generates
                                * any kind of query.  This identifier is copied from the query
                                * to the response and can be used by the sender to match
                                * responses to outstanding queries.  The ID field in a query
                                * SHOULD be set to a pseudo-random value. */
    unsigned char FLAGS[2];     /* Flags.*/
    unsigned char QDCOUNT[2];   /* An unsigned 16-bit integer specifying the number of entries
                                * in the question section.  A sender MUST place only one
                                * question into the question section of an LLMNR query.  LLMNR
                                * responders MUST silently discard LLMNR queries with QDCOUNT
                                * not equal to one.  LLMNR senders MUST silently discard LLMNR
                                * responses with QDCOUNT not equal to one.*/
    unsigned char ANCOUNT[2];   /* An unsigned 16-bit integer specifying the number of resource
                                * records in the answer section.  LLMNR responders MUST
                                * silently discard LLMNR queries with ANCOUNT not equal to zero.*/
    unsigned char NSCOUNT[2];   /* An unsigned 16-bit integer specifying the number of name
                                * server resource records in the authority records section.
                                * Authority record section processing is described in Section
                                * 2.9.  LLMNR responders MUST silently discard LLMNR queries
                                * with NSCOUNT not equal to zero.*/
    unsigned char ARCOUNT[2];   /* An unsigned 16-bit integer specifying the number of resource
                                * records in the additional records section.*/
} LLMNR_HEADER, * LLMNR_HEADER_PTR;

/* LLMNR Header Flags.*/
#define LLMNR_HEADER_FLAGS_QR       (0x8000)        /* Query/Response.  A 1-bit field, which, if set, indicates that
                                                    * the message is an LLMNR response; if clear, then the message
                                                    * is an LLMNR query.*/
#define LLMNR_HEADER_FLAGS_OPCODE   (0x7800)        /* A 4-bit field that specifies the kind of query in this
                                                    * message.  This value is set by the originator of a query and
                                                    * copied into the response.  This specification defines the
                                                    * behavior of standard queries and responses (opcode value of
                                                    * zero).  Future specifications may define the use of other
                                                    * opcodes with LLMNR.  LLMNR senders and responders MUST
                                                    * support standard queries (opcode value of zero).  LLMNR
                                                    * queries with unsupported OPCODE values MUST be silently
                                                    * discarded by responders.*/
#define LLMNR_HEADER_FLAGS_C        (0x0400)        /* Conflict.  When set within a query, the 'C'onflict bit
                                                    * indicates that a sender has received multiple LLMNR responses
                                                    * to this query.  In an LLMNR response, if the name is
                                                    * considered UNIQUE, then the 'C' bit is clear; otherwise, it
                                                    * is set.  LLMNR senders do not retransmit queries with the 'C'
                                                    * bit set.  Responders MUST NOT respond to LLMNR queries with
                                                    * the 'C' bit set, but may start the uniqueness verification
                                                    * process, as described in Section 4.2. */
#define LLMNR_HEADER_FLAGS_TC       (0x0200)        /* TrunCation.  The 'TC' bit specifies that this message was
                                                    * truncated due to length greater than that permitted on the
                                                    * transmission channel.  The 'TC' bit MUST NOT be set in an
                                                    * LLMNR query and, if set, is ignored by an LLMNR responder.
                                                    * If the 'TC' bit is set in an LLMNR response, then the sender
                                                    * SHOULD resend the LLMNR query over TCP using the unicast
                                                    * address of the responder as the destination address.  */
#define LLMNR_HEADER_FLAGS_T        (0x0100)        /* The 'T'entative bit is set in a response if the
                                                    * responder is authoritative for the name, but has not yet
                                                    * verified the uniqueness of the name.  A responder MUST ignore
                                                    * the 'T' bit in a query, if set.  A response with the 'T' bit
                                                    * set is silently discarded by the sender, except if it is a
                                                    * uniqueness query, in which case, a conflict has been detected
                                                    * and a responder MUST resolve the conflict as described in
                                                    * Section 4.1.*/
#define LLMNR_HEADER_FLAGS_RCODE    (0x000F)        /* Response code.  This 4-bit field is set as part of LLMNR
                                                    * responses.  In an LLMNR query, the sender MUST set RCODE to
                                                    * zero; the responder ignores the RCODE and assumes it to be
                                                    * zero.  The response to a multicast LLMNR query MUST have
                                                    * RCODE set to zero.  A sender MUST silently discard an LLMNR
                                                    * response with a non-zero RCODE sent in response to a
                                                    * multicast query. */

/* LLMNR Server context*/
typedef struct llmnrsrv_context
{
    volatile bool       b_run;
    _task_id            task_id;
    _rtcs_if_handle     interface;          /* Interface to configure LLMNR for. */
    volatile uint32_t   listensock_ip4;
    volatile uint32_t   listensock_ip6;
    char                *message;
    LLMNRSRV_HOST_NAME_STRUCT   *host_name_table;
    uint32_t                    host_name_table_size;
} LLMNRSRV_CONTEXT, * LLMNRSRV_CONTEXT_PTR;

/*
 * Parameters for server task
 */
typedef struct llmnrsrv_task_param
{
    uint32_t                handle;     /* [out] Server handle, non-zero if initialization was successful */
    LLMNRSRV_PARAM_STRUCT   *params;    /* [in] Server parameters */
}LLMNRSRV_TASK_PARAM;

static void llmnrsrv_task(void*, void*);
static void llmnrsrv_clean(LLMNRSRV_CONTEXT_PTR llmnr_context, bool user_release);
static void llmnrsrv_process_message( LLMNRSRV_CONTEXT  *llmnrsrv_context,  uint32_t listensock);
static LLMNRSRV_HOST_NAME_STRUCT *llmnrsrv_get_host_name_entry(LLMNRSRV_CONTEXT  *llmnrsrv_context, char * host_name);

/************************************************************************
* NAME: LLMNRSRV_init
* RETURNS: error code
* DESCRIPTION: Start the LLMNR server.
*************************************************************************/
uint32_t LLMNRSRV_init(LLMNRSRV_PARAM_STRUCT * params)
{
    LLMNRSRV_TASK_PARAM     server_params = {0};
    uint32_t                server_prio;
    
    /* Check input parameters.*/
    if(NULL == params)
    {
        _task_set_error(RTCSERR_INVALID_PARAMETER);
        return 0;
    }

    if (params->task_prio == 0)   /* Default priority.*/
    {
        server_prio = RTCSCFG_LLMNRSRV_SERVER_PRIO;
    }
    /* Server must run with lower priority than TCP/IP task. */
    else if (params->task_prio < _RTCSTASK_priority)
    {
        return(0);
    }
    else
    {
        server_prio = params->task_prio;
    }

    server_params.params = params;

    /* Run server task. */
    if (RTCS_task_create((char*)LLMNRSRV_TASK_NAME, 
                        server_prio, 
                        LLMNRSRV_TASK_STACK, 
                        llmnrsrv_task, 
                        (void*)&server_params) != RTCS_OK)
    {
        return(0);
    }
    return(server_params.handle);
}

/************************************************************************
* NAME: llmnrsrv_get_host_name_entry
* RETURNS: host name entry
* DESCRIPTION: Return host name table by requested host name.
*************************************************************************/
static LLMNRSRV_HOST_NAME_STRUCT *llmnrsrv_get_host_name_entry(LLMNRSRV_CONTEXT  *llmnrsrv_context, char * host_name)
{
    int                         i;
    LLMNRSRV_HOST_NAME_STRUCT   *result = NULL;
    for(i=0; i<llmnrsrv_context->host_name_table_size; i++)
    {
        if(dns_domains_equal(llmnrsrv_context->host_name_table[i].host_name, host_name) == TRUE)
        {
            result = &llmnrsrv_context->host_name_table[i];
        }
    }

    return result;
}

/************************************************************************
* NAME: llmnrsrv_process_message
* RETURNS: none
* DESCRIPTION: Process LLMNR message.
*************************************************************************/
static void llmnrsrv_process_message( LLMNRSRV_CONTEXT  *llmnrsrv_context,  uint32_t listensock)
{
    int32_t                     received;
    sockaddr                    addr;
    uint16_t                    addr_len;
    LLMNR_HEADER_PTR            llmnr_header;
    LLMNRSRV_HOST_NAME_STRUCT   *host_name_entry;
                
    addr_len = sizeof(addr);
    received = recvfrom(listensock, llmnrsrv_context->message, LLMNRSRV_MESSAGE_SIZE, 0, &addr, &addr_len);

    if((received != RTCS_ERROR) && (received >= sizeof(LLMNR_HEADER)))
    {
        llmnr_header = (LLMNR_HEADER_PTR) llmnrsrv_context->message;
                    
        if( ((mqx_ntohs(llmnr_header->FLAGS) & LLMNR_HEADER_FLAGS_QR) == 0) /* Request.*/
            /* LLMNR senders and responders MUST
            * support standard queries (opcode value of zero).  LLMNR
            * queries with unsupported OPCODE values MUST be silently
            * discarded by responders.*/
            && ((mqx_ntohs(llmnr_header->FLAGS) & LLMNR_HEADER_FLAGS_OPCODE) == 0)
            /* LLMNR responders MUST silently discard LLMNR queries with QDCOUNT
            * not equal to one.*/
            && (mqx_ntohs(llmnr_header->QDCOUNT) == 1)
            /* LLMNR responders MUST
            * silently discard LLMNR queries with ANCOUNT not equal to zero.*/
            && (mqx_ntohs(llmnr_header->ANCOUNT) == 0)
            /* LLMNR responders MUST silently discard LLMNR queries
            * with NSCOUNT not equal to zero.*/
            && (mqx_ntohs(llmnr_header->NSCOUNT) == 0)
        )
        {
            DNS_Q_TAIL_STRUCT_PTR   q_tail;
            char                    *req_hostname = llmnrsrv_context->message + sizeof(LLMNR_HEADER);
            int                     req_hostname_len;
                        
            LLMNRSRV_DEBUG(printf("LLMNR: Requested host name =%s .\n", req_hostname));

            /* Responders MUST NOT respond to LLMNR queries for names for which
            they are not authoritative.*/
            if((host_name_entry = llmnrsrv_get_host_name_entry(llmnrsrv_context, req_hostname)) != NULL)
            {
                req_hostname_len = strlen(req_hostname);
                q_tail = (DNS_Q_TAIL_STRUCT_PTR)(req_hostname + req_hostname_len + 1/*zero byte*/);

                /* Check Question Class. */
                if (mqx_ntohs(q_tail->QCLASS) == DNS_CLASS_IN)  
                {
                    char                *rr_name_ptr = (char *)(q_tail+1);
                    DNS_RR_STRUCT_PTR   rr_header = (DNS_RR_STRUCT_PTR)(rr_name_ptr+2);
                    unsigned char       *rr_data = (unsigned char *)(rr_header+1);
                    int                 send_size = ((char*)rr_name_ptr - (char *)llmnr_header);
                    uint16_t            ancount = 0;

                    /* Prepare query response.*/
                #if RTCSCFG_ENABLE_IP4       
                    if(mqx_ntohs(q_tail->QTYPE) == DNS_RR_TYPE_A)
                    {
                        LLMNRSRV_DEBUG(printf("LLMNR: A request.\n"));

                        (void) mqx_htonl(rr_data,  RTCS_if_get_addr(llmnrsrv_context->interface));
                        (void) mqx_htons(rr_header->RDLENGTH,   sizeof(_ip_address));
                        (void) mqx_htons(rr_header->TYPE,  DNS_RR_TYPE_A);
                        (void) mqx_htonl(rr_header->TTL, host_name_entry->host_name_ttl);
                        (void) mqx_htons(rr_header->CLASS, DNS_CLASS_IN);
                        (void) mqx_htons(rr_name_ptr, ((DNS_COMPRESSED_NAME_MASK<<8) | (DNS_COMPRESSED_LOCATION_MASK & sizeof(LLMNR_HEADER))));
                        send_size +=  sizeof(DNS_RR_STRUCT)+ sizeof(_ip_address) + 2;
                        ancount ++;
                    }
                    else
                #endif
                #if RTCSCFG_ENABLE_IP6       
                    if(mqx_ntohs(q_tail->QTYPE) == DNS_RR_TYPE_AAAA)
                    {
                        RTCS6_IF_ADDR_INFO  addr_info;
                        int                 n=0;
                        LLMNRSRV_DEBUG(printf("LLMNR: AAAA request.\n"));

                        while((RTCS6_if_get_addr(llmnrsrv_context->interface, n, &addr_info) == RTCS_OK) 
                            && ((send_size + sizeof(DNS_RR_STRUCT) + sizeof(in6_addr)) <= LLMNRSRV_MESSAGE_SIZE) )
                        {
                            _mem_copy(addr_info.ip_addr.s6_addr, rr_data, sizeof(addr_info.ip_addr.s6_addr));
                            (void) mqx_htons(rr_header->RDLENGTH,   sizeof(in6_addr));
                            (void) mqx_htons(rr_header->TYPE,  DNS_RR_TYPE_AAAA);
                            (void) mqx_htonl(rr_header->TTL, host_name_entry->host_name_ttl);
                            (void) mqx_htons(rr_header->CLASS, DNS_CLASS_IN);
                            (void) mqx_htons(rr_name_ptr, ((DNS_COMPRESSED_NAME_MASK<<8) | (DNS_COMPRESSED_LOCATION_MASK & sizeof(LLMNR_HEADER))));

                            send_size +=  sizeof(DNS_RR_STRUCT) + sizeof(in6_addr) + 2;
                            ancount ++;
                            
                            /* Update pointers */
                            rr_name_ptr = llmnrsrv_context->message + send_size;
                            rr_header = (DNS_RR_STRUCT_PTR)(rr_name_ptr + 2);
                            rr_data = (unsigned char *)(rr_header + 1);
                            n++;
                        }
                    }else
                #endif                                
                    {
                        goto DROP; /* Not supported query type.*/
                    }

                    if(ancount == 0)
                    {
                        goto DROP; /* No answer / no address.*/
                    }

                    /* Updtae LLMNR header response.*/
                    (void) mqx_htons(llmnr_header->ANCOUNT, ancount); /* One answer.*/
                    (void) mqx_htons(llmnr_header->FLAGS, mqx_ntohs(llmnr_header->FLAGS)|LLMNR_HEADER_FLAGS_QR /* Query response.*/
                                                                      |LLMNR_HEADER_FLAGS_C); /* The name is not considered unique.*/ 

                    sendto(listensock, llmnrsrv_context->message, send_size, 0, &addr, addr_len);
                }
            }
        }
    }
DROP:
    return;
}

/************************************************************************
* NAME: llmnrsrv_task
* RETURNS: none
* DESCRIPTION: LLMNR server task.
*************************************************************************/
static void llmnrsrv_task( void   *dummy,  void   *creator)
{
    LLMNRSRV_CONTEXT            llmnrsrv_context =  { 0 };
    LLMNRSRV_TASK_PARAM         *task_params = (LLMNRSRV_TASK_PARAM *)dummy;
    LLMNRSRV_PARAM_STRUCT       *params;
    uint16_t                    af;
    rtcs_fd_set                 readfds_init;
    int32_t                     retval;           
    bool                        user_release = FALSE;
    uint32_t                    error_code;
    LLMNRSRV_HOST_NAME_STRUCT   *host_name_table_entry;
    uint32_t                    host_name_buffer_len = 0;
    int                         i;
    char                        *host_name_ptr;

    /************************************************************************
    *    Process parameters.
    *************************************************************************/
    RTCS_ASSERT(task_params);   /* Checked by LLMNRSRV_init()*/

    params = task_params->params;

    RTCS_ASSERT(params);        /* Checked by LLMNRSRV_init()*/

    /************************************************************************
    *    Check input paramters.
    *************************************************************************/
    if((params->interface == 0) || (params->host_name_table == 0))
    {
        error_code = (uint32_t)RTCSERR_INVALID_PARAMETER;
        goto ERROR;
    }

    for(host_name_table_entry = params->host_name_table; host_name_table_entry->host_name != NULL; host_name_table_entry++)
    {
        uint32_t host_name_len = strlen(host_name_table_entry->host_name);

        if((host_name_len == 0) || (host_name_len > LLMNRSRV_MAME_SIZE))
        {
            error_code = (uint32_t)RTCSERR_INVALID_PARAMETER;
            goto ERROR;
        }
        else
        {
            host_name_buffer_len += host_name_len + 1 /*zero*/;
            /* Calculater number of host names.*/
            llmnrsrv_context.host_name_table_size++;
        }
    }

    if((llmnrsrv_context.host_name_table_size == 0) || (host_name_buffer_len == 0))
    {
        error_code = (uint32_t)RTCSERR_INVALID_PARAMETER;
        goto ERROR;
    }

    /* Allocate host name table. */
    if((llmnrsrv_context.host_name_table = _mem_alloc_system_zero((sizeof(LLMNRSRV_HOST_NAME_STRUCT) * llmnrsrv_context.host_name_table_size) + host_name_buffer_len)) == NULL)
    {
        error_code = (uint32_t)RTCSERR_OUT_OF_MEMORY;
        goto ERROR;
    }
    
    /* Init name table.*/
    memcpy(llmnrsrv_context.host_name_table, params->host_name_table, sizeof(LLMNRSRV_HOST_NAME_STRUCT) * llmnrsrv_context.host_name_table_size); 
    
    host_name_ptr = (char*)(llmnrsrv_context.host_name_table + llmnrsrv_context.host_name_table_size);
    for(i=0; i < llmnrsrv_context.host_name_table_size; i++)
    {
        uint32_t host_name_len = strlen(llmnrsrv_context.host_name_table[i].host_name);
    
        llmnrsrv_context.host_name_table[i].host_name = memcpy(host_name_ptr, llmnrsrv_context.host_name_table[i].host_name, host_name_len);
        host_name_ptr += host_name_len+1/*zero*/;

        /* Check optional TTL parameter.*/
        if(llmnrsrv_context.host_name_table[i].host_name_ttl == 0)
        {
            llmnrsrv_context.host_name_table[i].host_name_ttl = RTCSCFG_LLMNRSRV_HOSTNAME_TTL;
        }
    }

    /* Allocate resources.*/
    if( (((llmnrsrv_context.message = (char*)_mem_alloc_system(LLMNRSRV_MESSAGE_SIZE)))== NULL) )
    {
        error_code = (uint32_t)RTCSERR_OUT_OF_MEMORY;
        goto ERROR;
    }

    
    /* Check Address family.*/
    af = params->af & (AF_INET|AF_INET6);
    if(af == 0)
    {
        af = 0
        #if RTCSCFG_ENABLE_IP4
            | AF_INET
        #endif        
        #if RTCSCFG_ENABLE_IP6
            | AF_INET6
        #endif
            ;
    }

    llmnrsrv_context.interface = params->interface; 
    
    RTCS_FD_ZERO(&readfds_init);

    /************************************************************************
    *    Init Sockets.
    *************************************************************************/
#if RTCSCFG_ENABLE_IP4
    if(af & AF_INET)
    {
        struct ip_mreq      mreq; /* Multicast group information.*/
        sockaddr_in         sock_addr;
        _ip_address         interface_ip_addr = RTCS_if_get_addr(params->interface);
            
        if(interface_ip_addr != INADDR_ANY) /* Check if tnterface has bound IPv4 address.*/
        {
            /* Create listen socket */
            if((llmnrsrv_context.listensock_ip4 = socket(AF_INET, SOCK_DGRAM, 0)) == RTCS_SOCKET_ERROR)
            {
                llmnrsrv_context.listensock_ip4 = 0;
                error_code = (uint32_t)RTCSERR_CREATE_FAILED;
                goto ERROR;
            }

            /* Bind socket. */
            _mem_zero(&sock_addr, sizeof(sock_addr));
            sock_addr.sin_family = AF_INET;
            sock_addr.sin_port = RTCSCFG_LLMNRSRV_PORT;   
            if(bind(llmnrsrv_context.listensock_ip4, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) == RTCS_SOCKET_ERROR)
            {
                error_code = (uint32_t)RTCS_ERROR;
                goto ERROR;
            }

            /* Join multicast group. */
            mreq.imr_multiaddr.s_addr = LLMNRSRV_IP4_LINK_LOCAL_MULTICAST_ADDR;
            mreq.imr_interface.s_addr = interface_ip_addr;

            if(setsockopt(llmnrsrv_context.listensock_ip4, SOL_IGMP, RTCS_SO_IGMP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) != RTCS_OK) 
            {
                error_code = (uint32_t)RTCS_ERROR;
                goto ERROR;		
            }
                
            /* Add socket handle to fd_set to select */
            RTCS_FD_SET(llmnrsrv_context.listensock_ip4, &readfds_init);
        }
        /* else - not problem, try to start it on IPv6, if enabled.*/
    }
#endif

#if RTCSCFG_ENABLE_IP6
    if(af & AF_INET6)
    {
        ipv6_mreq       mreq6; /* Multicast group information.*/
        sockaddr_in6    sock_addr;

        /* Create listen socket */
        if((llmnrsrv_context.listensock_ip6 = socket(AF_INET6, SOCK_DGRAM, 0)) == RTCS_SOCKET_ERROR)
        {
            llmnrsrv_context.listensock_ip6 = 0;
            error_code = (uint32_t)RTCSERR_CREATE_FAILED;
            goto ERROR;
        }

        /* Bind socket. */
        _mem_zero(&sock_addr, sizeof(sock_addr));
        sock_addr.sin6_family = AF_INET6;
        sock_addr.sin6_port = RTCSCFG_LLMNRSRV_PORT;   
        if(bind(llmnrsrv_context.listensock_ip6, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) == RTCS_SOCKET_ERROR)
        {
            error_code = (uint32_t)RTCS_ERROR;
            goto ERROR;
        }
            
        IN6_ADDR_COPY(&llmnr_ip6_linklocal_multicast_addr, &mreq6.ipv6imr_multiaddr);
        mreq6.ipv6imr_interface = RTCS6_if_get_scope_id(params->interface);
            
        /* Join multicast group. */
        if(setsockopt(llmnrsrv_context.listensock_ip6 , SOL_IP6, RTCS_SO_IP6_JOIN_GROUP, (void *)&mreq6, sizeof(mreq6)) != RTCS_OK) 
        {
            error_code = (uint32_t)RTCS_ERROR;
            goto ERROR;			
        }
            
        /* Add socket handle to fd_set to select */
        RTCS_FD_SET(llmnrsrv_context.listensock_ip6, &readfds_init);
    }
#endif   

  
    if(readfds_init.fd_count == 0)
    {
        error_code = (uint32_t)RTCS_ERROR;
        goto ERROR;		
    }

    llmnrsrv_context.task_id = _task_get_id(); /* Save Task ID.*/
    /* Return context. */
    task_params->handle = (uint32_t)&llmnrsrv_context; 
    llmnrsrv_context.b_run = TRUE;
    RTCS_task_resume_creator(creator, RTCS_OK);

    /************************************************************************
    *    Run LLMNR Server 
    *************************************************************************/
    for(;;)
    {
        rtcs_fd_set   readfds = readfds_init; /* Init fds for select.*/

        /* Select on all sockets . */
        retval = select(readfds.fd_count, &readfds, 0, 0, 0);

        if(RTCS_ERROR == retval)
        {
            /* Socket is shutdown by release.*/
            user_release = TRUE;
            goto ERROR;
        }
        else /* Received data.*/
        {
        #if RTCSCFG_ENABLE_IP4  
            if(llmnrsrv_context.listensock_ip4 
                && RTCS_FD_ISSET(llmnrsrv_context.listensock_ip4, &readfds))
            {
                llmnrsrv_process_message(&llmnrsrv_context, llmnrsrv_context.listensock_ip4);
            }
        #endif
        #if RTCSCFG_ENABLE_IP6  
            if(llmnrsrv_context.listensock_ip6 
                && RTCS_FD_ISSET(llmnrsrv_context.listensock_ip6, &readfds))
            {
                llmnrsrv_process_message(&llmnrsrv_context, llmnrsrv_context.listensock_ip6);
            }
        #endif
        }
    } /* for (;;)*/

ERROR:
    if(user_release == FALSE)
    {
        RTCS_task_resume_creator(creator, error_code);
    }
    llmnrsrv_clean(&llmnrsrv_context, user_release);
}

/************************************************************************
* NAME: LLMNRSRV_release
* RETURNS: error code
* DESCRIPTION: Stop the LLMNR task and release sockets and memory.
*              Called by user application.
*************************************************************************/
uint32_t LLMNRSRV_release(uint32_t server_h)
{
    LLMNRSRV_CONTEXT_PTR  llmnr_context;

    llmnr_context = (LLMNRSRV_CONTEXT_PTR)server_h;
    
    if(llmnr_context)
    {
        if(llmnr_context->listensock_ip4)
        {
            shutdownsocket(llmnr_context->listensock_ip4, SHUT_RDWR);
        }

        if(llmnr_context->listensock_ip6)
        {
            shutdownsocket(llmnr_context->listensock_ip6, SHUT_RDWR);
        }
      
        /* Wait here until server task acknowledges release. */
        while(llmnr_context->b_run == TRUE)
        {
            _sched_yield();
        }

        _task_destroy(llmnr_context->task_id);
    }
 
    return RTCS_OK;
}
/************************************************************************
* NAME: llmnrsrv_clean
* RETURNS: none
* DESCRIPTION: Clean LLMNR context resources.
*************************************************************************/
static void llmnrsrv_clean(LLMNRSRV_CONTEXT_PTR llmnr_context, bool user_release)
{
    if(llmnr_context->listensock_ip4)
    {
        closesocket(llmnr_context->listensock_ip4);
    }

    if(llmnr_context->listensock_ip6)
    {
        closesocket(llmnr_context->listensock_ip6);
    }
    
    if(llmnr_context->message)
    {
        _mem_free(llmnr_context->message);
    }

    if(llmnr_context->host_name_table)
    {
        _mem_free(llmnr_context->host_name_table);
    }

    llmnr_context->b_run = FALSE; /* LLMNRSRV_release() monitors this flag. */
    
    if(user_release == TRUE)
    {
        _task_block(); /* LLMNRSRV_release() will destroy. */
    }
}
