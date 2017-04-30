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
*   This file contains misc. ENET driver utility functions
*   function.
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>

#include "enet.h"
#include "enetprv.h"


#if 0
/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_get_device_number
*  Returned Value : device number or -1
*  Comments       :
*        Retrieves the Ethernet device number of an initialized device.
*
*END*-----------------------------------------------------------------*/

uint32_t ENET_get_device_number
   (
      /* [IN] the Ethernet state structure */
      _enet_handle   handle
   )
{
    ENET_CONTEXT_STRUCT_PTR enet_ptr = (ENET_CONTEXT_STRUCT_PTR)handle;

    if (enet_ptr != NULL)
    {
        return enet_ptr->DEVICE;
    }
    return (uint32_t)-1;
}
#endif

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_Enqueue_Buffer
*  Returned Value :
*  Comments       :
*
*
*END*-----------------------------------------------------------------*/

void ENET_Enqueue_Buffer( void **q, void *b)
{
   *((void **)b) = *q;
   *q = b;
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_Dequeue_Buffer
*  Returned Value :
*  Comments       :
*
*
*END*-----------------------------------------------------------------*/

void *ENET_Dequeue_Buffer( void **q)
{
   void   *b = *q;

   if (b) {
      *q = *((void **)b);
   }
   return b;
}


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_link_status
*  Returned Value : TRUE if link active, FALSE otherwise
*  Comments       : Get actual link status.
*
*END*-----------------------------------------------------------------*/

bool ENET_link_status(_enet_handle   handle)
{
    ENET_CONTEXT_STRUCT_PTR enet_ptr = (ENET_CONTEXT_STRUCT_PTR)handle;

   return (*enet_ptr->PARAM_PTR->ENET_IF->PHY_IF->STATUS)(enet_ptr);
}



/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_get_speed
*  Returned Value : 10 or 100
*  Comments       : For Backward compatibility of old BSPs.
*        Get the negoiated speed.
*
*END*-----------------------------------------------------------------*/

uint32_t ENET_get_speed
   (
      /* [IN] the Ethernet state structure */
      _enet_handle   handle
   )
{
   ENET_CONTEXT_STRUCT_PTR enet_ptr = (ENET_CONTEXT_STRUCT_PTR)handle;

   return (*enet_ptr->PARAM_PTR->ENET_IF->PHY_IF->SPEED)(enet_ptr);
}



/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_max_framesize
*  Returned Value :
*  Comments       :
*
*     Calculate maximum frame size based on user parameters.
*  If the user specifies large buffers, then the max will be the largest
*  frame size (based on vlan).
*
*
*END*-----------------------------------------------------------------*/

uint16_t ENET_max_framesize(uint16_t best_buffer_size, uint16_t num_large_buffers, bool vlan)
{

  if (num_large_buffers || (best_buffer_size==0)) {
      if (vlan) {
         return ENET_FRAMESIZE_VLAN;
      } else {
         return ENET_FRAMESIZE;
      }
   } else {
      return best_buffer_size;
   }
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_get_MTU
*  Returned Value : mtu
*  Comments       :
*        Get the MTU.
*
*END*-----------------------------------------------------------------*/

uint32_t ENET_get_MTU
   (
      /* [IN] the Ethernet state structure */
      _enet_handle   handle
   )
{
   ENET_CONTEXT_STRUCT_PTR enet_ptr = (ENET_CONTEXT_STRUCT_PTR)handle;
   uint32_t  mtu = enet_ptr->PARAM_PTR->TX_BUFFER_SIZE;

   if (!mtu) {
      return ENET_FRAMESIZE_MAXDATA;  // The same regardless of whether VLAN is enabled.
   }

   if (enet_ptr->PARAM_PTR->OPTIONS & ENET_OPTION_VLAN) {
      return(mtu - ENET_FRAMESIZE_HEAD_VLAN - ENET_FRAMESIZE_TAIL);
   } else {
      return(mtu - ENET_FRAMESIZE_HEAD - ENET_FRAMESIZE_TAIL);
   }
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_read_mii
*  Returned Value : TRUE and MII Data register value, FALSE if timeout
*  Comments       :
*    Returns entire MII data register value.
*
*END*-----------------------------------------------------------------*/

bool ENET_read_mii
    (
        /* [IN] the Ethernet state structure */
        _enet_handle handle,
        uint32_t reg_index,
        uint32_t *data,
        uint32_t timeout
    )
{
    ENET_CONTEXT_STRUCT_PTR enet_ptr = (ENET_CONTEXT_STRUCT_PTR)handle;

    return (*enet_ptr->PARAM_PTR->ENET_IF->MAC_IF->PHY_READ)(enet_ptr, reg_index, data, timeout);
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_write_mii
*  Returned Value : TRUE if succeeded, FALSE if timeout
*    Writes entire MII data register.
*
*END*-----------------------------------------------------------------*/

bool ENET_write_mii
    (
        /* [IN] the Ethernet state structure */
        _enet_handle handle,
        uint32_t reg_index,
        uint32_t data,
        uint32_t timeout
    )
{
    ENET_CONTEXT_STRUCT_PTR enet_ptr = (ENET_CONTEXT_STRUCT_PTR)handle;

    return (*enet_ptr->PARAM_PTR->ENET_IF->MAC_IF->PHY_WRITE)(enet_ptr, reg_index, data, timeout);
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_get_phy_addr
*  Returned Value : phy address or -1
*  Comments       :
*        Retrieves the PHY address of an initialized device.
*
*END*-----------------------------------------------------------------*/

uint32_t ENET_get_phy_addr
    (
        /* [IN] the Ethernet state structure */
        _enet_handle handle
    )
{
    ENET_CONTEXT_STRUCT_PTR enet_ptr = (ENET_CONTEXT_STRUCT_PTR)handle;

    if (enet_ptr != NULL)
    {
        return (uint32_t)(enet_ptr->PHY_ADDRESS);
    }
    return (uint32_t)-1;
}


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_phy_registers
*  Returned Value : phy registers
*  Comments       :
*        Retrieves the PHY registers.
*
*END*-----------------------------------------------------------------*/

bool ENET_phy_registers
   (
      _enet_handle  handle,
      uint32_t      num_regs,
      uint32_t      *regs_ptr
   )
{
   ENET_CONTEXT_STRUCT_PTR enet_ptr = (ENET_CONTEXT_STRUCT_PTR)handle;
   uint32_t  i;
   bool  all_read=TRUE;
   for (i=0;i<num_regs;i++) {
      *regs_ptr=0;
      if (!(*enet_ptr->PARAM_PTR->ENET_IF->MAC_IF->PHY_READ)(enet_ptr, i, regs_ptr, 0x1000))
      {
         all_read=FALSE;
      }
      regs_ptr++;
   }
   return all_read;
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_get_device_handle
*  Returned Value : device_handle
*  Comments       :
*        Retrieves the device_handle
*
*END*-----------------------------------------------------------------*/

_enet_handle ENET_get_device_handle(uint32_t mac_number)
{
   ENET_CONTEXT_STRUCT_PTR enet_ptr = NULL;

   enet_ptr = _mqx_get_io_component_handle(IO_ENET_COMPONENT);
   
   while (enet_ptr) {
      if (enet_ptr->PARAM_PTR->ENET_IF->MAC_NUMBER == mac_number)
         return enet_ptr;
      
      enet_ptr = enet_ptr->NEXT;
   }
   
  return NULL;
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_get_next_device_handle
*  Returned Value : device_handle
*  Comments       :
*        Retrieves the next device_handle
*
*END*-----------------------------------------------------------------*/
_enet_handle ENET_get_next_device_handle(_enet_handle handle)
{
   ENET_CONTEXT_STRUCT_PTR enet_ptr = (ENET_CONTEXT_STRUCT_PTR) handle;
   
   if (enet_ptr != NULL) {
       return enet_ptr->NEXT;
   }
   return NULL;
}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_get_options
*  Returned Value : Bitwise of options (ENET_OPTION_xxx),
*                   set during initalisation of the ENET module.
*
*END*-----------------------------------------------------------------*/
uint32_t ENET_get_options(_enet_handle   handle)
{
    ENET_CONTEXT_STRUCT_PTR enet_ptr = (ENET_CONTEXT_STRUCT_PTR)handle;
    
    if(enet_ptr && enet_ptr->PARAM_PTR)
        return enet_ptr->PARAM_PTR->OPTIONS;
    else
        return 0;
}
