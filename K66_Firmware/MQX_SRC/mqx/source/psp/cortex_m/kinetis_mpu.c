
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
* See license agreement file for full license terms including other restrictions.
*****************************************************************************
*
* Comments:
*
*   This file contains source for basic mpu settings.
*
*
*END************************************************************************/

#include "mqx_inc.h"
#include "kinetis_mpu.h"

/*!
 * \brief Disable MPU on cortex
 *
 * \return Always return MQX_OK.
 */
_mqx_uint _kinetis_mpu_disable(void) {
    MPU_CESR &= ~MPU_CESR_VLD_MASK;

    return MQX_OK;
}


/*!
 * \brief Enable MPU on cortex
 *
 * \return Always return MQX_OK.
 */
_mqx_uint _kinetis_mpu_enable(void) {
    MPU_CESR |= MPU_CESR_VLD_MASK;

    return MQX_OK;
}


/*!
 * \brief Initialize MPU and clear all active records in MPU
 *
 * \return Always return MQX_OK.
 */
_mqx_uint _kinetis_mpu_init(void) {
    _mqx_uint i;

    MPU_CESR &= ~MPU_CESR_VLD_MASK;

    for (i = 0; i < CORTEX_MPU_REC; i++) {
        MPU_WORD(i, 2) = 0;
    }

    return MQX_OK;
}


/*!
 * \brief 	Add new region to MPU
 * 
 * \param[in] start mpu region start address
 * \param[in] end mpu region end address
 * \param[in] flags access rights, direct value - cortex mpu specific
 *
 * \return uint32_t Always return MQX_OK.
 */
_mqx_uint _kinetis_mpu_add_region
    (
        // [IN] mpu region start address
        unsigned char *start,
        // [IN] mpu region end address
        unsigned char *end,
        // [IN] access rights, direct value - cortex mpu specific
        _mqx_uint flags
    )
{
    _mqx_uint i;
    _mqx_uint res = MQX_OK;

    for (i = 1; (i < CORTEX_MPU_REC) && (MPU_WORD(i, 3) & MPU_WORD_VLD_MASK); i++) {}

    if (i < CORTEX_MPU_REC) {
        MPU_WORD(i, 0) = (_mem_size)start & MPU_WORD_SRTADDR_MASK;
        MPU_WORD(i, 1) = ((_mem_size)end & MPU_WORD_ENDADDR_MASK) | 0x01f;
        MPU_WORD(i, 2) = flags;
        MPU_WORD(i, 3) = MPU_WORD_VLD_MASK;
    }
    else
        res = MQX_ERROR;

    return res;
}


/*!
 * \brief 	Software check for memory access (check mpu regions) - this is crutch for ...
 * 
 * \param addr cheked block start address
 * \param[in] size checked block size
 * \param flags required access flags, direct value - cortex mpu specific
 *
 * \return uint32_t return non zero if any region in mpu contain required range with selected access rights return 0 when mpu don't contain any ragion with requested range with selected access rights
 */
_mqx_uint _kinetis_mpu_sw_check
    (
        // [IN] cheked block start address
        uint32_t addr,
        // [IN] checked block size
        _mem_size size,
        // [IN] required access flags, direct value - cortex mpu specific
        uint32_t flags
    )
{
    _mqx_uint i, res = 0;

    for (i = 0; i < CORTEX_MPU_REC; i++) {
        if ((MPU_WORD(i, 3) & MPU_WORD_VLD_MASK) && ((addr >= MPU_WORD(i, 0) && addr <= MPU_WORD(i, 1)) || ((addr + size - 1) >= MPU_WORD(i, 0) && (addr + size - 1) <= MPU_WORD(i, 1)))) {
            // founded record, check flags

            // only for core access
            if ((MPU_WORD_M0UM(MPU_WORD(i, 2)) & flags) == flags) {
                res = 1;
                break;
            }
        }
    }

    return res;
}

_mqx_uint _kinetis_mpu_sw_check_mask
    (
        // [IN] cheked block start address
        uint32_t addr,
        // [IN] checked block size
        _mem_size size,
        // [IN] required access flags, direct value - cortex mpu specific
        uint32_t flags,
        uint32_t mask
    )
{
    _mqx_uint i, res = 0, state = 0;

    for (i = 0; i < CORTEX_MPU_REC; i++) {
        if ((MPU_WORD(i, 3) & MPU_WORD_VLD_MASK) && ((addr >= MPU_WORD(i, 0) && addr <= MPU_WORD(i, 1)) || ((addr + size - 1) >= MPU_WORD(i, 0) && (addr + size - 1) <= MPU_WORD(i, 1)))) {
            // founded record, check flags

            state |= MPU_WORD_M0UM(MPU_WORD(i, 2));
        }
    }

    // only for core access
    if ((state & mask) == flags) {
        res = 1;
    }

    return res;
}
