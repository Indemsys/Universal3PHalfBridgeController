/*HEADER**********************************************************************
*
* Copyright 2009-2013 Freescale Semiconductor, Inc.
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
*   This file contains implementation of eDMA driver provided to other driver 
*
*
*END************************************************************************/

#include "mqx.h"
#include "bsp.h"
#include "io_prv.h"
#include "charq.h"
#include "fio_prv.h"
#include "serinprv.h"

/* DMA driver functions */

extern void _kuart_dma_write(IO_SERIAL_INT_DEVICE_STRUCT_PTR, char );
extern uint32_t _kuart_dma_init(IO_SERIAL_INT_DEVICE_STRUCT_PTR, char *);
extern uint32_t _kuart_dma_deinit(KUART_INIT_STRUCT_PTR, KUART_INFO_STRUCT_PTR);
extern uint32_t _kuart_dma_enable(KUART_INFO_STRUCT_PTR);
extern void    _kuart_dma_err_isr(void *);
extern void    _kuart_dma_tx_isr(void *, int tcd_done, uint32_t tcd_seq);
extern void    _kuart_dma_rx_isr(void *, int tcd_done, uint32_t tcd_seq);
/* Polled functions used */
extern uint32_t _kuart_polled_init(KUART_INIT_STRUCT_PTR, void ** , char *);
extern uint32_t _kuart_polled_deinit(KUART_INIT_STRUCT_PTR, KUART_INFO_STRUCT_PTR);
extern uint32_t _kuart_polled_ioctl(KUART_INFO_STRUCT_PTR, uint32_t, uint32_t *);
static void _kuart_prepare_tx_dma(IO_SERIAL_INT_DEVICE_STRUCT_PTR int_io_dev_ptr,uint32_t size);
static void _kuart_period_isr(void * data_ptr);
static void _kuart_prepare_rx_dma(IO_SERIAL_INT_DEVICE_STRUCT_PTR int_io_dev_ptr);



/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _kuart_dma_peripheral_enable
* Returned Value   : None
* Comments         :
*    Enables the SCI peripheral.
*
*END*----------------------------------------------------------------------*/

static void _kuart_dma_peripheral_enable
	 (
	     /* [IN] SCI channel */
	     UART_MemMapPtr sci_ptr
	 )
{
	 /* Enable only receive interrupt, transmit will be enabled during sending first character */
	 //TODO: why we need to enable the TX/RX in such an early step
	 sci_ptr->C2 |= UART_C2_RE_MASK | UART_C2_TE_MASK | UART_C2_TIE_MASK | UART_C2_RIE_MASK;
	 /* Enable dma request */
	 sci_ptr->C5 |= UART_C5_TDMAS_MASK | UART_C5_RDMAS_MASK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _kuart_dma_peripheral_disable
* Returned Value   : None
* Comments         :
*    Disables the SCI peripheral.
*
*END*----------------------------------------------------------------------*/

static void _kuart_dma_peripheral_disable
	 (
	     /* [IN] SCI channel */
	     UART_MemMapPtr sci_ptr
	 )
{
	 /* Transmitter and receiver disable */
	 sci_ptr->C2 &= (~ (UART_C2_RE_MASK | UART_C2_TE_MASK | UART_C2_TIE_MASK | UART_C2_RIE_MASK));
	 sci_ptr->C5 &= (~(UART_C5_TDMAS_MASK | UART_C5_RDMAS_MASK));
	 
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _kuart_dma_install
* Returned Value   : uint32_t a task error code or MQX_OK
* Comments         :
*    Install an dma driven uart serial device.
*
*END*----------------------------------------------------------------------*/

uint32_t _kuart_dma_install
	(
	   /* [IN] A string that identifies the device for fopen */
	   char * identifier,

	   /* [IN] The I/O init data void * */
	   KUART_INIT_STRUCT_CPTR  init_data_ptr,

	   /* [IN] The I/O queue size to use */
	   uint32_t  queue_size
	)
{ /* Body */

#if PE_LDD_VERSION
	 if (PE_PeripheralUsed((uint32_t)_bsp_get_serial_base_address(init_data_ptr->DEVICE)))
	 {
	     return IO_ERROR;
	 }
#endif

	return _io_serial_int_install(identifier,
	   (uint32_t (_CODE_PTR_)(void *, char *_))_kuart_dma_init,
	   (uint32_t (_CODE_PTR_)(void *))_kuart_dma_enable,
	   (uint32_t (_CODE_PTR_)(void *,void *))_kuart_dma_deinit,
	   (void    (_CODE_PTR_)(void *, char))_kuart_dma_write,
	   (uint32_t (_CODE_PTR_)(void *, uint32_t, void *))_kuart_polled_ioctl,
	   (void *)init_data_ptr, queue_size);

} /* Endbody */

uint32_t _kuart_dma_init
	(
	   /* [IN] the interrupt I/O initialization information */
	   IO_SERIAL_INT_DEVICE_STRUCT_PTR int_io_dev_ptr,

	   /* [IN] the rest of the name of the device opened */
	   char *                     open_name_ptr
	)
{ /* Body */
      KUART_INFO_STRUCT_PTR sci_info_ptr;
      KUART_INIT_STRUCT_PTR sci_init_ptr;
      uint32_t                     result = MQX_OK;

      MQX_TICK_STRUCT ticks, *p_ticks;

      p_ticks = &ticks;

      //Init basic uart setting
      sci_init_ptr = int_io_dev_ptr->DEV_INIT_DATA_PTR;
      result = _kuart_polled_init((void *)sci_init_ptr, &int_io_dev_ptr->DEV_INFO_PTR, open_name_ptr);
      if (result != MQX_OK) {
          return(result);
      }/* Endif */
      sci_info_ptr = int_io_dev_ptr->DEV_INFO_PTR;
      sci_info_ptr->TX_KEEP = 1;
      sci_info_ptr->RX_KEEP = 1;
      
      int_io_dev_ptr->TX_DMA_ONGOING = FALSE;
      _lwsem_create(&int_io_dev_ptr->LWSEM ,1);

      sci_info_ptr->RX_BUF = _mem_alloc_system_zero_uncached(sci_init_ptr->QUEUE_SIZE);
      if (!sci_info_ptr->RX_BUF) 
      {
              result = MQX_OUT_OF_MEMORY;
              goto error_free_buf;
      }
      sci_info_ptr->TX_BUF = _mem_alloc_system_zero_uncached(sci_init_ptr->QUEUE_SIZE);
      if (!sci_info_ptr->TX_BUF)
      {
              result = MQX_OUT_OF_MEMORY;
              goto error_free_buf;
      }
      sci_info_ptr->TX_DMA_CHAN = sci_init_ptr->TX_DMA_CHANNEL;
      sci_info_ptr->RX_DMA_CHAN = sci_init_ptr->RX_DMA_CHANNEL;
      sci_info_ptr->RX_DMA_HARDWARE_REQUEST = _bsp_get_serial_rx_dma_request(sci_init_ptr->DEVICE);
      sci_info_ptr->TX_DMA_HARDWARE_REQUEST = _bsp_get_serial_tx_dma_request(sci_init_ptr->DEVICE);
#if PSP_MQX_CPU_IS_KINETIS
      sci_info_ptr->ERR_INT = _bsp_get_serial_error_int_num(sci_init_ptr->DEVICE);
#endif
    sci_info_ptr->TX_DMA_SEQ = 0;
    sci_info_ptr->RX_DMA_SEQ = 0;
    sci_info_ptr->NUM_BYTES_RCVED_IN_CURRENT_RQST = 0;
    sci_info_ptr->NUM_BYTES_RQST = 0;
    result = dma_channel_claim(&sci_info_ptr->TX_DCH, sci_info_ptr->TX_DMA_CHAN);
    result = dma_callback_reg(sci_info_ptr->TX_DCH, _kuart_dma_tx_isr, int_io_dev_ptr);
    dma_channel_setup(sci_info_ptr->TX_DCH, 1, 0);
    dma_request_source(sci_info_ptr->TX_DCH, sci_info_ptr->TX_DMA_HARDWARE_REQUEST); 
    result = dma_channel_claim(&sci_info_ptr->RX_DCH, sci_info_ptr->RX_DMA_CHAN);
    result = dma_callback_reg(sci_info_ptr->RX_DCH, _kuart_dma_rx_isr, int_io_dev_ptr);
    dma_channel_setup(sci_info_ptr->RX_DCH, 1, 0);
    dma_request_source(sci_info_ptr->RX_DCH, sci_info_ptr->RX_DMA_HARDWARE_REQUEST);

    if (result != MQX_OK)
    {
            goto error_free_channel;
    }

    /* Init dma error vector */
    sci_info_ptr->OLD_ISR =
    _int_install_isr(sci_info_ptr->ERR_INT, _kuart_dma_err_isr, int_io_dev_ptr);
    _bsp_int_init(sci_info_ptr->ERR_INT, sci_init_ptr->ERR_PRIORITY, 0, TRUE);

    /* Add a timer config to check the UART RX state */
     
    sci_info_ptr->LW_TIMER_PTR = (LWTIMER_PERIOD_STRUCT_PTR)_mem_alloc_system_zero_uncached(sizeof(LWTIMER_PERIOD_STRUCT) + sizeof(LWTIMER_STRUCT));
    if (!sci_info_ptr->LW_TIMER_PTR)
    {
            result = MQX_OUT_OF_MEMORY;
            goto error_free_timer;
    }
    sci_info_ptr->LW_TIMER = (LWTIMER_STRUCT_PTR)((uint8_t *)sci_info_ptr->LW_TIMER_PTR + sizeof(LWTIMER_PERIOD_STRUCT));

    _time_init_ticks(p_ticks, 5);
    
    _lwtimer_create_periodic_queue(sci_info_ptr->LW_TIMER_PTR ,*(uint32_t *)p_ticks, 0);
    _lwtimer_add_timer_to_queue(
            sci_info_ptr->LW_TIMER_PTR, 
            sci_info_ptr->LW_TIMER,
            0,
            _kuart_period_isr,
            (void *)int_io_dev_ptr);
     
    /* config the UART RX channel first */
    _kuart_prepare_rx_dma(int_io_dev_ptr);      
    return(MQX_OK);

error_free_timer:
    if (!sci_info_ptr->LW_TIMER_PTR)
    _mem_free(sci_info_ptr->LW_TIMER_PTR);
    

error_free_channel:
    dma_channel_release(sci_info_ptr->TX_DCH);
    dma_channel_release(sci_info_ptr->RX_DCH);
    
error_free_buf:
    if (sci_info_ptr->TX_BUF)
            _mem_free(sci_info_ptr->TX_BUF);
    if (sci_info_ptr->RX_BUF)
            _mem_free(sci_info_ptr->RX_BUF);

    return result;
} /* Endbody */

/*FUNCTION****************************************************************
*
* Function Name    : _kuart_dma_deinit
* Returned Value   : uint32_t a task error code or MQX_OK
* Comments         :
*    This function de-initializes the UART in dma mode.
*
*END*********************************************************************/

uint32_t _kuart_dma_deinit
	(
	   /* [IN] the interrupt I/O initialization information */
	   KUART_INIT_STRUCT_PTR io_init_ptr,

	   /* [IN] the address of the device specific information */
	   KUART_INFO_STRUCT_PTR io_info_ptr
	)
{ /* Body */
    _kuart_polled_deinit(io_init_ptr, io_info_ptr);
    dma_request_disable(io_info_ptr->TX_DCH);
    dma_request_disable(io_info_ptr->RX_DCH);
    dma_channel_release(io_info_ptr->TX_DCH);
    dma_channel_release(io_info_ptr->RX_DCH);

    if (io_info_ptr->RX_BUF)
            _mem_free(io_info_ptr->RX_BUF);

    if (io_info_ptr->TX_BUF)
            _mem_free(io_info_ptr->TX_BUF);

    _int_install_isr(io_info_ptr->ERR_INT, io_info_ptr->OLD_ISR, io_info_ptr->OLD_ISR_DATA);

    return(MQX_OK);

} /* Endbody */

/*FUNCTION****************************************************************
*
* Function Name    : _kuart_dma_enable
* Returned Value   : uint32_t a task error code or MQX_OK
* Comments         :
*    This function enables the UART interrupts mode.
*
*END*********************************************************************/

uint32_t _kuart_dma_enable
	(
	   /* [IN] the address of the device specific information */
	   KUART_INFO_STRUCT_PTR io_info_ptr
	)
{ /* Body */
    uint8_t                  flags = IO_PERIPHERAL_MODULE_ENABLE | IO_PERIPHERAL_CLOCK_ENABLE;
    UART_MemMapPtr          sci_ptr = io_info_ptr->SCI_PTR;

    /* Enable module clocks to be able to write registers */
    _bsp_serial_io_init (io_info_ptr->INIT.DEVICE, IO_PERIPHERAL_CLOCK_ENABLE);

    /* Enable/disable module */
    if (flags & IO_PERIPHERAL_MODULE_ENABLE)
    {
       _kuart_dma_peripheral_enable (sci_ptr);
    }
    else
    {
       _kuart_dma_peripheral_disable (sci_ptr);
    }

    /* Disable module clocks if required */
    if (flags & IO_PERIPHERAL_CLOCK_DISABLE)
    {
       _bsp_serial_io_init (io_info_ptr->INIT.DEVICE, IO_PERIPHERAL_CLOCK_DISABLE);
    }
    /* start the convert receive of RX */
    dma_request_enable(io_info_ptr->RX_DCH);
    
    return MQX_OK;

} /* Endbody */

/*FUNCTION****************************************************************
*
* Function Name    : _kuart_dma_err_isr
* Returned Value   : none
* Comments         :
*   interrupt handler for the serial error interrupts.
*
*************************************************************************/

void _kuart_dma_err_isr
	(
	   /* [IN] the address of the device specific information */
	   void * parameter
	)
{ /* Body */

    IO_SERIAL_INT_DEVICE_STRUCT_PTR        int_io_dev_ptr = parameter;
    KUART_INFO_STRUCT_PTR                  sci_info_ptr = int_io_dev_ptr->DEV_INFO_PTR;
    UART_MemMapPtr                         sci_ptr = sci_info_ptr->SCI_PTR;
    uint16_t                                stat;

    ++sci_info_ptr->INTERRUPTS;
    stat = sci_ptr->S1;

    if(stat & UART_S1_OR_MASK) {
       ++sci_info_ptr->RX_OVERRUNS;
    }
    if(stat & UART_S1_PF_MASK) {
       ++sci_info_ptr->RX_PARITY_ERRORS;
    }
    if(stat & UART_S1_NF_MASK) {
       ++sci_info_ptr->RX_NOISE_ERRORS;
    }
    if(stat & UART_S1_FE_MASK) {
       ++sci_info_ptr->RX_FRAMING_ERRORS;
    }

}  /* Endbody */

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _kuart_period_isr
* Returned Value   : void
* Comments         :
*    Periodic interrupt for uart.
*
*END*----------------------------------------------------------------------*/
static void _kuart_period_isr(void * data_ptr)
{
    IO_SERIAL_INT_DEVICE_STRUCT_PTR        int_io_dev_ptr = data_ptr;
    KUART_INFO_STRUCT_PTR                  sci_info_ptr = int_io_dev_ptr->DEV_INFO_PTR;
    uint32_t num, i, size;
    char cc;

    if (sci_info_ptr->RX_KEEP)
    {
        sci_info_ptr->RX_KEEP = 0;
    }
    else
    {
        dma_channel_status(sci_info_ptr->RX_DCH, &sci_info_ptr->RX_DMA_SEQ, &num);

        num = sci_info_ptr->NUM_BYTES_RQST - num;

        if (num > sci_info_ptr->NUM_BYTES_RCVED_IN_CURRENT_RQST)
        {
            for (i=sci_info_ptr->NUM_BYTES_RCVED_IN_CURRENT_RQST; i < num; i++)
            {
                cc = sci_info_ptr->RX_BUF[i];
                _io_serial_int_addc(int_io_dev_ptr, cc);
            }
            sci_info_ptr->RX_CHARS += num - sci_info_ptr->NUM_BYTES_RCVED_IN_CURRENT_RQST;
            sci_info_ptr->NUM_BYTES_RCVED_IN_CURRENT_RQST = num;
        }
    }
    if (sci_info_ptr->TX_KEEP)
    {
        if (!int_io_dev_ptr->TX_DMA_ONGOING)
        {
            size = _CHARQ_SIZE(int_io_dev_ptr->OUT_QUEUE);
            if (size)
            {
                _kuart_prepare_tx_dma(int_io_dev_ptr,size);
            }
        }
        sci_info_ptr->TX_KEEP = 0;
    }
	
}


/*FUNCTION****************************************************************
*
* Function Name    : _kuart_dma_tx_isr
* Returned Value   : none
* Comments         :
*  dma callback for the serial tx.
*
*************************************************************************/

void _kuart_dma_tx_isr
	(
	   /* [IN] the address of the device specific information */
	   void * parameter,
	   int tcd_done,
	   uint32_t tcd_seq
	)
{ /* Body */
    IO_SERIAL_INT_DEVICE_STRUCT_PTR        int_io_dev_ptr = parameter;
    KUART_INFO_STRUCT_PTR                  sci_info_ptr = int_io_dev_ptr->DEV_INFO_PTR;
    UART_MemMapPtr                         sci_ptr = sci_info_ptr->SCI_PTR;
    uint32_t num, size;
    uint16_t stat;

    /*Check RX errors*/
    stat = sci_ptr->S1;	 
    ++sci_info_ptr->INTERRUPTS;

    if(stat & UART_S1_OR_MASK) {
       ++sci_info_ptr->RX_OVERRUNS;
    }
    if(stat & UART_S1_PF_MASK) {
       ++sci_info_ptr->RX_PARITY_ERRORS;
    }
    if(stat & UART_S1_NF_MASK) {
       ++sci_info_ptr->RX_NOISE_ERRORS;
    }
    if(stat & UART_S1_FE_MASK) {
       ++sci_info_ptr->RX_FRAMING_ERRORS;
    }

    dma_channel_status(sci_info_ptr->TX_DCH, &sci_info_ptr->TX_DMA_SEQ, &num);         
    size = _CHARQ_SIZE(int_io_dev_ptr->OUT_QUEUE);
    if (size)
    {
        _kuart_prepare_tx_dma(int_io_dev_ptr,size);
    }
    else
    {
        _lwsem_wait(&int_io_dev_ptr->LWSEM);
        int_io_dev_ptr->TX_DMA_ONGOING = FALSE;
        _lwsem_post(&int_io_dev_ptr->LWSEM);
    }
    sci_info_ptr->TX_CHARS += num;
}  /* Endbody */

/*FUNCTION****************************************************************
*
* Function Name    : _kuart_dma_rx_isr
* Returned Value   : none
* Comments         :
*  dma callback for the serial rx.
*
*************************************************************************/
void _kuart_dma_rx_isr
(
   /* [IN] the address of the device specific information */
   void * parameter,
   int tcd_done,
   uint32_t tcd_seq
)
{ /* Body */
    IO_SERIAL_INT_DEVICE_STRUCT_PTR        int_io_dev_ptr = parameter;
    KUART_INFO_STRUCT_PTR                  sci_info_ptr = int_io_dev_ptr->DEV_INFO_PTR;
    uint32_t num, i;
    int8_t cc;

    ++sci_info_ptr->INTERRUPTS;
    dma_channel_status(sci_info_ptr->RX_DCH, &sci_info_ptr->RX_DMA_SEQ, &num);
    //edma_get_status(sci_info_ptr->RX_DMA_CHAN, &num);
    num = sci_info_ptr->NUM_BYTES_RQST - num;
    for (i=sci_info_ptr->NUM_BYTES_RCVED_IN_CURRENT_RQST; i < num; i++)
    {
        cc = sci_info_ptr->RX_BUF[i];
        _io_serial_int_addc(int_io_dev_ptr, cc);
    }
    _kuart_prepare_rx_dma(int_io_dev_ptr);
    sci_info_ptr->RX_CHARS += num - sci_info_ptr->NUM_BYTES_RCVED_IN_CURRENT_RQST;
    sci_info_ptr->NUM_BYTES_RCVED_IN_CURRENT_RQST = 0;
}  /* Endbody */


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _kuart_dma_write
* Returned Value   : void
* Comments         :
*    Write the data to queue or enable dma tansmit.
*
*END*----------------------------------------------------------------------*/
void _kuart_dma_write
(
    IO_SERIAL_INT_DEVICE_STRUCT_PTR int_io_dev_ptr,
    char                       c
)
{ /* Endbody */
    KUART_INFO_STRUCT_PTR                  sci_info_ptr;
    uint32_t size = 0;
    sci_info_ptr = int_io_dev_ptr->DEV_INFO_PTR;
    _CHARQ_ENQUEUE(int_io_dev_ptr->OUT_QUEUE, c);
    if (!int_io_dev_ptr->TX_DMA_ONGOING)
    {
        size = _CHARQ_SIZE(int_io_dev_ptr->OUT_QUEUE);
        if (size < int_io_dev_ptr->OUT_QUEUE->MAX_SIZE/2)
        {
            sci_info_ptr->TX_KEEP = 1;
            return;
        }
        _kuart_prepare_tx_dma(int_io_dev_ptr,size);
     }   
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _kuart_prepare_tx_dma
* Returned Value   : void
* Comments         :
*    Configure and enable tx dma tansmit.
*
*END*----------------------------------------------------------------------*/
static void _kuart_prepare_tx_dma(IO_SERIAL_INT_DEVICE_STRUCT_PTR int_io_dev_ptr,uint32_t size)
{
    DMA_TCD				tcd;
    KUART_INFO_STRUCT_PTR                  sci_info_ptr;
    UART_MemMapPtr                         sci_ptr;
    sci_info_ptr = int_io_dev_ptr->DEV_INFO_PTR;
    sci_ptr = sci_info_ptr->SCI_PTR;
    int8_t cc;
    uint32_t i;
    for (i=0; i<size; i++)
    {
       sci_info_ptr->TX_BUF[i] = _io_serial_int_nextc(int_io_dev_ptr);
    }
    cc = _io_serial_int_nextc(int_io_dev_ptr);
    if (cc >= 0)
    {
        /* force the interrupt to change the internal state*/
        size++;
        sci_info_ptr->TX_BUF[i] = cc; 
     }
            
    dma_tcd_mem2reg(&tcd, &(sci_ptr->D) , 1, sci_info_ptr->TX_BUF, size);
    dma_transfer_submit(sci_info_ptr->TX_DCH, &tcd, &sci_info_ptr->TX_DMA_SEQ);    
    dma_request_enable(sci_info_ptr->TX_DCH);
        
    sci_info_ptr->TX_KEEP = 0;
    _lwsem_wait(&int_io_dev_ptr->LWSEM);
    int_io_dev_ptr->TX_DMA_ONGOING = TRUE;
    _lwsem_post(&int_io_dev_ptr->LWSEM);
	
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _kuart_prepare_rx_dma
* Returned Value   : void
* Comments         :
*    Configure and enable rx dma tansmit.
*
*END*----------------------------------------------------------------------*/
static void _kuart_prepare_rx_dma(IO_SERIAL_INT_DEVICE_STRUCT_PTR int_io_dev_ptr)
{
    DMA_TCD						tcd;
    KUART_INFO_STRUCT_PTR                  sci_info_ptr;
    UART_MemMapPtr                         sci_ptr;

    sci_info_ptr = int_io_dev_ptr->DEV_INFO_PTR;
    sci_ptr = sci_info_ptr->SCI_PTR;
    sci_info_ptr->NUM_BYTES_RQST = int_io_dev_ptr->IN_QUEUE->MAX_SIZE;
    dma_tcd_reg2mem(&tcd, &(sci_ptr->D) , 1, sci_info_ptr->RX_BUF, sci_info_ptr->NUM_BYTES_RQST);
    dma_transfer_submit(sci_info_ptr->RX_DCH, &tcd, &sci_info_ptr->RX_DMA_SEQ);      
    dma_request_enable(sci_info_ptr->RX_DCH);
    sci_info_ptr->RX_KEEP = 1;
}

/* EOF */
