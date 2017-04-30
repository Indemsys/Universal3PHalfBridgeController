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
*   This file contains the functions for the polled serial I/O
*   low level functions for the MCF51XX SCI.
*
*
*END************************************************************************/

#include "mqx.h"
#include "bsp.h"
#include "io_prv.h"
#include "charq.h"
#include "fio_prv.h"
#include "serplprv.h"


/* Polled driver functions */
extern char    _kuart_polled_getc(KUART_INFO_STRUCT_PTR);
extern void    _kuart_polled_putc(KUART_INFO_STRUCT_PTR, char);
extern bool _kuart_polled_status(KUART_INFO_STRUCT_PTR);
extern uint32_t _kuart_polled_ioctl(KUART_INFO_STRUCT_PTR, uint32_t, uint32_t *);
extern uint32_t _kuart_polled_init(KUART_INIT_STRUCT_PTR, void **, char *);
extern uint32_t _kuart_polled_deinit(KUART_INIT_STRUCT_PTR, KUART_INFO_STRUCT_PTR);
extern uint32_t _kuart_change_baudrate(UART_MemMapPtr, uint32_t, uint32_t);

/* The maximum number of transmit and receive datawords that can be stored in the transmit buffer */
static const char TRANSMIT_FIFO_SIZE[] = {1, 4, 8, 16, 32, 64, 128};
static const char RECEIVE_FIFO_SIZE[] = {1, 4, 8, 16, 32, 64, 128};


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _kuart_change_baudrate
* Returned Value   : MQX error code
* Comments         :
*    Calculates and sets new baudrate dividers for given SCI channel.
*
*END*----------------------------------------------------------------------*/

uint32_t _kuart_change_baudrate
    (
        /* [IN] SCI channel registers */
        UART_MemMapPtr sci_ptr,

        /* [IN] SCI input clock frequency */
        uint32_t        clock_frequency,

        /* [IN] Requested baud rate */
        uint32_t        baud_rate
    )
{
    uint32_t            baud_divisor = 0, brfa = 0;

    if ((clock_frequency > 0) && (baud_rate > 0))
    {
        baud_divisor = clock_frequency / (baud_rate << 4);
        if (baud_divisor > 0)
        {
            brfa = ((((clock_frequency - baud_rate*(baud_divisor << 4)) << 1) + (baud_rate >> 1)) / baud_rate);
            if (brfa >= 32) {
                baud_divisor += 1;
                brfa = 0;
            }
        }
        if (baud_divisor > ((UART_BDH_SBR_MASK << 8) | UART_BDL_SBR_MASK))
        {
            return MQX_INVALID_CONFIGURATION;
        }
    }
    sci_ptr->BDH = (unsigned char)((baud_divisor >> 8) & UART_BDH_SBR_MASK);
    sci_ptr->BDL = (unsigned char)(baud_divisor & UART_BDL_SBR_MASK);
    sci_ptr->C4 &= (~ UART_C4_BRFA_MASK);
    sci_ptr->C4 |= UART_C4_BRFA(brfa);
    return MQX_OK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _kuart_polled_peripheral_enable
* Returned Value   : None
* Comments         :
*    Enables the SCI peripheral.
*
*END*----------------------------------------------------------------------*/

static void _kuart_polled_peripheral_enable
    (
        /* [IN] SCI channel registers */
        UART_MemMapPtr sci_ptr
    )
{
    /* Transmitter and receiver enable */
    sci_ptr->C2 |= UART_C2_RE_MASK | UART_C2_TE_MASK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _kuart_polled_peripheral_disable
* Returned Value   : None
* Comments         :
*    Disables the SCI peripheral.
*
*END*----------------------------------------------------------------------*/

static void _kuart_polled_peripheral_disable
    (
        /* [IN] SCI channel registers */
        UART_MemMapPtr sci_ptr
    )
{
    /* Transmitter and receiver disable */
    sci_ptr->C2 &= (~ (UART_C2_RE_MASK | UART_C2_TE_MASK));
}


#if MQX_ENABLE_LOW_POWER

extern LPM_NOTIFICATION_RESULT _io_serial_polled_clock_configuration_callback (LPM_NOTIFICATION_STRUCT_PTR, void *);
extern LPM_NOTIFICATION_RESULT _io_serial_polled_operation_mode_callback (LPM_NOTIFICATION_STRUCT_PTR, void *);

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _io_serial_polled_clock_configuration_callback
* Returned Value   : Notification error code
* Comments         :
*    Low power clock configuration callback for polled serial.
*
*END*----------------------------------------------------------------------*/

LPM_NOTIFICATION_RESULT _io_serial_polled_clock_configuration_callback
    (
        /* [IN] Low power notification */
        LPM_NOTIFICATION_STRUCT_PTR      notification,

        /* [IN/OUT] Device specific data */
        void                            *device_specific_data
    )
{
    IO_SERIAL_POLLED_DEVICE_STRUCT_PTR dev_ptr = device_specific_data;
    KUART_INIT_STRUCT_PTR              init_data = dev_ptr->DEV_INIT_DATA_PTR;
    uint8_t                             flags;

    /* Get the device registers */
    UART_MemMapPtr sci_ptr = _bsp_get_serial_base_address (init_data->DEVICE);
    if (NULL == sci_ptr)
    {
        return LPM_NOTIFICATION_RESULT_ERROR;
    }

    if (LPM_NOTIFICATION_TYPE_PRE == notification->NOTIFICATION_TYPE)
    {
        /* Lock access from the higher level */
        _lwsem_wait (&(dev_ptr->LPM_INFO.LOCK));

        /* Enable module clocks to be able to write registers */
        _bsp_serial_io_init (init_data->DEVICE, IO_PERIPHERAL_CLOCK_ENABLE);

        /* Flush output if enabled */
        if (sci_ptr->C2 & UART_C2_TE_MASK)
        {
            while (! (sci_ptr->SFIFO & UART_SFIFO_TXEMPT_MASK))
                { };
            while (! (sci_ptr->S1 & UART_S1_TC_MASK))
                { };
        }

        /* Turn off module */
        _kuart_polled_peripheral_disable (sci_ptr);
    }

    if (LPM_NOTIFICATION_TYPE_POST == notification->NOTIFICATION_TYPE)
    {
        /* Enable module clocks to be able to write registers */
        _bsp_serial_io_init (init_data->DEVICE, IO_PERIPHERAL_CLOCK_ENABLE);

        /* Setup same baudrate for new clock frequency */
        _kuart_change_baudrate (sci_ptr, _cm_get_clock (notification->CLOCK_CONFIGURATION, init_data->CLOCK_SOURCE), init_data->BAUD_RATE);

        /* Turn on module if requested */
        flags = init_data->OPERATION_MODE[notification->OPERATION_MODE].FLAGS;
        if ((flags & IO_PERIPHERAL_MODULE_ENABLE)
          && (dev_ptr->COUNT > 0))
        {
            _kuart_polled_peripheral_enable (sci_ptr);
        }

        /* Disable module clocks if required */
        if (flags & IO_PERIPHERAL_CLOCK_DISABLE)
        {
            _bsp_serial_io_init (init_data->DEVICE, IO_PERIPHERAL_CLOCK_DISABLE);
        }

        /* Unlock */
        _lwsem_post (&(dev_ptr->LPM_INFO.LOCK));
    }
    return LPM_NOTIFICATION_RESULT_OK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _io_serial_polled_operation_mode_callback
* Returned Value   : Notification error code
* Comments         :
*    Low power operation mode callback for polled serial.
*
*END*----------------------------------------------------------------------*/

LPM_NOTIFICATION_RESULT _io_serial_polled_operation_mode_callback
    (
        /* [IN] Low power notification */
        LPM_NOTIFICATION_STRUCT_PTR       notification,

        /* [IN/OUT] Device specific data */
        void                             *device_specific_data
    )
{
    IO_SERIAL_POLLED_DEVICE_STRUCT_PTR  dev_ptr = device_specific_data;
    KUART_INIT_STRUCT_PTR               init_data = dev_ptr->DEV_INIT_DATA_PTR;
    uint8_t                              flags, tmp, bits;

    if (LPM_NOTIFICATION_TYPE_PRE == notification->NOTIFICATION_TYPE)
    {
        /* Get the device registers */
        UART_MemMapPtr sci_ptr = _bsp_get_serial_base_address (init_data->DEVICE);
        if (NULL == sci_ptr)
        {
            return LPM_NOTIFICATION_RESULT_ERROR;
        }

        /* Lock access from the higher level */
        _lwsem_wait (&(dev_ptr->LPM_INFO.LOCK));

        /* Get required HW changes */
        flags = init_data->OPERATION_MODE[notification->OPERATION_MODE].FLAGS;

        /* Setup module clock to be able to write registers */
        _bsp_serial_io_init (init_data->DEVICE, IO_PERIPHERAL_CLOCK_ENABLE);

        /* Flush output if enabled */
        if (sci_ptr->C2 & UART_C2_TE_MASK)
        {
            while (! (sci_ptr->SFIFO & UART_SFIFO_TXEMPT_MASK))
                { };
            while (! (sci_ptr->S1 & UART_S1_TC_MASK))
                { };
        }

        /* Setup pin mux */
        _bsp_serial_io_init (init_data->DEVICE, flags & (~ IO_PERIPHERAL_CLOCK_DISABLE));

        /* Setup wakeup */
        if (flags & IO_PERIPHERAL_WAKEUP_ENABLE)
        {
            bits = init_data->OPERATION_MODE[notification->OPERATION_MODE].WAKEUP_BITS;
            tmp = sci_ptr->C1 & (~ (UART_C1_WAKE_MASK | UART_C1_ILT_MASK));
            sci_ptr->C1 = tmp | (bits & (UART_C1_WAKE_MASK | UART_C1_ILT_MASK));
            tmp = sci_ptr->C4 & (~ (UART_C4_MAEN1_MASK | UART_C4_MAEN2_MASK));
            sci_ptr->C4 = tmp | (bits & (UART_C4_MAEN1_MASK | UART_C4_MAEN2_MASK));
            sci_ptr->MA1 = init_data->OPERATION_MODE[notification->OPERATION_MODE].MA1;
            sci_ptr->MA2 = init_data->OPERATION_MODE[notification->OPERATION_MODE].MA2;
            tmp = sci_ptr->C2 & (~ (UART_C2_RWU_MASK));
            sci_ptr->C2 = tmp | (bits & UART_C2_RWU_MASK);
        }
        else
        {
            sci_ptr->C2 &= (~ (UART_C2_RWU_MASK));
            sci_ptr->C1 &= (~ (UART_C1_WAKE_MASK | UART_C1_ILT_MASK));
            sci_ptr->C4 &= (~ (UART_C4_MAEN1_MASK | UART_C4_MAEN2_MASK));
            sci_ptr->MA1 = 0;
            sci_ptr->MA2 = 0;
        }

        /* Enable/disable module according to both clock configuration and operation mode */
        if ((flags & IO_PERIPHERAL_MODULE_ENABLE) && (dev_ptr->COUNT > 0))
        {
            _kuart_polled_peripheral_enable (sci_ptr);
        }
        else
        {
            _kuart_polled_peripheral_disable (sci_ptr);
        }

        /* Disable module clocks if required */
        if (flags & IO_PERIPHERAL_CLOCK_DISABLE)
        {
            _bsp_serial_io_init (init_data->DEVICE, IO_PERIPHERAL_CLOCK_DISABLE);
        }

        /* Unlock */
        _lwsem_post (&(dev_ptr->LPM_INFO.LOCK));
    }
    return LPM_NOTIFICATION_RESULT_OK;
}

#endif


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _kuart_polled_install
* Returned Value   : uint32_t a task error code or MQX_OK
* Comments         :
*    Install a polled sci device.
*
*END*----------------------------------------------------------------------*/

uint32_t _kuart_polled_install
   (
      /* [IN] A string that identifies the device for fopen */
      char           *identifier,

      /* [IN] The I/O init data pointer */
      KUART_INIT_STRUCT_CPTR            init_data_ptr,

      /* [IN] The I/O queue size to use */
      uint32_t            queue_size
   )
{ /* Body */

#if PE_LDD_VERSION
    if (PE_PeripheralUsed((uint32_t)_bsp_get_serial_base_address(init_data_ptr->DEVICE)))
    {
        return IO_ERROR;
    }
#endif

    return _io_serial_polled_install(identifier,
      (uint32_t (_CODE_PTR_)(void *, void **, char *))_kuart_polled_init,
      (uint32_t (_CODE_PTR_)(void *, void *))_kuart_polled_deinit,
      (char    (_CODE_PTR_)(void *))_kuart_polled_getc,
      (void    (_CODE_PTR_)(void *, char))_kuart_polled_putc,
      (bool (_CODE_PTR_)(void *))_kuart_polled_status,
      (uint32_t (_CODE_PTR_)(void *, uint32_t, void *))_kuart_polled_ioctl,
      (void *)init_data_ptr, queue_size);

} /* Endbody */



/*FUNCTION****************************************************************
*
* Function Name    : _kuart_polled_init
* Returned Value   : MQX_OK or a MQX error code.
* Comments         :
*    This function initializes the SCI
*
*END*********************************************************************/

uint32_t _kuart_polled_init
   (
      /* [IN] the initialization information for the device being opened */
      KUART_INIT_STRUCT_PTR               io_init_ptr,

      /* [OUT] the address to store device specific information */
      void                          **io_info_ptr_ptr,

      /* [IN] the rest of the name of the device opened */
      char                           *open_name_ptr
   )
{ /* Body */
   UART_MemMapPtr                       sci_ptr;
   KUART_INFO_STRUCT_PTR                sci_info_ptr;
   uint32_t                              channel, clock;
   uint8_t                               flags;

#if MQX_ENABLE_LOW_POWER
   uint32_t                              mode;
#endif

   /* Get peripheral address */
   channel = io_init_ptr->DEVICE;

   sci_ptr = _bsp_get_serial_base_address(channel);
   if (sci_ptr == NULL)
   {
      return MQX_INVALID_IO_CHANNEL;
   }

   sci_info_ptr = _mem_alloc_system_zero((uint32_t)sizeof(KUART_INFO_STRUCT));

#if MQX_CHECK_MEMORY_ALLOCATION_ERRORS
   if ( sci_info_ptr == NULL )
   {
      return MQX_OUT_OF_MEMORY;
   }
#endif

   *io_info_ptr_ptr = sci_info_ptr;

   sci_info_ptr->SCI_PTR = sci_ptr;

   /* Save initialization values */
   sci_info_ptr->INIT = *io_init_ptr;
   
   sci_info_ptr->FLAGS = IO_SERIAL_STOP_BITS_1;

   /* Setup HW according to low power mode, if enabled */
   clock = sci_info_ptr->INIT.CLOCK_SPEED;
   flags = IO_PERIPHERAL_PIN_MUX_ENABLE | IO_PERIPHERAL_CLOCK_ENABLE | IO_PERIPHERAL_MODULE_ENABLE;

#if MQX_ENABLE_LOW_POWER

   mode = _lpm_get_operation_mode ();
   if (mode >= LPM_OPERATION_MODES)
   {
      return MQX_INVALID_CONFIGURATION;
   }
   flags = io_init_ptr->OPERATION_MODE[mode].FLAGS;

   clock = _lpm_get_clock_configuration ();
   if (clock >= BSP_CLOCK_CONFIGURATIONS)
   {
      return MQX_INVALID_CONFIGURATION;
   }


   clock = _cm_get_clock ((BSP_CLOCK_CONFIGURATION)clock, io_init_ptr->CLOCK_SOURCE);

#endif

   /* Enable HW */
   _bsp_serial_io_init (channel, flags);

   /* Enable module clocks to be able to write registers */
   _bsp_serial_io_init (channel, IO_PERIPHERAL_CLOCK_ENABLE);

   /* Setup baudrate */
   _kuart_change_baudrate (sci_ptr, clock, sci_info_ptr->INIT.BAUD_RATE);

   /* 8-bit mode. Normal operation */
   sci_ptr->C1 = 0;

   /* Disable wakeups */
   sci_ptr->C2 &= (~ (UART_C2_RWU_MASK));
   sci_ptr->C4 &= (~ (UART_C4_MAEN1_MASK | UART_C4_MAEN2_MASK));
   sci_ptr->MA1 = 0;
   sci_ptr->MA2 = 0;

   /* Disable all error interrupts */
   sci_ptr->C3 = 0;

   /* Set watermark in the almost full TX buffer */
   if (((sci_ptr->PFIFO & UART_PFIFO_TXFIFOSIZE_MASK) >> UART_PFIFO_TXFIFOSIZE_SHIFT) == 0)
   {
      /* 1 dataword in D */
      sci_ptr->TWFIFO = UART_TWFIFO_TXWATER(0);
   }
   else
   {
      uint8_t txsize = 1 << (((sci_ptr->PFIFO & UART_PFIFO_TXFIFOSIZE_MASK) >> UART_PFIFO_TXFIFOSIZE_SHIFT) + 1);

      /* Watermark for TX buffer generates interrupts below & equal to watermark */
      sci_ptr->TWFIFO = UART_TWFIFO_TXWATER(txsize - 1);
   }

   /* Watermark for RX buffer generates interrupts above & equal to watermark */
   sci_ptr->RWFIFO = UART_RWFIFO_RXWATER(1);

   /* both RE,TE must be disabled before enable FIFO */
   sci_ptr->C2 &= ~(UART_C2_RE_MASK | UART_C2_TE_MASK);

   /* Enable TX FIFO, enable RX FIFO */
   sci_ptr->PFIFO |= UART_PFIFO_TXFE_MASK | UART_PFIFO_RXFE_MASK;

   /* Flush RX / TX buffers */
   sci_ptr->CFIFO |= UART_CFIFO_RXFLUSH_MASK | UART_CFIFO_TXFLUSH_MASK;

   /* Module enable/disable */
   if (flags & IO_PERIPHERAL_MODULE_ENABLE)
   {
       _kuart_polled_peripheral_enable (sci_ptr);
   }
   else
   {
       _kuart_polled_peripheral_disable (sci_ptr);
   }

   /* Disable module clocks if required */
   if (flags & IO_PERIPHERAL_CLOCK_DISABLE)
   {
       _bsp_serial_io_init (channel, IO_PERIPHERAL_CLOCK_DISABLE);
   }

   return MQX_OK;
} /* Endbody */


/*FUNCTION****************************************************************
*
* Function Name    : _kuart_polled_deinit
* Returned Value   : MQX_OK or a mqx error code.
* Comments         :
*    This function de-initializes the SCI.
*
*END*********************************************************************/

uint32_t _kuart_polled_deinit
   (
      /* [IN] the initialization information for the device being opened */
      KUART_INIT_STRUCT_PTR io_init_ptr,

      /* [IN] the address of the device specific information */
      KUART_INFO_STRUCT_PTR io_info_ptr
   )
{ /* Body */
   uint8_t flags = IO_PERIPHERAL_CLOCK_ENABLE;

#if MQX_ENABLE_LOW_POWER

   uint32_t mode = _lpm_get_operation_mode ();
   if (mode >= LPM_OPERATION_MODES)
   {
      return MQX_INVALID_CONFIGURATION;
   }
   flags = io_info_ptr->INIT.OPERATION_MODE[mode].FLAGS;

#endif

   /* Enable module clocks to be able to write registers */
   _bsp_serial_io_init (io_init_ptr->DEVICE, IO_PERIPHERAL_CLOCK_ENABLE);

   /* Disable interrupts, transmitter, receiver */
   io_info_ptr->SCI_PTR->C2 = 0;

   /* Disable module clocks if required */
   if (flags & IO_PERIPHERAL_CLOCK_DISABLE)
   {
       _bsp_serial_io_init (io_init_ptr->DEVICE, IO_PERIPHERAL_CLOCK_DISABLE);
   }

   _mem_free(io_info_ptr);

   return(MQX_OK);
} /* Endbody */


/*FUNCTION****************************************************************
*
* Function Name    : _kuart_polled_getc
* Returned Value   : char
* Comments         :
*   Return a character when it is available.  This function polls the
* device for input.
*
*************************************************************************/

char _kuart_polled_getc
   (
      /* [IN] the address of the device specific information */
      KUART_INFO_STRUCT_PTR io_info_ptr
   )
{ /* Body */
    UART_MemMapPtr sci_ptr = io_info_ptr->SCI_PTR;

   /* Wait while buffer is empty */
   while (!(sci_ptr->S1 & UART_S1_RDRF_MASK)) {
      /* Wait while buffer is empty */
   } /* Endwhile */

   io_info_ptr->RX_CHARS++;

   return sci_ptr->D;
} /* Endbody */


/*FUNCTION****************************************************************
*
* Function Name    : _kuart_polled_putc
* Returned Value   : void
* Comments         :
*   Writes the provided character.
*
*END*********************************************************************/

void _kuart_polled_putc
   (
      /* [IN] the address of the device specific information */
      KUART_INFO_STRUCT_PTR io_info_ptr,

      /* [IN] the character to write */
      char                               c
   )
{
   UART_MemMapPtr sci_ptr = io_info_ptr->SCI_PTR;
   
   while (!(sci_ptr->S1 & UART_S1_TDRE_MASK)) {
      /* Wait while buffer is full */
   } /* Endwhile */
   
   sci_ptr->D = c;

   io_info_ptr->TX_CHARS++;
}


/*FUNCTION****************************************************************
*
* Function Name    : _kuart_polled_status
* Returned Value   : bool
* Comments         :
*            This function returns TRUE if a character is available
*            on the on I/O device, otherwise it returns FALSE.
*
*END*********************************************************************/

bool _kuart_polled_status
   (
      /* [IN] the address of the device specific information */
      KUART_INFO_STRUCT_PTR io_info_ptr
   )
{ /* Body */
   KUART_INFO_STRUCT_PTR      sci_info_ptr = io_info_ptr;
   UART_MemMapPtr             sci_ptr = io_info_ptr->SCI_PTR;
   uint16_t                    stat = sci_ptr->S1;

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

   /*
   if "framming error" or "overrun" error occours 
   perform 'S1' cleanup. if not, 'S1' cleanup will be 
   performed during regular reading of 'D' register.
   */
   if (stat & (UART_S1_OR_MASK | UART_S1_FE_MASK))
   {
      // reading register 'D' to cleanup 'S1' may cause 'RFIFO' underflow
      sci_ptr->D;
      // if 'RFIFO' underflow detected, perform flush to reinitialize 'RFIFO'
      if (sci_ptr->SFIFO & UART_SFIFO_RXUF_MASK)
      {
         sci_ptr->CFIFO |= UART_CFIFO_RXFLUSH_MASK;
         sci_ptr->SFIFO |= UART_SFIFO_RXUF_MASK;
      }
      // set errno. some data may be still in 'RFIFO' so continue
      // TODO: add valid errno. IO_ERROR causes signed/unsigned assignment warning
      // _task_set_error(IO_ERROR);
   }

   if( (sci_ptr->SFIFO & UART_SFIFO_RXEMPT_MASK) == 0 )
        return TRUE;
   else
        return FALSE;
} /* Endbody */


/*FUNCTION****************************************************************
*
* Function Name    : _kuart_polled_ioctl
* Returned Value   : uint32_t MQX_OK or a mqx error code.
* Comments         :
*    This function performs miscellaneous services for
*    the I/O device.
*
*END*********************************************************************/

uint32_t _kuart_polled_ioctl
   (
      /* [IN] the address of the device specific information */
      KUART_INFO_STRUCT_PTR io_info_ptr,

      /* [IN] The command to perform */
      uint32_t                    cmd,

      /* [IN] Parameters for the command */
      uint32_t                *param_ptr
   )
{ /* Body */
   UART_MemMapPtr sci_ptr = io_info_ptr->SCI_PTR;
   uint32_t     tmp;
   uint32_t     channel = 0;
   (void)       channel; /* suppress 'unused variable' warning */

   /* Get peripheral address */
   channel = io_info_ptr->INIT.DEVICE;

   switch (cmd) {
      case IO_IOCTL_SERIAL_GET_DATA_BITS:
         tmp = 8;
         /* if 9bit mode */
         if (sci_ptr->C1 & UART_C1_M_MASK)
         {
            /* and bit8 not used as second stop bit */
            if (IO_SERIAL_STOP_BITS_1 == (io_info_ptr->FLAGS & IO_SERIAL_STOP_BITS_1))
            {
               /* and bit8 not used as parity */
               if ((0 == (sci_ptr->C1 & UART_C1_PE_MASK)) || (sci_ptr->C4 & UART_C4_M10_MASK))
               {
                  tmp++;
               }
            }
         }
         *param_ptr = tmp;
         break;

      case IO_IOCTL_SERIAL_SET_DATA_BITS:
         if ((8 != (*param_ptr)) && (9 != (*param_ptr)))
         {
            return MQX_INVALID_PARAMETER;
         }
         if (8 == (*param_ptr))
         {
            /* 2 stop bits means already in 8bit mode */
            if (IO_SERIAL_STOP_BITS_1 == (io_info_ptr->FLAGS & IO_SERIAL_STOP_BITS_1))
            {
               /* disable 10th bit */
               sci_ptr->C4 &= (~ UART_C4_M10_MASK);
               
               /* if parity, enable 9th bit, else disable 9th bit */
               if (sci_ptr->C1 & UART_C1_PE_MASK)
               {
                  sci_ptr->C1 |= UART_C1_M_MASK;
               }
               else
               {
                  sci_ptr->C1 &= (~ UART_C1_M_MASK);
               }
            }
         }
         else
         {
            /* 9bit mode is available only when using 1 stop bit */
            if (IO_SERIAL_STOP_BITS_1 == (io_info_ptr->FLAGS & IO_SERIAL_STOP_BITS_1))
            {
               /* enable 9th bit */
               sci_ptr->C1 |= UART_C1_M_MASK;
               
               /* if parity, enable 10th bit, else disable 10th bit */
               if (sci_ptr->C1 & UART_C1_PE_MASK) 
               {
                  sci_ptr->C4 |= UART_C4_M10_MASK;
               }
               else
               {
                  sci_ptr->C4 &= (~ UART_C4_M10_MASK);
               }
            }
            else
            {
               return MQX_INVALID_CONFIGURATION;
            }
         }
         break;

      case IO_IOCTL_SERIAL_GET_BAUD:
         *param_ptr = io_info_ptr->INIT.BAUD_RATE;
         break;

      case IO_IOCTL_SERIAL_SET_BAUD:

#if MQX_ENABLE_LOW_POWER
         tmp = _lpm_get_clock_configuration ();
         if (tmp >= BSP_CLOCK_CONFIGURATIONS)
         {
            return MQX_INVALID_CONFIGURATION;
         }
         tmp = _cm_get_clock ((BSP_CLOCK_CONFIGURATION)tmp, io_info_ptr->INIT.CLOCK_SOURCE);
#else
         tmp = io_info_ptr->INIT.CLOCK_SPEED;
#endif

         tmp = _kuart_change_baudrate (sci_ptr, tmp, *param_ptr);
         if (MQX_OK != tmp)
         {
            return tmp;
         }
         io_info_ptr->INIT.BAUD_RATE = *param_ptr;
         break;

      case IO_IOCTL_SERIAL_GET_STATS:
         *param_ptr++ = io_info_ptr->INTERRUPTS;
         *param_ptr++ = io_info_ptr->RX_CHARS;
         *param_ptr++ = io_info_ptr->TX_CHARS;
         *param_ptr++ = io_info_ptr->RX_BREAKS;
         *param_ptr++ = io_info_ptr->RX_PARITY_ERRORS;
         *param_ptr++ = io_info_ptr->RX_FRAMING_ERRORS;
         *param_ptr++ = io_info_ptr->RX_OVERRUNS;
         *param_ptr++ = io_info_ptr->RX_DROPPED_INPUT;
         *param_ptr++ = io_info_ptr->RX_NOISE_ERRORS;
         break;

      case IO_IOCTL_SERIAL_CLEAR_STATS:
         io_info_ptr->INTERRUPTS = 0;
         io_info_ptr->RX_CHARS = 0;
         io_info_ptr->TX_CHARS = 0;
         io_info_ptr->RX_BREAKS = 0;
         io_info_ptr->RX_PARITY_ERRORS = 0;
         io_info_ptr->RX_FRAMING_ERRORS = 0;
         io_info_ptr->RX_OVERRUNS = 0;
         io_info_ptr->RX_DROPPED_INPUT = 0;
         io_info_ptr->RX_NOISE_ERRORS = 0;
         break;

      case IO_IOCTL_SERIAL_CAN_TRANSMIT:
         if (sci_ptr->TCFIFO < TRANSMIT_FIFO_SIZE[(sci_ptr->PFIFO & UART_PFIFO_TXFIFOSIZE_MASK) >> UART_PFIFO_TXFIFOSIZE_SHIFT])
             *param_ptr = 1;
         else
             *param_ptr = 0;
         break;

      case IO_IOCTL_SERIAL_CAN_RECEIVE:
         if(sci_ptr->RCFIFO < RECEIVE_FIFO_SIZE[(sci_ptr->PFIFO & UART_PFIFO_RXFIFOSIZE_MASK) >> UART_PFIFO_RXFIFOSIZE_SHIFT])
             *param_ptr = 1;
         else
             *param_ptr = 0;
         break;

      case IO_IOCTL_SERIAL_GET_PARITY:
         tmp = IO_SERIAL_PARITY_NONE;
         if (sci_ptr->C1 & UART_C1_PE_MASK) {
            if (sci_ptr->C1 & UART_C1_PT_MASK) {
               tmp = IO_SERIAL_PARITY_ODD;
            } else {
               tmp = IO_SERIAL_PARITY_EVEN;
            }
         }
         *param_ptr = tmp;
         break;

      case IO_IOCTL_SERIAL_SET_PARITY:
         switch (*param_ptr) {
            case IO_SERIAL_PARITY_NONE:
               /* disable parity only if enabled (once) */
               if (sci_ptr->C1 & UART_C1_PE_MASK)
               {
                  sci_ptr->C1 &= (~ (UART_C1_PE_MASK | UART_C1_PT_MASK));
                  
                  /* disable parity bit */
                  if (sci_ptr->C4 & UART_C4_M10_MASK)
                  {
                     sci_ptr->C4 &= (~ UART_C4_M10_MASK);
                  }
                  else
                  {
                     sci_ptr->C1 &= (~ UART_C1_M_MASK);
                  }
               }
               break;
            case IO_SERIAL_PARITY_ODD:
               /* can't enable parity in 2 stop bits mode */
               if (IO_SERIAL_STOP_BITS_1 != (io_info_ptr->FLAGS & IO_SERIAL_STOP_BITS_1))
               {
                  return MQX_INVALID_CONFIGURATION;
               }

               /* if parity already enabled, just set the requested one */
               if (sci_ptr->C1 & UART_C1_PE_MASK)
               {
                  sci_ptr->C1 |= UART_C1_PT_MASK;
               }
               else
               {
                  /* enable odd parity */
                  sci_ptr->C1 |= (UART_C1_PE_MASK | UART_C1_PT_MASK);
                  
                  /* enable parity bit */
                  if (sci_ptr->C1 & UART_C1_M_MASK)
                  {
                     sci_ptr->C4 |= UART_C4_M10_MASK;
                  }
                  else
                  {
                     sci_ptr->C4 &= (~ UART_C4_M10_MASK);
                     sci_ptr->C1 |= UART_C1_M_MASK;
                  }
               }
               break;
            case IO_SERIAL_PARITY_EVEN:
                /* can't enable parity in 2 stop bits mode */
               if (IO_SERIAL_STOP_BITS_1 != (io_info_ptr->FLAGS & IO_SERIAL_STOP_BITS_1))
               {
                  return MQX_INVALID_CONFIGURATION;
               }
               
               /* if parity already enabled, just set the requested one */
               if (sci_ptr->C1 & UART_C1_PE_MASK)
               {
                  sci_ptr->C1 &= (~ UART_C1_PT_MASK);
               }
               else
               {
                  /* enable even parity */
                  sci_ptr->C1 |= UART_C1_PE_MASK;
                  sci_ptr->C1 &= (~ UART_C1_PT_MASK);
                  
                  /* enable parity bit */
                  if (sci_ptr->C1 & UART_C1_M_MASK)
                  {
                     sci_ptr->C4 |= UART_C4_M10_MASK;
                  }
                  else
                  {
                     sci_ptr->C4 &= (~ UART_C4_M10_MASK);
                     sci_ptr->C1 |= UART_C1_M_MASK;
                  }
               }
               break;
            default:
               return MQX_INVALID_PARAMETER;
         }
         break;

      case IO_IOCTL_SERIAL_SET_FLAGS:
         // automatic HW flow control
         if (*param_ptr & IO_SERIAL_HW_485_FLOW_CONTROL) {
             // RTS hw flow control - support for RS485
             
             /* get serial device index and set GPIO pin functionality to RTS */
             _bsp_serial_rts_init( io_info_ptr->INIT.DEVICE );
             /* enable hardware support for 485 RTS pin drive */
             sci_ptr->MODEM &= (~(UART_MODEM_RXRTSE_MASK | UART_MODEM_TXCTSE_MASK));
             sci_ptr->MODEM |= UART_MODEM_TXRTSE_MASK;
         }
         else if (*param_ptr & IO_SERIAL_HW_FLOW_CONTROL) {
             // RTS, CTS hw flow control
             sci_ptr->MODEM &= ~UART_MODEM_TXRTSE_MASK;
             sci_ptr->MODEM |= (UART_MODEM_RXRTSE_MASK | UART_MODEM_TXCTSE_MASK);
         }
         else {
             sci_ptr->MODEM &= (~(UART_MODEM_RXRTSE_MASK | UART_MODEM_TXCTSE_MASK | UART_MODEM_TXRTSE_MASK));
         }
         
         break;

      case IO_IOCTL_SERIAL_DISABLE_RX:
         if( *(bool *)param_ptr == TRUE )
         {
            /* disable receiver */
            sci_ptr->C2 &=~UART_C2_RE_MASK;
         }
         else
         {
            /* enable receiver */
            sci_ptr->C2 |= UART_C2_RE_MASK;
         }
         break;

      case IO_IOCTL_SERIAL_WAIT_FOR_TC:
         /* wait for transmission end signal */
         while( ! (sci_ptr->S1 & UART_S1_TC_MASK) )
             { };
         break;

      case IO_IOCTL_FLUSH_OUTPUT:
         while (! (sci_ptr->SFIFO & UART_SFIFO_TXEMPT_MASK))
             { };
         while (! (sci_ptr->S1 & UART_S1_TC_MASK))
             { };
         break;

      case IO_IOCTL_SERIAL_GET_STOP_BITS:
         if (IO_SERIAL_STOP_BITS_1 == (io_info_ptr->FLAGS & IO_SERIAL_STOP_BITS_1))
         {
            *param_ptr = IO_SERIAL_STOP_BITS_1;
         }
         else
         {
            *param_ptr = IO_SERIAL_STOP_BITS_2;
         }
         break;

      case IO_IOCTL_SERIAL_SET_STOP_BITS:
         if (IO_SERIAL_STOP_BITS_1 == *param_ptr)
         {
            if (IO_SERIAL_STOP_BITS_1 != (io_info_ptr->FLAGS & IO_SERIAL_STOP_BITS_1))
            {
               /* back to initial 8bit settings */
               sci_ptr->C4 &= (~ UART_C4_M10_MASK);
               sci_ptr->C1 &= (~ UART_C1_M_MASK);
               sci_ptr->C3 &= (~ UART_C3_T8_MASK);
                 
               io_info_ptr->FLAGS |= IO_SERIAL_STOP_BITS_1;
            }
         } 
         else if (IO_SERIAL_STOP_BITS_2 == *param_ptr)
         {
            if (IO_SERIAL_STOP_BITS_1 == (io_info_ptr->FLAGS & IO_SERIAL_STOP_BITS_1))
            {
               /* 2 stop bits are impossible in 9bit data mode or with parity enabled */
               if ((sci_ptr->C1 & UART_C1_M_MASK) || (sci_ptr->C1 & UART_C1_PE_MASK))
               {
                  return MQX_INVALID_CONFIGURATION;
               }
                  
               /* enable 9bit mode */
               sci_ptr->C4 &= (~ UART_C4_M10_MASK);
               sci_ptr->C1 |= UART_C1_M_MASK;
                  
               /* last bit simulates second stop bit */
               sci_ptr->C3 |= UART_C3_T8_MASK;
                  
               io_info_ptr->FLAGS &= (~ IO_SERIAL_STOP_BITS_1);
            }
         }
         else
         {
            return MQX_INVALID_PARAMETER;
         }
         break;
#if PSP_MQX_CPU_IS_KINETIS
      case IO_IOCTL_SERIAL_SET_IRDA_TX:
          if( *(bool *)param_ptr == TRUE )
          {
              sci_ptr->IR |= UART_IR_IREN_MASK;
              if (MQX_OK != _bsp_serial_irda_tx_init(channel, TRUE)) {
                  return MQX_INVALID_CONFIGURATION;
              }
          }
          else
          {
              sci_ptr->IR &= ~UART_IR_IREN_MASK;
              if (MQX_OK != _bsp_serial_irda_tx_init(channel, FALSE)) {
                  return MQX_INVALID_CONFIGURATION;
              }
          }
          break;
      case IO_IOCTL_SERIAL_SET_IRDA_RX:
          if( *(bool *)param_ptr == TRUE )
          {
              sci_ptr->IR |= UART_IR_IREN_MASK;
              if (MQX_OK != _bsp_serial_irda_rx_init(channel,TRUE))
              {
                  return MQX_INVALID_CONFIGURATION;
              }
          }
          else
          {
              sci_ptr->IR &= ~UART_IR_IREN_MASK;
              if (MQX_OK != _bsp_serial_irda_rx_init(channel, FALSE)) {
                  return MQX_INVALID_CONFIGURATION;
              }
          }
          break;
#endif
      case IO_IOCTL_SERIAL_GET_CONFIG:
      {
         KUART_INIT_STRUCT_PTR io_init_ptr = (KUART_INIT_STRUCT_PTR)param_ptr;
         *io_init_ptr = io_info_ptr->INIT;
         break;
      }
      default:
         return IO_ERROR_INVALID_IOCTL_CMD;
   } /* End switch */

   return (MQX_OK);

} /* Endbody */

/* EOF */
