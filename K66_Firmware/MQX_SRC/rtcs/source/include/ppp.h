#ifndef __ppp_h__
#define __ppp_h__
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
*   This file contains the defines, externs and data
*   structure definitions required by application
*   programs in order to use the PPP Library.
*
*
*END************************************************************************/


#include <rtcs.h>
#if RTCSCFG_ENABLE_PPP && !PLATFORM_SDK_ENABLED

#include <rtcsrtos.h>
#include <rtcs_in.h>
#if MQX_USE_IO_OLD
  #include <fio.h>
#else
  #include <stdio.h>
#endif
#include <ppphdlc.h>

#ifndef PPP_SECRETS_NOT_SHARED
    #define PPP_SECRETS_NOT_SHARED 0
#endif

/***************************************
**
** Constants
*/


/*
** The default and minimum Maximum Receive Unit.
*/

#define DEFAULT_MRU  1500
#define MINIMUM_MRU    68

/*
** LCP States.  Do not modify these constants!
*/

#define PPP_STATE_INITIAL  0
#define PPP_STATE_STARTING 1
#define PPP_STATE_CLOSED   2
#define PPP_STATE_STOPPED  3
#define PPP_STATE_CLOSING  4
#define PPP_STATE_STOPPING 5
#define PPP_STATE_REQ_SENT 6
#define PPP_STATE_ACK_RCVD 7
#define PPP_STATE_ACK_SENT 8
#define PPP_STATE_OPENED   9

/*
** LCP Options.  These control the LCP negotiation.
*/

#define PPP_OPT_SILENT  0x01
#define PPP_OPT_RESTART 0x02


/*
** PPP callback function numbers.
*/

#define PPP_CALL_LINK_UP      0
#define PPP_CALL_LINK_DOWN    1
#define PPP_CALL_ECHO_REPLY   2
#define PPP_CALL_AUTH_FAIL    3

#define PPP_CALL_MAX          4

/*
 * ppp connection status
 */
#define PPP_CONNECTED         0
#define PPP_DISCONNECTED      1

/* Start CR 2207 */
#define PPP_PAP_LSECRET       1
#define PPP_PAP_RSECRETS      2
#define PPP_CHAP_LSECRETS     3
#define PPP_CHAP_RSECRETS     4
/* End CR 2207 */
/***************************************
**
** Global parameters
*/

extern uint32_t _PPPTASK_stacksize;
extern uint32_t _PPPTASK_priority;

extern uint32_t _PPP_MIN_XMIT_TIMEOUT;
extern uint32_t _PPP_MAX_XMIT_TIMEOUT;
extern uint32_t _PPP_MAX_CONF_RETRIES;
extern uint32_t _PPP_MAX_TERM_RETRIES;
extern uint32_t _PPP_MAX_CONF_NAKS;

extern uint32_t _PPP_ACCM;


/***************************************
**
** Type definitions
*/

/*
** The PPP handle.  This is returned by PPP_init(), and required
** as a parameter for all other PPP functions.
*/

typedef void   *_ppp_handle;

/*
** A secret.  This is the name/secret pair used by both PAP and CHAP.
*/

typedef struct ppp_secret
{
   uint16_t     PPP_ID_LENGTH;
   uint16_t     PPP_PW_LENGTH;
   char         *PPP_ID_PTR;
   char         *PPP_PW_PTR;
} PPP_SECRET, * PPP_SECRET_PTR;

/* Start CR 2207 */
#if PPP_SECRETS_NOT_SHARED
#define PPP_SECRET(p,s) p->s
#else
#define PPP_SECRET(p,s) s
/* End CR 2207 */

#ifdef __cplusplus
extern "C" {
#endif

/* The secret to use if the peer requests PAP authentication */
extern PPP_SECRET_PTR   _PPP_PAP_LSECRET;

/* The array of secrets to use to authenticate the peer with PAP */
extern PPP_SECRET_PTR   _PPP_PAP_RSECRETS;

/* The name of the local host (required for CHAP) */
extern char   *_PPP_CHAP_LNAME;

/* The array of secrets to use if the peer requests CHAP authentication */
extern PPP_SECRET_PTR   _PPP_CHAP_LSECRETS;

/* The array of secrets to use to authenticate the peer with CHAP */
extern PPP_SECRET_PTR   _PPP_CHAP_RSECRETS;
/* Start CR 2207 */
#endif
/* End CR 2207 */

/*
** Each PPP channel requires two tasks.  The Tx task is a mindless drone
** that sends out everything we tell it to.  The Rx task contains the
** LCP automaton.
*/

extern void PPP_rx_task (void *, void *);
extern void PPP_tx_task (void *, void *);

/*
** PPP initialization structure
*/
typedef struct ppp_param_struct
{
    char*                  device;          /* Low-level communication device name */
    void (_CODE_PTR_ up)   (void *);        /* Function to be called when link goes up */
    void (_CODE_PTR_ down) (void *);        /* Function to be called when link goes down */
    void                  *callback_param;  /* Parameter for UP/DOWN callbacks */
    _rtcs_if_handle        if_handle;       /* Handle to ipcp interface */
    _ip_address            local_addr;      /* Local IP address */
    _ip_address            remote_addr;     /* IP addres of remote peer */
    int                    listen_flag;     /* Flag for determining if PPP should listen (true) or connect (false) */
}PPP_PARAM_STRUCT;

/***************************************
**
** The PPP interface functions
*/

extern _ppp_handle PPP_init(PPP_PARAM_STRUCT* params);
extern uint32_t PPP_release(_ppp_handle);
extern uint32_t PPP_pause(_ppp_handle);
extern uint32_t PPP_resume(_ppp_handle);
extern uint32_t PPP_check_connection(_ppp_handle handle);

extern uint32_t PPP_getcall(_ppp_handle, uint32_t, void (_CODE_PTR_ *)(), void **);
extern uint32_t PPP_setcall(_ppp_handle, uint32_t, void (_CODE_PTR_ *)(), void **);
extern uint32_t PPP_register(_ppp_handle, uint16_t, void (_CODE_PTR_)(PCB_PTR, void *), void *);
extern uint32_t PPP_unregister(_ppp_handle, uint16_t);
extern void     PPP_lowerup(_ppp_handle);
extern void     PPP_lowerdown(_ppp_handle);
extern uint32_t PPP_open(_ppp_handle, uint32_t);
extern uint32_t PPP_close(_ppp_handle);
extern uint32_t PPP_send(_ppp_handle, uint16_t, PCB_PTR);
extern uint32_t PPP_getmtu(_ppp_handle);
extern bool     PPP_set_secrets(_ppp_handle, uint32_t, PPP_SECRET_PTR);
extern bool     PPP_set_chap_lname(_ppp_handle, char*);

#ifdef __cplusplus
}
#endif

#endif
#endif
