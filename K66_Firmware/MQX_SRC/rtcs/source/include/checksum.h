#ifndef __checksum_h__
#define __checksum_h__
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
*   This file contains the IP checksum function prototypes.
*
*
*END************************************************************************/


/***************************************
**
** Code macros
**
*/

#define IP_Sum_invert(s)   (((s) == 0xFFFF) ? (s) : ~(s) & 0xFFFF)


/***************************************
**
** Prototypes
**
*/
#ifdef __cplusplus
extern "C" {
#endif

uint16_t IP_Sum_immediate
(
   uint16_t,             /* [IN] initial sum           */
   uint16_t              /* [IN] number to add to sum  */
);

uint16_t IP_Sum_PCB
(
   uint16_t,             /* [IN] initial sum           */
   RTCSPCB_PTR          /* [IN] the PCB               */
);

uint16_t IP_Sum_pseudo
(
   uint16_t,             /* [IN] initial sum           */
   RTCSPCB_PTR,         /* [IN] the PCB               */
   int32_t               /* [IN] IP layer              */
);

#if RTCSCFG_ENABLE_IP6
    uint16_t IP6_Sum_pseudo ( uint16_t, RTCSPCB_PTR );
#endif

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
