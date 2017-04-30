/*HEADER**********************************************************************
 *
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * Freescale Confidential and Proprietary - use of this software is
 * governed by the Freescale MQX RTOS License distributed with this
 * material. See the MQX_RTOS_LICENSE file distributed for more
 * details.
 *
 *****************************************************************************
 *
 * THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************
 *
 * Comments:
 *
 *   RTCS Hosts file.
 *
 *END************************************************************************/
#include <rtcs.h>

/* Host entry structure. Version of the hosts file. */
typedef const struct rtcs_host_entry
{
    const char      *host_name;
#if RTCSCFG_ENABLE_IP4
    in_addr         ip4_address;
#endif
#if RTCSCFG_ENABLE_IP6
    const in6_addr  *ip6_address;
#endif
} RTCS_HOST_ENTRY, *RTCS_HOST_ENTRY_PTR;

/* Host names.*/
#define RTCS_HOST_LOCALHOST "localhost"
#define RTCS_HOST_BROADCAST "broadcast"

/* RTCS host list. */
static RTCS_HOST_ENTRY RTCS_HOSTS[] =
{
    /* Host Name            IPv4 Address        IPv6 Address */
    {RTCS_HOST_LOCALHOST,   /* Loopback */
        #if RTCSCFG_ENABLE_IP4
                            INADDR_LOOPBACK,
        #endif
        #if RTCSCFG_ENABLE_IP6
                                                &in6addr_loopback,
        #endif
    },             
    {RTCS_HOST_BROADCAST,   /* Broadcast */
        #if RTCSCFG_ENABLE_IP4
                            INADDR_BROADCAST,
        #endif   
        #if RTCSCFG_ENABLE_IP6
                                                &in6addr_linklocal_allnodes,
        #endif
    },   
    /* Put here your Hosts. */
    {0,},
};

/************************************************************************
* NAME: RTCS_get_host_entry
* DESCRIPTION: Function returns pointer to binary form of address (in_addr* or in6_addr*) if found; 
*              NULL if not found.
*************************************************************************/
void *RTCS_hosts_get_addr(const char *host_name, int family)
{
    void                    *found_addr = NULL;
    RTCS_HOST_ENTRY_PTR     host_entry = &RTCS_HOSTS[0];

    for(host_entry = &RTCS_HOSTS[0];  host_entry->host_name;  host_entry++)
    {
        if(strcmp(host_entry->host_name, host_name) == 0) 
        {
        #if RTCSCFG_ENABLE_IP4
            if(family == AF_INET)
            {
                if(host_entry->ip4_address.s_addr != INADDR_ANY)
                {
                    found_addr = (void *)(&host_entry->ip4_address);
                }
            }
            else 
        #endif
        #if RTCSCFG_ENABLE_IP6
            if(family == AF_INET6)
            {
                if((host_entry->ip6_address != NULL) && !IN6_IS_ADDR_UNSPECIFIED(host_entry->ip6_address))
                {
                    found_addr = (void *)(host_entry->ip6_address);
                }
            }
            else
        #endif
            { }

            break;
        }
    }
    return found_addr;
}

/************************************************************************
* NAME: RTCS_hosts_get_name
* DESCRIPTION: Function returns pointer to host name string if found; 
*              NULL if not found.
*************************************************************************/
const char *RTCS_hosts_get_name(const sockaddr  *sa)
{
    const char              *found_name = NULL;
    RTCS_HOST_ENTRY_PTR     host_entry = &RTCS_HOSTS[0];

    for(host_entry = &RTCS_HOSTS[0];  host_entry->host_name;  host_entry++)
    {
        if( 
        #if RTCSCFG_ENABLE_IP4
            ((sa->sa_family == AF_INET) && (host_entry->ip4_address.s_addr == ((sockaddr_in *)sa)->sin_addr.s_addr)) ||
        #endif
        #if RTCSCFG_ENABLE_IP6
            ((sa->sa_family == AF_INET6) && (IN6_ARE_ADDR_EQUAL(host_entry->ip6_address, &((sockaddr_in6 *)sa)->sin6_addr))) ||
        #endif
            0 )
        {
            found_name = host_entry->host_name;
            break;
        }
    }
    return found_name;
}
