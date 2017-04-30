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
*   This file contains the interface functions for the
*   IPCP client.
*
*
*END************************************************************************/
#include <rtcs.h>

#if RTCSCFG_ENABLE_PPP && !PLATFORM_SDK_ENABLED

#if MQX_USE_IO_OLD
  #include <fio.h>
#else
  #include <stdio.h>
#endif
#include "rtcs_prv.h"
#include "tcpip.h"
#include "ipcp.h"


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_if_bind_IPCP
* Returned Values : uint32_t (error code)
* Comments        :
*     Initialize a PPP interface.
*
*END*-----------------------------------------------------------------*/

uint32_t RTCS_if_bind_IPCP
   (
      _rtcs_if_handle         handle,
         /* [IN] the RTCS interface state structure */
      IPCP_DATA_STRUCT_PTR    data_ptr
         /* [IN/OUT] IPCP parameters */
   )
{ /* Body */
   TCPIP_PARM_IPCP         parms;

   if (handle==NULL) 
   {
      return RTCSERR_INVALID_PARAMETER; 
   }
   
   parms.handle = handle;
   parms.data   = data_ptr;

   return RTCSCMD_issue(parms, IPCP_bind);

} /* Endbody */

#endif // RTCSCFG_ENABLE_PPP
