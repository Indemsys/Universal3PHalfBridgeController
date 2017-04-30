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
*   This file contains the MACNET MII Interface functions.
*
*
*END************************************************************************/

#include "mqx.h"
#include "bsp.h"
#include "enet.h"
#include "enetprv.h"
#include "macnet_prv.h"


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_mii_enabled
*  Returned Value : bool
*  Comments       :
*    We can only read the PHY registers if the PHY is enabled
*
*END*-----------------------------------------------------------------*/

static bool MACNET_mii_enabled
   (
		   ENET_MemMapPtr macnet_ptr
   )
{  
   return (macnet_ptr->MSCR & 0x7E) != 0;
} 

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_read_write_mii
*  Returned Value : TRUE and MII Data register value, FALSE if timeout
*  Comments       :
*    Return entire MII data register value
*
*END*-----------------------------------------------------------------*/

static bool MACNET_read_write_mii
   (
      ENET_MemMapPtr             macnet_ptr,
      unsigned char                      phy_addr,
      uint32_t                    reg_index,
      uint32_t                    op,
      uint32_t                    write_data,
      uint32_t                *read_data_ptr,
      uint32_t                    timeout
   )
{
   uint32_t                    tm;
    
   if (macnet_ptr == NULL) 
      return FALSE;
   
   if (!MACNET_mii_enabled(macnet_ptr))
        return FALSE;
    
   // Clear the MII interrupt bit 
   macnet_ptr->EIR = ENET_EIR_MII_MASK;

   // Kick-off the MII read or write operation 
   macnet_ptr->MMFR = (uint32_t)(0 
      | (ENET_MMFR_ST_MASK & (0x01 << ENET_MMFR_ST_SHIFT))
      | op
      | (ENET_MMFR_PA_MASK & (phy_addr << ENET_MMFR_PA_SHIFT))
      | (ENET_MMFR_RA_MASK & (reg_index << ENET_MMFR_RA_SHIFT))
      | (ENET_MMFR_TA_MASK & (0x02 << ENET_MMFR_TA_SHIFT))
      | (write_data & 0xffff));

    // Poll for MII complete
   for (tm = 0; tm < timeout; tm++)
   {
      if(macnet_ptr->EIR & ENET_EIR_MII_MASK)
         break;
      _time_delay(0);
   }

   if (tm != timeout) 
      if (read_data_ptr) 
         *read_data_ptr = (ENET_MMFR_DATA_MASK & (macnet_ptr->MMFR));

   // Clear the MII interrupt bit 
   macnet_ptr->EIR = ENET_EIR_MII_MASK;

   return (tm != timeout);
}


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_read_mii
*  Returned Value : TRUE and MII Data register value, FALSE if timeout
*  Comments       :
*    Return entire MII data register value
*
*END*-----------------------------------------------------------------*/

bool MACNET_read_mii
    (
        ENET_CONTEXT_STRUCT_PTR enet_ptr,
        uint32_t reg_index,
        uint32_t *data,
        uint32_t timeout
    )
{
    MACNET_CONTEXT_STRUCT_PTR macnet_context_ptr = (MACNET_CONTEXT_STRUCT_PTR)enet_ptr->MAC_CONTEXT_PTR;

    return MACNET_read_write_mii(macnet_context_ptr->PHY_PTR, enet_ptr->PHY_ADDRESS, reg_index, ENET_MMFR_OP_MASK & (0x02 << ENET_MMFR_OP_SHIFT), 0, data, timeout);
}


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : MACNET_write_mii
*  Returned Value : TRUE if success, FALSE if timeout
*  Comments       :
*    Write MII data register value
*
*END*-----------------------------------------------------------------*/

bool MACNET_write_mii
    (
        ENET_CONTEXT_STRUCT_PTR enet_ptr,
        uint32_t reg_index,
        uint32_t data,
        uint32_t timeout
    )
{
    MACNET_CONTEXT_STRUCT_PTR macnet_context_ptr = (MACNET_CONTEXT_STRUCT_PTR)enet_ptr->MAC_CONTEXT_PTR;

    return MACNET_read_write_mii(macnet_context_ptr->PHY_PTR, enet_ptr->PHY_ADDRESS, reg_index, ENET_MMFR_OP_MASK & (0x01 << ENET_MMFR_OP_SHIFT), data, NULL, timeout);
}
