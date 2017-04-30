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
*   This file contains the MCF52xx Ethernet shutdown
*   functions.
*
*
*END************************************************************************/

#include "mqx.h"
#include "bsp.h"
#include "enet.h"
#include "enetprv.h"

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_shutdown
*  Returned Value : ENET_OK or error code
*  Comments       :
*        Stops the chip.
*
*END*-----------------------------------------------------------------*/
uint32_t ENET_shutdown(_enet_handle   handle /* [IN] the Ethernet state structure */)
{  
    ENET_CONTEXT_STRUCT_PTR    enet_ptr;
    uint32_t                    result;
   
    if(handle == NULL)
        return ENETERR_INVALID_DEVICE;
    
    enet_ptr = (ENET_CONTEXT_STRUCT_PTR)handle;

    /* Make sure upper layers have closed the device.*/
    if (enet_ptr->ECB_HEAD) 
        return ENETERR_DEVICE_IN_USE;
      
    /* Notify the lower layer.*/
    result = (*enet_ptr->PARAM_PTR->ENET_IF->MAC_IF->STOP)(enet_ptr);
   
    if (result == ENET_OK)
    {
        _lwsem_destroy(&enet_ptr->CONTEXT_LOCK);
        _mqx_unlink_io_component_handle(IO_ENET_COMPONENT,enet_ptr, (void **)&enet_ptr->NEXT);
        _mem_free((void *) enet_ptr);
    }

    return result;
} 


