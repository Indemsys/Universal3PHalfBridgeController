#ifndef __rip_prv_h__
#define __rip_prv_h__
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
*   Private definitions for the RIP protocol layer.
*
*
*END************************************************************************/

typedef struct rip_info {
   TCPIP_EVENT TIMEOUT; /* used for timeout _AND_ garbage collecting */
   uint32_t     METRIC;
   uint32_t     RT_TAG;
   bool     CHANGED_F;
   IP_IF_PTR   IPIFSRC; /* the incoming interface of this route */
}  RIP_INFO, * RIP_INFO_PTR;

#endif   /* __rip_prv_h__ */
/* EOF */
