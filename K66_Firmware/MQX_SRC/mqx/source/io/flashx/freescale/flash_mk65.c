/*HEADER**********************************************************************
*
* Copyright 2010 Freescale Semiconductor, Inc.
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
*   The file contains device specific flash functions.
*
*
*END************************************************************************/

#include "mqx.h"
#include "bsp.h"
#include "flashx.h"
#include "flashxprv.h"
#include "flash_ftfe.h"
#include "flash_ftfe_prv.h"
#include "flash_mk65.h"

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _bsp_get_ftfe_address
* Returned Value   : Address upon success
* Comments         :
*    This function returns the base register address of the FTFE.
*
*END*----------------------------------------------------------------------*/
void *_bsp_get_ftfe_address
(
    void
)
{
    return (void *)(FTFE_BASE_PTR);
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : kinetis_flash_invalidate_cache
* Returned Value   : none
* Comments         :
*    Invalidate flash cache to expose flash changes.
*
*END*----------------------------------------------------------------------*/
void kinetis_flash_invalidate_cache
(
    /* [IN] What exactly to invalidate */
    volatile uint32_t flags
) 
{
   /* Invalidate flash cache block 0 */
      if (flags & (FLASHX_INVALIDATE_CACHE_BLOCK0 | FLASHX_INVALIDATE_CACHE_BLOCK1))
      {
          FMC_PFB0CR |= FMC_PFB0CR_S_B_INV_MASK | FMC_PFB0CR_CINV_WAY_MASK;
      }
}

void kinetis_flash_invalidate_cache_end(void) {}
