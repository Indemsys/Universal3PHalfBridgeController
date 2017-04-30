/*HEADER**********************************************************************
*
* Copyright 2008-2010 Freescale Semiconductor, Inc.
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
*   This file contains the type definitions for the kxx FlexCAN module.
*
*
*END************************************************************************/

#include <psp.h>

#ifndef __kxx_fcan_h__
#define __kxx_fcan_h__

#define __kxx_fcan_h__version "$Version:3.6.1.0$"
#define __kxx_fcan_h__date    "$Date:Nov-2-2010$"

#ifdef __cplusplus
extern "C" {
#endif

/*
** Message buffer codes for Rx Buffers
*/
#define CAN_RX_MSG_BUFFER_NOT_ACTIVE   (0x0)
#define CAN_RX_MSG_BUFFER_EMPTY        (0x4)
#define CAN_RX_MSG_BUFFER_FULL         (0x2)
#define CAN_RX_MSG_BUFFER_OVERRUN      (0x6)
#define CAN_RX_MSG_BUFFER_BUSY         (0x1)

/*
** Message buffer codes for Rx Buffers
*/
#define CAN_TX_MSG_BUFFER_NOT_ACTIVE       (0x8)
#define CAN_MESSAGE_TRANSMIT_ONCE          (0xC)
#define CAN_MESSAGE_TRANSMIT_REMOTE        (0xC)
#define CAN_MESSAGE_TRANSMIT_RESPONED      (0xA)
#define CAN_MESSAGE_TRANSMIT_RESPONED_ONLY (0xE)


/* Message buffer memory map */
#define FLEXCAN_MSGCTRL_RTR                     (CAN_CS_RTR_MASK)
#define FLEXCAN_MSGCTRL_IDE                     (CAN_CS_IDE_MASK)
#define FLEXCAN_MSGCTRL_SRR                     (CAN_CS_SRR_MASK)
#define FLEXCAN_MSGCRTL_CODE                    (CAN_CS_CODE_MASK)
#define FLEXCAN_MSG_CTRL_DLEN                   (CAN_CS_DLC_MASK)
#define FLEXCAN_MSG_DISABLE                     (~ FLEXCAN_MSGCRTL_CODE)

/*
** Message Buffer Codes for Rx Buffers
*/
#define FLEXCAN_RX_MSG_BUFFER_NOT_ACTIVE        CAN_CS_CODE(CAN_RX_MSG_BUFFER_NOT_ACTIVE)
#define FLEXCAN_RX_MSG_BUFFER_EMPTY             CAN_CS_CODE(CAN_RX_MSG_BUFFER_EMPTY)
#define FLEXCAN_RX_MSG_BUFFER_FULL              CAN_CS_CODE(CAN_RX_MSG_BUFFER_FULL)
#define FLEXCAN_RX_MSG_BUFFER_OVERRUN           CAN_CS_CODE(CAN_RX_MSG_BUFFER_OVERRUN)
#define FLEXCAN_RX_MSG_BUFFER_BUSY              CAN_CS_CODE(CAN_RX_MSG_BUFFER_BUSY)

/* Message Buffer Codes for Tx Buffers */
#define FLEXCAN_TX_MSG_BUFFER_NOT_ACTIVE        CAN_CS_CODE(CAN_TX_MSG_BUFFER_NOT_ACTIVE)
#define FLEXCAN_MESSAGE_TRANSMIT_ONCE           CAN_CS_CODE(CAN_MESSAGE_TRANSMIT_ONCE)
#define FLEXCAN_MESSAGE_TRANSMIT_REMOTE         CAN_CS_CODE(CAN_MESSAGE_TRANSMIT_REMOTE)
#define FLEXCAN_MESSAGE_TRANSMIT_RESPONED       CAN_CS_CODE(CAN_MESSAGE_TRANSMIT_RESPONED)
#define FLEXCAN_MESSAGE_TRANSMIT_RESPONED_ONLY  CAN_CS_CODE(CAN_MESSAGE_TRANSMIT_RESPONED_ONLY)

/* Interrupt masks */
#define FLEXCAN_WAKEUP_INT              CAN_ESR1_WAKINT_MASK
#define FLEXCAN_ERROR_INT               CAN_ESR1_ERRINT_MASK
#define FLEXCAN_BUSOFF_INT              CAN_ESR1_BOFFINT_MASK
#define FLEXCAN_ALL_INT                 (CAN_ESR1_WAKINT_MASK | CAN_ESR1_ERRINT_MASK | CAN_ESR1_BOFFINT_MASK)
#define FLEXCAN_TX_RX_INT               (~ FLEXCAN_ALL_INT)
#define FLEXCAN_IMASK_VALUE             CAN_IMASK2_BUFHM_MASK

/*------------------------------------------------------------------------*/
/*
** FCAN registers bit set GENERALIZED
*/

#define FLEXCAN_CANMCR_MAXMB                  CAN_MCR_MAXMB
#define FLEXCAN_CANMCR_FRZACK                 CAN_MCR_FRZACK_MASK
#define FLEXCAN_CANMCR_SOFTRST                CAN_MCR_SOFTRST_MASK
#define FLEXCAN_CANMCR_HALT                   CAN_MCR_HALT_MASK
#define FLEXCAN_CANMCR_FRZ                    CAN_MCR_FRZ_MASK
#define FLEXCAN_CANMCR_MDIS                   CAN_MCR_MDIS_MASK
#define FLEXCAN_CANMCR_LPMACK                 CAN_MCR_LPMACK_MASK
#define FLEXCAN_CANMCR_NOTRDY                 CAN_MCR_NOTRDY_MASK

#define FLEXCAN_CANCTRL_LOM                   CAN_CTRL1_LOM_MASK
#define FLEXCAN_CANCTRL_TSYNC                 CAN_CTRL1_TSYN_MASK
#define FLEXCAN_CANCTRL_LPB                   CAN_CTRL1_LPB_MASK
#define FLEXCAN_CANCTRL_BOFFREC               CAN_CTRL1_BOFFREC_MASK
#define FLEXCAN_CANCTRL_SAMP                  CAN_CTRL1_SMP_MASK
#define FLEXCAN_CANCTRL_CLK_SRC               CAN_CTRL1_CLKSRC_MASK
#define FLEXCAN_CANCTRL_ERRMSK                CAN_CTRL1_ERRMSK_MASK
#define FLEXCAN_CANCTRL_BOFFMSK               CAN_CTRL1_BOFFMSK_MASK

#define FLEXCAN_CANCTRL_PROPSEG               CAN_CTRL1_PROPSEG
#define FLEXCAN_CANCTRL_PSEG2                 CAN_CTRL1_PSEG2
#define FLEXCAN_CANCTRL_PSEG1                 CAN_CTRL1_PSEG1
#define FLEXCAN_CANCTRL_RJW                   CAN_CTRL1_RJW
#define FLEXCAN_CANCTRL_PRESDIV               CAN_CTRL1_PRESDIV

#define FLEXCAN_STANDARD_ID_MASK              CAN_ID_STD
#define FLEXCAN_EXTENDED_ID_MASK              CAN_ID_EXT

#define _flexcan_int_init                     _bsp_int_init

#ifdef __cplusplus
}
#endif

#endif  //__kxx_fcan_h__
