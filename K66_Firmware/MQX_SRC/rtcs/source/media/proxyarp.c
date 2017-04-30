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
*   This file contains a hook into ARP to allow an
*   application to do ProxyARP.
*
*
*END************************************************************************/

#include <rtcs.h>

#if RTCSCFG_ENABLE_IP4

#include "rtcs_prv.h"
#include "tcpip.h"
#include "arp.h"

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ARP_do_proxy
*  Returned Value : TRUE or FALSE
*  Comments       :
*        Decides whether or not to send ARP replies on behalf
*        of another IP host.  Default is to never do ProxyARP,
*        but this function can be replaced in an application.
*
*END*-----------------------------------------------------------------*/

bool ARP_do_proxy
   (
      IP_IF_PTR      iflocal,
            /* [IN] the interface that the ARP request arrived on */
      _ip_address    iplocal
            /* [IN] the IP address to test */
   )
{ /* Body */

   return FALSE;

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/* EOF */
