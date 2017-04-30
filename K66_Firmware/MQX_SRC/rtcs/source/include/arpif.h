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
*   
*
*
*END************************************************************************/

#ifndef __arpif_h__
#define __arpif_h__

/* ARPIF_*() */
typedef struct arpif_parm {
   TCPIP_PARM           COMMON;
   _rtcs_if_handle      ihandle;
   _ip_address          PADDR;
   char        LADDR[6];
} ARPIF_PARM, * ARPIF_PARM_PTR;

#ifdef __cplusplus
extern "C" {
#endif

extern void ARPIF_delete
   (
      ARPIF_PARM_PTR  parms
   );
extern void ARPIF_add
   (
      ARPIF_PARM_PTR  parms
   );
extern void ARP_display_if_table(_rtcs_if_handle   ihandle);

#ifdef __cplusplus
}
#endif

#endif
