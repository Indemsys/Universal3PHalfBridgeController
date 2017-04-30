/*HEADER**********************************************************************
*
* Copyright 2010 Freescale Semiconductor, Inc.
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
*   This file contains the definitions of constants,IOCTL'S and structures
*   required for the PTP functionality.
*
*
*END************************************************************************/
#ifndef macnet_1588_h
#define macnet_1588_h 1

/* IOCTL calls */
#define MACNET_PTP_GET_TX_TIMESTAMP             _IO(IO_TYPE_MEDIACTL_PTP,0x01)
#define MACNET_PTP_GET_RX_TIMESTAMP             _IO(IO_TYPE_MEDIACTL_PTP,0x02)
#define MACNET_PTP_GET_CURRENT_TIME             _IO(IO_TYPE_MEDIACTL_PTP,0x03)
#define MACNET_PTP_SET_RTC_TIME                 _IO(IO_TYPE_MEDIACTL_PTP,0x04)
#define MACNET_PTP_SET_COMPENSATION             _IO(IO_TYPE_MEDIACTL_PTP,0x05)
#define MACNET_PTP_FLUSH_TIMESTAMP              _IO(IO_TYPE_MEDIACTL_PTP,0x06)
#define MACNET_PTP_GET_ORIG_COMP                _IO(IO_TYPE_MEDIACTL_PTP,0x07)
#define MACNET_PTP_REGISTER_ETHERTYPE_PTPV2     _IO(IO_TYPE_MEDIACTL_PTP,0x08)
#define MACNET_PTP_SEND_ETHERTYPE_PTPV2_PCK     _IO(IO_TYPE_MEDIACTL_PTP,0x09)
#define MACNET_PTP_RECV_ETHERTYPE_PTPV2_PCK     _IO(IO_TYPE_MEDIACTL_PTP,0x0A)
#define MACNET_PTP_IS_IN_INBAND_MODE            _IO(IO_TYPE_MEDIACTL_PTP,0x0B)
#define MACNET_PTP_UNREGISTER_ETHERTYPE_PTPV2   _IO(IO_TYPE_MEDIACTL_PTP,0x0C)

#define MACNET_PTP_RX_TIMESTAMP_SYNC            0x0
#define MACNET_PTP_RX_TIMESTAMP_DEL_REQ         0x1
#define MACNET_PTP_RX_TIMESTAMP_PDELAY_REQ      0x2
#define MACNET_PTP_RX_TIMESTAMP_PDELAY_RESP     0x3

#define MQX1588_PTP_ETHERTYPE_1588              0x88F7

#define MACNET_PTP_EVENT_MSG_FRAME_SIZE         0x56
#define MACNET_PTP_EVENT_MSG_PTP_SIZE           0x2C
#define MACNET_PTP_INBAND_SEC_OFFS              0x00
#define MACNET_PTP_INBAND_NANOSEC_OFFS          0x08

/* PTP message version */
#define MACNET_PTP_MSG_VER_1                    1
#define MACNET_PTP_MSG_VER_2                    2

/* PTP standard time representation structure */
typedef struct {
	uint64_t SEC;	/* seconds */
	uint32_t NSEC;	/* nanoseconds */
}MACNET_PTP_TIME;

/* interface for PTP driver command MACNET_PTP_GET_RX_TIMESTAMP/MACNET_PTP_GET_TX_TIMESTAMP */
typedef struct {
	/* PTP version */
	uint8_t VERSION;
	/* PTP source port ID */
	uint8_t SPID[10];
	/* PTP sequence ID */
	uint16_t SEQ_ID;
	/* PTP message type */
	uint8_t MESSAGE_TYPE;
	/* PTP timestamp */
	MACNET_PTP_TIME TS;
}MACNET_PTP_TS_DATA;

/* interface for PTP driver command MACNET_PTP_SET_RTC_TIME/MACNET_PTP_GET_CURRENT_TIME */
typedef struct {
	MACNET_PTP_TIME RTC_TIME;
}MACNET_PTP_RTC_TIME;

/* interface for PTP driver command MACNET_PTP_SET_COMPENSATION */
typedef struct {
	int32_t DRIFT;
}MACNET_PTP_SET_COMP;

/* interface for PTP driver command MACNET_PTP_GET_ORIG_COMP */
typedef struct {
	/* the initial compensation value */
	uint32_t DW_ORIGCOMP;
	/* the minimum compensation value */
	uint32_t DW_MINCOMP;
	/*the max compensation value*/
	uint32_t DW_MAXCOMP;
	/*the min drift applying min compensation value in ppm*/
	uint32_t DW_MINDRIFT;
	/*the max drift applying max compensation value in ppm*/
	uint32_t DW_MAXDRIFT;
}MACNET_PTP_GET_COMP;

/* interface for PTP driver command MACNET_PTP_SEND_ETHERTYPE_PTPV2_PCK */
typedef struct {
	/* pointer to the PTP message */
	uint8_t* PTP_MSG;
	/* size of the PTP message */
	uint16_t LENGTH;
	/* destination MAC address */
	_enet_address DEST_MAC;
#if RTCSCFG_ENABLE_8021Q 
    /* vlan frame enable flag*/
    bool vlanEnabled;
    /* vlan id*/
    uint16_t vlanId;
    /* vlan priority */
    uint8_t vlanPrior;
#endif
}MACNET_PTP_ETHERTYPE_PCK;


#endif
/* EOF */

