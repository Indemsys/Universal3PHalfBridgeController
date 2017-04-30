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
*   This file contains board-specific pin initialization functions.
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>
#include <bsp_prv.h>

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_serial_io_init
* Returned Value   : MQX_OK for success, -1 for failure
* Comments         :
*    This function performs BSP-specific initialization related to serial
*
*END*----------------------------------------------------------------------*/

_mqx_int _bsp_serial_io_init
(
 /* [IN] Serial device number */
 uint8_t dev_num,

 /* [IN] Required functionality */
 uint8_t flags
)
{
  SIM_MemMapPtr   sim = SIM_BASE_PTR;
  PORT_MemMapPtr  pctl;

  /* Setup GPIO for UART devices */
  switch (dev_num)
  {
  case 0:
    //pctl = (PORT_MemMapPtr)PORTF_BASE_PTR;
    if (flags & IO_PERIPHERAL_PIN_MUX_ENABLE)
    {
      // Перенаправить !!!
      // /* PTF17 as RX function (Alt.4) + drive strength */
      // pctl->PCR[17] = 0 | PORT_PCR_MUX(4) | PORT_PCR_DSE_MASK;
      // /* PTF18 as TX function (Alt.4) + drive strength */
      // pctl->PCR[18] = 0 | PORT_PCR_MUX(4) | PORT_PCR_DSE_MASK;
    }
    if (flags & IO_PERIPHERAL_PIN_MUX_DISABLE)
    {
      // Перенаправить !!!
      // /* PTF17 default */
      // pctl->PCR[17] = 0;
      // /* PTF18 default */
      // pctl->PCR[18] = 0;
    }
    if (flags & IO_PERIPHERAL_CLOCK_ENABLE)
    {
      /* start SGI clock */
      sim->SCGC4 |= SIM_SCGC4_UART0_MASK;
    }
    if (flags & IO_PERIPHERAL_CLOCK_DISABLE)
    {
      /* stop SGI clock */
      sim->SCGC4 &= (~SIM_SCGC4_UART0_MASK);
    }
    break;

  case 1:
    //pctl = (PORT_MemMapPtr)PORTC_BASE_PTR;
    if (flags & IO_PERIPHERAL_PIN_MUX_ENABLE)
    {
      // Перенаправить !!!
      // /* PTC3 as RX function (Alt.3) + drive strength */
      // pctl->PCR[3] = 0 | PORT_PCR_MUX(3) | PORT_PCR_DSE_MASK;
      // /* PTC4 as TX function (Alt.3) + drive strength */
      // pctl->PCR[4] = 0 | PORT_PCR_MUX(3) | PORT_PCR_DSE_MASK;
    }
    if (flags & IO_PERIPHERAL_PIN_MUX_DISABLE)
    {
      // Перенаправить !!!
      // /* PTC3 default */
      // pctl->PCR[3] = 0;
      // /* PTC4 default */
      // pctl->PCR[4] = 0;
    }
    if (flags & IO_PERIPHERAL_CLOCK_ENABLE)
    {
      /* start SGI clock */
      sim->SCGC4 |= SIM_SCGC4_UART1_MASK;
    }
    if (flags & IO_PERIPHERAL_CLOCK_DISABLE)
    {
      /* start SGI clock */
      sim->SCGC4 &= (~SIM_SCGC4_UART1_MASK);
    }
    break;

  case 2:
    //pctl = (PORT_MemMapPtr)PORTE_BASE_PTR;
    if (flags & IO_PERIPHERAL_PIN_MUX_ENABLE)
    {
      // Перенаправить !!!
      // /* PTE16 as TX function (Alt.3) + drive strength */
      // pctl->PCR[16] = 0 | PORT_PCR_MUX(3) | PORT_PCR_DSE_MASK;
      // /* PTE17 as RX function (Alt.3) + drive strength */
      // pctl->PCR[17] = 0 | PORT_PCR_MUX(3) | PORT_PCR_DSE_MASK;
    }
    if (flags & IO_PERIPHERAL_PIN_MUX_DISABLE)
    {
      // Перенаправить !!!
      // /* PTE16 default */
      // pctl->PCR[16] = 0;
      // /* PTE17 default */
      // pctl->PCR[17] = 0;
    }
    if (flags & IO_PERIPHERAL_CLOCK_ENABLE)
    {
      /* start SGI clock */
      sim->SCGC4 |= SIM_SCGC4_UART2_MASK;
    }
    if (flags & IO_PERIPHERAL_CLOCK_DISABLE)
    {
      /* stop SGI clock */
      sim->SCGC4 &= (~SIM_SCGC4_UART2_MASK);
    }
    break;

  case 3:
    //pctl = (PORT_MemMapPtr)PORTF_BASE_PTR;
    if (flags & IO_PERIPHERAL_PIN_MUX_ENABLE)
    {
      // Перенаправить !!!
      // /* PTF7 as RX function (Alt.4) + drive strength */
      // pctl->PCR[7] = 0 | PORT_PCR_MUX(4) | PORT_PCR_DSE_MASK;
      // /* PTF8 as TX function (Alt.4) + drive strength */
      // pctl->PCR[8] = 0 | PORT_PCR_MUX(4) | PORT_PCR_DSE_MASK;
    }
    if (flags & IO_PERIPHERAL_PIN_MUX_DISABLE)
    {
      // Перенаправить !!!
      // /* PTF7 default */
      // pctl->PCR[7] = 0;
      // /* PTC17 default */
      // pctl->PCR[8] = 0;
      // // /* PTC18 default */
    }
    if (flags & IO_PERIPHERAL_CLOCK_ENABLE)
    {
      /* start SGI clock */
      sim->SCGC4 |= SIM_SCGC4_UART3_MASK;
    }
    if (flags & IO_PERIPHERAL_CLOCK_DISABLE)
    {
      /* stop SGI clock */
      sim->SCGC4 &= (~SIM_SCGC4_UART3_MASK);
    }
    break;

  case 4:
    //pctl = (PORT_MemMapPtr)PORTE_BASE_PTR;
    if (flags & IO_PERIPHERAL_PIN_MUX_ENABLE)
    {
      // Перенаправить !!!
      // /* PTE25 as RX function (Alt.3) + drive strength */
      // pctl->PCR[25] = 0 | PORT_PCR_MUX(3) | PORT_PCR_DSE_MASK;
      // /* PTE24 as TX function (Alt.3) + drive strength */
      // pctl->PCR[24] = 0 | PORT_PCR_MUX(3) | PORT_PCR_DSE_MASK;
    }
    if (flags & IO_PERIPHERAL_PIN_MUX_DISABLE)
    {
      // Перенаправить !!!
      // /* PTE25 default */
      // pctl->PCR[25] = 0;
      // /* PTE24 default */
      // pctl->PCR[24] = 0;
    }
    if (flags & IO_PERIPHERAL_CLOCK_ENABLE)
    {
      /* starting SGI clock */
      sim->SCGC1 |= SIM_SCGC1_UART4_MASK;
    }
    if (flags & IO_PERIPHERAL_CLOCK_DISABLE)
    {
      /* starting SGI clock */
      sim->SCGC1 &= (~SIM_SCGC1_UART4_MASK);
    }
    break;

  case 5:
    // pctl = (PORT_MemMapPtr)PORTF_BASE_PTR;
    if (flags & IO_PERIPHERAL_PIN_MUX_ENABLE)
    {
      // Перенаправить !!!
      // /* PTF20 as TX function (Alt.4) + drive strength */
      // pctl->PCR[20] = 0 | PORT_PCR_MUX(4) | PORT_PCR_DSE_MASK;
      // /* PTF19 as RX function (Alt.4) + drive strength */
      // pctl->PCR[19] = 0 | PORT_PCR_MUX(4) | PORT_PCR_DSE_MASK;
    }
    if (flags & IO_PERIPHERAL_PIN_MUX_DISABLE)
    {
      // Перенаправить !!!
      // /* PTF20 default */
      // pctl->PCR[20] = 0;
      // /* PTF19 default */
      // pctl->PCR[19] = 0;
    }
    if (flags & IO_PERIPHERAL_CLOCK_ENABLE)
    {
      /* starting SGI clock */
      //sim->SCGC1 |= SIM_SCGC1_UART5_MASK;
    }
    if (flags & IO_PERIPHERAL_CLOCK_DISABLE)
    {
      /* starting SGI clock */
      //sim->SCGC1 &= (~SIM_SCGC1_UART5_MASK);
    }
    break;

  default:
    return -1;
  }

  return 0;
}



/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_rtc_io_init
* Returned Value   : MQX_OK or -1
* Comments         :
*    This function performs BSP-specific initialization related to RTC
*
*END*----------------------------------------------------------------------*/

_mqx_int _bsp_rtc_io_init
(
 void
)
{

#if PE_LDD_VERSION
  /* Check if peripheral is not used by Processor Expert RTC_LDD component */
  if (PE_PeripheralUsed((uint32_t)RTC_BASE_PTR) == TRUE)
  {
    /* IO Device used by PE Component*/
    return IO_ERROR;
  }
#endif

  /* Enable the clock gate to the RTC module. */
  SIM_SCGC6 |= SIM_SCGC6_RTC_MASK;

  return MQX_OK;
}


/*-------------------------------------------------------------------------------------------------------------
  Функция вызывается в драйвере ESDHC для управления линиями интерефейса SDIO
  value = 0 - линни переведены в режим GPIO
  value = 1 - линииям назаначена функциональность контроллера SDIO
-------------------------------------------------------------------------------------------------------------*/
_mqx_int _bsp_esdhc_io_init
(
 uint8_t  dev_num,
 uint16_t value
)
{
  SIM_MemMapPtr   sim  = SIM_BASE_PTR;
  PORT_MemMapPtr  pctl;

  switch (dev_num)
  {
  case 0:
    /* Configure GPIO for SDHC peripheral function */
    pctl = (PORT_MemMapPtr)PORTE_BASE_PTR;
    pctl->PCR[0] = value & (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    /* ESDHC.D1  */
    pctl->PCR[1] = value & (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    /* ESDHC.D0  */
    pctl->PCR[2] = value & (PORT_PCR_MUX(4) | PORT_PCR_DSE_MASK);                                          /* ESDHC.CLK */
    pctl->PCR[3] = value & (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    /* ESDHC.CMD */
    pctl->PCR[4] = value & (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    /* ESDHC.D3  */
    pctl->PCR[5] = value & (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    /* ESDHC.D2  */

    /* Enable clock gate to SDHC module */
    sim->SCGC3 |= SIM_SCGC3_SDHC_MASK;
    break;

  default:
    /* Do nothing if bad dev_num was selected */
    return -1;
  }

  return MQX_OK;
}


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_i2c_io_init
* Returned Value   : MQX_OK or -1
* Comments         :
*    This function performs BSP-specific initialization related to I2C
*
*END*----------------------------------------------------------------------*/

_mqx_int _bsp_i2c_io_init
(
 uint32_t dev_num
)
{

  PORT_MemMapPtr  pctl;
  SIM_MemMapPtr sim = SIM_BASE_PTR;

  switch (dev_num)
  {
  case 0:
    sim->SCGC4 |= SIM_SCGC4_I2C0_MASK;
    break;
  default:
    /* Do nothing if bad dev_num was selected */
    return -1;
  }
  return MQX_OK;

}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_enet_io_init
* Returned Value   : MQX_OK or -1
* Comments         :
*    This function performs BSP-specific initialization related to ENET
*
*END*----------------------------------------------------------------------*/

_mqx_int _bsp_enet_io_init
(
 uint32_t device
)
{
  PORT_MemMapPtr pctl;
  SIM_MemMapPtr  sim  = (SIM_MemMapPtr)SIM_BASE_PTR;

  // Перенаправить !!!
  // pctl = (PORT_MemMapPtr)PORTA_BASE_PTR;
  // pctl->PCR[12] = 0x00000400;     /* PTA12, RMII0_RXD1/MII0_RXD1      */
  // pctl->PCR[13] = 0x00000400;     /* PTA13, RMII0_RXD0/MII0_RXD0      */
  // pctl->PCR[14] = 0x00000400;     /* PTA14, RMII0_CRS_DV/MII0_RXDV    */
  // pctl->PCR[15] = 0x00000400;     /* PTA15, RMII0_TXEN/MII0_TXEN      */
  // pctl->PCR[16] = 0x00000400;     /* PTA16, RMII0_TXD0/MII0_TXD0      */
  // pctl->PCR[17] = 0x00000400;     /* PTA17, RMII0_TXD1/MII0_TXD1      */
  //
  //
  // pctl = (PORT_MemMapPtr)PORTB_BASE_PTR;
  // pctl->PCR[0] = PORT_PCR_MUX(4) | PORT_PCR_ODE_MASK; /* PTB0, RMII0_MDIO/MII0_MDIO   */
  // pctl->PCR[1] = PORT_PCR_MUX(4);                     /* PTB1, RMII0_MDC/MII0_MDC     */

#if ENETCFG_SUPPORT_PTP
  // Перенаправить !!!
  // pctl = (PORT_MemMapPtr)PORTC_BASE_PTR;
  // pctl->PCR[16 + MACNET_PTP_TIMER] = PORT_PCR_MUX(4) | PORT_PCR_DSE_MASK; /* PTC16, ENET0_1588_TMR0   */
#endif

  /* Enable clock for ENET module */
  sim->SCGC2 |= SIM_SCGC2_ENET_MASK;

  return MQX_OK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_usb_io_init
* Returned Value   : MQX_OK or -1
* Comments         :
*    This function performs BSP-specific I/O initialization related to USB
*
*END*----------------------------------------------------------------------*/

static _mqx_int _bsp_usb_io_init(_mqx_uint dev_num)
{
  return MQX_OK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_usb_dev_io_init
* Returned Value   : MQX_OK or -1
* Comments         :
*    This function performs BSP-specific I/O initialization related to USB
*
*END*----------------------------------------------------------------------*/

_mqx_int _bsp_usb_dev_io_init
(
 struct usb_dev_if_struct *dev_if
)
{
  if (dev_if->DEV_INIT_PARAM == &_khci0_dev_init_param)
  {
    _bsp_usb_io_init(0);
  }
  else if (dev_if->DEV_INIT_PARAM == &_ehci0_dev_init_param)
  {
    _bsp_usb_io_init(1);
  }
  else
  {
    return IO_ERROR; //unknown controller
  }

  return MQX_OK;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_usb_host_io_init
* Returned Value   : MQX_OK or -1
* Comments         :
*    This function performs BSP-specific I/O initialization related to USB
*
*END*----------------------------------------------------------------------*/

_mqx_int _bsp_usb_host_io_init
(
 struct usb_host_if_struct *dev_if
)
{
  if (dev_if->HOST_INIT_PARAM == &_khci0_host_init_param)
  {
    _bsp_usb_io_init(0);
  }
  else if (dev_if->HOST_INIT_PARAM == &_ehci0_host_init_param)
  {
    _bsp_usb_io_init(1);
  }
  else
  {
    return IO_ERROR; //unknown controller
  }

  return MQX_OK;
}


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_serial_rts_init
* Returned Value   : MQX_OK or -1
* Comments         :
*    This function performs BSP-specific initialization related to GPIO
*
*END*----------------------------------------------------------------------*/

_mqx_int _bsp_serial_rts_init
(
 uint32_t device_index
)
{
  PORT_MemMapPtr           pctl;

  /* set pin to RTS functionality */
  switch (device_index)
  {
  case 3:
    // Перенаправить !!!
    // pctl = (PORT_MemMapPtr)PORTC_BASE_PTR;
    // pctl->PCR[18] = 0 | PORT_PCR_MUX(3);
    break;
  case 4:
    // Перенаправить !!!
    // pctl = (PORT_MemMapPtr)PORTE_BASE_PTR;
    // pctl->PCR[27] = 0 | PORT_PCR_MUX(3);
    break;
  default:
    /* not used on this board */
    break;
  }
  return MQX_OK;
}




/*FUNCTION*-------------------------------------------------------------------
 *
 * Function Name    : _bsp_serial_irda_tx_init
 * Returned Value   : MQX_OK or -1
 * Comments         :
 *    This function performs BSP-specific initialization related to IrDA
 *
 *END*----------------------------------------------------------------------*/

_mqx_int _bsp_serial_irda_tx_init(uint32_t device_index, bool enable)
{
  return MQX_OK;
}

/*FUNCTION*-------------------------------------------------------------------
 *
 * Function Name    : _bsp_serial_irda_rx_init
 * Returned Value   : MQX_OK or -1
 * Comments         :
 *    This function performs BSP-specific initialization related to IrDA
 *
 *END*----------------------------------------------------------------------*/

_mqx_int _bsp_serial_irda_rx_init(uint32_t device_index, bool enable)
{
  if (TRUE == enable)
  {
    /* hardware does not support this feature. The IrDA receiver connected
    ** to CMR2_IN3 but the UART does not support slecting source from CMP2 out put
    */
    return -1;
  }
  return MQX_OK;
}


/*------------------------------------------------------------------------------



 \param channel

 \return _mqx_int
 ------------------------------------------------------------------------------*/
_mqx_int _bsp_sai_io_init(uint32_t channel)
{
  return MQX_OK;
}


/*------------------------------------------------------------------------------



 \param source

 \return _mqx_int
 ------------------------------------------------------------------------------*/
_mqx_int _bsp_adc_channel_io_init(uint16_t source)
{
  return MQX_OK;
}


/*------------------------------------------------------------------------------



 \param adc_num

 \return _mqx_int
 ------------------------------------------------------------------------------*/
_mqx_int _bsp_adc_io_init(_mqx_uint adc_num)
{
  return MQX_OK;
}
