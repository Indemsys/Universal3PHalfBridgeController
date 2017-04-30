#ifndef __chap_h__
#define __chap_h__
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
*   definitions required by CHAP.
*
*
*END************************************************************************/


/*
** CHAP header = Code, id, length.
*/

#define CHAP_HDR_LEN    4

/*
** The length of generated challenges
*/

#define CHAP_CHALLENGE_LEN    64

/*
** CHAP packet types
*/

#define CHAP_CODE_CHALLENGE   1     /* Challenge */
#define CHAP_CODE_RESPONSE    2     /* Response to challenge */
#define CHAP_CODE_SUCCESS     3     /* Successful response */
#define CHAP_CODE_FAILURE     4     /* Failed response */

/*
** Internal CHAP states
*/

#define CHAP_STATE_CLOSED     0
#define CHAP_STATE_INITIAL    1
#define CHAP_STATE_SUCCESS    CHAP_CODE_SUCCESS
#define CHAP_STATE_FAILURE    CHAP_CODE_FAILURE

/*
** Private state required by the CHAP server
*/

typedef struct chap_data {

   /* Statistics counters */
   uint32_t  ST_CHAP_SHORT;    /* packet too short */
   uint32_t  ST_CHAP_CODE;     /* invalid code */
   uint32_t  ST_CHAP_ID;       /* incorrect id */
   uint32_t  ST_CHAP_NOAUTH;   /* challenge received but CHAP not negotiated */
   uint32_t  ST_CHAP_NOCHAL;   /* response received but no challenge made */
   uint32_t  ST_CHAP_NOPW;     /* no password for peer */

   unsigned char MD5[64];

   unsigned char CLIENT_STATE;
   unsigned char SERVER_STATE;
   unsigned char CURID;

} CHAP_DATA, * CHAP_DATA_PTR;


/*
** Prototypes
*/

#ifdef __cplusplus
extern "C" {
#endif

extern void CHAP_init      (_ppp_handle);
extern void CHAP_challenge (_ppp_handle);
extern void CHAP_open      (_ppp_handle);

extern void CHAP_input (PCB_PTR, _ppp_handle);


/*
** Available hashing algorithms
*/

extern void PPP_MD5(unsigned char *, ...);

#ifdef __cplusplus
}
#endif


#endif
/* EOF */
