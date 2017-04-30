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
*   This file contains the Telnet client implementation.
*
*
*END************************************************************************/

#include <rtcs.h>
#include <ctype.h>
#include <telnetcln_prv.h>

#if !MQX_USE_IO_OLD
#define IO_EOF (-1)
#endif

/*
 * Task for reading from socket and sending data to output file descriptor.
 */
void telnetcln_in_task(void * v_context, void * creator)
{
    TELNETCLN_CONTEXT *context;
   
    context = (TELNETCLN_CONTEXT *) v_context;
    context->rx_tid = _task_get_id();
    RTCS_task_resume_creator(creator, RTCS_OK);
    _task_block();

    while(context->valid == TELNETCLN_CONTEXT_VALID)
    {
        bool b_result = TRUE;
        #if MQX_USE_IO_OLD  
        /* old IO (fio.h) fstatus() */
        b_result = fstatus(context->telnetfd);
        #endif
        if(b_result && context->tx_tid)
        {
            int32_t c;

            c = (int32_t) fgetc(context->telnetfd);
            if(c == IO_EOF) 
            {
                break;
            }
            fputc(c & 0x7F, context->params.fd_out);
        }
        else
        {
            /* 
             * this executes only for old IO. 
             * for NIO we expect fgetc() is blocking
             */
             if (context->tx_tid == 0)
             {
                context->params.callbacks.on_disconnected(context->params.callbacks.param);
                telnetcln_cleanup(context);
                _mem_free(context);
                return;
            }
            RTCS_time_delay(10);
        }
    }
    context->rx_tid = 0;
}

/*
 * Task for reading from input and sending data through socket.
 */
void telnetcln_out_task(void * v_context, void * creator)
{
    TELNETCLN_CONTEXT *context;
    
    context = (TELNETCLN_CONTEXT *) v_context;
    context->tx_tid = _task_get_id();
    RTCS_task_resume_creator(creator, RTCS_OK);

    while(context->valid == TELNETCLN_CONTEXT_VALID)
    {
        bool b_result = TRUE;
        #if MQX_USE_IO_OLD  
        /* 
         * old IO (fio.h) input might be polled UART driver
         * thus only for this case we check if a character is available.
         * is there is no character, sleep for a tick
         */
        b_result = fstatus(context->params.fd_in);
        #endif
        if (b_result && context->rx_tid)
        {
            int32_t c;
            c = (int32_t) fgetc(context->params.fd_in);
            #if !MQX_USE_IO_OLD
            if (EOF == c)
            {
                clearerr(context->params.fd_in);
                break;
            }
            #endif
            if (fputc(c & 0x7F, context->telnetfd) == IO_EOF)  
            {
                break;   
            }
        }
        else
        {
            /* 
             * this executes only for old IO uart driver. 
             * for NIO tty, fgetc() above is blocking function.
             */
             if (context->rx_tid == 0)
             {
                context->params.callbacks.on_disconnected(context->params.callbacks.param);
                telnetcln_cleanup(context);
                _mem_free(context);
                return;
            }
            RTCS_time_delay(10);
        }
    }
    context->tx_tid = 0;
}
