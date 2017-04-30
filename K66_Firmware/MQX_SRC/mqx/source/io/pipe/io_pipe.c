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
*   This file contains the driver functions for the Pipe device.
*
*
*END************************************************************************/

/* suppress warning if mutex component is not enabled in MQX */
#define MQX_DISABLE_CONFIG_CHECK 1

#include "mqx_inc.h"                         
#include "fio.h"
#include "fio_prv.h"
#include "io.h"
#include "io_prv.h"
#include "charq.h"
#include "mutex.h"
#include "pipe_prv.h"
#include "io_pipe.h"


/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : _io_pipe_install
* Returned Value   : uint32_t a task error code or MQX_OK
* Comments         :
*    Install an interrupt driven serial device.
*
*END*----------------------------------------------------------------------*/

_mqx_uint _io_pipe_install
    (
        /* [IN] A string that identifies the device for fopen */
        char            *identifier,

        /* [IN] The pipe queue size to use */
        uint32_t             queue_size,

        /* [IN] Currently not used */
        uint32_t             flags
    )
{
    IO_PIPE_INIT_STRUCT_PTR  io_pipe_init_ptr;

    /* Allocate an Pipe installation structure */
    io_pipe_init_ptr = _mem_alloc_system_zero((uint32_t)sizeof(IO_PIPE_INIT_STRUCT));

#if MQX_CHECK_MEMORY_ALLOCATION_ERRORS
    if (io_pipe_init_ptr == NULL) {
        return(MQX_OUT_OF_MEMORY);
    }
#endif

    /* Set queue size for Pipe */
    io_pipe_init_ptr->QUEUE_SIZE = queue_size;

    /* Copy flags field */
    io_pipe_init_ptr->FLAGS      = flags;

    /* Install device */
    return (_io_dev_install_ext(
        identifier, 
        _io_pipe_open,
        _io_pipe_close, 
        _io_pipe_read,
        _io_pipe_write, 
        _io_pipe_ioctl, 
        _io_pipe_uninstall, 
        (void *)io_pipe_init_ptr
    ));
}


/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : _io_pipe_uninstall
* Returned Value   : _mqx_int a task error code or MQX_OK
* Comments         :
*
*END*----------------------------------------------------------------------*/

_mqx_int _io_pipe_uninstall
    (
        /* [IN] The devices context */
        IO_DEVICE_STRUCT_PTR  io_dev_ptr
    )
{
    if (io_dev_ptr->DRIVER_INIT_PTR) {
        _mem_free(io_dev_ptr->DRIVER_INIT_PTR);
    }

    return(MQX_OK);

}


/*FUNCTION****************************************************************
* 
* Function Name    : _io_pipe_open
* Returned Value   : int32_t task error code or MQX_OK
* Comments         :
*    This routine opens an instance of a Pipe. It acquires
*    memory, then stores information into it about the Pipe.
*    This memory is returned as a FILE 'handle' to be used for 
*    all other pipe functions by any task. 
*
* 
*END**********************************************************************/

_mqx_int _io_pipe_open
    (
        /* [IN] the file handle for the device being opened */
        FILE_DEVICE_STRUCT_PTR    fd_ptr,

        /* [IN] the remaining portion of the name of the device */
        char                      *open_name_ptr,

        /* [IN] the flags to be used during operation:
        ** echo, translation, xon/xoff
        */
        char                      *flags
    )
{
    (void)open_name_ptr; /* disable 'unused variable' warning */
    (void)flags; /* disable 'unused variable' warning */
    IO_DEVICE_STRUCT_PTR        io_dev_ptr = fd_ptr->DEV_PTR;
    IO_PIPE_INIT_STRUCT_PTR     io_pipe_init_ptr;
    IO_PIPE_INFO_STRUCT_PTR     io_pipe_info_ptr;
    MUTEX_ATTR_STRUCT           mutex_attr;
    uint32_t                    result = MQX_OK;

    /* Allocate a Pipe information structure */
    io_pipe_info_ptr = _mem_alloc_system_zero((uint32_t)sizeof(IO_PIPE_INFO_STRUCT));

#if MQX_CHECK_MEMORY_ALLOCATION_ERRORS
    if (io_pipe_info_ptr == NULL){
        return(MQX_OUT_OF_MEMORY);
    }
#endif 

    fd_ptr->DEV_DATA_PTR = io_pipe_info_ptr;
    io_pipe_init_ptr = (IO_PIPE_INIT_STRUCT_PTR)io_dev_ptr->DRIVER_INIT_PTR;

    io_pipe_info_ptr->QUEUE_SIZE = io_pipe_init_ptr->QUEUE_SIZE;

    /* Initialize mutexes */
    result = _mutatr_init(&mutex_attr); 

#if MQX_CHECK_ERRORS
    if (result != MQX_EOK) {
        _mem_free(io_pipe_info_ptr);
        return (_mqx_int)(result);
    }
#endif 

    _mutatr_set_wait_protocol(&mutex_attr, MUTEX_PRIORITY_QUEUEING);
    _mutatr_set_sched_protocol(&mutex_attr, MUTEX_PRIO_INHERIT);

    result = _mutex_init(&io_pipe_info_ptr->READ_MUTEX, &mutex_attr);
#if MQX_CHECK_ERRORS
    if (result != MQX_EOK) {
        _mem_free(io_pipe_info_ptr);
        return (_mqx_int)(result);
    }
#endif 

    result = _mutex_init(&io_pipe_info_ptr->WRITE_MUTEX, &mutex_attr);
#if MQX_CHECK_ERRORS
    if (result != MQX_EOK) {
        _mem_free(io_pipe_info_ptr);
        return (_mqx_int)(result);
    }
#endif 

    result = _mutex_init(&io_pipe_info_ptr->ACCESS_MUTEX, &mutex_attr);
#if MQX_CHECK_ERRORS
    if (result != MQX_EOK) {
        _mem_free(io_pipe_info_ptr);
        return (_mqx_int)(result);
    }
#endif 

    /* Initialize semaphores */
    result = _lwsem_create(&io_pipe_info_ptr->FULL_SEM, 0);

#if MQX_CHECK_ERRORS
    if (result != MQX_OK) {
        _mem_free(io_pipe_info_ptr);
        return (_mqx_int)(result);
    }
#endif 

    result = _lwsem_create(&io_pipe_info_ptr->EMPTY_SEM, 0);
#if MQX_CHECK_ERRORS
    if (result != MQX_OK) {
        _mem_free(io_pipe_info_ptr);
        return (_mqx_int)(result);
    }
#endif 

    /* Allocate queue structure for pipe char queue */
    io_pipe_info_ptr->QUEUE = (void *)_mem_alloc_system(
        sizeof(CHARQ_STRUCT) - (4 * sizeof(char)) + io_pipe_info_ptr->QUEUE_SIZE
    );

#if MQX_CHECK_MEMORY_ALLOCATION_ERRORS
    if (io_pipe_info_ptr->QUEUE == NULL){
        _mem_free(io_pipe_info_ptr);
        return(MQX_OUT_OF_MEMORY);
    }
#endif            

    /* Initialize Pipe queue */
    _CHARQ_INIT(io_pipe_info_ptr->QUEUE, io_pipe_init_ptr->QUEUE_SIZE);

    return (_mqx_int)(result);
}


/*FUNCTION****************************************************************
* 
* Function Name    : _io_pipe_close
* Returned Value   : int32_t error code
* Comments         :
*    This routine closes the Pipe and free the datastructures.
* 
*END**********************************************************************/

_mqx_int _io_pipe_close
    (
        /* [IN] the file handle for the device being closed */
        FILE_DEVICE_STRUCT_PTR fd_ptr
    )
{
    IO_PIPE_INFO_STRUCT_PTR         io_pipe_info_ptr;
    int32_t                          result = MQX_OK;

    io_pipe_info_ptr = fd_ptr->DEV_DATA_PTR;

    /* Destroy the mutexes - doesn't free memory*/
    _mutex_destroy(&io_pipe_info_ptr->READ_MUTEX);
    _mutex_destroy(&io_pipe_info_ptr->WRITE_MUTEX);
    _mutex_destroy(&io_pipe_info_ptr->ACCESS_MUTEX);
                                 
    /* Derstroy the lightweight semaphores */
    _lwsem_destroy(&io_pipe_info_ptr->FULL_SEM);
    _lwsem_destroy(&io_pipe_info_ptr->EMPTY_SEM);

    /* Free Pipe data structures */
    _mem_free(io_pipe_info_ptr->QUEUE);
    _mem_free(io_pipe_info_ptr);

    return(result);
}


/*FUNCTION****************************************************************
* 
* Function Name    : _io_pipe_read
* Returned Value   : int32_t number of characters read
* Comments         :
*    This routine reads characters from the Pipe queue.
*    This function block a task if there are no characters in the
*    Pipe queue.
*
*END*********************************************************************/

_mqx_int _io_pipe_read
    (
        /* [IN] the handle returned from _fopen */
        FILE_DEVICE_STRUCT_PTR fd_ptr,

        /* [IN] where the characters are to be stored */
        char              *data_ptr,

        /* [IN] the number of characters to input */
        _mqx_int               num
    )
{
    volatile IO_PIPE_INFO_STRUCT_PTR        io_pipe_info_ptr;
    unsigned char                           c = 0;
    uint32_t                                i = (uint32_t)(num + 1);
    volatile CHARQ_STRUCT                   *pipe_queue;
    uint32_t                                error;

    io_pipe_info_ptr = fd_ptr->DEV_DATA_PTR;

    pipe_queue = io_pipe_info_ptr->QUEUE;

    /* Turn off pipe abort flags */
    io_pipe_info_ptr->KM_ABORT_READ = FALSE;

    /* Lock out other reading tasks */       
    error = _mutex_lock(&io_pipe_info_ptr->READ_MUTEX);

#if MQX_CHECK_ERRORS
    if ( error != MQX_EOK ) {
        return (-1);
    }
#endif

    /* Lock out access of a writer task to the pipe data structures */
    error = _mutex_lock(&io_pipe_info_ptr->ACCESS_MUTEX);
#if MQX_CHECK_ERRORS
    if ( error != MQX_EOK ) {
        return (-1);
    }
#endif   

    /* Start reading characters from the pipe*/
    while ( --i ) {

        /* If the pipe is empty, wait until the pipe is not empty */                            
        while (_CHARQ_EMPTY(pipe_queue)) {


            error = _mutex_unlock(&io_pipe_info_ptr->ACCESS_MUTEX);
#if MQX_CHECK_ERRORS
            if ( error != MQX_EOK ) {
                return (-1);
            }
#endif

            error = _lwsem_post(&io_pipe_info_ptr->FULL_SEM);
#if MQX_CHECK_ERRORS
            if ( error != MQX_OK ) {
                return (-1);
            } /* Endif */
#endif   

            error = _lwsem_wait(&io_pipe_info_ptr->EMPTY_SEM);
#if MQX_CHECK_ERRORS
            if ( error != MQX_OK ) {
                return (-1);
            }
#endif   

            if(io_pipe_info_ptr->KM_ABORT_READ)
            {
                error = _mutex_unlock(&io_pipe_info_ptr->READ_MUTEX);
#if MQX_CHECK_ERRORS
                if ( error != MQX_EOK ) {
                    return (-1);
                }
#endif

                return (_mqx_int)(num - i);
           }


            /* Lock out access of a writer task to the pipe data structures */
            error = _mutex_lock(&io_pipe_info_ptr->ACCESS_MUTEX);
#if MQX_CHECK_ERRORS
            if ( error != MQX_EOK ) {
                return (-1);
            }
#endif

        }

        /* Read char from pipe */
        _CHARQ_DEQUEUE(pipe_queue,c);

        /* Copy data to the returned buffer */
        *data_ptr++ = c;
    }

    error = _lwsem_post(&io_pipe_info_ptr->FULL_SEM);
#if MQX_CHECK_ERRORS
    if ( error != MQX_OK ) {
        return (-1);
    }
#endif   

    /* Unlock access access mutex and read mutex */
    error = _mutex_unlock(&io_pipe_info_ptr->ACCESS_MUTEX);

#if MQX_CHECK_ERRORS
    if ( error != MQX_EOK ) {
        return (-1);
    }
#endif

    error = _mutex_unlock(&io_pipe_info_ptr->READ_MUTEX);

#if MQX_CHECK_ERRORS
    if ( error != MQX_EOK ) {
        return (-1);
    }
#endif

    return num;
}


/*FUNCTION****************************************************************
* 
* Function Name    : _io_pipe_write
* Returned Value   : void
* Comments         :
*    This routine writes characters to the Pipe.  It will block the task 
**   if the Pipe is full.
*
*END**********************************************************************/

_mqx_int _io_pipe_write
    (
        /* [IN] the handle returned from _fopen */
        FILE_DEVICE_STRUCT_PTR fd_ptr,

        /* [IN] where the characters to write out are */
        char              *data_ptr,

        /* [IN] the number of characters to write */
        _mqx_int               num
    )
{ /* Body */
    volatile IO_PIPE_INFO_STRUCT_PTR        io_pipe_info_ptr;
    uint32_t                                i = (uint32_t)(num + 1);
    unsigned char                           c;
    volatile CHARQ_STRUCT                   *pipe_queue;
    uint32_t                                error;


    io_pipe_info_ptr = fd_ptr->DEV_DATA_PTR;

    pipe_queue = io_pipe_info_ptr->QUEUE;

    /* Turn off pipe abort flags */
    io_pipe_info_ptr->KM_ABORT_WRITE = FALSE;

    /* Lock out other reading tasks */       
    error = _mutex_lock(&io_pipe_info_ptr->WRITE_MUTEX);
#if MQX_CHECK_ERRORS
    if ( error != MQX_EOK ) {
        return (-1);
    }
#endif

    /* Lock out access of a writer task to the pipe data structures */
    error = _mutex_lock(&io_pipe_info_ptr->ACCESS_MUTEX);
#if MQX_CHECK_ERRORS
    if ( error != MQX_EOK ) {
        return (-1);
    }
#endif

    /* Start writing characters to the pipe */
    while ( --i ) {
        /* If the pipe is full, wait until the pipe is not full */
        while ( _CHARQ_FULL(pipe_queue) ) {
            error = _mutex_unlock(&io_pipe_info_ptr->ACCESS_MUTEX);

#if MQX_CHECK_ERRORS
            if ( error != MQX_EOK ) {
                return (-1);
            }
#endif   

            error = _lwsem_post(&io_pipe_info_ptr->EMPTY_SEM);
#if MQX_CHECK_ERRORS
            if ( error != MQX_OK ) {
                return (-1);
            }
#endif

            error = _lwsem_wait(&io_pipe_info_ptr->FULL_SEM);
#if MQX_CHECK_ERRORS
            if ( error != MQX_OK ) {
                return (-1);
            }
#endif

            if(io_pipe_info_ptr->KM_ABORT_WRITE)
            {
                io_pipe_info_ptr->KM_ABORT_WRITE = FALSE;
                error = _mutex_unlock(&io_pipe_info_ptr->WRITE_MUTEX);
#if MQX_CHECK_ERRORS
                if ( error != MQX_EOK ) {
                    return (-1);
                }
#endif
                return (_mqx_int)(num - i);
            }
            /* Lock out access of a writer task to the pipe data structures */
            error = _mutex_lock(&io_pipe_info_ptr->ACCESS_MUTEX);
#if MQX_CHECK_ERRORS
            if ( error != MQX_EOK ) {
                return (-1);
            }
#endif
        }
        c = *data_ptr++;

        /* Write the next char into the pipe */
        _CHARQ_ENQUEUE(pipe_queue,c);
    }

    error = _lwsem_post(&io_pipe_info_ptr->EMPTY_SEM);
#if MQX_CHECK_ERRORS
    if ( error != MQX_OK ) {
        return (-1);
    }
#endif

    /* Unlock access mutex and write mutex */
    error = _mutex_unlock(&io_pipe_info_ptr->ACCESS_MUTEX);
#if MQX_CHECK_ERRORS
    if ( error != MQX_EOK ) {
        return (-1);
    }
#endif

    error = _mutex_unlock(&io_pipe_info_ptr->WRITE_MUTEX);

#if MQX_CHECK_ERRORS
    if ( error != MQX_EOK ) {
        return (-1);
    }
#endif


    return num;
}


/*FUNCTION*****************************************************************
* 
* Function Name    : _io_pipe_ioctl
* Returned Value   : int32_t
* Comments         :
*    Returns result of ioctl operation.
*
*END*********************************************************************/

_mqx_int _io_pipe_ioctl
    (
        /* [IN] the handle returned from _fopen */
        FILE_DEVICE_STRUCT_PTR fd_ptr,

        /* [IN] the ioctl command */
        uint32_t                cmd,

        /* [IN] the ioctl parameters */
        void                  *param_ptr
    )
{
    IO_PIPE_INFO_STRUCT_PTR io_pipe_info_ptr;
    uint32_t                result = MQX_OK;
    _mqx_uint_ptr           uparam_ptr = param_ptr;

    io_pipe_info_ptr = fd_ptr->DEV_DATA_PTR;

    result = _mutex_lock(&io_pipe_info_ptr->ACCESS_MUTEX);
#if MQX_CHECK_ERRORS
    if ( result != MQX_OK ) {
        return (_mqx_int)(result);
    }
#endif

    switch (cmd) {

        case PIPE_IOCTL_CHAR_AVAIL:
            if ( !_CHARQ_EMPTY(io_pipe_info_ptr->QUEUE) ) {
                *uparam_ptr = (uint32_t)TRUE;
            } else {
                *uparam_ptr = (uint32_t)FALSE;
            }
        break;

        case PIPE_IOCTL_GET_SIZE:
            *uparam_ptr =  io_pipe_info_ptr->QUEUE_SIZE;
        break;

        case PIPE_IOCTL_FULL:
            if ( _CHARQ_FULL(io_pipe_info_ptr->QUEUE) ) {
                *uparam_ptr = (uint32_t)TRUE;
            } else {
                *uparam_ptr = (uint32_t)FALSE;
            }
        break;
   
        case PIPE_IOCTL_EMPTY:
            if ( _CHARQ_EMPTY(io_pipe_info_ptr->QUEUE) ) {
                *uparam_ptr = (uint32_t)TRUE;
            } else {
                *uparam_ptr = (uint32_t)FALSE;
            }
        break;

        case PIPE_IOCTL_RE_INIT:
            _CHARQ_INIT(io_pipe_info_ptr->QUEUE, io_pipe_info_ptr->QUEUE_SIZE)
            io_pipe_info_ptr->KM_ABORT_READ = TRUE;
            io_pipe_info_ptr->KM_ABORT_WRITE = TRUE;

            result = _lwsem_post(&io_pipe_info_ptr->EMPTY_SEM);
#if MQX_CHECK_ERRORS
            if ( result != MQX_OK ) {
                return (-1);
            }
#endif
            result = _lwsem_post(&io_pipe_info_ptr->FULL_SEM);
#if MQX_CHECK_ERRORS
            if ( result != MQX_OK ) {
                return (-1);
            }
#endif
        break;

      case PIPE_IOCTL_NUM_CHARS_FULL:
           *uparam_ptr = _CHARQ_SIZE(io_pipe_info_ptr->QUEUE);
      break;

      case PIPE_IOCTL_NUM_CHARS_FREE:
           *uparam_ptr = io_pipe_info_ptr->QUEUE_SIZE - _CHARQ_SIZE(io_pipe_info_ptr->QUEUE);
      break;
      
      default:
      break;
   }

    result = _mutex_unlock(&io_pipe_info_ptr->ACCESS_MUTEX);
#if MQX_CHECK_ERRORS
    if ( result != MQX_OK ) {
        return (_mqx_int)(result);
    }
#endif

    return (_mqx_int)(result);

}


