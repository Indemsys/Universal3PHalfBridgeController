#ifndef __dhcpsrv_h__
#define __dhcpsrv_h__
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
*   Dynamic Host Configuration Protocol definitions.
*
*
*END************************************************************************/


/***************************************
**
** Constants
**
*/

#define DHCPTIME_OFFER  10       /* 10 seconds */

/* Start CR 1547 */
#define DHCPSVR_FLAG_DO_PROBE   0x00000001
/* End CR 1547 */

/***************************************
**
** Type definitions
**
*/

typedef struct dhcpsrv_options_struct {

   uint32_t        COUNT;

   _ip_address    SERVERID;
   uint32_t        LEASE;
   uint32_t        MASK;

   unsigned char      *OPTION_PTR;
   uint32_t        OPTION_LEN;

   _ip_address    SADDR;
   unsigned char          SNAME[64];
   unsigned char          FILE[128];

} DHCPSRV_OPTIONS_STRUCT, * DHCPSRV_OPTIONS_STRUCT_PTR;

typedef struct dhcpsrv_addr_struct {

   struct dhcpsrv_addr_struct       *NEXT;
   uint32_t                    EXPIRE;
   uint32_t                    OFFER;
   uint32_t                    CLIENTID_LEN;
   unsigned char                  *CLIENTID_PTR;

   _ip_address                IP_ADDR;
   DHCPSRV_OPTIONS_STRUCT_PTR OPTIONS;

} DHCPSRV_ADDR_STRUCT, * DHCPSRV_ADDR_STRUCT_PTR;

typedef struct dhcpsrv_state_struct {

   unsigned char                   SND_BUFFER[DHCP_MSGSIZE_MIN];
   unsigned char                   RCV_BUFFER[DHCP_MSGSIZE_MIN];
   uint32_t                 SND_BUFFER_LEN;
   uint32_t                 RCV_BUFFER_LEN;
   uint32_t                 SOCKET;
   uint32_t                 TIME;
   _rtcs_mutex             IPLIST_SEM;
   /* Start CR 1547 */
   uint32_t                 FLAGS;
   /* End CR 1547 */
   DHCPSRV_ADDR_STRUCT_PTR IP_AVAIL;
   DHCPSRV_ADDR_STRUCT_PTR IP_OFFERED;
   DHCPSRV_ADDR_STRUCT_PTR IP_LEASED;
   /* Start CR 1547 */
   DHCPSRV_ADDR_STRUCT_PTR IP_TAKEN;
   /* End CR 1547 */

} DHCPSRV_STATE_STRUCT, * DHCPSRV_STATE_STRUCT_PTR;


#endif
/* EOF */
