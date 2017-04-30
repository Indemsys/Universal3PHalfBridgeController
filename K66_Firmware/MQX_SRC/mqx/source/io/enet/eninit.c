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
*   This file contains the MCF52xx Coldfire processor
*   Ethernet initialization
*
*
*END************************************************************************/

#include "mqx.h"
#include "bsp.h"
#include "enet.h"
#include "enetprv.h"


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_initialize_ex
*  Returned Value : ENET_OK or error code
*  Comments       :
*        Initializes the chip (extended version).
*
*END*-----------------------------------------------------------------*/

uint32_t ENET_initialize_ex
   (
         /* [IN] optional parameters */
      const ENET_PARAM_STRUCT *   param_ptr,
         /* [IN] the local Ethernet address */
      _enet_address           address,
         /* [OUT] the Ethernet state structure */
      _enet_handle       *handle
   )
{
   ENET_CONTEXT_STRUCT_PTR enet_ptr = NULL;//, other_enet_ptr;
   uint32_t                 result;
   bool                 vlan;
   
   if (param_ptr == NULL)
      return ENETERR_INVALID_DEVICE;

   if (param_ptr->NUM_RX_BUFFERS < param_ptr->NUM_RX_ENTRIES) 
      return ENETERR_INVALID_INIT_PARAM;
   
   enet_ptr = _mqx_get_io_component_handle(IO_ENET_COMPONENT);
   
   while (enet_ptr) {
      if (enet_ptr->PARAM_PTR->ENET_IF->MAC_NUMBER == param_ptr->ENET_IF->MAC_NUMBER)
         break;
      
      enet_ptr = enet_ptr->NEXT;
   }
   
   if (enet_ptr) {
      *handle = enet_ptr;
   
      return ENETERR_INITIALIZED_DEVICE;
   }
   else
      *handle = NULL;

   /* Allocate the Enet context structure.*/
   enet_ptr = _mem_alloc_system_zero(sizeof(ENET_CONTEXT_STRUCT));
   if (NULL==enet_ptr)
      return ENETERR_ALLOC_CFG;
      
   _mem_set_type((void *)enet_ptr, MEM_TYPE_IO_ENET_CONTEXT_STRUCT);

   /* Initialize the Enet context structure.*/
   eaddrcpy(enet_ptr->ADDRESS, address);
   enet_ptr->PARAM_PTR = param_ptr;
   vlan = (enet_ptr->PARAM_PTR->OPTIONS & ENET_OPTION_VLAN) == ENET_OPTION_VLAN;
   enet_ptr->MaxTxFrameSize = ENET_max_framesize(enet_ptr->PARAM_PTR->TX_BUFFER_SIZE,0,vlan);
   enet_ptr->MaxRxFrameSize = ENET_max_framesize(enet_ptr->PARAM_PTR->RX_BUFFER_SIZE,enet_ptr->PARAM_PTR->NUM_LARGE_BUFFERS,vlan);
   _lwsem_create(&enet_ptr->CONTEXT_LOCK, 1);
    
   /* Initialize the MAC.*/
   result = (*param_ptr->ENET_IF->MAC_IF->INIT)(enet_ptr);
   
    if (ENET_OK == result)
    {
        // Link the driver into the kernel component list
        _mqx_link_io_component_handle(IO_ENET_COMPONENT,enet_ptr, (void **)&enet_ptr->NEXT);

        *handle = enet_ptr;
    } 
    else
    {
        _lwsem_destroy(&enet_ptr->CONTEXT_LOCK);
        _mem_free(enet_ptr);
        *handle = NULL;
    }
   
    return result;
}   

 
/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_initialize
*  Returned Value : ENET_OK or error code
*  Comments       :
*        Initializes the chip.
*
*END*-----------------------------------------------------------------*/

uint32_t ENET_initialize
   (
         /* [IN] the FEC to initialize */
      uint32_t              device,
         /* [IN] the local Ethernet address */
      _enet_address        address,
         /* [IN] optional flags, zero = default, 
            this parameter IS NOT USED! (ignored) */
      uint32_t              flags,
         /* [OUT] the Ethernet state structure */
      _enet_handle    *handle
   )
{ 
   if (device <BSP_ENET_DEVICE_COUNT) {
      return ENET_initialize_ex(&ENET_default_params[device],address,handle);
   } else {
      return ENETERR_INVALID_DEVICE;
   }
}

/* EOF */
