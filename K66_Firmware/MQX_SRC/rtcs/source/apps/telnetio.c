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
*   This file contains the implementation of an MQX
*   telnet I/O device.
*
*
*END************************************************************************/

#include <rtcs.h>

#if MQX_USE_IO_OLD
    #include <fio.h>
    #include <serial.h>
    #else
    #include <mqx_inc.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <nio.h>
    #include <nio/ioctl.h>
    #include <fcntl.h>
    #include <errno.h>

    #if PLATFORM_SDK_ENABLED
        #include <nio/drivers/nio_tty/nio_tty.h>
        #define IO_SERIAL_ECHO NIO_TTY_FLAGS_ECHO
    #else
        #include <nserial.h>
        #include <ntty.h>
        #define IO_SERIAL_ECHO NTTY_FLAGS_ECHO
    #endif

    #define MQX_FILE_PTR  FILE *
    #define _io_write(f,b,n) fwrite(b,1,n,f)
    #define _io_read(f, b, n) fread(b,1,n,f)
    #define fungetc(c,f) ungetc(c,f)
    #define IO_EOF (-1)
    #define IO_ERROR (-1)
#endif

#include "telnet.h"

#define CRLF_MODE(io_ptr)        (io_ptr->TERMINAL_OPTIONS & TEROPT_CRLF_MODE)
#define RAW_MODE(io_ptr)         (io_ptr->TERMINAL_OPTIONS & TEROPT_RAW_MODE)

typedef struct io_telnet
{
    MQX_FILE_PTR  NETWORK_FD;
    MQX_FILE_PTR  CONSOLE_FD;
    unsigned char REMOTE_OPTIONS[TELNET_MAX_OPTS];
    unsigned char LOCAL_OPTIONS[TELNET_MAX_OPTS];
    uint32_t      TERMINAL_OPTIONS;
    uint32_t      FLAGS;
    int32_t       STATE;
} IO_TELNET, * IO_TELNET_PTR;

static size_t telnetio_fwrite(MQX_FILE_PTR fd, void *data, size_t length);
static size_t telnetio_fread(MQX_FILE_PTR fd, void *data, size_t length);

#if MQX_USE_IO_OLD
_mqx_int _io_telnet_open  (MQX_FILE_PTR, char *,  char *);
_mqx_int _io_telnet_close (MQX_FILE_PTR);
_mqx_int _io_telnet_read  (MQX_FILE_PTR, char *, _mqx_int);
_mqx_int _io_telnet_write (MQX_FILE_PTR, char *, _mqx_int);
_mqx_int _io_telnet_ioctl (MQX_FILE_PTR, _mqx_uint, void *);

_mqx_int _io_telnet_process_char (char *, MQX_FILE_PTR);
#else // !MQX_USE_IO_OLD
static int _io_telnet_open(void *dev_context, const char *dev_name, int flags, void **fp_context, int *error);
static int _io_telnet_read(void *dev_context, void *fp_context, void *buf, size_t nbytes, int *error);
static int _io_telnet_write(void *dev_context, void *fp_context, const void *buf, size_t nbytes, int *error);
static int _io_telnet_ioctl(void *dev_context, void *fp_context, unsigned long int request, int *error, va_list ap);
static int _io_telnet_close(void *dev_context, void *fp_context, int *error);
static int _io_telnet_init(void *init_data, void **dev_context, int *error);
static int _io_telnet_deinit(void *dev_context, int *error);

const NIO_DEV_FN_STRUCT _io_telnet_dev_fn = {
    .OPEN = _io_telnet_open,
    .READ = _io_telnet_read,
    .WRITE = _io_telnet_write,
    .LSEEK = NULL,
    .IOCTL = _io_telnet_ioctl,
    .CLOSE = _io_telnet_close,
    .INIT = _io_telnet_init,
    .DEINIT = _io_telnet_deinit,
};

_mqx_int _io_telnet_process_char (char *, void *fp_context);
#endif

bool _io_telnet_echo(IO_TELNET_PTR io_ptr, char c);

/*
** Telnet Response strings
*/
/*
static _mqx_int TELNET_go_ahead(MQX_FILE_PTR fd) {
    char buf[] = {TELCMD_IAC, TELCMD_GA};
    return _io_write(fd, buf, 2);
}
*/

static size_t telnetio_fwrite(MQX_FILE_PTR fd, void *data, size_t length)
{
    #if MQX_USE_IO_OLD
    return(write(fd, data, length));
    #else
    size_t retval;

    retval = fwrite(data, 1, length, fd);
    fflush(fd);
    return(retval);
    #endif
}

static size_t telnetio_fread(MQX_FILE_PTR fd, void *data, size_t length)
{
     #if MQX_USE_IO_OLD
    return(read(fd, data, length));
    #else
    return(fread(data, 1, length, fd));
    #endif
}

static _mqx_int TELNET_will(MQX_FILE_PTR fd, char c)
{
    char buf[3] = {TELCMD_IAC, TELCMD_WILL};
    buf[2] = c;
    return(telnetio_fwrite(fd, buf, 3));
}

static _mqx_int TELNET_wont(MQX_FILE_PTR fd, char c)
{
    char buf[3] = {TELCMD_IAC, TELCMD_WONT};
    buf[2] = c;
    return(telnetio_fwrite(fd, buf, 3));
}

static _mqx_int TELNET_do(MQX_FILE_PTR fd, char c)
{
    char buf[3] = {TELCMD_IAC, TELCMD_DO};
    buf[2] = c;
    return(telnetio_fwrite(fd, buf, 3));
}

static _mqx_int TELNET_dont(MQX_FILE_PTR fd, char c)
{
    char buf[3] = {TELCMD_IAC, TELCMD_DONT};
    buf[2] = c;
    return(telnetio_fwrite(fd, buf, 3));
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name  : _io_telnet_install
* Returned Value : int32_t a task error code or MQX_OK
* Comments       : Install the telnet device
*
*END*-----------------------------------------------------------------*/
#if MQX_USE_IO_OLD
int32_t _io_telnet_install(char *identifier)
{

    return _io_dev_install(identifier,
                          _io_telnet_open,
                          _io_telnet_close,
                          _io_telnet_read,
                          _io_telnet_write,
                          _io_telnet_ioctl,
                          NULL );
}
#endif // MQX_USE_IO_OLD

/*FUNCTION*-------------------------------------------------------------
*
* Function Name  : _io_telnet_open
* Returned Value : int32_t a task error code or MQX_OK
* Comments       : Open the telnet device
*
*END*-----------------------------------------------------------------*/
#if MQX_USE_IO_OLD
_mqx_int _io_telnet_open(MQX_FILE_PTR fd_ptr, char *open_name_ptr, char *flags)
{
    IO_TELNET_PTR io_ptr;

    io_ptr = RTCS_mem_alloc_zero(sizeof(*io_ptr));
    if (io_ptr == NULL)
    {
        return(MQX_OUT_OF_MEMORY);
    }

    _mem_set_type(io_ptr, MEM_TYPE_IO_TELNET);

    fd_ptr->DEV_DATA_PTR = io_ptr;
    fd_ptr->FLAGS = IO_SERIAL_ECHO;  /* To work around a bug in fgetline() */

    io_ptr->NETWORK_FD = (MQX_FILE_PTR) flags;
#else
static int _io_telnet_open(void *dev_context, const char *dev_name, 
                           int flags, void **fp_context, int *error) 
{
    const char * sockio_handle_str = NULL;  
    IO_TELNET_PTR io_ptr;

    io_ptr = RTCS_mem_alloc_zero(sizeof(*io_ptr));
    if (io_ptr == NULL)
    {
        if (error) {
            *error = NIO_ENOMEM;
        }
        return(-1);
    }
    _mem_set_type(io_ptr, MEM_TYPE_IO_TELNET);
  
    (*fp_context) = (void*)io_ptr;
    
    /* expected dev_name: "telnet:SOCKIO_HANDLE" 
     * where SOCKIO_HANDLE is decimal string of socket: NIO device handle */
    sockio_handle_str = strchr(dev_name, ':');
    if(NULL == sockio_handle_str)
    {
        if (error) {
            *error = NIO_EBADF;
        }
        return(-1);
    }
    sockio_handle_str++;    
    io_ptr->NETWORK_FD = (FILE *) strtoul(sockio_handle_str, NULL, 10);
    io_ptr->REMOTE_OPTIONS[TELOPT_BINARY] = 1;
#endif   
    
    io_ptr->CONSOLE_FD = NULL;
    io_ptr->TERMINAL_OPTIONS = TEROPT_CRLF_MODE;

    /*
     * Send WILL SGA and DO SGA options
     */
    TELNET_will(io_ptr->NETWORK_FD, TELOPT_SGA);
    TELNET_do(io_ptr->NETWORK_FD, TELOPT_SGA);
    fflush(io_ptr->NETWORK_FD);
    return(MQX_OK);
}


/*FUNCTION*-------------------------------------------------------------
*
* Function Name  :  _io_telnet_close
* Returned Value :  int32_t Error Code
* Comments       :  Close the telnet device
*
*END*-----------------------------------------------------------------*/
#if MQX_USE_IO_OLD
_mqx_int _io_telnet_close(MQX_FILE_PTR fd_ptr)
{
    IO_TELNET_PTR  io_ptr;
    _mqx_uint      flags;

    io_ptr = fd_ptr->DEV_DATA_PTR;
    if (io_ptr != NULL)
    {
        /* Reenable local echo */
        if (io_ptr->CONSOLE_FD)
        {
            ioctl(io_ptr->CONSOLE_FD, IO_IOCTL_SERIAL_GET_FLAGS, &flags);
            flags |= IO_SERIAL_ECHO;
            ioctl(io_ptr->CONSOLE_FD, IO_IOCTL_SERIAL_SET_FLAGS, &flags);
        }
        _mem_free(io_ptr);
        fd_ptr->DEV_DATA_PTR = NULL;
    }
    return MQX_OK;
}

#else // !MQX_USE_IO_OLD
static int _io_telnet_close(void *dev_context, void *fp_context, int *error) 
{
    IO_TELNET_PTR  io_ptr = (IO_TELNET_PTR)fp_context;
    /* Reenable local echo */
    #if PLATFORM_SDK_ENABLED
    uint32_t flags = NIO_TTY_FLAGS_EOL_RN | NIO_TTY_FLAGS_ECHO;
    ioctl(fileno(io_ptr->CONSOLE_FD), IOCTL_NIO_TTY_SET_FLAGS, flags);
    #else
    uint32_t flags = NTTY_FLAGS_EOL_CRLF | NTTY_FLAGS_ECHO;
    ioctl(fileno(io_ptr->CONSOLE_FD), IOCTL_NTTY_SET_FLAGS, flags);
    #endif
    _mem_free(fp_context);
    return(0);
}
#endif

/*FUNCTION*-------------------------------------------------------------
*
* Function Name  :  _io_telnet_read
* Returned Value :  Number of characters read
* Comments       :  Read data from the Telnet device
*
*END*-----------------------------------------------------------------*/
#if MQX_USE_IO_OLD
_mqx_int _io_telnet_read(MQX_FILE_PTR fd_ptr, char *data_ptr, _mqx_int num)
{
    IO_TELNET_PTR  io_ptr;
    io_ptr = fd_ptr->DEV_DATA_PTR;
#else // !MQX_USE_IO_OLD   
static int _io_telnet_read(void *dev_context, void *fp_context, void *data_ptr, size_t num, int *error) 
{
    IO_TELNET *io_ptr = (IO_TELNET *) fp_context;
    void      *fd_ptr = fp_context;   
#endif // MQX_USE_IO_OLD
   
    _mqx_int  count;
    _mqx_int  read_count;
    _mqx_int  write_count;
    _mqx_int  ret;
    char      *read_ptr;
    char      *write_ptr;
    char      c;

    write_ptr = data_ptr;
    write_count = 0;

    for (;;)
    {
        read_ptr = write_ptr;
        count = telnetio_fread(io_ptr->NETWORK_FD, read_ptr, num-write_count);
        #if MQX_USE_IO_OLD
        if (count == IO_EOF)
        {
            fflush(io_ptr->NETWORK_FD);
            return IO_EOF;
        }
        #else      
        if((feof(io_ptr->NETWORK_FD)) || (ferror(io_ptr->NETWORK_FD)))
        {
            fflush(io_ptr->NETWORK_FD);
            if (error) {
                *error = IO_EOF;
            }
            return -1;
        }
        #endif // MQX_USE_IO_OLD

        for (read_count = 0; read_count < count; read_count++)
        {
            c = *read_ptr++;
            if ((ret = _io_telnet_process_char(&c, fd_ptr)) == IO_ERROR)
            {
                return ret;
            }
            else if (ret)
            {
                *write_ptr++ = c;
                write_count++;
            }
        }

        if (write_count >= num)
        {
            break;
        }
        #if MQX_USE_IO_OLD
        if (write_count && !_io_fstatus(io_ptr->NETWORK_FD))
        {
            break;
        }
        #else
        {
            bool result = TRUE;
            if (write_count)
            {            
                ioctl(fileno(io_ptr->NETWORK_FD), IO_IOCTL_CHAR_AVAIL, &result);
            }
            if(!result)
            {
                break;
            }
        }
#endif
    }
    fflush(io_ptr->NETWORK_FD);
    return(write_count);
}


/*FUNCTION*-------------------------------------------------------------
*
* Function Name  : _io_telnet_write
* Returned Value : Number of characters written
* Comments       : Writes the data to the Telnet device
*
*END*-----------------------------------------------------------------*/
#if MQX_USE_IO_OLD
_mqx_int _io_telnet_write(MQX_FILE_PTR fd_ptr, char *data_ptr, _mqx_int num)
{
    IO_TELNET_PTR io_ptr;
    io_ptr = fd_ptr->DEV_DATA_PTR;
#else
static int _io_telnet_write(void *dev_context, void *fp_context, 
                            const void *buf, size_t num, int *error) {
    IO_TELNET * io_ptr = (IO_TELNET *)fp_context;
    char * data_ptr = (char*)buf;
#endif // MQX_USE_IO_OLD
    _mqx_int count;
    char *scan_ptr;
    
    for (count = 0, scan_ptr = data_ptr; count < num; count++, scan_ptr++) {
        //transmitting data in real binary mode and avoiding workarounds
        //in handling EOLs (different CR LF combinations) valid only for character based terminals
        if (RAW_MODE(io_ptr))
        {
            if (io_ptr->REMOTE_OPTIONS[TELOPT_BINARY])
            {
                if (*scan_ptr == TELCMD_IAC)
                {
                    if (fputc(TELCMD_IAC, io_ptr->NETWORK_FD) == IO_EOF)
                    {
                        return count;
                    }
                    if (fputc(TELCMD_IAC, io_ptr->NETWORK_FD) == IO_EOF)
                    {
                        return count;
                    }
                }
                else
                {
                    if (fputc(*scan_ptr, io_ptr->NETWORK_FD) == IO_EOF)
                    {
                        return count;
                    }
                }
            }
            else
            { //sending binary data to ASCII remote end should never happen
                #if !MQX_USE_IO_OLD
                if (error)
                {
                    *error = IO_ERROR;
                }
                #endif
                return -1;
            }
            continue;
        }

        //handle transmissions for character based terminals including CR LF workarounds
        switch (*scan_ptr)
        {
            case TELCMD_IAC:
            if (io_ptr->REMOTE_OPTIONS[TELOPT_BINARY])
            {
                if (fputc(TELCMD_IAC, io_ptr->NETWORK_FD) == IO_EOF)
                {
                    return count;
                }
                if (fputc(TELCMD_IAC, io_ptr->NETWORK_FD) == IO_EOF)
                {
                    return count;
                }
            } // else character is not valid USASCII so filter it
            break;

            case TELCC_LF:
                if (fputc(TELCC_CR, io_ptr->NETWORK_FD) == IO_EOF)
                {
                    return count;
                }
                if (io_ptr->REMOTE_OPTIONS[TELOPT_BINARY])
                {
                    if (fputc(TELCC_LF, io_ptr->NETWORK_FD) == IO_EOF)
                    {
                        return count;
                    }
                }
                else if (CRLF_MODE(io_ptr))
                {//in ascii mode CR should be followed by either NULL or LF
                    if (fputc(TELCC_LF, io_ptr->NETWORK_FD) == IO_EOF)
                    {
                        return count;
                    }
                }
                else
                {
                    if (fputc(TELCC_NULL, io_ptr->NETWORK_FD) == IO_EOF)
                    {
                        return count;
                    }
                }
                break;

            case TELCC_CR:
                //in character based terminal in CRLF mode is possible to transmit stand-alone CRs
                if (CRLF_MODE(io_ptr) && !RAW_MODE(io_ptr))
                {
                    if ((count < num) && (*(scan_ptr + 1) != TELCC_LF))
                    {
                        if (fputc(TELCC_CR, io_ptr->NETWORK_FD) == IO_EOF)
                        {
                            return count;
                        }
                        if (fputc(TELCC_NULL, io_ptr->NETWORK_FD) == IO_EOF)
                        {
                            return count;
                        }
                    }
                }
                break;

            default:
                if (io_ptr->REMOTE_OPTIONS[TELOPT_BINARY])
                {
                    if (fputc(*scan_ptr, io_ptr->NETWORK_FD) == IO_EOF)
                    {
                        return count;
                    }
                }
                else if (!(*scan_ptr & 0x80))
                {
                    if (fputc(*scan_ptr, io_ptr->NETWORK_FD) == IO_EOF)
                    {
                        return count;
                    }
                } //else filter it
                break;
        }
    }
    fflush(io_ptr->NETWORK_FD);
    return count;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name  : _io_telnet_ioctl
* Returned Value : int32_t
* Comments       : Returns result of ioctl operation.
*
*END*-----------------------------------------------------------------*/
#if MQX_USE_IO_OLD
_mqx_int _io_telnet_ioctl(MQX_FILE_PTR fd_ptr, _mqx_uint cmd, void *param_ptr)
{
    IO_TELNET_PTR  io_ptr;
    int32_t         result;
    _mqx_uint      flags;
    int32_t         rawc;
    char           c;
    io_ptr = (IO_TELNET_PTR) fd_ptr->DEV_DATA_PTR;
#else
static int _io_telnet_ioctl(void *dev_context, void *fp_context, unsigned long int cmd, int *error, va_list ap) 
{
    IO_TELNET *io_ptr = (IO_TELNET *)fp_context;
    int32_t   result;
    int32_t   rawc;
    char      c;
    void      *param_ptr;
    void      *fd_ptr = fp_context;    

    param_ptr = (void*) va_arg(ap, void*);
#endif
    result = IO_ERROR;   
    bool do_fflush;

    do_fflush = true;
    if (io_ptr == NULL)
    {
        return (_mqx_int)result;
    }

    switch (cmd)
    {
        case IO_IOCTL_CHAR_AVAIL:
            while (1)
            {
                bool b_result;
                #if MQX_USE_IO_OLD
                b_result = fstatus(io_ptr->NETWORK_FD);
                #else
                ioctl(fileno(io_ptr->NETWORK_FD), IO_IOCTL_CHAR_AVAIL, &b_result);
                #endif
                *(bool *) param_ptr = b_result;
                if (!*(bool *)param_ptr)
                {
                    break;
                }
                rawc = (int32_t) fgetc(io_ptr->NETWORK_FD);
                if (rawc == IO_EOF)
                {
                    break;
                }
                c = (char)rawc;
                result = _io_telnet_process_char(&c, fd_ptr);
                if (result == IO_ERROR)
                {
                    #if !MQX_USE_IO_OLD
                    if (error) {
                        *error = IO_ERROR;
                    }
                    #endif
                    return -1;
                }
                else if (result)
                {
                    do_fflush = false;
                    fungetc(c, io_ptr->NETWORK_FD);
                    break;
                }
            }
            result = MQX_OK;
            break;

        case IO_IOCTL_SET_STREAM:
            io_ptr->CONSOLE_FD = (MQX_FILE_PTR)param_ptr;
            #if MQX_USE_IO_OLD
            /* Clear or set echoing by server */
            if (io_ptr->REMOTE_OPTIONS[TELOPT_ECHO])
            {
                ioctl(io_ptr->CONSOLE_FD, IO_IOCTL_SERIAL_GET_FLAGS, &flags);
                flags &= ~IO_SERIAL_ECHO;
                ioctl(io_ptr->CONSOLE_FD, IO_IOCTL_SERIAL_SET_FLAGS, &flags);
            }
            #else
            if(io_ptr->REMOTE_OPTIONS[TELOPT_ECHO])
            {
                #if PLATFORM_SDK_ENABLED
                ioctl(fileno(io_ptr->CONSOLE_FD), IOCTL_NIO_TTY_SET_FLAGS, 0);
                #else
                ioctl(fileno(io_ptr->CONSOLE_FD), IOCTL_NTTY_SET_FLAGS, 0);
                #endif
            }    
            #endif // MQX_USE_IO_OLD
            result = MQX_OK;
            break;

        case IO_IOCTL_SERIAL_GET_FLAGS:
            *(_mqx_uint_ptr)param_ptr = io_ptr->FLAGS;
            result = MQX_OK;
            break;

        case IO_IOCTL_SERIAL_SET_FLAGS:
            io_ptr->FLAGS = *(_mqx_uint_ptr)param_ptr;
            /* Handle defaults */

            /* Echo */
            if (!io_ptr->LOCAL_OPTIONS[TELOPT_ECHO] && io_ptr->REMOTE_OPTIONS[TELOPT_ECHO])
            {
                TELNET_dont(io_ptr->NETWORK_FD, TELOPT_ECHO);
            }

            if (io_ptr->FLAGS & IO_SERIAL_ECHO)
            {
                TELNET_will(io_ptr->NETWORK_FD, TELOPT_ECHO);
            }
            else
            {
                TELNET_dont(io_ptr->NETWORK_FD, TELOPT_ECHO);
            }

            /* Binary mode - local */
            if (!io_ptr->LOCAL_OPTIONS[TELOPT_BINARY])
            {
                TELNET_will(io_ptr->NETWORK_FD, TELOPT_BINARY);
            }
            /* Binary mode - remote */
            if (!io_ptr->REMOTE_OPTIONS[TELOPT_BINARY])
            {
                TELNET_do(io_ptr->NETWORK_FD, TELOPT_BINARY);
            }
            result = MQX_OK;
            break;

        case IO_IOCTL_SET_ECHO:
            break;

        case IO_IOCTL_GET_ECHO:
            break;

        case IO_IOCTL_SET_INBINARY:
            if (!io_ptr->LOCAL_OPTIONS[TELOPT_BINARY])
            { //inbinary
                TELNET_will(io_ptr->NETWORK_FD, TELOPT_BINARY);
                *(bool *)param_ptr = TRUE;
            }
            else
            {
                *(bool *)param_ptr = FALSE;
            }
            result = MQX_OK;
            break;

        case IO_IOCTL_GET_INBINARY:
            *(bool *)param_ptr = io_ptr->LOCAL_OPTIONS[TELOPT_BINARY];
            result = MQX_OK;
            break;

        case IO_IOCTL_SET_OUTBINARY:
            if (!io_ptr->REMOTE_OPTIONS[TELOPT_BINARY])
            { //outbinary
                TELNET_do(io_ptr->NETWORK_FD, TELOPT_BINARY);
                *(bool *)param_ptr = TRUE;
            }
            else
            {
                *(bool *)param_ptr = FALSE;
            }
            result = MQX_OK;
            break;

        case IO_IOCTL_GET_OUTBINARY:
            *(bool *)param_ptr = io_ptr->REMOTE_OPTIONS[TELOPT_BINARY];
            result = MQX_OK;
            break;

        case IO_IOCTL_SET_BINARY:
            *(bool *)param_ptr = FALSE;
            if (!io_ptr->LOCAL_OPTIONS[TELOPT_BINARY])
            { //inbinary
                TELNET_will(io_ptr->NETWORK_FD, TELOPT_BINARY);
                *(bool *)param_ptr = TRUE;
            }
            if (!io_ptr->REMOTE_OPTIONS[TELOPT_BINARY])
            { //outbinary
                TELNET_do(io_ptr->NETWORK_FD, TELOPT_BINARY);
                *(bool *)param_ptr = TRUE;
            }
            result = MQX_OK;
            break;

        case IO_IOCTL_GET_BINARY:
            *(bool *)param_ptr = io_ptr->LOCAL_OPTIONS[TELOPT_BINARY] && io_ptr->REMOTE_OPTIONS[TELOPT_BINARY];
            result = MQX_OK;
            break;

        case IO_IOCTL_SET_CRLF:
            if (*(bool *)param_ptr)
            {
                io_ptr->TERMINAL_OPTIONS |= TEROPT_CRLF_MODE;
            }
            else
            {
                io_ptr->TERMINAL_OPTIONS &= ~TEROPT_CRLF_MODE;
            }
            result = MQX_OK;
            break;

        case IO_IOCTL_GET_CRLF:
            *(bool *)param_ptr = (io_ptr->TERMINAL_OPTIONS & TEROPT_CRLF_MODE) > 0;
            result = MQX_OK;
            break;

        case IO_IOCTL_SET_RAW:
            if (*(bool *)param_ptr)
            {
                io_ptr->TERMINAL_OPTIONS |= TEROPT_RAW_MODE;
            }
            else
            {
                io_ptr->TERMINAL_OPTIONS &= ~TEROPT_RAW_MODE;
            }
            result = MQX_OK;
            break;

        case IO_IOCTL_GET_RAW:
            *(bool *)param_ptr = (io_ptr->TERMINAL_OPTIONS & TEROPT_RAW_MODE) > 0;
            result = MQX_OK;
            break;

        case IO_IOCTL_FLUSH_OUTPUT:
            #if MQX_USE_IO_OLD
            result = ioctl(io_ptr->NETWORK_FD, IO_IOCTL_FLUSH_OUTPUT, param_ptr);
            #else
            result = ioctl(fileno(io_ptr->NETWORK_FD), IO_IOCTL_FLUSH_OUTPUT, param_ptr);
            #endif
            break;
    }
    
    if (do_fflush)
    {
        fflush(io_ptr->NETWORK_FD);
    }
   
    return (_mqx_int)result;
}


/*FUNCTION*-------------------------------------------------------------
*
* Function Name  : _io_telnet_echo
* Returned Value : TRUE when character was successfully echoed, FALSE otherwise
* Comments       : Echoes given character if server echo is set
*
*
*END*-----------------------------------------------------------------*/
bool _io_telnet_echo(IO_TELNET_PTR io_ptr, char c)
{
    //echoing received character
    if (io_ptr->LOCAL_OPTIONS[TELOPT_ECHO])
    {
        if (c == '\b')
        {
            fputs("\b ", io_ptr->NETWORK_FD);
        }
        if (fputc(c, io_ptr->NETWORK_FD) != c)
        {
            return FALSE;
        }
        fflush(io_ptr->NETWORK_FD);
        return TRUE;
    }
    return FALSE;
}


/*FUNCTION*-------------------------------------------------------------
*
* Function Name  : _io_telnet_process_char
* Returned Value : IO_ERROR, TRUE or FALSE
* Comments       : process the arrived characters, including negotiated options
*       TRUE is returned when valid ASCII character has been processed.
*       FALSE is returned for either IAC, commands or CR which should be combined
*       with next character.
*       IO_ERROR is returned when received bytes doesn't follow RFC 854, i.e. CR
*       followed neither by LF nor by NULL.
*
*END*-----------------------------------------------------------------*/
#if MQX_USE_IO_OLD
_mqx_int _io_telnet_process_char(char *cp, MQX_FILE_PTR fd_ptr)
{
    IO_TELNET_PTR  io_ptr;
    MQX_FILE_PTR   console_stream;

    io_ptr = fd_ptr->DEV_DATA_PTR;   
#else
_mqx_int _io_telnet_process_char (char *cp, void *fp_context)
{
    IO_TELNET * io_ptr = (IO_TELNET *)fp_context;
    FILE * console_stream;
#endif
    char           c = *cp;
    _mqx_uint      flags;
    console_stream = io_ptr->CONSOLE_FD;

    switch (io_ptr->STATE)
    {
        case 0: /* Start state */
            //receiving data in real binary mode so avoiding workarounds
            //in handling EOLs (different CR LF combinations) valid only for character based terminals
            if (RAW_MODE(io_ptr) && io_ptr->LOCAL_OPTIONS[TELOPT_BINARY])
            {
                switch (c)
                {
                    case TELCMD_IAC:
                        io_ptr->STATE = 1;
                        return FALSE;
                    default:
                        if (io_ptr->REMOTE_OPTIONS[TELOPT_BINARY] || !(c & 0x80))
                        {
                            _io_telnet_echo(io_ptr, c);
                            return TRUE;
                        }
                        else
                        {
                            return FALSE; //filter character
                        }
                }
            }

            if (io_ptr->LOCAL_OPTIONS[TELOPT_BINARY])
            { //receiving in binary mode
                switch (c)
                {
                    case TELCMD_IAC:
                        io_ptr->STATE = 1;
                        return FALSE;
                    case TELCC_CR:
                        _io_telnet_echo(io_ptr, c);
                        _io_telnet_echo(io_ptr, TELCC_LF);
                        *cp = TELCC_LF; //application character mapping to LF
                        return TRUE;
                    default:
                        if (io_ptr->REMOTE_OPTIONS[TELOPT_BINARY])
                        {
                            //strip extra LF because in binary mode each CR is echoed as CR LF
                            if (!CRLF_MODE(io_ptr) || (CRLF_MODE(io_ptr) && (c != TELCC_LF)))
                            {
                                _io_telnet_echo(io_ptr, c);
                            }
                            else
                            {
                                return FALSE;
                            }
                        }
                        else if (c & 80)
                        { //remote end is in ASCII mode but we have illegal data to send
                            return FALSE;
                        }
                        else
                        {
                            _io_telnet_echo(io_ptr, c);
                        }
                        return TRUE;
                }
            }
            else
            { //ASCII mode
                switch (c)
                {
                    case TELCMD_IAC:
                        io_ptr->STATE = 1;
                        return FALSE;
                    case TELCC_CR:
                        _io_telnet_echo(io_ptr, c);
                        io_ptr->STATE = 6;
                        return FALSE;
                    default:
                        if (c & 0x80)
                        { //in ASCII only USASCII codes could be send
                            return FALSE;
                        }
                        else
                        {
                            _io_telnet_echo(io_ptr, c);
                            return TRUE;
                        }
                }
            }

        case 1: /* IAC */
            switch (c)
            {
                case TELCMD_AYT:
                    /*
                     * Request to send a string to prove we're alive.
                     * Send back comforting message.
                     */
                    #if MQX_USE_IO_OLD
                    fputs("\nRTCS Telnet client online.\n", fd_ptr);
                    #else
                    {
                        register KERNEL_DATA_STRUCT_PTR  kernel_data;
                        register TD_STRUCT_PTR           active_ptr;
                        _GET_KERNEL_DATA(kernel_data);
                        active_ptr = kernel_data->ACTIVE_PTR;
                        fputs("\nRTCS Telnet client online.\n", active_ptr->STDOUT_STREAM);
                    }
                    #endif
                    io_ptr->STATE = 0;
                    return FALSE;

                case TELCMD_EC:
                    /*
                     * Request to erase the last character before this command.
                     */
                    *cp = '\b';
                    io_ptr->STATE = 0;
                    return TRUE;

                case TELCMD_WILL:
                    io_ptr->STATE = 2;
                    return FALSE;

                case TELCMD_DO:
                    io_ptr->STATE = 3;
                    return FALSE;

                case TELCMD_WONT:
                    io_ptr->STATE = 4;
                    return FALSE;

                case TELCMD_DONT:
                    io_ptr->STATE = 5;
                    return FALSE;

                case TELCMD_IAC:
                    /* Second IAC is not interpreted as command, it's data */
                    if (io_ptr->LOCAL_OPTIONS[TELOPT_BINARY] && io_ptr->REMOTE_OPTIONS[TELOPT_BINARY])
                    { //BINARY mode
                        _io_telnet_echo(io_ptr, TELCMD_IAC);
                        _io_telnet_echo(io_ptr, TELCMD_IAC);
                        io_ptr->STATE = 0;
                        return TRUE;
                    } //else filter it (continue with default state)

                default:
                    /* Ignore all other telnet commands (e.g GA) */
                    io_ptr->STATE = 0;
                    return FALSE;
            }

        case 2: /* IAC WILL */
            switch (c)
            {
                case TELOPT_ECHO:
                    if (!io_ptr->REMOTE_OPTIONS[TELOPT_ECHO])
                    {
                        if ((io_ptr->LOCAL_OPTIONS[TELOPT_ECHO]) || !(io_ptr->FLAGS & IO_SERIAL_ECHO))
                        {
                            TELNET_dont(io_ptr->NETWORK_FD, TELOPT_ECHO);
                        }
                        else
                        {
                            /* Clear the echo bit */
                            #if MQX_USE_IO_OLD
                            ioctl(console_stream, IO_IOCTL_SERIAL_GET_FLAGS, &flags);
                            flags &= ~IO_SERIAL_ECHO;
                            ioctl(console_stream, IO_IOCTL_SERIAL_SET_FLAGS, &flags);
                            #else
                                flags = 0;
                                #if PLATFORM_SDK_ENABLED
                                ioctl(fileno(console_stream), IOCTL_NIO_TTY_SET_FLAGS, flags);
                                #else
                                ioctl(fileno(console_stream), IOCTL_NTTY_SET_FLAGS, flags);
                                #endif
                            #endif
                            io_ptr->REMOTE_OPTIONS[TELOPT_ECHO] = TRUE;
                            TELNET_do(io_ptr->NETWORK_FD, TELOPT_ECHO);
                        }
                    }
                    else if (!(io_ptr->FLAGS & IO_SERIAL_ECHO))
                    {
                        TELNET_dont(io_ptr->NETWORK_FD, TELOPT_ECHO);
                    }
                    break;

                case TELOPT_SGA:
                case TELOPT_BINARY:
                    /* ACK only */
          			if (!io_ptr->REMOTE_OPTIONS[(int) c])
                    {
             			io_ptr->REMOTE_OPTIONS[(int) c] = TRUE;
                        TELNET_do(io_ptr->NETWORK_FD, c);
                    } /* Endif */
                    break;

                default:
                    TELNET_dont(io_ptr->NETWORK_FD, c);
                    break;
            }

            io_ptr->STATE = 0;
            return FALSE;

        case 3: /* IAC DO */
            switch (c)
            {
                case TELOPT_ECHO:
                    /* ACK only */
                    if (!io_ptr->LOCAL_OPTIONS[TELOPT_ECHO])
                    {
                        if (io_ptr->REMOTE_OPTIONS[TELOPT_ECHO])
                        {
                            if (io_ptr->FLAGS & IO_SERIAL_ECHO)
                            {
                                TELNET_wont(io_ptr->NETWORK_FD, TELOPT_ECHO);
                            }
                            else
                            {
                                TELNET_dont(io_ptr->NETWORK_FD, TELOPT_ECHO);
                            }
                        }
                        else
                        {
                            if (io_ptr->FLAGS & IO_SERIAL_ECHO)
                            {
                                io_ptr->LOCAL_OPTIONS[TELOPT_ECHO] = TRUE;
                                TELNET_will(io_ptr->NETWORK_FD, TELOPT_ECHO);
                            }
                            else
                            {
                                TELNET_dont(io_ptr->NETWORK_FD, TELOPT_ECHO);
                            }
                        }
                    }
                    break;

                case TELOPT_SGA:
                case TELOPT_BINARY:
                        /* ACK only */
          				if (!io_ptr->LOCAL_OPTIONS[(unsigned int) c])
                        {
             				io_ptr->LOCAL_OPTIONS[(unsigned int) c] = TRUE;
                            TELNET_will(io_ptr->NETWORK_FD, c);
                        }
                        break;

                default:
                    TELNET_wont(io_ptr->NETWORK_FD, c);
                    break;
            }

            io_ptr->STATE = 0;
            return FALSE;

        case 4: /* IAC WONT */
            switch (c)
            {
                case TELOPT_ECHO:
                    if (io_ptr->REMOTE_OPTIONS[TELOPT_ECHO])
                    {
                        if (io_ptr->FLAGS & IO_SERIAL_ECHO)
                        {
                            /* Set the echo bit */
                            #if MQX_USE_IO_OLD
                            ioctl(console_stream, IO_IOCTL_SERIAL_GET_FLAGS, &flags);
                            flags |= IO_SERIAL_ECHO;
                            ioctl(console_stream, IO_IOCTL_SERIAL_SET_FLAGS, &flags);
                            #else
                                #if PLATFORM_SDK_ENABLED
                                flags = NIO_TTY_FLAGS_EOL_RN | NIO_TTY_FLAGS_ECHO;
                                ioctl(fileno(io_ptr->CONSOLE_FD), IOCTL_NIO_TTY_SET_FLAGS, flags);
                                #else
                                flags = NTTY_FLAGS_EOL_CRLF | NTTY_FLAGS_ECHO;
                                ioctl(fileno(io_ptr->CONSOLE_FD), IOCTL_NTTY_SET_FLAGS, flags);
                                #endif
                            #endif
                    }
                    io_ptr->REMOTE_OPTIONS[TELOPT_ECHO] = FALSE;
                    TELNET_dont(io_ptr->NETWORK_FD, TELOPT_ECHO);
                    }
                    break;

                case TELOPT_BINARY:
                default:
                    if (c < TELNET_MAX_OPTS)
                    {
                        /* ACK only */
            			if (io_ptr->REMOTE_OPTIONS[(unsigned int) c])
                        {
                            TELNET_dont(io_ptr->NETWORK_FD, c);
               				io_ptr->REMOTE_OPTIONS[(int) c] = FALSE;
                        }
                    }
                    break;
            }

            io_ptr->STATE = 0;
            return FALSE;

        case 5: /* IAC DONT */
            if (c < TELNET_MAX_OPTS)
            {
                /* ACK only */
         		if (io_ptr->LOCAL_OPTIONS[(unsigned int)c])
                {
            		io_ptr->LOCAL_OPTIONS[(unsigned int)c] = FALSE;
                    TELNET_wont(io_ptr->NETWORK_FD, c);
                }
            }

            io_ptr->STATE = 0;
            return FALSE;

        case 6: /* CR received in ASCII mode */
            switch (c)
            {
                case TELCC_NULL:
                    if (CRLF_MODE(io_ptr))
                    {
                        *cp = TELCC_CR; //application character mapping to CR
                    }
                    else
                    {
                        *cp = TELCC_LF; //application character mapping to LF
                    }
                    _io_telnet_echo(io_ptr, *cp);
                    break;
                case TELCC_LF:
                    _io_telnet_echo(io_ptr, c);
                    *cp = TELCC_LF; //application character mapping to LF
                    break;
                default:
                    return IO_ERROR;
            }
            io_ptr->STATE = 0;
            return TRUE;
    }
    /* Will never reach here, this is just for compiler warning. */
    return FALSE;
}

#if !MQX_USE_IO_OLD
static int _io_telnet_init(void *init_data, void **dev_context, int *error)
{  
   (*dev_context) = NULL; /* dummy value - not used */

   return 0;
}

static int _io_telnet_deinit(void *dev_context, int *error)
{
  //IO_TELNET *devc = dev_context;
  
  //_mem_free(dev_context);
  return 0;
}
#endif //!MQX_USE_IO_OLD

