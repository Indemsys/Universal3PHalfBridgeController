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
*  This file contains definitions that belongs to the ksz8041 PHY chip
*
*
*END************************************************************************/

#include "mqx.h"
#include "bsp.h"
#include "bsp_prv.h"
#include "enet.h"
#include "enetprv.h"
#include "phy_ksz8041.h"

static bool phy_ksz8041_discover_addr(ENET_CONTEXT_STRUCT_PTR enet_ptr);
static bool phy_ksz8041_init(ENET_CONTEXT_STRUCT_PTR enet_ptr);
static uint32_t phy_ksz8041_get_speed(ENET_CONTEXT_STRUCT_PTR enet_ptr);
static bool phy_ksz8041_get_link_status(ENET_CONTEXT_STRUCT_PTR enet_ptr);

const ENET_PHY_IF_STRUCT phy_ksz8041_IF = {
  phy_ksz8041_discover_addr,
  phy_ksz8041_init,
  phy_ksz8041_get_speed,
  phy_ksz8041_get_link_status
};
  
  

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : phy_ksz8041_discover_addr
*  Returned Value : none
*  Comments       :
*    Scan possible PHY addresses looking for a valid device
*
*END*-----------------------------------------------------------------*/

static bool phy_ksz8041_discover_addr
   (
       ENET_CONTEXT_STRUCT_PTR     enet_ptr
   )
{ 
   uint32_t              i;
   uint32_t              id;

   for (i = 0; i < 32; i++) {
      id = 0;
      enet_ptr->PHY_ADDRESS = i;
      if ((*enet_ptr->PARAM_PTR->ENET_IF->MAC_IF->PHY_READ)(enet_ptr, PHY_MII_REG_PHYIR1, &id, MII_TIMEOUT)) 
      {
          if ((id != 0) && (id != 0xffff)) 
          {
                return TRUE;
          }
      } 
   } 
   
   return FALSE; 
} 


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : phy_ksz8041_init
*  Returned Value : bool
*  Comments       :
*    Wait for PHY to automatically negotiate its configuration
*
*END*-----------------------------------------------------------------*/

static bool phy_ksz8041_init
   (
       ENET_CONTEXT_STRUCT_PTR     enet_ptr
   )
{ 
   uint32_t              phy_status=0;

   if (enet_ptr->PARAM_PTR->OPTIONS & ENET_OPTION_PHY_LOOPBACK) {
      if ((*enet_ptr->PARAM_PTR->ENET_IF->MAC_IF->PHY_READ)(enet_ptr, PHY_MII_REG_CR, &phy_status, MII_TIMEOUT)) {
         phy_status |= PHY_MII_REG_CR_LOOP;
         (*enet_ptr->PARAM_PTR->ENET_IF->MAC_IF->PHY_WRITE)(enet_ptr, PHY_MII_REG_CR, phy_status, MII_TIMEOUT);
         return TRUE;
      }
   } else {
      if ((*enet_ptr->PARAM_PTR->ENET_IF->MAC_IF->PHY_READ)(enet_ptr, PHY_MII_REG_SR, &phy_status, MII_TIMEOUT)) {
         if (phy_status & PHY_MII_REG_SR_AN_ABLE) { 
            // Has auto-negotiate ability
            int i;
            for (i = 0; i < 3 * BSP_ALARM_FREQUENCY; i++) {
               if ((*enet_ptr->PARAM_PTR->ENET_IF->MAC_IF->PHY_READ)(enet_ptr, PHY_MII_REG_SR, &phy_status, MII_TIMEOUT)) 
                  if ((phy_status & PHY_MII_REG_SR_AN_COMPLETE) != 0) 
                     return TRUE;
               _time_delay(BSP_ALARM_RESOLUTION);
            }  
         }
         return TRUE;
      }  
   }  
   return FALSE;
}  


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : phy_ksz8041_get_speed
*  Returned Value : uint32_t - connection speed
*  Comments       :
*    Determine if connection speed is 10 or 100 Mbit
*
*END*-----------------------------------------------------------------*/

static uint32_t phy_ksz8041_get_speed
   (
       ENET_CONTEXT_STRUCT_PTR     enet_ptr
   )
{ 
   uint32_t speed_status;

   if ((*enet_ptr->PARAM_PTR->ENET_IF->MAC_IF->PHY_READ)(enet_ptr, PHY_MII_REG_PHYC2, &speed_status, MII_TIMEOUT)) {
      speed_status &= PHY_MII_REG_PHYC2_OP_MODE_MASK;
  
      if ((PHY_MII_REG_PHYC2_OP_MODE_10_HD==speed_status) || (PHY_MII_REG_PHYC2_OP_MODE_10_FD==speed_status)) {
         return 10;
      } else if ((PHY_MII_REG_PHYC2_OP_MODE_100_HD==speed_status) || (PHY_MII_REG_PHYC2_OP_MODE_100_FD==speed_status)) {
         return 100;
      }
   }
   return 0;
} 

  
/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : phy_ksz8041_get_link_status
*  Returned Value : TRUE if link active, FALSE otherwise
*  Comments       : 
*    Get actual link status.
*
*END*-----------------------------------------------------------------*/

static bool phy_ksz8041_get_link_status
   (
       ENET_CONTEXT_STRUCT_PTR     enet_ptr
   )
{ 
   uint32_t data;
   bool res = FALSE;
   
   if ((*enet_ptr->PARAM_PTR->ENET_IF->MAC_IF->PHY_READ)(enet_ptr, PHY_MII_REG_CR, &data, MII_TIMEOUT))
   {
       /* After power up, when PHY does not fully initial PHY_MII_REG_SR_LINK_STATUS bit 
        * is '1'. So if we get link status in this case, the return result is always "Connected" 
        * even when we haven't plug the cable in yet. 
        * Check if PHY in normal operation -> ready to get link status */
       if ((data & PHY_MII_REG_CR_RESET) == 0 )
       {
           /* Some PHY (e.g.DP8340) returns "unconnected" and than "connected" state
           *  just to show that was transition event from one state to another.
           *  As we need only curent state,  read 2 times and return 
           *  the current/latest state. */
           if ((*enet_ptr->PARAM_PTR->ENET_IF->MAC_IF->PHY_READ)(enet_ptr, PHY_MII_REG_SR, &data, MII_TIMEOUT)) {
              if ((*enet_ptr->PARAM_PTR->ENET_IF->MAC_IF->PHY_READ)(enet_ptr, PHY_MII_REG_SR, &data, MII_TIMEOUT))
              {
                 res = ((data & PHY_MII_REG_SR_LINK_STATUS) != 0) ? TRUE : FALSE;
              }
           }
       }
   }

  return res;
}
