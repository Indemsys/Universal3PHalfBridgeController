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
*   This file contains the ENET_get_stats utility
*   function.
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>

#include "enet.h"
#include "enetprv.h"

#if BSPCFG_ENABLE_ENET_STATS
/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ENET_get_stats
*  Returned Value : pointer to the statistics structure
*  Comments       :
*        Retrieves the Ethernet statistics for an initialized device.
*
*END*-----------------------------------------------------------------*/

ENET_STATS_PTR ENET_get_stats
   (
      /* [IN] the Ethernet state structure */
      _enet_handle   handle
   )
{
   return (ENET_STATS_PTR)&((ENET_CONTEXT_STRUCT_PTR)handle)->STATS;
} 

#endif
/* EOF */
