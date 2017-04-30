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
*   This file contains low level functions for the USB DCD polled device driver
*   for Kinetis family.
*
*
*END************************************************************************/


#include <mqx.h>
#include <bsp.h>
#include <io_prv.h>
#include <fio_prv.h>
#include "usb_dcd.h"
#include "usb_dcd_pol_prv.h"
#include "usb_dcd_kn_prv.h"

extern uint32_t _kusb_dcd_polled_init   (KUSB_DCD_INIT_STRUCT_PTR, void **, char *);
extern uint32_t _kusb_dcd_polled_deinit (IO_USB_DCD_POLLED_DEVICE_STRUCT_PTR, KUSB_DCD_INFO_STRUCT_PTR);
extern uint32_t _kusb_dcd_polled_read   (IO_USB_DCD_POLLED_DEVICE_STRUCT_PTR, unsigned char *, uint32_t);
extern uint32_t _kusb_dcd_polled_write  (IO_USB_DCD_POLLED_DEVICE_STRUCT_PTR, unsigned char *, uint32_t);
extern uint32_t _kusb_dcd_polled_ioctl  (KUSB_DCD_INFO_STRUCT_PTR, uint32_t, uint32_t *);

/*FUNCTION****************************************************************
*
* Function Name    : _usb_dcd_finish_check
* Returned Value   : void
* Comments         :
*    Install an USB DCD device.
*
*END*********************************************************************/
void _usb_dcd_finish_check
    (
    )
{

}

/*FUNCTION****************************************************************
*
* Function Name    : _kusb_dcd_polled_install
* Returned Value   : MQX error code
* Comments         :
*    Install an USB DCD device.
*
*END*********************************************************************/

uint32_t _kusb_dcd_polled_install
   (
      /* [IN] A string that identifies the device for fopen */
      char                           *identifier,

      /* [IN] The I/O init data pointer */
      KUSB_DCD_INIT_STRUCT_CPTR      init_data_ptr
   )
{ /* Body */


#if PE_LDD_VERSION
    if (PE_PeripheralUsed((uint32_t)_bsp_get_usb_dcd_base_address()))
    {
        /* IO Device used by PE Component*/
        return IO_DEVICE_EXISTS;
    }
#endif


    return _io_usb_dcd_polled_install(identifier,
      (uint32_t (_CODE_PTR_)(void *, void **, char *))_kusb_dcd_polled_init,
      (uint32_t (_CODE_PTR_)(void *, void *))_kusb_dcd_polled_deinit,
      (int32_t (_CODE_PTR_)(void *, char *, int32_t))_kusb_dcd_polled_read,
      (int32_t (_CODE_PTR_)(void *, char *, int32_t))_kusb_dcd_polled_write,
      (int32_t (_CODE_PTR_)(void *, uint32_t, uint32_t *))_kusb_dcd_polled_ioctl,
      (void *)init_data_ptr);

} /* Endbody */


/*FUNCTION****************************************************************
*
* Function Name    : _kusb_dcd_polled_init
* Returned Value   : MQX error code
* Comments         :
*    This function initializes an USB_DCD device.
*
*END*********************************************************************/

uint32_t _kusb_dcd_polled_init
   (
      /* [IN] Initialization information for the device being opened */
      KUSB_DCD_INIT_STRUCT_PTR      io_init_ptr,

      /* [OUT] Address to store device specific information */
      void                    **io_info_ptr_ptr,

      /* [IN] The rest of the name of the device opened */
      char                      *open_name_ptr

   )
{ /* Body */
   USBDCD_MemMapPtr             usb_dcd_ptr;
   KUSB_DCD_INFO_STRUCT_PTR     io_info_ptr;
   SIM_MemMapPtr                sim = SIM_BASE_PTR;
   USB_MemMapPtr                usbotg = USB0_BASE_PTR;
   usb_dcd_ptr = _bsp_get_usb_dcd_base_address();
   if (NULL == usb_dcd_ptr)
   {
        return USB_DCD_ERROR_CHANNEL_INVALID;
   }

   io_info_ptr = (KUSB_DCD_INFO_STRUCT_PTR)_mem_alloc_system_zero ((uint32_t)sizeof (KUSB_DCD_INFO_STRUCT));
   if (NULL == io_info_ptr)
   {
      return MQX_OUT_OF_MEMORY;
   }
   _mem_set_type (io_info_ptr, MEM_TYPE_IO_USB_DCD_INFO_STRUCT);

   *io_info_ptr_ptr                 = io_info_ptr;
   io_info_ptr->INIT          = *io_init_ptr;
   io_info_ptr->USB_DCD_PTR   = usb_dcd_ptr;
   io_info_ptr->METHOD        = USB_DCD_POLLED;
   io_info_ptr->STATE         = USB_DCD_DISABLE;

   /* USB DCD board specific initialization */

   /* USB Clock Gating for OTG */
#ifdef BSP_TWRMCF51JF
   sim->SCGC6 |= SIM_SCGC6_USBOTG_MASK;
#elif BSP_TWR_K70F120M || BSP_TWR_K60F120M
   sim->SCGC4 |= SIM_SCGC4_USBFS_MASK;
#else
   sim->SCGC4 |= SIM_SCGC4_USBOTG_MASK;
#endif
   /* USB Clock Gating for DCD */
   sim->SCGC6 |= SIM_SCGC6_USBDCD_MASK;

   /* USB pre-initialization */
   usbotg->USBTRC0 |= USB_USBTRC0_USBRESET_MASK;
   while (usbotg->USBTRC0 & USB_USBTRC0_USBRESET_MASK){};
   usbotg->ISTAT |= USB_ISTAT_USBRST_MASK;

   /* USB DCD initialization */

   /* CLOCK SPEED (in Khz)*/
   if (io_init_ptr->CLOCK_SPEED < 0x400) /*Khz*/
   {
       if (io_init_ptr->CLOCK_SPEED < 0x04)
           return USB_DCD_CLOCK_SPEED_INVALID;
       usb_dcd_ptr->CLOCK = ((uint32_t)((uint32_t)io_init_ptr->CLOCK_SPEED << USBDCD_CLOCK_CLOCK_SPEED_SHIFT)) & USBDCD_CLOCK_CLOCK_SPEED_MASK;
   }
   else
   {
       if (((uint32_t)(io_init_ptr->CLOCK_SPEED/1000)) > 0x3FF)
            return USB_DCD_CLOCK_SPEED_INVALID;
      usb_dcd_ptr->CLOCK = (((uint32_t)(((uint32_t)(io_init_ptr->CLOCK_SPEED/1000)) << USBDCD_CLOCK_CLOCK_SPEED_SHIFT)) & USBDCD_CLOCK_CLOCK_SPEED_MASK) | USBDCD_CLOCK_CLOCK_UNIT_MASK;
   }
   /* Timer0 initialization */
   usb_dcd_ptr->TIMER0 = (uint32_t)((uint32_t)(io_init_ptr->TSEQ_INIT)<<USBDCD_TIMER0_TSEQ_INIT_SHIFT) & USBDCD_TIMER0_TSEQ_INIT_MASK;

   /* Timer1 initialization */
   usb_dcd_ptr->TIMER1 = ((uint32_t)((uint32_t)(io_init_ptr->TDCD_DBNC)<<USBDCD_TIMER1_TDCD_DBNC_SHIFT) & USBDCD_TIMER1_TDCD_DBNC_MASK) |
                                ((uint32_t)((uint32_t)(io_init_ptr->TVDPSRC_ON)<<USBDCD_TIMER1_TVDPSRC_ON_SHIFT) & USBDCD_TIMER1_TVDPSRC_ON_MASK);
#ifndef USBDCD_TIMER2_BC11_REG
   /* Timer2 initialization */
   usb_dcd_ptr->TIMER2 = ((uint32_t)((uint32_t)(io_init_ptr->TVDPSRC_CON)<<USBDCD_TIMER2_TVDPSRC_CON_SHIFT) & USBDCD_TIMER2_TVDPSRC_CON_MASK) |
                                ((uint32_t)((uint32_t)(io_init_ptr->CHECK_DM)<<USBDCD_TIMER2_CHECK_DM_SHIFT) & USBDCD_TIMER2_CHECK_DM_MASK);
#else
   /*Timer2 initialization */
   usb_dcd_ptr->TIMER2_BC11 = ((uint32_t)((uint32_t)(io_init_ptr->TVDPSRC_CON)<<USBDCD_TIMER2_BC11_TVDPSRC_CON_SHIFT) & USBDCD_TIMER2_BC11_TVDPSRC_CON_MASK) |
                                ((uint32_t)((uint32_t)(io_init_ptr->CHECK_DM)<<USBDCD_TIMER2_BC11_CHECK_DM_SHIFT) & USBDCD_TIMER2_BC11_CHECK_DM_MASK);
#endif   
   /* Disable USB DCD module */
   usb_dcd_ptr->CONTROL = USBDCD_CONTROL_IACK_MASK | USBDCD_CONTROL_SR_MASK;

   return USB_DCD_OK;
} /* Endbody */


/*FUNCTION****************************************************************
*
* Function Name    : _kusb_dcd_polled_deinit
* Returned Value   : MQX error code
* Comments         :
*    This function de-initializes an USB DCD device.
*
*END*********************************************************************/

uint32_t _kusb_dcd_polled_deinit
   (
      /* [IN] the initialization information for the device being opened */
      IO_USB_DCD_POLLED_DEVICE_STRUCT_PTR pol_io_dev_ptr,

      /* [IN] the address of the device specific information */
      KUSB_DCD_INFO_STRUCT_PTR     io_info_ptr
   )
{ /* Body */
   USBDCD_MemMapPtr            usb_dcd_ptr;

   if ((NULL == io_info_ptr) || (NULL == pol_io_dev_ptr))
   {
      return USB_DCD_ERROR_INVALID_PARAMETER;
   }

   usb_dcd_ptr = io_info_ptr->USB_DCD_PTR;

   /* Disable the USB DCD module */
   usb_dcd_ptr->CONTROL = USBDCD_CONTROL_IACK_MASK | USBDCD_CONTROL_SR_MASK;

   /* Free info struct */
   _mem_free (pol_io_dev_ptr->DEV_INFO_PTR);
   pol_io_dev_ptr->DEV_INFO_PTR = NULL;

   return MQX_OK;

} /* Endbody */


/*FUNCTION****************************************************************
*
* Function Name    : _kusb_dcd_polled_ioctl
* Returned Value   : MQX error code.
* Comments         :
*    This function performs miscellaneous services for
*    the USB DCD I/O device.
*
*END*********************************************************************/

uint32_t _kusb_dcd_polled_ioctl
   (
      /* [IN] the address of the device specific information */
      KUSB_DCD_INFO_STRUCT_PTR io_info_ptr,

      /* [IN] The command to perform */
      uint32_t                     cmd,

      /* [IN] Parameters for the command */
      uint32_t                 *param_ptr
   )
{ /* Body */
   USBDCD_MemMapPtr             usb_dcd_ptr;
   uint32_t                      result = MQX_OK;
   USB_MemMapPtr                usbotg = USB0_BASE_PTR;

   usb_dcd_ptr = io_info_ptr->USB_DCD_PTR;

   switch (cmd)
   {
      case IO_IOCTL_USB_DCD_SET_TSEQ_INIT:
          usb_dcd_ptr->TIMER0 |= (uint32_t)((uint32_t)(*param_ptr)<<USBDCD_TIMER0_TSEQ_INIT_SHIFT);
         break;
      case IO_IOCTL_USB_DCD_GET_TSEQ_INIT:
          *param_ptr = (uint32_t)((usb_dcd_ptr->TIMER0 & USBDCD_TIMER0_TSEQ_INIT_MASK)>>USBDCD_TIMER0_TSEQ_INIT_SHIFT);
         break;
      case IO_IOCTL_USB_DCD_GET_TUNITCON:
          *param_ptr = (uint32_t)((usb_dcd_ptr->TIMER0 & USBDCD_TIMER0_TUNITCON_MASK)>>USBDCD_TIMER0_TUNITCON_SHIFT);
          break;
      case IO_IOCTL_USB_DCD_SET_TDCD_DBNC:
          usb_dcd_ptr->TIMER1 |= (uint32_t)((uint32_t)(*param_ptr)<<USBDCD_TIMER1_TDCD_DBNC_SHIFT);
          break;
      case IO_IOCTL_USB_DCD_GET_TDCD_DBNC:
          *param_ptr = (uint32_t)((usb_dcd_ptr->TIMER1 & USBDCD_TIMER1_TDCD_DBNC_MASK)>>USBDCD_TIMER1_TDCD_DBNC_SHIFT);
          break;
      case IO_IOCTL_USB_DCD_SET_TVDPSRC_ON:
          usb_dcd_ptr->TIMER1 |= (uint32_t)((uint32_t)(*param_ptr)<<USBDCD_TIMER1_TVDPSRC_ON_SHIFT);
          break;
      case IO_IOCTL_USB_DCD_GET_TVDPSRC_ON:
          *param_ptr = (uint32_t)((usb_dcd_ptr->TIMER1 & USBDCD_TIMER1_TVDPSRC_ON_MASK)>>USBDCD_TIMER1_TVDPSRC_ON_SHIFT);
          break;
      case IO_IOCTL_USB_DCD_SET_TVDPSRC_CON:
#ifndef USBDCD_TIMER2_BC11_REG
          usb_dcd_ptr->TIMER2 |= (uint32_t)((uint32_t)(*param_ptr)<<USBDCD_TIMER2_TVDPSRC_CON_SHIFT);
        break;
#else 
        usb_dcd_ptr->TIMER2_BC11 |= (uint32_t)((uint32_t)(*param_ptr)<<USBDCD_TIMER2_BC11_TVDPSRC_CON_SHIFT);
        break;
#endif        
      case IO_IOCTL_USB_DCD_GET_TVDPSRC_CON:
#ifndef USBDCD_TIMER2_BC11_REG        
          *param_ptr = (uint32_t)((usb_dcd_ptr->TIMER2 & USBDCD_TIMER2_TVDPSRC_CON_MASK)>>USBDCD_TIMER2_TVDPSRC_CON_SHIFT);
          break;
#else
        *param_ptr = (uint32_t)((usb_dcd_ptr->TIMER2_BC11 & USBDCD_TIMER2_BC11_TVDPSRC_CON_MASK)>>USBDCD_TIMER2_BC11_TVDPSRC_CON_SHIFT);
          break;
#endif        
      case IO_IOCTL_USB_DCD_SET_CHECK_DM:
#ifndef USBDCD_TIMER2_BC11_REG    
          usb_dcd_ptr->TIMER2 |= (uint32_t)((uint32_t)(*param_ptr)<<USBDCD_TIMER2_CHECK_DM_SHIFT);
          break;
#else
        usb_dcd_ptr->TIMER2_BC11 |= (uint32_t)((uint32_t)(*param_ptr)<<USBDCD_TIMER2_BC11_CHECK_DM_SHIFT);
          break;
#endif        
      case IO_IOCTL_USB_DCD_GET_CHECK_DM:
#ifndef USBDCD_TIMER2_BC11_REG
          *param_ptr = (uint32_t)((usb_dcd_ptr->TIMER2 & USBDCD_TIMER2_CHECK_DM_MASK)>>USBDCD_TIMER2_CHECK_DM_SHIFT);
          break;
#else
        *param_ptr = (uint32_t)((usb_dcd_ptr->TIMER2_BC11 & USBDCD_TIMER2_BC11_CHECK_DM_MASK)>>USBDCD_TIMER2_BC11_CHECK_DM_SHIFT);
          break;
#endif        
      case IO_IOCTL_USB_DCD_SET_CLOCK_SPEED:
          if (*param_ptr < 0x400) /*Khz*/
          {
              if (*param_ptr < 0x04)
                  return USB_DCD_CLOCK_SPEED_INVALID;
              usb_dcd_ptr->CLOCK = ((uint32_t)((uint32_t)(*param_ptr) << USBDCD_CLOCK_CLOCK_SPEED_SHIFT)) & USBDCD_CLOCK_CLOCK_SPEED_MASK;
          }
          else
          {
              if (((uint32_t)(*param_ptr/1000)) > 0x3FF)
                   return USB_DCD_CLOCK_SPEED_INVALID;
             usb_dcd_ptr->CLOCK = (((uint32_t)(((uint32_t)(*param_ptr/1000)) << USBDCD_CLOCK_CLOCK_SPEED_SHIFT)) & USBDCD_CLOCK_CLOCK_SPEED_MASK) | USBDCD_CLOCK_CLOCK_UNIT_MASK;
          }
          break;
      case IO_IOCTL_USB_DCD_GET_CLOCK_SPEED:
          if (usb_dcd_ptr->CLOCK & USBDCD_CLOCK_CLOCK_UNIT_MASK)
          {
              *param_ptr = (uint32_t)(((usb_dcd_ptr->CLOCK & USBDCD_CLOCK_CLOCK_SPEED_MASK) >> USBDCD_CLOCK_CLOCK_SPEED_SHIFT) * 1000);
          }
          else
          {
              *param_ptr = (uint32_t)((usb_dcd_ptr->CLOCK & USBDCD_CLOCK_CLOCK_SPEED_MASK) >> USBDCD_CLOCK_CLOCK_SPEED_SHIFT);
          }
          break;
      case IO_IOCTL_USB_DCD_GET_STATUS:
          *param_ptr = (uint32_t)(usb_dcd_ptr->STATUS>>16);
          break;
      case IO_IOCTL_USB_DCD_RESET:
          usb_dcd_ptr->CONTROL |= (USBDCD_CONTROL_SR_MASK | USBDCD_CONTROL_IACK_MASK);
          io_info_ptr->STATE = USB_DCD_DISABLE;
          break;
      case IO_IOCTL_USB_DCD_START:
          usb_dcd_ptr->CONTROL |= (USBDCD_CONTROL_START_MASK | USBDCD_CONTROL_IACK_MASK);
          io_info_ptr->STATE = USB_DCD_ENABLE;
          break;
      case IO_IOCTL_USB_DCD_GET_STATE:
            if (io_info_ptr->METHOD == USB_DCD_POLLED)
            {
                /* update state */
                if (usb_dcd_ptr->CONTROL & USBDCD_CONTROL_IF_MASK)
                {
                    io_info_ptr->STATE = USB_DCD_SEQ_COMPLETE;
                    usb_dcd_ptr->CONTROL |= USBDCD_CONTROL_IACK_MASK;

                    /* Enable pull-up register */
                    if (usb_dcd_ptr->STATUS & USBDCD_STATUS_SEQ_RES_MASK)
                    {
                         usbotg->CONTROL |= USB_CONTROL_DPPULLUPNONOTG_MASK;
                    }
                }
            }
         *param_ptr = io_info_ptr->STATE;
          break;
      case IO_IOCTL_USB_DCD_GET_CHARGER_TYPE:
          *param_ptr = (uint32_t)((usb_dcd_ptr->STATUS & USBDCD_STATUS_SEQ_RES_MASK)>>16);
           *param_ptr |= (uint32_t)((usb_dcd_ptr->STATUS & (USBDCD_STATUS_ERR_MASK | USBDCD_STATUS_TO_MASK))>>16);
          break;
      default:
          return USB_DCD_ERROR_INVALID_CMD;
   }
   return result;
} /* Endbody */


/*FUNCTION****************************************************************
*
* Function Name    : _kusb_dcd_polled_read
* Returned Value   : MQX_OK
* Comments         :
*
*
*END*********************************************************************/

uint32_t _kusb_dcd_polled_read
   (
      /* [IN] the address of the device specific information */
      IO_USB_DCD_POLLED_DEVICE_STRUCT_PTR pol_io_dev_ptr,

      /* [IN] The array to copy data into */
      unsigned char                       *buffer,

      /* [IN] number of bytes to read */
      uint32_t                         length
   )
{ /* Body */
   return MQX_OK;
} /* Endbody */


/*FUNCTION****************************************************************
*
* Function Name    : _kusb_dcd_polled_write
* Returned Value   : MQX_OK
* Comments         :
*
*
*END*********************************************************************/

uint32_t _kusb_dcd_polled_write
   (
      /* [IN] the address of the device specific information */
      IO_USB_DCD_POLLED_DEVICE_STRUCT_PTR pol_io_dev_ptr,

      /* [IN] The array characters are to be read from */
      unsigned char                       *buffer,

      /* [IN] number of bytes to output */
      uint32_t                         length
   )
{ /* Body */
   return MQX_OK;
} /* Endbody */

/* EOF */
