#ifndef __dhcpcln6_prv_h__
#define __dhcpcln6_prv_h__
/*HEADER**********************************************************************
*
* Copyright 2013-2014 Freescale Semiconductor, Inc.
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
*   Private header for DHCPv6 client.
*
*END************************************************************************/

#include <mqx.h>
#include <ip_if.h>
#include <dhcp6.h>
#include <dhcpcln6.h>
#include <lwmsgq.h>

#if RTCSCFG_ENABLE_IP6

#define DHCPCLN6_TASK_NAME      "DHCPv6 client task"
#define DHCPCLN6_TASK_PRIO      (RTCSCFG_DEFAULT_RTCSTASK_PRIO+1)
#define DHCPCLN6_STACK_SIZE     (2500)
#define DHCPCLN6_IAID_MAGIC     (0x44484350)
#define DHCPCLN6_DUID_LENGTH    (10)
#define DHCPCLN6_BUFFER_SIZE    (256)

#define DHCPCLN6_CONTEXT_VALID (0x44435856)
/*
 * Default values for T1 and T2.
 */
#define DHCPCLN6_DEFAULT_EXTEND_TIME_SERVER (600)  /* 10 minutes */
#define DHCPCLN6_DEFAULT_EXTEND_TIME_ANY    (3600) /* one hour */

#define DHCPCLN6_DEFAULT_PREFERRED_LIFETIME  (3600)
#define DHCPCLN6_DEFAULT_VALID_LIFETIME     (86400)

#define DHCPCLN6_ERR_OK        (0x0)
#define DHCPCLN6_ERR_NO_DUID   (0x1)
#define DHCPCLN6_ERR_NO_ID     (0x2)
#define DHCPCLN6_ERR_BAD_DUID  (0x3)
#define DHCPCLN6_ERR_BAD_SID   (0x4)
#define DHCPCLN6_ERR_BAD_IAID  (0x5)
#define DHCPCLN6_ERR_NO_ADDR   (0x6)
#define DHCPCLN6_ERR_SKIP_SOL  (0x7)
#define DHCPCLN6_ERR_STOP      (0x8)
#define DHCPCLN6_ERR_RESP      (0x9)
#define DHCPCLN6_ERR_FAIL      (0xA)
#define DHCPCLN6_ERR_TIMEOUT   (0xB)
#define DHCPCLN6_ERR_ABORT     (0xC)

#define DHCPCLN6_ID_MASK_CID   (0x01)
#define DHCPCLN6_ID_MASK_SID   (0x02)
#define DHCPCLN6_ID_MASK_IA_NA (0x04)
 
/* Number of messages in message queue. */
#define DHCPCLN6_NUM_MESSAGES (3)

/* Default time at which renew message exchange is started. */
#define DHCPCLN6_DEFAULT_T1     (3600)
/* Default time at which rebind message exchange is started. */
#define DHCPCLN6_DEFAULT_T2     (7200)
/* Time delay when between link checks */
#define DHCPCLN6_LINK_DELAY     (500)
/* Time delay between Duplicate address detection checks. */
#define DHCPCLN6_DAD_DELAY      (100)

/* Options offset in DHCP message */
#define DHCPCLN6_OPTIONS_OFFSET (sizeof(uint32_t))

#define DHCPCLN6_IS_RETR(x)     ((x->rt_context.rc) > (0))
/*
 * Parameters for DHCPv6 client task
 */
typedef struct dhcpcln6_task_params
{
    /* Initialization parameters. */
    DHCPCLN6_PARAM_STRUCT* init_params;
    /* Handle to DHCP client */
    uint32_t               handle;
}DHCPCLN6_TASK_PARAMS;

/*
 * Commands send by API 
 */
typedef enum dhcpcln6_api_command
{
    DHCPCLN6_COMMAND_RELEASE,
    DHCPCLN6_COMMAND_STOP
}DHCPCLN6_API_COMMAND;

/* Message send by API functions to DHCPv6 task. */
typedef struct dhcpcln6_api_msg
{   
    /* DHCP handle. */
    uint32_t             handle;
    /* Command to invoke. */
    DHCPCLN6_API_COMMAND command;
    /* Command parameter. */
    in6_addr             param;
}DHCPCLN6_API_MSG;

/*
 * DHCPv6 states. Do not change order!
 */
typedef enum dhcpcln6_state
{
    DHCPCLN6_STATE_SEND_SOLICIT,
    DHCPCLN6_STATE_RECV_ADVERTISE,
    DHCPCLN6_STATE_SEND_REQUEST,
    DHCPCLN6_STATE_RECV_REPLY,
    DHCPCLN6_STATE_SEND_DECLINE,
    DHCPCLN6_STATE_SEND_INF_REQ,
    DHCPCLN6_STATE_BOUND,
    DHCPCLN6_STATE_SEND_RELEASE,
    DHCPCLN6_STATE_SEND_CONFIRM,
    DHCPCLN6_STATE_SEND_RENEW,
    DHCPCLN6_STATE_SEND_REBIND,
    DHCPCLN6_STATE_WAIT_CMD,
    DHCPCLN6_STATE_FIN
}DHCPCLN6_STATE;

/*
 * IA address states
 */
typedef enum dhcpcln6_iaaddr_state
{
    DHCPCLN6_IAADDR_STATE_UNBOUND,
    DHCPCLN6_IAADDR_STATE_BOUND,
    DHCPCLN6_IAADDR_STATE_RELEASE,
    DHCPCLN6_IAADDR_STATE_DECLINE
}DHCPCLN6_IAADDR_STATE;

/*
 * DHCPv6 client IP address structure.
 */
typedef struct dhcpcln6_iaaddr_struct
{
    /* IPv6 address. */
    in6_addr                address;
    /* preferred lifetime. */
    uint32_t                p_lifetime;
    /* valid lifetime */
    uint32_t                v_lifetime;
    /* address state (bound, unbound, etc.) */
    DHCPCLN6_IAADDR_STATE   state;
}DHCPCLN6_IAADDR_STRUCT;

/*
 * DHCP retransmission context structure
 */
typedef struct dhcpcln6_rt_context
{
    /* Initial retransmission time */
    uint32_t    irt;
    /* Maximum retransmission time */
    uint32_t    mrt;
    /* Maximum retransmission count */
    uint32_t    mrc;
    /* Time in future (in SECONDS since boot-up) when transmission will expire */
    uint32_t    mrd;
    /* Last calculated retransmission time */
    uint32_t    rt;
    /* Retransmission count */
    uint32_t    rc;
}DHCPCLN6_RT_CONTEXT;

/*
 * Identity Association structure.
 */
typedef struct dhcpcln6_ia_struct
{
    /* IAID */
    uint32_t                id;
    /* Timeout 1 */
    uint32_t                t1;
    /* Timeout 2 */
    uint32_t                t2;
    /* Array of assigned addresses */
    DHCPCLN6_IAADDR_STRUCT  addresses[IP6_IF_ADDRESSES_MAX];
}DHCPCLN6_IA_STRUCT;

typedef struct dhcpcln6_message_struct
{
    /* Type of message */
    uint32_t               type;
    /* Transaction ID */
    uint32_t               xid;
    /* Size of data in buffer */
    uint32_t               size;
    /* Elapsed time */
    uint32_t               time_e;     
    /* Buffer for message sending/receiving */                          
    uint8_t                buffer[DHCPCLN6_BUFFER_SIZE];
}DHCPCLN6_MESSAGE_STRUCT;

typedef struct dhcpcln6_server_struct
{
    /* Unicast IPv6 address. */
    in6_addr    address;
    /* Server DUID. */
    uint8_t     duid[DHCP6_DUID_MAX_LENGTH];
    /* DUID size */
    uint32_t    duid_size;
    /* Preference value */
    uint8_t     preference;
}DHCPCLN6_SERVER_STRUCT;

/*
 * Client context structure.
 */
typedef struct dhcpcln6_context_struct
{
    /* Current state of interface link. */
    bool                     has_link;
    /* Previous state of interface link. */
    bool                     had_link;
    /* Client task ID */
    volatile _task_id        tid;
    /* Validity field */
    uint32_t                 valid;
    /* Time at which addresses were acquired (in seconds). */
    uint32_t                 time_base;
    /* Socket used for communication with server */
    _mqx_int                 sock;
    /* DHCP client state */
    DHCPCLN6_STATE           state;
    /* Previous state */
    DHCPCLN6_STATE           state_prev;
    /* User specified parameters */
    DHCPCLN6_PARAM_STRUCT    params;
    /* Identity association structure */
    DHCPCLN6_IA_STRUCT       ia;
    /* Buffer for message sending */                          
    DHCPCLN6_MESSAGE_STRUCT  *message_out;
    /* Buffer for message receiving */                          
    DHCPCLN6_MESSAGE_STRUCT  *message_in;
    /* Retransmission context */
    DHCPCLN6_RT_CONTEXT      rt_context;
    /* List of discovered servers */
    DHCPCLN6_SERVER_STRUCT   server;
    /* Message queue for API calls */
    void                     *msg_queue;
    /* Maximum retransmission time for solicit message */
    uint32_t                 sol_max_rt;
    /* Maximum retransmission time for information-request message */
    uint32_t                 inf_max_rt;
    /* Pointer to next context */
    struct dhcpcln6_context_struct  *next;
    /* Pointer to previous context */
    struct dhcpcln6_context_struct  *prev;
}DHCPCLN6_CONTEXT_STRUCT;

typedef uint32_t (*DHCPCLN6_SEND_FUCTION) (DHCPCLN6_CONTEXT_STRUCT *context);

/* 
 * DHCP client task prototype
 */

#ifdef __cplusplus
extern "C" {
#endif

void dhcpcln6_task(void *init_ptr, void *creator);
uint32_t dhcpcln6_check_params(DHCPCLN6_PARAM_STRUCT* params);
void dhcpcln6_state_machine(DHCPCLN6_CONTEXT_STRUCT* context);

/* Unbind addresses bound by DHCP. */
void DHCPCLN6_unbind(uint32_t handle, in6_addr *address);
/* Get DHCPv6 handle from interface handle. */
uint32_t DHCPCLN6_get_handle(void* if_handle);

#ifdef __cplusplus
}
#endif

#endif /* RTCSCFG_ENABLE_IP6 */
#endif
