/*HEADER**********************************************************************
*
* Copyright 2008, 2014 Freescale Semiconductor, Inc.
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
*   This file contains the implementation of an MQX
*   socket I/O device.
*
*
*END************************************************************************/


#include <rtcs.h>
#include <limits.h>
#include <socket.h>

#if MQX_USE_IO_OLD
#include <fio.h>

#define SOCKIO_MIN_BUFFER (64)
#define SOCKIO_MAX_BUFFER (64*1024)

typedef struct sockio_buffer_struct
{
    uint8_t  *data;
    uint32_t size;
    uint32_t offset;
}SOCKIO_BUFFER_STRUCT;

typedef struct io_socket_struct
{
   uint32_t             sock;       /* Communication socket. */
   LWSEM_STRUCT         flush_sema; /* Semaphore for protection of TX buffer during fflush. */
   SOCKIO_BUFFER_STRUCT rx_buffer;  /* Receive buffer. */
   SOCKIO_BUFFER_STRUCT tx_buffer;  /* Transmit buffer. */
} IO_SOCKET_STRUCT;

static int32_t sockio_flush(IO_SOCKET_STRUCT *io_ptr);
static int32_t sockio_send(IO_SOCKET_STRUCT *io_ptr, uint8_t *buffer, uint32_t length, uint32_t flags);
static int32_t sockio_recv(IO_SOCKET_STRUCT *io_ptr, uint8_t *buffer, uint32_t length, uint32_t flags);

_mqx_int _io_socket_open  (MQX_FILE_PTR, char *,  char *);
_mqx_int _io_socket_close (MQX_FILE_PTR);
_mqx_int _io_socket_read  (MQX_FILE_PTR, char *, _mqx_int);
_mqx_int _io_socket_write (MQX_FILE_PTR, char *, _mqx_int);
_mqx_int _io_socket_ioctl (MQX_FILE_PTR, _mqx_uint, void *);

/*
 * Install sockio driver in old I/O.
 */
int32_t _io_socket_install(char *identifier)
{

   return _io_dev_install(identifier,
                          _io_socket_open,
                          _io_socket_close,
                          _io_socket_read,
                          _io_socket_write,
                          _io_socket_ioctl,
                          NULL);
}


/*
 * Open sockio driver, old I/O version.
 */
_mqx_int _io_socket_open(MQX_FILE_PTR fd_ptr, char *open_name_ptr, char *flags_ptr)
{
    IO_SOCKET_STRUCT *io_ptr;
    uint32_t         error;

    io_ptr = RTCS_mem_alloc_zero(sizeof(IO_SOCKET_STRUCT));
    if (io_ptr == NULL)
    {
        return(MQX_OUT_OF_MEMORY);
    }
    _mem_set_type(io_ptr, MEM_TYPE_IO_SOCKET_STRUCT);

    io_ptr->rx_buffer.data = RTCS_mem_alloc_zero(SOCKIO_MIN_BUFFER*sizeof(uint8_t));
    if (io_ptr->rx_buffer.data == NULL)
    {
        _mem_free(io_ptr);
        return(MQX_OUT_OF_MEMORY);
    }
    io_ptr->rx_buffer.size = SOCKIO_MIN_BUFFER;

    io_ptr->tx_buffer.data = RTCS_mem_alloc_zero(SOCKIO_MIN_BUFFER*sizeof(uint8_t));
    if (io_ptr->tx_buffer.data == NULL)
    {
        _mem_free(io_ptr->rx_buffer.data);
        _mem_free(io_ptr);
        return(MQX_OUT_OF_MEMORY);
    }
    io_ptr->tx_buffer.size = SOCKIO_MIN_BUFFER;
    error = _lwsem_create(&io_ptr->flush_sema, 1);
    if (error != MQX_OK)
    {
        _mem_free(io_ptr->rx_buffer.data);
        _mem_free(io_ptr->tx_buffer.data);
        _mem_free(io_ptr);
        return(error);
    }

    fd_ptr->DEV_DATA_PTR = io_ptr;
    io_ptr->sock = (uint32_t) flags_ptr;
    if (!SOCK_check_valid(io_ptr->sock))
    {
        _mem_free(io_ptr->rx_buffer.data);
        _mem_free(io_ptr->tx_buffer.data);
        _mem_free(io_ptr);
        return(MQX_INVALID_PARAMETER);
    }

    return(MQX_OK);
}

/*
 * Close sockio driver - old I/O version.
 */
_mqx_int _io_socket_close(MQX_FILE_PTR fd_ptr)
{
    IO_SOCKET_STRUCT *io_ptr;

    io_ptr = fd_ptr->DEV_DATA_PTR;

    if (io_ptr != NULL) 
    {
        uint8_t *rx_buffer;
        uint8_t *tx_buffer;

        rx_buffer = io_ptr->rx_buffer.data;
        tx_buffer = io_ptr->tx_buffer.data;

        if (rx_buffer != NULL) 
        {
            _mem_free(rx_buffer);
        }
        if (tx_buffer != NULL) 
        {
            _mem_free(tx_buffer);
        }
        _lwsem_destroy(&io_ptr->flush_sema);
        _mem_free(io_ptr);
        fd_ptr->DEV_DATA_PTR = NULL;
    }
    return(MQX_OK);
} 

/*
 * Write data from user buffer to socket - old I/O version.
 */
_mqx_int _io_socket_write(MQX_FILE_PTR fd_ptr, char *data_ptr, _mqx_int num)
{
    IO_SOCKET_STRUCT *io_ptr;
    uint32_t         space;
    uint32_t         retval;

    /* Check input. */
    if 
        (
            (fd_ptr == NULL) ||
            (
                (data_ptr == NULL) && 
                (num != 0)
            )
        )
    {
        return(IO_ERROR);
    }

    io_ptr = fd_ptr->DEV_DATA_PTR;
    space = io_ptr->tx_buffer.size - io_ptr->tx_buffer.offset;
    retval = num;

    /* User data is bigger than sockio buffer - send user data directly */
    if (num > io_ptr->tx_buffer.size)
    {
        /* If there are some data already buffered send them to client first */
        if (sockio_flush(io_ptr) == IO_ERROR)
        {
            return(IO_ERROR);
        }
        return(sockio_send(io_ptr, (uint8_t *) data_ptr, num, 0));
    }

    /* No space in buffer - make some */
    if (space < num)
    {
        if (sockio_flush(io_ptr) == IO_ERROR)
        {
            return(IO_ERROR);
        }
    }

    /* Now we can save user data to buffer and eventually send them to client */
    _mem_copy(data_ptr, io_ptr->tx_buffer.data+io_ptr->tx_buffer.offset, num);
    io_ptr->tx_buffer.offset += num;
    
    if (io_ptr->tx_buffer.offset >= io_ptr->tx_buffer.size)
    {
        if (sockio_flush(io_ptr) == IO_ERROR)
        {
            return(IO_ERROR);
        }
    }
    return(retval);
}

/*
 * Read data from socket to user buffer - old I/O version
 */
_mqx_int _io_socket_read(MQX_FILE_PTR fd_ptr, char *data_ptr, _mqx_int num)
{
    int              read = 0;
    IO_SOCKET_STRUCT *io_ptr;
    uint32_t         data_size;

    /* Check input. */
    if 
        (
            (fd_ptr == NULL) ||
            (
                (data_ptr == NULL) && 
                (num != 0)
            )
        )
    {
        return(IO_ERROR);
    }

    io_ptr = fd_ptr->DEV_DATA_PTR;
    data_size = io_ptr->rx_buffer.offset;
    
    /* If there are any data in buffer copy them to user buffer */
    if (data_size > 0)
    {
        uint32_t length;
        uint32_t tail;
        uint32_t size;
        uint8_t  *data;

        size = io_ptr->rx_buffer.size;
        data = io_ptr->rx_buffer.data;
        length = (data_size < num) ? data_size : num;
        tail = size-length;
        
        _mem_copy(data, data_ptr, length);
        memmove(data, data+length, tail);
        _mem_zero(data+tail, length);
        io_ptr->rx_buffer.offset -= length;
        read = length;
    }

    /* If there is some space remaining in user buffer try to read from socket */
    while (read < num)
    {
        uint32_t received;
        
        received = sockio_recv(io_ptr, (uint8_t *) data_ptr+read, num-read, 0);
        if ((uint32_t) RTCS_ERROR == received)
        {
            return(IO_ERROR);
        }
        read += received;
    }
    
    /* If there is some space in buffer, try to read data from socket to it. */
    uint32_t space;

    space = io_ptr->rx_buffer.size - io_ptr->rx_buffer.offset;
    if (space != 0)
    {
        int32_t  retval;

        retval = sockio_recv(io_ptr, io_ptr->rx_buffer.data+io_ptr->rx_buffer.offset, space, MSG_DONTWAIT);
        if (retval == RTCS_ERROR)
        {
            return(IO_ERROR);
        }
        io_ptr->rx_buffer.offset += retval;
    }
    return(read);
}

/*
 * Flush content of sockio tx_buffer to client.
 */
static int32_t sockio_flush(IO_SOCKET_STRUCT *io_ptr)
{
    uint32_t send_total;
    uint32_t data_length;
    uint8_t  *data;
    
    /* Wait until buffer is available */
    _lwsem_wait(&io_ptr->flush_sema);
    send_total = 0;
    data_length = io_ptr->tx_buffer.offset;
    data = io_ptr->tx_buffer.data;
    
    while (send_total < data_length)
    {
        uint32_t send_now;

        send_now = sockio_send(io_ptr, data+send_total, data_length-send_total, 0);
        if (send_now == (uint32_t) RTCS_ERROR)
        {
            send_total = (uint32_t) IO_ERROR;
            break;
        }
        send_total += send_now;
    }
    io_ptr->tx_buffer.offset = 0;
    _lwsem_post(&io_ptr->flush_sema);
    return(send_total);
}

/*
 * Send data to socket.
 */
static int32_t sockio_send(IO_SOCKET_STRUCT *io_ptr, uint8_t *buffer, uint32_t length, uint32_t flags)
{
    int32_t retval;

    retval = send(io_ptr->sock, buffer, length, flags);
    if (retval == RTCS_ERROR)
    {
        retval = IO_ERROR;
    }
    return(retval);
}

/*
 * Receive data from socket.
 */
static int32_t sockio_recv(IO_SOCKET_STRUCT *io_ptr, uint8_t *buffer, uint32_t length, uint32_t flags)
{
    int32_t retval;

    retval = recv(io_ptr->sock, buffer, length, flags);
    if (retval == RTCS_ERROR)
    {
        retval = IO_ERROR;
    }
    return(retval);
}

/*
 * I/O control for sockio driver.
 */
_mqx_int _io_socket_ioctl(MQX_FILE_PTR fd_ptr, _mqx_uint cmd, void *param_ptr)
{
    IO_SOCKET_STRUCT *  io_ptr = fd_ptr->DEV_DATA_PTR;
    _mqx_int       result = IO_ERROR;

    if (io_ptr == NULL) 
    {
        return(result);
    }

    switch (cmd)
    {
        case IO_IOCTL_CHAR_AVAIL:
            if (io_ptr->rx_buffer.offset > 0)
            {
                *(bool *)param_ptr = TRUE;
            }
            else
            {
                rtcs_fd_set rfds;

                RTCS_FD_ZERO(&rfds);
                RTCS_FD_SET(io_ptr->sock, &rfds);

                if(1 == select(1, &rfds, NULL, NULL, (uint32_t) -1)) /* last parameter = poll mode */
                {
                    *(bool *)param_ptr = TRUE;
                }
                else
                {
                    *(bool *)param_ptr = FALSE;
                }
            }
            result = MQX_OK;
            break;

        case IO_IOCTL_FLUSH_OUTPUT:
            if (io_ptr->tx_buffer.offset > 0)
            {
                sockio_flush(io_ptr);
            }
            else
            {
                result = MQX_OK;
            }
            break;

        case IO_IOCTL_SET_BLOCK_MODE:
            break;      

        case IO_IOCTL_GET_BLOCK_SIZE:
            break;   
    }
    return result;
}
#endif
