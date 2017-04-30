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
*   This file contains the implementation of the Internet
*   Protocol.  For more details, refer to RFC791 and RFC1122.
*
*
*END************************************************************************/

#include <rtcs.h>
#include "rtcs_prv.h"
#include "tcpip.h"
#include "ip_prv.h"
#include "tcp_prv.h"
#include "udp_prv.h"
#include "icmp_prv.h"

#if RTCSCFG_ENABLE_IP4
static uint32_t ip_option_handler(RTCSPCB_PTR rtcs_pcb, uint8_t  *ip_options, uint32_t  ip_options_length );
static void ip_option_timestamp_add (IP_OPTION_TIMESTAMP_PTR  ip_option_timestamp, uint32_t timestamp_offset, uint32_t pointer_inc);
#endif

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IP_init
* Returned Values : RTCS_OK or error code
* Comments        :
*     Initialize the IP layer.
*
*END*-----------------------------------------------------------------*/
#if RTCSCFG_ENABLE_IP4  

uint32_t IP_init
   (
      void
   )
{ /* Body */
   IP_CFG_STRUCT_PTR IP_cfg_ptr;

   IP_cfg_ptr = RTCS_mem_alloc_zero(sizeof(IP_CFG_STRUCT));

   if (IP_cfg_ptr == NULL)  {
      return RTCSERR_OUT_OF_MEMORY;
   }

   _mem_set_type(IP_cfg_ptr, MEM_TYPE_IP_CFG_STRUCT);
   
   RTCS_setcfg(IP, IP_cfg_ptr);

   RTCS_LIST_INIT(IP_cfg_ptr->ICB_HEAD);
   IP_cfg_ptr->NEXT_ID  = 0;
   IP_cfg_ptr->DEFAULT_TTL = IPTTL_DEFAULT;

   IP_cfg_ptr->ROUTE_PARTID  = RTCS_part_create(sizeof(IP_ROUTE_DIRECT),
      IPROUTEALLOC_SIZE, IPROUTEALLOC_SIZE, 0, NULL, NULL);
#if RTCSCFG_ENABLE_GATEWAYS
   IP_cfg_ptr->GATE_PARTID   = RTCS_part_create(sizeof(IP_ROUTE_INDIRECT),
      IPGATEALLOC_SIZE,  IPGATEALLOC_SIZE,  0, NULL, NULL);
#endif
#if RTCSCFG_ENABLE_VIRTUAL_ROUTES
   IP_cfg_ptr->VIRTUAL_PARTID   = RTCS_part_create(sizeof(IP_ROUTE_VIRTUAL),
      IPROUTEALLOC_SIZE,  IPROUTEALLOC_SIZE,  0, NULL, NULL);
#endif
   IP_cfg_ptr->ROUTE_FN      = NULL;
#if RTCSCFG_ENABLE_IGMP && RTCSCFG_ENABLE_IP4
   IP_cfg_ptr->MCB_PARTID    = RTCS_part_create(sizeof(MC_MEMBER),
      IPMCBALLOC_SIZE,   IPMCBALLOC_SIZE,   0, NULL, NULL);
#endif
   IP_cfg_ptr->RADIX_PARTID  = RTCS_part_create(sizeof(IP_ROUTE),
      RADIXALLOC_SIZE,   RADIXALLOC_SIZE,   0, NULL, NULL);

   IP_route_init(&IP_cfg_ptr->ROUTE_ROOT);

   return RTCS_OK;
} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IP_send
*  Returned Value : RTCS_OK or error code
*  Comments       :
*        Sends an IP packet generated on the local host.
*
*END*-----------------------------------------------------------------*/

uint32_t IP_send
   (
      RTCSPCB_PTR    rtcs_pcb,
            /* [IN] the packet to send */
      uint32_t        protocol,
            /* [IN] the transport layer protocol */
      _ip_address    ipsrc,
            /* [IN] the destination interface (0 = any) */
      _ip_address    ipdest,
            /* [IN] the ultimate destination */
      uint32_t        flags
            /* [IN] optional flags */
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   IF_IP_STATS_ENABLED(IP_CFG_STRUCT_PTR    IP_cfg_ptr);

   IF_IP_STATS_ENABLED(IP_cfg_ptr = RTCS_getcfg(IP));

   IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_TX_TOTAL++);

   rtcs_pcb->IP_COMPLETE = IP_complete_send;
   rtcs_pcb->IFSRC       = RTCS_IF_LOCALHOST_PRV;

   /*
   ** Validate destination address
   */
   if (IN_ZERONET(ipdest)) {
      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_TX_DISCARDED++);
      RTCSLOG_PCB_FREE(rtcs_pcb, RTCSERR_IP_BAD_ADDRESS);
      RTCSPCB_free(rtcs_pcb);
      return RTCSERR_IP_BAD_ADDRESS;
   } /* Endif */

   if ((ipdest == INADDR_BROADCAST)
    || IN_MULTICAST(ipdest)) {
      return IP_route_multi(rtcs_pcb, protocol, ipsrc, ipdest, flags);
   } else {
      return IP_route(rtcs_pcb, protocol, ipsrc, INADDR_ANY, ipdest, flags);
   } /* Endif */

#else

    RTCSLOG_PCB_FREE(rtcs_pcb, RTCSERR_IP_IS_DISABLED);
    RTCSPCB_free(rtcs_pcb);
    return RTCSERR_IP_IS_DISABLED;

#endif /* RTCSCFG_ENABLE_IP4 */   

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IP_send_IF
*  Returned Value : RTCS_OK or error code
*  Comments       :
*        Sends an IP packet through a specified interface.
*
*END*-----------------------------------------------------------------*/

uint32_t IP_send_IF
   (
      RTCSPCB_PTR    rtcs_pcb,
            /* [IN] the packet to send */
      uint32_t        protocol,
            /* [IN] the transport layer protocol */
      void          *interface
            /* [IN] the destination interface */
   )
{ 

#if RTCSCFG_ENABLE_IP4

   IF_IP_STATS_ENABLED(IP_CFG_STRUCT_PTR    IP_cfg_ptr);
   IP_IF_PTR            ifdest = interface;

   IF_IP_STATS_ENABLED(IP_cfg_ptr = RTCS_getcfg(IP));

   IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_TX_TOTAL++);


   rtcs_pcb->IP_COMPLETE = IP_complete_send;
   rtcs_pcb->IFSRC       = RTCS_IF_LOCALHOST_PRV;

   return IP_send_dgram(ifdest, rtcs_pcb, INADDR_ANY, INADDR_BROADCAST,
      INADDR_BROADCAST, protocol, NULL);
      
#else

    RTCSLOG_PCB_FREE(rtcs_pcb, RTCSERR_IP_IS_DISABLED);
    RTCSPCB_free(rtcs_pcb);
    return RTCSERR_IP_IS_DISABLED;

#endif /* RTCSCFG_ENABLE_IP4 */  

} 

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IP_complete_send
*  Returned Value : RTCS_OK or error code
*  Comments       :
*        Generates an IP header for outgoing packets.  This function
*        must not consume pcb.
*
*END*-----------------------------------------------------------------*/
#if RTCSCFG_ENABLE_IP4

uint32_t IP_complete_send
   (
      void             *ifdest,
            /* [IN] the destination interface */
      RTCSPCB_PTR  *rtcs_pcb_ptr_ptr,
            /* [IN] the packet to send */
      _ip_address       hopsrc,
            /* [IN] the destination interface */
       _ip_address      ipdest,
            /* [IN] the ultimate dest */
      uint32_t           protocol
            /* [IN] the transport layer protocol */
   )
{ /* Body */
    RTCSPCB_PTR             rtcs_pcb = *rtcs_pcb_ptr_ptr;
    IP_CFG_STRUCT_PTR       IP_cfg_ptr;
    IP_HEADER_PTR           packet;
    unsigned char           proto, ttl, tos;
    uint32_t                error;
    uint16_t                chksum;
    unsigned char           dfrag;
    uint32_t                ip_header_size = sizeof(IP_HEADER) + rtcs_pcb->ip4_options_length;        

   IP_cfg_ptr = RTCS_getcfg(IP);

   error = RTCSPCB_insert_header(rtcs_pcb, ip_header_size);
   if (error) {
      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_TX_MISSED++);
      IF_IP_STATS_ENABLED(RTCS_seterror(&IP_cfg_ptr->STATS.ERR_TX, error, (uint32_t)rtcs_pcb));
      /* The PCB will be freed for us. */
      return error;
   } /* Endif */

   RTCSLOG_PCB_WRITE(rtcs_pcb, RTCS_LOGCTRL_PROTO(IPPROTO_IP), IP_VERSION);
   packet = (IP_HEADER_PTR)RTCSPCB_DATA(rtcs_pcb);
   RTCSPCB_DATA_NETWORK(rtcs_pcb) = RTCSPCB_DATA(rtcs_pcb);
   proto  = IPPROTO_GET(protocol);
   tos    = IPTOS_GET(protocol);
   if (!tos) 
     tos = rtcs_pcb->LINK_OPTIONS.TX.TOS;
   dfrag  = IPDFRAG_GET(protocol);
   
      
   ttl = rtcs_pcb->LINK_OPTIONS.TX.TTL;
   if (!ttl) ttl = IPTTL_GET(protocol);
   if (!ttl) ttl = IP_cfg_ptr->DEFAULT_TTL;

   (void) mqx_htonc(packet->VERSLEN,  (IP_VERSION << 4) | (ip_header_size >> 2));
   (void) mqx_htonc(packet->TOS,      tos);
   (void) mqx_htons(packet->LENGTH,   RTCSPCB_SIZE(rtcs_pcb));
   (void) mqx_htons(packet->ID,       IP_cfg_ptr->NEXT_ID);
   IP_cfg_ptr->NEXT_ID++;
   if (dfrag) {
      (void) mqx_htons(packet->FRAGMENT, IP_FRAG_DF);
   } else {
      (void) mqx_htons(packet->FRAGMENT, 0);
   } /* Endif */
   (void) mqx_htonc(packet->TTL,      ttl);
   (void) mqx_htonc(packet->PROTOCOL, proto);
   (void) mqx_htonl(packet->DEST,     ipdest);
   (void) mqx_htons(packet->CHECKSUM, 0);
   (void) mqx_htonl(packet->SOURCE,   hopsrc);

    /* Add Ipv4 Options.*/
    if(rtcs_pcb->ip4_options_length)
    {
        memmove(&((uint8_t*)RTCSPCB_DATA(rtcs_pcb))[sizeof(IP_HEADER)], rtcs_pcb->ip4_options, rtcs_pcb->ip4_options_length);
    }

   /* Set up the transport layer protocol and delta. */
   RTCSPCB_SET_TRANS_PROTL(rtcs_pcb, proto);
   RTCSPCB_SET_TRANS_DELTA(rtcs_pcb, ip_header_size);

   /*
   ** If the transport layer needs a pseudo header checksum,
   ** calculate it now.
   */
   if (rtcs_pcb->IP_SUM_PTR) {
      chksum = IP_Sum_pseudo(rtcs_pcb->IP_SUM, rtcs_pcb, 0);
      chksum = IP_Sum_invert(chksum);
      (void) mqx_htons(rtcs_pcb->IP_SUM_PTR, chksum);
   } /* Endif */

   rtcs_pcb = *rtcs_pcb_ptr_ptr;

   /* We need to refresh the header location, as it may have been shifted.*/
   packet = (IP_HEADER_PTR)RTCSPCB_DATA(rtcs_pcb);

#if BSPCFG_ENET_HW_TX_IP_CHECKSUM    
    /* HW checksum offload.*/
    if ( (((IP_IF_PTR)ifdest)->FEATURES & IP_IF_FEATURE_HW_TX_IP_CHECKSUM)
    #if RTCSCFG_LINKOPT_8023
        && (rtcs_pcb->LINK_OPTIONS.TX.OPT_8023 == 0)
    #endif
        )
    {
        rtcs_pcb->TYPE|= RTCSPCB_TYPE_HW_IP_CHECKSUM;
    }
    else
#endif
    {
        chksum = _mem_sum_ip(0, ip_header_size, packet);
        chksum = IP_Sum_invert(chksum);
        (void) mqx_htons(packet->CHECKSUM, chksum);
    }

   /* Special case for loopback */
   if (ifdest == RTCS_IF_LOCALHOST_PRV) {
      RTCSPCB_PTR            new_rtcs_pcb;
      unsigned char              *data;
      PCB_FRAGMENT_PTR       frag;

      data = RTCS_mem_alloc_system(sizeof(PCB) + sizeof(PCB_FRAGMENT) + RTCSPCB_SIZE(rtcs_pcb));

      if (!data)  {
         return RTCSERR_OUT_OF_MEMORY;
      } /* Endif */

      _mem_set_type(data, MEM_TYPE_IP_DATA);

      frag = ((PCB_PTR)data)->FRAG;
      ((PCB_PTR)data)->FREE = (void(_CODE_PTR_)(PCB_PTR))_mem_free;
      ((PCB_PTR)data)->PRIVATE = NULL;
      frag[0].LENGTH = RTCSPCB_SIZE(rtcs_pcb);
      frag[0].FRAGMENT = data + sizeof(PCB) + sizeof(PCB_FRAGMENT);
      frag[1].LENGTH = 0;
      frag[1].FRAGMENT = NULL;

      new_rtcs_pcb = RTCSPCB_alloc_recv((PCB_PTR)data);
      if (!new_rtcs_pcb)  {
         _mem_free(data);
         return RTCSERR_OUT_OF_MEMORY;
      } /* Endif */

      new_rtcs_pcb->TYPE = rtcs_pcb->TYPE;
      new_rtcs_pcb->IFSRC = rtcs_pcb->IFSRC;
      new_rtcs_pcb->LINK_OPTIONS = rtcs_pcb->LINK_OPTIONS;
      new_rtcs_pcb->IP_COMPLETE = IP_complete_recv;

      data += sizeof(PCB) + sizeof(PCB_FRAGMENT);
      RTCSPCB_memcopy(rtcs_pcb, data, 0, RTCSPCB_SIZE(rtcs_pcb));
      RTCSPCB_DATA_NETWORK(new_rtcs_pcb) = RTCSPCB_DATA(new_rtcs_pcb);
      RTCSPCB_free(rtcs_pcb);

      *rtcs_pcb_ptr_ptr = new_rtcs_pcb;

      return IP_complete_recv(ifdest, rtcs_pcb_ptr_ptr, hopsrc, ipdest, protocol);

   } /* Endif */

   return RTCS_OK;
} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */



#if RTCSCFG_ENABLE_IP4
/* 
 * Add timestamp.
 */
static void ip_option_timestamp_add (IP_OPTION_TIMESTAMP_PTR  ip_option_timestamp, uint32_t timestamp_offset, uint32_t pointer_inc)
{
    /* If the time is not available in
    * milliseconds or cannot be provided with respect to midnight UT
    * then any time may be inserted as a timestamp provided the high
    * order bit of the timestamp field is set to one to indicate the
    * use of a non-standard value.*/                  
    (void) mqx_htonl(ip_option_timestamp->TIMESTAMP[timestamp_offset], 0x80000000| RTCS_get_milliseconds());
                                                            
    /*Increment pointer*/
    (void) mqx_htonc(ip_option_timestamp->POINTER, mqx_ntohc(ip_option_timestamp->POINTER) + pointer_inc);
}

/* 
 * Handle IPv4 Options.
 */
static uint32_t ip_option_handler(RTCSPCB_PTR    rtcs_pcb, uint8_t  *ip_options, uint32_t  ip_options_length )
{
    uint32_t    offset = 0;
    uint32_t    paramprob_pointer = 0;
    bool        eol = false;

    /* Verify IPv4 options.*/
    while((offset < ip_options_length) && (eol == false) && (paramprob_pointer == 0))
    {
        /*RFC791: The option field is variable in length.  There may be zero or more options.*/
        switch(ip_options[offset])
        {
            /* There are two cases for the format of an option:
            * Case 1:  A single octet of option-type.*/
            case IP_OPTION_TYPE_EOL:    /* End of Option List. */
                eol = true;
                break;
            case IP_OPTION_TYPE_NOP:    /* No Operation. */  
                offset++;
                break;
            /* Case 2:  An option-type octet, an option-length octet, and the
            * actual option-data octets.*/
            default: /* Other options*/
                if((offset+1) < ip_options_length)
                {
                    /* The option-length octet counts the 
                    * option-type octet and the option-length octet as well as the option-data octets.*/
                    uint8_t option_length = ip_options[offset+1];

                    /* Check min and max. length*/
                    if((option_length >= 2)&&((option_length+offset) <= ip_options_length))
                    {
                        _ip_address     if_ip_addr = IP_get_ipif_addr(rtcs_pcb->IFSRC);

                        /* Supported IPv4 options.*/
                        switch(ip_options[offset])
                        {
                            /* RFC791, p23:
                            *      Internet Timestamp
                            *
                            *        +--------+--------+--------+--------+
                            *        |01000100| length | pointer|oflw|flg|
                            *        +--------+--------+--------+--------+
                            *        |         internet address          |
                            *        +--------+--------+--------+--------+
                            *        |             timestamp             |
                            *        +--------+--------+--------+--------+
                            *        |                 .                 |
                            */
                            case IP_OPTION_TYPE_TIMESTAMP:
                                /* Check min length (type+length+pointer+flags).*/
                                if(option_length >= sizeof(IP_OPTION_TIMESTAMP))
                                {
                                    IP_OPTION_TIMESTAMP_PTR  ip_option_timestamp = (IP_OPTION_TIMESTAMP_PTR)(&ip_options[offset]);

                                    /* The smallest legal value for pointer is 5.*/
                                    if(mqx_ntohc(ip_option_timestamp->POINTER) >= IP_OPTION_TIMESTAMP_POINTER_MIN)
                                    {
                                        unsigned char   flg = mqx_ntohc(ip_option_timestamp->OFLW_FLG)&0xF;
                                        unsigned char   ts_pointer = mqx_ntohc(ip_option_timestamp->POINTER);
                                        unsigned char   ts_offset = (ts_pointer-IP_OPTION_TIMESTAMP_POINTER_MIN)/sizeof(uint32_t);
                                                
                                        switch(flg)
                                        {
                                            case 0:
                                                /* 0 -- time stamps only, stored in consecutive 32-bit words */

                                                /* The timestamp area is full when the pointer is greater than the length.*/
                                                if(ts_pointer > option_length)
                                                {
                                                    /* Increment oflw field.*/
                                                    (void) mqx_htonc(ip_option_timestamp->OFLW_FLG, mqx_ntohc(ip_option_timestamp->OFLW_FLG)+(1<<4)); 
                                                }
                                                else
                                                {
                                                    /* Add timestamp.*/
                                                    ip_option_timestamp_add (ip_option_timestamp, ts_offset, sizeof(uint32_t));
                                                }
                                                break;
                                            case 1:
                                                /* 1 -- each timestamp is preceded with internet address of the
                                                *      registering entity*/

                                                /* The timestamp area is full when the pointer+4 is greater than the length.*/
                                                if((ts_pointer + sizeof(uint32_t)) > option_length)
                                                {
                                                    /* Increment oflw field.*/
                                                    (void) mqx_htonc(ip_option_timestamp->OFLW_FLG, mqx_ntohc(ip_option_timestamp->OFLW_FLG)+(1<<4)); 
                                                }
                                                else
                                                {
                                                    /* Add IP address.*/
                                                    (void) mqx_htonl(ip_option_timestamp->TIMESTAMP[ts_offset], if_ip_addr);

                                                    /* Add timestamp.*/
                                                    ip_option_timestamp_add (ip_option_timestamp, ts_offset+1, 2*sizeof(uint32_t));
                                                }
                                                break;
                                            case 3:
                                                /* 3 -- the internet address fields are prespecified.  An IP
                                                *      module only registers its timestamp if it matches its own
                                                *      address with the next specified internet address. */
                                                if((ts_pointer + sizeof(uint32_t)) <= option_length)
                                                {
                                                    if(if_ip_addr == mqx_ntohl(ip_option_timestamp->TIMESTAMP[ts_offset]))
                                                    {
                                                        /* Add timestamp.*/
                                                        ip_option_timestamp_add (ip_option_timestamp, ts_offset+1, 2*sizeof(uint32_t));
                                                    }
                                                }
                                                break;
                                            default:
                                                break;
                                        }
                                                
                                        /* Save pointer to IPv4 options.*/
                                        rtcs_pcb->ip4_options = ip_options;
                                        rtcs_pcb->ip4_options_length = ip_options_length;
                                    }
                                }
                                break;
                            /* RFC791, p20:
                            *      Record Route
                            *
                            *        +--------+--------+--------+---------//--------+
                            *        |00000111| length | pointer|     route data    |
                            *        +--------+--------+--------+---------//--------+
                            */
                            case IP_OPTION_TYPE_RR:
                                /* Check min length.*/
                                if(option_length >= sizeof(IP_OPTION_RECORDROUTE))
                                {
                                    IP_OPTION_RECORDROUTE_PTR   ip_option_recordroute = (IP_OPTION_RECORDROUTE_PTR)(&ip_options[offset]);
                                    unsigned char               rr_pointer = mqx_ntohc(ip_option_recordroute->POINTER);

                                    /* The smallest legal value for pointer is 4.*/
                                    if(rr_pointer >= IP_OPTION_RR_POINTER_MIN)
                                    {
                                        unsigned char   rr_offset = (rr_pointer - IP_OPTION_RR_POINTER_MIN) / sizeof(uint32_t);

                                        if((mqx_ntohc(ip_option_recordroute->POINTER) + sizeof(uint32_t)) <= option_length)
                                        {
                                            /* Add IP address.*/
                                            (void) mqx_htonl(ip_option_recordroute->ROUTE[rr_offset], if_ip_addr);

                                            /*Increment pointer*/
                                            (void) mqx_htonc(ip_option_recordroute->POINTER, mqx_ntohc(ip_option_recordroute->POINTER) + sizeof(uint32_t));
                                        }
                                        /* ELSE rfc791: If the route data area is already full (the pointer exceeds the
                                        * length) the datagram is forwarded without inserting the address
                                        * into the recorded route.
                                        */
                                    }
                                }
                                break;
                            default:
                                break;
                        }
                                
                        offset+=option_length; 
                    }
                    else /* Wrong length.*/
                    {   
                        /* ICMP error.*/
                        paramprob_pointer = ((offset+1+sizeof(IP_HEADER))&0xFF)<<12;
                    }
                }
                else /* No length field.*/
                {
                    /* ICMP error.*/            
                    paramprob_pointer = ((offset+sizeof(IP_HEADER))&0xFF)<<12;
                }
                break;
        } /* switch(ip_options[offset]) */
    } /* while */
    
    return paramprob_pointer;
}
#endif /* RTCSCFG_ENABLE_IP4 */


#if RTCSCFG_ENABLE_IP4
/************************************************************************
* NAME: IP_addr_is_broadcast
*
* DESCRIPTION: Returns TRUE if address is broadcast. 
*************************************************************************/
bool IP_addr_is_broadcast (RTCSPCB_PTR rtcs_pcb, _ip_address ip_addr)
{ 
    IP_IF_PTR       ipif = (IP_IF_PTR)rtcs_pcb->IFSRC;
    _ip_address     ipif_addr = IP_get_ipif_addr(ipif);        /* Get IP address of interface */
    _ip_address     netmask = 0;
    _ip_address     netbroadcast;
    bool            result;

    IP_get_netmask(ipif, ipif_addr, &netmask);                  /* Get netmask */

    netbroadcast = ipif_addr | (~netmask);

    if((ip_addr == INADDR_BROADCAST)|| (ip_addr == netbroadcast))
    {
        result = TRUE;
    }
    else
    {
        result = FALSE;
    }

    return result;
}
#endif

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IP_service
*  Returned Value : RTCS_OK or error code
*  Comments       :
*        Parses a received IP packet.
*
*END*-----------------------------------------------------------------*/
#if RTCSCFG_ENABLE_IP4
void IP_service
   (
      RTCSPCB_PTR    rtcs_pcb
            /* [IN] the packet to send */
   )
{ /* Body */
   IF_IP_STATS_ENABLED(IP_CFG_STRUCT_PTR          IP_cfg_ptr);
   IP_HEADER_PTR              packet = (IP_HEADER_PTR)RTCSPCB_DATA(rtcs_pcb);
   _ip_address                ipsrc, ipdest;
   uint16_t                    hdrlen, pktlen;
   uint32_t                    routeopt, error;
   #if RTCSCFG_ENABLE_NAT
   uint32_t (_CODE_PTR_   *nat_exec)(RTCSPCB_PTR *);
   #endif
   
   RTCSLOG_FNE2(RTCSLOG_FN_IP_service, rtcs_pcb);

   IF_IP_STATS_ENABLED(IP_cfg_ptr = RTCS_getcfg(IP));

   IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_RX_TOTAL++);
   hdrlen = (mqx_ntohc(packet->VERSLEN) & 0x0F) << 2;
   pktlen = mqx_ntohs(packet->LENGTH);
   RTCSPCB_DATA_NETWORK(rtcs_pcb) = RTCSPCB_DATA(rtcs_pcb);

   /*
   ** Make sure that
   **    sizeof(IP_HEADER) <= hdrlen <= pktlen <= RTCSPCB_SIZE(pcb)
   */
   if (hdrlen < sizeof(IP_HEADER)) {
      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_RX_DISCARDED++);
      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_RX_HDR_ERRORS++);
      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_RX_SMALL_HDR++);
      RTCSLOG_PCB_FREE(rtcs_pcb, RTCSERR_IP_BAD_HEADER);
      RTCSPCB_free(rtcs_pcb);
      RTCSLOG_FNX2(RTCSLOG_FN_IP_service, 1);
      return;
   } /* Endif */
   if (pktlen < hdrlen) {
      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_RX_DISCARDED++);
      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_RX_HDR_ERRORS++);
      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_RX_SMALL_DGRAM++);
      RTCSLOG_PCB_FREE(rtcs_pcb, RTCSERR_IP_BAD_HEADER);
      RTCSPCB_free(rtcs_pcb);
      RTCSLOG_FNX2(RTCSLOG_FN_IP_service, 2);
      return;
   } /* Endif */
   if (RTCSPCB_SIZE(rtcs_pcb) < pktlen) {
      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_RX_DISCARDED++);
      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_RX_HDR_ERRORS++);
      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_RX_SMALL_PKT++);
      RTCSLOG_PCB_FREE(rtcs_pcb, RTCSERR_IP_BAD_HEADER);
      RTCSPCB_free(rtcs_pcb);
      RTCSLOG_FNX2(RTCSLOG_FN_IP_service, 3);
      return;
   } /* Endif */

   /*
   ** IP version must be 4
   */
   if ((mqx_ntohc(packet->VERSLEN) >> 4) != IP_VERSION) {
      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_RX_DISCARDED++);
      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_RX_HDR_ERRORS++);
      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_RX_BAD_VERSION++);
      RTCSLOG_PCB_FREE(rtcs_pcb, RTCSERR_IP_BAD_HEADER);
      RTCSPCB_free(rtcs_pcb);
      RTCSLOG_FNX2(RTCSLOG_FN_IP_service, 4);
      return;
   } /* Endif */

   RTCSLOG_PCB_READ(rtcs_pcb, RTCS_LOGCTRL_PROTO(IPPROTO_IP), IP_VERSION);

   /*
   ** Validate source address
   */
   ipsrc = mqx_ntohl(packet->SOURCE);
   if (IN_LOOPBACK(ipsrc)
    || IN_MULTICAST(ipsrc)
    || IN_EXPERIMENTAL(ipsrc)) {
      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_RX_DISCARDED++);
      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_RX_HDR_ERRORS++);
      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_RX_BAD_SOURCE++);
      RTCSLOG_PCB_FREE(rtcs_pcb, RTCSERR_IP_BAD_ADDRESS);
      RTCSPCB_free(rtcs_pcb);
      RTCSLOG_FNX2(RTCSLOG_FN_IP_service, 5);
      return;
   } 

   /*
   ** Validate destination address
   */
   ipdest = mqx_ntohl(packet->DEST);
   if (IN_LOOPBACK(ipdest)
    || IN_ZERONET(ipdest)
    || (IN_EXPERIMENTAL(ipdest) && ipdest != INADDR_BROADCAST)) {
      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_RX_DISCARDED++);
      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_RX_ADDR_ERRORS++);
      RTCSLOG_PCB_FREE(rtcs_pcb, RTCSERR_IP_BAD_ADDRESS);
      RTCSPCB_free(rtcs_pcb);
      RTCSLOG_FNX2(RTCSLOG_FN_IP_service, 6);
      return;
   } /* Endif */


#if BSPCFG_ENET_HW_RX_IP_CHECKSUM 
    /* HW checksum offload.*/
    if ( (((((IP_IF_PTR)(rtcs_pcb->IFSRC))->FEATURES & IP_IF_FEATURE_HW_RX_IP_CHECKSUM) == 0) 
        &&((rtcs_pcb->TYPE & RTCSPCB_TYPE_HW_IP_CHECKSUM) == 0) )
    #if RTCSCFG_LINKOPT_8023
        ||(rtcs_pcb->LINK_OPTIONS.RX.OPT_8023 == 1)
    #endif
        )
#endif
    {
        /* Verify the checksum */
        uint32_t  sum_ip = _mem_sum_ip(0, hdrlen, packet);
        /* RFC 1624: section 3 */
        if ((sum_ip != 0xFFFF) && (sum_ip != 0x0000))
        {
            IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_RX_DISCARDED++);
            IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_RX_HDR_ERRORS++);
            IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_RX_BAD_CHECKSUM++);
            RTCSLOG_PCB_FREE(rtcs_pcb, RTCSERR_IP_BAD_CHECKSUM);
            RTCSPCB_free(rtcs_pcb);
            RTCSLOG_FNX2(RTCSLOG_FN_IP_service, 7);
            return;
       } 
    }

   /* 
    * Handle Ipv4 Options.
    */
    if(hdrlen > sizeof(IP_HEADER))
    {   /*WIIKI: If the header length is greater than 5, it means that the options field is present and must be considered.*/

        uint32_t    paramprob_pointer = ip_option_handler(rtcs_pcb, &RTCSPCB_DATA(rtcs_pcb)[sizeof(IP_HEADER)], hdrlen- sizeof(IP_HEADER) );

        if(paramprob_pointer != 0)
        {
            /* ICMP error.*/
            IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_RX_DISCARDED++);
            IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_RX_HDR_ERRORS++);
        #if RTCSCFG_ENABLE_ICMP
            /* Parameter Problem Message:
            *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            *    |     Type      |     Code      |          Checksum             |
            *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            *    |    Pointer    |                   unused                      |
            *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
            *  Code 0 = Pointer indicates the error.*/
            ICMP_send_error(ICMPTYPE_PARMPROB, 0, paramprob_pointer , rtcs_pcb, 0);
        #endif
            RTCSLOG_PCB_FREE(rtcs_pcb, RTCSERR_IP_BAD_HEADER);
            RTCSPCB_free(rtcs_pcb);
            return;
        }
    }
 
   /*
   ** In this function, the PCB always has only one fragment, so it's
   ** always safe to call RTCSPCB_shrink() here.
   */
   error = RTCSPCB_shrink(rtcs_pcb, RTCSPCB_SIZE(rtcs_pcb) - pktlen);
   if (error) {
      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_RX_ERRORS++);
      IF_IP_STATS_ENABLED(RTCS_seterror(&IP_cfg_ptr->STATS.ERR_RX, error, (uint32_t)rtcs_pcb));
      RTCSLOG_PCB_FREE(rtcs_pcb, error);
      RTCSPCB_free(rtcs_pcb);
      RTCSLOG_FNX2(RTCSLOG_FN_IP_service, 8);
      return;
   } 


   rtcs_pcb->IP_COMPLETE = IP_complete_recv;
/* rtcs_pcb->IFSRC has already been set by link layer */

   /*
   ** Now decide what to do with the packet.
   **
   ** Normally, we should do:
   **
   **    if (!FORWARD || !(TYPE & UNICAST))
   **       IP_route_local();
   **    else
   **       IP_route();
   **
   ** i.e. we only forward unicast packets, and only if forwarding is
   ** turned on.  Unfortunately, this isn't sufficient.
   **
   ** If forwarding is turned on, limited broadcasts from PPP links
   ** will be discarded, because on PPP links, (TYPE & UNICAST) is
   ** always true (PPP packets are both unicast and broadcast), and
   ** IP_route() discards limited broadcasts.
   **
   ** Hence, we have to explicitly make sure that limited broadcasts
   ** and multicasts go through IP_route_local().
   **
   ** Once multicast routing is implemented, we'll remove IN_MULTICAST()
   ** from the first condition.
   **
   ** RFC 2131 says:
   ** "In the case of a client using DHCP for initial configuration (before
   ** the client's TCP/IP software has been completely configured), DHCP
   ** requires creative use of the client's TCP/IP software and liberal
   ** interpretation of RFC 1122.  The TCP/IP software SHOULD accept and
   ** forward to the IP layer any IP packets delivered to the client's
   ** hardware address before the IP address is configured."
   ** 
   ** So if the broadcast flag is disabled, we check, and if the packet's 
   ** source interface is not bound, we make sure the packet goes through 
   ** IP_route_local(), where we have the same test to send the packet to 
   ** IF_LOCALHOST.  
   */

   if (!_IP_forward
    || !(rtcs_pcb->TYPE & RTCSPCB_TYPE_UNICAST)
    || (IP_addr_is_broadcast (rtcs_pcb, ipdest) == TRUE) 
    || IN_MULTICAST(ipdest)
    || (!_DHCP_broadcast && (IP_get_ipif_addr(rtcs_pcb->IFSRC) == INADDR_ANY))) { 

      IP_route_local(rtcs_pcb, ipdest);

   } else {

      if (IN_ZERONET(ipsrc)) {
         IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_RX_DISCARDED++);
         IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_RX_HDR_ERRORS++);
         IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_RX_BAD_SOURCE++);
         RTCSLOG_PCB_FREE(rtcs_pcb, RTCSERR_IP_BAD_ADDRESS);
         RTCSPCB_free(rtcs_pcb);
         RTCSLOG_FNX2(RTCSLOG_FN_IP_service, 9);
         return;
      } /* Endif */

      /*
      ** If the packet was broadcast, don't send it out the receiving
      ** interface.  This protects mainly against bouncing PPP directed
      ** broadcasts.
      */
      if (rtcs_pcb->TYPE & RTCSPCB_TYPE_BROADCAST) {
         routeopt = IPROUTEOPT_RECVIF;
      } else {
         routeopt = 0;
      } /* Endif */

      #if RTCSCFG_ENABLE_NAT   
      /* Apply NAT if it is installed */
      nat_exec = RTCS_getcfg(NAT);
      if (nat_exec && *nat_exec) {
         error = (*nat_exec)(&rtcs_pcb);
         if (error) {
            if (rtcs_pcb!=NULL) {
               IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_RX_DISCARDED++);
               RTCSLOG_PCB_FREE(rtcs_pcb, error);
               RTCSPCB_free(rtcs_pcb);
            }
            RTCSLOG_FNX2(RTCSLOG_FN_IP_service, 10);
            return;
         } /* Endif */

         // NAT may have consumed the PCB if it was a fragment
         if (rtcs_pcb==NULL) {
            RTCSLOG_FNX2(RTCSLOG_FN_IP_service, 11);
            return;
         }

         /* PCB pointer may have changed */
         rtcs_pcb->IP_COMPLETE = IP_complete_recv;
         packet = (IP_HEADER_PTR)RTCSPCB_DATA(rtcs_pcb);
         ipdest = mqx_ntohl(packet->DEST);
      } /* Endif */
      #endif

      IP_route(rtcs_pcb, 0, INADDR_ANY, ipsrc, ipdest, routeopt);

   } /* Endif */
   
    RTCSLOG_FNX2(RTCSLOG_FN_IP_service, 0);
} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IP_complete_recv
*  Returned Value : RTCS_OK or error code
*  Comments       :
*        Decrements the TTL on forwarding packets.  This function
*        must not consume pcb.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

uint32_t IP_complete_recv
   (
      void             *ifdest,
            /* [IN] the destination interface */
      RTCSPCB_PTR  *rtcs_pcb_ptr_ptr,
            /* [IN] the packet to send */
      _ip_address       hopsrc,
            /* [IN] the destination interface */
      _ip_address       ipdest,
      /* [IN] the ultimate dest */
      uint32_t           protocol
      /* [IN] the transport layer protocol */
   )
{ /* Body */
   RTCSPCB_PTR          rtcs_pcb = *rtcs_pcb_ptr_ptr;
   IF_IP_STATS_ENABLED(IP_CFG_STRUCT_PTR    IP_cfg_ptr);
   IP_HEADER_PTR packet = (IP_HEADER_PTR)RTCSPCB_DATA(rtcs_pcb);
   uint16_t       hdrlen, pktlen;
   uint16_t       chksum;
   unsigned char         ttl;
   uint32_t       error;

   IF_IP_STATS_ENABLED(IP_cfg_ptr = RTCS_getcfg(IP));

   /* Set up the transport layer protocol and delta. */
   RTCSPCB_SET_TRANS_PROTL(rtcs_pcb, mqx_ntohc(packet->PROTOCOL));
   RTCSPCB_SET_TRANS_DELTA(rtcs_pcb, ((mqx_ntohc(packet->VERSLEN) & 0x0F) << 2));

   /* Either no security database exists, or the packet has been decoded, or */
   /* the packet has been let through unchanged (bypass). */
   rtcs_pcb = *rtcs_pcb_ptr_ptr;

   if ((IP_IF_PTR)ifdest != RTCS_IF_LOCALHOST_PRV) {
      /*
      ** Check the TTL
      */

      ttl = mqx_ntohc(packet->TTL);
      if (ttl <= 1) {
         IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_RX_DISCARDED++);
         IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_RX_HDR_ERRORS++);
         IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_RX_TTL_EXCEEDED++);
         #if RTCSCFG_ENABLE_ICMP
         ICMP_send_error(ICMPTYPE_TIMEEXCEED, ICMPCODE_TE_TTL, 0, rtcs_pcb, 0);
         #endif
         return RTCSERR_IP_TTL;
      } /* Endif */

      /*
      ** Fork the PCB and duplicate the IP header.  Also copy up to
      ** 8 bytes of data in case we need to generate an ICMP error.
      */

      hdrlen = (mqx_ntohc(packet->VERSLEN) & 0x0F) << 2;
      pktlen = mqx_ntohs(packet->LENGTH);
      if (pktlen > hdrlen + 8) {
         pktlen = hdrlen + 8;
      } /* Endif */

      error = RTCSPCB_next(rtcs_pcb, pktlen);
      if (error) {
         IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_TX_MISSED++);
         IF_IP_STATS_ENABLED(RTCS_seterror(&IP_cfg_ptr->STATS.ERR_TX, error, (uint32_t)rtcs_pcb));
         return error;
      } /* Endif */

      error = RTCSPCB_fork(rtcs_pcb);
      if (error) {
         IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_TX_MISSED++);
         IF_IP_STATS_ENABLED(RTCS_seterror(&IP_cfg_ptr->STATS.ERR_TX, error, (uint32_t)rtcs_pcb));
         return error;
      } /* Endif */

      RTCSLOG_PCB_WRITE(rtcs_pcb, RTCS_LOGCTRL_PROTO(IPPROTO_IP), IP_VERSION);

      error = RTCSPCB_insert_header(rtcs_pcb, pktlen);
      if (error) {
         IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_TX_MISSED++);
         IF_IP_STATS_ENABLED(RTCS_seterror(&IP_cfg_ptr->STATS.ERR_TX, error, (uint32_t)rtcs_pcb));
         return error;
      } /* Endif */

      _mem_copy((unsigned char *)packet, RTCSPCB_DATA(rtcs_pcb), pktlen);
      packet = (IP_HEADER_PTR)RTCSPCB_DATA(rtcs_pcb);
      RTCSPCB_DATA_NETWORK(rtcs_pcb) = RTCSPCB_DATA(rtcs_pcb);

      /*
      ** Decrement the TTL and adjust the checksum
      */

      ttl--;
      (void) mqx_htonc(packet->TTL, ttl);
      chksum = mqx_ntohs(packet->CHECKSUM);
      chksum = IP_Sum_immediate(chksum, 0x100);
      (void) mqx_htons(packet->CHECKSUM, chksum);

      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_RX_FORWARDED++);

   } /* Endif */

   rtcs_pcb->LINK_OPTIONS.RX.DEST  = ipdest;
   rtcs_pcb->LINK_OPTIONS.RX.TOS   = mqx_ntohc(packet->TOS);
   rtcs_pcb->LINK_OPTIONS.RX.TTL   = mqx_ntohc(packet->TTL);

   return RTCS_OK;

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/************************************************************************
* NAME: IP_will_fragment
*
* DESCRIPTION: This function returns TRUE if the protocol message 
*              will be fragmented by IPv4, and FALSE otherwise.
*************************************************************************/
bool IP_will_fragment( IP_IF_PTR  ifdest, uint32_t protocol_message_size)
{
    int res;

    if((protocol_message_size + sizeof(IP_HEADER)) > ifdest->MTU)
        res = TRUE;
    else
        res = FALSE;

    return res;
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : IP_send_dgram
*  Returned Value : RTCS_OK or error code
*  Comments       :
*        Sends an IP datagram out an interface, fragmenting it if
*        necessary.
*
*END*-----------------------------------------------------------------*/

uint32_t IP_send_dgram
   (
      IP_IF_PTR      ifdest,     /* [IN] the outgoing interface */
      RTCSPCB_PTR    inrtcs_pcb, /* [IN] the packet to send */
      _ip_address    hopsrc,     /* [IN] the hop src */
      _ip_address    hopdest,    /* [IN] the hop dest */
      _ip_address    ipdest,     /* [IN] the ultimate dest */
      uint32_t        protocol,   /* [IN] the transport layer protocol */
      void          *data        /* [IN] routing entry data */
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   IP_CFG_STRUCT_PTR IP_cfg_ptr = RTCS_getcfg(IP);
   IP_HEADER_PTR     srciph;
   uint32_t           iphlen;
   uint32_t           srcdatalen;
   uint32_t           dmtumax, dmtu;
   uint32_t           srclen, offset, error;
   unsigned char         *srcdata;
   PCB_FRAGMENT_PTR  frag_ptr;
   
   RTCSLOG_FNE2(RTCSLOG_FN_IP_send_dgram, ifdest);

   error = inrtcs_pcb->IP_COMPLETE(ifdest, &inrtcs_pcb, hopsrc, ipdest, protocol);
   if (error) {
      RTCSLOG_PCB_FREE(inrtcs_pcb, error);
      RTCSPCB_free(inrtcs_pcb);
      
      RTCSLOG_FNX2(RTCSLOG_FN_IP_send_dgram, error);
      return error;
   } /* Endif */

    /* If the packet doesn't exceed the MTU, send it directly */
    if (RTCSPCB_SIZE(inrtcs_pcb) <= ifdest->MTU)
    {

    #if BSPCFG_ENET_HW_TX_PROTOCOL_CHECKSUM
        /* HW checksum offload.*/
        if( (((inrtcs_pcb->TYPE & RTCSPCB_TYPE_HW_PROTOCOL_CHECKSUM) == 0)
            && (ifdest->FEATURES & IP_IF_FEATURE_HW_TX_PROTOCOL_CHECKSUM))  
        #if RTCSCFG_LINKOPT_8023
            && (inrtcs_pcb->LINK_OPTIONS.TX.OPT_8023 == 0)
        #endif
            )
        {
            /* If an IP frame with a known protocol is transmitted, 
            * the checksum is inserted automatically into the frame. 
            * The checksum field MUST be cleared!!!!! 
            * This is workaround, in case the checksum is not cleared.*/
            IP_HEADER_PTR       ip_hdr = (IP_HEADER_PTR)RTCSPCB_DATA(inrtcs_pcb);
            uint32_t            ip_hdr_size = IPH_LEN(ip_hdr);
            unsigned char       *prot_hdr = (unsigned char *)ip_hdr + ip_hdr_size; 
            unsigned char       protocol; 
            
            protocol = mqx_ntohc(ip_hdr->PROTOCOL);

            /* Clear the checksum field.*/
            switch(protocol)
            {
                case IPPROTO_TCP:
                    if(RTCSPCB_SIZE(inrtcs_pcb) >= (ip_hdr_size+sizeof(TCP_HEADER))) /*Check length.*/
                        (void) mqx_htons(((TCP_HEADER_PTR)(prot_hdr))->checksum, 0);
                    break;
                case IPPROTO_UDP:
                    if(RTCSPCB_SIZE(inrtcs_pcb) >= (ip_hdr_size+sizeof(UDP_HEADER))) /*Check length.*/
                        (void) mqx_htons(((UDP_HEADER_PTR)(prot_hdr))->CHECKSUM, 0);
                    break;
                case IPPROTO_ICMP:
                    if(RTCSPCB_SIZE(inrtcs_pcb) >= (ip_hdr_size+sizeof(ICMP_HEADER))) /*Check length.*/
                        (void) mqx_htons(((ICMP_HEADER_PTR)(prot_hdr))->CHECKSUM, 0);
                    break;
            }
        }
    #endif
        
        /* Send to the interface.*/
        error = ifdest->DEVICE.SEND(ifdest, inrtcs_pcb, hopsrc, hopdest, data);
      
        RTCSLOG_FNX2(RTCSLOG_FN_IP_send_dgram, error);
        return error;
    } 

   srciph = (IP_HEADER_PTR)RTCSPCB_DATA(inrtcs_pcb);
   iphlen = (mqx_ntohc(srciph->VERSLEN) & 0x0F) << 2;
   srcdatalen = RTCSPCB_SIZE(inrtcs_pcb) - iphlen;

   /* If the datagram exceeds the MTU, but DF is set, respond with ICMP */
   if (mqx_ntohs(srciph->FRAGMENT) & IP_FRAG_DF) {
      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_TX_FRAG_DISCARDED++);
      #if RTCSCFG_ENABLE_ICMP
      ICMP_send_error(ICMPTYPE_DESTUNREACH, ICMPCODE_DU_NEEDFRAG, ifdest->MTU, inrtcs_pcb, 0);
      #endif
      RTCSLOG_PCB_FREE(inrtcs_pcb, RTCSERR_IP_CANTFRAG);
      RTCSPCB_free(inrtcs_pcb);
      
      RTCSLOG_FNX2(RTCSLOG_FN_IP_send_dgram, RTCSERR_IP_CANTFRAG);
      return RTCSERR_IP_CANTFRAG;
   } /* Endif */

   /* Sanity check.  Note: it's an error to bind an interface with an MTU < 68 */
   if (iphlen + IP_FRAG_MIN > ifdest->MTU) {
      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_TX_FRAG_DISCARDED++);
      RTCSLOG_PCB_FREE(inrtcs_pcb, RTCSERR_IP_SMALLMTU);
      RTCSPCB_free(inrtcs_pcb);
      
      RTCSLOG_FNX2(RTCSLOG_FN_IP_send_dgram, RTCSERR_IP_SMALLMTU);
      return RTCSERR_IP_SMALLMTU;
   } /* Endif */

   IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_TX_FRAG_FRAGD++);
   dmtumax = ifdest->MTU - iphlen;
   dmtu = dmtumax & (((uint32_t)(-1))<<IP_FRAG_SHIFT);
   offset = (mqx_ntohs(srciph->FRAGMENT) & IP_FRAG_MASK) << IP_FRAG_SHIFT;
   srcdata = RTCSPCB_DATA(inrtcs_pcb) + iphlen;
   srclen = inrtcs_pcb->HEADER_FRAG_USED - iphlen;
   frag_ptr = inrtcs_pcb->PCBPTR->FRAG;

   do {
      IP_HEADER_PTR  dstiph;
      unsigned char      *dstdata;
      uint32_t        dstlen = (srcdatalen > dmtumax) ? dmtu : srcdatalen;
      RTCSPCB_PTR    rtcs_pcb;
      uint16_t        chksum, fragment;

      /* Allocate the PCB */
      rtcs_pcb = RTCSPCB_alloc_send();
      if (rtcs_pcb == NULL) {
         IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_TX_MISSED++);
         RTCSLOG_PCB_FREE(inrtcs_pcb, RTCSERR_PCB_ALLOC);
         RTCSPCB_free(inrtcs_pcb);
         
         RTCSLOG_FNX2(RTCSLOG_FN_IP_send_dgram, RTCSERR_PCB_ALLOC);
         return RTCSERR_PCB_ALLOC;
      } /* Endif */

      /* Allocate the memory for the IP data */
      dstdata = RTCS_mem_alloc_system(dstlen);
      if (dstdata == NULL) {
         IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_TX_MISSED++);
         RTCSLOG_PCB_FREE(rtcs_pcb, RTCSERR_OUT_OF_MEMORY);
         RTCSPCB_free(rtcs_pcb);
         RTCSLOG_PCB_FREE(inrtcs_pcb, RTCSERR_OUT_OF_MEMORY);
         RTCSPCB_free(inrtcs_pcb);
         
         RTCSLOG_FNX2(RTCSLOG_FN_IP_send_dgram, RTCSERR_OUT_OF_MEMORY);
         return RTCSERR_OUT_OF_MEMORY;
      } /* Endif */
      _mem_set_type(dstdata, MEM_TYPE_IP_DATA);

      /* Append the data to the PCB */
      error = RTCSPCB_append_fragment_autofree(rtcs_pcb, dstlen, dstdata);
      if (error) {
         IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_TX_MISSED++);
         IF_IP_STATS_ENABLED(RTCS_seterror(&IP_cfg_ptr->STATS.ERR_TX, error, (uint32_t)rtcs_pcb));
         _mem_free(dstdata);
         RTCSLOG_PCB_FREE(rtcs_pcb, error);
         RTCSPCB_free(rtcs_pcb);
         RTCSLOG_PCB_FREE(inrtcs_pcb, error);
         RTCSPCB_free(inrtcs_pcb);
         
         RTCSLOG_FNX2(RTCSLOG_FN_IP_send_dgram, error);
         return error;
      } /* Endif */

      /* Allocate the IP header */
      error = RTCSPCB_insert_header(rtcs_pcb, iphlen);
      if (error) {
         IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.COMMON.ST_TX_MISSED++);
         IF_IP_STATS_ENABLED(RTCS_seterror(&IP_cfg_ptr->STATS.ERR_TX, error, (uint32_t)rtcs_pcb));
         /* don't _mem_free(dstdata) here because RTCSPCB_free(pcb) will do it */
         RTCSLOG_PCB_FREE(rtcs_pcb, error);
         RTCSPCB_free(rtcs_pcb);
         RTCSLOG_PCB_FREE(inrtcs_pcb, error);
         RTCSPCB_free(inrtcs_pcb);
         
         RTCSLOG_FNX2(RTCSLOG_FN_IP_send_dgram, error);
         return error;
      } /* Endif */

      /* Copy the IP header */
      dstiph = (IP_HEADER_PTR)RTCSPCB_DATA(rtcs_pcb);
      RTCSPCB_DATA_NETWORK(rtcs_pcb) = RTCSPCB_DATA(rtcs_pcb);
      _mem_copy(srciph, dstiph, iphlen);

      /* Update the LENGTH field */
      (void) mqx_htons(dstiph->LENGTH, iphlen + dstlen);

      /* Build the FRAGMENT field */
      fragment = offset >> IP_FRAG_SHIFT;
      offset     += dstlen;
      srcdatalen -= dstlen;
      /* An MF bit must be added on all fragments except the last */
      if (srcdatalen > 0 || (mqx_ntohs(srciph->FRAGMENT) & IP_FRAG_MF)) {
         fragment |= IP_FRAG_MF;
      } /* Endif */
      (void) mqx_htons(dstiph->FRAGMENT, fragment);

      /* Recompute the IP header checksum */
      (void) mqx_htons(dstiph->CHECKSUM, 0);

#if BSPCFG_ENET_HW_TX_IP_CHECKSUM 
        /* HW checksum offload.*/
        if( (((IP_IF_PTR)ifdest)->FEATURES & IP_IF_FEATURE_HW_TX_IP_CHECKSUM)  
    #if RTCSCFG_LINKOPT_8023
        && (rtcs_pcb->LINK_OPTIONS.TX.OPT_8023 == 0)
    #endif
           )
        {
            rtcs_pcb->TYPE|= RTCSPCB_TYPE_HW_IP_CHECKSUM;
        }
        else
#endif
        {
            chksum = _mem_sum_ip(0, iphlen, dstiph);
            chksum = IP_Sum_invert(chksum);
            (void) mqx_htons(dstiph->CHECKSUM, chksum);
        }

      /* Copy the IP data */
      for (;;) {
         uint32_t copylen = (srclen > dstlen) ? dstlen : srclen;
         _mem_copy(srcdata, dstdata, copylen);
         srcdata += copylen;
         dstdata += copylen;
         srclen  -= copylen;
         dstlen  -= copylen;
         if (dstlen == 0) break;
         frag_ptr++;
         srcdata = frag_ptr->FRAGMENT;
         srclen = frag_ptr->LENGTH;
      } /* Endfor */

      /* Send the fragment through the interface */
      IF_IP_STATS_ENABLED(IP_cfg_ptr->STATS.ST_TX_FRAG_SENT++);
      error = ifdest->DEVICE.SEND(ifdest, rtcs_pcb, hopsrc, hopdest, data);
      if (error) {
         RTCSLOG_PCB_FREE(inrtcs_pcb, error);
         RTCSPCB_free(inrtcs_pcb);
         
         RTCSLOG_FNX2(RTCSLOG_FN_IP_send_dgram, error);
         return error;
      } /* Endif */

   } while (srcdatalen > 0);

   RTCSLOG_PCB_FREE(inrtcs_pcb, RTCS_OK);
   RTCSPCB_free(inrtcs_pcb);
   
   RTCSLOG_FNX2(RTCSLOG_FN_IP_send_dgram, RTCS_OK);
   return RTCS_OK;

#else
    
    RTCSLOG_PCB_FREE(inrtcs_pcb, RTCSERR_IP_IS_DISABLED);
    RTCSPCB_free(inrtcs_pcb);
    return RTCSERR_IP_IS_DISABLED; 

#endif /* RTCSCFG_ENABLE_IP4 */
   
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IP_source
* Returned Value  : source IP address
* Comments        :
*     Return source IP address of packet.
*
*END*-----------------------------------------------------------------*/


_ip_address IP_source
   (
      RTCSPCB_PTR    rtcs_pcb      /* [IN] packet to find source of */
   )
{
#if RTCSCFG_ENABLE_IP4

   unsigned char *srcptr = ((IP_HEADER_PTR)RTCSPCB_DATA_NETWORK(rtcs_pcb))->SOURCE;

   return mqx_ntohl(srcptr);
   
#else
    
    return 0;

#endif /* RTCSCFG_ENABLE_IP4 */   

} 


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : IP_dest
* Returned Value  : destination IP address
* Comments        :
*     Return destination IP address of packet.
*
*END*-----------------------------------------------------------------*/

_ip_address IP_dest
   (
      RTCSPCB_PTR    rtcs_pcb      /* [IN] packet to find dest of */
   )
{ 
#if RTCSCFG_ENABLE_IP4

   unsigned char *dstptr = ((IP_HEADER_PTR)RTCSPCB_DATA_NETWORK(rtcs_pcb))->DEST;

   return mqx_ntohl(dstptr);
   
#else
    
    return 0;

#endif /* RTCSCFG_ENABLE_IP4 */
}




/* EOF */
