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
*   This file contains the ENET_get_address utility
*   function.
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>

#include "enet.h"
#include "enetprv.h"

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_get_address
*  Returned Value : ENET_OK or error code
*  Comments       :
*        Retrieves the Ethernet address of an initialized device.
*
*END*-----------------------------------------------------------------*/

uint32_t ENET_get_address
   (
      /* [IN] the Ethernet state structure */
      _enet_handle   handle,

      /* [OUT] the local Ethernet address */
      _enet_address  address
   )
{
   ENET_CONTEXT_STRUCT_PTR enet_ptr = (ENET_CONTEXT_STRUCT_PTR)handle;

   eaddrcpy(address, enet_ptr->ADDRESS);

   return ENET_OK;

}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_get_mac_address
*  Returned Value : ENET_OK or error code
*  Comments       :
*        Retrieves the Ethernet address of a device.
*
*
*
*END*-----------------------------------------------------------------*/

uint32_t ENET_get_mac_address
   (
      uint32_t        device,
      uint32_t        value,
      _enet_address  address
   )

{
   return _bsp_get_mac_address(device,value,address);
}


/* EOF */
