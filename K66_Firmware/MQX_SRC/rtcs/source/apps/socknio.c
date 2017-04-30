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
*   This file contains the implementation of an MQX
*   socket I/O device in NIO.
*
*
*END************************************************************************/



#include <rtcs.h>

#if !MQX_USE_IO_OLD
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <socket.h>
#include <nio.h>
#include <nio/ioctl.h>
#include <fcntl.h>
#include <errno.h>

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

static int _io_socket_open(void *dev_context, const char *dev_name, int flags, void **fp_context, int *error);
static int _io_socket_read(void *dev_context, void *fp_context, void *buf, size_t nbytes, int *error);
static int _io_socket_write(void *dev_context, void *fp_context, const void *buf, size_t nbytes, int *error);
static int _io_socket_ioctl(void *dev_context, void *fp_context, unsigned long int request, int *error, va_list ap);
static int _io_socket_close(void *dev_context, void *fp_context, int *error);
static int _io_socket_init(void *init_data, void **dev_context, int *error);
static int _io_socket_deinit(void *dev_context, int *error);

const NIO_DEV_FN_STRUCT _io_socket_dev_fn = {
    .OPEN = _io_socket_open,
    .READ = _io_socket_read,
    .WRITE = _io_socket_write,
    .LSEEK = NULL,
    .IOCTL = _io_socket_ioctl,
    .CLOSE = _io_socket_close,
    .INIT = _io_socket_init,
    .DEINIT = _io_socket_deinit,
};

/*
 * Open sockio driver, NIO version.
 */
static int _io_socket_open(void *dev_context, const char *dev_name, int flags, void **fp_context, int *error) 
{
    IO_SOCKET_STRUCT  *io_ptr;
    const char        *sock_handle_str = NULL;

    io_ptr = RTCS_mem_alloc_zero(sizeof(IO_SOCKET_STRUCT));   
    if(io_ptr == NULL) 
    {
        if (error) {
            *error = NIO_ENOMEM;
        }
        return(-1);
    }
    *fp_context = (void*) io_ptr;
    _mem_set_type(io_ptr, MEM_TYPE_IO_SOCKET_STRUCT);

    io_ptr->rx_buffer.data = RTCS_mem_alloc_zero(SOCKIO_MIN_BUFFER*sizeof(uint8_t));
    if (io_ptr->rx_buffer.data == NULL)
    {
        _mem_free(io_ptr);
        if (error) {
            *error = NIO_ENOMEM;
        }
        return(-1);
    }
    io_ptr->rx_buffer.size = SOCKIO_MIN_BUFFER;

    io_ptr->tx_buffer.data = RTCS_mem_alloc_zero(SOCKIO_MIN_BUFFER*sizeof(uint8_t));
    if (io_ptr->tx_buffer.data == NULL)
    {
        _mem_free(io_ptr->rx_buffer.data);
        _mem_free(io_ptr);
        if (error) {
            *error = NIO_ENOMEM;
        }
        return(-1);
    }
    io_ptr->tx_buffer.size = SOCKIO_MIN_BUFFER;
    /* 
     * expected dev_name: "socket:SOCKET_HANDLE" 
     * where SOCKET_HANDLE is decimal string of socket handle
     */
    sock_handle_str = strchr(dev_name, ':');
    if(NULL == sock_handle_str)
    {
        _mem_free(io_ptr->rx_buffer.data);
        _mem_free(io_ptr->tx_buffer.data);
        _mem_free(io_ptr);
        if (error) {
            *error = NIO_EBADF;
        }
        return(-1);
    }
    sock_handle_str++;
    _lwsem_create(&io_ptr->flush_sema, 1);
    io_ptr->sock = (uint32_t) strtoul(sock_handle_str, NULL, 10);

    /* Check conversion result and socket validity. */
    if 
        (
            (io_ptr->sock == 0) || 
            (io_ptr->sock == LONG_MAX) || 
            (io_ptr->sock == (uint32_t) LONG_MIN) || 
            (io_ptr->sock == RTCS_SOCKET_ERROR)
        )
    {
        _mem_free(io_ptr->rx_buffer.data);
        _mem_free(io_ptr->tx_buffer.data);
        _mem_free(io_ptr);
        if (error) {
            *error = NIO_EINVAL;
        }
        return(-1);
    }

    return(0);
}

/*
 * Close sockio driver - new I/O version.
 */
static int _io_socket_close(void *dev_context, void *fp_context, int *error) 
{
    IO_SOCKET_STRUCT *io_ptr;

    io_ptr = (IO_SOCKET_STRUCT*) fp_context;

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
    }
    _mem_free(fp_context);
    return(0);
}

/*
 * Write data from user buffer to socket - new I/O version.
 */
static int _io_socket_write(void *dev_context, void *fp_context, const void *data_ptr, size_t num, int *error)
{
    IO_SOCKET_STRUCT *io_ptr;
    uint32_t         space;
    uint32_t         retval;

    /* Check input. */
    if 
        (
            (fp_context == NULL) ||
            (
                (data_ptr == NULL) && 
                (num != 0)
            )
        )
    {
        if (error) {
            *error = NIO_EIO;
        }
        return(-1);
    }

    io_ptr = (IO_SOCKET_STRUCT *) fp_context;
    space = io_ptr->tx_buffer.size - io_ptr->tx_buffer.offset;
    retval = num;

    /* User data is bigger than sockio buffer - send user data directly */
    if (num > io_ptr->tx_buffer.size)
    {
        /* If there are some data already buffered send them to client first */
        if (sockio_flush(io_ptr) == -1)
        {
            if (error) {
                *error = NIO_EIO;
            }
            return(-1);
        }
        return(sockio_send(io_ptr, (void*) data_ptr, num, 0));
    }

    /* No space in buffer - make some */
    if (space < num)
    {
        if (sockio_flush(io_ptr) == -1)
        {
            if (error) {
                *error = NIO_EIO;
            }
            return(-1);
        }
    }

    /* Now we can save user data to buffer and eventually send them to client */
    _mem_copy(data_ptr, io_ptr->tx_buffer.data+io_ptr->tx_buffer.offset, num);
    io_ptr->tx_buffer.offset += num;
    
    if 
        (
            (io_ptr->tx_buffer.offset >= io_ptr->tx_buffer.size) || 
            ((data_ptr == NULL) && (num == 0))
        )
    {
        if (sockio_flush(io_ptr) == -1)
        {
            if (error) {
                *error = NIO_EIO;
            }
            return(-1);
        }
    }
    return(retval);
}

/*
 * Read data from socket to user buffer - new I/O version
 */
static int _io_socket_read(void *dev_context, void *fp_context, void *data_ptr, size_t num, int *error)
{
    int               read = 0;
    IO_SOCKET_STRUCT  *io_ptr;
    uint32_t          data_size;

    /* Check input. */
    if 
        (
            (fp_context == NULL) ||
            (
                (data_ptr == NULL) && 
                (num != 0)
            )
        )
    {
        if (error) {
            *error = NIO_EIO;
        }
        return(-1);
    }

    io_ptr = (IO_SOCKET_STRUCT *) fp_context;
    data_size = io_ptr->rx_buffer.offset;
    
    /* If there are any data in buffer copy them to user buffer. */
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

    /* If there is some space remaining in user buffer try to read from socket. */
    while (read < num)
    {
        uint32_t received;
        
        received = sockio_recv(io_ptr, (uint8_t*)data_ptr+read, num-read, 0);
        if ((uint32_t) RTCS_ERROR == received)
        {
            break;
        }
        read += received;
    }
    
    /* If buffer is at least half-empty, try to read data from socket to it. */
    uint32_t space;

    space = io_ptr->rx_buffer.size - io_ptr->rx_buffer.offset;
    if (space > io_ptr->rx_buffer.size/2)
    {
        int32_t  retval;

        retval = sockio_recv(io_ptr, io_ptr->rx_buffer.data+io_ptr->rx_buffer.offset, space, MSG_DONTWAIT);
        if (retval == RTCS_ERROR)
        {
            if (error) {
                *error = NIO_EIO;
            }
            return(-1);
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
            send_total = (uint32_t) -1;
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
        retval = -1;
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
        retval = -1;
    }
    return(retval);
}

/*
 * I/O control for sockio driver.
 */
static int _io_socket_ioctl(void *dev_context, void *fp_context, unsigned long int cmd, int *error, va_list ap)
{

    IO_SOCKET_STRUCT      *io_ptr = (IO_SOCKET_STRUCT *) fp_context;
    _mqx_int              result = -1;
    void                  *param_ptr;

    param_ptr = va_arg(ap, void*);    
    

    if (io_ptr == NULL) 
    {
        if (error) {
            *error = NIO_EIO;
        }
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

    if (!result && error) {
        *error = NIO_EIO;
    }

    return result;
}

/* 
 * Dummy init function for NIO
 */
static int _io_socket_init(void *init_data, void **dev_context, int *error)
{
    *dev_context = NULL;
    return 0;
}

/* 
 * Dummy deinit function for NIO
 */
static int _io_socket_deinit(void *dev_context, int *error)
{
    return 0;
}
#endif
