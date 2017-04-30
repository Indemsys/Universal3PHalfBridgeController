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
#include "spi_dspi.h"
#include "spi_dspi_prv.h"


/* SPI low level driver interface functions */
static _mqx_int _dspi_init(const void  *init_data_ptr, void **io_info_ptr_ptr);
static _mqx_int _dspi_deinit(void *io_info_ptr);
static _mqx_int _dspi_setparam(void *io_info_ptr, SPI_PARAM_STRUCT_PTR params);
static _mqx_int _dspi_tx_rx(void *io_info_ptr, uint8_t *txbuf, uint8_t *rxbuf, uint32_t len);
static _mqx_int _dspi_cs_deassert(void *io_info_ptr);
static _mqx_int _dspi_ioctl(void *io_info_ptr, SPI_PARAM_STRUCT_PTR params, uint32_t cmd, uint32_t *param_ptr);

const SPI_DEVIF_STRUCT _spi_dspi_devif = {
    _dspi_init,
    _dspi_deinit,
    _dspi_setparam,
    _dspi_tx_rx,
    _dspi_cs_deassert,
    _dspi_ioctl
};


/* Forward declarations */
static void _dspi_isr(void *parameter);


/*FUNCTION****************************************************************
*
* Function Name    : _dspi_init
* Returned Value   : MQX error code
* Comments         :
*    This function initializes the SPI driver
*
*END*********************************************************************/
static _mqx_int _dspi_init
    (
        /* [IN] The initialization information for the device being opened */
        const void                *init_data_ptr,

        /* [OUT] The address to store device specific information */
        void                          **io_info_ptr_ptr
    )
{
    DSPI_INIT_STRUCT_PTR               dspi_init_ptr = (DSPI_INIT_STRUCT_PTR)init_data_ptr;

    DSPI_INFO_STRUCT_PTR               dspi_info_ptr;
    VDSPI_REG_STRUCT_PTR               dspi_ptr;

    const uint32_t                     *vectors;
    uint32_t                            i;

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
    dspi_info_ptr = (DSPI_INFO_STRUCT_PTR)_mem_alloc_system_zero((uint32_t)sizeof(DSPI_INFO_STRUCT));
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

    _lwsem_create(&dspi_info_ptr->EVENT_IO_FINISHED, 0);

    /* Install ISRs */
    dspi_info_ptr->NUM_VECTORS = _bsp_get_dspi_vectors(dspi_info_ptr->CHANNEL, &vectors);

    for (i=0; i<dspi_info_ptr->NUM_VECTORS; i++)
    {
        _int_install_isr(vectors[i], _dspi_isr, dspi_info_ptr);
        _bsp_int_init((PSP_INTERRUPT_TABLE_INDEX)vectors[i], BSP_DSPI_INT_LEVEL, 0, TRUE);
    }


    return SPI_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : _dspi_deinit
* Returned Value   : MQX error code
* Comments         :
*    This function de-initializes the SPI module
*
*END*********************************************************************/
static _mqx_int _dspi_deinit
    (
        /* [IN] the address of the device specific information */
        void                          *io_info_ptr
    )
{
    DSPI_INFO_STRUCT_PTR               dspi_info_ptr = (DSPI_INFO_STRUCT_PTR)io_info_ptr;

    const uint32_t                     *vectors;
    int                                num_vectors;
    int                                i;

    if (NULL == dspi_info_ptr)
    {
        return SPI_ERROR_DEINIT_FAILED;
    }

    _dspi_deinit_low(dspi_info_ptr->DSPI_PTR);

    /* Uninstall interrupt service routines */
    num_vectors = _bsp_get_dspi_vectors(dspi_info_ptr->CHANNEL, &vectors);

    for (i=0; i<num_vectors; i++)
    {
        /* Disable interrupt on vector */
        _bsp_int_disable(vectors[i]);
        /* Install default isr routine */
        _int_install_isr(vectors[i], _int_get_default_isr(), NULL);
    }

    _lwsem_destroy(&dspi_info_ptr->EVENT_IO_FINISHED);

    _mem_free(dspi_info_ptr);
    return SPI_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : _dspi_setparam
* Returned Value   :
* Comments         :
*    Set parameters for following transfers.
*
*END*********************************************************************/
static _mqx_int _dspi_setparam
   (
        /* [IN] Device specific context structure */
        void                          *io_info_ptr,

        /* [IN] Parameters to set */
        SPI_PARAM_STRUCT_PTR           params
   )
{
    DSPI_INFO_STRUCT_PTR               dspi_info_ptr = (DSPI_INFO_STRUCT_PTR)io_info_ptr;
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
        _dspi_tx_rx(io_info_ptr, NULL, NULL, (params->FRAMESIZE+7)/8);
    }

    /* Set CS signals */
    dspi_ptr->MCR = (dspi_ptr->MCR & ~DSPI_MCR_PCSIS_MASK) | DSPI_MCR_PCSIS(~(params->CS));

    return SPI_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : _dspi_isr
* Returned Value   : SPI interrupt routine
* Comments         :
*   State machine transferring data between buffers and DSPI FIFO.
*
*END*********************************************************************/
static void _dspi_isr
    (
        /* [IN] The address of the device specific information */
        void                          *parameter
    )
{
    DSPI_INFO_STRUCT_PTR               dspi_info_ptr = parameter;
    VDSPI_REG_STRUCT_PTR               dspi_ptr = dspi_info_ptr->DSPI_PTR;

    uint32_t                            data;

    /* drain RX FIFO */
    if (DSPI_CTAR_FMSZ_GET(dspi_ptr->CTAR[0]) > 7)
    {
        /* frame is larger than a single byte */
        while (dspi_ptr->SR & DSPI_SR_RFDF_MASK)
        {
            data = DSPI_POPR_RXDATA_GET(dspi_ptr->POPR);
            dspi_ptr->SR = DSPI_SR_RFDF_MASK;
            if (dspi_info_ptr->RX_LEN)
            {
                dspi_info_ptr->RX_LEN--;
                if (dspi_info_ptr->RX_BUF)
                {
                    dspi_info_ptr->RX_BUF[0] = (uint8_t)(data >> 8);
                    dspi_info_ptr->RX_BUF[1] = (uint8_t)(data & 0xff);
                    dspi_info_ptr->RX_BUF += 2;
                }
            }
        }
    }
    else
    {
        /* single byte frame */
        while (dspi_ptr->SR & DSPI_SR_RFDF_MASK)
        {
            data = DSPI_POPR_RXDATA_GET(dspi_ptr->POPR);
            dspi_ptr->SR = DSPI_SR_RFDF_MASK;
            if (dspi_info_ptr->RX_LEN)
            {
                dspi_info_ptr->RX_LEN--;
                if (dspi_info_ptr->RX_BUF)
                {
                    *(dspi_info_ptr->RX_BUF) = data;
                    dspi_info_ptr->RX_BUF++;
                }
            }
        }
    }

    /* check whether all requested data was received */
    if (dspi_info_ptr->RX_LEN == 0)
    {
        /* signalize finished job and disable further interrupts */
        dspi_ptr->RSER = 0;
        _lwsem_post(&dspi_info_ptr->EVENT_IO_FINISHED);
        return;
    }

    /* fill TX FIFO */
    if (DSPI_CTAR_FMSZ_GET(dspi_ptr->CTAR[0]) > 7)
    {
        /* frame is larger than a single byte */
        while (dspi_info_ptr->TX_LEN && (dspi_ptr->SR & DSPI_SR_TFFF_MASK) && ((dspi_info_ptr->RX_LEN-dspi_info_ptr->TX_LEN)<DSPI_FIFO_DEPTH))
        {
            dspi_info_ptr->TX_LEN--;
            if (dspi_info_ptr->TX_BUF)
            {
                data = dspi_info_ptr->TX_BUF[0];
                data = (data << 8) | dspi_info_ptr->TX_BUF[1];
                dspi_info_ptr->TX_BUF += 2;
            }
            else
            {
              data = dspi_info_ptr->DUMMY_PATTERN;
            }
            dspi_ptr->PUSHR = DSPI_PUSHR_TXDATA(data);
            dspi_ptr->SR = DSPI_SR_TFFF_MASK;
        }
    }
    else
    {
        /* single byte frame */
        while (dspi_info_ptr->TX_LEN && (dspi_ptr->SR & DSPI_SR_TFFF_MASK) && ((dspi_info_ptr->RX_LEN-dspi_info_ptr->TX_LEN)<DSPI_FIFO_DEPTH))
        {
            dspi_info_ptr->TX_LEN--;
            if (dspi_info_ptr->TX_BUF)
            {
                data = *(dspi_info_ptr->TX_BUF);
                dspi_info_ptr->TX_BUF ++;
            }
            else
            {
                data = dspi_info_ptr->DUMMY_PATTERN;
            }
            dspi_ptr->PUSHR = DSPI_PUSHR_TXDATA(data);
            dspi_ptr->SR = DSPI_SR_TFFF_MASK;
        }
    }
}


/*FUNCTION****************************************************************
*
* Function Name    : _dspi_tx_rx
* Returned Value   : number of bytes transferred
* Comments         :
*   Actual transmit and receive function.
*   Overrun prevention used, no need to update statistics in this function
*
*END*********************************************************************/
static _mqx_int _dspi_tx_rx
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
    DSPI_INFO_STRUCT_PTR               dspi_info_ptr = (DSPI_INFO_STRUCT_PTR)io_info_ptr;
    VDSPI_REG_STRUCT_PTR               dspi_ptr = dspi_info_ptr->DSPI_PTR;

    uint32_t                            tx_len;
    uint32_t                            rx_len;

    uint32_t                            data;

    bool                            use_isr;

    use_isr = dspi_info_ptr->NUM_VECTORS && dspi_info_ptr->ATTR & DSPI_ATTR_USE_ISR;

    data = dspi_info_ptr->DUMMY_PATTERN;

    /* Is frame larger than a single byte? */
    if (DSPI_CTAR_FMSZ_GET(dspi_ptr->CTAR[0]) > 7)
    {
        len = len & (~1UL);
        rx_len = tx_len = len/2;

        while (rx_len)
        {
            if (tx_len) {
                if ((dspi_ptr->SR & DSPI_SR_TFFF_MASK) && ((rx_len-tx_len)<DSPI_FIFO_DEPTH))
                {
                    if (txbuf)
                    {
                        data = *txbuf++;
                        data = (data << 8) | *txbuf++;
                    }
                    dspi_ptr->PUSHR = DSPI_PUSHR_TXDATA(data);
                    dspi_ptr->SR = DSPI_SR_TFFF_MASK;
                    tx_len--;
                }
                else if (use_isr)
                {
                    /* do not wait for RX data in a loop, break it and use ISR */
                    break;
                }
            }

            if (dspi_ptr->SR & DSPI_SR_RFDF_MASK)
            {
                data = DSPI_POPR_RXDATA_GET(dspi_ptr->POPR);
                dspi_ptr->SR = DSPI_SR_RFDF_MASK;
                if (rxbuf)
                {
                    *rxbuf++ = data >> 8;
                    *rxbuf++ = data & 0xff;
                }
                rx_len--;
            }
            else if (tx_len == 0 && use_isr)
            {
                /* do not wait for RX data in a loop, break it and use ISR */
                break;
            }
        }
    }
    else
    {
        rx_len = tx_len = len;

        /* Optimized loop for single byte frames */
        while (rx_len)
        {
            if (tx_len) {
                if ((dspi_ptr->SR & DSPI_SR_TFFF_MASK) && ((rx_len-tx_len)<DSPI_FIFO_DEPTH))
                {
                    if (txbuf)
                        data = *txbuf++;
                    dspi_ptr->PUSHR = DSPI_PUSHR_TXDATA(data);
                    dspi_ptr->SR = DSPI_SR_TFFF_MASK;
                    tx_len--;
                }
                else if (use_isr)
                {
                    /* do not wait for RX data in a loop, break it and use ISR */
                    break;
                }
            }

            if (dspi_ptr->SR & DSPI_SR_RFDF_MASK)
            {
                if (rxbuf)
                    *rxbuf++ = DSPI_POPR_RXDATA_GET(dspi_ptr->POPR);
                else
                    (void)dspi_ptr->POPR; /* dummy read to drain FIFO,  suppress 'expression has no side effect ' */
                dspi_ptr->SR = DSPI_SR_RFDF_MASK;
                rx_len--;
            }
            else if (tx_len == 0 && use_isr)
            {
                /* do not wait for RX data in a loop, break it and use ISR */
                break;
            }
        }
    }

    if (rx_len)
    {
        /* finish the transfer using ISR */
        dspi_info_ptr->TX_BUF = txbuf;
        dspi_info_ptr->TX_LEN = tx_len;
        dspi_info_ptr->RX_BUF = rxbuf;
        dspi_info_ptr->RX_LEN = rx_len;

        /* enable interrupt and wait for ISR state machine to finish the job */
        dspi_ptr->RSER = DSPI_RSER_RFDF_RE_MASK;
        _lwsem_wait(&dspi_info_ptr->EVENT_IO_FINISHED);
    }

    return len;
}


/*FUNCTION****************************************************************
*
* Function Name    : _dspi_cs_deassert
* Returned Value   :
* Comments         :
*   Deactivates chip select signals.
*
*END*********************************************************************/
static _mqx_int _dspi_cs_deassert
    (
        /* [IN] The address of the device registers */
        void                          *io_info_ptr
    )
{
    DSPI_INFO_STRUCT_PTR               dspi_info_ptr = (DSPI_INFO_STRUCT_PTR)io_info_ptr;
    VDSPI_REG_STRUCT_PTR               dspi_ptr = dspi_info_ptr->DSPI_PTR;

    dspi_ptr->MCR = (dspi_ptr->MCR & ~(uint32_t)DSPI_MCR_PCSIS_MASK) | DSPI_MCR_PCSIS(0xFF);

    return MQX_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : _dspi_ioctl
* Returned Value   : MQX error code
* Comments         :
*    This function performs miscellaneous services for
*    the SPI I/O device.
*
*END*********************************************************************/
static _mqx_int _dspi_ioctl
    (
        /* [IN] The address of the device specific information */
        void                          *io_info_ptr,

        /* [IN] SPI transfer parameters */
        SPI_PARAM_STRUCT_PTR          params,

        /* [IN] The command to perform */
        uint32_t                      cmd,

        /* [IN] Parameters for the command */
        uint32_t                      *param_ptr
    )
{
    (void)                          params; /* disable 'unused variable' warning */
    DSPI_INFO_STRUCT_PTR            dspi_info_ptr = (DSPI_INFO_STRUCT_PTR)io_info_ptr;
    uint32_t                        result = SPI_OK;

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
