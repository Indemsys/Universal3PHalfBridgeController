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
*   The file contains functions prototype, defines for the internal 
*   flash driver.
*
*
*END************************************************************************/

#ifndef __flash_mk65_h__
#define __flash_mk65_h__


/*----------------------------------------------------------------------*/
/*
**                    FUNCTION PROTOTYPES
 *----------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

void   *_bsp_get_ftfe_address(void);
void kinetis_flash_invalidate_cache(volatile uint32_t);
void kinetis_flash_invalidate_cache_end(void);

#ifdef __cplusplus
}
#endif

#endif
