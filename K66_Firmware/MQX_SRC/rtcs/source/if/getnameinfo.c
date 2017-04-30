/*HEADER**********************************************************************
*
* Copyright 2011-2014 Freescale Semiconductor, Inc.
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
* See license agreement file for full license terms including other restrictions.
*****************************************************************************
*
* Comments:
*
*   
*
*END************************************************************************/
/*
 * Portions Copyright (C) 2004, 2005, 2007  Internet Systems Consortium, Inc. ("ISC")
 * Portions Copyright (C) 1999-2001, 2003  Internet Software Consortium.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
#include <ctype.h>
#include <rtcs.h>
#include <addrinfo.h>
#if MQX_USE_IO_OLD
#include <fio.h>
#else
#include <stdio.h>
#include <string.h>
#endif



#define ENI_EXIT(code) \
	do { result = (code); \
		 goto cleanup;	\
	} while (0)

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : getnameinfo()
* Returned Value  : RTCS_OK or error code
* Comment: Name resolution can be by the Domain Name System (DNS), 
*          a local hosts file, or by other naming mechanisms.
*END*-----------------------------------------------------------------*/
int32_t getnameinfo( const struct sockaddr *sa, unsigned int salen, char *host, unsigned int hostlen, char *serv, unsigned int servlen, int flags)
{
	unsigned short          port;
	int                     family;
    unsigned int            socklen;
    int                     i;
	const void              *addr;
	char                    numserv[sizeof("65000")];
	char                    numaddr[RTCS_IP6_ADDR_STR_SIZE + RTCS_SCOPEID_STR_SIZE];
	int                     result;
    const char              *host_name = NULL;
    DNSCLN_RECORD_STRUCT    *dns_record_list = NULL;
    DNSCLN_PARAM_STRUCT     dns_params;
 
	if (sa == NULL)
    {
		ENI_EXIT(EAI_BADFLAGS);
    }

	family = sa->sa_family;

    switch (family)
    {
        case AF_INET:
            port = ((const struct sockaddr_in *)sa)->sin_port;
            addr = &((const struct sockaddr_in *)sa)->sin_addr.s_addr;
            socklen = sizeof(struct sockaddr_in);
            break;
        case AF_INET6:
            port = ((const struct sockaddr_in6 *)sa)->sin6_port;
            addr = ((const struct sockaddr_in6 *)sa)->sin6_addr.s6_addr;
            socklen = sizeof(struct sockaddr_in6);
            break;
        default:
            ENI_EXIT(EAI_FAMILY);
	}

    if (salen != socklen)
    {
		ENI_EXIT(EAI_FAMILY);
    }

    /* Port number.*/
#if 0 /*We do not support getservbyport function */ 
	proto = (flags & NI_DGRAM) ? "udp" : "tcp";
#endif

	if((serv == NULL) || (servlen == 0))
    {
		/* Caller does not want service. */
	}
	else
#if 0 /*We do not support getservbyport function */ 
     if ((flags & NI_NUMERICSERV) != 0 ||(sp = getservbyport(port, proto)) == NULL) 
#endif
	{
		snprintf(numserv, sizeof(numserv), "%d", ntohs(port));
		if ((strlen(numserv) + 1) > servlen)
		{	
			ENI_EXIT(EAI_MEMORY);
		}
		strcpy(serv, numserv);
	} 
#if 0 /*We do not support getservbyport function */
    else {
		if ((strlen(sp->s_name) + 1) > servlen)
		{
			ENI_EXIT(EAI_MEMORY);
		}
		strcpy(serv, sp->s_name);
	}
#endif

	if((host == NULL) || (hostlen == 0))
    {
        ENI_EXIT(EAI_BADFLAGS);
	} 
    else if(flags & NI_NUMERICHOST) /* Numeric.*/
    {
        /* If NI_NUMERICHOST set, then the numeric form of the hostname is returned.
         * (When not set, this will still happen in case the node's name cannot be determined.) */
NUMERICHOST:
		if (inet_ntop(family, addr, numaddr, sizeof(numaddr)) == NULL)
		{
			ENI_EXIT(EAI_SYSTEM);
		}
        
        /* Scope ID.*/
		if ((family == AF_INET6) && ((const struct sockaddr_in6 *)sa)->sin6_scope_id) 
		{
            char        *p = numaddr + strlen(numaddr);
            sprintf(p,"%%%u",((const struct sockaddr_in6 *)sa)->sin6_scope_id);
		}

		if ((strlen(numaddr) + 1) > hostlen)
        {
			ENI_EXIT(EAI_MEMORY);
        }

		strcpy(host, numaddr);
	} 
    else /* Host name */
    {
        if( (host_name = RTCS_hosts_get_name(sa)) == NULL) /* Check Local Hosts table.*/
        {  /* Resolve address using DNS */
            char            addr_str[sizeof("0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.ip6.arpa")]; 
            unsigned char   *addr_ptr;
            
            /* Prepare request.*/
            if(family == AF_INET)
            {
                _ip_address     ip4_addr = htonl(((sockaddr_in*)sa)->sin_addr.s_addr);

                addr_ptr = (unsigned char *)&ip4_addr;
                snprintf(addr_str, sizeof(addr_str), "%ld.%ld.%ld.%ld.in-addr.arpa", (uint32_t)addr_ptr[3], (uint32_t)addr_ptr[2], (uint32_t)addr_ptr[1], (uint32_t)addr_ptr[0]);
            }
            else /* AF_INET6 */
            {
                addr_ptr = (unsigned char *)&(((sockaddr_in6*)sa)->sin6_addr.s6_addr);
                snprintf(addr_str, sizeof(addr_str), "%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.ip6.arpa", 
                                    addr_ptr[15]&0xf, (addr_ptr[15]>>4)&0xf,
                                    addr_ptr[14]&0xf, (addr_ptr[14]>>4)&0xf,
                                    addr_ptr[13]&0xf, (addr_ptr[13]>>4)&0xf,
                                    addr_ptr[12]&0xf, (addr_ptr[12]>>4)&0xf,
                                    addr_ptr[11]&0xf, (addr_ptr[11]>>4)&0xf,
                                    addr_ptr[10]&0xf, (addr_ptr[10]>>4)&0xf,
                                    addr_ptr[9]&0xf, (addr_ptr[9]>>4)&0xf,
                                    addr_ptr[8]&0xf, (addr_ptr[8]>>4)&0xf,
                                    addr_ptr[7]&0xf, (addr_ptr[7]>>4)&0xf,
                                    addr_ptr[6]&0xf, (addr_ptr[6]>>4)&0xf,
                                    addr_ptr[5]&0xf, (addr_ptr[5]>>4)&0xf,
                                    addr_ptr[4]&0xf, (addr_ptr[4]>>4)&0xf,
                                    addr_ptr[3]&0xf, (addr_ptr[3]>>4)&0xf,
                                    addr_ptr[2]&0xf, (addr_ptr[2]>>4)&0xf,
                                    addr_ptr[1]&0xf, (addr_ptr[1]>>4)&0xf,
                                    addr_ptr[0]&0xf, (addr_ptr[0]>>4)&0xf);
            }

            dns_params.name_to_resolve = addr_str;  /* Address string to resolve (null-terminated string). */
            dns_params.type = DNS_RR_TYPE_PTR;      /* Domain name pointer is queried. */
            
            /* Get DNS server address.*/
            for(i=0; (DNSCLN_get_dns_addr(NULL, i, &dns_params.dns_server) == TRUE); i++)
            {
                /* Send DNS Query.*/
                dns_record_list = DNSCLN_query(&dns_params);
                /* Process DNS result.*/
                if((dns_record_list != NULL) && (dns_record_list->data_length > 0))
                {   /* Resolved.*/
                    dns_record_list->data[dns_record_list->data_length-1] = '\0'; /* Put end of line, just inj case.*/
                    host_name = dns_record_list->data;
                    break;
                }
            }
        }

        if(host_name == NULL)
        { /* NOT resolved.*/
            if (flags & NI_NAMEREQD)
            {
                /* If NI_NAMEREQD set, then an error is returned if the hostname cannot be determined. */
                ENI_EXIT(EAI_NONAME);
            }
            else
            {   
                /* The numeric form of the hostname is returned.*/
                goto NUMERICHOST;
            }
        }
        else /* Resolved.*/
        {
            if((strlen(host_name) + 1) > hostlen)
            {
                ENI_EXIT(EAI_MEMORY);
            }
            strcpy(host, host_name);

            if (flags & NI_NOFQDN)
            {
                /* If NI_NOFQDN set, return only the hostname part of the fully 
                 * qualified domain name.*/
                char    *chr;
                if ( (chr = strchr(host, '.')) != NULL)
                {
                    *chr = 0;	/* Limit by first dot.*/
				}
            }
        }
    }
	result = EAI_OK;

 cleanup:
    if(dns_record_list != NULL)
    {
        DNSCLN_record_list_free(dns_record_list);
    }

	return (result);
}




