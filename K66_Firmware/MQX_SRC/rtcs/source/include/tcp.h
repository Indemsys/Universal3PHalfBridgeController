#ifndef __tcp_h__
#define __tcp_h__
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
*   Definitions for the Transmission Control Protocol layer.
*
*
*END************************************************************************/


/*
** References
**
** [1] "Congestion Avoidance and Control", Van Jacobson, ACM SIGCOMM-88,
**      August 1988 (1988 ACM 0-89791-279-9/88/008/0314)
** [2] "Round Trip Time Estimation" (not full title),
**      P. Karn & C. Partridge, ACM SIGCOMM-87, August 1987
** RFC 793
** RFC 1122
** RFC 896
*/
#include "ip.h"
#include "ticker.h"

/************************************************************************/
/* Constants                                                            */
/************************************************************************/

/*
 *  Useful TCP assigned port numbers (see RFC 1010) (as in /etc/services)
 */
#define TCPPORT_ECHO       7   /* see RFC862; returns what was sent */
#define TCPPORT_DISCARD    9   /* see RFC863; returns nothing */
#define TCPPORT_TELNET    23   /* see RFC854, RFC855, ... */
#define TCPPORT_EXEC     512   /* `exec' service; see man rexecd(8C) */
#define TCPPORT_LOGIN    513   /* `login' service; see man rlogind(8C) */
#define TCPPORT_SHELL    514   /* `cmd' service; see man rshd(8C) */

/*  Options bits for TCB, also used in send and open calls
 */
#define TCPO_ACTIVE    0x0001  /* open/option: actively start connection */
#define TCPO_NONAGLE   0x0002  /* open/option: disable Nagle algorithm */
#define TCPO_NOSWITCH  0x0004  /* open/option: disable using user rcv buffer */
#define TCPO_NOWAIT    0x0010  /* send/rcv: return immediately */
#define TCPO_PUSH      0x0020  /* send/rcv: push data          */
#define TCPO_URGENT    0x0040  /* send/rcv: send bytes as urgent data */
#define TCPO_WAIT      0x0080  /* send/rcv: always block       */
                               /*    (wait for ack of data)    */

/*  Status bits for TCB, also used as event bits in wait and receive calls:
 */
#define TCPS_DUMMY     0x0000  /* Dummy event to TCP_Event */
#define TCPS_STARTED   0x0001  /* set once ESTABLISHED (or in any state in
                                  TCPSTATE_STARTED(state)), never cleared */
#define TCPS_FINACKED  0x0002  /* see TCPSTATE_FINACKED(state) */
#define TCPS_FINRECV   0x0004  /* see TCPSTATE_FINRECV(state) */
#define TCPS_FINDONE   0x0008  /* both FINACKED and FINRECV (not either) */
#define TCPS_FINTOSEND 0x2000  /* Close requested, FIN not necessarily sent */
#define TCPS_TPUSH     0x0010  /* PSH being sent, tcb->sndpush is valid */
#define TCPS_RPUSH     0x0020  /* PSH received, tcb->rcvpush is valid */
#define TCPS_TURGENT   0x0040  /* URG being sent, tcb->sndup is valid */
#define TCPS_RURGENT   0x0080  /* URG received, tcb->rcvup is valid */

/* transients */
#define TCPS_TREADY    0x0100  /* more data can be transmitted; set when send
                                  buffer becomes less than half full, cleared
                                  when send buffer becomes full */
#define TCPS_TACKED    0x0200  /* set when all sent data was acked; cleared if
                                  any more data is sent */
#define TCPS_TIMEOUT   0x0400  /* transmit timeout (transmission not ack'ed)*/
#define TCPS_SOFTERROR 0x0800  /* non-fatal ("soft") errors */
#define TCPS_NOWAIT    0x1000  /* return immediately */
#define TCPS_DEFER     0x4000  /* receive: asynchronous receive into reader's
                                  buffer; or, reader's buffer has been
                                  (asynchronously) filled */
#define TCPS_FINSENT   0x8000  /* FIN was sent to IP layer. */

#define TCB_VALID_ID   0x4b454720L
#define TCB_INVALID_ID 0x20544159L

/************************************************************************/
/* Macros                                                               */
/************************************************************************/
#if RTCSCFG_ENABLE_TCP_STATS
#define IF_TCP_STATS_ENABLED(x) x
#else
#define IF_TCP_STATS_ENABLED(x)
#endif

/************************************************************************/
/* Typedefs                                                             */
/************************************************************************/

/*
** TCP options set by upper layer
*/
typedef struct tcp_options_struct  {

   /*
   ** TCP_Transmit buffer size (use 0 to disable TCP buffer i.e. alway block on send,
   ** use -1 to default)
   */
   int32_t      tbsize;

   int32_t      rbsize;        /* Receive buffer size (use 0 to enforce
                                 that a user receive must be available, use
                                 -1 to default) */

   int32_t      utimeout;      /* User timeout (msec) - TCP R1 period */
   int32_t      timeout;       /* Connection timeout (msec) - TCP R2 period
                                 (use 0 to default) */
   int32_t      rto;           /* Initial retransmission timeout (use 0 to
                                 default) */
   int32_t      maxrto;        /* Max. retransmission timeout (use 0 to
                                 default) */
   uint32_t     maxrcvwnd;     /* Max. receive window size (use 0 to
                                 default) */
   
   unsigned char       ttl;           /* Time to live (use 0 to default) */
   unsigned char       tos;           /* Specify Internet type of service (use 0
                                 to default) */
   unsigned char       precede;       /* Specify precedence (0 to 7) (overrides
                                 precedence set in "tos" field) (use 0xFF
                                 to default) */

   int32_t      so_keepalive;     /* zero = disable keepalive; non-zero = enable keepalive */

   bool     active;        /* True when originating connection, false
                                 to listen */
   bool     nowait;        /* Return requests immediately to upper layer
                                 without waiting for completion */
   bool     nonagle;       /* Disable the Nagle algorithm */
   bool     noswrbuf;      /* Disable switching to user receive buffer */

   bool     bypass_rx_chksum;  /* TRUE to bypass checksum check on Rx */
   bool     bypass_tx_chksum;  /* TRUE to bypass checksum calc. on Tx */

   int32_t     timewaitto;     /* TIME_WAIT timeout (use 0 for 2*MSL) */
   bool     tcpsecuredraft0; /* draft-ietf-tcpm-tcpsecure-00.txt */
   uint32_t delayackto;   /* delay ack timeout */
   uint32_t backlog;      /* listen backlog */
} TCP_OPTIONS_STRUCT, * TCP_OPTIONS_PTR;

/*
** TCP parameters
*/
typedef struct tcb_parm {
   TCPIP_PARM              COMMON;
   struct tcb_parm        *NEXT;

   uint32_t        OPCODE;        /* Command to TCP */
   uint32_t        ERROR_CODE;    /* Error code returned */

   struct tcb_struct      *TCB_PTR;     /* TCP Context (TCB) */
   TCP_OPTIONS_PTR  OPTIONS_PTR; /* TcpOpen options */

   /* ip address, sa_family, port, if_scope_id */
   struct sockaddr * saddr_ptr;

   void          *BUFFER_PTR;    /* Send or receive buffer */
   uint32_t        BUFF_LENG;     /* Send or receive buffer length */
   int32_t         TIMEOUT;       /* Send or receive timeout */

   uint16_t        OPTIONS;       /* Send or receive flags */
   uint16_t        STATUS;        /* TCP connection status */

   uint32_t        URGENT;        /* Length of urgent data in buffer */
   uint32_t        disallow_mask;  

   uint32_t        SOCKET;        /* Upper layer socket structure */

} TCP_PARM, * TCP_PARM_PTR;

typedef struct tcp_message_struct
{
   MESSAGE_HEADER_STRUCT   HEADER;
   TCP_PARM                TCP_REQUEST;
} TCP_MESSAGE_STRUCT, * TCP_MESSAGE_STRUCT_PTR;


/*  Send buffer chain, describes contents of transmit buffer; some parts
 *   of it may be in the TCB's `sndringbuf', other parts may be from the
 *   clients' buffers themselves.
 */
typedef struct SbufNode {

   struct SbufNode      *next;/* next buffer in this output stream */

   unsigned char               *data; /* if use_cbuf is TRUE, this is ptr to client
                                 Send buffer; else (client is FALSE), this is
                                 the offset (by casting to (uint32_t)) in the
                                 TCB's `sndringbuf' of a piece of data to be
                                 sent (may wrap around the ring buffer) */

   int32_t               size; /* nb of bytes (in client buf or in ring) */

   TCP_PARM_PTR         req;  /* pointer to Send request, or NULL */

   bool              use_cbuf;
                              /* TRUE if the Send request buffer is
                                 being used to transmit, FALSE if
                                 transmitting from transmit ring buffer */

} SbufNode;

/*  An array of these is in each TCB (sndclks), to record transmit times for
 *   packets (segments)...
 */
typedef struct sndclock {

    uint32_t   seq;      /* 1st seq # past last byte transmitted */
    int32_t    time;     /* time (INFO(Time)) when packet transmitted */

} SndClock;

/*  These are used for all timers within TCB's; since expired TimeQ's can
 *   be automatically put in lists, the `tcb' field indicates in which TCB
 *   the expired TimeQ's are located.
 */
typedef struct tcbtimeq {

    TimeQ            t;     /* timer (see ticker.h); *must be 1st element**/
    struct tcb_struct      *tcb;   /* TCB which contains this timer */

} TcbTimeQ;

/*  Describes portions of the receive buffer which have been filled (received)
 */
typedef struct Rchunk {

    struct Rchunk      *next;
    int32_t              used; /* offset from beginning of receive buffer */
    int32_t              size; /* chunk size in bytes */

} Rchunk;

/*  Transmission Control Block (TCB)
 *  This is TCP's central data structure; one TCB is allocated to each
 *   TCP connection.
 */
typedef struct tcb_struct {
   struct tcb_struct         *next;             /* TCP server maintained list of TCB's */

   uint32_t  SOCKET;           /* Upper layer socket structure */
   uint32_t  VALID;            /* this TCB is valid if = TCB_VALID_ID */
                                                /* and validly closed if = TCB_INVALID_ID */
   bool  bypass_rx_chksum; /* TRUE to bypass checksum check on Rx */
   bool  bypass_tx_chksum; /* TRUE to bypass checksum calc. on Tx */

   uint16_t   state;          /* state of connection */

   uint16_t   status;         /* bit-wise status of connection (TCPS_*) */
   uint16_t   options;        /* TcpOpen() options (TCPO_*) */
   
   unsigned char     tos;            /* IP type-of-service (includes precedence) */

   int32_t    lasterror;      /* last error code on IP_Send(), or last soft
                                error; cleared when read by oclient */

   uint32_t   send_error;     /* last error on sending data  */

   struct sockaddr laddr; /* local sa_family, host and port; may be INADDR_ANY in LISTEN state */
   struct sockaddr raddr; /* remote sa_family, host and port */

   /* receive section */

   Rchunk      *rcvchunks;   /* portions of receive buffer which we have
                                (since we keep out-of-order packets' data)*/
   unsigned char      *rcvbuf;       /* current receive buffer (equals rcvringbuf or
                                rcvuserbuf); it is a circular buffer,
                                except when it equals rcvuserbuf ... */
   uint32_t     rcvlen;       /* size of receive buffer */
   uint32_t     rcvpos;       /* position within rcvbuf (0..rcvlen-1)
                                of next byte to be received by client;
                                always zero if rcvbuf==rcvuserbuf */
   unsigned char      *rcvringbuf;   /* default circular receive buffer */
   uint32_t     rcvringlen;   /* total size of rcvringbuf */
   unsigned char      *rcvuserbuf;   /* (user) buffer of Receive client, if any */
   uint32_t     rcvuserlen;   /* total size of rcvuserbuf */


   uint32_t   rcvusernxt;     /* nb of data bytes in rcvuserbuf; undefined in
                                the special case when rcvuserbuf==rcvbuf;
                                otherwise, if rcvuserbuf!=0 then the
                                implementation insures that whatever
                                contiguous data is available at the head
                                of the receive buffer has been put in the
                                user buffer, which implies that either the
                                data in rcvbuf is non-contiguous with that
                                in rcvuserbuf, or that rcvuserbuf is full */

   TCP_PARM_PTR   rcvHead;   /* First receive currently active */
   TCP_PARM_PTR   rcvTail;   /* Last receive currently active */

   uint16_t   conn_pending;   /* 1 = conn established but not accepted */
   uint16_t   rcvevents;      /* options as specified in TcpReceive();
                                may contain TCPS_RPUSH, ? */

   int32_t    rcvto;          /* user Receive() timeout (msecs) or 0 */
   uint32_t   rcvbufseq;      /* sequence # of 1st byte in rcvbuf */
                             /* The quantity  rcvbufseq + rcvlen  should
                                always increase monotonically (to avoid the
                                `shrinking window' misbehaviour) */
   uint32_t   rcvnxt;         /* next (contiguous) sequence # to receive */
   uint32_t   rcvwndedge;     /* right edge of advertised receive window
                                (bytes available in receive buffer for peer
                                TCP); left edge is rcvnxt, so size of
                                advertised rcv. wnd. is rcvwndedge - rcvnxt;
                                this size may be less than the real receive
                                window (because of SWS-avoidance and the
                                0xffff and rcvwndmax limits), which is
                                always rcvbufseq + rcvlen - rcvnxt */
   uint32_t   rcvwndmax;      /* maximum window to advertise; usually 0xffff,
                                but may be made 0x7fff for peer TCP's which
                                don't properly handle large advertised
                                windows, or be made the size of a packet
                                to avoid being sent back to back packets
                                */


   uint32_t   rcvup;          /* 1st seq # following received urgent data;
                                valid if (options & TCPS_RURGENT) != 0) */
   uint32_t   rcvpush;        /* 1st seq # following last received PSH;
                                valid if (options & TCPS_RPUSH) != 0) */
   uint32_t   rcvirs;         /* initial receive sequence # */

   uint32_t   rcvuna;         /* (1st) sequence # unacknowledged; used for
                                delayed ACKs (TCP_ACKDELAY_DEFAULT ms or
                                until rcv2full bytes) */


   TcbTimeQ  rcvtq;          /* user timer, for receive timeout */

   uint16_t   rcvmss;         /* maximum segment size for receiving (mss_r);
                                any IP or TCP options must be subtracted
                                from this value; computed at connection
                                setup and doesn't change */

   uint32_t   rcvmax;         /* biggest segment received (data only), but at
                                most rcvmss; used to compute rcv2full */
   uint32_t   rcv2full;       /* 1.75 times rcvmax, this is the amount of
                                data received which always triggers sending
                                an ACK; initially 1, it is computed from
                                rcvmax when the latter changes */

   /* send section */

   SbufNode      *sndbuf;    /* head of send buffer chain (0 if empty) */


   SbufNode      *sndbuftail;/* tail of send buffer chain (0 if empty) */
   uint32_t   sndlen;         /* total size of send buffers */

   unsigned char      *sndringbuf;   /* default circular transmit buffer */
   uint32_t   sndringlen;     /* length of sndringbuf */
   uint32_t   sndringhead;    /* next avail. byte within sndringbuf */
   uint32_t   sndringtail;    /* follows last avail. byte within sndringbuf */
   uint32_t   sndringavail;   /* nb of bytes available in sndringbuf */
   uint32_t   sndbufseq;      /* sequence # of 1st byte of sndbuf */
                             /* The quantity  sndbufseq + sndlen  should
                                always increase monotonically */

   uint32_t   snduna;         /* (1st) sequence # unacknowledged */
   uint32_t   sndxmit;        /* sequence # next to (re)transmit */
   int32_t    sndrto;         /* retransmission timeout (RTO) in msecs */
                             /* ^ expon. backed off by 2's... */
   int32_t    sndrtomax;      /* maximum value (clamp) for sndrto */
   int32_t    sndprobeto;     /* retransmission timeout for 0-window probes */
   int32_t    sndprobetomax;  /* maximum value (clamp) for sndproberto */
   int32_t    sndrtta;        /* RTT average estimate; scaled by 2^3 */
   int32_t    sndrttd;        /* RTT deviation estimate; scaled by 2^2 */

   TcbTimeQ  sndxmittq;      /* retransmit timer, for next packet to rexmit;
                                also used for TIME_WAIT delay, and for
                                probe retransmits (see sndxmitwhat) */

   uint16_t   keepalive;      /* when nonzero, next call to TCP_Transmit()
                                must send a keepalive probe */

   char      sndxmitwhat;    /* xmit: 0==idle/timewait, 1==probe, 2==xmit */
   bool   sndtohard;      /* true if sndtq active on sndto_2, that is,
                                soft timeout expired, hard timeout coming*/
   int32_t    timewaitto;     /* TIME_WAIT timeout (msecs) */
   int32_t    keepaliveto;    /* keepalive period (msecs), or 0 if none */
   
   uint32_t   delayackto;     /* DELAY_ACK timeout (msec) */

   TcbTimeQ  sndacktq;       /* transmit timer for delayed ACKs; actually,
                                this is generally used (TCP_Timer_oneshot_max())
                                when "something must be sent"
                                within perhaps a certain amount of time */

   int32_t    sndto_1;        /* user (soft) timeout value to use for sndtq,
                                0 if none */
   int32_t    sndto_2;        /* hard timeout value for sndtq, or 0 if none;
                                this timeout is started after sndtq expires
                                on sndto_1, so it is a delta from sndto_1;
                                if sndto_1 is zero, sndto_2 is also zero */
   TcbTimeQ  sndtq;          /* timer for timeout on ack of transmissions */

   SndClock      *sndclk;    /* packet transmission times, used for
                                calculating RTT (round trip time) */
   SndClock      *sndclkend; /* sndclk + sndclks */
   SndClock      *sndclkhead;/* circular queue of entries in sndclk[]... */
   SndClock      *sndclktail;/*    "    */
   int16_t    sndclks;        /* size of sndclk[] (nb of entries) */
   int16_t    sndclkavail;    /* nb of available entries in sndclk[] queue */

   uint32_t   sndnxt;         /* 1st sequence # of unsent data (next new data
                                to send) */
   uint32_t   sndwnd;         /* window allowed to send (from snduna) */

   int32_t    sndcwnd;        /* congestion window, in bytes, for Jacobson's
                                combined `slow-start' and congestion
                                avoidance (see [1]) */
   int32_t    sndstresh;      /* window size threshold for switching between
                                `slow-start' and congestion avoidance
                                (again, see [1]) */
   uint32_t   sndwl1;         /* sequence # of last seg. to update sndwnd */
   uint32_t   sndwl2;         /* ack # of last seg. to update sndwnd */

   uint16_t   sndmss;         /* maximum segment size for sending (mss_s);
                                the length of any IP or TCP options sent
                                (but not headers) must be subtracted from
                                this value */
   uint16_t   sndmax;         /* maximum data bytes to send in a segment;
                                this is sndmss - (length of IP and TCP
                                options sent with data) */
   uint32_t   sndup;          /* urgent void   *sequence # (1st byte after);
                                valid if (options & TCPS_TURGENT) != 0) */
   uint32_t   sndpush;        /* seq # of 1st byte after last PSH to send;
                                valid if (options & TCPS_TPUSH) != 0) */
   uint32_t   sndiss;         /* initial send sequence # */

   /* statistics */
   uint32_t   sndwndmax;      /* max value of sndwnd during connection */
   uint32_t   sndcwndmax;     /* max value of sndcwnd during connection */
   uint32_t   rexmts;         /* number of retransmissions */
   uint32_t   sndtmouts;      /* number of send timeouts */

   struct tcb_struct          *LISTEN_TCB;  /* The listening TCB which was cloned */
   struct tcb_struct          *NOSOCK_NEXT; /* A linked list of TCBs spawned from
                                        listening TCB */
   uint32_t   tcb_spawn_time; /* keep the time when tcb is created */

   RTCS_LINKOPT_STRUCT        TX;
   bool   tcpsecuredraft0; /* invoke draft-ietf-tcpm-tcpsecure-00.txt processing */
   int32_t    ackmodifier;     /* used by tcpsecure (above) processing to sometimes decriment value being ack'ed */
   
   TCPIP_EVENT expire;    /* the FIN-WAIT-2 timeout. */
   int32_t     blocking_close;
   uint32_t    backlog;   /* limit for number pending connections for a listen TCB. configured by user in listen(). */
   
   int32_t     keepcnt_act;   /* actual number of Tx'd keepalives. increments from zero up to keepcnt. */
   TCPIP_EVENT ev_keepalive;
   
   /* SOL_TCP socket options. as TCP specific, they are in the TCB structure. */
   int32_t    keepidle_ms;    /* configured value of SOL_TCP's TCP_KEEPIDLE option */
   int32_t    keepintvl_ms;   /* configured value of SOL_TCP's TCP_KEEPINTVL option */
   int32_t    keepcnt;        /* configured value of SOL_TCP's TCP_KEEPCNT option */
} TCB_STRUCT, * TCB_STRUCT_PTR;

/*
** TCP Configuration
**    This information is persistent for the TCP layer.
*/
typedef struct tcp_cfg_struct {
#if RTCSCFG_ENABLE_TCP_STATS
   TCP_STATS      STATS;         /* Statistics */
#endif

   TCB_STRUCT_PTR TCBhead;       /* List of active TCBs */
   Rchunk        *Rchunkhead;    /* List of active Rchunks */
   Rchunk        *Rchunkfree;    /* List of free Rchunks */
   SbufNode      *SbufNodehead;  /* List of active SbufNodes */
   SbufNode      *SbufNodefree;  /* List of free SbufNodes */

   uint32_t        nextno;        /* for TCB->no */
   ICB_STRUCT_PTR icb;           /* IP "connection" for TCP protocol */
   
   uint16_t        next_port;     /* next port to assign
                                    for TcpOpen with localport==0 */

   TimeQ         *qh;            /* the timeout queue head */
   uint32_t        lasttime;      /* time TCP_Tick_server() was last called */

   TimeQ           **qhead;      /* Address of the timeout queue head */

   TcbTimeQ      *acktq;         /* expired delayed-ACK timers */
   TcbTimeQ      *xmittq;        /* expired rexmit/probe/timewait timers*/
   TcbTimeQ      *sndtq;         /* expired ack-of-transmission timers */
   TcbTimeQ      *rcvtq;         /* expired receive timeout timers */

   TCP_PARM_PTR   OPENS;         /* pending open requests */
   TCPIP_EVENT    EVENT_TO_DEQUEUE_PCB; /*
                                           This event is used to
                                           dequeue an enqueued PCB
                                           (for simultaneos SYN problem)
                                           if it remains on the queue for
                                           more than 1 second
                                        */

   RTCSPCB_PTR       DEFER_HEAD;
   RTCSPCB_PTR      *DEFER_TAIL;

   uint32_t        CONN_COUNT;    /* current connection count */

   uint32_t        DEFER_COUNT;   /* number of deferred PCBs */

   TCB_STRUCT_PTR NOSOCK_TCB;    /* List of TCBs spawned from listening TCB */

   uint32_t         HALF_OPEN_TCB_COUNT;   /* current half open TCB count */
#if RTCSCFG_TCP_MAX_HALF_OPEN
   TCB_STRUCT_PTR  HALF_OPEN_TCB_LIST[RTCSCFG_TCP_MAX_HALF_OPEN];
                                 /* array of half open TCBs */
#endif

} TCP_CFG_STRUCT, * TCP_CFG_STRUCT_PTR;

typedef struct tcp_service_cfg_struct 
{  
  TCP_CFG_STRUCT_PTR  tcp_cfg_ptr;
  uint16_t sa_family;
} TCP_SERVICE_CFG_STRUCT, * TCP_SERVICE_CFG_STRUCT_PTR;

/************************************************************************/
/* Functions                                                            */
/************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t TCP_Init (void);

extern void TCP_Process_create  (TCP_PARM_PTR);
extern void TCP_Process_bind    (TCP_PARM_PTR);

extern void TCP_Process_open    (TCP_PARM_PTR);

extern void TCP_Process_accept  (TCP_PARM_PTR);

extern void TCP_Process_send    (TCP_PARM_PTR);
extern void TCP_Process_receive (TCP_PARM_PTR);

extern void TCP_Process_close   (TCP_PARM_PTR);
extern void TCP_Process_abort   (TCP_PARM_PTR);
extern void TCP_Process_shutdown(TCP_PARM_PTR);

extern void TCP_Process_signal  (void);

extern void TCP_Service_packet
   (
      RTCSPCB_PTR    pcb,     /* IN/OUT - the receive packet */
      void      *cfg      /* IN/OUT - TCP layer constants */
   ); 

#ifdef __cplusplus
}
#endif
   
#endif
/* EOF */
