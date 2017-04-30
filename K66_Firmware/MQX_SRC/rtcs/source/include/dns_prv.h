#ifndef __dns_prv_h__
#define __dns_prv_h__
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
*   Definitions for use by DNS services.
*
*
*END************************************************************************/

/* Messages carried by UDP are restricted to 512 bytes (not counting the IP or UDP headers).  
 * Longer messages (not supported) are truncated and the TC bit is set in  the header.*/
#define DNS_MESSAGE_SIZE         (512)  /* bytes.*/


/************************************************************************
*    DNS header [RFC1035, 4.1.1.]
*************************************************************************
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      ID                       |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    QDCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ANCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    NSCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ARCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*/  
typedef struct dns_header_struct
{
    unsigned char   ID[2];          /* Identifier used to match replies to outstanding queries */
    unsigned char   CONTROL[2];     /* Bits set to define query or response */
    unsigned char   QDCOUNT[2];     /* Number of entries in question section */
    unsigned char   ANCOUNT[2];     /* Number of RR's in answer section */
    unsigned char   NSCOUNT[2];     /* Number of Name Server RR's in the authority records   */
    unsigned char   ARCOUNT[2];     /* Number of RR's in the additional records */
}DNS_HEADER_STRUCT, * DNS_HEADER_STRUCT_PTR;

/*
** Domain Name Server Response Resource Record Structure
** This is defined to make writing a response easier.  This section of
** the response buffer should have a name first, this information second,
** and then response information of RDLENGTH after the structure.
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                                               |
    /                                               /
    /                      NAME                     /
    |                                               |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      TYPE                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     CLASS                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      TTL                      |
    |                                               |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                   RDLENGTH                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     RDATA                     /
    /                                               /
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*/  
typedef struct dns_rr_struct
{
   unsigned char         TYPE[2];
   unsigned char         CLASS[2];
   unsigned char         TTL[4];
   unsigned char         RDLENGTH[2];
}DNS_RR_STRUCT, * DNS_RR_STRUCT_PTR;


/************************************************************************
*    DNS Question section [RFC1035, 4.1.2.]
*************************************************************************
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                                               |
    /                     QNAME                     /
    /                                               /
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     QTYPE                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     QCLASS                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*/  
typedef struct dns_q_tail_struct
{
    unsigned char   QTYPE[2];
    unsigned char   QCLASS[2];
}DNS_Q_TAIL_STRUCT, * DNS_Q_TAIL_STRUCT_PTR;

/* CLASS values */
#define DNS_CLASS_IN                    0x0001   /* The Internet */

/* QDCOUNT Codes */
#define DNS_SINGLE_QUERY                0x0001


/* Response Codes  */
#define DNS_RESPONSE                    0x8000
#define DNS_RECURSION_DESIRED           0x0100
#define DNS_RECURSION_AVAILABLE         0x0080
#define DNS_AUTHORITATIVE_ANSWER        0x0400
#define DNS_TRUNCATION                  0x0200
#define DNS_NO_ERROR                    0x0000
#define DNS_FORMAT_ERROR                0x0001
#define DNS_SERVER_FAILURE              0x0002
#define DNS_NAME_ERROR                  0x0003
#define DNS_AUTHORITATIVE_NAME_ERROR    0x0403
#define DNS_QUERY_NOT_IMPLEMENTED       0x0004
#define DNS_QUERY_REFUSED               0x0005

/* Masks */
#define DNS_RCODE_MASK                  0x000F
#define DNS_OPCODE_MASK                 0x7800
#define DNS_COMPRESSED_NAME_MASK        0xC0
#define DNS_COMPRESSED_LOCATION_MASK    0x3FFF

bool dns_domains_equal(const char *hostname, char *res_domain);

#endif  /* __dns_prv_h__ */
