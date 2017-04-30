/*HEADER**********************************************************************
*
* Copyright 2013 Freescale Semiconductor, Inc.
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
*   This file contains generic DMA support functions
*
*
*END************************************************************************/

#include <mqx.h>
#include "dma.h"


static const int alignment_tab[4]  = { 4, 1, 2, 1 };

static const DMA_DEVIF_LIST *devif_list;
static int devif_list_count;


/*FUNCTION****************************************************************
*
* Function Name    : dma_tcd_memcpy
* Returned Value   :
* Comments         :
*    Prepares TCD for memory to memory copy
*
*END**********************************************************************/
void dma_tcd_memcpy(DMA_TCD *tcd, void *src, void *dst, uint32_t size)
{
    int src_alignment;
    int dst_alignment;
    int loop_alignment;

    src_alignment = alignment_tab[((uint32_t)src | (uint32_t)size) & 3];
    dst_alignment = alignment_tab[((uint32_t)src | (uint32_t)size) & 3];

    loop_alignment = (src_alignment > dst_alignment) ? src_alignment : dst_alignment;

    _mem_zero(tcd, sizeof(*tcd));

    tcd->SRC_ADDR = (uint32_t)src;
    tcd->SRC_WIDTH = src_alignment;
    tcd->SRC_OFFSET = src_alignment;

    tcd->DST_ADDR = (uint32_t)dst;
    tcd->DST_WIDTH = dst_alignment;
    tcd->DST_OFFSET = dst_alignment;

    tcd->LOOP_BYTES = loop_alignment;
    tcd->LOOP_COUNT = (size >> (loop_alignment/2)); /* loop_alignment is one of 1, 2 or 4, more efficient than plain division */
}


/*FUNCTION****************************************************************
*
* Function Name    : dma_tcd_mem2reg
* Returned Value   :
* Comments         :
*    Prepares TCD for memory to register copy
*
*END**********************************************************************/
int dma_tcd_mem2reg(DMA_TCD *tcd, volatile void *reg, int regw, void *src, uint32_t size)
{
    uint32_t src_alignment;
    uint32_t reg_alignment;
    int endian_swap;
    int srcw;

    endian_swap = (regw < 0);
    regw = (regw < 0) ? -regw : regw;

    if ((regw != 1) && (regw != 2) && (regw != 4)) {
        return MQX_INVALID_PARAMETER;
    }

    reg_alignment = alignment_tab[((uint32_t)reg | (uint32_t)size) & 3];

    if (reg_alignment < regw) {
        return MQX_INVALID_PARAMETER;
    }

    _mem_zero(tcd, sizeof(*tcd));

    if (endian_swap) {
        tcd->SRC_ADDR = (uint32_t)src + regw - 1;
        tcd->SRC_WIDTH = 1;
        tcd->SRC_OFFSET = -1;
        tcd->LOOP_SRC_OFFSET = 2*regw;
    }
    else {
        src_alignment = alignment_tab[((uint32_t)src | (uint32_t)size) & 3];
        srcw = (src_alignment > regw) ? regw : src_alignment;
        tcd->SRC_ADDR = (uint32_t)src;
        tcd->SRC_WIDTH = srcw;
        tcd->SRC_OFFSET = srcw;
    }

    tcd->DST_ADDR = (uint32_t)reg;
    tcd->DST_WIDTH = regw;
    tcd->DST_OFFSET = 0; /* periodic write to the same address */

    tcd->LOOP_BYTES = regw;
    tcd->LOOP_COUNT = (size >> (regw/2));

    return MQX_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : dma_tcd_reg2mem
* Returned Value   :
* Comments         :
*    Prepares TCD for register to memory copy
*
*END**********************************************************************/
int dma_tcd_reg2mem(DMA_TCD *tcd, volatile void *reg, int regw, void *dst, uint32_t size)
{
    uint32_t dst_alignment;
    uint32_t reg_alignment;
    int endian_swap;
    int dstw;

    endian_swap = (regw < -1);
    regw = (regw < 0) ? -regw : regw;

    if ((regw != 1) && (regw != 2) && (regw != 4)) {
        return MQX_INVALID_PARAMETER;
    }

    reg_alignment = alignment_tab[((uint32_t)reg | (uint32_t)size) & 3];

    if (reg_alignment < regw) {
        return MQX_INVALID_PARAMETER;
    }

    _mem_zero(tcd, sizeof(*tcd));

    tcd->SRC_ADDR = (uint32_t)reg; /* periodic read from the same address */
    tcd->SRC_WIDTH = regw;
    tcd->SRC_OFFSET = 0;

    if (endian_swap) {
        tcd->DST_ADDR = (uint32_t)dst + regw - 1;
        tcd->DST_WIDTH = 1;
        tcd->DST_OFFSET = -1;
        tcd->LOOP_DST_OFFSET = 2*regw;
    }
    else {
        dst_alignment = alignment_tab[((uint32_t)dst | (uint32_t)size) & 3];
        dstw = (dst_alignment > regw) ? regw : dst_alignment;
        tcd->DST_ADDR = (uint32_t)dst;
        tcd->DST_WIDTH = dstw;
        tcd->DST_OFFSET = dstw;
    }

    tcd->LOOP_BYTES = regw;
    tcd->LOOP_COUNT = (size >> (regw/2));

    return MQX_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : dma_init
* Returned Value   :
* Comments         :
*    Initialization of DMA driver list
*
*END**********************************************************************/
int dma_init(const DMA_DEVIF_LIST *init_devif_list)
{
    const DMA_DEVIF_LIST *devif_list_item;
    int result;

    #if MQX_CHECK_ERRORS
    if (init_devif_list == NULL)
    {
        return MQX_INVALID_POINTER;
    }
    #endif

    /* Deinit current list first */
    dma_deinit();

    devif_list = init_devif_list;
    devif_list_count = 0;

    devif_list_item = devif_list;
    while (devif_list_item->CHANNELS) {
        if (devif_list_item->DEVIF && devif_list_item->DEVIF->INIT) {
            result = devif_list_item->DEVIF->INIT(devif_list_item->DEVIF_DATA);
            if (result != MQX_OK) {
                return result;
            }
        }
        devif_list_count++;
        devif_list_item++;
    }

    return MQX_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : dma_deinit
* Returned Value   :
* Comments         :
*    Deinitialization of all DMA drivers list
*    MQX_OK or first error which occurs is returned
*
*END**********************************************************************/
int dma_deinit(void)
{
    const DMA_DEVIF_LIST *devif_list_item;
    int result_tmp;
    int result = MQX_OK;

    if (devif_list == NULL) {
        return MQX_OK;
    }

    devif_list_item = devif_list;
    while (devif_list_count) {
        if (devif_list_item->DEVIF && devif_list_item->DEVIF->DEINIT) {
            result_tmp = devif_list_item->DEVIF->DEINIT(devif_list_item->DEVIF_DATA);
            if (MQX_OK == result) {
                result = result_tmp;
            }
        }
        devif_list_item++;
        devif_list_count--;
    }
    devif_list = NULL;
    
    return result;
}


/*FUNCTION*****8***********************************************************
*
* Function Name    : dma_channel_claim
* Returned Value   :
* Comments         :
*    Reserves requested channel and fills in the channel handle
*
*END**********************************************************************/
int dma_channel_claim(DMA_CHANNEL_HANDLE *handle_ptr, int vchannel)
{
    const DMA_DEVIF_LIST *devif_list_item;
    int result;
    int channel;

    #if MQX_CHECK_ERRORS
    if (handle_ptr == NULL)
    {
        return MQX_INVALID_POINTER;
    }
    if (vchannel < 0)
    {
        return MQX_INVALID_PARAMETER;
    }
    #endif

    channel = vchannel;
    devif_list_item = devif_list;
    while (devif_list_item->CHANNELS && (channel >= devif_list_item->CHANNELS)) {
        channel -= devif_list_item->CHANNELS;
        devif_list_item++;
    }

    if (0 == devif_list_item->CHANNELS) {
        /* End of device interface list reached - the requested channel number is not service by any device interface */
        return MQX_INVALID_PARAMETER;
    }
    
    if ((NULL == devif_list_item->DEVIF) || (NULL == devif_list_item->DEVIF->CHANNEL_CLAIM))
    {
        /* The device interface for this channel is a dummy one (channel range placeholder) */
        return MQX_IO_OPERATION_NOT_AVAILABLE;
    }

    /* Memory for channel handle is allocated by specific CHANNEL_CLAIM */
    result = devif_list_item->DEVIF->CHANNEL_CLAIM(handle_ptr, devif_list_item->DEVIF_DATA, channel);

    if (result == MQX_OK)
    {
        /* Init generic channel context */
        (*handle_ptr)->DEVIF = devif_list_item->DEVIF;
        (*handle_ptr)->CALLBACK_FUNC = NULL;
        (*handle_ptr)->CALLBACK_DATA = NULL;
    }

    return result;
}


/*FUNCTION****************************************************************
*
* Function Name    : dma_channel_release
* Returned Value   :
* Comments         :
*    Releases previously reserved channel and invalidates the handle
*
*END**********************************************************************/
int dma_channel_release(DMA_CHANNEL_HANDLE handle)
{
    #if MQX_CHECK_ERRORS
    if ((NULL == handle) || (NULL == handle->DEVIF))
    {
        return MQX_INVALID_POINTER;
    }
    if (NULL == handle->DEVIF->CHANNEL_RELEASE)
    {
        return MQX_IO_OPERATION_NOT_AVAILABLE;
    }
    #endif

    return handle->DEVIF->CHANNEL_RELEASE(handle);
}


/*FUNCTION****************************************************************
*
* Function Name    : dma_channel_reset
* Returned Value   :
* Comments         :
*    Aborts transfer in progress and discards any waiting transfers
*
*END**********************************************************************/
int dma_channel_reset(DMA_CHANNEL_HANDLE handle)
{
    #if MQX_CHECK_ERRORS
    if ((NULL == handle) || (NULL == handle->DEVIF))
    {
        return MQX_INVALID_POINTER;
    }
    if (NULL == handle->DEVIF->CHANNEL_RESET)
    {
        return MQX_IO_OPERATION_NOT_AVAILABLE;
    }
    #endif

    return handle->DEVIF->CHANNEL_RESET(handle);
}


/*FUNCTION****************************************************************
*
* Function Name    : dma_channel_setup
* Returned Value   :
* Comments         :
*    Allocates TCD FIFO for and prepares the channel to accept transfers
*
*END**********************************************************************/
int dma_channel_setup(DMA_CHANNEL_HANDLE handle, int tcd_slots, uint32_t flags)
{
    #if MQX_CHECK_ERRORS
    if ((NULL == handle) || (NULL == handle->DEVIF))
    {
        return MQX_INVALID_POINTER;
    }
    if (NULL == handle->DEVIF->CHANNEL_SETUP)
    {
        return MQX_IO_OPERATION_NOT_AVAILABLE;
    }
    #endif

    return handle->DEVIF->CHANNEL_SETUP(handle, tcd_slots, flags);
}


/*FUNCTION****************************************************************
*
* Function Name    : dma_channel_status
* Returned Value   :
* Comments         :
*    Returns current status of given channel and optionally returns transfer progress
*
*END**********************************************************************/
int dma_channel_status(DMA_CHANNEL_HANDLE handle, uint32_t *tcd_seq, uint32_t *tcd_remaining)
{
    #if MQX_CHECK_ERRORS
    if ((NULL == handle) || (NULL == handle->DEVIF))
    {
        return MQX_INVALID_POINTER;
    }
    if (NULL == handle->DEVIF->CHANNEL_STATUS)
    {
        return MQX_IO_OPERATION_NOT_AVAILABLE;
    }
    #endif

    return handle->DEVIF->CHANNEL_STATUS(handle, tcd_seq, tcd_remaining);
}


/*FUNCTION****************************************************************
*
* Function Name    : dma_transfer_submit
* Returned Value   :
* Comments         :
*    Queues transfer on given channel according to parameters in DMA_TCD structure
*
*END**********************************************************************/
int dma_transfer_submit(DMA_CHANNEL_HANDLE handle, DMA_TCD *tcd, uint32_t *tcd_seq)
{
    #if MQX_CHECK_ERRORS
    if ((NULL == handle) || (NULL == handle->DEVIF))
    {
        return MQX_INVALID_POINTER;
    }
    if (NULL == handle->DEVIF->TRANSFER_SUBMIT)
    {
        return MQX_IO_OPERATION_NOT_AVAILABLE;
    }
    #endif

    return handle->DEVIF->TRANSFER_SUBMIT(handle, tcd, tcd_seq);
}


/*FUNCTION****************************************************************
*
* Function Name    : dma_request_source
* Returned Value   :
* Comments         :
*    Configures request source for given channel
*
*END**********************************************************************/
int dma_request_source(DMA_CHANNEL_HANDLE handle, uint32_t source)
{
    #if MQX_CHECK_ERRORS
    if ((NULL == handle) || (NULL == handle->DEVIF))
    {
        return MQX_INVALID_POINTER;
    }
    if (NULL == handle->DEVIF->REQUEST_SOURCE)
    {
        return MQX_IO_OPERATION_NOT_AVAILABLE;
    }
    #endif

    return handle->DEVIF->REQUEST_SOURCE(handle, source);
}

/*FUNCTION****************************************************************
*
* Function Name    : dma_request_enable
* Returned Value   :
* Comments         :
*    Enables request on given channel to start/resume a transfer
*
*END**********************************************************************/
int dma_request_enable(DMA_CHANNEL_HANDLE handle)
{
    #if MQX_CHECK_ERRORS
    if ((NULL == handle) || (NULL == handle->DEVIF))
    {
        return MQX_INVALID_POINTER;
    }
    if (NULL == handle->DEVIF->REQUEST_ENABLE)
    {
        return MQX_IO_OPERATION_NOT_AVAILABLE;
    }
    #endif

    return handle->DEVIF->REQUEST_ENABLE(handle);
}


/*FUNCTION****************************************************************
*
* Function Name    : dma_request_disable
* Returned Value   :
* Comments         :
*    Disables request on given channel to pause a transfer
*
*END**********************************************************************/
int dma_request_disable(DMA_CHANNEL_HANDLE handle)
{
    #if MQX_CHECK_ERRORS
    if ((NULL == handle) || (NULL == handle->DEVIF))
    {
        return MQX_INVALID_POINTER;
    }
    if (NULL == handle->DEVIF->REQUEST_DISABLE)
    {
        return MQX_IO_OPERATION_NOT_AVAILABLE;
    }
    #endif

    return handle->DEVIF->REQUEST_DISABLE(handle);
}


/*FUNCTION****************************************************************
*
* Function Name    : dma_callback_reg
* Returned Value   :
* Comments         :
*    Registers function to be called in case of an event related to the channel
*
*END**********************************************************************/
int dma_callback_reg(DMA_CHANNEL_HANDLE handle, DMA_EOT_CALLBACK callback_func, void *callback_data)
{
    DMA_CHANNEL_STRUCT volatile *handle_vol;

    #if MQX_CHECK_ERRORS
    if (NULL == handle)
    {
        return MQX_INVALID_POINTER;
    }
    #endif

    handle_vol = handle;
    handle_vol->CALLBACK_FUNC = NULL; /* Safety precaution if ISR is executed meanwhile */
    handle_vol->CALLBACK_DATA = callback_data;
    handle_vol->CALLBACK_FUNC = callback_func;

    return MQX_OK;
}
