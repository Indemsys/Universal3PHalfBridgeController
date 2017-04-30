#ifndef __ccp_h__
#define __ccp_h__
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
*   definitions required by CCP.
*
*
*END************************************************************************/

/*
** CCP-specific packet types
*/

#define CCP_CODE_RESET_REQ 14      /* Reset Request */
#define CCP_CODE_RESET_ACK 15      /* Reset Ack */


/*
** CCP Configuration Options
*/

#define CCP_CI_PRED1    1     /* Predictor type 1 */
#define CCP_CI_LZS      17    /* Stac LZS */


struct ccp_opt;
struct ppp_opt;
struct ppp_cfg;

/*
** The Compression Protocol Data Union.  This union contains the private
** data required by each supported compression method.
*/

typedef union cp_data {
   uint32_t     DUMMY;
} CP_DATA, * CP_DATA_PTR;

/*
** The Compression Procotol Call Table structure.  This structure
** contains pointers to functions that compress and decompress packets.
*/

typedef struct cp_call {
   void    (_CODE_PTR_ CP_init)  (CP_DATA_PTR, struct ccp_opt *);
   PCB_PTR (_CODE_PTR_ CP_comp)  (CP_DATA_PTR, PCB_PTR,
                                  struct ppp_cfg *, struct ppp_opt *);
   PCB_PTR (_CODE_PTR_ CP_decomp)(CP_DATA_PTR, PCB_PTR,
                                  struct ppp_cfg *, struct ppp_opt *);

   void    (_CODE_PTR_ CP_resetreq) (CP_DATA_PTR, PCB_PTR);
   void    (_CODE_PTR_ CP_resetack) (CP_DATA_PTR, PCB_PTR);
} CP_CALL, * CP_CALL_PTR;

/*
** The CCP Options structure.  This structure contains all the
** negotiated options for an open CCP link.
*/

typedef struct ccp_opt {
   CP_CALL_PTR    CP;
   union {
      uint32_t     DUMMY;
#ifdef PPP_CP_LZS
      struct {
         uint16_t  HIST;
         unsigned char    CHECK;
      }           LZS;
#endif
   }              DATA;
} CCP_OPT, * CCP_OPT_PTR;

/*
** The CCP Options state used during negotiation
*/

typedef struct ccp_neg {
   unsigned NEG_PRED1 : 1;    /* Not used */
   unsigned NEG_LZS   : 1;    /* Not used */
   unsigned           : 0;
#ifdef PPP_CP_LZS
   uint16_t  LZS_HIST;
   unsigned char    LZS_CHECK;
#endif
} CCP_NEG, * CCP_NEG_PTR;

/*
** The CCP Configuration structure
*/

typedef struct ccp_cfg {

   /* Statistics counters */
   uint32_t  ST_CCP_RX_RESET;
   uint32_t  ST_CCP_RX_ACK;

   CCP_OPT  SEND_OPT;
   CCP_OPT  RECV_OPT;
   CCP_NEG  RECV_NEG;
   CP_DATA  SEND_DATA;
   CP_DATA  RECV_DATA;
} CCP_CFG, * CCP_CFG_PTR;


/*
** Prototypes
*/
#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t CCP_init    (_ppp_handle);
extern uint32_t CCP_destroy (_ppp_handle);

#define CCP_lowerup(handle)   PPPFSM_lowerup(&((PPP_CFG_PTR)(handle))->CCP_FSM)
#define CCP_lowerdown(handle) PPPFSM_lowerdown(&((PPP_CFG_PTR)(handle))->CCP_FSM)
#define CCP_open(handle)      PPPFSM_open(&((PPP_CFG_PTR)(handle))->CCP_FSM, 0)
#define CCP_close(handle)     PPPFSM_close(&((PPP_CFG_PTR)(handle))->CCP_FSM)
#define CCP_input             PPPFSM_input

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
