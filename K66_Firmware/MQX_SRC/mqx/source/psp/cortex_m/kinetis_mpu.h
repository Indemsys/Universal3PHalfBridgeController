/*HEADER**********************************************************************
*
* Copyright 2011 Freescale Semiconductor, Inc.
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
*   This file contains header for basic mpu settings.
*
*
*END************************************************************************/

#ifndef __kinetis_mpu_h__
#define __kinetis_mpu_h__

#ifdef __cplusplus
extern "C" {
#endif
    
_mqx_uint _kinetis_mpu_init(void);
_mqx_uint _kinetis_mpu_enable(void);
_mqx_uint _kinetis_mpu_disable(void);
_mqx_uint _kinetis_mpu_add_region(unsigned char *start, unsigned char *end, _mqx_uint flags);
_mqx_uint _kinetis_mpu_sw_check(uint32_t addr, _mem_size size, uint32_t flags);
_mqx_uint _kinetis_mpu_sw_check_mask(uint32_t addr, _mem_size size, uint32_t flags, uint32_t mask);
    
#ifdef __cplusplus
}
#endif

#endif
