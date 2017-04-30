#ifndef __tftp_prv_h__
#define __tftp_prv_h__
/*HEADER**********************************************************************
*
* Copyright 2014 Freescale Semiconductor, Inc.
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
*   This file contains private definitions needed by TFTP.
*
*
*END************************************************************************/

#include <rtcs.h>

#if !MQX_USE_IO_OLD
#include <stdio.h>
#define MQX_FILE_PTR FILE *
#define IO_SEEK_SET  SEEK_SET
#endif

#define TFTP_OPCODE_RRQ   (1)
#define TFTP_OPCODE_WRQ   (2)
#define TFTP_OPCODE_DATA  (3)
#define TFTP_OPCODE_ACK   (4)
#define TFTP_OPCODE_ERROR (5)

#define TFTP_ERR_UNKNOWN            (0)
#define TFTP_ERR_FILE_NOT_FOUND     (-1)
#define TFTP_ERR_ACCESS_VIOLATION   (-2)
#define TFTP_ERR_DISK_FULL          (-3)
#define TFTP_ERR_ILLEGAL_OP         (-4)
#define TFTP_ERR_ILLEGAL_TID        (-5)
#define TFTP_ERR_FILE_EXISTS        (-6)
#define TFTP_ERR_ILLEGAL_USER       (-7)
#define TFTP_ERR_NO_FILE            (-8)
#define TFTP_ERR_TIMEOUT            (-9)

#define TFTP_DATA_SIZE          (512UL)
#define TFTP_MAX_MESSAGE_SIZE   (576UL)
#define TFTP_HEADER_SIZE        (4)

#define TFTP_ERR_BAD_TID_STRING         "Unknown transfer ID."
#define TFTP_ERR_ILLEGAL_OP_STRING      "Illegal Operation."
#define TFTP_ERR_ACCESS_STRING          "Access Violation."
#define TFTP_ERR_SERVER_STRING          "TFTP Local Failure."
#define TFTP_ERR_BUSY_STRING            "TFTP Busy."
#define TFTP_ERR_RETRANSMISSION_STRING  "Excessive Retransmissions."
#define TFTP_ERR_NOT_FOUND_STRING       "File Not Found."
#define TFTP_ERR_EXISTS_STRING          "File Already Exists."
#define TFTP_ERR_DISK_FULL_STRING       "Disk full or allocation exceeded."
#define TFTP_ERR_NO_USER_STRING         "No such user."
#define TFTP_ERR_TIMEOUT_STRING         "Transfer timeout."

#define TFTP_RD_MAX   (30000)
#define TFTP_RC_MAX   (10)
#define TFTP_RC_INIT  (1000)

#define TFTP_MODE_OCTET          "octet"
#define TFTP_MODE_OCTET_SIZE     sizeof("octet")
#define TFTP_MODE_NETASCII       "netascii"
#define TFTP_MODE_NETASCII_SIZE  sizeof("netascii")

#define TFTP_FILENAME_MAX (128)

#define TFTP_FLAG_SAVE_REMOTE  (1 << 0)
#define TFTP_FLAG_RECV_ERROR   (1 << 1)

/* TFTP Structure definitions */
typedef struct tftp_header {
   unsigned char       OP[2];
   unsigned char       BLOCK[2];
} TFTP_HEADER, * TFTP_HEADER_PTR;

/*
 * TFTP transaction state
 */
typedef enum tftp_transaction_state
{
    TFTP_STATE_SEND_DATA,
    TFTP_STATE_SEND_ACK,
    TFTP_STATE_READ_DATA,
    TFTP_STATE_READ_ACK
}TFTP_TRANSACTION_STATE;

typedef struct tftp_transaction
{
    volatile uint32_t      sock;         /* Transaction socket */
    uint16_t               buffer_ack[2];/* Buffer for ACKs */
    char                   *buffer;      /* Pointer to transaction buffer */
    MQX_FILE_PTR           file;         /* File used in transaction */
    sockaddr               remote_sa;    /* Information about remote host. */
    uint16_t               n_block;      /* Serial number of transaction block. */
    TFTP_TRANSACTION_STATE state;        /* Session state. */
    bool                   last;         /* Flag signalizing last packet. */
    uint32_t               recv_timeout; /* Time for which server will wait for data. */
    uint32_t               rt_count;     /* number of retransmissions. */
    uint32_t               flags;        /* Transaction flags. */
    char                   *err_string;   /* String with error description. */
}TFTP_TRANSACTION;

#ifdef __cplusplus
extern "C" {
#endif

uint32_t tftp_rt_step(TFTP_TRANSACTION *trans);
void tftp_rt_reset(TFTP_TRANSACTION *trans);
void tftp_send_error(TFTP_TRANSACTION *trans, int32_t error_code);
int32_t tftp_send(uint32_t sock, char* data, uint32_t data_size, uint32_t flags);
int32_t tftp_send_ack(TFTP_TRANSACTION *trans);
int32_t tftp_send_data(TFTP_TRANSACTION *trans);
int32_t tftp_recv(TFTP_TRANSACTION *trans, char* data, uint32_t data_size);
int32_t tftp_recv_ack(TFTP_TRANSACTION *trans);
int32_t tftp_recv_data(TFTP_TRANSACTION *trans);
size_t tftp_fread(void * ptr, size_t size, size_t count, MQX_FILE_PTR stream);
size_t tftp_fwrite(void * ptr, size_t size, size_t count, MQX_FILE_PTR stream);
int tftp_fseek (MQX_FILE_PTR stream, long int offset);

#ifdef __cplusplus
}
#endif

#endif
