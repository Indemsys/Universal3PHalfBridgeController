/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
* Copyright 2004-2008 Embedded Access Inc.
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
*   This file contains definitions that belongs to the PHY chip
*   ksz8041.
*
*
*END************************************************************************/
#ifndef _phy_ksz8041_h_
#define _phy_ksz8041_h_ 1

#ifdef __cplusplus
extern "C" {
#endif

/*
** Register definitions for the PHY.
*/

enum phy_reg {
   PHY_MII_REG_CR       = 0,  // Control Register
   PHY_MII_REG_SR       = 1,  // Status Register
   PHY_MII_REG_PHYIR1   = 2,  // PHY Identification Register 1
   PHY_MII_REG_PHYIR2   = 3,  // PHY Identification Register 2
   PHY_MII_REG_ANAR     = 4,  // A-N Advertisement Register
   PHY_MII_REG_ANLPAR   = 5,  // A-N Link Partner Ability Register
   PHY_MII_REG_ANER     = 6,  // A-N Expansion Register
   PHY_MII_REG_ANNPTR   = 7,  // A-N Next Page Transmit Register
   PHY_MII_REG_ANLPRNPR = 8,  // A-N Link Partner Received Next Page Reg.

   PHY_MII_REG_RXER     = 0x15,  // RX Error counter 
   PHY_MII_REG_ICS      = 0x1b,  //  Interrupt Control Status Register 
   PHY_MII_REG_PHYC1    = 0x1e,  // Phy control 1
   PHY_MII_REG_PHYC2    = 0x1f   // Phy control 2
};


// values for PHY_MII_REG_CR Status Register

#define PHY_MII_REG_CR_AN_ENABLE   0x1000
#define PHY_MII_REG_CR_LOOP        0x4000
#define PHY_MII_REG_CR_RESET       0x8000


// values for PHY_MII_REG_SR Status Register

#define PHY_MII_REG_SR_EXTCAP        0x0001
#define PHY_MII_REG_SR_JABBER        0x0002
#define PHY_MII_REG_SR_LINK_STATUS   0x0004
#define PHY_MII_REG_SR_AN_ABLE       0x0008 // Auto-negotiate ability
#define PHY_MII_REG_SR_REMOTE_FAULT  0x0010
#define PHY_MII_REG_SR_AN_COMPLETE   0x0020 // Auto-negotiate completed


// values for PHY_MII_REG_SR2 Status Register

#define PHY_MII_REG_PHYC2_OP_MODE_MASK    0x001c 
#define PHY_MII_REG_PHYC2_OP_MODE_AN      0x0000 
#define PHY_MII_REG_PHYC2_OP_MODE_10_HD   0x0004 
#define PHY_MII_REG_PHYC2_OP_MODE_100_HD  0x0008 
#define PHY_MII_REG_PHYC2_OP_MODE_10_FD   0x0014 
#define PHY_MII_REG_PHYC2_OP_MODE_100_FD  0x0018 


extern const ENET_PHY_IF_STRUCT phy_ksz8041_IF;
 
#define MII_TIMEOUT                     (0x10000)

#ifdef __cplusplus
}
#endif

#endif /* _phy_ksz8041_h_ */
/* EOF */
