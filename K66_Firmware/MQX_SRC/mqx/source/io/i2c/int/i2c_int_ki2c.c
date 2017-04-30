/*HEADER**********************************************************************
*
* Copyright 2008-2014 Freescale Semiconductor, Inc.
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
*   This file contains low level functions for the I2C interrupt device driver
*   for Kinetis family.
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>
#include <io_prv.h>
#include <fio_prv.h>
#include "i2c.h"
#include "i2c_int_prv.h"
#include "i2c_ki2c_prv.h"


extern uint32_t _ki2c_polled_init (KI2C_INIT_STRUCT_PTR, void **, char *);
extern uint32_t _ki2c_polled_ioctl (VKI2C_INFO_STRUCT_PTR, uint32_t, uint32_t *);

extern uint32_t _ki2c_int_init (IO_I2C_INT_DEVICE_STRUCT_PTR, char *);
extern uint32_t _ki2c_int_deinit (IO_I2C_INT_DEVICE_STRUCT_PTR, VKI2C_INFO_STRUCT_PTR);
extern uint32_t _ki2c_int_rx (IO_I2C_INT_DEVICE_STRUCT_PTR, unsigned char *, uint32_t);
extern uint32_t _ki2c_int_tx (IO_I2C_INT_DEVICE_STRUCT_PTR, unsigned char *, uint32_t);
static void    _ki2c_isr (void *);


/*FUNCTION****************************************************************
* 
* Function Name    : _ki2c_int_install
* Returned Value   : MQX error code
* Comments         :
*    Install an I2C device.
*
*END*********************************************************************/

uint32_t _ki2c_int_install
   (
      /* [IN] A string that identifies the device for fopen */
      char              *identifier,
  
      /* [IN] The I/O init data pointer */
      KI2C_INIT_STRUCT_CPTR init_data_ptr
   )
{ /* Body */

   return _io_i2c_int_install(identifier,
      (_mqx_uint (_CODE_PTR_)(void *, char *))_ki2c_int_init,
      (_mqx_uint (_CODE_PTR_)(void *, void *))_ki2c_int_deinit,
      (_mqx_int (_CODE_PTR_)(void *, char *, _mqx_int))_ki2c_int_rx,
      (_mqx_int (_CODE_PTR_)(void *, char *, _mqx_int))_ki2c_int_tx,
      (_mqx_int (_CODE_PTR_)(void *, _mqx_uint, _mqx_uint_ptr))_ki2c_polled_ioctl, 
      (void *)init_data_ptr);

} /* Endbody */


/*FUNCTION****************************************************************
* 
* Function Name    : _ki2c_int_init
* Returned Value   : MQX error code
* Comments         :
*    This function initializes an I2C device.
*
*END*********************************************************************/

uint32_t _ki2c_int_init
   (
      /* [IN] Initialization information for the device being opened */
      IO_I2C_INT_DEVICE_STRUCT_PTR int_io_dev_ptr,

      /* [IN] The rest of the name of the device opened */
      char                     *open_name_ptr

   )
{ /* Body */
   I2C_MemMapPtr                   i2c_ptr;
   VKI2C_INFO_STRUCT_PTR           io_info_ptr;
   KI2C_INIT_STRUCT_PTR            i2c_init_ptr;
   uint32_t                         vector, result;

   i2c_init_ptr = (KI2C_INIT_STRUCT_PTR)(int_io_dev_ptr->DEV_INIT_DATA_PTR);
   result = _ki2c_polled_init (i2c_init_ptr,
                                     &(int_io_dev_ptr->DEV_INFO_PTR),
                                     open_name_ptr);
   if (result)
   {
      return result;
   }
   
   io_info_ptr = int_io_dev_ptr->DEV_INFO_PTR;
   i2c_ptr = io_info_ptr->I2C_PTR;
   vector = _bsp_get_i2c_vector (i2c_init_ptr->CHANNEL);
   if (0 == vector)
   {
      return I2C_ERROR_CHANNEL_INVALID;
   }
   io_info_ptr->VECTOR = vector;
   
   _lwsem_create((LWSEM_STRUCT_PTR)(&(io_info_ptr->LWSEM)), 0);

   /* Install new vectors and backup the old ones */
   io_info_ptr->OLD_ISR_DATA = _int_get_isr_data (vector);
   io_info_ptr->OLD_ISR = _int_install_isr (vector, _ki2c_isr, (void *)io_info_ptr);

   /* Enable I2C interrupts */
   _bsp_int_init((IRQInterruptIndex)vector, i2c_init_ptr->LEVEL, i2c_init_ptr->SUBLEVEL, TRUE);
   
   /* Enable Start/Stop detection */
   i2c_ptr->FLT |= I2C_FLT_SSIE_MASK;
   
   i2c_ptr->C1 |= I2C_C1_IICIE_MASK;

   return result;
} /* Endbody */


/*FUNCTION****************************************************************
* 
* Function Name    : _ki2c_int_deinit
* Returned Value   : MQX error code
* Comments         :
*    This function de-initializes an I2C device.
*
*END*********************************************************************/

uint32_t _ki2c_int_deinit
   (
      /* [IN] the initialization information for the device being opened */
      IO_I2C_INT_DEVICE_STRUCT_PTR int_io_dev_ptr,

      /* [IN] the address of the device specific information */
      VKI2C_INFO_STRUCT_PTR        io_info_ptr
   )
{ /* Body */
   I2C_MemMapPtr                   i2c_ptr;
      
   if ((NULL == io_info_ptr) || (NULL == int_io_dev_ptr)) 
   {
      return I2C_ERROR_INVALID_PARAMETER;
   }

   i2c_ptr = io_info_ptr->I2C_PTR;
   if (i2c_ptr->S & I2C_S_BUSY_MASK) 
   {
      return I2C_ERROR_DEVICE_BUSY;
   }
   
   /* Disable Start/Stop detection */
   i2c_ptr->FLT &= ~(I2C_FLT_SSIE_MASK);
   
   /* Disable the I2C */
   i2c_ptr->C1 = 0x00;

   /* Clear the I2C events */
   i2c_ptr->S = 0xFF; 
 
   /* Install original vectors */
   _int_install_isr (io_info_ptr->VECTOR, io_info_ptr->OLD_ISR, io_info_ptr->OLD_ISR_DATA);
   
   _lwsem_destroy((LWSEM_STRUCT_PTR)(&(io_info_ptr->LWSEM)));
   
   /* Free info struct */
   _mem_free (int_io_dev_ptr->DEV_INFO_PTR);
   int_io_dev_ptr->DEV_INFO_PTR = NULL;

   return I2C_OK;
} /* Endbody */

/*FUNCTION****************************************************************
* 
* Function Name    : _ki2c_int_rx
* Returned Value   : number of bytes read
* Comments         : 
*   Returns the number of bytes received.
*   Reads the data into provided array when data is available.
*
*END*********************************************************************/

uint32_t _ki2c_int_rx
   (
      /* [IN] the address of the device specific information */
      IO_I2C_INT_DEVICE_STRUCT_PTR int_io_dev_ptr,

      /* [IN] The array to copy data into */
      unsigned char                    *buffer,
      
      /* [IN] number of bytes to read */
      uint32_t                      length
   )
{ /* Body */
   VKI2C_INFO_STRUCT_PTR           io_info_ptr;
   I2C_MemMapPtr                   i2c_ptr;
   volatile uint8_t                 tmp;
         
   io_info_ptr = int_io_dev_ptr->DEV_INFO_PTR;
   i2c_ptr = io_info_ptr->I2C_PTR;

   if(0 == length)
      return 0;
   
   /* Critical section + avoiding spurious interrupt */
   _int_disable ();
   _bsp_int_disable (io_info_ptr->VECTOR);
   _int_enable ();

   /* If beginning of transmission, set state and send address (master only) */
   io_info_ptr->OPERATION |= I2C_OPERATION_READ;
   io_info_ptr->RX_BUFFER      = buffer;
   io_info_ptr->RX_BUFFER_SIZE = length;
   io_info_ptr->RX_INDEX       = 0;

   tmp = io_info_ptr->STATE;
   if (I2C_MODE_MASTER == io_info_ptr->MODE)
   {
      if ((I2C_STATE_READY == tmp) || (I2C_STATE_REPEATED_START == tmp))
      {
         i2c_ptr->C1 |= I2C_C1_TX_MASK;
         i2c_ptr->S |= I2C_S_IICIF_MASK;
         if (I2C_STATE_REPEATED_START == tmp)
         {
            i2c_ptr->C1 |= I2C_C1_RSTA_MASK;
         }
         else
         {
            i2c_ptr->C1 |= I2C_C1_MST_MASK;
         }
         io_info_ptr->OPERATION |= I2C_OPERATION_STARTED;
         i2c_ptr->D = (io_info_ptr->ADDRESSEE << 1) | I2C_OPERATION_READ;
         io_info_ptr->STATISTICS.TX_PACKETS++;
      }
   }
   
   /* Interrupt enable - end of critical section */
   _bsp_int_enable (io_info_ptr->VECTOR);
      
   /* Wait for rx complite */
   _lwsem_wait((LWSEM_STRUCT_PTR)(&(io_info_ptr->LWSEM)));

   return io_info_ptr->RX_INDEX;
   
} /* Endbody */


/*FUNCTION****************************************************************
* 
* Function Name    : _ki2c_int_tx
* Returned Value   : number of bytes written
* Comments         : 
*   Returns the number of bytes written.
*   Writes the data provided into transmition buffer if available.
*
*END*********************************************************************/

uint32_t _ki2c_int_tx
   (
      /* [IN] the address of the device specific information */
      IO_I2C_INT_DEVICE_STRUCT_PTR int_io_dev_ptr,

      /* [IN] The array characters are to be read from */
      unsigned char                    *buffer,
      
      /* [IN] number of bytes to output */
      uint32_t                      length
   )
{ /* Body */
   VKI2C_INFO_STRUCT_PTR           io_info_ptr;
   I2C_MemMapPtr                   i2c_ptr;   
   uint32_t                        tmp;
   
   io_info_ptr  = int_io_dev_ptr->DEV_INFO_PTR;
   i2c_ptr = io_info_ptr->I2C_PTR;

   /* Critical section + avoiding spurious interrupt */
   _int_disable ();
   _bsp_int_disable (io_info_ptr->VECTOR);
   _int_enable ();
   
   /* If beginning of transmission, set state and send address (master only) */
   io_info_ptr->OPERATION &= (~ I2C_OPERATION_READ);
   io_info_ptr->TX_BUFFER      = buffer;
   io_info_ptr->TX_BUFFER_SIZE = length;
   io_info_ptr->TX_INDEX       = 0;
   
   tmp = io_info_ptr->STATE;
   if (I2C_MODE_MASTER == io_info_ptr->MODE)
   {
      if ((I2C_STATE_READY == tmp) || (I2C_STATE_REPEATED_START == tmp))
      {
         i2c_ptr->C1 |= I2C_C1_TX_MASK;
         i2c_ptr->S |= I2C_S_IICIF_MASK;
         if (I2C_STATE_REPEATED_START == tmp)
         {
            i2c_ptr->C1 |= I2C_C1_RSTA_MASK;
         }
         else
         {
            i2c_ptr->C1 |= I2C_C1_MST_MASK;
         }
         io_info_ptr->OPERATION |= I2C_OPERATION_STARTED;
         i2c_ptr->D = (io_info_ptr->ADDRESSEE << 1) | I2C_OPERATION_WRITE;
         io_info_ptr->STATISTICS.TX_PACKETS++;
      }
      else if(I2C_STATE_TRANSMIT == tmp)
      {
         if(length != 0)
         {
            /* send first byte */
            i2c_ptr->D = io_info_ptr->TX_BUFFER[io_info_ptr->TX_INDEX++];   /*  transmit data */
            io_info_ptr->STATISTICS.TX_PACKETS++;
         }
         else
            return 0;
      }
   }

   /* Interrupt enable - end of critical section */
   _bsp_int_enable (io_info_ptr->VECTOR);
   
   /* Wait for tx complite */
   _lwsem_wait((LWSEM_STRUCT_PTR)(&(io_info_ptr->LWSEM)));

   return io_info_ptr->TX_INDEX;

} /* Endbody */


/*FUNCTION****************************************************************
* 
* Function Name    :_ki2c_isr
* Returned Value   : none   
*
*END*********************************************************************/
static void _ki2c_isr
   (
      void              *parameter
   )
{ /* Body */
   VKI2C_INFO_STRUCT_PTR io_info_ptr = parameter;
   I2C_MemMapPtr         i2c_ptr = io_info_ptr->I2C_PTR;
   uint8_t               i2csr;
   volatile uint8_t      tmp;

   i2csr = i2c_ptr->S;
   
   /* Check start signal */
   if(i2c_ptr->FLT & I2C_FLT_STARTF_MASK)
   {
       i2c_ptr->FLT |= I2C_FLT_STARTF_MASK;
       i2c_ptr->S |= I2C_S_IICIF_MASK;
       io_info_ptr->STATISTICS.INTERRUPTS++;
       return;
   }

   /* Check stop signal */
   if(i2c_ptr->FLT & I2C_FLT_STOPF_MASK)
   {
       i2c_ptr->FLT |= I2C_FLT_STOPF_MASK;
       i2c_ptr->S |= I2C_S_IICIF_MASK;
       io_info_ptr->STATISTICS.INTERRUPTS++;
       if(0 == (i2c_ptr->C1 & I2C_C1_MST_MASK))
       {
          if (io_info_ptr->OPERATION & I2C_OPERATION_STARTED)
          {
             io_info_ptr->OPERATION = 0;
             io_info_ptr->RX_REQUEST = 0;
             io_info_ptr->STATE = I2C_STATE_FINISHED;
             _bsp_int_disable(io_info_ptr->VECTOR);
             _lwsem_post((LWSEM_STRUCT_PTR)(&(io_info_ptr->LWSEM)));
          }
       }
       return;
   }

   io_info_ptr->STATISTICS.INTERRUPTS++;
   
   if(I2C_MODE_SLAVE == io_info_ptr->MODE)
      io_info_ptr->OPERATION |= I2C_OPERATION_STARTED;
   
   /* Master */
   if (i2c_ptr->C1 & I2C_C1_MST_MASK)
   {
      i2c_ptr->S |= I2C_S_IICIF_MASK;
      /* Transmit */
      if (i2c_ptr->C1 & I2C_C1_TX_MASK)
      {
         /* Not ack */
         if (i2csr & I2C_S_RXAK_MASK)
         {
            io_info_ptr->STATE = I2C_STATE_FINISHED;
            io_info_ptr->STATISTICS.TX_NAKS++;
            _bsp_int_disable(io_info_ptr->VECTOR);
            _lwsem_post((LWSEM_STRUCT_PTR)(&(io_info_ptr->LWSEM)));
         }
         /* Ack */
         else
         {
            /* End of address cycle? */
            if((I2C_STATE_READY == io_info_ptr->STATE) || (I2C_STATE_REPEATED_START == io_info_ptr->STATE))
            {
               /* Transmit operation */
               if(0 == (I2C_OPERATION_READ & io_info_ptr->OPERATION))
               {
                  io_info_ptr->STATE = I2C_STATE_TRANSMIT;
                  // For fwrite(0)
                  if(0 == io_info_ptr->TX_BUFFER_SIZE)
                  {
                     _bsp_int_disable(io_info_ptr->VECTOR);
                     _lwsem_post((LWSEM_STRUCT_PTR)(&(io_info_ptr->LWSEM)));
                  }
                  else
                  {
                     /*Transmit first byte*/
                     i2c_ptr->D = io_info_ptr->TX_BUFFER[io_info_ptr->TX_INDEX++];
                     io_info_ptr->STATISTICS.TX_PACKETS++;
                  }
               }
               /* Receive operation */
               else
               {
                  /* Change to receive state */
                  io_info_ptr->STATE = I2C_STATE_RECEIVE;
                  i2c_ptr->C1 &= (~ I2C_C1_TX_MASK);

                  if(1 == io_info_ptr->RX_REQUEST)
                  {
                      /* Send Nack */
                      i2c_ptr->C1 |= I2C_C1_TXAK_MASK;
                  }
                  else
                  {
                      /* Send ack */
                      i2c_ptr->C1 &= (~ I2C_C1_TXAK_MASK);
                  }
                  /* dummy read to clock in 1st byte */
                  tmp = i2c_ptr->D;
               }
            }
            /* Normal i2c transmit */
            else
            {
               /* Anything to transmit? */
               if (io_info_ptr->TX_INDEX < io_info_ptr->TX_BUFFER_SIZE)
               {
                  i2c_ptr->D = io_info_ptr->TX_BUFFER[io_info_ptr->TX_INDEX++];   /*  transmit data */
                  io_info_ptr->STATISTICS.TX_PACKETS++;
               }
               else
               {
                  /* Transmit finish */
                  _bsp_int_disable(io_info_ptr->VECTOR);
                  _lwsem_post((LWSEM_STRUCT_PTR)(&(io_info_ptr->LWSEM)));
               }
            }
         }
      }
      /* Receive */
      else
      {
         /* Buffer full */
         if (io_info_ptr->RX_INDEX >= io_info_ptr->RX_BUFFER_SIZE)
         {
            _bsp_int_disable (io_info_ptr->VECTOR);
            _lwsem_post((LWSEM_STRUCT_PTR)(&(io_info_ptr->LWSEM)));
         }
         /* Buffer not full */
         else
         {
            /* 2nd last byte to read */
            if (2 == io_info_ptr->RX_REQUEST)
            {
              i2c_ptr->C1 |= I2C_C1_TXAK_MASK;
            }
            else
            {
              i2c_ptr->C1 &= (~ I2C_C1_TXAK_MASK);
            }

            if (1 == io_info_ptr->RX_REQUEST)
                i2c_ptr->C1 |= I2C_C1_TX_MASK;       /* no more reading */

            tmp = i2c_ptr->D;   /* receive data */
            io_info_ptr->RX_BUFFER[io_info_ptr->RX_INDEX++] = (tmp & I2C_D_DATA_MASK);
            io_info_ptr->RX_REQUEST--;
            io_info_ptr->STATISTICS.RX_PACKETS++;

            /* Receive Finished */
            if (0 == io_info_ptr->RX_REQUEST)
            {
                _bsp_int_disable (io_info_ptr->VECTOR);
                io_info_ptr->STATE = I2C_STATE_FINISHED;
                _lwsem_post((LWSEM_STRUCT_PTR)(&(io_info_ptr->LWSEM)));
            }
         }
      }
   }
   /* Slave */
   else
   {
      /* Master arbitration lost */
      if (i2csr & I2C_S_ARBL_MASK)
      {
         i2c_ptr->S |= I2C_S_ARBL_MASK;
         io_info_ptr->STATE = I2C_STATE_LOST_ARBITRATION;
         io_info_ptr->STATISTICS.TX_LOST_ARBITRATIONS++;
      }
      /* Addressed as slave */
      if (i2csr & I2C_S_IAAS_MASK)
      {
         if (I2C_MODE_MASTER == io_info_ptr->MODE)
         {
            io_info_ptr->STATISTICS.TX_ADDRESSED_AS_SLAVE++;
         }
         /* Transmit requested */
         if (i2csr & I2C_S_SRW_MASK)
         {
            io_info_ptr->STATE = I2C_STATE_ADDRESSED_AS_SLAVE_TX;
            if ((I2C_OPERATION_STARTED == (io_info_ptr->OPERATION & (I2C_OPERATION_READ | I2C_OPERATION_STARTED))) && (io_info_ptr->TX_BUFFER_SIZE != 0))
            {
               i2c_ptr->C1 |= I2C_C1_TX_MASK;
               i2c_ptr->S |= I2C_S_IICIF_MASK;
               i2c_ptr->D = io_info_ptr->TX_BUFFER[io_info_ptr->TX_INDEX++];   /* transmit data */
               io_info_ptr->STATISTICS.TX_PACKETS++;
               _bsp_int_disable (io_info_ptr->VECTOR);
               _lwsem_post((LWSEM_STRUCT_PTR)(&(io_info_ptr->LWSEM)));
            }
            else
            {
               _bsp_int_disable (io_info_ptr->VECTOR);
               _lwsem_post((LWSEM_STRUCT_PTR)(&(io_info_ptr->LWSEM)));
            }
         }
         /* Receive requested */
         else
         {
            io_info_ptr->STATE = I2C_STATE_ADDRESSED_AS_SLAVE_RX;
            if (((I2C_OPERATION_READ | I2C_OPERATION_STARTED) == (io_info_ptr->OPERATION & (I2C_OPERATION_READ | I2C_OPERATION_STARTED))) && (io_info_ptr->RX_INDEX == 0) && (0 != io_info_ptr->RX_REQUEST))
            {
               i2c_ptr->C1 &= (~ I2C_C1_TX_MASK);
               i2c_ptr->S |= I2C_S_IICIF_MASK;
               if (1 == io_info_ptr->RX_REQUEST)
               {
                  i2c_ptr->C1 |= I2C_C1_TXAK_MASK;
               }
               else
               {
                  i2c_ptr->C1 &= (~ I2C_C1_TXAK_MASK);
               }
               tmp = i2c_ptr->D;   /* dummy read to release bus */
            }
            else
            {
               _bsp_int_disable (io_info_ptr->VECTOR);
               _lwsem_post((LWSEM_STRUCT_PTR)(&(io_info_ptr->LWSEM)));
            }
         }
      }
      /* Normal slave operation */
      else
      {
         /* No master arbitration lost */
         if (! (i2csr & I2C_S_ARBL_MASK))
         {
            /* Transmit */
            if (i2c_ptr->C1 & I2C_C1_TX_MASK)
            {
               /* Not ack */
               if (i2csr & I2C_S_RXAK_MASK)
               {
                  i2c_ptr->S |= I2C_S_IICIF_MASK;
                  io_info_ptr->STATE = I2C_STATE_FINISHED;
                  io_info_ptr->STATISTICS.TX_NAKS++;
                  _bsp_int_disable (io_info_ptr->VECTOR);
                  _lwsem_post((LWSEM_STRUCT_PTR)(&(io_info_ptr->LWSEM)));
               }
               /* Ack */
               else
               {
                  /* Transmit requested */
                  if (((I2C_STATE_TRANSMIT == io_info_ptr->STATE) || (I2C_STATE_ADDRESSED_AS_SLAVE_TX == io_info_ptr->STATE)) 
                     && (io_info_ptr->TX_INDEX != io_info_ptr->TX_BUFFER_SIZE) && (I2C_OPERATION_STARTED == (io_info_ptr->OPERATION & (I2C_OPERATION_READ | I2C_OPERATION_STARTED))))
                  {
                     i2c_ptr->S |= I2C_S_IICIF_MASK;
                     i2c_ptr->D = io_info_ptr->TX_BUFFER[io_info_ptr->TX_INDEX++];   /*  transmit data */
                     io_info_ptr->STATISTICS.TX_PACKETS++;
                     if(io_info_ptr->TX_INDEX == io_info_ptr->TX_BUFFER_SIZE)
                     {
                        _bsp_int_disable (io_info_ptr->VECTOR);
                        _lwsem_post((LWSEM_STRUCT_PTR)(&(io_info_ptr->LWSEM)));
                     }
                  }
                  else
                  {
                     _bsp_int_disable (io_info_ptr->VECTOR);
                     _lwsem_post((LWSEM_STRUCT_PTR)(&(io_info_ptr->LWSEM)));
                  }
               }
            }
            /* Receive */
            else
            {
               /* Receive requested */
               if (((I2C_STATE_RECEIVE == io_info_ptr->STATE) || (I2C_STATE_ADDRESSED_AS_SLAVE_RX == io_info_ptr->STATE)) 
                  && ((I2C_OPERATION_READ | I2C_OPERATION_STARTED) == (io_info_ptr->OPERATION & (I2C_OPERATION_READ | I2C_OPERATION_STARTED))) && (0 != io_info_ptr->RX_REQUEST))
               {
                  i2c_ptr->S |= I2C_S_IICIF_MASK; 
                  io_info_ptr->RX_REQUEST--;
                  if (1 == io_info_ptr->RX_REQUEST)
                  {
                     i2c_ptr->C1 |= I2C_C1_TXAK_MASK;
                  }
                  else
                  {
                     i2c_ptr->C1 &= (~ I2C_C1_TXAK_MASK);
                  }
                  io_info_ptr->RX_BUFFER[io_info_ptr->RX_INDEX++] = i2c_ptr->D;   /* receive data */
                  io_info_ptr->STATISTICS.RX_PACKETS++;
                  if ((0 == io_info_ptr->RX_REQUEST) || (io_info_ptr->RX_INDEX == io_info_ptr->RX_BUFFER_SIZE))
                  {
                     if (0 == io_info_ptr->RX_REQUEST)
                        io_info_ptr->STATE = I2C_STATE_FINISHED;
                     _bsp_int_disable (io_info_ptr->VECTOR);
                     _lwsem_post((LWSEM_STRUCT_PTR)(&(io_info_ptr->LWSEM)));
                  }
               }
               else
               {
                  _bsp_int_disable (io_info_ptr->VECTOR);
                  _lwsem_post((LWSEM_STRUCT_PTR)(&(io_info_ptr->LWSEM)));
               }
            }
         }
         else
         {
            _bsp_int_disable (io_info_ptr->VECTOR);
            _lwsem_post((LWSEM_STRUCT_PTR)(&(io_info_ptr->LWSEM)));
         }
      }
   }
} /* Endbody */

/* EOF */
