#ifndef __pap_h__
#define __pap_h__
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
*   This file contains the defines and data structure
*   definitions required by PAP.
*
*
*END************************************************************************/


/*
** PAP header = Code, id, length.
*/

#define PAP_HDR_LEN   (4)

/*
** PAP packet types
*/

#define PAP_CODE_AUTH_REQ  1     /* Authenticate Request */
#define PAP_CODE_AUTH_ACK  2     /* Authenticate Ack */
#define PAP_CODE_AUTH_NAK  3     /* Authenticate Nak */

/*
** Internal PAP states
*/

#define PAP_STATE_CLOSED   0
#define PAP_STATE_INITIAL  1
#define PAP_STATE_AUTH_ACK PAP_CODE_AUTH_ACK
#define PAP_STATE_AUTH_NAK PAP_CODE_AUTH_NAK

/*
** Private state required by the PAP server
*/

typedef struct pap_data {

   /* Statistics counters */
   uint32_t  ST_PAP_SHORT;     /* packet too short */
   uint32_t  ST_PAP_CODE;      /* invalid code */
   uint32_t  ST_PAP_ID;        /* incorrect id */
   uint32_t  ST_PAP_NOAUTH;    /* request received but PAP not negotiated */
   uint32_t  ST_PAP_NOREQ;     /* reply received but no request made */

   unsigned char CLIENT_STATE;
   unsigned char SERVER_STATE;
   unsigned char CURID;
} PAP_DATA, * PAP_DATA_PTR;


/*
** Prototypes
*/

#ifdef __cplusplus
extern "C" {
#endif

extern void PAP_init  (_ppp_handle);
extern void PAP_send  (_ppp_handle);
extern void PAP_open  (_ppp_handle);

extern void PAP_input (PCB_PTR, _ppp_handle);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
