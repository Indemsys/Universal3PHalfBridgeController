/*HEADER**********************************************************************
*
* Copyright 2012 Freescale Semiconductor, Inc.
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
*   The file contains low level SPI driver functions for DSPI module
*
*
*END************************************************************************/


#include <mqx.h>
#include <bsp.h>
#include <io_prv.h>

#include "spi.h"
#include "spi_prv.h"

#include "spi_dspi_common.h"
#include "spi_dspi_dma.h"
#include "spi_dspi_dma_prv.h"

#include "dma.h"


/* SPI low level driver interface functions */
static _mqx_int _dspi_dma_init(const void  *init_data_ptr, void **io_info_ptr_ptr);
static _mqx_int _dspi_dma_deinit(void *io_info_ptr);
static _mqx_int _dspi_dma_setparam(void *io_info_ptr, SPI_PARAM_STRUCT_PTR params);
static _mqx_int _dspi_dma_tx_rx(void *io_info_ptr, uint8_t *txbuf, uint8_t *rxbuf, uint32_t len);
static _mqx_int _dspi_dma_cs_deassert(void *io_info_ptr);
static _mqx_int _dspi_dma_ioctl(void *io_info_ptr, SPI_PARAM_STRUCT_PTR params, uint32_t cmd, uint32_t *param_ptr);

const SPI_DEVIF_STRUCT _spi_dspi_dma_devif = {
    _dspi_dma_init,
    _dspi_dma_deinit,
    _dspi_dma_setparam,
    _dspi_dma_tx_rx,
    _dspi_dma_cs_deassert,
    _dspi_dma_ioctl
};


/* Forward declarations */
static void _dspi_dma_callback(void *parameter, int tcds_done, uint32_t tcd_seq);


/*FUNCTION****************************************************************
*
* Function Name    : _dspi_dma_init
* Returned Value   : MQX error code
* Comments         :
*    This function initializes the SPI driver
*
*END*********************************************************************/
static _mqx_int _dspi_dma_init
    (
        /* [IN] The initialization information for the device being opened */
        const void                     *init_data_ptr,

        /* [OUT] The address to store device specific information */
        void                           **io_info_ptr_ptr
    )
{
    DSPI_DMA_INIT_STRUCT_PTR           dspi_init_ptr = (DSPI_DMA_INIT_STRUCT_PTR)init_data_ptr;
    DSPI_DMA_INFO_STRUCT_PTR           dspi_info_ptr;
    VDSPI_REG_STRUCT_PTR               dspi_ptr;
    int                                result;

    #if PSP_HAS_DEVICE_PROTECTION
    if (!_bsp_dspi_enable_access(dspi_init_ptr->CHANNEL)) {
        return SPI_ERROR_CHANNEL_INVALID;
    }
    #endif

    /* Check channel */
    dspi_ptr = _bsp_get_dspi_base_address (dspi_init_ptr->CHANNEL);
    if (NULL == dspi_ptr)
    {
        return SPI_ERROR_CHANNEL_INVALID;
    }

    if (_bsp_dspi_io_init (dspi_init_ptr->CHANNEL) == -1)
    {
        return SPI_ERROR_CHANNEL_INVALID;
    }

    /* Initialize internal data */
    dspi_info_ptr = (DSPI_DMA_INFO_STRUCT_PTR)_mem_alloc_system_zero((uint32_t)sizeof(DSPI_DMA_INFO_STRUCT));
    if (dspi_info_ptr == NULL)
    {
        return MQX_OUT_OF_MEMORY;
    }
    _mem_set_type(dspi_info_ptr, MEM_TYPE_IO_SPI_INFO_STRUCT);

    *io_info_ptr_ptr = (void *)dspi_info_ptr;

    dspi_info_ptr->DSPI_PTR = dspi_ptr;
    dspi_info_ptr->CHANNEL = dspi_init_ptr->CHANNEL;
    dspi_info_ptr->CLOCK_SOURCE = dspi_init_ptr->CLOCK_SOURCE;

    _dspi_init_low(dspi_info_ptr->DSPI_PTR);

    /* Claim DMA channels and perform setup */
    if ((result = dma_channel_claim(&dspi_info_ptr->DMA_RX_CHANNEL, dspi_init_ptr->DMA_RX_CHANNEL)) != MQX_OK
        || (result = dma_channel_claim(&dspi_info_ptr->DMA_TX_CHANNEL, dspi_init_ptr->DMA_TX_CHANNEL)) != MQX_OK
        || (result = dma_channel_setup(dspi_info_ptr->DMA_RX_CHANNEL, 1, 0)) != MQX_OK
        || (result = dma_channel_setup(dspi_info_ptr->DMA_TX_CHANNEL, 1, 0)) != MQX_OK
        || (result = dma_request_source(dspi_info_ptr->DMA_RX_CHANNEL, dspi_init_ptr->DMA_RX_SOURCE)) != MQX_OK
        || (result = dma_request_source(dspi_info_ptr->DMA_TX_CHANNEL, dspi_init_ptr->DMA_TX_SOURCE)) != MQX_OK
       )
    {
        dma_channel_release(dspi_info_ptr->DMA_RX_CHANNEL);
        dma_channel_release(dspi_info_ptr->DMA_TX_CHANNEL);
        _mem_free(dspi_info_ptr);
        return result;
    }

    /* Allocate cache line aligned block of memory and split it in half to form RX and TX buffer */
    dspi_info_ptr->RX_BUF = _mem_alloc_system(4*PSP_CACHE_LINE_SIZE);
    if (dspi_info_ptr->RX_BUF == NULL)
    {
        dma_channel_release(dspi_info_ptr->DMA_RX_CHANNEL);
        dma_channel_release(dspi_info_ptr->DMA_TX_CHANNEL);
        _mem_free(dspi_info_ptr);
        return MQX_OUT_OF_MEMORY;
    }
    dspi_info_ptr->TX_BUF = dspi_info_ptr->RX_BUF + 2*PSP_CACHE_LINE_SIZE;

    _lwsem_create(&dspi_info_ptr->EVENT_IO_FINISHED, 0);

    dma_callback_reg(dspi_info_ptr->DMA_RX_CHANNEL, _dspi_dma_callback, dspi_info_ptr);

    /* Route data s to DMA */
    dspi_ptr->RSER = DSPI_RSER_RFDF_DIRS_MASK | DSPI_RSER_RFDF_RE_MASK | DSPI_RSER_TFFF_DIRS_MASK | DSPI_RSER_TFFF_RE_MASK;
    dma_request_enable(dspi_info_ptr->DMA_RX_CHANNEL);
    dma_request_enable(dspi_info_ptr->DMA_TX_CHANNEL);

    return SPI_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : _dspi_dma_deinit
* Returned Value   : MQX error code
* Comments         :
*    This function de-initializes the SPI module
*
*END*********************************************************************/
static _mqx_int _dspi_dma_deinit
    (
        /* [IN] the address of the device specific information */
        void                          *io_info_ptr
    )
{
    DSPI_DMA_INFO_STRUCT_PTR           dspi_info_ptr = (DSPI_DMA_INFO_STRUCT_PTR)io_info_ptr;

    if (NULL == dspi_info_ptr)
    {
        return SPI_ERROR_DEINIT_FAILED;
    }

    /* Release DMA channels */
    dma_channel_release(dspi_info_ptr->DMA_RX_CHANNEL);
    dma_channel_release(dspi_info_ptr->DMA_TX_CHANNEL);

    /* Free buffers */
    if (dspi_info_ptr->RX_BUF) {
        _mem_free(dspi_info_ptr->RX_BUF);
    }

    _lwsem_destroy(&dspi_info_ptr->EVENT_IO_FINISHED);

    _dspi_deinit_low(dspi_info_ptr->DSPI_PTR);

    _mem_free(dspi_info_ptr);

    return SPI_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : _dspi_dma_setparam
* Returned Value   :
* Comments         :
*    Set parameters for following transfers.
*
*END*********************************************************************/
static _mqx_int _dspi_dma_setparam
   (
        /* [IN] Device specific context structure */
        void                          *io_info_ptr,

        /* [IN] Parameters to set */
        SPI_PARAM_STRUCT_PTR           params
   )
{
    DSPI_DMA_INFO_STRUCT_PTR           dspi_info_ptr = (DSPI_DMA_INFO_STRUCT_PTR)io_info_ptr;
    VDSPI_REG_STRUCT_PTR               dspi_ptr = dspi_info_ptr->DSPI_PTR;

    BSP_CLOCK_CONFIGURATION clock_config;
    uint32_t clock_speed;

    uint32_t ctar;
    uint32_t cpol_invert;

    /* Transfer mode */
    if ((params->ATTR & SPI_ATTR_TRANSFER_MODE_MASK) != SPI_ATTR_MASTER_MODE)
        return SPI_ERROR_TRANSFER_MODE_INVALID;

    /* Set master mode */
    dspi_ptr->MCR |= DSPI_MCR_MSTR_MASK;

    clock_config = _bsp_get_clock_configuration();

    /* Check the parameter against most recent values to avoid time consuming baudrate finding routine */
    if ((dspi_info_ptr->CLOCK_CONFIG != clock_config) || (dspi_info_ptr->BAUDRATE != params->BAUDRATE))
    {
        dspi_info_ptr->CLOCK_CONFIG = clock_config;
        dspi_info_ptr->BAUDRATE = params->BAUDRATE;

        /* Find configuration of prescalers best matching the desired value */
        clock_speed = _bsp_get_clock(dspi_info_ptr->CLOCK_CONFIG, dspi_info_ptr->CLOCK_SOURCE);
        _dspi_find_baudrate(clock_speed, dspi_info_ptr->BAUDRATE, &(dspi_info_ptr->CTAR_TIMING));
    }

    /* Set up prescalers */
    ctar = dspi_info_ptr->CTAR_TIMING;

    /* Set up transfer parameters */
    _dspi_ctar_params(params, &ctar);

    /* Check whether it is necessary to invert idle clock polarity */
    cpol_invert = (dspi_ptr->CTAR[0] ^ ctar) & DSPI_CTAR_CPOL_MASK;

    /* Store to register */
    dspi_ptr->CTAR[0] = ctar;

    dspi_info_ptr->DUMMY_PATTERN = params->DUMMY_PATTERN;
    dspi_info_ptr->ATTR = params->ATTR;

    if (cpol_invert) {
        /* Dummy transfer with inactive CS to invert idle clock polarity */
        dspi_ptr->MCR = (dspi_ptr->MCR & ~(uint32_t)DSPI_MCR_PCSIS_MASK) | DSPI_MCR_PCSIS(0xFF);
        _dspi_dma_tx_rx(io_info_ptr, NULL, NULL, (params->FRAMESIZE+7)/8);
    }

    /* Set CS signals */
    dspi_ptr->MCR = (dspi_ptr->MCR & ~DSPI_MCR_PCSIS_MASK) | DSPI_MCR_PCSIS(~(params->CS));

    return SPI_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : _dspi_dma_callback
* Returned Value   : SPI DMA callback routine
* Comments         :
*   Notifies task about transfer completion.
*
*END*********************************************************************/
static void _dspi_dma_callback
    (
        /* [IN] The address of the device specific information */
        void                          *parameter,
        int                           tcds_done,
        uint32_t                      tcd_seq
    )
{
    DSPI_DMA_INFO_STRUCT_PTR           dspi_info_ptr = parameter;

   _lwsem_post(&dspi_info_ptr->EVENT_IO_FINISHED);

   return;
}


/*FUNCTION****************************************************************
*
* Function Name    : _dspi_dma_transfer
* Returned Value   : number of bytes transferred
* Comments         :
*   Internal routine performing actual DMA transfer of given width
*   If txbuf is NULL the function expects dummy pattern already prepared in dspi_info_ptr->TX_BUFFER
*
*END*********************************************************************/
static _mqx_int _dspi_dma_transfer
    (
        /* [IN] Device specific context structure */
        DSPI_DMA_INFO_STRUCT_PTR       dspi_info_ptr,

        /* [IN] Data to transmit */
        uint8_t                     *txbuf,

        /* [OUT] Received data */
        uint8_t                     *rxbuf,

        /* [IN] Length of transfer in bytes */
        uint32_t                        len,

        /* [IN] Width of data register access for DMA transfer */
        int                            regw
    )
{
    DMA_TCD                            tx_tcd;
    DMA_TCD                            rx_tcd;

    #if PSP_ENDIAN == MQX_LITTLE_ENDIAN
        if (regw > 1) {
            regw = -regw;
        }
    #endif

    if (NULL != rxbuf) {
        dma_tcd_reg2mem(&rx_tcd, &(dspi_info_ptr->DSPI_PTR->POPR), regw, rxbuf, len);
    }
    else {
        dma_tcd_reg2mem(&rx_tcd, &(dspi_info_ptr->DSPI_PTR->POPR), regw, dspi_info_ptr->RX_BUF, len);
        rx_tcd.LOOP_DST_OFFSET = -regw;
    }

    if (NULL != txbuf) {
        dma_tcd_mem2reg(&tx_tcd, &(dspi_info_ptr->DSPI_PTR->PUSHR), regw, txbuf, len);
    }
    else {
        dma_tcd_mem2reg(&tx_tcd, &(dspi_info_ptr->DSPI_PTR->PUSHR), regw, dspi_info_ptr->TX_BUF, len);
        tx_tcd.LOOP_SRC_OFFSET = -regw;
    }

    /* ensure that the semaphore is at zero count */
    while (_lwsem_poll(&dspi_info_ptr->EVENT_IO_FINISHED)) {}

    dma_transfer_submit(dspi_info_ptr->DMA_RX_CHANNEL, &rx_tcd, NULL);
    dma_transfer_submit(dspi_info_ptr->DMA_TX_CHANNEL, &tx_tcd, NULL);

    /* block the task until completion of the background operation */
    _lwsem_wait(&dspi_info_ptr->EVENT_IO_FINISHED);

    if (dma_channel_status(dspi_info_ptr->DMA_RX_CHANNEL, NULL, NULL)
        || dma_channel_status(dspi_info_ptr->DMA_TX_CHANNEL, NULL, NULL)
       )
    {
        return 0;
    }

    return len;
}


/*FUNCTION****************************************************************
*
* Function Name    : _dspi_dma_tx_rx
* Returned Value   : number of bytes transferred
* Comments         :
*   Actual transmit and receive function.
*   Overrun prevention used, no need to update statistics in this function
*
*END*********************************************************************/
#if PSP_HAS_DATA_CACHE

static _mqx_int _dspi_dma_tx_rx
    (
        /* [IN] Device specific context structure */
        void                          *io_info_ptr,

        /* [IN] Data to transmit */
        uint8_t                     *txbuf,

        /* [OUT] Received data */
        uint8_t                     *rxbuf,

        /* [IN] Length of transfer in bytes */
        uint32_t                        len
    )
{
    DSPI_DMA_INFO_STRUCT_PTR           dspi_info_ptr = (DSPI_DMA_INFO_STRUCT_PTR)io_info_ptr;
    int                                regw;

    uint32_t                            head_len;
    uint32_t                            tail_len;
    uint32_t                            zero_copy_len;

    _mqx_int                           result;

    /* Check whether there is at least something to transfer */
    if (0 == len) {
        return 0;
    }

    /* Check frame width */
    if (DSPI_CTAR_FMSZ_GET(dspi_info_ptr->DSPI_PTR->CTAR[0]) > 7)
    {
        len = len & (~1UL); /* Round down to whole frames */
        regw = 2;
    }
    else {
        regw = 1;
    }

    /* If there is no data to transmit, prepare dummy pattern in proper byte order */
    if (NULL == txbuf) {
        if (regw == 1) {
            dspi_info_ptr->TX_BUF[0] = dspi_info_ptr->DUMMY_PATTERN & 0xFF;
        }
        else {
            dspi_info_ptr->TX_BUF[0] = (dspi_info_ptr->DUMMY_PATTERN>>8) & 0xFF;
            dspi_info_ptr->TX_BUF[1] = dspi_info_ptr->DUMMY_PATTERN & 0xFF;
        }
        _dcache_flush_line(dspi_info_ptr->TX_BUF);
    }

    if (!(len % PSP_CACHE_LINE_SIZE) && !((uint32_t)txbuf % PSP_CACHE_LINE_SIZE) && !((uint32_t)rxbuf % PSP_CACHE_LINE_SIZE)) {
        /* Everything is perfectly aligned, perform single zero copy operation without any head or tail */
        head_len = 0;
        tail_len = 0;
    }
    else if (len <= 2*PSP_CACHE_LINE_SIZE) {
        /* The whole transfer fits into intermediate buffers, perform single transfer (head only) */
        head_len = len;
        tail_len = 0;
    }
    else {
        /* Split the transfer into head, zero copy portion and tail */
        uint32_t cache_line_offset;

        uint32_t tx_head_len;
        uint32_t tx_tail_len;

        uint32_t rx_head_len;
        uint32_t rx_tail_len;

        if (NULL != rxbuf) {
            cache_line_offset = (uint32_t)rxbuf % PSP_CACHE_LINE_SIZE;
            rx_head_len = cache_line_offset ? PSP_CACHE_LINE_SIZE - cache_line_offset : 0;
            rx_tail_len = (((uint32_t)rxbuf + len) % PSP_CACHE_LINE_SIZE);
        }
        else {
            rx_head_len = 0;
            rx_tail_len = 0;
        }

        if (NULL != txbuf) {
            cache_line_offset = (uint32_t)txbuf % PSP_CACHE_LINE_SIZE;
            tx_head_len = cache_line_offset ? PSP_CACHE_LINE_SIZE - cache_line_offset : 0;
            tx_tail_len = (((uint32_t)txbuf + len) % PSP_CACHE_LINE_SIZE);

        }
        else {
            tx_head_len = 0;
            tx_tail_len = 0;
        }

        head_len = (rx_head_len > tx_head_len) ? rx_head_len : tx_head_len;
        tail_len = (rx_tail_len > tx_tail_len) ? rx_tail_len : tx_tail_len;

        if (regw > 1) {
            head_len += (head_len & 1);
            tail_len += (tail_len & 1);
        }
    }

    zero_copy_len =  len - head_len - tail_len;

    /* Head processed through intermediate buffers */
    if (head_len) {
        if (txbuf) {
            _mem_copy(txbuf, dspi_info_ptr->TX_BUF, head_len);
            _dcache_flush_mlines(dspi_info_ptr->TX_BUF, len);
            result = _dspi_dma_transfer(dspi_info_ptr, dspi_info_ptr->TX_BUF, dspi_info_ptr->RX_BUF, head_len, regw);
        }
        else {
            result = _dspi_dma_transfer(dspi_info_ptr, NULL, dspi_info_ptr->RX_BUF, head_len, regw);
        }
        if (result != head_len) {
            return IO_ERROR;
        }
        /*
         * Copy to application buffer intentionally ommited.
         * It is done later after invalidation of zero copy area as it may overlap into it.
         */
    }

    /* Zero copy area */
    if (zero_copy_len) {
        uint8_t *txbuf_real;
        uint8_t *rxbuf_real;

        txbuf_real = txbuf ? txbuf + head_len : NULL;
        rxbuf_real = rxbuf ? rxbuf + head_len : NULL;

        if (txbuf_real) {
            _dcache_flush_mlines(txbuf_real, zero_copy_len);
        }
        result = _dspi_dma_transfer(dspi_info_ptr, txbuf_real, rxbuf_real, zero_copy_len, regw);
        if (rxbuf_real)
        {
            _dcache_invalidate_mlines(rxbuf_real, zero_copy_len);
        }

        if (result != zero_copy_len) {
            return IO_ERROR;
        }
    }

    /* Copy head data into application buffer if desired */
    if (head_len && rxbuf) {
       _dcache_invalidate_mlines(dspi_info_ptr->RX_BUF, head_len);
       _mem_copy(dspi_info_ptr->RX_BUF, rxbuf, head_len);
    }

    /* Tail processed through intermediate buffers */
    if (tail_len) {
        if (txbuf) {
            _mem_copy(txbuf + len - tail_len, dspi_info_ptr->TX_BUF, tail_len);
            _dcache_flush_mlines(dspi_info_ptr->TX_BUF, tail_len);
            result = _dspi_dma_transfer(dspi_info_ptr, dspi_info_ptr->TX_BUF, dspi_info_ptr->RX_BUF, tail_len, regw);
        }
        else {
            result = _dspi_dma_transfer(dspi_info_ptr, NULL, dspi_info_ptr->RX_BUF, tail_len, regw);
        }
        if (result != tail_len) {
            return IO_ERROR;
        }
        if (rxbuf) {
            _dcache_invalidate_mlines(dspi_info_ptr->RX_BUF, tail_len);
            _mem_copy(dspi_info_ptr->RX_BUF, rxbuf + len - tail_len, tail_len);
        }
    }

    return len;
}

#else /* PSP_HAS_DATA_CACHE */

static _mqx_int _dspi_dma_tx_rx
    (
        /* [IN] Device specific context structure */
        void                          *io_info_ptr,

        /* [IN] Data to transmit */
        uint8_t                     *txbuf,

        /* [OUT] Received data */
        uint8_t                     *rxbuf,

        /* [IN] Length of transfer in bytes */
        uint32_t                        len
    )
{
    DSPI_DMA_INFO_STRUCT_PTR           dspi_info_ptr = (DSPI_DMA_INFO_STRUCT_PTR)io_info_ptr;
    int                                regw;

    /* Check whether there is at least something to transfer */
    if (0 == len) {
        return 0;
    }

    /* Check frame width */
    if (DSPI_CTAR_FMSZ_GET(dspi_info_ptr->DSPI_PTR->CTAR[0]) > 7)
    {
        len = len & (~1UL); /* Round down to whole frames */
        regw = 2;
    }
    else {
        regw = 1;
    }

    /* If there is no data to transmit, prepare dummy pattern in proper byte order */
    if (NULL == txbuf) {
        if (regw == 1) {
            dspi_info_ptr->TX_BUF[0] = dspi_info_ptr->DUMMY_PATTERN & 0xFF;
        }
        else {
            dspi_info_ptr->TX_BUF[0] = (dspi_info_ptr->DUMMY_PATTERN>>8) & 0xFF;
            dspi_info_ptr->TX_BUF[1] = dspi_info_ptr->DUMMY_PATTERN & 0xFF;
        }
    }

    return _dspi_dma_transfer(dspi_info_ptr, txbuf, rxbuf, len, regw);
}

#endif /* PSP_HAS_DATA_CACHE */


/*FUNCTION****************************************************************
*
* Function Name    : _dspi_dma_cs_deassert
* Returned Value   :
* Comments         :
*   Deactivates chip select signals.
*
*END*********************************************************************/
static _mqx_int _dspi_dma_cs_deassert
    (
        /* [IN] The address of the device registers */
        void                          *io_info_ptr
    )
{
    DSPI_DMA_INFO_STRUCT_PTR           dspi_info_ptr = (DSPI_DMA_INFO_STRUCT_PTR)io_info_ptr;
    VDSPI_REG_STRUCT_PTR               dspi_ptr = dspi_info_ptr->DSPI_PTR;

    dspi_ptr->MCR = (dspi_ptr->MCR & ~(uint32_t)DSPI_MCR_PCSIS_MASK) | DSPI_MCR_PCSIS(0xFF);

    return MQX_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : _dspi_dma_ioctl
* Returned Value   : MQX error code
* Comments         :
*    This function performs miscellaneous services for
*    the SPI I/O device.
*
*END*********************************************************************/
static _mqx_int _dspi_dma_ioctl
    (
        /* [IN] The address of the device specific information */
        void                          *io_info_ptr,

        /* [IN] SPI transfer parameters */
        SPI_PARAM_STRUCT_PTR           params,

        /* [IN] The command to perform */
        uint32_t                        cmd,

        /* [IN] Parameters for the command */
        uint32_t                    *param_ptr
    )
{
    DSPI_DMA_INFO_STRUCT_PTR           dspi_info_ptr = (DSPI_DMA_INFO_STRUCT_PTR)io_info_ptr;
    uint32_t                            result = SPI_OK;

    BSP_CLOCK_CONFIGURATION clock_config;
    uint32_t clock_speed;

    switch (cmd)
    {
        case IO_IOCTL_SPI_GET_BAUD:
            clock_config = _bsp_get_clock_configuration();
            clock_speed = _bsp_get_clock(clock_config, dspi_info_ptr->CLOCK_SOURCE);
            *((uint32_t *)param_ptr) = _dspi_find_baudrate(clock_speed, *((uint32_t *)param_ptr), NULL);
            break;

        default:
            result = IO_ERROR_INVALID_IOCTL_CMD;
            break;
    }
    return result;
}

