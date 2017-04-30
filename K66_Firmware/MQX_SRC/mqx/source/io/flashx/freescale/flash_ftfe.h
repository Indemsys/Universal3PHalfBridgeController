#ifndef __flash_ftfe_h__
#define __flash_ftfe_h__
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
*   The file contains function prototypes and defines for the internal 
*   flash driver.
*
*
*END************************************************************************/

/*----------------------------------------------------------------------*/
/* 
**              DEFINED VARIABLES
*/

extern const FLASHX_BLOCK_INFO_STRUCT _flashx_kinetisN_block_map[];
extern const FLASHX_BLOCK_INFO_STRUCT _flashx_kinetisX_block_map[];
extern const FLASHX_BLOCK_INFO_STRUCT _flashx_mcf51xx_plus_block_map[];

extern const FLASHX_DEVICE_IF_STRUCT _flashx_ftfe_if;

#endif //__flash_ftfl_h__
