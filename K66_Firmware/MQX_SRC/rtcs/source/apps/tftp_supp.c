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
*   This file contains functions common for the TFTP server and client.
*
*END************************************************************************/

#include "tftp_prv.h"
#include <limits.h>

/*
 * Do one step in retransmission mechanism.
 */
uint32_t tftp_rt_step(TFTP_TRANSACTION *trans)
{
    uint32_t retval;

    RTCS_ASSERT(trans != NULL);
    retval = 2*trans->recv_timeout;
    trans->rt_count++;

    if ((retval > TFTP_RD_MAX) || (trans->rt_count > TFTP_RC_MAX))
    {
        retval = 0;
    }
    return(retval);
}

/*
 * Reset retransmission mechanism
 */
void tftp_rt_reset(TFTP_TRANSACTION *trans)
{
    RTCS_ASSERT(trans != NULL);
    trans->recv_timeout = TFTP_RC_INIT;
    trans->rt_count = 0;
}

/*
 * Send error to client.
 */
void tftp_send_error(TFTP_TRANSACTION *trans, int32_t error_code)
{
    char     *buffer;
    uint32_t size;

    switch(error_code)
    {
        case TFTP_ERR_FILE_NOT_FOUND:
            trans->err_string = TFTP_ERR_NOT_FOUND_STRING;
            break;
        case TFTP_ERR_ACCESS_VIOLATION:
            trans->err_string = TFTP_ERR_ACCESS_STRING;
            break;
        case TFTP_ERR_DISK_FULL:
            trans->err_string = TFTP_ERR_DISK_FULL_STRING;
            break;
        case TFTP_ERR_ILLEGAL_OP:
            trans->err_string = TFTP_ERR_ILLEGAL_OP_STRING;
            break;
        case TFTP_ERR_ILLEGAL_TID:
            trans->err_string = TFTP_ERR_BAD_TID_STRING;
            break;
        case TFTP_ERR_FILE_EXISTS:
            trans->err_string = TFTP_ERR_EXISTS_STRING;
            break;
        case TFTP_ERR_ILLEGAL_USER:
            trans->err_string = TFTP_ERR_NO_USER_STRING;
            break;
        case TFTP_ERR_TIMEOUT:
            trans->err_string = TFTP_ERR_TIMEOUT_STRING;
            break;
        case TFTP_ERR_UNKNOWN:
        default:
            trans->err_string = TFTP_ERR_SERVER_STRING;
            break;
    }
    RTCS_ASSERT(trans != NULL);
    /* Write message to buffer and send it */
    buffer = trans->buffer;
    *((uint16_t *) buffer) = htons(TFTP_OPCODE_ERROR);
    buffer += sizeof(uint16_t);
    *((uint16_t *) buffer) = htons(-error_code);
    buffer += sizeof(uint16_t);
    size = TFTP_HEADER_SIZE;
    _mem_copy(trans->err_string, buffer, strlen(trans->err_string)+1);
    size += strlen(trans->err_string)+1;

    tftp_send(trans->sock, trans->buffer, size, 0);
}

/*
 * TFTP wrapper for send function.
 */
int32_t tftp_send(uint32_t sock, char* data, uint32_t data_size, uint32_t flags)
{
    return(send(sock, data, data_size, flags));
}

/*
 * Send acknowledgment packet.
 */
int32_t tftp_send_ack(TFTP_TRANSACTION *trans)
{
    char     *buffer;
    uint32_t length;
    
    RTCS_ASSERT(trans != NULL);
    buffer = trans->buffer;
    *((uint16_t *) buffer) = htons(TFTP_OPCODE_ACK);
    buffer += sizeof(uint16_t);
    *((uint16_t *) buffer) = htons(trans->n_block);
    length = TFTP_HEADER_SIZE;
    
    return(tftp_send(trans->sock, trans->buffer, length, 0));
}

/*
 * Send data packet.
 */
int32_t tftp_send_data(TFTP_TRANSACTION *trans)
{
    uint32_t length;
    char     *buffer;
    int32_t  size;

    RTCS_ASSERT(trans != NULL);
    buffer = trans->buffer;
    *((uint16_t *) buffer) = htons(TFTP_OPCODE_DATA);
    buffer += sizeof(uint16_t);
    *((uint16_t *) buffer) = htons(trans->n_block);
    buffer += sizeof(uint16_t);
    length = TFTP_HEADER_SIZE;
    tftp_fseek(trans->file, (trans->n_block-1)*TFTP_DATA_SIZE);
    size = tftp_fread((void *) buffer, 1, TFTP_DATA_SIZE, trans->file);
    if (size >= 0)
    {
        length += size;
        if 
            (
                (length < TFTP_DATA_SIZE+TFTP_HEADER_SIZE) ||
                (trans->n_block == USHRT_MAX)
            )
        {
            trans->last = true;
        }
        return(tftp_send(trans->sock, trans->buffer, length, 0));
    }
    else
    {
        tftp_send_error(trans, TFTP_ERR_UNKNOWN);
        return(RTCS_ERROR);
    }
}

/*
 * Receive data from TFTP socket.
 */
int32_t tftp_recv(TFTP_TRANSACTION *trans, char* data, uint32_t data_size)
{
    sockaddr    remote_sin = {0};
    uint16_t    sin_size;
    int32_t     retval;
    int32_t     error;
    rtcs_fd_set rfds;

    RTCS_ASSERT(trans != NULL);
    RTCS_FD_ZERO(&rfds);
    RTCS_FD_SET(trans->sock, &rfds);

    /* Wait for incoming ACK. */
    error = select(1, &rfds, NULL, NULL, trans->recv_timeout);
    if ((error == RTCS_ERROR) || (error == 0))
    {
        /* timeout or error */
        retval = error;
        goto EXIT;
    }
    else
    {
        /* data received */
        tftp_rt_reset(trans);
    }
    sin_size = sizeof(remote_sin);
    retval = recvfrom(trans->sock, data, data_size, 0, &remote_sin, &sin_size);

    if (trans->flags & TFTP_FLAG_SAVE_REMOTE)
    {
        trans->remote_sa = remote_sin;
    }

    /* Verify the sender's address and port */
    if (memcmp((void *) &remote_sin, (void *) &trans->remote_sa, sizeof(sockaddr)) != 0)
    {
        tftp_send_error(trans, TFTP_ERR_ILLEGAL_TID);
        retval = TFTP_ERR_ILLEGAL_TID;
    }
    
    EXIT:
    /* Prepare next retransmission. */
    trans->recv_timeout = tftp_rt_step(trans);
    if (trans->recv_timeout == 0)
    {
       retval = TFTP_ERR_TIMEOUT;
    }
    return(retval);
}

/*
 * Receive acknowledgment packet.
 */
int32_t tftp_recv_ack(TFTP_TRANSACTION *trans)
{
    int32_t     retval;

    RTCS_ASSERT(trans != NULL);
    retval = tftp_recv(trans, (char *) trans->buffer_ack, TFTP_HEADER_SIZE);
    if 
    (
        (retval == 0) ||
        (retval == RTCS_ERROR) ||
        (retval == TFTP_ERR_ILLEGAL_TID)
    )
    {
        goto EXIT;
    }
    /* Report timeout to client. */
    else if (retval == TFTP_ERR_TIMEOUT)
    {
        tftp_send_error(trans, TFTP_ERR_TIMEOUT);
        retval = RTCS_ERROR;
        goto EXIT;
    }
    /* Increase number of expected block and reset retransmission mechanism. */
    else if (retval == TFTP_HEADER_SIZE)
    {
        uint16_t    n_block;
        uint16_t    opcode;

        opcode = ntohs(trans->buffer_ack[0]);
        n_block = ntohs(trans->buffer_ack[1]);
        if 
            (
                (n_block == trans->n_block) &&
                (opcode == TFTP_OPCODE_ACK)
            )
        {
            trans->n_block++;
        }

        tftp_rt_reset(trans);
    }

    EXIT:
    return(retval);
}

/*
 * Receive data packet.
 */
int32_t tftp_recv_data(TFTP_TRANSACTION *trans)
{
    int32_t  retval;
    uint16_t n_block;
    uint16_t opcode;
    uint32_t length;
    char     *buffer;
    int32_t  size;

    RTCS_ASSERT(trans != NULL);
    /* Read data from socket */
    retval = tftp_recv(trans, trans->buffer, TFTP_MAX_MESSAGE_SIZE);
    if 
        (
            (retval == 0) ||
            (retval == RTCS_ERROR) ||
            (retval == TFTP_ERR_ILLEGAL_TID)
        )
    {
        trans->err_string = "";
        goto EXIT;
    }
    /* Report timeout to client. */
    else if (retval == TFTP_ERR_TIMEOUT)
    {
        tftp_send_error(trans, TFTP_ERR_TIMEOUT);
        retval = RTCS_ERROR;
        goto EXIT;
    }
    else
    {
        tftp_rt_reset(trans);
    }

    buffer = trans->buffer;
    opcode = ntohs(*((uint16_t *) buffer));
    buffer += sizeof(uint16_t);
    n_block = ntohs(*((uint16_t *) buffer));
    buffer += sizeof(uint16_t);

    /* Write valid data to file. */
    if 
    (
        (n_block == (trans->n_block+1)) &&
        (opcode == TFTP_OPCODE_DATA)
    )
    {
        trans->n_block++;
        length = retval - TFTP_HEADER_SIZE;
        if (length < TFTP_DATA_SIZE)
        {
            trans->last = true;
        }
        tftp_fseek(trans->file, (trans->n_block-1)*TFTP_DATA_SIZE);
        size = tftp_fwrite((void *) buffer, 1, length, trans->file);
        if (size < 0)
        {
            tftp_send_error(trans, TFTP_ERR_UNKNOWN);
            retval = RTCS_ERROR;
        }
        else if (size < length)
        {
            tftp_send_error(trans, TFTP_ERR_DISK_FULL);
            retval = RTCS_ERROR;
        }
        else
        {
        	retval = size;
        }
    }
    else if (opcode == TFTP_OPCODE_ERROR)
    {
        trans->err_string = trans->buffer+TFTP_HEADER_SIZE;
        retval = RTCS_ERROR;
    }
    else
    {
        trans->err_string = TFTP_ERR_BAD_TID_STRING;
        retval = RTCS_ERROR;
    }
    
    EXIT:
    return(retval);
}

/*
 * TFTP wrapper for fread()
 */
size_t tftp_fread(void * ptr, size_t size, size_t count, MQX_FILE_PTR stream)
{
    if (stream != NULL)
    {
        return(fread(ptr, size, count, stream));
    }
    else
    {
        return(size*count);
    }
}

/*
 * TFTP wrapper for fwrite()
 */
size_t tftp_fwrite(void * ptr, size_t size, size_t count, MQX_FILE_PTR stream)
{
    if (stream != NULL)
    {
        return(fwrite(ptr, size, count, stream));
    }
    else
    {
        return(size*count);
    }
}

/*
 * TFTP wrapper for fseek()
 */
int tftp_fseek (MQX_FILE_PTR stream, long int offset)
{
    if (stream)
    {
        return(fseek(stream, offset, IO_SEEK_SET));
    }
    else
    {
        return(0);
    }
}
