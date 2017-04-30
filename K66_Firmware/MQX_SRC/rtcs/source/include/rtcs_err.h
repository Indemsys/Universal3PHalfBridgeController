#ifndef __rtcs_err_h__
#define __rtcs_err_h__
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
*   RTCS Error Definitions.
*
*
*END************************************************************************/

/*
** Successful return code.  No errors.
*/
#define RTCS_OK                     0
#define RTCS_ERROR                  (-1)
#define RTCS_HANDLE_ERROR           (0xffffffffL)
#define RTCS_SOCKET_ERROR           (0xffffffffL)


/*
** RTCS Error Code Definitions
*/

/*
** Generic Error codes
*/
#define RTCSERR_OUT_OF_MEMORY           (RTCS_ERROR_BASE|0x102)
#define RTCSERR_TIMEOUT                 (RTCS_ERROR_BASE|0x103)
#define RTCSERR_INVALID_ADDRESS         (RTCS_ERROR_BASE|0x104)
#define RTCSERR_INVALID_PARAMETER       (RTCS_ERROR_BASE|0x105)
#define RTCSERR_READ_ABORTED            (RTCS_ERROR_BASE|0x11f)
#define RTCSERR_OUT_OF_BUFFERS          (RTCS_ERROR_BASE|0x120)
#define RTCSERR_CREATE_PARTITION_FAILED (RTCS_ERROR_BASE|0x121)
#define RTCSERR_OUT_OF_SOCKETS          (RTCS_ERROR_BASE|0x122)
#define RTCSERR_FOPEN_FAILED            (RTCS_ERROR_BASE|0x123)
#define RTCSERR_FEATURE_NOT_ENABLED     (RTCS_ERROR_BASE|0x124)
#define RTCSERR_SEND_FAILED             (RTCS_ERROR_BASE|0x132)
#define RTCSERR_CREATE_POOL_FAILED      (RTCS_ERROR_BASE|0x135)
#define RTCSERR_OPEN_QUEUE_FAILED       (RTCS_ERROR_BASE|0x136)
#define RTCSERR_CREATE_FAILED           (RTCS_ERROR_BASE|0x137)
#define RTCSERR_RECEIVE_FAILED          (RTCS_ERROR_BASE|0x138)
#define RTCSERR_DEALLOC_FAILED          (RTCS_ERROR_BASE|0x139)
#define RTCSERR_SERVER_ALREADY_RUNNING  (RTCS_ERROR_BASE|0x140)
#define RTCSERR_SERVER_NOT_RUNNING      (RTCS_ERROR_BASE|0x141)

/*
** PPP error codes
*/
#define PPP_OK                                RTCS_OK
#define RTCSERR_PPP_ALLOC_FAILED              (RTCS_ERROR_BASE|0x200)
#define RTCSERR_PPP_INIT_MUTEX_FAILED         (RTCS_ERROR_BASE|0x201)
#define RTCSERR_PPP_CREATE_CTRL_POOL_FAILED   (RTCS_ERROR_BASE|0x202)
#define RTCSERR_PPP_CREATE_PKT_POOL_FAILED    (RTCS_ERROR_BASE|0x203)
#define RTCSERR_PPP_CREATE_RX_FAILED          (RTCS_ERROR_BASE|0x204)
#define RTCSERR_PPP_CREATE_TX_FAILED          (RTCS_ERROR_BASE|0x205)
#define RTCSERR_PPP_INVALID_CALLBACK          (RTCS_ERROR_BASE|0x206)
#define RTCSERR_PPP_HDLC_FAILED               (RTCS_ERROR_BASE|0x207)
#define RTCSERR_PPP_IO_FAILED                 (RTCS_ERROR_BASE|0x208)
#define RTCSERR_PPP_INVALID_HANDLE            (RTCS_ERROR_BASE|0x240)
#define RTCSERR_PPP_LINK_NOT_OPEN             (RTCS_ERROR_BASE|0x241)
#define RTCSERR_PPP_PACKET_TOO_SHORT          (RTCS_ERROR_BASE|0x242)
#define RTCSERR_PPP_PACKET_TOO_LONG           (RTCS_ERROR_BASE|0x243)
#define RTCSERR_PPP_OUT_OF_BUFFERS            (RTCS_ERROR_BASE|0x244)
#define RTCSERR_PPP_INVALID_PROTOCOL          (RTCS_ERROR_BASE|0x245)
#define RTCSERR_PPP_ALLOC_CALL_FAILED         (RTCS_ERROR_BASE|0x246)
#define RTCSERR_PPP_PROT_NOT_FOUND            (RTCS_ERROR_BASE|0x247)
#define RTCSERR_PPP_FSM_ACTIVE                (RTCS_ERROR_BASE|0x280)
#define RTCSERR_PPP_OPEN_QUEUE_FAILED         (RTCS_ERROR_BASE|0x2C0)

/*
** Initialization failures
*/
#define RTCSERR_INITIALIZED      (RTCS_ERROR_BASE|0x300)    /* RTCS already initialized */

/*
** PCB module errors
*/
#define RTCSERR_PCB_ALLOC        (RTCS_ERROR_BASE|0x310)    /* couldn't allocate PCBs         */
#define RTCSERR_PCB_FORK         (RTCS_ERROR_BASE|0x311)    /* PCB already forked             */
#define RTCSERR_PCB_LAYER        (RTCS_ERROR_BASE|0x312)    /* too few PCB layers             */
#define RTCSERR_PCB_BUFFER       (RTCS_ERROR_BASE|0x313)    /* PCB buffer too small           */
#define RTCSERR_PCB_FRAG         (RTCS_ERROR_BASE|0x314)    /* too few PCB fragments          */
#define RTCSERR_PCB_NOFRAG       (RTCS_ERROR_BASE|0x315)    /* can't add fragment (must fork) */
#define RTCSERR_PCB_DEPEND       (RTCS_ERROR_BASE|0x316)    /* can't create dependency        */

/*
** ARP errors
*/
#define RTCSERR_ARP_CFG          (RTCS_ERROR_BASE|0x400)    /* couldn't allocate state       */
#define RTCSERR_ARP_CACHE        (RTCS_ERROR_BASE|0x401)    /* couldn't allocate cache entry */
#define RTCSERR_ARP_CANT_RESOLVE (RTCS_ERROR_BASE|0x402)    /* couldn't resolve address      */
#define RTCSERR_ARP_BAD_HEADER   (RTCS_ERROR_BASE|0x403)    /* error in header               */

/*
** IPCP errors
*/
#define RTCSERR_IPCP_CFG         (RTCS_ERROR_BASE|0x410)    /* couldn't allocate state */

/*
** IP errors
*/
#define RTCSERR_IP_ICB_ALLOC     (RTCS_ERROR_BASE|0x500)    /* couldn't allocate ICB       */
#define RTCSERR_IP_PROT_OPEN     (RTCS_ERROR_BASE|0x501)    /* protocol already open       */
#define RTCSERR_IP_IF_ALLOC      (RTCS_ERROR_BASE|0x502)    /* couldn't allocate interface */
#define RTCSERR_IP_ROUTE_ALLOC   (RTCS_ERROR_BASE|0x503)    /* couldn't allocate route     */
#define RTCSERR_IP_GATE_ALLOC    (RTCS_ERROR_BASE|0x504)    /* couldn't allocate gateway   */
#define RTCSERR_IP_BIND_ADDR     (RTCS_ERROR_BASE|0x505)    /* invalid local address       */
#define RTCSERR_IP_BIND_MASK     (RTCS_ERROR_BASE|0x506)    /* invalid network mask        */
#define RTCSERR_IP_SEC_ALLOC     (RTCS_ERROR_BASE|0x507)    /* couldn't allocate SPD node   */
#define RTCSERR_IP_VIRTUAL_ALLOC (RTCS_ERROR_BASE|0x508)    /* couldn't alloc virtual route */

/*
** IP datagram processing failures
*/
#define RTCSERR_IP_UNREACH       (RTCS_ERROR_BASE|0x510)    /* no route to host            */
#define RTCSERR_IP_TTL           (RTCS_ERROR_BASE|0x511)    /* TTL expired                 */
#define RTCSERR_IP_SMALLMTU      (RTCS_ERROR_BASE|0x512)    /* packet exceeds MTU          */
#define RTCSERR_IP_CANTFRAG      (RTCS_ERROR_BASE|0x513)    /* need to fragment but DF set */
#define RTCSERR_IP_BAD_HEADER    (RTCS_ERROR_BASE|0x514)    /* error in header             */
#define RTCSERR_IP_BAD_ADDRESS   (RTCS_ERROR_BASE|0x515)    /* illegal source or dest      */
#define RTCSERR_IP_BAD_CHECKSUM  (RTCS_ERROR_BASE|0x516)    /* invalid checksum            */

/*
** IPIP errors
*/
#define RTCSERR_IPIP_NOT_INITIALIZED   (RTCS_ERROR_BASE|0x550)   /* IPIP not initialized      */
#define RTCSERR_IPIP_LOOP              (RTCS_ERROR_BASE|0x551)   /* received a packet we sent */

/*
** ICMP errors
*/
#define RTCSERR_ICMP_ECHO_TIMEOUT   (RTCS_ERROR_BASE|0x580) /* timed out waiting for echo reply */
#define RTCSERR_ICMP_BAD_HEADER     (RTCS_ERROR_BASE|0x581) /* error in header                  */
#define RTCSERR_ICMP_BAD_CHECKSUM   (RTCS_ERROR_BASE|0x582) /* invalid checksum                 */

/*
** IGMP errors
*/
#define RTCSERR_IGMP_CFG            (RTCS_ERROR_BASE|0x5C0) /* couldn't allocate state        */
#define RTCSERR_IGMP_GROUP_ALLOC    (RTCS_ERROR_BASE|0x5C1) /* couldn't allocate MCB          */
#define RTCSERR_IGMP_GROUP_FREE     (RTCS_ERROR_BASE|0x5C2) /* couldn't free MCB              */
#define RTCSERR_IGMP_INVALID_IP     (RTCS_ERROR_BASE|0x5C3) /* can't join group (nonlocal IP) */
#define RTCSERR_IGMP_NOT_JOINED     (RTCS_ERROR_BASE|0x5C4) /* can't leave group (not joined) */
#define RTCSERR_IGMP_BAD_HEADER     (RTCS_ERROR_BASE|0x5C5) /* error in header                */
#define RTCSERR_IGMP_BAD_CHECKSUM   (RTCS_ERROR_BASE|0x5C6 )/* invalid checksum               */

/*
** UDP errors
*/
#define RTCSERR_UDP_UCB_ALLOC    (RTCS_ERROR_BASE|0x660)    /* couldn't allocate UCB */
#define RTCSERR_UDP_UCB_FREE     (RTCS_ERROR_BASE|0x661)    /* couldn't free UCB     */
#define RTCSERR_UDP_UCB_CLOSE    (RTCS_ERROR_BASE|0x662)    /* UCB not open          */
#define RTCSERR_UDP_PORT_OPEN    (RTCS_ERROR_BASE|0x663)    /* port already open     */
#define RTCSERR_UDP_PORT_ALLOC   (RTCS_ERROR_BASE|0x664)    /* no more ports         */
#define RTCSERR_UDP_PORT_ZERO    (RTCS_ERROR_BASE|0x665)    /* can't send to port 0  */
#define RTCSERR_UDP_BAD_HEADER   (RTCS_ERROR_BASE|0x666)    /* error in header       */
#define RTCSERR_UDP_BAD_CHECKSUM (RTCS_ERROR_BASE|0x667)    /* invalid checksum      */
#define RTCSERR_UDP_SOCKET_CLOSE (RTCS_ERROR_BASE|0x668)    /* socket closed by tasks other than waiting task */


/*
** TCPIP error codes
*/
#define RTCSERR_TCPIP_INVALID_ARGUMENT  (RTCS_ERROR_BASE|0x610)  /* Invalid argument            */
#define RTCSERR_TCPIP_DESTADDR_REQUIRED (RTCS_ERROR_BASE|0x611)  /* Destination address         */
                                                                 /*    required                 */
#define RTCSERR_TCPIP_NO_BUFFS          (RTCS_ERROR_BASE|0x612)  /* No buffer space available   */
#define RTCSERR_TCPIP_DELAY_REQUESTED   (RTCS_ERROR_BASE|0x613)  /* TCP_Delay() called, but not */
                                                                 /*    supported                */
#define RTCSERR_TCPIP_TIMER_CORRUPT     (RTCS_ERROR_BASE|0x614)  /* Corrupt timer pointers      */

/*
** TCP error codes
*/
#define RTCSERR_TCP_OPEN_FAILED         (RTCS_ERROR_BASE|0x630)  /* TcpOpen failed              */
#define RTCSERR_TCP_INVALID_OPTION      (RTCS_ERROR_BASE|0x631)  /* Option was invalid          */
#define RTCSERR_TCP_IN_PROGRESS         (RTCS_ERROR_BASE|0x632)  /* Operation already in        */
                                                                 /*    progress                 */
#define RTCSERR_TCP_ADDR_IN_USE         (RTCS_ERROR_BASE|0x633)  /* Address already in use      */
#define RTCSERR_TCP_ADDR_NA             (RTCS_ERROR_BASE|0x634)  /* Can't assign requested      */
                                                                 /*    address                  */
#define RTCSERR_TCP_CONN_ABORTED        (RTCS_ERROR_BASE|0x635)  /* Software caused connection  */
                                                                 /*    abort                    */
#define RTCSERR_TCP_CONN_RESET          (RTCS_ERROR_BASE|0x636)  /* Connection reset by peer    */
#define RTCSERR_TCP_HOST_DOWN           (RTCS_ERROR_BASE|0x637)  /* Host is down                */
#define RTCSERR_TCP_CONN_CLOSING        (RTCS_ERROR_BASE|0x638)  /* Connection closing          */
#define RTCSERR_TCP_CONN_RLSD           (RTCS_ERROR_BASE|0x639)  /* Connection/TCB released     */
#define RTCSERR_TCP_MISSING_OPEN        (RTCS_ERROR_BASE|0x63A)  /* TCB exists in LISTEN state  */
                                                                 /*    with no associated Listn */
                                                                 /*    request from upper layer */
#define RTCSERR_TCP_CTR_ZERO_RSIZE      (RTCS_ERROR_BASE|0x63B)  /* TCP_Copy_to_ring got 0      */
                                                                 /*    ring buf size            */
#define RTCSERR_TCP_SP_BAD_SEND_STATE   (RTCS_ERROR_BASE|0x63C)  /* Attempted to TCP_Send_packet*/
                                                                 /*    in invalid state         */
#define RTCSERR_TCP_SP_OUT_OF_PCBS      (RTCS_ERROR_BASE|0x63D)  /* Could not get a PCB in      */
                                                                 /*    TCP_Send_packet          */
#define RTCSERR_TCP_SRR_OUT_OF_PCBS     (RTCS_ERROR_BASE|0x63E)  /* Could not get a PCB in      */
                                                                 /*    TCP_Send_reply_reset     */
#define RTCSERR_TCP_SR_OUT_OF_PCBS      (RTCS_ERROR_BASE|0x63F)  /* Could not get a PCB in      */
                                                                 /*    TCP_Send_reset           */
#define RTCSERR_TCP_SHRINKER_HOST       (RTCS_ERROR_BASE|0x640)  /* TCP detected a 'shrinker'   */
                                                                 /*    peer host                */
#define RTCSERR_TCP_PA_BUFF_CORRUPT     (RTCS_ERROR_BASE|0x642)  /* TCP send buffer corruption  */
                                                                 /*    detected in TCP_Process_ack   */
#define RTCSERR_TCP_PS_FAILED_GET_SBUF  (RTCS_ERROR_BASE|0x643)  /* Could not get an SbufNode   */
                                                                 /*    in TCP_Process_send      */
#define RTCSERR_TCP_PR_OUT_OF_MEM       (RTCS_ERROR_BASE|0x644)  /* Could not get a Rcvchunk    */
                                                                 /*    buffer in Setup Receive  */
#define RTCSERR_TCP_PP_OUT_OF_MEM       (RTCS_ERROR_BASE|0x645)  /* Could not add a host IpList */
                                                                 /*    node in TCP_Process_packet    */
#define RTCSERR_TCP_TIMER_FAILURE       (RTCS_ERROR_BASE|0x646)  /* Could not start a TCP timer */
#define RTCSERR_TCP_NOT_CONN            (RTCS_ERROR_BASE|0x647)  /* Socket is not connected     */
#define RTCSERR_TCP_SHUTDOWN            (RTCS_ERROR_BASE|0x648)  /* Can't send after socket     */
                                                                 /*    shutdown                 */
#define RTCSERR_TCP_TIMED_OUT           (RTCS_ERROR_BASE|0x649)  /* Connection timed out        */
#define RTCSERR_TCP_CONN_REFUSED        (RTCS_ERROR_BASE|0x64A)  /* Connection refused          */
#define RTCSERR_TCP_NO_MORE_PORTS       (RTCS_ERROR_BASE|0x64B)  /* No more ports available     */
                                                                 /*    for connections          */
#define RTCSERR_TCP_BAD_STATE_FOR_CLOSE (RTCS_ERROR_BASE|0x64C)  /* Attempted to call           */
                                                                 /*    TCP_Process_effective_close or */
                                                                 /*    TCP_Process_close in invalid  */
                                                                 /*    state                    */
#define RTCSERR_TCP_BAD_STATE_FOR_REL   (RTCS_ERROR_BASE|0x64D)  /* Attempted to release a TCB  */
                                                                 /*    in an invalid state      */
#define RTCSERR_TCP_REXMIT_PROBLEM      (RTCS_ERROR_BASE|0x64E)  /* A retransmission timer      */
                                                                 /*    timed out while in IDLE  */
                                                                 /*    mode (i.e. invalid)      */
#define RTCSERR_TCP_URGENT_DATA_PROB    (RTCS_ERROR_BASE|0x64F)  /* Urgent data pointer         */
                                                                 /*    was corrupted in the tcb */
#define RTCSERR_TCP_DEALLOC_FAILED      (RTCS_ERROR_BASE|0x650)  /* A call to release memory failed   */
#define RTCSERR_TCP_HOST_UNREACH        (RTCS_ERROR_BASE|0x651)  /* No route to host            */
#define RTCSERR_TCP_BAD_HEADER          (RTCS_ERROR_BASE|0x652)  /* error in header             */
#define RTCSERR_TCP_BAD_CHECKSUM        (RTCS_ERROR_BASE|0x653)  /* invalid checksum            */

/*
** Socket error codes
*/
#define RTCSERR_SOCK_INVALID           (RTCS_ERROR_BASE|0x704)
#define RTCSERR_SOCK_INVALID_AF        (RTCS_ERROR_BASE|0x702)
#define RTCSERR_SOCK_SHORT_ADDRESS     (RTCS_ERROR_BASE|0x706)
#define RTCSERR_SOCK_NOT_BOUND         (RTCS_ERROR_BASE|0x709)
#define RTCSERR_SOCK_IS_BOUND          (RTCS_ERROR_BASE|0x705)
#define RTCSERR_SOCK_NOT_LISTENING     (RTCS_ERROR_BASE|0x70C)
#define RTCSERR_SOCK_IS_LISTENING      (RTCS_ERROR_BASE|0x70B)
#define RTCSERR_SOCK_NOT_CONNECTED     (RTCS_ERROR_BASE|0x70D)
#define RTCSERR_SOCK_IS_CONNECTED      (RTCS_ERROR_BASE|0x70A)
#define RTCSERR_SOCK_NOT_OWNER         (RTCS_ERROR_BASE|0x70E)
#define RTCSERR_SOCK_CLOSED            (RTCS_ERROR_BASE|0x70F)
#define RTCSERR_SOCK_NOT_SUPPORTED     (RTCS_ERROR_BASE|0x710)
#define RTCSERR_SOCK_INVALID_OPTION    (RTCS_ERROR_BASE|0x711)
#define RTCSERR_SOCK_SHORT_OPTION      (RTCS_ERROR_BASE|0x712)
#define RTCSERR_SOCK_ALLOC_GROUP       (RTCS_ERROR_BASE|0x713)
#define RTCSERR_SOCK_NOT_JOINED        (RTCS_ERROR_BASE|0x714) /* Can't leave group (not joined) */
#define RTCSERR_SOCK_INVALID_PARAMETER (RTCS_ERROR_BASE|0x715)
#define RTCSERR_SOCK_EBADF             (RTCS_ERROR_BASE|0x716)
#define RTCSERR_SOCK_EINVAL            (RTCS_ERROR_BASE|0x717)
#define RTCSERR_SOCK_OPTION_IS_READ_ONLY  (RTCS_ERROR_BASE|0x718)
#define RTCSERR_SOCK_ESHUTDOWN         (RTCS_ERROR_BASE|0x719)

/*
** SNMP TRAP Interface Error Codes
*/
#define RTCSERR_TRAP_INSERT_FAILED      (RTCS_ERROR_BASE|0x901)  /* TRAP Insertion failed     */
#define RTCSERR_TRAP_REMOVE_FAILED      (RTCS_ERROR_BASE|0x902)  /* TRAP Insertion failed     */

/*
** DHCP error codes
*/
#define RTCSERR_DHCP_ALREADY_INIT           (RTCS_ERROR_BASE|0xA01)
#define RTCSERR_DHCP_MESSAGE_OPTION_MISSING (RTCS_ERROR_BASE|0xA02)
#define RTCSERR_DHCP_SERVER_OPTION_MISSING  (RTCS_ERROR_BASE|0xA03)
#define RTCSERR_DHCP_ADDR_IN_USE            (RTCS_ERROR_BASE|0xA04)
#define RTCSERR_DHCP_PACKET_ERROR           (RTCS_ERROR_BASE|0xA81)
#define RTCSERR_DHCPCLNT_ERROR_DECLINED     (RTCS_ERROR_BASE|0xA82)
#define RTCSERR_DHCPCLNT_XID_MISMATCH       (RTCS_ERROR_BASE|0xA83)
#define RTCSERR_DHCPCLNT_PACKET_SIZE_ERROR  (RTCS_ERROR_BASE|0xA84)

/*
** DNS error codes
*/
#define DNS_OK                                      RTCS_OK
#define RTCSERR_DNS_QUERY_OK                        (RTCS_ERROR_BASE|0xB00)
#define RTCSERR_DNS_NO_NAME_SERVER_RESPONSE         (RTCS_ERROR_BASE|0xB01)
#define RTCSERR_DNS_UNABLE_TO_OPEN_SOCKET           (RTCS_ERROR_BASE|0xB02)
#define RTCSERR_DNS_UNABLE_TO_BIND_SOCKET           (RTCS_ERROR_BASE|0xB03)
#define RTCSERR_DNS_UNABLE_TO_UPDATE_ROOT_SERVERS   (RTCS_ERROR_BASE|0xB04)
#define RTCSERR_DNS_SOCKET_RECV_ERROR               (RTCS_ERROR_BASE|0xB05)
#define RTCSERR_DNS_UNABLE_TO_SEND_QUERY            (RTCS_ERROR_BASE|0xB06)
#define RTCSERR_DNS_NO_RESPONSE_FROM_RESOLVER       (RTCS_ERROR_BASE|0xB07)
#define RTCSERR_DNS_PACKET_RECEPTION_ERROR          (RTCS_ERROR_BASE|0xB08)
#define RTCSERR_DNS_INVALID_NAME                    (RTCS_ERROR_BASE|0xB09)
#define RTCSERR_DNS_INVALID_IP_ADDR                 (RTCS_ERROR_BASE|0xB0A)
#define RTCSERR_DNS_ALL_SERVERS_QUERIED             (RTCS_ERROR_BASE|0xB0B)
#define RTCSERR_DNS_INVALID_LOCAL_NAME              (RTCS_ERROR_BASE|0xB0C)
#define RTCSERR_DNS_UNABLE_TO_ALLOCATE_MEMORY       (RTCS_ERROR_BASE|0xB0D)
#define RTCSERR_DNS_UNABLE_TO_FREE_SOCKET           (RTCS_ERROR_BASE|0xB0E)
#define RTCSERR_DNS_VALID_IP_ADDR                   (RTCS_ERROR_BASE|0xB0F)
#define RTCSERR_DNS_INITIALIZATION_FAILURE          (RTCS_ERROR_BASE|0xB10)
#define RTCSERR_DNS_NOT_IMPLEMENTED                 (RTCS_ERROR_BASE|0xB11)
#define RTCSERR_DNS_UNABLE_TO_CREATE_PARTITION      (RTCS_ERROR_BASE|0xB12)
#define RTCSERR_DNS_SHORT_MESSAGE_ERROR             (RTCS_ERROR_BASE|0xB13)

/*
** NAT error codes
*/
#define RTCSERR_NAT_NO_SESSION                   (RTCS_ERROR_BASE|0xC00)
#define RTCSERR_NAT_UNSUPPORTED_PROTOCOL         (RTCS_ERROR_BASE|0xC01)
#define RTCSERR_NAT_INITIALIZED                  (RTCS_ERROR_BASE|0xC02)
#define RTCSERR_NAT_UNEXPECTED                   (RTCS_ERROR_BASE|0xC03)
#define RTCSERR_NAT_NOT_INITIALIZED              (RTCS_ERROR_BASE|0xC04)
#define RTCSERR_NAT_INVALID_RULE                 (RTCS_ERROR_BASE|0xC05)
#define RTCSERR_NAT_INVALID_PRIVATE_NETWORK      (RTCS_ERROR_BASE|0xC07)
#define RTCSERR_NAT_MAX_PRIVATE_NETWORKS         (RTCS_ERROR_BASE|0xC08)
#define RTCSERR_NAT_DUPLICATE_PRIORITY           (RTCS_ERROR_BASE|0xC09)
#define RTCSERR_NAT_END_OF_RULES                 (RTCS_ERROR_BASE|0xC0A)
#define RTCSERR_NAT_INVALID_PRIVATE_ADDRESS      (RTCS_ERROR_BASE|0xC0B)

/*
** IPCFG error codes
*/
#define IPCFG_OK                        RTCS_OK
#define RTCSERR_IPCFG_BUSY              (RTCS_ERROR_BASE|0xD01)
#define RTCSERR_IPCFG_DEVICE_NUMBER     (RTCS_ERROR_BASE|0xD02)
#define RTCSERR_IPCFG_INIT              (RTCS_ERROR_BASE|0xD03)
#define RTCSERR_IPCFG_BIND              (RTCS_ERROR_BASE|0xD04)
#define RTCSERR_IPCFG_NOT_INIT          (RTCS_ERROR_BASE|0xD05)


/****************************************************************
* Additional RTCS IPv6 error codes .
*****************************************************************/
#define RTCSERR_IP6_IF_NO_ADDRESS_SPACE      (RTCS_ERROR_BASE|0xE01)
#define RTCSERR_ND6_CFG                      (RTCS_ERROR_BASE|0xE02)
#define RTCSERR_IP_IS_DISABLED               (RTCS_ERROR_BASE|0xE03)


/*
** Protocol layer definitions.  These are used by RTCS_[non]fatal_error().
*/
#define ERROR_RTCS                  (0x1)
#define ERROR_SOCKET                (0x2)
#define ERROR_TCPIP                 (0x3)
#define ERROR_TCP                   (0x4)
#define ERROR_UDP                   (0x5)
#define ERROR_RPC                   (0x6)
#define ERROR_ECHO                  (0x7)
#define ERROR_EDS                   (0x8)
#define ERROR_TELNET                (0x9)
#define ERROR_DHCPSRV               (0xA)
#define ERROR_DNS                   (0xB)
#define ERROR_IGMP                  (0xC)

#endif


/* EOF */
