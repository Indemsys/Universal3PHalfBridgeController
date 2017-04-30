/*HEADER**********************************************************************
*
* Copyright 2011-2014  Freescale Semiconductor, Inc.
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
*   getaddrinfo() is used to get a list of IP addresses and port
*   numbers for host hostname and service servname. 
*
*
*END************************************************************************/

/*
 * Copyright (C) 2004-2007, 2009  Internet Systems Consortium, Inc. ("ISC")
 * Copyright (C) 1999-2001  Internet Software Consortium.
 *
 * This code is derived from software contributed to ISC by
 * Berkeley Software Design, Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC AND BERKELEY SOFTWARE DESIGN, INC.
 * DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE
 * FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <string.h>
#include <ctype.h>
#include <rtcs.h>
#include <addrinfo.h>
#include <stdlib.h>


#define SA(addr)    ((struct sockaddr *)(addr))
#define SIN(addr)   ((struct sockaddr_in *)(addr))
#define SIN6(addr)  ((struct sockaddr_in6 *)(addr))

/*! \struct addrinfo
 */
static struct addrinfo
    *ai_reverse(struct addrinfo *oai),
    *ai_clone(struct addrinfo *oai, int family),
    *ai_alloc(int family, int addrlen);

static char *RTCS_strsep(char **stringp, const char *delim);
#if RTCSCFG_ENABLE_IP4
static int add_ipv4(const char *hostname, int flags, struct addrinfo **aip, int socktype, int port);
#endif
#if RTCSCFG_ENABLE_IP6
static int add_ipv6(const char *hostname, int flags, struct addrinfo **aip, int socktype, int port);
#endif
static void set_order(int, int (**)(const char *, int, struct addrinfo **, int, int));
static int add_ip(int family, const char *hostname, int flags, struct addrinfo **aip, int socktype, int port);
static char * strdup( const char *s);


#define FOUND_IPV4  0x1
#define FOUND_IPV6  0x2
#define FOUND_MAX   2

#define ISC_AI_MASK (AI_PASSIVE|AI_CANONNAME|AI_NUMERICHOST)

#define GAI_EXIT(code) \
    do { result = (code);           \
        goto cleanup;   \
    } while (0)



/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : getaddrinfo
* Returned Value  : RTCS_OK or error code
* Comments        :
*        Get a list of IP addresses and port numbers for host hostname and service servname.
*
*END*-----------------------------------------------------------------*/
int32_t getaddrinfo
(
    const char              *hostname,  /* host name or IP or NULL                          */
    const char              *servname,  /* service name or port number                      */
    const struct addrinfo   *hints,     /* set flags                                        */
    struct addrinfo         **res       /* [OUT]list of address structures                  */
)

{
    const char      *proto = NULL;
    int             family; 
    int             socktype; 
    int             flags; 
    int             protocol;
    struct addrinfo *ai;
    struct addrinfo *ai_list;
    uint16_t        port; 
    uint16_t        err; 
    uint16_t        i;
    
    
    int (*net_order[FOUND_MAX+1])(const char *, int, struct addrinfo **,int, int);

    /*hostname and send name can not be NULL in same time*/
    if (hostname == NULL && servname == NULL)
    {
        return (EAI_NONAME);
    }

    proto = NULL;
    if (hints != NULL) 
    {
        if ((hints->ai_flags & ~(ISC_AI_MASK)) != 0)
        {
            return (EAI_BADFLAGS);
        }
        if (hints->ai_addrlen || hints->ai_canonname || hints->ai_addr || hints->ai_next) 
        {
            _task_errno = MQX_EINVAL;
            return (EAI_SYSTEM);
        }

        family = hints->ai_family;
        socktype = hints->ai_socktype;
        protocol = hints->ai_protocol;
        flags = hints->ai_flags;
        
        /* looking for correct protocol depended on family and socket type */
        switch (family) {

            case AF_UNSPEC:
        #if RTCSCFG_ENABLE_IP4
            case AF_INET:
        #endif
        #if RTCSCFG_ENABLE_IP6
            case AF_INET6:
        #endif
                if(hints->ai_socktype == 0)
                {
                    break;
            
                }else if(hints->ai_socktype == SOCK_STREAM) {
                
                    proto = "tcp";
                
                }else if(hints->ai_socktype == SOCK_DGRAM)  {
                    
                    proto = "udp";
            
                }else{
                    return (EAI_SOCKTYPE);
                }
            break;

        default:
            return (EAI_FAMILY);

        }/* end of  switch (family) */
    } else {
    
        protocol = 0;
        family = 0;
        socktype = 0;
        flags = 0;
    
    }/* end of (hints != NULL) */

    if(proto !=NULL)
    {
        protocol = (strcmp(proto,"tcp")) ? 2 : 1;

    }

    /*
     * Ok, only AF_INET and AF_INET6 left.
     */
     
    /*************************************/

    /*
     * First, look up the service port if it was
     * requested.  If the socket type wasn't specified, then
     * try and figure it out.
     */
    if (servname != NULL) {
        char *e;

        port = strtol(servname, &e, 10);
        if (*e == '\0')                     //*e - end pointer
        {   /* Port number.*/
            if (socktype == 0)              //Not sure that it is necessary here
            {
                return (EAI_SOCKTYPE);
            }
            /* When port will be in network endian, do mqx_htons(&tmp,(uint16_t) port);   */
        }
        else
        {
            /* We use only port number. We do not use a service name */
            return (EAI_SERVICE);           
        }/*end of (*e == '\0')*/
    }
    else
    {
        port = 0;
    } /* end (servname != NULL) */

    /*
     * Next, deal with just a service name, and no hostname.
     * (we verified that one of them was non-null up above).
     */
    
    ai_list = NULL;
                 
    if (hostname == NULL && (flags & AI_PASSIVE) != 0) 
    {
    #if RTCSCFG_ENABLE_IP4
        if (family == AF_INET || family == 0) 
        {
        
            /* Allocate an addrinfo structure, and a sockaddr structure */      
            ai = ai_alloc(AF_INET, sizeof(struct sockaddr_in));

            if (ai == NULL)
            {
                freeaddrinfo(ai_list);
                return (EAI_MEMORY);
            }
            ai->ai_socktype = socktype;
            ai->ai_protocol = protocol;
            SIN(ai->ai_addr)->sin_port = port;
            ai->ai_next = ai_list;
            ai_list = ai;
        }
    #endif
    #if RTCSCFG_ENABLE_IP6
        if (family == AF_INET6 || family == 0) 
        {
            ai = ai_alloc(AF_INET6, sizeof(struct sockaddr_in6));
            if (ai == NULL) 
            {
                freeaddrinfo(ai_list);
                return (EAI_MEMORY);
            }
            ai->ai_socktype = socktype;
            ai->ai_protocol = protocol;
            SIN6(ai->ai_addr)->sin6_port = port;
            ai->ai_next = ai_list;
            ai_list = ai;
        }
    #endif

        *res = ai;
        return (0);
    }/* end of (hostname == NULL && (flags & AI_PASSIVE) != 0) */


    /*
     * If the host is not NULL and the family isn't specified or AI_NUMERICHOST 
     * specified, check first to see if it is a numeric address.
     */
    if (hostname != NULL /*&& (family == 0 || (flags & AI_NUMERICHOST) != 0)*/) 
    {
        char abuf[sizeof(struct in6_addr)];
        char nbuf[NI_MAXHOST];
        int addrsize, addroff;
        char *p, *ep;
        char ntmp[NI_MAXHOST];
        uint32_t scopeid;

        /*
         * Scope identifier portion.
         * scope id must be a decimal number
         */
        ntmp[0] = '\0';
        if (strchr(hostname, '%') != NULL) 
        {
            strncpy(ntmp, hostname, sizeof(ntmp) - 1);
            ntmp[sizeof(ntmp) - 1] = '\0';
            /*Returns a pointer to the first occurrence of character '%'.*/         
            p = strchr(ntmp, '%');
            ep = NULL;

            /*
             * Vendors may want to support non-numeric
             * scopeid around here.
             */

            if (p != NULL)
            {
                scopeid = (uint32_t)strtoul(p + 1, &ep, 10);
            }               
            if (p != NULL && ep != NULL && ep[0] == '\0')
            {
                *p = '\0';
            }
            else
            {
                ntmp[0] = '\0';
                scopeid = 0;
            }
        } 
        else
        {
            scopeid = 0;
        } /* end of  (strchr(hostname, '%') != NULL) */ 


          /*
           * Converts a human readable IP address into an address 
           * family appropriate 32bit or 128bit binary structure.
           */
        if (inet_pton(AF_INET, hostname, (struct in_addr *)abuf,sizeof( *((struct in_addr *)abuf)))== RTCS_OK)
        {
        #if RTCSCFG_ENABLE_IP6
            if (family == AF_INET6) 
            {
                /* Convert to a V4 mapped address. */
                struct in6_addr *a6 = (struct in6_addr *)abuf;
                memcpy(&a6->s6_addr[12], &a6->s6_addr[0], 4);
                memset(&a6->s6_addr[10], 0xff, 2);
                memset(&a6->s6_addr[0], 0, 10);
                goto inet6_addr;
            }
        #endif
            addrsize = sizeof(struct in_addr);
            addroff = (char *)(&SIN(0)->sin_addr) - (char *)0;
            family = AF_INET;
            goto common;
        } else if (ntmp[0] != '\0' && inet_pton(AF_INET6, ntmp, abuf,sizeof(struct in6_addr)) == RTCS_OK)
        {
            if (family && family != AF_INET6)
            {
                return (EAI_NONAME);
            }
            addrsize = sizeof(struct in6_addr);
            addroff = (char *)(&SIN6(0)->sin6_addr) - (char *)0;
            family = AF_INET6;
            goto common;
        } else if (inet_pton(AF_INET6, hostname, abuf,sizeof(struct in6_addr)) == RTCS_OK) 
        {
            if (family != 0 && family != AF_INET6)
            {
                return (EAI_NONAME);
            }
#if RTCSCFG_ENABLE_IP6
inet6_addr:
#endif
            addrsize = sizeof(struct in6_addr);
            addroff = (char *)(&SIN6(0)->sin6_addr) - (char *)0;
            family = AF_INET6;

common:
            ai = ai_clone(ai_list, family);
            if (ai == NULL)
            {
                return (EAI_MEMORY);
            }
            ai_list = ai;
            ai->ai_socktype = socktype;
            //ai->ai_protocol = protocol;
            SIN(ai->ai_addr)->sin_port = port;
            memcpy((char *)ai->ai_addr + addroff, abuf, addrsize);

            if (ai->ai_family == AF_INET6)
            {
                SIN6(ai->ai_addr)->sin6_scope_id = scopeid;
            }   

            if (flags & AI_CANONNAME) 
            {
                if (getnameinfo(    ai->ai_addr,
                                    ai->ai_addrlen, 
                                    nbuf, 
                                    sizeof(nbuf),
                                    NULL, 
                                    0,
                                    NI_NUMERICHOST
                                ) == 0) 
                {
                    ai->ai_canonname = strdup(nbuf);
                    if (ai->ai_canonname == NULL) 
                    {
                        freeaddrinfo(ai_list);
                        return (EAI_MEMORY);
                    }
                } else  
                {
                    /* XXX raise error? */
                    ai->ai_canonname = NULL;
                }
            }/*end of (flags & AI_CANONNAME)*/
            goto done;
        } else if ((flags & AI_NUMERICHOST) != 0) 
        {
            return (EAI_NONAME);
        }
    }

    if ((flags & AI_NUMERICHOST) == 0)
    {
        set_order(family, net_order);
        for (i = 0; i < FOUND_MAX; i++)
        {
            if (net_order[i] == NULL)
                break;
            err = (net_order[i])(hostname, flags, &ai_list, socktype, port);
            if (err != 0)
            {
                return (err);
            }
        }
    }

    if (ai_list == NULL)
        return (EAI_NODATA);

done:
    ai_list->ai_protocol = protocol;
    ai_list->ai_flags = flags;

    ai_list = ai_reverse(ai_list);

    *res = ai_list;
    return (0);
}

static char *RTCS_strsep    (
                                char **stringp, 
                                const char *delim
                            ) 
{
    char *string = *stringp;
    char *s;
    const char *d;
    char sc, dc;

    if (string == NULL)
        return (NULL);

    for (s = string; *s != '\0'; s++) {
        sc = *s;
        for (d = delim; (dc = *d) != '\0'; d++)
            if (sc == dc) {
                *s++ = '\0';
                *stringp = s;
                return (string);
            }
    }
    *stringp = NULL;
    return (string);
}

static void set_order   (
                            int family, 
                            int (**net_order)(const char *, int, struct addrinfo **,int, int)
                        )
{
    char *order, *tok;
    int found;

    if (family) 
    {
        switch (family) 
        {
        #if RTCSCFG_ENABLE_IP4
            case AF_INET:
                *net_order++ = add_ipv4;
                break;
        #endif
        #if RTCSCFG_ENABLE_IP6
            case AF_INET6:
                *net_order++ = add_ipv6;
                break;
        #endif
        }
    } else {
        order = NET_ORDER;//"inet";// : inet6";//getenv("NET_ORDER");
        found = 0;
        while (order != NULL) 
        {
            /*
             * We ignore any unknown names.
             */
            tok = RTCS_strsep(&order, ":");
        #if RTCSCFG_ENABLE_IP6
            if (strcasecmp(tok, "inet6") == 0) 
            {
                if ((found & FOUND_IPV6) == 0)
                    {
                        *net_order++ = add_ipv6;
                    }
                        
                found |= FOUND_IPV6;
            } 
            else
        #endif
        #if RTCSCFG_ENABLE_IP4
            if (strcasecmp(tok, "inet") == 0 || strcasecmp(tok, "inet4") == 0) 
            {
                if ((found & FOUND_IPV4) == 0)
                {
                    *net_order++ = add_ipv4;
                }
                found |= FOUND_IPV4;
            }
            else
        #endif
            {};
        }

        /*
         * Add in anything that we didn't find.
         */
    #if RTCSCFG_ENABLE_IP4
        if ((found & FOUND_IPV4) == 0)
            *net_order++ = add_ipv4;
    #endif
    #if RTCSCFG_ENABLE_IP6
        if ((found & FOUND_IPV6) == 0)
            *net_order++ = add_ipv6;
    #endif
    }
    *net_order = NULL;
    return;
}

#if RTCSCFG_ENABLE_IP4
static char v4_loop[4] = { 127, 0, 0, 1 };

static int add_ipv4
(
    const char      *hostname, 
    int             flags, 
    struct addrinfo **aip,
    int             socktype, 
    int             port
)
{
    return add_ip(AF_INET, hostname, flags, aip, socktype, port);
}
#endif

#if RTCSCFG_ENABLE_IP6
static char v6_loop[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };

static int add_ipv6 (
                        const char          *hostname, 
                        int                 flags, 
                        struct addrinfo     **aip,
                        int                 socktype, 
                        int                 port
                    )
{

    return add_ip(AF_INET6, hostname, flags, aip, socktype, port);
}
#endif

/************************************************************************
* NAME: add_ip
* RETURNS: 0 if OK.
* DESCRIPTION: Try to Resolve IP address using DNS Client.
*************************************************************************/
static int add_ip(int family, const char *hostname, int flags, struct addrinfo **aip, int socktype, int port)
{
    struct addrinfo         *ai;
    int                     result = EAI_OK;
    char                    *ip_loop;
    _mem_size               ip_length;
    DNS_RR_TYPE              dns_type; 
    DNSCLN_RECORD_STRUCT    *dns_record_list = NULL;
    DNSCLN_PARAM_STRUCT     dns_params;
    int                     i;
    char                    *fqdn = NULL;

#if RTCSCFG_ENABLE_IP4
    if(family == AF_INET)
    {
        ip_loop = v4_loop;
        ip_length = sizeof(in_addr);
        dns_type = DNS_RR_TYPE_A;
    }
    else 
#endif
#if RTCSCFG_ENABLE_IP6
    if(family == AF_INET6)
    {
        ip_loop = v6_loop;
        ip_length = sizeof(in6_addr);
        dns_type = DNS_RR_TYPE_AAAA;
    }
    else
#endif
    {
        GAI_EXIT(EAI_FAIL);
    };
    
    if (hostname == NULL && (flags & AI_PASSIVE) == 0) 
    {
        /*  In this case to get connection 
        *   inside host using LOOPBACK interface . */
        ai = ai_clone(*aip, family);
        if (ai == NULL) 
        {
            freeaddrinfo(*aip);
            GAI_EXIT(EAI_MEMORY);
        }
        *aip = ai;

        ai->ai_socktype = socktype;
        SIN(ai->ai_addr)->sin_port = port;
        _mem_copy(ip_loop, &SIN(ai->ai_addr)->sin_addr, ip_length);

        GAI_EXIT(EAI_OK); /* Resolved. */
    } 
    else /* If the hostname is not NULL, lets try to resolve it. */     
    {
        void  *ip_addr;
       
        /* Check Hosts.*/
        if((ip_addr = RTCS_hosts_get_addr(hostname, family)) != NULL)
        {
            ai = ai_clone(*aip, family);
            if (ai == NULL) 
            {
                freeaddrinfo(*aip);
                GAI_EXIT(EAI_MEMORY);
            }
            *aip = ai;

            ai->ai_socktype = socktype;
            SIN(ai->ai_addr)->sin_port = port;

            _mem_copy(ip_addr, &SIN(ai->ai_addr)->sin_addr, ip_length);

            if (flags & AI_CANONNAME) 
            {
                /* If using /etc/hosts, the hostname on the list is
                 * considered as the canonical name.*/

                ai->ai_canonname = strdup(hostname);
                if (ai->ai_canonname == NULL)
                {
                    freeaddrinfo(*aip);
                    GAI_EXIT(EAI_MEMORY);
                }
            }
            
            GAI_EXIT(EAI_OK); /* Resolved. */
        }
        else /* Use RTCS DNS Client to get addr by name.    */
        {
            const char  *dns_suffix;
 
            if((strchr(hostname, '.') == NULL) && ((dns_suffix = RTCS_get_primary_dns_suffix()) != NULL ))
            {
                /* When a query is made for a hostname that does not have a dot, the hostname is extended 
                * by Primary DNS suffix to a fully qualified domain name before the query is passed 
                * to the name server.*/
                size_t  hostname_size = strlen(hostname);
                size_t  dns_suffix_size = strlen(dns_suffix);
                size_t  fqdn_size = hostname_size + 1/*.*/ + dns_suffix_size + 1/*0*/;
                
                fqdn = _mem_alloc(fqdn_size);
                if(fqdn)
                {
                    _mem_copy((void*)hostname, &fqdn[0], hostname_size);
                    fqdn[hostname_size] = '.';
                    _mem_copy((void*)dns_suffix, &fqdn[hostname_size+1], dns_suffix_size+1);
                    
                    dns_params.name_to_resolve = fqdn;      /* Host name to resolve (null-terminated string). */ 
                }
                else
                {
                    freeaddrinfo(*aip);
                    GAI_EXIT(EAI_MEMORY);
                }
            }
            else
            {
                dns_params.name_to_resolve = hostname;      /* Host name to resolve (null-terminated string). */                
            }

            dns_params.type = dns_type;                     /* DNS Resource Record Type that is queried. */
            
            /* Get DNS server address.*/
            for(i=0; (DNSCLN_get_dns_addr(NULL, i, &dns_params.dns_server) == TRUE); i++)
            {
                /* Send DNS Query.*/
                dns_record_list = DNSCLN_query(&dns_params);
                /* Process DNS result.*/
                if(dns_record_list)
                {   /* Resolved.*/
                    DNSCLN_RECORD_STRUCT    *dns_record = dns_record_list;
                    
                    do
                    {
                        ai = ai_clone(*aip, family);
                        if (ai == NULL) 
                        {
                            freeaddrinfo(*aip);
                            GAI_EXIT(EAI_MEMORY);
                        }
                        *aip = ai;

                        ai->ai_socktype = socktype;
                        ((struct sockaddr_in *)(ai->ai_addr))->sin_port = port;

                        /* Special case for AF_INET. From network to host endian.*/
                        if(family == AF_INET)
                        {
                            ((struct sockaddr_in *)(ai->ai_addr))->sin_addr.s_addr = mqx_ntohl(dns_record->data);
                        }
                        else
                        {
                            _mem_copy(dns_record->data, &((struct sockaddr_in *)(ai->ai_addr))->sin_addr, ip_length);
                        }

                        if (flags & AI_CANONNAME) 
                        {
                            if(dns_record->name)  
                            {
                                ai->ai_canonname = strdup(dns_record->name);
                                if (ai->ai_canonname == NULL)
                                {
                                    freeaddrinfo(*aip);
                                    GAI_EXIT(EAI_MEMORY);
                                }
                            }
                        }
                        dns_record = dns_record->next;
                    }
                    while(dns_record);
                    
                    GAI_EXIT(EAI_OK); /* Resolved. */
                }
            }
        }
        /* Not resolved.*/
    }/* end of (hostname == NULL && (flags & AI_PASSIVE) == 0) */
 
 cleanup:
    if(fqdn)
    {
        _mem_free(fqdn);
    }

    if(dns_record_list)
    {
        DNSCLN_record_list_free(dns_record_list);
    }

    return (result);
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : freeaddrinfo
* Returned Value  : void
* Comments        : Free address info.
*
*END*-----------------------------------------------------------------*/
void freeaddrinfo(struct addrinfo *ai) 
{
    struct addrinfo *ai_next;

    while (ai != NULL) 
    {
        ai_next = ai->ai_next;
        if (ai->ai_addr != NULL)
        {
            _mem_free(ai->ai_addr);
        }
        if (ai->ai_canonname)
        {
            _mem_free(ai->ai_canonname);
        }
        _mem_free(ai);
        ai = ai_next;
    }
}


/*!
 * Allocate an addrinfo structure, and a sockaddr structure
 * of the specificed length.  We initialize:
 *  ai_addrlen
 *  ai_family
 *  ai_addr
 *  ai_addr->sa_family
 * and everything else is initialized to zero.
 */
static struct addrinfo *ai_alloc(int family, int addrlen) 
{
    struct addrinfo *ai;

    ai = (struct addrinfo *) _mem_alloc_system_zero(sizeof(*ai));
    if (ai == NULL)
    {
        return (NULL);
    }
    ai->ai_addr = SA(_mem_alloc_system_zero(addrlen));
    if (ai->ai_addr == NULL) 
    {
        _mem_free(ai);
        return (NULL);
    }
    ai->ai_addrlen = addrlen;
    ai->ai_family = family;

    ai->ai_addr->sa_family = family;
    
    return (ai);
}

static struct addrinfo *ai_clone(struct addrinfo *oai, int family) 
{
    struct addrinfo *ai;

    ai = ai_alloc(family, ((family == AF_INET6) ? sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in)));
    if (ai == NULL) 
    {
        freeaddrinfo(oai);
        return (NULL);
    }
    if (oai == NULL)
    {
        return (ai);
    }
    ai->ai_flags = oai->ai_flags;
    ai->ai_socktype = oai->ai_socktype;
    ai->ai_protocol = oai->ai_protocol;
    ai->ai_canonname = NULL;
    ai->ai_next = oai;
    return (ai);
}

static struct addrinfo *ai_reverse(struct addrinfo *oai) 
{
    struct addrinfo *nai, *tai;

    nai = NULL;

    while (oai != NULL) 
    {
        /*
         * Grab one off the old list.
         */
        tai = oai;
        oai = oai->ai_next;
        /*
         * Put it on the front of the new list.
         */
        tai->ai_next = nai;
        nai = tai;
    }
    return (nai);
}


static char * strdup(const char *s)
{
    unsigned int len = strlen (s) + 1;
  
    char *result = (char*) _mem_alloc_system(len);
    if (result != NULL)
    {
        _mem_copy((void*) s, result, len);
    }
    return result;
}

/******************************** Legacy IPv4 resolver function ***************/
static bool RTCS_get_dotted_address (char *name, uint32_t *address)
{
   uint32_t                     i;
   uint32_t                     digit_count = 0;
   uint32_t                     dot_count = 0;
   int32_t                      byte_num = 0;
   bool                     dotted_decimal = TRUE;
   
   if ((name == NULL) || (address == NULL)) return FALSE;
   
   *address = 0;
   
   for (i=0; name[i] != '\0'; ++i ) {
      if ( name[i] == '.' ) {
         dot_count++;
         if ( dot_count > 3 ) {
            /* invalid IP address */
            dotted_decimal = FALSE;
            break;
         }/* Endif */

         if ( digit_count == 0 ) {
            /* there are no digits before the '.' */
            dotted_decimal = FALSE;
            break;
         }/* Endif */
         digit_count = 0;
         byte_num = byte_num / 10; /* shift back */

         if ( (byte_num < 0 ) || (byte_num > 255) ) {
         /* if the number does fall within this range it's invalid */
            dotted_decimal = FALSE;
            break;
         } else  {
            *address = (*address) + byte_num;
            *address = (*address) * 0x100;
         }/* Endif */
      } else { /* a digit */

         if ( digit_count == 0 ) {
            byte_num = 0;
         }/* Endif */
         ++digit_count;

         if ( digit_count > 3 ) {
            /* too many digits between the '.' */
            dotted_decimal = FALSE;
            break;
         }/* Endif */

         if ( (name[i] >= '0') && (name[i] <= '9') ) {
            /* number is in range */
            byte_num = byte_num + name[i] - '0';
            byte_num = byte_num * 10;
         } else {
            /* if the characters are not decimal digits it's invalid */
            dotted_decimal = FALSE;
            break;
         }/* Endif */
      }/* Endif */
   } /* Endfor */

   if ( dotted_decimal ) { /* check last number */
      if ( digit_count == 0 ) {
         /* there are no digits before the '.' */
         dotted_decimal = FALSE;
      }/* Endif */

      byte_num = byte_num / 10;
      if ( (byte_num < 0 ) || (byte_num > 255) ) {
         /* if the number does fall within this range it's invalid */
         dotted_decimal = FALSE;
      } else {
         *address = (*address) + byte_num;
      }/* Endif */

      if ( dot_count != 3 ) {
         /* the wrong number of dots were found */
         dotted_decimal = FALSE;
      }/* Endif */

   }/* Endif */


   if ( i == 0 ) {
      /* no name string of first char was not a number */
      dotted_decimal = FALSE;
   }/* Endif */
    
   return dotted_decimal;
}

bool RTCS_resolve_ip_address( char *arg, _ip_address  *ipaddr_ptr, char *ipname, uint32_t ipnamesize)
{
    bool result = FALSE;

    if(arg && ipaddr_ptr)
    {
        /* first check to see if name is written in dotted decimal IP format */
        if(RTCS_get_dotted_address (arg, ipaddr_ptr))
        {
            /* Name is in dotted decimal format */
            result = TRUE;
        }
        else
        {
            struct addrinfo     addrinfo_hints;
            struct addrinfo     *addrinfo_result;
                
            _mem_zero(&addrinfo_hints, sizeof(addrinfo_hints));
            addrinfo_hints.ai_flags = AI_CANONNAME;
            addrinfo_hints.ai_family = AF_INET;

            if(getaddrinfo(arg, NULL, &addrinfo_hints, &addrinfo_result) == 0) 
            {
                *ipaddr_ptr = ((struct sockaddr_in *)((*addrinfo_result).ai_addr))->sin_addr.s_addr;
                if(ipname && addrinfo_result->ai_canonname)    /* Canonical name.*/
                {
                    strncpy(ipname, addrinfo_result->ai_canonname, ipnamesize);
                }
                freeaddrinfo(addrinfo_result);
                result = TRUE; /* Resolved.*/
            }
        }
    }

    return result;
}


