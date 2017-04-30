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
*   This file contains the Ramdisk driver functions
*
*
*END************************************************************************/

#include "mqx.h"
#include "fio.h"
#include "fio_prv.h"
#include "io.h"
#include "io_prv.h"
#include "io_mem.h"
#include "iomemprv.h"

/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : _io_mem_install
* Returned Value   : _mqx_uint a task error code or MQX_OK
* Comments         :
*    Install a fdv_ram driver.
*
*END*----------------------------------------------------------------------*/

_mqx_uint _io_mem_install
    (
        /* [IN] A string that identifies the device for fopen */
        char            *identifier,

        /* [IN] the address of the fdv_ram */
        void               *base_address, 

        /* [IN] the total size of the device */
        _file_size          size

    )
{
    IO_MEM_STRUCT_PTR handle_ptr;

#if (MAX_FILE_SIZE > MAX_MEM_SIZE)
   #if MQX_CHECK_MEMORY_ALLOCATION_ERRORS
    if (size > MAX_MEM_SIZE) {
        return(MQX_OUT_OF_MEMORY);
    }
   #endif
#endif

    handle_ptr = (IO_MEM_STRUCT_PTR)
    _mem_alloc_system_zero((_mem_size)sizeof(IO_MEM_STRUCT));
#if MQX_CHECK_MEMORY_ALLOCATION_ERRORS
    if (handle_ptr == NULL) {
        return(MQX_OUT_OF_MEMORY);
    }
#endif
    _mem_set_type(handle_ptr,MEM_TYPE_IO_MEM_STRUCT);            

    if (base_address == NULL) {
        base_address = _mem_alloc_system((_mem_size)size);
#if MQX_CHECK_MEMORY_ALLOCATION_ERRORS
        if (base_address == NULL) {
            _mem_free(handle_ptr);
            return(MQX_OUT_OF_MEMORY);
        }
#endif
        _mem_set_type(base_address,MEM_TYPE_IO_MEM_DATA);              

        /* Indicate the the RAM drive was allocated from kernel memory */
        handle_ptr->TYPE = MEM_TYPE_DYNAMIC;
    } else {
        /* Indicate the the RAM drive was statically allocated (global mem) */
        handle_ptr->TYPE = MEM_TYPE_STATIC;
    }

    handle_ptr->BASE_ADDR   = base_address;
    handle_ptr->SIZE        = size;

    _lwsem_create(&handle_ptr->LWSEM, 1);

    return (_io_dev_install(
        identifier,
        _io_mem_open,
        _io_mem_close,
        _io_mem_read,
        _io_mem_write,
        _io_mem_ioctl,
        (void *)handle_ptr
    ));
}


/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : _io_mem_uninstall
* Returned Value   : _mqx_uint a task error code or MQX_OK
* Comments         :
*    Uninstall a fdv_ram driver.
*
*END*----------------------------------------------------------------------*/

_mqx_int _io_mem_uninstall
    (
        /* [IN] The devices context */
        IO_DEVICE_STRUCT_PTR  io_dev_ptr
    )
{
    IO_MEM_STRUCT_PTR     handle_ptr = (IO_MEM_STRUCT_PTR)io_dev_ptr->DRIVER_INIT_PTR;

    _lwsem_destroy(&handle_ptr->LWSEM);

    if (handle_ptr->TYPE == MEM_TYPE_DYNAMIC) {
        _mem_free(handle_ptr->BASE_ADDR);
    }
    _mem_free(handle_ptr);

    return(MQX_OK);
}


/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : _io_mem_open
* Returned Value   : MQX_OK
* Comments         : Opens and initializes fdv_ram driver.
* 
*END*----------------------------------------------------------------------*/

_mqx_int _io_mem_open
    (
        /* [IN] the file handle for the device being opened */
        FILE_DEVICE_STRUCT_PTR   fd_ptr,

        /* [IN] the remaining portion of the name of the device */
        char                 *open_name_ptr,

        /* [IN] the flags to be used during operation:
        ** blockmode
        */
        char                 *flags
    )
{
    (void)                  open_name_ptr; /* disable 'unused variable' warning */
    IO_DEVICE_STRUCT_PTR    io_dev_ptr = fd_ptr->DEV_PTR;
    IO_MEM_STRUCT_PTR       handle_ptr = (IO_MEM_STRUCT_PTR)io_dev_ptr->DRIVER_INIT_PTR;

    if (flags && (*flags == 'b')) {
        handle_ptr->BLOCK_MODE = TRUE;
        handle_ptr->NUM_BLOCKS = handle_ptr->SIZE >> IO_MEM_BLOCK_SIZE_POWER;

        /* Redirect the read/write functions */
        io_dev_ptr->IO_READ  = _io_mem_read_blocks;
        io_dev_ptr->IO_WRITE = _io_mem_write_blocks;
    }

    return(MQX_OK);
}


/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : _io_mem_close
* Returned Value   : ERROR CODE
* Comments         : Closes fdv_ram driver
* 
*END*----------------------------------------------------------------------*/

_mqx_int _io_mem_close
    (
        /* [IN] the file handle for the device being closed */
        FILE_DEVICE_STRUCT_PTR fd_ptr
    )
{
    (void)fd_ptr; /* disable 'unused variable' warning */
    /* Nothing to do */
    return(MQX_OK);
}


/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : _io_mem_read
* Returned Value   : number of characters read
* Comments         : Reads data from fdv_ram driver
* 
*END*----------------------------------------------------------------------*/

_mqx_int _io_mem_read
    (
        /* [IN] the file handle for the device */
        FILE_DEVICE_STRUCT_PTR   fd_ptr,

        /* [IN] where the characters are to be stored */
        char                 *data_ptr,

        /* [IN] the number of characters to input */
        _mqx_int                 num
    )
{
    IO_DEVICE_STRUCT_PTR    io_dev_ptr = fd_ptr->DEV_PTR;
    IO_MEM_STRUCT_PTR       handle_ptr = (IO_MEM_STRUCT_PTR)io_dev_ptr->DRIVER_INIT_PTR;
    unsigned char           *src_ptr;
    _file_size              size = handle_ptr->SIZE;

    if( fd_ptr->LOCATION >= size) {
        fd_ptr->FLAGS |= IO_FLAG_AT_EOF;
        return(0);
    } else {
        src_ptr = fd_ptr->LOCATION + handle_ptr->BASE_ADDR;
        if ( (src_ptr + num) > (handle_ptr->BASE_ADDR + size) ) {
            fd_ptr->FLAGS |= IO_FLAG_AT_EOF;
            num = (_mqx_int)((handle_ptr->BASE_ADDR + size) - 
            (handle_ptr->BASE_ADDR + fd_ptr->LOCATION) + 1);
        }
        _lwsem_wait(&handle_ptr->LWSEM);
        _mem_copy((void *)src_ptr, (void *)data_ptr, (_mem_size)num);
        fd_ptr->LOCATION += (_file_size)num;
        _lwsem_post(&handle_ptr->LWSEM);
        return(num);
    }
}


/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : _io_mem_write
* Returned Value   : number of characters written
* Comments         : Writes data to the fdv_ram device
* 
*END*----------------------------------------------------------------------*/

_mqx_int _io_mem_write
    (
        /* [IN] the file handle for the device */
        FILE_DEVICE_STRUCT_PTR  fd_ptr,

        /* [IN] where the characters are */
        char                *data_ptr,

        /* [IN] the number of characters to output */
        _mqx_int                num
    )
{
    IO_DEVICE_STRUCT_PTR    io_dev_ptr = fd_ptr->DEV_PTR;
    IO_MEM_STRUCT_PTR       handle_ptr = (IO_MEM_STRUCT_PTR)io_dev_ptr->DRIVER_INIT_PTR;
    unsigned char           *dest_ptr;
    _file_size              size = handle_ptr->SIZE;

    if(fd_ptr->LOCATION >= size) {
        fd_ptr->FLAGS |= IO_FLAG_AT_EOF;
        return(0);
    } else {
        dest_ptr = fd_ptr->LOCATION + handle_ptr->BASE_ADDR;
        if ( (dest_ptr + num) > (handle_ptr->BASE_ADDR + size) )
        {
            fd_ptr->FLAGS |= IO_FLAG_AT_EOF;
            num = (_mqx_int)((handle_ptr->BASE_ADDR + size) - (handle_ptr->BASE_ADDR + fd_ptr->LOCATION) + 1);
        }
        _lwsem_wait(&handle_ptr->LWSEM);
        _mem_copy((void *)data_ptr, (void *)dest_ptr, (_mem_size)num);
        fd_ptr->LOCATION += (_file_size)num;
        _lwsem_post(&handle_ptr->LWSEM);
        return(num);
    }
}


/*FUNCTION*****************************************************************
* 
* Function Name    : _io_mem_ioctl
* Returned Value   : _mqx_int
* Comments         :
*    Returns result of ioctl operation.
*
*END*********************************************************************/

_mqx_int _io_mem_ioctl
    (
        /* [IN] the file handle for the device */
        FILE_DEVICE_STRUCT_PTR fd_ptr,

        /* [IN] the ioctl command */
        _mqx_uint               cmd,

        /* [IN/OUT] the ioctl parameters */
        void                  *param_ptr
    )
{
    IO_DEVICE_STRUCT_PTR io_dev_ptr = fd_ptr->DEV_PTR;
    IO_MEM_STRUCT_PTR    handle_ptr = (IO_MEM_STRUCT_PTR)io_dev_ptr->DRIVER_INIT_PTR;
    void                 **pparam_ptr;
    _mem_size            *mparam_ptr;
    _mqx_uint            *uparam_ptr;
    bool                 *bparam_ptr;
    _mqx_int             result = MQX_OK;
    uint32_t             *u32param_ptr;

    switch (cmd) {
        case IO_MEM_IOCTL_GET_BASE_ADDRESS:
            pparam_ptr  = (void **)param_ptr;
            *pparam_ptr = (void *)handle_ptr->BASE_ADDR;
        break;
        case IO_MEM_IOCTL_GET_TOTAL_SIZE:
            mparam_ptr  = (_mem_size_ptr)param_ptr;
            *mparam_ptr = (_mem_size)handle_ptr->SIZE;
        break;
        case IO_MEM_IOCTL_GET_DEVICE_ERROR:
            uparam_ptr  = (_mqx_uint_ptr)param_ptr;
            *uparam_ptr = (_mqx_uint)ferror(fd_ptr);
        break;
        case IO_IOCTL_SEEK_AVAIL:
            bparam_ptr  = (bool *)param_ptr;
            *bparam_ptr = TRUE;
        break;
        case IO_IOCTL_GET_BLOCK_SIZE:
            uparam_ptr  = (_mqx_uint_ptr)param_ptr;
            *uparam_ptr = IO_MEM_BLOCK_SIZE;
        break;
        case IO_IOCTL_DEVICE_IDENTIFY:
            u32param_ptr  = (uint32_t *)param_ptr;
            u32param_ptr[IO_IOCTL_ID_PHY_ELEMENT]  = IO_DEV_TYPE_PHYS_MEMIO;
            u32param_ptr[IO_IOCTL_ID_LOG_ELEMENT]  = IO_DEV_TYPE_LOGICAL_MFS;
            u32param_ptr[IO_IOCTL_ID_ATTR_ELEMENT] = IO_MEM_ATTRIBS;
            if (handle_ptr->BLOCK_MODE) {
                u32param_ptr[IO_IOCTL_ID_ATTR_ELEMENT] |= IO_DEV_ATTR_BLOCK_MODE;
            }
        break;
        case IO_IOCTL_GET_NUM_SECTORS:
            /* return number of sectors, needed for MFS format */
            mparam_ptr = (_mem_size_ptr)param_ptr;
            *mparam_ptr = (_mem_size)handle_ptr->SIZE / IO_MEM_BLOCK_SIZE;
        break;
        default:
            result = IO_ERROR_INVALID_IOCTL_CMD;
        break;
    }
    return result;
}


/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : _io_mem_read_blocks
* Returned Value   : number of blocks read
* Comments         : Reads data from fdv_ram driver
* 
*END*----------------------------------------------------------------------*/

_mqx_int _io_mem_read_blocks
    (
        /* [IN] the file handle for the device */
        FILE_DEVICE_STRUCT_PTR    fd_ptr,

        /* [IN] where the blocks are to be stored */
        char                      *data_ptr,

        /* [IN] the number of blocks to input */
        _mqx_int                  num
    )
{
    IO_DEVICE_STRUCT_PTR  io_dev_ptr = fd_ptr->DEV_PTR;
    IO_MEM_STRUCT_PTR     handle_ptr = (IO_MEM_STRUCT_PTR)io_dev_ptr->DRIVER_INIT_PTR;
    unsigned char             *src_ptr;
    _file_size            blocks     = handle_ptr->NUM_BLOCKS;
    _file_size            bytes_to_be_read;

    if ( (fd_ptr->LOCATION + num) > blocks) {
        fd_ptr->FLAGS |= IO_FLAG_AT_EOF;
        num = (_mqx_int)(blocks - fd_ptr->LOCATION - 1);
    }

    if (num <= 0) {
        return 0;
    }

    bytes_to_be_read = (_mqx_uint)(num << IO_MEM_BLOCK_SIZE_POWER);
    src_ptr = (fd_ptr->LOCATION << IO_MEM_BLOCK_SIZE_POWER) + handle_ptr->BASE_ADDR;

    _lwsem_wait(&handle_ptr->LWSEM);
    _mem_copy((void *)src_ptr, (void *)data_ptr, (_mem_size)bytes_to_be_read);
    fd_ptr->LOCATION += (_file_size)num;
    _lwsem_post(&handle_ptr->LWSEM);

    return num;
}


/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : _io_mem_write_blocks
* Returned Value   : number of blocks written
* Comments         : Writes data to fdv_ram driver
* 
*END*----------------------------------------------------------------------*/

_mqx_int _io_mem_write_blocks
    (
        /* [IN] the file handle for the device */
        FILE_DEVICE_STRUCT_PTR   fd_ptr,

        /* [IN] where the blocks are stored */
        char                 *data_ptr,

        /* [IN] the number of blocks to output */
        _mqx_int                 num
    )
{
    IO_DEVICE_STRUCT_PTR    io_dev_ptr = fd_ptr->DEV_PTR;
    IO_MEM_STRUCT_PTR       handle_ptr = (IO_MEM_STRUCT_PTR)io_dev_ptr->DRIVER_INIT_PTR;
    unsigned char           *src_ptr;
    _file_size              blocks     = handle_ptr->NUM_BLOCKS;
    _file_size              bytes_to_be_written;

    if ( (fd_ptr->LOCATION + num) > blocks) {
        fd_ptr->FLAGS |= IO_FLAG_AT_EOF;
        num = (_mqx_int)(blocks - fd_ptr->LOCATION);
    }

    if (num <= 0) {
        return 0;
    }

    bytes_to_be_written = (_file_size)(num << IO_MEM_BLOCK_SIZE_POWER);
    src_ptr = (fd_ptr->LOCATION << IO_MEM_BLOCK_SIZE_POWER) + handle_ptr->BASE_ADDR;

    _lwsem_wait(&handle_ptr->LWSEM);
    _mem_copy((void *)data_ptr, (void *)src_ptr, (_mem_size)bytes_to_be_written);
    fd_ptr->LOCATION += (_file_size)num;
    _lwsem_post(&handle_ptr->LWSEM);

    return num;
}

