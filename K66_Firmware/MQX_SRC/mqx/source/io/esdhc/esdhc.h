#ifndef __esdhc_h__
#define __esdhc_h__
/*HEADER**********************************************************************
*
* Copyright 2009 Freescale Semiconductor, Inc.
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
*   This file contains the definitions of constants and structures
*   required for the eSDHC driver
*
*
*END************************************************************************/


/*--------------------------------------------------------------------------*/
/*
**                    CONSTANT DEFINITIONS
*/

/*
** IOCTL calls specific to eSDHC
*/
#define IO_IOCTL_ESDHC_INIT                  _IO(IO_TYPE_ESDHC,0x01)
#define IO_IOCTL_ESDHC_SEND_COMMAND          _IO(IO_TYPE_ESDHC,0x02)
#define IO_IOCTL_ESDHC_GET_CARD              _IO(IO_TYPE_ESDHC,0x03)
#define IO_IOCTL_ESDHC_GET_BAUDRATE          _IO(IO_TYPE_ESDHC,0x04)
#define IO_IOCTL_ESDHC_SET_BAUDRATE          _IO(IO_TYPE_ESDHC,0x05)
#define IO_IOCTL_ESDHC_GET_BUS_WIDTH         _IO(IO_TYPE_ESDHC,0x06)
#define IO_IOCTL_ESDHC_SET_BUS_WIDTH         _IO(IO_TYPE_ESDHC,0x07)
#define IO_IOCTL_ESDHC_GET_BAUDRATE_MAX      _IO(IO_TYPE_ESDHC,0x08)
#define IO_IOCTL_ESDHC_SET_IO_CALLBACK       _IO(IO_TYPE_ESDHC,0x09)
#define IO_IOCTL_ESDHC_SET_CARD_CALLBACK     _IO(IO_TYPE_ESDHC,0x0A)
#define IO_IOCTL_ESDHC_GET_CARD_PRESENCE     _IO(IO_TYPE_ESDHC,0x0B)


/* ESDHC error codes */
#define ESDHC_OK                             (0x00)
#define ESDHC_ERROR_INIT_FAILED              (ESDHC_ERROR_BASE | 0x01)
#define ESDHC_ERROR_COMMAND_FAILED           (ESDHC_ERROR_BASE | 0x02)
#define ESDHC_ERROR_COMMAND_TIMEOUT          (ESDHC_ERROR_BASE | 0x03)
#define ESDHC_ERROR_DATA_TRANSFER            (ESDHC_ERROR_BASE | 0x04)
#define ESDHC_ERROR_INVALID_BUS_WIDTH        (ESDHC_ERROR_BASE | 0x05)


/* ESDHC bus widths */
#define ESDHC_BUS_WIDTH_1BIT                 (0x00)
#define ESDHC_BUS_WIDTH_4BIT                 (0x01)
#define ESDHC_BUS_WIDTH_8BIT                 (0x02)


/* ESDHC card types */
#define ESDHC_CARD_NONE                      (0x00)
#define ESDHC_CARD_UNKNOWN                   (0x01)
#define ESDHC_CARD_SD                        (0x02)
#define ESDHC_CARD_SDHC                      (0x03)
#define ESDHC_CARD_SDIO                      (0x04)
#define ESDHC_CARD_SDCOMBO                   (0x05)
#define ESDHC_CARD_SDHCCOMBO                 (0x06)
#define ESDHC_CARD_MMC                       (0x07)
#define ESDHC_CARD_CEATA                     (0x08)


/* ESDHC standard baud rates */
#define ESDHC_DEFAULT_BAUDRATE               (25000000)
#define ESDHC_INIT_BAUDRATE                  (400000)


/*--------------------------------------------------------------------------*/
/*
**                    DATATYPE DECLARATIONS
*/

typedef SDHC_MemMapPtr  ESDHC_REG_STRUCT_PTR;

/*
** ESDHC_IO_INT_CALLBACK
**
** This callback function is used to notify that IO card provide interrupt
*/
typedef void (_CODE_PTR_ ESDHC_IO_INT_CALLBACK)(void *context_data);

/*
** ESDHC_IO_INT_CALLBACK_STRUCT
**
** This structure defines the parameters of the IO card interrupt callback
** when passed to IO_IOCTL_ESDHC_SET_IO_CALLBACK.
*/

typedef struct esdhc_io_int_callback_struct
{
   /* The ESDHC callback itself */
   ESDHC_IO_INT_CALLBACK  CALLBACK;

   /* User data */
   void            *USERDATA;

} ESDHC_IO_INT_CALLBACK_STRUCT, * ESDHC_IO_INT_CALLBACK_STRUCT_PTR;

/*
** ESDHC_CARD_PRESENCE_CALLBACK
**
** This callback function is used to notify that card presence changed
*/
typedef void (_CODE_PTR_ ESDHC_CARD_PRESENCE_CALLBACK)(void *context_data, bool presence);

/*
** ESDHC_CARD_PRESENCE_CALLBACK_STRUCT
**
** This structure defines the parameters of the card presence change callback
** when passed to IO_IOCTL_ESDHC_SET_CARD_CALLBACK.
*/

typedef struct esdhc_card_presence_callback_struct
{
   /* The ESDHC callback itself */
   ESDHC_CARD_PRESENCE_CALLBACK  CALLBACK;

   /* User data */
   void            *USERDATA;

} ESDHC_CARD_PRESENCE_CALLBACK_STRUCT, * ESDHC_CARD_PRESENCE_CALLBACK_STRUCT_PTR;

#define ESDHC_COMMAND_CMDIX_MASK        0x0000003F
#define ESDHC_COMMAND_CMDTYPE_MASK      0x00000F00
#define ESDHC_COMMAND_CMDRESPONSE_MASK  0x00FFF000
#define ESDHC_COMMAND_FLAGS_MASK        0xFF000000

#define ESDHC_COMMAND_CMDIX_SHIFT       0
#define ESDHC_COMMAND_CMDTYPE_SHIFT     8
#define ESDHC_COMMAND_CMDRESPONSE_SHIFT 12
#define ESDHC_COMMAND_FLAGS_SHIFT       24

#define ESDHC_COMMAND_TYPE_NORMAL       (((0x0) << ESDHC_COMMAND_CMDTYPE_SHIFT) & ESDHC_COMMAND_CMDTYPE_MASK)
#define ESDHC_COMMAND_TYPE_SUSPEND      (((0x1) << ESDHC_COMMAND_CMDTYPE_SHIFT) & ESDHC_COMMAND_CMDTYPE_MASK)
#define ESDHC_COMMAND_TYPE_RESUME       (((0x2) << ESDHC_COMMAND_CMDTYPE_SHIFT) & ESDHC_COMMAND_CMDTYPE_MASK)
#define ESDHC_COMMAND_TYPE_ABORT        (((0x3) << ESDHC_COMMAND_CMDTYPE_SHIFT) & ESDHC_COMMAND_CMDTYPE_MASK)

#define ESDHC_COMMAND_RESPONSE_NO       (((0x000) << ESDHC_COMMAND_CMDRESPONSE_SHIFT) & ESDHC_COMMAND_CMDRESPONSE_MASK)
#define ESDHC_COMMAND_RESPONSE_R1       (((0x001) << ESDHC_COMMAND_CMDRESPONSE_SHIFT) & ESDHC_COMMAND_CMDRESPONSE_MASK)
#define ESDHC_COMMAND_RESPONSE_R1b      (((0x002) << ESDHC_COMMAND_CMDRESPONSE_SHIFT) & ESDHC_COMMAND_CMDRESPONSE_MASK)
#define ESDHC_COMMAND_RESPONSE_R2       (((0x004) << ESDHC_COMMAND_CMDRESPONSE_SHIFT) & ESDHC_COMMAND_CMDRESPONSE_MASK)
#define ESDHC_COMMAND_RESPONSE_R3       (((0x008) << ESDHC_COMMAND_CMDRESPONSE_SHIFT) & ESDHC_COMMAND_CMDRESPONSE_MASK)
#define ESDHC_COMMAND_RESPONSE_R4       (((0x010) << ESDHC_COMMAND_CMDRESPONSE_SHIFT) & ESDHC_COMMAND_CMDRESPONSE_MASK)
#define ESDHC_COMMAND_RESPONSE_R5       (((0x020) << ESDHC_COMMAND_CMDRESPONSE_SHIFT) & ESDHC_COMMAND_CMDRESPONSE_MASK)
#define ESDHC_COMMAND_RESPONSE_R5b      (((0x040) << ESDHC_COMMAND_CMDRESPONSE_SHIFT) & ESDHC_COMMAND_CMDRESPONSE_MASK)
#define ESDHC_COMMAND_RESPONSE_R6       (((0x080) << ESDHC_COMMAND_CMDRESPONSE_SHIFT) & ESDHC_COMMAND_CMDRESPONSE_MASK)
#define ESDHC_COMMAND_RESPONSE_R7       (((0x100) << ESDHC_COMMAND_CMDRESPONSE_SHIFT) & ESDHC_COMMAND_CMDRESPONSE_MASK)

#define ESDHC_COMMAND_NONE_FLAG         (((0x000) << ESDHC_COMMAND_FLAGS_SHIFT) & ESDHC_COMMAND_FLAGS_MASK)
#define ESDHC_COMMAND_ACMD_FLAG         (((0x001) << ESDHC_COMMAND_FLAGS_SHIFT) & ESDHC_COMMAND_FLAGS_MASK)
#define ESDHC_COMMAND_DATACMD_FLAG      (((0x002) << ESDHC_COMMAND_FLAGS_SHIFT) & ESDHC_COMMAND_FLAGS_MASK)
#define ESDHC_COMMAND_DATA_READ_FLAG    (((0x004) << ESDHC_COMMAND_FLAGS_SHIFT) & ESDHC_COMMAND_FLAGS_MASK)


#define ESDHC_CREATE_CMD(cmdIx, cmdType, cmdResponse, cmdFlags) ((cmdFlags) | (cmdResponse) | (cmdType) | ((cmdIx) & ESDHC_COMMAND_CMDIX_MASK))


typedef struct esdhc_command_struct
{
    uint32_t COMMAND;
    uint32_t ARGUMENT;
    uint32_t BLOCKS;
    uint32_t BLOCKSIZE;
    uint32_t RESPONSE[4];
} ESDHC_COMMAND_STRUCT, * ESDHC_COMMAND_STRUCT_PTR;


/*
** ESDHC_INIT_STRUCT
**
** This structure defines the initialization parameters to be used
** when a esdhc driver is initialized.
*/
typedef struct esdhc_init_struct
{
    /* The device number */
    uint32_t CHANNEL;

    /* The communication board allowed maximal baud rate */
    uint32_t MAX_BAUD_RATE;

    /* The module input clock */
    uint32_t CLOCK_SPEED;

} ESDHC_INIT_STRUCT, * ESDHC_INIT_STRUCT_PTR;

typedef const ESDHC_INIT_STRUCT * ESDHC_INIT_STRUCT_CPTR;


/*--------------------------------------------------------------------------*/
/*
**                        FUNCTION PROTOTYPES
*/

#ifdef __cplusplus
extern "C" {
#endif


extern void   *_bsp_get_esdhc_base_address(uint8_t);
extern uint32_t _bsp_get_esdhc_vector(uint8_t);
extern _mqx_int _esdhc_install(char *,ESDHC_INIT_STRUCT_CPTR);


#ifdef __cplusplus
}
#endif


#endif

/* EOF */
