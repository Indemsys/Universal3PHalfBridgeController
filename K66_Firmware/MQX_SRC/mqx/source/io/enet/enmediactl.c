/*HEADER**********************************************************************
*
* Copyright 2009 Freescale Semiconductor, Inc.
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
*   This file contains the Ethernet IOCTL implementation
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>

#include "enet.h"
#include "enetprv.h"

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_mediactl
*  PARAMS         : handle -> handle to enet layer.
*                   command_id -> ENET command to execute.
*                   inout_param -> input/output data for command.
*  Returned Value : ENET_OK or error code
*  Comments       :
*        Joins a multicast group on an Ethernet channel.
*
*END*-----------------------------------------------------------------*/
uint32_t ENET_mediactl
   (
      /* [IN] the Ethernet state structure */
      _enet_handle      handle,

      /* [IN] IOCTL COMMAND */
      uint32_t           command_id,

      /* [IN]/[OUT] input/output param */
      void       *inout_param
   )
{ 
   uint32_t error = ENET_ERROR;
   ENET_CONTEXT_STRUCT_PTR enet_ptr = (ENET_CONTEXT_STRUCT_PTR)handle;

   #if MQX_CHECK_ERRORS
      if (NULL == handle) {
         return ENET_ERROR;
      } /* Endif */
   #endif

   /*
   ** This function can be called from any context, and it needs mutual
   ** exclusion with itself and with ENET_leave.
   */
   if (enet_ptr->PARAM_PTR->ENET_IF->MAC_IF->MEDIACTL)
   {  
      error =  (*enet_ptr->PARAM_PTR->ENET_IF->MAC_IF->MEDIACTL)(enet_ptr, command_id,inout_param);
   }
   
   return error;
}
/* EOF */
