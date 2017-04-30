/*HEADER**********************************************************************
*
* Copyright 2011-2013 Freescale Semiconductor, Inc.
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
*   This file contains the implementation for a one's
*   complement checksum (IPv6 version).
*
*
*END************************************************************************/

#include <rtcs.h>

#if RTCSCFG_ENABLE_IP6

#include "rtcs_prv.h"
#include "tcpip.h"
#include "ip_prv.h"

/************************************************************************
* NAME: IP6_Sum_pseudo
*
* DESCRIPTION: Returns a one's complement checksum of the IP pseudo header
*
*     Note:  This function returns 0 iff all summands are 0..
*************************************************************************/
uint16_t IP6_Sum_pseudo( uint16_t sum /* [IN] initial sum */, RTCSPCB_PTR pcb /* [IN] the PCB */)
{
    IP6_HEADER_PTR iphead = (IP6_HEADER_PTR)RTCSPCB_DATA_NETWORK(pcb);
    uint16_t protocol;

    protocol = RTCSPCB_TRANSPORT_PROTL(pcb);
    sum = _mem_sum_ip(sum, 32, iphead->SOURCE); /* source and dest */
    sum = IP_Sum_immediate(sum, protocol);      /* protocol */
    return sum;
}

#endif /* RTCSCFG_ENABLE_IP6 */
