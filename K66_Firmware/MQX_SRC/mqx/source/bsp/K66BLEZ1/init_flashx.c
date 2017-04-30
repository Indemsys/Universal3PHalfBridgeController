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
*   This file contains the global generic settings for FLASHX driver.
*
*
*END************************************************************************/

#include "mqx.h"
#include "bsp.h"

#ifdef __CC_ARM
/* suppress realview compiler warning "pointer points outside of underlying objects".
 * "flashx:code" file has end address determined by __FLASHX_START_ADDR - 1 which is
 * outside of __FLASHX_START_ADDR object
 */
#pragma diag_suppress 170
#endif

const FLASHX_FILE_BLOCK _bsp_flashx_file_blocks[] = {
    { "bank0", BSP_INTERNAL_FLASH_BASE                                    , BSP_INTERNAL_FLASH_BASE + 1 * (BSP_INTERNAL_FLASH_SIZE / 4) - 1 },
    { "bank1", BSP_INTERNAL_FLASH_BASE + 1 * (BSP_INTERNAL_FLASH_SIZE / 4), BSP_INTERNAL_FLASH_BASE + 2 * (BSP_INTERNAL_FLASH_SIZE / 4) - 1 },
    { "bank2", BSP_INTERNAL_FLASH_BASE + 2 * (BSP_INTERNAL_FLASH_SIZE / 4), BSP_INTERNAL_FLASH_BASE + 3 * (BSP_INTERNAL_FLASH_SIZE / 4) - 1 },
    { "bank3", BSP_INTERNAL_FLASH_BASE + 3 * (BSP_INTERNAL_FLASH_SIZE / 4), BSP_INTERNAL_FLASH_BASE + 4 * (BSP_INTERNAL_FLASH_SIZE / 4) - 1 },
// swap file definition according to the default value of BSPCFG_SWAP_INDICATOR_ADDR and sector size
    { "swap0", BSP_INTERNAL_FLASH_BASE                                , BSP_INTERNAL_FLASH_BASE + (BSP_INTERNAL_FLASH_SIZE / 2) - (1 + BSP_INTERNAL_FLASH_SECTOR_SIZE) },
    { "swap1", BSP_INTERNAL_FLASH_BASE + (BSP_INTERNAL_FLASH_SIZE / 2), BSP_INTERNAL_FLASH_BASE + (BSP_INTERNAL_FLASH_SIZE    ) - (1 + BSP_INTERNAL_FLASH_SECTOR_SIZE) },
// flash space used by application
    { "code",  BSP_INTERNAL_FLASH_BASE, (uint32_t) __FLASHX_START_ADDR - 1 },
// remaining free flash space
    { ""     , (uint32_t) __FLASHX_START_ADDR, (uint32_t) __FLASHX_END_ADDR },
    { NULL   ,                             0,                           0 }
};

const FLASHX_INIT_STRUCT _bsp_flashx_init = {
    0x00000000, /* BASE_ADDR should be 0 for internal flashes */
    _flashx_kinetisN_block_map, /* HW block map for KinetisN devices */
    _bsp_flashx_file_blocks, /* Files on the device defined by the BSP */
    &_flashx_ftfe_if, /* Interface for low level driver */
    32, /* For external devices, data lines for the flash. Not used for internal flash devices. */
    1,  /* Number of parallel external flash devices. Not used for internal flash devices. */
    0,  /* 0 if the write verify is requested, non-zero otherwise */
    NULL /* low level driver specific data */
};
