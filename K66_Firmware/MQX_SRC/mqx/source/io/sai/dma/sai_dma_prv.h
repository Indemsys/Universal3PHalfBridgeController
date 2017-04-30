/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
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
*   This include file is used to provide information needed by
*   applications using the SAI I/O functions.
*
*
*END************************************************************************/

#ifndef __SAI_DMA_PRV_H__
#define __SAI_DMA_PRV_H__

/* 
** TYPE DECLARATION 
*/

typedef struct io_sai_device_struct
{

    /* The I/O init function */
    _mqx_int (_CODE_PTR_ DEV_INIT)(void *, uint8_t);

    /* The I/O deinit function */
    _mqx_int (_CODE_PTR_ DEV_DEINIT)(void *, uint8_t);

    /* The ioctl function, (change bauds etc) */
    _mqx_int (_CODE_PTR_ DEV_IOCTL)(void *, _mqx_int, void *);

    /* The I/O channel initialization data */
    void   *DEV_INIT_DATA_PTR;

    /* Device specific information for tx */
    void   *DEV_INFO_PTR;
    
    /* Open count for number of accessing file descriptor for reading */
    _mqx_int READ_COUNT;
    
    /* Open count for number of accessing file descriptor for writing */
    _mqx_int WRITE_COUNT;
    
    /* Open flags for this channel */
    _mqx_int FLAGS;
    
    LWSEM_STRUCT LWSEM;

} IO_SAI_DEVICE_STRUCT, * IO_SAI_DEVICE_STRUCT_PTR;

/*
** FUNCTION PROTOTYPE
*/

#ifdef __cplusplus
extern "C" 
{
#endif

    _mqx_int _io_sai_dma_open(MQX_FILE_PTR fd_ptr, char *open_name_ptr, char *flags);
    _mqx_int _io_sai_dma_close(MQX_FILE_PTR fd_ptr);
    _mqx_int _io_sai_dma_ioctl(MQX_FILE_PTR fd_ptr, _mqx_uint cmd, void *input_param_ptr);
    _mqx_int _io_sai_dma_uninstall(IO_DEVICE_STRUCT_PTR io_dev_ptr);

#ifdef __cplusplus
}
#endif

#endif /* __SAI_INT_PRV_H__ */

/* EOF */
