#ifndef __mld_h__
#define __mld_h__
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
* See license agreement file for full license terms including other restrictions.
*****************************************************************************
*
* Comments:
*
*   MLD Protocol.
*
*
*END************************************************************************/
#include "rtcspcb.h"
#include "rtcstime.h"
#include "icmp6_prv.h"

/************************************************************************
 * RFC3810: MLD is used by an IPv6 router to discover the presence of 
 * multicast listeners on directly attached links, and to discover which
 * multicast addresses are of interest to those neighboring nodes.
 ************************************************************************/

/**********************************************************************
* MLD messages have the following format (RFC 2710)
***********************************************************************
*
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   |     Type      |     Code      |          Checksum             |
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   |     Maximum Response Delay    |          Reserved             |
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   |                                                               |
*   +                                                               +
*   |                                                               |
*   +                       Multicast Address                       +
*   |                                                               |
*   +                                                               +
*   |                                                               |
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*
***********************************************************************/
typedef struct MLD_HEADER
{
    ICMP6_HEADER        HEAD;
    unsigned char       MAX_RESP_DELAY[2];
    unsigned char       RESERVED[2];
    unsigned char       MULTICAST_ADDR[16];
} MLD_HEADER, * MLD_HEADER_PTR;

/***********************************************************************
 * MLD Router Alert option, in  IPv6 Hop-by-Hop Options.
 ***********************************************************************
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  | NextHeader(58)| Hdr Ext Len(0)|Option Type (5)|Opt Data Len(2)|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  | Value (0=MLD)                 |Option Type (1)|Opt Data Len(0)|                         |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 ***********************************************************************/
typedef struct MLD_RA_OPTION_HEADER
{
    unsigned char       NEXT_HEADER;            /* 8-bit selector. Identifies the type of header
                                                 * immediately following the Options
                                                 * header. */
    unsigned char       HDR_EXT_LENGTH;         /* 8-bit unsigned integer. Length of the Hop-by-
                                                 * Hop Options header in 8-octet units, not
                                                 * including the first 8 octets. */
    IP6_OPTION_HEADER   RA_OPTION_HEADER;       /* Router Alert Option. */
    unsigned char       RA_OPTION_VALUE[2];     /* Router Alert Option value. */   
    IP6_OPTION_HEADER   PADN_OPTION_HEADER;     /* Padding. It must be multiple to 8 octets. */                                                 
} MLD_RA_OPTION_HEADER, * MLD_RA_OPTION_HEADER_PTR;

/************************************************************************
*     Function Prototypes
*************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

struct ip_if; /* Forward declaration. Just to avoid header conflicts.*/
void mld_join(struct ip_if *if_ptr, in6_addr *group_addr);
void mld_leave(struct ip_if *if_ptr, in6_addr *group_addr);
void mld_report_all(struct ip_if *if_ptr);
int mld_query_receive(RTCSPCB_PTR *rtcs_pcb_p);

#ifdef __cplusplus
}
#endif

#endif /* __mld_h__ */

