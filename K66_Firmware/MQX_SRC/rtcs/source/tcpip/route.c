/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
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
*   This file contains the implementation of the
*   route core functions.
*
* TODO:
*  - when a aux route is added, how to set the metric ? especially hard
*  in case of a redirect route. i dont have to. a router must discard a
*  icmp redirect. rfc1812.4.2.2.2. i act as a router if IP_cfg_ptr->ROUTE_FN
*  is non null.
*  - even for the static route, a metric is required because rtcs will advertise
*  this route. how to choose the metric unit ? not the same for rip/ospf... 
*  a number between 0 and 64k. thus each protocol can handle it.
*  ospf will get it as is. and rip will divide it by 4096.
*
*
*END************************************************************************/

#include <rtcs.h>

#if RTCSCFG_ENABLE_IP4 

#if MQX_USE_IO_OLD
#include <fio.h>
#else
#include <stdio.h>
#endif
#include "rtcs_prv.h"
#include "tcpip.h"
#include "ip_prv.h"
#include "route.h"

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ROUTE_register
*  Parameters     :
*  Comments       :
*
*END*-----------------------------------------------------------------*/

void ROUTE_register
(
   IP_ROUTE_FN_PTR   fn
)
{ /* Body */
   IP_CFG_STRUCT_PTR    IP_cfg_ptr = RTCS_getcfg( IP );

   fn->NEXT = IP_cfg_ptr->ROUTE_FN;
   IP_cfg_ptr->ROUTE_FN = fn;

} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ROUTE_get
*  Parameters     :
*  Comments       :
*
*END*-----------------------------------------------------------------*/
struct ROUTE_get_struct  {
   _ip_address             netaddr;
   _ip_address             netmask;
   IP_ROUTE_INDIRECT_PTR   gate;
};

static bool ROUTE_get_test
   (
      _ip_address    node_ip,
      _ip_address    node_mask,
      void          *node_data,
      void          *data
   ) 
{ /* Body */
   struct ROUTE_get_struct      *testdata = data;
   IP_ROUTE_PTR                  route = node_data;
   
   if (route && testdata->netaddr == node_ip && 
      testdata->netmask == node_mask)  
   {
      if (route->DIRECT) {
         /* Start SPR P122-0266-20 remove all warnings from RTCS code. */
         /* testdata->gate = (IP_ROUTE_INDIRECT_PTR) route->DIRECT; */
         testdata->gate = (IP_ROUTE_INDIRECT_PTR)((void *)route->DIRECT);
         /* End SPR P122-0266-20. */
      /* Start CR 1135 */
      } else if (route->VIRTUAL) {
         testdata->gate = (IP_ROUTE_INDIRECT_PTR)route->VIRTUAL;
      /* End CR 1135 */
      } else if (route->INDIRECT) { 
         testdata->gate = route->INDIRECT;
      } /* Endif */
      
      return TRUE;   
   } /* Endif */
   
   return FALSE;
} /* Endbody */

IP_ROUTE_INDIRECT_PTR ROUTE_get
(
   _ip_address netaddr, /* [IN] the network address */
   _ip_address netmask  /* [IN] the network mask */
)
{ /* Body */
   IP_CFG_STRUCT_PTR             IP_cfg_ptr = RTCS_getcfg( IP );
   struct ROUTE_get_struct       testdata;
   
   testdata.netaddr = netaddr;
   testdata.netmask = netmask;
   testdata.gate    = NULL;
   
   IPRADIX_walk(&IP_cfg_ptr->ROUTE_ROOT.NODE, ROUTE_get_test, &testdata);
   
   return testdata.gate;
} /* Endbody */

#if 1

static  bool ROUTE_walk_test
   (
      _ip_address    node_ip,
      _ip_address    node_mask,
      void          *node_data,
      void          *data
   ) 
{ /* Body */
   IP_ROUTE_PTR                  route = node_data;
   IP_IF_PTR                     ip_if_ptr;

   if (route != NULL) {
      if (route->DIRECT) {
         printf("DIRECT Route: %d.%d.%d.%d, NetIF=%x,DestIF=%x\n", 
             IPBYTES(route->DIRECT->ADDRESS),route->DIRECT->NETIF->HANDLE, route->DIRECT->DESTIF->HANDLE );
      } else if (route->VIRTUAL) {
         printf("VIRTUAL Route: %d.%d.%d.%d, IS DIRECT=%x,DestIF=%x\n", 
             IPBYTES(route->VIRTUAL->ADDRESS), route->VIRTUAL->IS_DIRECT->HANDLE, route->VIRTUAL->DESTIF->HANDLE);
      } else if (route->INDIRECT) { 
         printf("INDIRECT Route: %d.%d.%d.%d", IPBYTES(route->INDIRECT->GATEWAY) );
         if (route->INDIRECT->IS_DIRECT) {
            ip_if_ptr = (IP_IF_PTR)route->INDIRECT->IS_DIRECT;
            printf(", IS DIRECT IF=%x\n", ip_if_ptr->HANDLE);
         } else {
            printf("\n");
         }
     } /* Endif */
   }

   return FALSE;
} /* Endbody */

#endif

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ROUTE_insert
*  Parameters     :
*  Comments       : 
*
*END*-----------------------------------------------------------------*/

struct ROUTE_insert_struct {
   IP_ROUTE_INDIRECT_PTR   route;
};

static void ROUTE_insert_indirect
   (
      void     **node,   /* ptr to address of the IPRADIX_NODE DATA ptr */
      void          *data    /* ptr to an ip_route_insert_indirect struct   */
   )
{ /* Body */
   struct ROUTE_insert_struct      *insertdata = data;
   IP_ROUTE_PTR route;

   /* Make sure the node is initialized */
   if (!*node) {  
      *node = node;
      ((IP_ROUTE_PTR) *node)->DIRECT = NULL;
      ((IP_ROUTE_PTR) *node)->INDIRECT = NULL;      
      /* Start CR 1135 */
      ((IP_ROUTE_PTR) *node)->VIRTUAL = NULL;      
      /* End CR 1135 */
   } /* endif */
      
   route = *node;
   if (!route->INDIRECT) {                               
      insertdata->route->NEXT = insertdata->route;  
      route->INDIRECT = insertdata->route;            
   } else {
      insertdata->route->NEXT = route->INDIRECT->NEXT;
      route->INDIRECT->NEXT = insertdata->route;
   } /* Endif */

} /* Endbody */


uint32_t ROUTE_insert
(
   IP_ROUTE_INDIRECT_PTR   gate,    /* [IN] the route entry to insert */
   _ip_address             network,
   _ip_address             netmask  
)
{ /* Body */
   IP_CFG_STRUCT_PTR          IP_cfg_ptr = RTCS_getcfg(IP);
   struct ROUTE_insert_struct insertdata;
   
   insertdata.route = gate;
   
   /* Add to gateway list */
   IPRADIX_insert(&IP_cfg_ptr->ROUTE_ROOT.NODE, network & netmask, netmask,
                  IP_cfg_ptr->RADIX_PARTID,
                  ROUTE_insert_indirect, &insertdata);
   
   return RTCS_OK;

} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ROUTE_remove
*  Parameters     :
*  Comments       : 
*
*END*-----------------------------------------------------------------*/
struct ROUTE_remove_struct {
   IP_ROUTE_INDIRECT_PTR   gate;
   _ip_address             network;
   _ip_address             netmask;
   bool                 found;
};

static bool ROUTE_remove_test 
   (
      _ip_address node_ip,
      _ip_address node_mask,
      void       *node_data,
      void       *data
   )
{ /* Body */
   IP_ROUTE_PTR                     route = node_data;
   IP_ROUTE_INDIRECT_PTR            indirect;
   struct ROUTE_remove_struct      *testdata = data;
      
   if (route && route->INDIRECT) {
      indirect = route->INDIRECT;
      do {
         if (indirect == testdata->gate) {
            testdata->found = TRUE;
            testdata->network = node_ip;
            testdata->netmask = node_mask;
            return TRUE;
         } /* Endif */
         indirect = indirect->NEXT;
      } while(indirect != route->INDIRECT);
   } /* Endif */
   
   return FALSE;
} /* Endbody */

uint32_t ROUTE_remove
(
   IP_ROUTE_INDIRECT_PTR gate  /* [IN] the route entry to remove */
)
{ /* Body */
   IP_CFG_STRUCT_PTR             IP_cfg_ptr = RTCS_getcfg(IP);
   IP_ROUTE_FN_PTR               fn;
   struct ROUTE_remove_struct    testdata;
   
   fn = IP_cfg_ptr->ROUTE_FN;
   while (fn) {
      fn->REM_RT(gate);
      fn = fn->NEXT;
   } /* Endwhile */

   testdata.gate = gate;
   testdata.network = INADDR_ANY;
   testdata.netmask = INADDR_ANY;
   testdata.found = FALSE;
   
   IPRADIX_walk(&IP_cfg_ptr->ROUTE_ROOT.NODE, ROUTE_remove_test, &testdata);
   
   if (testdata.found) {
      /* Start CR 1135 */
      IP_route_remove_indirect(gate->GATEWAY, testdata.netmask, 
         testdata.network, gate->FLAGS, gate->METRIC);
      /* End CR 1135 */
   } /* Endif */

   return RTCS_OK;
} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : ROUTE_new_bindif
*  Parameters     :
*  Comments       : 
*
*END*-----------------------------------------------------------------*/

uint32_t ROUTE_new_bindif
(
   IP_ROUTE_DIRECT_PTR bindif  /* [IN] the outgoing binded interface */
)
{ /* Body */
   IP_CFG_STRUCT_PTR    IP_cfg_ptr = RTCS_getcfg(IP);
   IP_ROUTE_FN_PTR      fn;
   
   fn = IP_cfg_ptr->ROUTE_FN;
   while (fn) {
      fn->INIT_IF(bindif);
      fn = fn->NEXT;
   } /* Endwhile */

   return RTCS_OK;
} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */


_ip_address RTCS_get_route
(
   _ip_address netaddr, /* [IN] the network address */
   _ip_address netmask  /* [IN] the network mask */
)
{

#if RTCSCFG_ENABLE_IP4

    IP_ROUTE_INDIRECT_PTR  gate;

    gate = ROUTE_get(netaddr, netmask);
    
    if(gate)
    {
        return gate->GATEWAY;
    }
    else
    {
        return 0;
    }

#else

    return 0;

#endif /* RTCSCFG_ENABLE_IP4 */
} 


void RTCS_walk_route(void)
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   IP_CFG_STRUCT_PTR             IP_cfg_ptr = RTCS_getcfg( IP );
   struct ROUTE_get_struct       testdata;
   
   testdata.netaddr = 0;
   testdata.netmask = 0;
   testdata.gate    = NULL;
   
   IPRADIX_walk(&IP_cfg_ptr->ROUTE_ROOT.NODE, ROUTE_walk_test, &testdata);
   
#endif   
   
} /* Endbody */



/* EOF */
