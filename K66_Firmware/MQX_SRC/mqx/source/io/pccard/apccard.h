#ifndef _apccard_h_
#define _apccard_h_
/*HEADER**********************************************************************
*
* Copyright 2011 Freescale Semiconductor, Inc.
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
*   The file contains the structure definitions
*   public to the PC Card drivers
*
*
*END************************************************************************/

#include "ioctl.h"

/*----------------------------------------------------------------------*/
/*
**                          CONSTANT DEFINITIONS
*/

/* Error codes */
#define APCCARD_OK                   0
#define APCCARD_INVALID_HANDLE       1
#define APCCARD_INVALID_SLOT         2
#define APCCARD_INVALID_VOLTAGE      3

/* Slot definitions */
#define APCCARD_SLOT_A         0x100
#define APCCARD_SLOT_B         0x101

/* Voltage levels */
#define APCCARD_VOLTAGE_HI_Z   0x1000
#define APCCARD_VOLTAGE_12V    0x1001
#define APCCARD_VOLTAGE_5V     0x1002
#define APCCARD_VOLTAGE_33V    0x1003
#define APCCARD_VOLTAGE_0V     0x1004

/* Address space specifiers */
#define APCCARD_ATTRIB_SPACE   0x200
#define APCCARD_COMMON_SPACE   0x201
#define APCCARD_IO_SPACE       0x202

/* IOCTL Commands */
#define APCCARD_IOCTL_IS_CARD_INSERTED          _IO(IO_TYPE_APCCARD,0x01)
#define APCCARD_IOCTL_SET_CRD_INSERT_CALLBACK   _IO(IO_TYPE_APCCARD,0x02)
#define APCCARD_IOCTL_SET_CRD_REMOVE_CALLBACK   _IO(IO_TYPE_APCCARD,0x03)
#define APCCARD_IOCTL_GET_ADDR_SPACE            _IO(IO_TYPE_APCCARD,0x06)
#define APCCARD_IOCTL_FREE_ADDR_SPACE           _IO(IO_TYPE_APCCARD,0x07)
#define APCCARD_IOCTL_SENSE_VOLTAGE             _IO(IO_TYPE_APCCARD,0x08)
#define APCCARD_IOCTL_VCC_ENABLE                _IO(IO_TYPE_APCCARD,0x09)
#define APCCARD_IOCTL_VPP_ENABLE                _IO(IO_TYPE_APCCARD,0x0A)
#define APCCARD_IOCTL_RESET                     _IO(IO_TYPE_APCCARD,0x0B)
#define APCCARD_IOCTL_VCC_DISABLE               _IO(IO_TYPE_APCCARD,0x0C)
#define APCCARD_IOCTL_VPP_DISABLE               _IO(IO_TYPE_APCCARD,0x0D)
#define APCCARD_IOCTL_IDENTIFY                  _IO(IO_TYPE_APCCARD,0x0E)
#define APCCARD_IOCTL_POWERUP_CARD              _IO(IO_TYPE_APCCARD,0x0F)
#define APCCARD_IOCTL_POWERDOWN_CARD            _IO(IO_TYPE_APCCARD,0x10)
#define APCCARD_IOCTL_WAIT_TILL_READY           _IO(IO_TYPE_APCCARD,0x11)
#define APCCARD_IOCTL_CARD_READY                _IO(IO_TYPE_APCCARD,0x12)
#define APCCARD_IOCTL_READ_TUPLE                _IO(IO_TYPE_APCCARD,0x13)

/* CIS offsets */
#define IO_APCCARD_TUPLE_CODE_CIS_OFFSET        0x00
#define IO_APCCARD_TUPLE_LINK_CIS_OFFSET        0x02
#define IO_APCCARD_TUPLE_DATA_CIS_OFFSET        0x04

/* Some tuple codes */
#define IO_APCCARD_TUPLE_CISTPL_NULL            0x00
#define IO_APCCARD_TUPLE_CISTPL_DEVICE          0x01
#define IO_APCCARD_TUPLE_CISTPL_FUNCID          0x21
#define IO_APCCARD_TUPLE_CISTPL_FUNCE           0x22
#define IO_APCCARD_TUPLE_CISTPL_CFTABLE_ENTRY   0x1B
#define IO_APCCARD_TUPLE_END_OF_CIS             0xFF

/*----------------------------------------------------------------------*/
/*
**                    DATATYPE DEFINITIONS
*/

/* This structure is used in the ioctl command APCCARD_IOCTL_GET_ADDR_SPACE */
typedef struct apccard_addr_info_struct
{
   uint32_t OFFSET;      /* Offset from PCMCIA base               */
   uint32_t TYPE;        /* Address type: Attrib, common, io      */
   uint32_t BANK_SIZE;   /* bank size from 1 to 64 megabytes      */
   uint32_t PORT_SIZE;   /* Port size: 8 or 16                    */
   void   *ADDRESS;     /* Filled in by driver. Absolute address */
} APCCARD_ADDR_INFO_STRUCT, * APCCARD_ADDR_INFO_STRUCT_PTR;


/*-----------------------------------------------------------------------
**                      FUNCTION PROTOTYPES
*/
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
