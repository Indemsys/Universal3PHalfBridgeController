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
*   This file contains the code which implements a PCB driver
*   that sends MQX format packets over an asynchronous serial driver.
*
*
*END************************************************************************/

#include "mqx_inc.h"

#if MQX_USE_IPC

#include "fio.h"
#include "io.h"
#include "io_prv.h"
#include "serial.h"
#include "io_pcb.h"
#include "iopcbprv.h"
#include "pcb_mqxa.h"
#include "pcbmqxav.h"


/*FUNCTION*-----------------------------------------------------------------
*
* Function Name   : _io_pcb_mqxa_install
* Returned Value  : _mqx_uint result   MQX_OK or an error code.
* Comments        :
*     This function is use to install the I/O device.
*
*END*---------------------------------------------------------------------*/

_mqx_uint _io_pcb_mqxa_install
    (
        /* [IN] the name of this device */
        char    *device_name_ptr,

        /* [IN] the initialization record for this device */
        void       *init_ptr
    )
{
    IO_PCB_MQXA_INFO_STRUCT_PTR info_ptr;

    info_ptr = _mem_alloc_system_zero(sizeof(IO_PCB_MQXA_INFO_STRUCT));
    if (info_ptr == NULL) {
        return(MQX_OUT_OF_MEMORY);
    }
    info_ptr->INIT = *((IO_PCB_MQXA_INIT_STRUCT_PTR)init_ptr);

    return (_io_dev_install_ext(device_name_ptr,
        _io_pcb_mqxa_open,
        _io_pcb_mqxa_close,
        _io_pcb_mqxa_read,
        _io_pcb_mqxa_write, 
        _io_pcb_mqxa_ioctl,
        _io_pcb_mqxa_uninstall,
        (void *)info_ptr)
    ); 
}


/*FUNCTION*-----------------------------------------------------------------
*
* Function Name   : _io_pcb_mqxa_open
* Returned Value  : _mqx_int result MQX_OK or an error code.
* Comments        :
*
*END*---------------------------------------------------------------------*/

_mqx_int _io_pcb_mqxa_open
    (
        /* [IN] the file handle */
        FILE_DEVICE_STRUCT_PTR fd_ptr, 

        /* [IN] the rest of the filename used to open the device */
        char              *open_name_ptr,

        /* [IN] the open flags for this device */
        char              *open_mode_flags
    )
{
    (void)open_name_ptr; /* disable 'unused variable' warning */
    (void)open_mode_flags; /* disable 'unused variable' warning */
    IO_PCB_MQXA_INFO_STRUCT_PTR info_ptr;
    IO_DEVICE_STRUCT_PTR    dev_ptr;
    _mqx_uint                   flags;

    dev_ptr = (IO_DEVICE_STRUCT_PTR)fd_ptr->DEV_PTR;
    fd_ptr->DEV_DATA_PTR = dev_ptr->DRIVER_INIT_PTR;
    info_ptr = (IO_PCB_MQXA_INFO_STRUCT_PTR)fd_ptr->DEV_DATA_PTR;
    if (info_ptr->FD) {
        fd_ptr->ERROR = IO_PCB_MQXA_DEVICE_ALREADY_OPEN;
        return(IO_ERROR);
    }
    info_ptr->FD = fopen(info_ptr->INIT.IO_PORT_NAME, (char *)0);
    if (info_ptr->FD == NULL) {
        fd_ptr->ERROR = IO_PCB_MQXA_INCORRECT_SERIAL_DEVICE;
        return(IO_ERROR);
    }

   /* Set the baud rate on the serial device, if a non zero value for baud is supplied. Otherwise, used default rate */
    if (info_ptr->INIT.BAUD_RATE) {
        if (ioctl(info_ptr->FD, IO_IOCTL_SERIAL_SET_BAUD, (void *)&info_ptr->INIT.BAUD_RATE) != MQX_OK)
        {
            fclose(info_ptr->FD);
            info_ptr->FD = NULL;
            fd_ptr->ERROR = IO_PCB_MQXA_INCORRECT_SERIAL_DEVICE;
            return(IO_ERROR);
        }
    }

    /* Turn off xon/xoff, translation and echo on the serial device */
    ioctl(info_ptr->FD, IO_IOCTL_SERIAL_GET_FLAGS, (void *)&flags);
    flags &= ~(IO_SERIAL_XON_XOFF | IO_SERIAL_TRANSLATION | IO_SERIAL_ECHO);
    ioctl(info_ptr->FD, IO_IOCTL_SERIAL_SET_FLAGS, &flags);

    fd_ptr->DEV_DATA_PTR = info_ptr;
    fd_ptr->FLAGS       |= IO_FLAG_IS_PCB_DEVICE;

    info_ptr->READ_CALLBACK_FUNCTION = NULL;
    _lwsem_create(&info_ptr->READ_LWSEM, 0);
    _lwsem_create(&info_ptr->WRITE_LWSEM, 0);
    _queue_init(&info_ptr->READ_QUEUE, 0);
    _queue_init(&info_ptr->WRITE_QUEUE, 0);

    return(MQX_OK);
}


/*FUNCTION*-----------------------------------------------------------------
*
* Function Name   : _io_pcb_mqxa_close
* Returned Value  : _mqx_int result   MQX_OK or an error code.
* Comments        :
*
*END*---------------------------------------------------------------------*/

_mqx_int _io_pcb_mqxa_close
    (
        /* [IN] the file handle */
        FILE_DEVICE_STRUCT_PTR fd_ptr
    )
{
    IO_PCB_MQXA_INFO_STRUCT_PTR info_ptr;
    IO_PCB_STRUCT_PTR           pcb_ptr;

#if MQX_CHECK_ERRORS
    if (!(fd_ptr->FLAGS & IO_FLAG_IS_PCB_DEVICE)) {
        fd_ptr->ERROR = IO_PCB_NOT_A_PCB_DEVICE;
        return(IO_ERROR);
    }
#endif
    info_ptr = (IO_PCB_MQXA_INFO_STRUCT_PTR)fd_ptr->DEV_DATA_PTR;
    if (info_ptr->FD) {
        fclose(info_ptr->FD);
        info_ptr->FD = NULL;
    }
    if (info_ptr->INPUT_TASK) {
#if MQX_TASK_DESTRUCTION   
        _task_destroy(info_ptr->INPUT_TASK);
#endif      
        info_ptr->INPUT_TASK = 0;
    }
    if (info_ptr->OUTPUT_TASK ) {
#if MQX_TASK_DESTRUCTION   
        _task_destroy(info_ptr->OUTPUT_TASK);
#endif
        info_ptr->OUTPUT_TASK = 0;
    }
    _lwsem_destroy(&info_ptr->READ_LWSEM);
    _lwsem_destroy(&info_ptr->WRITE_LWSEM);

    while (_queue_get_size(&info_ptr->WRITE_QUEUE)) {
        pcb_ptr = (IO_PCB_STRUCT_PTR)((void *)_queue_dequeue(&info_ptr->WRITE_QUEUE));
        _io_pcb_free(pcb_ptr);
    }

    while (_queue_get_size(&info_ptr->READ_QUEUE)) {
        pcb_ptr = (IO_PCB_STRUCT_PTR)((void *)_queue_dequeue(&info_ptr->READ_QUEUE));
        _io_pcb_free(pcb_ptr);
    }

    return(MQX_OK);
}


/*READ*---------------------------------------------------------------------
*
* Function Name   : _io_pcb_mqxa_read
* Returned Value  : _mqx_int result MQX_OK or an error code.
* Comments        :
*     This function reads a packet directly 
*
*END*---------------------------------------------------------------------*/

_mqx_int _io_pcb_mqxa_read
    (
        /* [IN] the file descriptor */
        FILE_DEVICE_STRUCT_PTR  fd_ptr,

        /* [IN] the pcb address from which to read data */
        char            *data_ptr,

        /* [IN] the number of characters to input */
        _mqx_int        num
    )
{
    IO_PCB_MQXA_INFO_STRUCT_PTR info_ptr = fd_ptr->DEV_DATA_PTR;;
    IO_PCB_STRUCT_PTR *pcb_ptr = (IO_PCB_STRUCT_PTR*)data_ptr;

    if (info_ptr->FD) {
        _int_disable();
        /* 
        ** Start CR 383
        ** if (info_ptr->FD->FLAGS & IO_O_NONBLOCK) {
        */
        if (fd_ptr->FLAGS & IO_O_NONBLOCK) {
            if (! _queue_get_size(&info_ptr->READ_QUEUE)) {
                *pcb_ptr = NULL;
                _int_enable();
                return MQX_OK;
            }
        }
        _lwsem_wait(&info_ptr->READ_LWSEM);
        *pcb_ptr = (IO_PCB_STRUCT_PTR)((void *)_queue_dequeue(&info_ptr->READ_QUEUE));
        _int_enable();
        return MQX_OK;
    }
    return(IO_ERROR);
}


/*FUNCTION*-----------------------------------------------------------------
*
* Function Name   : _io_pcb_mqxa_write
* Returned Value  : _mqx_uint result   MQX_OK or an error code.
* Comments        :
*     This function sends the pcb to the pcb output task.
*
*END*---------------------------------------------------------------------*/

_mqx_int _io_pcb_mqxa_write
    (
        /* [IN] the file descriptor */
        FILE_DEVICE_STRUCT_PTR  fd_ptr,

        /* [IN] the pcb address from which to write data */
        char            *data_ptr,

        /* [IN] the number of characters to input */
        _mqx_int        num
    )
{
    IO_PCB_MQXA_INFO_STRUCT_PTR info_ptr = fd_ptr->DEV_DATA_PTR;
    IO_PCB_STRUCT_PTR pcb_ptr = (IO_PCB_STRUCT_PTR)data_ptr;

    if (info_ptr->FD) {
        _queue_enqueue((QUEUE_STRUCT_PTR)(
            (void *)&info_ptr->WRITE_QUEUE), (QUEUE_ELEMENT_STRUCT_PTR)((void *)&pcb_ptr->QUEUE)
        );
        _lwsem_post(&info_ptr->WRITE_LWSEM);
        return MQX_OK;
    }
    return(IO_ERROR);
}


/*TASK*---------------------------------------------------------------------
*
* Task Name       : _io_pcb_mqxa_read_task
* Comments        :
*     This task reads a packet from the async serial device.
*  The packet is in MQX Asynchronous Packet format.
*
*END*---------------------------------------------------------------------*/

void _io_pcb_mqxa_read_task
    (
        /* [IN] the device info */
        uint32_t parameter
    )
{
    IO_PCB_MQXA_INFO_STRUCT_PTR info_ptr;
    IO_PCB_STRUCT_PTR           pcb_ptr;
    unsigned char               *input_ptr = NULL;
    unsigned char               *input_init_ptr;
    bool                        got_length = 0;
    _mem_size                   input_length = 0;
    _mem_size                   max_length = 0;
    _mem_size                   count = 0;
    _mqx_uint                   state = 0;
    _mqx_uint                   next_state = 0;
    unsigned char               crc0 = 0;
    unsigned char               crc1 = 0;
    unsigned char               packet_crc0 = 0;
    unsigned char               packet_crc1 = 0;
    unsigned char               tmp;
    unsigned char               c;

    info_ptr = (IO_PCB_MQXA_INFO_STRUCT_PTR)parameter;

    /* Get a PCB */
    pcb_ptr = _io_pcb_alloc(info_ptr->READ_PCB_POOL, FALSE);
#if MQX_CHECK_ERRORS
    if (pcb_ptr == NULL) {
        _task_block();
    }
#endif
    max_length     = info_ptr->INIT.INPUT_MAX_LENGTH;
    input_init_ptr = pcb_ptr->FRAGMENTS[0].FRAGMENT;

    state      = AP_STATE_SYNC; /* Waiting for sync */
    next_state = AP_STATE_SYNC; /* Waiting for sync */

    while (TRUE) {
        if (info_ptr->INIT.IS_POLLED) {
            while (!fstatus(info_ptr->FD)) {
                _time_delay_ticks(1);
            }
        }
        c = (unsigned char)fgetc(info_ptr->FD);

        switch (state) {

            case AP_STATE_SYNC:
                if (c == AP_SYNC) {
                    /* Sync detected. Start packet reception. */
                    state      = AP_STATE_READING;
                    next_state = AP_STATE_SYNC;
                    count      = 0;
                    input_ptr  = input_init_ptr;
                    crc0       = 0x7e;
                    crc1       = 0x7e;
                    got_length = FALSE;
                }
                break;

            case AP_STATE_SYNC_SKIP:
                if (c != AP_SYNC) {
                    /* Single sync detected. Restart message reception. */
                    count      = 0;
                    input_ptr  = input_init_ptr;
                    crc0       = 0x7e;
                    crc1       = 0x7e;
                    got_length = FALSE;
                    *input_ptr++ = c;
                    ++count;
                    AP_CHECKSUM(c, crc0, crc1);
                    state = AP_STATE_READING;
                } else {
                    state = next_state;
                }
                break;

            case AP_STATE_READING:
                *input_ptr++ = c;
                ++count;
                AP_CHECKSUM(c, crc0, crc1);

                if (got_length ) {
                    if (count >= input_length){
                        state = AP_STATE_CS0;
                    }
                } else {
                    if ( count > MQXA_MSG_CONTROL_OFFSET) {
                        /* The complete packet header has been read in */
                        input_length = (_mem_size)(GET_LENGTH(input_init_ptr));
                        if (input_length > max_length) {
                            next_state = AP_STATE_SYNC;
                            ++info_ptr->RX_PACKETS_TOO_LONG;
                        } else {
                            got_length = TRUE;
                            if (count >= input_length) {
                                state = AP_STATE_CS0;
                            }
                        }
                    }
                }

                if (c == AP_SYNC) {
                    next_state = state;
                    state      = AP_STATE_SYNC_SKIP;
                }
                break;

            case AP_STATE_CS0:
                packet_crc0 = c;
                state = AP_STATE_CS1;
                if (c == AP_SYNC) {
                    next_state  = state;
                    state       = AP_STATE_SYNC_SKIP;
                }
                break;

            case AP_STATE_CS1:
                packet_crc1 = c;
                state = AP_STATE_DONE;
                if (c == AP_SYNC) {
                    next_state  = state;
                    state       = AP_STATE_SYNC_SKIP;
                }
                break;

            default:
                state = AP_STATE_SYNC;
                break;
        }

        if ( state == AP_STATE_DONE ) {
            /* Calculate the CRCs */
            crc1  = (crc1 + 2 * crc0) & 0xFF;
            tmp   = crc0 - crc1;
            crc1  = (crc1 - (crc0 * 2)) & 0xFF;
            crc0  = tmp & 0xFF;

            if ((crc0 == packet_crc0) && (crc1 == packet_crc1)) {
                ++info_ptr->RX_PACKETS;
                pcb_ptr->FRAGMENTS[0].LENGTH = input_length;
                if (info_ptr->READ_CALLBACK_FUNCTION) {
                    (*info_ptr->READ_CALLBACK_FUNCTION)(info_ptr->CALLBACK_FD, pcb_ptr);
                } else {
                    _queue_enqueue((QUEUE_STRUCT_PTR)&info_ptr->READ_QUEUE,
                    (QUEUE_ELEMENT_STRUCT_PTR)&pcb_ptr->QUEUE);
                    _lwsem_post(&info_ptr->READ_LWSEM);
                }
                pcb_ptr = _io_pcb_alloc(info_ptr->READ_PCB_POOL, TRUE);
                if (pcb_ptr == NULL) {
                    while (pcb_ptr == NULL) {
                        _time_delay_ticks(2);
                        pcb_ptr = _io_pcb_alloc(info_ptr->READ_PCB_POOL, TRUE);
                    }
                }
                input_init_ptr = pcb_ptr->FRAGMENTS[0].FRAGMENT;
            } else {
                ++info_ptr->RX_PACKETS_BAD_CRC;
            }
            state = AP_STATE_SYNC;
        }
    }
}


/*TASK*---------------------------------------------------------------------
*
* Task Name       : _io_pcb_mqxa_write_task
* Comments        :
*     This task writes a packet to the async serial device.
*  The packet is in MQX Asynchronous Packet format.
*
*END*---------------------------------------------------------------------*/

void _io_pcb_mqxa_write_task
   (
      /* [IN] the device info */
      uint32_t parameter
   )
{
    IO_PCB_MQXA_INFO_STRUCT_PTR     info_ptr;
    IO_PCB_STRUCT_PTR               pcb_ptr;
    MQX_FILE_PTR                    fd;
    char                            *output_ptr;
    _mem_size                       count;
    _mqx_uint                       i;
    unsigned char                   crc0;
    unsigned char                   crc1;
    unsigned char                   tmp;
    unsigned char                   c;

    info_ptr = (IO_PCB_MQXA_INFO_STRUCT_PTR)parameter;
    fd = info_ptr->FD;

    while (1) {
        _lwsem_wait(&info_ptr->WRITE_LWSEM);
        pcb_ptr = (IO_PCB_STRUCT_PTR)((void *)_queue_dequeue(&info_ptr->WRITE_QUEUE));

        crc0   = 0x7e;
        crc1   = 0x7e;
        fputc(AP_SYNC, fd);

        for (i = 0; i < pcb_ptr->NUMBER_OF_FRAGMENTS; i++) {
            count      = pcb_ptr->FRAGMENTS[i].LENGTH + 1;
            output_ptr = (char *)pcb_ptr->FRAGMENTS[i].FRAGMENT;
            while (--count) {
                c = *output_ptr++;
                fputc(c, fd);
                if ( c == AP_SYNC ) {
                    fputc(AP_SYNC, fd);
                }
                AP_CHECKSUM(c, crc0, crc1);
            }
        }

        /* Calculate the CRCs */
        crc1  = (crc1 + 2 * crc0) & 0xFF;
        tmp   = crc0 - crc1;
        crc1  = (crc1 - (crc0 * 2)) & 0xFF;
        crc0  = tmp  & 0xFF;

        fputc(crc0, fd);
        if ( crc0 == AP_SYNC ) {
            fputc(AP_SYNC, fd);
        }

        /* One more crc character to output */
        fputc(crc1, fd);
        if ( crc1 == AP_SYNC ) {
            fputc(AP_SYNC, fd);
        }

        info_ptr->TX_PACKETS++;
        _io_pcb_free(pcb_ptr);
    }
}


/*FUNCTION*-----------------------------------------------------------------
*
* Function Name   : _io_pcb_mqxa_ioctl
* Returned Value  : _mqx_uint result   MQX_OK or an error code.
* Comments        :
*
*END*---------------------------------------------------------------------*/

_mqx_int _io_pcb_mqxa_ioctl
    (
        /* [IN] the file handle for the device */
        FILE_DEVICE_STRUCT_PTR fd_ptr,

        /* [IN] the ioctl command */
        _mqx_uint              cmd,

        /* [IN] the ioctl parameters */
        void                  *param_ptr
    )
{
    TASK_TEMPLATE_STRUCT input_tt = 
        { 0, _io_pcb_mqxa_read_task,  IO_PCB_MQXA_STACK_SIZE, 0, "io_pcb_mqxa_read_task",  0, 0, 0};
    TASK_TEMPLATE_STRUCT output_tt = 
        { 0, _io_pcb_mqxa_write_task, IO_PCB_MQXA_STACK_SIZE, 0, "io_pcb_mqxa_write_task", 0, 0, 0};
    IO_PCB_MQXA_INFO_STRUCT_PTR info_ptr;
    IO_PCB_STRUCT_PTR           pcb_ptr;
    _mqx_uint                   result = MQX_OK;
    _psp_code_addr              old_value;
    _psp_code_addr_ptr          pc_ptr = (_psp_code_addr_ptr)param_ptr;
    _psp_data_addr_ptr          pd_ptr = (_psp_data_addr_ptr)param_ptr;
    bool                    *bool_param_ptr;

    info_ptr = (IO_PCB_MQXA_INFO_STRUCT_PTR)fd_ptr->DEV_DATA_PTR;

    switch (cmd) {

        case IO_PCB_IOCTL_ENQUEUE_READQ:
            pcb_ptr = (IO_PCB_STRUCT_PTR)*pd_ptr;
            _queue_enqueue((QUEUE_STRUCT_PTR)&info_ptr->READ_QUEUE, 
            (QUEUE_ELEMENT_STRUCT_PTR)&pcb_ptr->QUEUE);
            _lwsem_post(&info_ptr->READ_LWSEM);
            break;

        case IO_PCB_IOCTL_READ_CALLBACK_SET:
            old_value = (_psp_code_addr)info_ptr->READ_CALLBACK_FUNCTION;
            info_ptr->CALLBACK_FD = fd_ptr;
            info_ptr->READ_CALLBACK_FUNCTION = (void (_CODE_PTR_)(
            FILE_DEVICE_STRUCT_PTR, IO_PCB_STRUCT_PTR))*pc_ptr;
            *pc_ptr = old_value;
            break;
      
        case IO_PCB_IOCTL_SET_INPUT_POOL:
            old_value = (_psp_code_addr)info_ptr->READ_PCB_POOL;
            info_ptr->READ_PCB_POOL = (_io_pcb_pool_id)*pc_ptr;
            *pc_ptr = old_value;
            break;

        case IO_PCB_IOCTL_START:
            if (info_ptr->INPUT_TASK == MQX_NULL_TASK_ID) {
                input_tt.TASK_PRIORITY  = info_ptr->INIT.INPUT_TASK_PRIORITY;
                input_tt.CREATION_PARAMETER  = (uint32_t)info_ptr;

                output_tt.TASK_PRIORITY = info_ptr->INIT.OUTPUT_TASK_PRIORITY;
                output_tt.CREATION_PARAMETER = (uint32_t)info_ptr;
   
                info_ptr->INPUT_TASK = _task_create(0, 0, (uint32_t)&input_tt);
                if (info_ptr->INPUT_TASK == MQX_NULL_TASK_ID){
                   return((_mqx_int)_task_get_error());
                }

                info_ptr->OUTPUT_TASK = _task_create(0, 0, (uint32_t)&output_tt);
                if (info_ptr->OUTPUT_TASK == MQX_NULL_TASK_ID){
                   return((_mqx_int)_task_get_error());
                }
            }
            break;

        case IO_PCB_IOCTL_UNPACKED_ONLY:
            bool_param_ptr = (bool *)param_ptr;
            *bool_param_ptr = TRUE;
            break;

        default:
            result = (_mqx_uint)_io_ioctl(info_ptr->FD, cmd, param_ptr);
            break;

    }
    return (_mqx_int)result;
}


/*FUNCTION*-----------------------------------------------------------------
*
* Function Name   : _io_pcb_mqxa_uninstall
* Returned Value  : _mqx_uint result   MQX_OK or an error code.
* Comments        :
*
*END*---------------------------------------------------------------------*/

_mqx_int _io_pcb_mqxa_uninstall
    (
        IO_DEVICE_STRUCT_PTR dev_ptr
    )
{
    IO_PCB_MQXA_INFO_STRUCT_PTR info_ptr;
    _mqx_int                    result = MQX_OK;

    info_ptr = (IO_PCB_MQXA_INFO_STRUCT_PTR)dev_ptr->DRIVER_INIT_PTR;
    if (info_ptr->FD) {
        return IO_ERROR_DEVICE_BUSY;
    }
    _mem_free(info_ptr);

    return result;
}

#endif

