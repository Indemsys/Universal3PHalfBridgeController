/*
 * Copyright (c) 2013-2014, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "fsl_flexcan_hal.h"
#include <mqx.h>
#include <bsp.h>
#include "fsl_flexcan_int.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

#define FLEXCAN_RX_FIFO_ID_FILTER_FORMATA_EXT_MASK    (0x3FFFFFFFU)  //!< FlexCAN RX FIFO ID filter
                                                                     //!  format A extended mask.
#define FLEXCAN_RX_FIFO_ID_FILTER_FORMATA_EXT_SHIFT   (1U)           //!< FlexCAN RX FIFO ID filter
                                                                     //!  format A extended shift.
#define FLEXCAN_RX_FIFO_ID_FILTER_FORMATA_STD_MASK    (0x3FF80000U)  //!< FlexCAN RX FIFO ID filter
                                                                     //!  format A standard mask.
#define FLEXCAN_RX_FIFO_ID_FILTER_FORMATA_STD_SHIFT   (19U)          //!< FlexCAN RX FIFO ID filter
                                                                     //!  format A standard shift.
#define FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_EXT_MASK    (0x3FFFU)      //!< FlexCAN RX FIFO ID filter
                                                                     //!  format B extended mask.
#define FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_EXT_SHIFT1  (16U)          //!< FlexCAN RX FIFO ID filter
                                                                     //!  format B extended mask.
#define FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_EXT_SHIFT2  (0U)           //!< FlexCAN RX FIFO ID filter
                                                                     //!  format B extended mask.
#define FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_STD_MASK    (0x3FF8U)      //!< FlexCAN RX FIFO ID filter
                                                                     //!  format B standard mask.
#define FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_STD_SHIFT1  (19U)          //!< FlexCAN RX FIFO ID filter
                                                                     //!  format B standard shift1.
#define FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_STD_SHIFT2  (3U)           //!< FlexCAN RX FIFO ID filter
                                                                     //!  format B standard shift2.
#define FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_MASK        (0xFFU)        //!< FlexCAN RX FIFO ID filter
                                                                     //!  format C mask.
#define FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_SHIFT1      (24U)          //!< FlexCAN RX FIFO ID filter
                                                                     //!  format C shift1.
#define FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_SHIFT2      (16U)          //!< FlexCAN RX FIFO ID filter
                                                                     //!  format C shift2.
#define FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_SHIFT3      (8U)           //!< FlexCAN RX FIFO ID filter
                                                                     //!  format C shift3.
#define FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_SHIFT4      (0U)           //!< FlexCAN RX FIFO ID filter
                                                                     //!  format C shift4.

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

/*!
 * @brief Enable FlexCAN controller
 *
 * @param   instance    The FlexCAN instance number.
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_hal_enable(uint8_t instance)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    // Check for low power mode
    if(flexcan_reg_ptr->MCR & CAN_MCR_LPMACK_MASK)
    {
        // Enable clock
        flexcan_reg_ptr->MCR &= (~CAN_MCR_MDIS_MASK);
        // Wait until enabled
        while (flexcan_reg_ptr->MCR & CAN_MCR_LPMACK_MASK){}
    }

    return (kFlexCan_OK);
}

/*!
 * @brief Disable FlexCAN controller
 *
 * @param   instance    The FlexCAN instance number.
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_hal_disable(uint8_t instance)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    // To access the memory mapped registers
    // Entre disable mode (hard reset).
    if((flexcan_reg_ptr->MCR & CAN_MCR_MDIS_MASK) == 0x0)
    {
        // clock disable (module)
        flexcan_reg_ptr->MCR = CAN_MCR_MDIS_MASK;

        // wait until disable mode acknowledged
        while (!(flexcan_reg_ptr->MCR & CAN_MCR_LPMACK_MASK)) {}
    }

    return (kFlexCan_OK);
}

/*!
 * @brief Reset FlexCAN controller
 *
 * @param   instance    The FlexCAN instance number.
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_hal_sw_reset(uint8_t instance)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    // Check for low power mode
    if(flexcan_reg_ptr->MCR & CAN_MCR_LPMACK_MASK)
    {
        // Enable clock
        flexcan_reg_ptr->MCR &= (~CAN_MCR_MDIS_MASK);
        // Wait until enabled
        while (flexcan_reg_ptr->MCR & CAN_MCR_LPMACK_MASK){}
    }

    // Reset the FLEXCAN
    flexcan_reg_ptr->MCR = CAN_MCR_SOFTRST_MASK;

    // Wait for reset cycle to complete
    while (flexcan_reg_ptr->MCR & CAN_MCR_SOFTRST_MASK){}

    // Set Freeze, Halt
    flexcan_reg_ptr->MCR |= (CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);

    // check for freeze Ack
    if(((flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK) != CAN_MCR_FRZACK_MASK) ||
      ((flexcan_reg_ptr->MCR & CAN_MCR_NOTRDY_MASK) != CAN_MCR_NOTRDY_MASK))
        return (kFlexCan_SOFTRESET_FAILED);

    return (kFlexCan_OK);
}

/*!
 * @brief Select the clock source for FlexCAN
 *
 * @param   instance    The FlexCAN instance number.
 * @param   clk         The FlexCAN clock source.
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_hal_select_clk(uint8_t instance, flexcan_clk_source_t clk)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    if(clk == kFlexCanClkSource_Ipbus)
        // Internal bus clock (fsys/2)
        flexcan_reg_ptr->CTRL1 |= CAN_CTRL1_CLKSRC_MASK;
    else if (clk == kFlexCanClkSource_Osc)
        // External clock
        flexcan_reg_ptr->CTRL1 &= (~CAN_CTRL1_CLKSRC_MASK);
    else
        return (kFlexCan_CLOCK_SOURCE_INVALID);

    return (kFlexCan_OK);
}

#if PSP_MQX_CPU_IS_VYBRID
/*!
 * @brief Get all error interrupt flags.
 *
 * @param   instance     The FlexCAN instance number.
 * @return  all error interrupt flags
 */
uint32_t flexcan_hal_get_err_int_flags(uint8_t instance)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    return flexcan_reg_ptr->ERRSR;
}

/*!
 * @brief Clear the error interrupt flags
 *
 * @param   instance     The FlexCAN instance number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_clear_err_int_flag(uint8_t instance)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    flexcan_reg_ptr->ERRSR = 0xD000D;

    return (kFlexCan_OK);
}
#endif

/*!
 * @brief Initialize FlexCAN controller
 *
 * @param   instance    The FlexCAN instance number.
 * @param   data   The FlexCAN platform data.
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_hal_init(uint8_t instance, const flexcan_config_t *data)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;
    uint32_t i;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    // To access the memory mapped registers
    // Entre disable mode (hard reset).
    if((flexcan_reg_ptr->MCR & CAN_MCR_MDIS_MASK) == 0x0)
    {
        // clock disable (module)
        flexcan_reg_ptr->MCR = CAN_MCR_MDIS_MASK;

        // wait until disable mode acknowledged
        while (!(flexcan_reg_ptr->MCR & CAN_MCR_LPMACK_MASK)) {}
    }

    flexcan_reg_ptr->CTRL1 |= CAN_CTRL1_CLKSRC_MASK;

    // Enable clock
    flexcan_reg_ptr->MCR &= (~CAN_MCR_MDIS_MASK);
    // Wait until enabled
    while (flexcan_reg_ptr->MCR & CAN_MCR_LPMACK_MASK){}

    // Reset FLEXCAN, Halt, freeze mode
    if(flexcan_hal_sw_reset(instance))
        return (kFlexCan_INIT_FAILED);

    // Set Freeze, Halt
    flexcan_reg_ptr->MCR |= (CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);

    // Wait for entering the freeze mode
    while((flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK) == 0) {}

    // Set maximum number of message buffers
    flexcan_reg_ptr->MCR |= CAN_MCR_MAXMB((data->max_num_mb -1));

#if PSP_MQX_CPU_IS_VYBRID
    // Configuration to clear Internal RAM for ECC operation
    flexcan_reg_ptr->CTRL2 |= (CAN_CTRL2_ECRWRE_MASK | CAN_CTRL2_WRMFRZ_MASK);
#endif

    // Initialize all message buffers as inactive
    for (i = 0; i < data->max_num_mb; i++)
    {
        flexcan_reg_ptr->MB[i].CS = 0x0;
        flexcan_reg_ptr->MB[i].ID = 0x0;
        flexcan_reg_ptr->MB[i].WORD0 = 0x0;
        flexcan_reg_ptr->MB[i].WORD1 = 0x0;
        /* Set MB as a RX buffer  */
        flexcan_reg_ptr->MB[i].CS = CAN_CS_CODE(kFlexCanRX_Empty);
    }

    flexcan_reg_ptr->CTRL2 &= (~CAN_CTRL2_WRMFRZ_MASK);

    // Disable time sync feature
    flexcan_reg_ptr->CTRL1 &= ~CAN_CTRL1_TSYN_MASK;
    flexcan_reg_ptr->CTRL1 |= CAN_CTRL1_LBUF_MASK;
    
    // Enable RX FIFO if need
    if (data->is_rx_fifo_needed)
    {
        // Enable RX FIFO
        flexcan_reg_ptr->MCR |= CAN_MCR_RFEN_MASK;
        // Set the number of the RX FIFO filters needed
        flexcan_reg_ptr->CTRL2 &= (~CAN_CTRL2_RFFN_MASK);

        // Select RX FIFO ID element format A
        flexcan_reg_ptr->MCR &= (~CAN_MCR_IDAM_MASK);
        
        // RX FIFO global mask
        flexcan_reg_ptr->RXFGMASK = 0;
    }

    for (i = 0; i < data->num_rximr; i++)
    {
        // RX individual mask
        flexcan_reg_ptr->RXIMR[i] = 0;
    }

#if PSP_MQX_CPU_IS_VYBRID
   // Configuration to clear Internal RAM for ECC operation
    // CLear all bits but ECCDIS
    flexcan_reg_ptr->MECR &= (~CAN_MECR_ECRWRDIS_MASK);
    flexcan_reg_ptr->MECR = (CAN_MECR_ECCDIS_MASK);
    flexcan_reg_ptr->CTRL2 &= (~CAN_CTRL2_ECRWRE_MASK);
#endif

    // Disable self reception
    flexcan_reg_ptr->MCR |= CAN_MCR_SRXDIS_MASK;

    // Rx global mask
    flexcan_reg_ptr->RXMGMASK  = 0;

    // Rx reg 14 mask
    flexcan_reg_ptr->RX14MASK  = 0;

    // Rx reg 15 mask
    flexcan_reg_ptr->RX15MASK  = 0;

    // De-assert Freeze Mode
    flexcan_reg_ptr->MCR &= ~(CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);

    // Wait till exit of freeze mode
    while(flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK){}

    // Disable recovering from Bus off state
    flexcan_reg_ptr->CTRL1 |= CAN_CTRL1_BOFFREC_MASK;

    // Disable all MB interrupts
    flexcan_reg_ptr->IMASK1 = 0x0;
#if !((MQX_CPU == PSP_CPU_MK65F180M) || (MQX_CPU == PSP_CPU_MK60DN512Z))
    flexcan_reg_ptr->IMASK2 = 0x0;
#endif

    // Clear all interrupt flags
    flexcan_reg_ptr->IFLAG1 = 0xFFFFFFFF;
    
#if !((MQX_CPU == PSP_CPU_MK65F180M) || (MQX_CPU == PSP_CPU_MK60DN512Z))
    flexcan_reg_ptr->IFLAG2 = 0xFFFFFFFF;
#endif
    flexcan_hal_clear_err_interrupt_status(instance);
#if PSP_MQX_CPU_IS_VYBRID
    flexcan_hal_clear_err_int_flag(instance);
#endif

    return (kFlexCan_OK);
}

/*!
 * @brief Set FlexCAN time segments for setting up bit rate
 *
 * @param   instance    The FlexCAN instance number.
 * @param   time_seg    FlexCAN time segments need to be set for bit rate.
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_hal_set_time_segments(uint8_t instance, flexcan_time_segment_t *time_seg)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    flexcan_reg_ptr->MCR |= (CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);

    // Wait for entering the freeze mode
    while((flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK) == 0) {}

    flexcan_reg_ptr->CTRL1 &= ~(CAN_CTRL1_PROPSEG_MASK | CAN_CTRL1_PSEG2_MASK |
                                CAN_CTRL1_PSEG1_MASK | CAN_CTRL1_PRESDIV_MASK |
                                CAN_CTRL1_RJW_MASK);
    flexcan_reg_ptr->CTRL1 |= (CAN_CTRL1_PROPSEG(time_seg->propseg) |
                               CAN_CTRL1_PSEG2(time_seg->pseg2) |
                               CAN_CTRL1_PSEG1(time_seg->pseg1) |
                               CAN_CTRL1_PRESDIV(time_seg->pre_divider) |
                               CAN_CTRL1_RJW(time_seg->rjw));

    // De-assert Freeze Mode
    flexcan_reg_ptr->MCR &= ~(CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);

    // Wait till exit of freeze mode
    while(flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK){}

    return (kFlexCan_OK);
}

/*!
 * @brief Get FlexCAN time segments for calculating the bitrate.
 *
 * @param   instance    The FlexCAN instance number.
 * @param   time_seg    FlexCAN time segments read for bitrate.
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_hal_get_time_segments(uint8_t instance, flexcan_time_segment_t *time_seg)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    time_seg->pre_divider = (flexcan_reg_ptr->CTRL1 & CAN_CTRL1_PRESDIV_MASK) >>
                             CAN_CTRL1_PRESDIV_SHIFT;
    time_seg->propseg = (flexcan_reg_ptr->CTRL1 & CAN_CTRL1_PROPSEG_MASK) >>
                         CAN_CTRL1_PROPSEG_SHIFT;
    time_seg->pseg1 = (flexcan_reg_ptr->CTRL1 & CAN_CTRL1_PSEG1_MASK) >> CAN_CTRL1_PSEG1_SHIFT;
    time_seg->pseg2 = (flexcan_reg_ptr->CTRL1 & CAN_CTRL1_PSEG2_MASK) >> CAN_CTRL1_PSEG2_SHIFT;
    time_seg->rjw = (flexcan_reg_ptr->CTRL1 & CAN_CTRL1_RJW_MASK) >> CAN_CTRL1_RJW_SHIFT;

    return (kFlexCan_OK);
}

/*!
 * @brief Set FlexCAN message buffer CODE field as inactive for transmitting
 *
 * @param   instance     The FlexCAN instance number.
 * @param   data         The FlexCAN platform data.
 * @param   mb_idx       Index of the message buffer
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_hal_inactive_mb_tx(
    uint8_t instance,
    const flexcan_config_t *data,
    uint32_t mb_idx)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;
    uint32_t val1, val2 = 1;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    if (data->num_mb > data->max_num_mb)
        return (kFlexCan_INVALID_MAILBOX);

    if ( mb_idx >= (data->max_num_mb) )
    {
        return( kFlexCan_INVALID_MAILBOX );
    }

    // Check if RX FIFO is enabled
    if ((flexcan_reg_ptr->MCR) & CAN_MCR_RFEN_MASK)
    {
        /* Get the number of RX FIFO Filters */
        val1 = (flexcan_reg_ptr->CTRL2 & CAN_CTRL2_RFFN_MASK) >> CAN_CTRL2_RFFN_SHIFT;

        // Get the number if MBs occupied by RX FIFO and ID filter table
        // the Rx FIFO occupies the memory space originally reserved for MB0-5
        // Every number of RFFN means 8 number of RX FIFO filters
        // and every 4 number of RX FIFO filters occupied one MB
        val2 = 6 + (val1 + 1) * 8 / 4;

        if (mb_idx <= val2)
            return (kFlexCan_INVALID_MAILBOX);
    }

    flexcan_reg_ptr->MB[mb_idx].CS = CAN_CS_CODE(kFlexCanTX_Inactive);

    return (kFlexCan_OK);
}

/*!
 * @brief Set FlexCAN message buffer fields for transmitting
 *
 * @param   instance     The FlexCAN instance number.
 * @param   data         The FlexCAN platform data.
 * @param   mb_idx       Index of the message buffer
 * @param   cs           CODE/status values (TX)
 * @param   msg_id       ID of the message to transmit
 * @param   mb_data      Bytes of the FlexCAN message
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_hal_set_mb_tx(
    uint8_t instance,
    const flexcan_config_t *data,
    uint32_t mb_idx,
    flexcan_mb_code_status_tx_t *cs,
    uint32_t msg_id,
    uint8_t *mb_data)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;
    uint32_t i;
    uint32_t val1, val2 = 1;
    uint32_t mb_1st_idx = 0;
    uint32_t reg_val;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    if (data->num_mb > data->max_num_mb)
        return (kFlexCan_INVALID_MAILBOX);

    if ( mb_idx >= (data->max_num_mb) )
    {
        return( kFlexCan_INVALID_MAILBOX );
    }

    // Check if RX FIFO is enabled
    if ((flexcan_reg_ptr->MCR) & CAN_MCR_RFEN_MASK)
    {
        /* Get the number of RX FIFO Filters */
        val1 = (flexcan_reg_ptr->CTRL2 & CAN_CTRL2_RFFN_MASK) >> CAN_CTRL2_RFFN_SHIFT;

        // Get the number if MBs occupied by RX FIFO and ID filter table
        // the Rx FIFO occupies the memory space originally reserved for MB0-5
        // Every number of RFFN means 8 number of RX FIFO filters
        // and every 4 number of RX FIFO filters occupied one MB
        val2 = 6 + (val1 + 1) * 8 / 4;

        if (mb_idx <= val2)
            return (kFlexCan_INVALID_MAILBOX);

        if (cs->code == kFlexCanTX_Data)
        {
            mb_1st_idx = val2;
        }
    }
    else
    {
        if (cs->code == kFlexCanTX_Data)
        {
            mb_1st_idx = 0;
        }
    }

    // Set the ID according the format structure
    if (cs->msg_id_type == kFlexCanMbId_Ext)
    {
        // ID [28-0]
        flexcan_reg_ptr->MB[mb_idx].ID = (msg_id & (CAN_ID_STD_MASK | CAN_ID_EXT_MASK));

        // Copy user's buffer into mailbox data area
        flexcan_reg_ptr->MB[mb_idx].WORD0 = 0x0;
        flexcan_reg_ptr->MB[mb_idx].WORD1 = 0x0;

        for ( i=0 ; i < cs->data_length ; i++ )
        {
            if (i < 4)
                flexcan_reg_ptr->MB[mb_idx].WORD0 |= (*(mb_data + i)) << ((3 - i) * 8);
            else
                flexcan_reg_ptr->MB[mb_idx].WORD1 |= (*(mb_data + i)) << ((7 - i) * 8);
        }

        // Set IDE
        reg_val = CAN_CS_IDE_MASK;

        // Set the length of data in bytes
        reg_val |= CAN_CS_DLC(cs->data_length);

        // Set MB CODE
        // Reset the code
        if (cs->code != kFlexCanTX_NotUsed)
        {
            if (cs->code == kFlexCanTX_Remote)
            {
                // Set RTR bit
                reg_val |= CAN_CS_RTR_MASK;
                cs->code = kFlexCanTX_Data;
            }

            // Reset the code
            reg_val &= ~(CAN_CS_CODE_MASK);

            // Activating mailbox
            reg_val |= CAN_CS_CODE(cs->code);
            flexcan_reg_ptr->MB[mb_idx].CS = reg_val;

            // Workaround for ERR005641
            flexcan_reg_ptr->MB[mb_1st_idx].CS = CAN_CS_CODE(kFlexCanTX_Inactive);
            flexcan_reg_ptr->MB[mb_1st_idx].CS = CAN_CS_CODE(kFlexCanTX_Inactive);
        }
    }
    else if(cs->msg_id_type == kFlexCanMbId_Std)
    {
        // ID[28-18]
        flexcan_reg_ptr->MB[mb_idx].ID = CAN_ID_STD(msg_id);

        // Copy user's buffer into mailbox data area
        flexcan_reg_ptr->MB[mb_idx].WORD0 = 0x0;
        flexcan_reg_ptr->MB[mb_idx].WORD1 = 0x0;

        for ( i=0 ; i < cs->data_length ; i++ )
        {
            if (i < 4)
                flexcan_reg_ptr->MB[mb_idx].WORD0 |= (*(mb_data + i)) << ((3 - i) * 8);
            else
                flexcan_reg_ptr->MB[mb_idx].WORD1 |= (*(mb_data + i)) << ((7 - i) * 8);
        }

        // Set the length of data in bytes
        reg_val = (cs->data_length) << CAN_CS_DLC_SHIFT;

        // Set MB CODE
        if (cs->code != kFlexCanTX_NotUsed)
        {
            if (cs->code == kFlexCanTX_Remote)
            {
                // Set RTR bit
                reg_val |= CAN_CS_RTR_MASK;
                cs->code = kFlexCanTX_Data;
            }
            
            // Set the code
            reg_val |= CAN_CS_CODE(cs->code);
            flexcan_reg_ptr->MB[mb_idx].CS = reg_val;

            // Workaround for ERR005641
            flexcan_reg_ptr->MB[mb_1st_idx].CS = CAN_CS_CODE(kFlexCanTX_Inactive);
            flexcan_reg_ptr->MB[mb_1st_idx].CS = CAN_CS_CODE(kFlexCanTX_Inactive);
        }
    }
    else
    {
        return (kFlexCan_MESSAGE_FORMAT_UNKNOWN);
    }

    return (kFlexCan_OK);
}

/*!
 * @brief Set FlexCAN message buffer fields for receiving
 *
 * @param   instance     The FlexCAN instance number.
 * @param   data         The FlexCAN platform data.
 * @param   mb_idx       Index of the message buffer
 * @param   cs           CODE/status values (RX)
 * @param   msg_id       ID of the message to receive
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_hal_set_mb_rx(
    uint8_t instance,
    const flexcan_config_t *data,
    uint32_t mb_idx,
    flexcan_mb_code_status_rx_t *cs,
    uint32_t msg_id)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;
    uint32_t val1, val2 = 1;
    uint32_t reg_val;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    if (data->num_mb > data->max_num_mb)
    {
        return (kFlexCan_INVALID_MAILBOX);
    }

    if ( mb_idx >= (data->max_num_mb) )
    {
        return( kFlexCan_INVALID_MAILBOX );
    }

    // Check if RX FIFO is enabled
    if ((flexcan_reg_ptr->MCR) & CAN_MCR_RFEN_MASK)
    {
        // Get the number of RX FIFO Filters
        val1 = (flexcan_reg_ptr->CTRL2 & CAN_CTRL2_RFFN_MASK) >> CAN_CTRL2_RFFN_SHIFT;

        // Get the number if MBs occupied by RX FIFO and ID filter table
        // the Rx FIFO occupies the memory space originally reserved for MB0-5
        // Every number of RFFN means 8 number of RX FIFO filters
        // and every 4 number of RX FIFO filters occupied one MB
        val2 = 6 + (val1 + 1) * 8 / 4;

        if (mb_idx <= (val2 - 1))
        return (kFlexCan_INVALID_MAILBOX);
    }

    // Set the ID according the format structure
    if (cs->msg_id_type == kFlexCanMbId_Ext)
    {
        // Set IDE
        reg_val = CAN_CS_IDE_MASK;

        // Set the length of data in bytes
        reg_val |= CAN_CS_DLC(cs->data_length);

        // ID [28-0]
        flexcan_reg_ptr->MB[mb_idx].ID |= ((msg_id >> CAN_ID_EXT_SHIFT) &
                                           (CAN_ID_STD_MASK | CAN_ID_EXT_MASK));

        // Set MB CODE
        if (cs->code != kFlexCanRX_NotUsed)
        {
            reg_val |= CAN_CS_CODE(cs->code);
            flexcan_reg_ptr->MB[mb_idx].CS = reg_val;
        }
    }
    else if(cs->msg_id_type == kFlexCanMbId_Std)
    {
        // Set the length of data in bytes
        reg_val = (cs->data_length) << CAN_CS_DLC_SHIFT;

        // ID[28-18]
        flexcan_reg_ptr->MB[mb_idx].ID = CAN_ID_STD(msg_id);

        // Set MB CODE
        if (cs->code != kFlexCanRX_NotUsed)
        {
            reg_val |= CAN_CS_CODE(cs->code);
            flexcan_reg_ptr->MB[mb_idx].CS = reg_val;
        }

   }
   else
        return (kFlexCan_MESSAGE_FORMAT_UNKNOWN);

   return (kFlexCan_OK);
}

/*!
 * @brief Get FlexCAN message buffer fields
 *
 * @param   instance     The FlexCAN instance number.
 * @param   data         The FlexCAN platform data.
 * @param   mb_idx       Index of the message buffer
 * @param   mb           The fields of the message buffer
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_hal_get_mb(
    uint8_t instance,
    const flexcan_config_t *data,
    uint32_t mb_idx,
    flexcan_mb_t *mb)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;
    uint32_t i;
    uint32_t val1, val2 = 1;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    if (mb_idx >= (data->max_num_mb))
    {
        return (kFlexCan_INVALID_MAILBOX);
    }

    // Check if RX FIFO is enabled
    if ((flexcan_reg_ptr->MCR) & CAN_MCR_RFEN_MASK)
    {
        // Get the number of RX FIFO Filters
        val1 = (flexcan_reg_ptr->CTRL2 & CAN_CTRL2_RFFN_MASK) >> CAN_CTRL2_RFFN_SHIFT;
        // Get the number if MBs occupied by RX FIFO and ID filter table
        // the Rx FIFO occupies the memory space originally reserved for MB0-5
        // Every number of RFFN means 8 number of RX FIFO filters
        // and every 4 number of RX FIFO filters occupied one MB
        val2 = 6 + (val1 + 1) * 8 / 4;

        if (mb_idx <= (val2 - 1))
        return (kFlexCan_INVALID_MAILBOX);
    }

    // Get a MB field values
    mb->cs = flexcan_reg_ptr->MB[mb_idx].CS;
    if ((mb->cs) & CAN_CS_IDE_MASK)
    {
        mb->msg_id = flexcan_reg_ptr->MB[mb_idx].ID;
    }
    else
    {
        mb->msg_id = (flexcan_reg_ptr->MB[mb_idx].ID) >> CAN_ID_STD_SHIFT;
    }

    // Copy MB data erea into user's buffer
    for (i=0 ; i < kFlexCanMessageSize ; i++)
    {
        if (i < 4)
            mb->data[3 - i] = ((flexcan_reg_ptr->MB[mb_idx].WORD0) >> (i * 8)) & 0xFF;
        else
            mb->data[11 - i] = ((flexcan_reg_ptr->MB[mb_idx].WORD1) >> ((i - 4) * 8)) & 0xFF;
    }
    return (kFlexCan_OK);
}

/*!
 * @brief Lock FlexCAN RX message buffer
 *
 * @param   instance     The FlexCAN instance number.
 * @param   data         The FlexCAN platform data.
 * @param   mb_idx       Index of the message buffer
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_hal_lock_rx_mb(
    uint8_t instance,
    const flexcan_config_t *data,
    uint32_t mb_idx)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
    {
        return (kFlexCan_INVALID_ADDRESS);
    }

    if ( mb_idx >= (data->max_num_mb) )
    {
        return( kFlexCan_INVALID_MAILBOX );
    }

    // Lock the mailbox
    flexcan_reg_ptr->MB[mb_idx].CS;

    return( kFlexCan_OK );
}

/*!
 * @brief Unlock FlexCAN RX message buffer
 *
 * @param   instance     The FlexCAN instance number.
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_hal_unlock_rx_mb(uint8_t instance)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
    {
        return (kFlexCan_INVALID_ADDRESS);
    }

    // Unlock the mailbox
    flexcan_reg_ptr->TIMER;

    return( kFlexCan_OK );
}

/*!
 * @brief Enable Rx FIFO.
 *
 * @param   instance     The FlexCAN instance number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_enable_rx_fifo(uint8_t instance)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    // Set Freeze mode
    flexcan_reg_ptr->MCR |= CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK;

    // Wait for entering the freeze mode
    while((flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK) == 0){}

    flexcan_reg_ptr->MCR |= CAN_MCR_RFEN_MASK;

    // De-assert Freeze Mode
    flexcan_reg_ptr->MCR &= ~(CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);

    // Wait till exit of freeze mode
    while( flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK){}

    return (kFlexCan_OK);
}

/*!
 * @brief Disable Rx FIFO.
 *
 * @param   instance     The FlexCAN instance number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_disable_rx_fifo(uint8_t instance)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    // Set Freeze mode
    flexcan_reg_ptr->MCR |= CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK;

    // Wait for entering the freeze mode
    while((flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK) == 0){}

    flexcan_reg_ptr->MCR &= (~CAN_MCR_RFEN_MASK);

    // De-assert Freeze Mode
    flexcan_reg_ptr->MCR &= ~(CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);

    // Wait till exit of freeze mode
    while( flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK){}

    return (kFlexCan_OK);
}

/*!
 * @brief Set the number of Rx FIFO filters.
 *
 * @param   instance     The FlexCAN instance number.
 * @param   number       The number of Rx FIFO filters.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_set_rx_fifo_filters_number(uint8_t instance, uint32_t number)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address (instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    // Set Freeze mode
    flexcan_reg_ptr->MCR |= CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK;

    // Wait for entering the freeze mode
    while((flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK) == 0){}

    flexcan_reg_ptr->CTRL2 &= (~CAN_CTRL2_RFFN_MASK);
    flexcan_reg_ptr->CTRL2 |= CAN_CTRL2_RFFN(number);

    // De-assert Freeze Mode
    flexcan_reg_ptr->MCR &= ~(CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);

    // Wait till exit of freeze mode
    while( flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK){}

    return (kFlexCan_OK);
}

/*!
 * @brief Set the maximum number of Message Buffers.
 *
 * @param   instance     The FlexCAN instance number.
 * @param   data         The FlexCAN platform data.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_set_max_mb_number(
    uint8_t instance,
    const flexcan_config_t *data)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address (instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    // Set Freeze mode
    flexcan_reg_ptr->MCR |= CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK;

    // Wait for entering the freeze mode
    while((flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK) == 0){}

    flexcan_reg_ptr->MCR &= (~CAN_MCR_MAXMB_MASK);
    flexcan_reg_ptr->MCR |= CAN_MCR_MAXMB((data->max_num_mb - 1));

    // De-assert Freeze Mode
    flexcan_reg_ptr->MCR &= ~(CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);

    // Wait till exit of freeze mode
    while( flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK){}

    return (kFlexCan_OK);
}

/*!
 * @brief Set RX FIFO ID filter table elements.
 *
 * @param   instance                The FlexCAN instance number.
 * @param   data                    The FlexCAN platform data.
 * @param   id_format               The format of the Rx FIFO ID Filter Table Elements.
 * @param   id_filter_table         The ID filter table elements which contain if RTR bit,
 *                                  IDE bit need to be set, and RX message ID.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_set_id_filter_table_elements(
    uint8_t instance,
    const flexcan_config_t *data,
    flexcan_rx_fifo_id_element_format_t id_format,
    flexcan_id_table_t *id_filter_table)
{
   volatile CAN_MemMapPtr  flexcan_reg_ptr;
   uint32_t i, j;
   uint32_t val1, val2, val;

   flexcan_reg_ptr = _bsp_get_flexcan_base_address (instance);
   if (NULL == flexcan_reg_ptr)
      return (kFlexCan_INVALID_ADDRESS);

   switch(id_format)
    {
    case (kFlexCanRxFifoIdElementFormat_A):
        // One full ID (standard and extended) per ID Filter Table element.
        flexcan_reg_ptr->MCR &= (~CAN_MCR_IDAM_MASK);
        flexcan_reg_ptr->MCR |= kFlexCanRxFifoIdElementFormat_A << CAN_MCR_IDAM_SHIFT;
        if (id_filter_table->is_remote_mb)
            val = (1U << 31);
        if (id_filter_table->is_extended_mb)
        {
            val |= 1 << 30;
            j = 0;
            for (i = 0; i < (data->num_id_filters + 1) * 8; i += 4)
            {
                flexcan_reg_ptr->MB[6 + i - j * 3].CS =
                val + ((*(id_filter_table->id_filter + i)) <<
                       FLEXCAN_RX_FIFO_ID_FILTER_FORMATA_EXT_SHIFT &
                         FLEXCAN_RX_FIFO_ID_FILTER_FORMATA_EXT_MASK);
                flexcan_reg_ptr->MB[6 + i - j * 3].ID =
                val + ((*(id_filter_table->id_filter + i + 1)) <<
                       FLEXCAN_RX_FIFO_ID_FILTER_FORMATA_EXT_SHIFT &
                         FLEXCAN_RX_FIFO_ID_FILTER_FORMATA_EXT_MASK);
                flexcan_reg_ptr->MB[6 + i - j * 3].WORD0 =
                val + ((*(id_filter_table->id_filter + i + 2)) <<
                       FLEXCAN_RX_FIFO_ID_FILTER_FORMATA_EXT_SHIFT &
                         FLEXCAN_RX_FIFO_ID_FILTER_FORMATA_EXT_MASK);
                flexcan_reg_ptr->MB[6 + i - j * 3].WORD1 =
                val + ((*(id_filter_table->id_filter + i + 3)) <<
                       FLEXCAN_RX_FIFO_ID_FILTER_FORMATA_EXT_SHIFT &
                         FLEXCAN_RX_FIFO_ID_FILTER_FORMATA_EXT_MASK);
                j++;
            }
        }
        else
        {
            j = 0;
            for (i = 0; i < (data->num_id_filters + 1) * 8; i += 4)
            {
                flexcan_reg_ptr->MB[6 + i - j * 3].CS =
                val + ((*(id_filter_table->id_filter + i)) <<
                       FLEXCAN_RX_FIFO_ID_FILTER_FORMATA_STD_SHIFT &
                         FLEXCAN_RX_FIFO_ID_FILTER_FORMATA_STD_MASK);
                flexcan_reg_ptr->MB[6 + i - j * 3].ID =
                val + ((*(id_filter_table->id_filter + i + 1)) <<
                       FLEXCAN_RX_FIFO_ID_FILTER_FORMATA_STD_SHIFT &
                         FLEXCAN_RX_FIFO_ID_FILTER_FORMATA_STD_MASK);
                flexcan_reg_ptr->MB[6 + i - j * 3].WORD0 =
                val + ((*(id_filter_table->id_filter + i + 2)) <<
                       FLEXCAN_RX_FIFO_ID_FILTER_FORMATA_STD_SHIFT &
                         FLEXCAN_RX_FIFO_ID_FILTER_FORMATA_STD_MASK);
                flexcan_reg_ptr->MB[6 + i - j * 3].WORD1 =
                val + ((*(id_filter_table->id_filter + i + 3)) <<
                       FLEXCAN_RX_FIFO_ID_FILTER_FORMATA_STD_SHIFT &
                         FLEXCAN_RX_FIFO_ID_FILTER_FORMATA_STD_MASK);
                j++;
            }
        }
        break;
    case (kFlexCanRxFifoIdElementFormat_B):
        // Two full standard IDs or two partial 14-bit (standard and extended) IDs
        // per ID Filter Table element.
        flexcan_reg_ptr->MCR &= (~CAN_MCR_IDAM_MASK);
        flexcan_reg_ptr->MCR |= kFlexCanRxFifoIdElementFormat_B << CAN_MCR_IDAM_SHIFT;
        if (id_filter_table->is_remote_mb)
        {
            val1 = (1U << 31);
            val2 = 1 << 15;
        }
        if (id_filter_table->is_extended_mb)
        {
            val1 |= 1 << 30;
            val2 |= 1 << 14;
            j = 0;
            for (i = 0; i < (data->num_id_filters + 1) * 8; i += 8)
            {
                flexcan_reg_ptr->MB[6 + i - j * 3].CS =
                val1 + ((*(id_filter_table->id_filter + i)) &
                        FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_EXT_MASK <<
                          FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_EXT_SHIFT1);
                flexcan_reg_ptr->MB[6 + i - j * 3].CS |=
                val2 + ((*(id_filter_table->id_filter + i + 1)) &
                        FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_EXT_MASK <<
                          FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_EXT_SHIFT2);

                flexcan_reg_ptr->MB[6 + i - j * 3].ID =
                val1 + ((*(id_filter_table->id_filter + i + 2)) &
                        FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_EXT_MASK <<
                          FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_EXT_SHIFT1);
                flexcan_reg_ptr->MB[6 + i - j * 3].ID |=
                val2 + ((*(id_filter_table->id_filter + i + 3)) &
                        FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_EXT_MASK <<
                          FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_EXT_SHIFT2);

                flexcan_reg_ptr->MB[6 + i - j * 3].WORD0 =
                val1 + ((*(id_filter_table->id_filter + i + 4)) &
                        FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_EXT_MASK <<
                          FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_EXT_SHIFT1);
                flexcan_reg_ptr->MB[6 + i - j * 3].WORD0 |=
                val2 + ((*(id_filter_table->id_filter + i + 5)) &
                        FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_EXT_MASK <<
                          FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_EXT_SHIFT2);

                flexcan_reg_ptr->MB[6 + i - j * 3].WORD1 =
                val1 + ((*(id_filter_table->id_filter + i + 6)) &
                        FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_EXT_MASK <<
                          FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_EXT_SHIFT1);
                flexcan_reg_ptr->MB[6 + i - j * 3].WORD1 |=
                val2 + ((*(id_filter_table->id_filter + i + 7)) &
                        FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_EXT_MASK <<
                          FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_EXT_SHIFT2);
                j++;
            }
        }
        else
        {
            j = 0;
            for (i = 0; i < (data->num_id_filters + 1) * 8; i += 8)
            {
                flexcan_reg_ptr->MB[6 + i - j * 3].CS =
                val1 + (((*(id_filter_table->id_filter + i)) &
                        FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_STD_MASK) <<
                          FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_STD_SHIFT1);
                flexcan_reg_ptr->MB[6 + i - j * 3].CS |=
                val2 + ((*(id_filter_table->id_filter + i + 1)) &
                        FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_STD_MASK <<
                          FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_STD_SHIFT2);

                flexcan_reg_ptr->MB[6 + i - j * 3].ID =
                val1 + (((*(id_filter_table->id_filter + i + 2)) &
                        FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_STD_MASK) <<
                          FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_STD_SHIFT1);
                flexcan_reg_ptr->MB[6 + i - j * 3].ID |=
                val2 + ((*(id_filter_table->id_filter + i + 3)) &
                        FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_STD_MASK <<
                          FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_STD_SHIFT2);

                flexcan_reg_ptr->MB[6 + i - j * 3].WORD0 =
                val1 + (((*(id_filter_table->id_filter + i + 4)) &
                        FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_STD_MASK) <<
                          FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_STD_SHIFT1);
                flexcan_reg_ptr->MB[6 + i - j * 3].WORD0 |=
                val2 + ((*(id_filter_table->id_filter + i + 5)) &
                        FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_STD_MASK <<
                          FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_STD_SHIFT2);

                flexcan_reg_ptr->MB[6 + i - j * 3].WORD1 =
                val1 + (((*(id_filter_table->id_filter + i + 6)) &
                        FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_STD_MASK) <<
                          FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_STD_SHIFT1);
                flexcan_reg_ptr->MB[6 + i - j * 3].WORD1 |=
                val2 + ((*(id_filter_table->id_filter + i + 7)) &
                        FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_STD_MASK <<
                          FLEXCAN_RX_FIFO_ID_FILTER_FORMATB_STD_SHIFT2);
                j++;
            }
        }
      break;
    case (kFlexCanRxFifoIdElementFormat_C):
        // Four partial 8-bit Standard IDs per ID Filter Table element.
        flexcan_reg_ptr->MCR &= (~CAN_MCR_IDAM_MASK);
        flexcan_reg_ptr->MCR |= kFlexCanRxFifoIdElementFormat_C << CAN_MCR_IDAM_SHIFT;
        j = 0;
        for (i = 0; i < (data->num_id_filters + 1) * 8; i += 16)
        {
            flexcan_reg_ptr->MB[6 + i - j * 3].CS =
            ((*(id_filter_table->id_filter + i)) & FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_MASK <<
             FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_SHIFT1);
            flexcan_reg_ptr->MB[6 + i - j * 3].CS |=
            ((*(id_filter_table->id_filter + i + 1)) & FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_MASK <<
             FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_SHIFT2);
            flexcan_reg_ptr->MB[6 + i - j * 3].CS |=
            ((*(id_filter_table->id_filter + i + 2)) & FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_MASK <<
             FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_SHIFT3);
            flexcan_reg_ptr->MB[6 + i - j * 3].CS |=
            ((*(id_filter_table->id_filter + i + 3)) & FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_MASK <<
             FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_SHIFT4);

            flexcan_reg_ptr->MB[6 + i - j * 3].ID =
            ((*(id_filter_table->id_filter + i + 4)) & FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_MASK <<
             FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_SHIFT1);
            flexcan_reg_ptr->MB[6 + i - j * 3].ID |=
            ((*(id_filter_table->id_filter + i + 5)) & FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_MASK <<
             FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_SHIFT2);
            flexcan_reg_ptr->MB[6 + i - j * 3].ID |=
            ((*(id_filter_table->id_filter + i + 6)) & FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_MASK <<
             FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_SHIFT3);
            flexcan_reg_ptr->MB[6 + i - j * 3].ID |=
            ((*(id_filter_table->id_filter + i + 7)) & FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_MASK <<
             FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_SHIFT4);

            flexcan_reg_ptr->MB[6 + i - j * 3].WORD0 =
            ((*(id_filter_table->id_filter + i + 8)) & FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_MASK <<
             FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_SHIFT1);
            flexcan_reg_ptr->MB[6 + i - j * 3].WORD0 |=
            ((*(id_filter_table->id_filter + i + 9)) & FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_MASK <<
             FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_SHIFT2);
            flexcan_reg_ptr->MB[6 + i - j * 3].WORD0 |=
            ((*(id_filter_table->id_filter + i + 10)) & FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_MASK <<
             FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_SHIFT3);
            flexcan_reg_ptr->MB[6 + i - j * 3].WORD0 |=
            ((*(id_filter_table->id_filter + i + 11)) & FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_MASK <<
             FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_SHIFT4);

            flexcan_reg_ptr->MB[6 + i - j * 3].WORD1 =
            ((*(id_filter_table->id_filter + i + 12)) & FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_MASK <<
             FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_SHIFT1);
            flexcan_reg_ptr->MB[6 + i - j * 3].WORD1 |=
            ((*(id_filter_table->id_filter + i + 13)) & FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_MASK <<
             FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_SHIFT2);
            flexcan_reg_ptr->MB[6 + i - j * 3].WORD1 |=
            ((*(id_filter_table->id_filter + i + 14)) & FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_MASK <<
             FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_SHIFT3);
            flexcan_reg_ptr->MB[6 + i - j * 3].WORD1 |=
            ((*(id_filter_table->id_filter + i + 15)) & FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_MASK <<
             FLEXCAN_RX_FIFO_ID_FILTER_FORMATC_SHIFT4);

            j++;
        }
        break;
    case (kFlexCanRxFifoIdElementFormat_D):
        // All frames rejected.
        flexcan_reg_ptr->MCR &= (~CAN_MCR_IDAM_MASK);
        flexcan_reg_ptr->MCR |= kFlexCanRxFifoIdElementFormat_D << CAN_MCR_IDAM_SHIFT;
        break;
    default:
        return kFlexCan_INVALID_MODE;
   }

   return (kFlexCan_OK);
}

/*!
 * @brief Set FlexCAN RX FIFO fields
 *
 * @param   instance                The FlexCAN instance number.
 * @param   data                    The FlexCAN platform data.
 * @param   config_val              RX FIFO fields configure value.
 * @param   id_format               The format of the Rx FIFO ID Filter Table Elements.
 * @param   id_filter_table         The ID filter table elements which contain RTR bit, IDE bit,
 *                                  and RX message ID.
 * @param   msg_id                  ID of the message to receive.
 * @param   fifo_data               Bytes of the FlexCAN message.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_set_rx_fifo(
    uint8_t instance,
    const flexcan_config_t *data,
    flexcan_rx_fifo_id_element_format_t id_format,
    flexcan_id_table_t *id_filter_table)
{
    if (!data->is_rx_fifo_needed)
        return (kFlexCan_UNDEF_ERROR);

    if (data->num_mb > (data->max_num_mb))
        return (kFlexCan_INVALID_MAILBOX);

    return flexcan_hal_set_id_filter_table_elements(instance, data, id_format, id_filter_table);
}

/*!
 * @brief Enable the FlexCAN Message Buffer interrupt
 *
 * @param   instance     The FlexCAN instance number.
 * @param   data         The FlexCAN platform data.
 * @param   mb_idx       Index of the message buffer.
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_hal_enable_mb_interrupt(
    uint8_t instance,
    const flexcan_config_t *data,
    uint32_t mb_idx)
{
    volatile CAN_MemMapPtr         flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)  
        return (kFlexCan_INVALID_ADDRESS);
   
    if ( mb_idx >= (data->max_num_mb) ) 
        return (kFlexCan_INVALID_MAILBOX);
      
    // IMASK, enable the corresponding message buffer Interrupt
    if (mb_idx < 32)
        (flexcan_reg_ptr->IMASK1) |= (0x1 << mb_idx);
#if !((MQX_CPU == PSP_CPU_MK65F180M) || (MQX_CPU == PSP_CPU_MK60DN512Z))
    else if (mb_idx < 64)
        (flexcan_reg_ptr->IMASK2) |= (0x1 << (mb_idx - 32));
#endif
    return( kFlexCan_OK );
}

/*!
 * @brief Disable the FlexCAN Message Buffer interrupt
 *
 * @param   instance     The FlexCAN instance number.
 * @param   data         The FlexCAN platform data.
 * @param   mb_idx       Index of the message buffer.
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_hal_disable_mb_interrupt(
    uint8_t instance,
    const flexcan_config_t *data,
    uint32_t mb_idx)
{
    volatile CAN_MemMapPtr         flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address (instance);
    if (NULL == flexcan_reg_ptr)  
        return (kFlexCan_INVALID_ADDRESS);

    if (mb_idx >= (data->max_num_mb)) 
        return (kFlexCan_INVALID_MAILBOX);

    /* IMASK, disable the corresponding message buffer Interrupt */
    if (mb_idx < 32)
        flexcan_reg_ptr->IMASK1  &= ~(0x1 << mb_idx);
#if !((MQX_CPU == PSP_CPU_MK65F180M) || (MQX_CPU == PSP_CPU_MK60DN512Z))
    else if (mb_idx < 64)
        flexcan_reg_ptr->IMASK2  &= ~(0x1 << (mb_idx - 32));
#endif
    return (kFlexCan_OK);
}

/*!
 * @brief Enable all error interrupts of the FlexCAN module
 *
 * It enables the related HW module interrupt.
 *
 * @param   instance     The FlexCAN instance number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_enable_error_interrupt(uint8_t instance)
{
    volatile CAN_MemMapPtr         flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address (instance);
    if (NULL == flexcan_reg_ptr)  
        return (kFlexCan_INVALID_ADDRESS);
   
    // Unmask BOFFMSK, ERRMSK, and WAKMSK
    flexcan_reg_ptr->CTRL1 |= (CAN_CTRL1_BOFFMSK_MASK | CAN_CTRL1_ERRMSK_MASK |
                               CAN_CTRL1_TWRNMSK_MASK | CAN_CTRL1_RWRNMSK_MASK);
#if (MQX_CPU == PSP_CPU_MK70F120M) || (MQX_CPU == PSP_CPU_MK60DN512Z)
    flexcan_reg_ptr->MCR |= CAN_MCR_WAKMSK_MASK;
#endif

    return (kFlexCan_OK);
}

/*!
 * @brief Disable all error interrupts of the FlexCAN module
 *
 * It disables the related HW module interrupt.
 *
 * @param   instance     The FlexCAN instance number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_disable_error_interrupt(uint8_t instance)
{
    volatile CAN_MemMapPtr         flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address (instance);
    if (NULL == flexcan_reg_ptr)  
        return (kFlexCan_INVALID_ADDRESS);

    // Mask BOFFMSK, ERRMSK, and WAKMSK
    flexcan_reg_ptr->CTRL1 &= ~(CAN_CTRL1_BOFFMSK_MASK | CAN_CTRL1_ERRMSK_MASK);
#if (MQX_CPU == PSP_CPU_MK70F120M) || (MQX_CPU == PSP_CPU_MK60DN512Z)
    flexcan_reg_ptr->MCR &= ~CAN_MCR_WAKMSK_MASK;
#endif

    return (kFlexCan_OK);
}

/*!
 * @brief Enable Tx warning and Rx warning interrupts of the FlexCAN module
 *
 * It enables Tx warning interrupt and Rx warning interrupt.
 *
 * @param   instance     The FlexCAN instance number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_enable_tx_rx_warning_interrupt(uint8_t instance)
{
    volatile CAN_MemMapPtr         flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address (instance);
    if (NULL == flexcan_reg_ptr)  
        return (kFlexCan_INVALID_ADDRESS);
   
    // Unmask TWRN_MSK, RWRN_MSK
    flexcan_reg_ptr->CTRL1 |= (CAN_CTRL1_TWRNMSK_MASK | CAN_CTRL1_RWRNMSK_MASK);

    return (kFlexCan_OK);
}

/*!
 * @brief Disable Tx warning and Rx warning interrupts of the FlexCAN module
 *
 * It disables Tx warning interrupt and Rx warning interrupt.
 *
 * @param   instance     The FlexCAN instance number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_disable_tx_rx_warning_interrupt(uint8_t instance)
{
    volatile CAN_MemMapPtr         flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address (instance);
    if (NULL == flexcan_reg_ptr)  
        return (kFlexCan_INVALID_ADDRESS);

    // Mask TWRN_MSK, RWRN_MSK
    flexcan_reg_ptr->CTRL1 &= ~(CAN_CTRL1_TWRNMSK_MASK | CAN_CTRL1_RWRNMSK_MASK);

    return (kFlexCan_OK);
}

/*!
 * @brief Un-freeze the FlexCAN module
 *
 * @param   instance     The FlexCAN instance number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_exit_freeze_mode(uint8_t instance)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    flexcan_reg_ptr->MCR &= ~(CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);

    // Wait till exit of freeze mode
    while( flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK){}

    return (kFlexCan_OK);
}

/*!
 * @brief Freeze the FlexCAN module
 *
 * @param   instance     The FlexCAN instance number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_enter_freeze_mode(uint8_t instance)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    flexcan_reg_ptr->MCR |= (CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);

    // Wait for entering the freeze mode
    while((flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK) == 0){}

    return (kFlexCan_OK);
}

/*!
 * @brief Get freeze ack.
 *
 * @param   instance     The FlexCAN instance number.
 * @return  freeze ack state (1-freeze mode, 0-not in freeze mode, otherwise failed).
 */
uint32_t flexcan_hal_get_freeze_ack(uint8_t instance)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    return (flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK) >> CAN_MCR_FRZACK_SHIFT;
}

/*!
 * @brief Get the individual FlexCAN MB interrupt flag
 *
 * @param   instance     The FlexCAN instance number.
 * @param   mb_idx      Index of the message buffer.
 * @return  the individual MB interrupt flag (0 and 1 are the flag value; otherwise failed).
 */
uint32_t flexcan_hal_get_mb_int_flag(
    uint8_t instance,
    uint32_t mb_idx)
{
    uint32_t val = 0;
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    if (mb_idx < 32) {
        val = flexcan_reg_ptr->IFLAG1;
        val &= (0x1 << mb_idx);
    } 
#if !((MQX_CPU == PSP_CPU_MK65F180M) || (MQX_CPU == PSP_CPU_MK60DN512Z))
    else if (mb_idx < 64) {
        val = flexcan_reg_ptr->IFLAG2;
        val &= (0x1 << (mb_idx - 32));
    }
#endif
    
    if (val == 0x0)
        return 0;
    else
        return 1;
}

/*!
 * @brief Get all FlexCAN MB interrupt flags.
 *
 * @param   instance     The FlexCAN instance number.
 * @return  all MB interrupt flags
 */
uint32_t flexcan_hal_get_all_mb_int_flags(uint8_t instance)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    return flexcan_reg_ptr->IFLAG1;
}

/*!
 * @brief Clear the interrupt flag of the message buffers
 *
 * @param   instance     The FlexCAN instance number.
 * @param   reg_val     The value to be writen to the interrupt flag1 register.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_clear_mb_int_flag(
    uint8_t instance,
    uint32_t reg_val)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    flexcan_reg_ptr->IFLAG1 = reg_val;

    return (kFlexCan_OK);
}

/*!
 * @brief Get transmit error counter and receive error counter
 *
 * @param   instance     The FlexCAN instance number.
 * @param   err_cnt      Transmit error counter and receive error counter.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_get_err_counter(
    uint8_t instance,
    flexcan_berr_counter_t *err_cnt)
{
    uint32_t val;
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    val = flexcan_reg_ptr->ECR;
    err_cnt->rxerr = (val & CAN_ECR_RXERRCNT_MASK) >> CAN_ECR_RXERRCNT_SHIFT;
    err_cnt->txerr = (val & CAN_ECR_TXERRCNT_MASK) >> CAN_ECR_TXERRCNT_SHIFT;

    return (kFlexCan_OK);
}

/*!
 * @brief Get error and status
 *
 * @param   instance     The FlexCAN instance number.
 * @return  The current error and status.
 */
uint32_t flexcan_hal_get_err_status(uint8_t instance)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    return (flexcan_reg_ptr->ESR1);
}

/*!
 * @brief Clear all other interrupts in ERRSTAT register (Error, Busoff, Wakeup
 *
 * @param   instance     The FlexCAN instance number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_clear_err_interrupt_status(uint8_t instance)
{
    uint32_t val;
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    val = flexcan_reg_ptr->ESR1;
#if (MQX_CPU == PSP_CPU_MK70F120M) || (MQX_CPU == PSP_CPU_MK60DN512Z)
    if(val & (CAN_ESR1_WAKINT_MASK | CAN_ESR1_ERRINT_MASK | CAN_ESR1_BOFFINT_MASK))
    {
        flexcan_reg_ptr->ESR1 = (val & (CAN_ESR1_WAKINT_MASK | CAN_ESR1_ERRINT_MASK |
                                        CAN_ESR1_BOFFINT_MASK));
    }
#else
    if(val & (CAN_ESR1_ERRINT_MASK | CAN_ESR1_BOFFINT_MASK))
    {
        flexcan_reg_ptr->ESR1 = (val & (CAN_ESR1_ERRINT_MASK | CAN_ESR1_BOFFINT_MASK));
    }
#endif

    return (kFlexCan_OK);
}

/*!
 * @brief Get FlexCAN RX FIFO data
 *
 * @param   instance     The FlexCAN instance number.
 * @param   rx_fifo      The FlexCAN receive FIFO data.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_read_fifo(
    uint8_t instance,
    flexcan_mb_t *rx_fifo)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;
    uint32_t i;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    rx_fifo->cs = flexcan_reg_ptr->MB[0].CS;
    if ((rx_fifo->cs) & CAN_CS_IDE_MASK)
    {
        rx_fifo->msg_id = flexcan_reg_ptr->MB[0].ID;
    }
    else
    {
        rx_fifo->msg_id = (flexcan_reg_ptr->MB[0].ID) >> CAN_ID_STD_SHIFT;
    }

    // Copy MB data erea into user's buffer
    for ( i=0 ; i < kFlexCanMessageSize ; i++ )
    {
        if (i < 4)
            rx_fifo->data[3 - i] = ((flexcan_reg_ptr->MB[0].WORD0) >> (i * 8)) & 0xFF;
        else
            rx_fifo->data[11 - i] = ((flexcan_reg_ptr->MB[0].WORD1) >> ((i - 4) * 8)) & 0xFF;
    }

    return (kFlexCan_OK);
}

/*!
 * @brief Set RX masking type
 *
 * @param   instance     The FlexCAN instance number.
 * @param   type         The FlexCAN RX mask type.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_set_mask_type(uint8_t instance, flexcan_rx_mask_type_t type)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    // Set Freeze mode
    flexcan_reg_ptr->MCR |= CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK;

    // Wait for entering the freeze mode
    while((flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK) == 0){}

    if (type == kFlexCanRxMask_Global)
        // Enable Global RX masking
        flexcan_reg_ptr->MCR &= (~CAN_MCR_IRMQ_MASK);
    else
        // Enable Individual Rx Masking and Queue
        flexcan_reg_ptr->MCR |= CAN_MCR_IRMQ_MASK;

    // De-assert Freeze Mode
    flexcan_reg_ptr->MCR &= ~(CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);

    // Wait till exit of freeze mode
    while( flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK){}

    return (kFlexCan_OK);
}

/*!
 * @brief Set FlexCAN RX FIFO global standard mask.
 *
 * @param   instance     The FlexCAN instance number.
 * @param   std_mask     Standard mask.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_set_rx_fifo_global_std_mask(
    uint8_t instance,
    uint32_t std_mask)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address (instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    // Set Freeze mode
    flexcan_reg_ptr->MCR |= CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK;

    // Wait for entering the freeze mode
    while((flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK) == 0){}

    // 11 bit standard mask
    flexcan_reg_ptr->RXFGMASK = CAN_ID_STD(std_mask);

    // De-assert Freeze Mode
    flexcan_reg_ptr->MCR &= ~(CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);

    // Wait till exit of freeze mode
    while( flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK){}

    return (kFlexCan_OK);
}

/*!
 * @brief Set FlexCAN RX FIFO global extended mask.
 *
 * @param   instance     The FlexCAN instance number.
 * @param   ext_mask     Extended mask.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_set_rx_fifo_global_ext_mask(
    uint8_t instance,
    uint32_t ext_mask)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address (instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    // Set Freeze mode
    flexcan_reg_ptr->MCR |= CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK;

    // Wait for entering the freeze mode
    while((flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK) == 0){}

    // MID[28-0]
    flexcan_reg_ptr->RXFGMASK = CAN_ID_EXT(ext_mask);

    // De-assert Freeze Mode
    flexcan_reg_ptr->MCR &= ~(CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);

    // Wait till exit of freeze mode
    while( flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK){}

    return (kFlexCan_OK);
}

/*!
 * @brief Set FlexCAN RX individual standard mask for ID filtering in the Rx MBs and the Rx FIFO.
 *
 * @param   instance     The FlexCAN instance number.
 * @param   mb_idx       Index of the message buffer.
 * @param   std_mask     Individual standard mask.
 * @return  0 if successful; non-zero failed.
*/
uint32_t flexcan_hal_set_rx_individual_std_mask(
    uint8_t instance,
    uint32_t mb_idx,
    uint32_t std_mask)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address (instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    // Set Freeze mode
    flexcan_reg_ptr->MCR |= CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK;

    // Wait for entering the freeze mode
    while((flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK) == 0){}

    // 11 bit standard mask
    flexcan_reg_ptr->RXIMR[mb_idx] = CAN_ID_STD(std_mask);

    // De-assert Freeze Mode
    flexcan_reg_ptr->MCR &= ~(CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);

    // Wait till exit of freeze mode
    while( flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK){}

    return (kFlexCan_OK);
}

/*!
 * @brief Set FlexCAN RX individual extended mask for ID filtering in the Rx MBs and the Rx FIFO.
 *
 * @param   instance     The FlexCAN instance number.
 * @param   mb_idx       Index of the message buffer.
 * @param   ext_mask     Individual extended mask.
 * @return  0 if successful; non-zero failed.
*/
uint32_t flexcan_hal_set_rx_individual_ext_mask(
    uint8_t instance,
    uint32_t mb_idx,
    uint32_t ext_mask)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address (instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    // Set Freeze mode
    flexcan_reg_ptr->MCR |= CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK;

    // Wait for entering the freeze mode
    while((flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK) == 0){}

    // MID[28-0]
    flexcan_reg_ptr->RXIMR[mb_idx] = CAN_ID_EXT(ext_mask);

    // De-assert Freeze Mode
    flexcan_reg_ptr->MCR &= ~(CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);

    // Wait till exit of freeze mode
    while( flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK){}

    return (kFlexCan_OK);
}


/*!
 * @brief Get the FlexCAN ID acceptance filter hit indicator on Rx FIFO.
 *
 * @param   instance     The FlexCAN instance number.
 * @return  RX FIFO information.
 */
uint32_t  flexcan_hal_get_rx_fifo_id_acceptance_filter(uint8_t instance)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address (instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    return ((flexcan_reg_ptr->RXFIR) & CAN_RXFIR_IDHIT_MASK);
}

/*!
 * @brief Set FlexCAN RX MB global standard mask.
 *
 * @param   instance     The FlexCAN instance number.
 * @param   std_mask     Standard mask.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_set_rx_mb_global_std_mask(
    uint8_t instance,
    uint32_t std_mask)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address (instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    // Set Freeze mode
    flexcan_reg_ptr->MCR |= CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK;

    // Wait for entering the freeze mode
    while((flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK) == 0){}

    // 11 bit standard mask
    flexcan_reg_ptr->RXMGMASK = CAN_ID_STD(std_mask);

    // De-assert Freeze Mode
    flexcan_reg_ptr->MCR &= ~(CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);

    // Wait till exit of freeze mode
    while( flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK){}

    return (kFlexCan_OK);
}

/*!
 * @brief Set FlexCAN RX MB BUF14 standard mask.
 *
 * @param   instance     The FlexCAN instance number.
 * @param   std_mask     Standard mask.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_set_rx_mb_buf14_std_mask(
    uint8_t instance,
    uint32_t std_mask)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address (instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    // Set Freeze mode
    flexcan_reg_ptr->MCR |= CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK;

    // Wait for entering the freeze mode
    while((flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK) == 0){}

    // 11 bit standard mask
    flexcan_reg_ptr->RX14MASK = CAN_ID_STD(std_mask);

    // De-assert Freeze Mode
    flexcan_reg_ptr->MCR &= ~(CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);

    // Wait till exit of freeze mode
    while( flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK){}

    return (kFlexCan_OK);
}

/*!
 * @brief Set FlexCAN RX MB BUF15 standard mask.
 *
 * @param   instance     The FlexCAN instance number.
 * @param   std_mask     Standard mask.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_set_rx_mb_buf15_std_mask(
    uint8_t instance,
    uint32_t std_mask)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    // Set Freeze mode
    flexcan_reg_ptr->MCR |= CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK;

    // Wait for entering the freeze mode
    while((flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK) == 0){}

    // 11 bit standard mask
    flexcan_reg_ptr->RX15MASK = CAN_ID_STD(std_mask);

    // De-assert Freeze Mode
    flexcan_reg_ptr->MCR &= ~(CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);

    // Wait till exit of freeze mode
    while( flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK){}

    return (kFlexCan_OK);
}

/*!
 * @brief Set FlexCAN RX MB global extended mask.
 *
 * @param   instance     The FlexCAN instance number.
 * @param   ext_mask     Extended mask.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_set_rx_mb_global_ext_mask(
    uint8_t instance,
    uint32_t ext_mask)
{
   volatile CAN_MemMapPtr  flexcan_reg_ptr;

   flexcan_reg_ptr = _bsp_get_flexcan_base_address (instance);
   if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

   // Set Freeze mode
   flexcan_reg_ptr->MCR |= CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK;

   // Wait for entering the freeze mode
   while((flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK) == 0){}

   // MID[28-0]
   flexcan_reg_ptr->RXMGMASK = CAN_ID_EXT(ext_mask);

   // De-assert Freeze Mode
   flexcan_reg_ptr->MCR &= ~(CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);

   // Wait till exit of freeze mode
   while( flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK){}

   return (kFlexCan_OK);
}

/*!
 * @brief Set FlexCAN RX MB BUF14 extended mask.
 *
 * @param   instance     The FlexCAN instance number.
 * @param   ext_mask     Extended mask.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_set_rx_mb_buf14_ext_mask(
    uint8_t instance,
    uint32_t ext_mask)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address (instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    // Set Freeze mode
    flexcan_reg_ptr->MCR |= CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK;

    // Wait for entering the freeze mode
    while((flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK) == 0){}

    // MID[28-0]
    flexcan_reg_ptr->RX14MASK = CAN_ID_EXT(ext_mask);

    // De-assert Freeze Mode
    flexcan_reg_ptr->MCR &= ~(CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);

    // Wait till exit of freeze mode
    while( flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK){}

    return (kFlexCan_OK);
}

/*!
 * @brief Set FlexCAN RX MB BUF15 extended mask.
 *
 * @param   instance     The FlexCAN instance number.
 * @param   ext_mask     Extended mask.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_set_rx_mb_buf15_ext_mask(
    uint8_t instance,
    uint32_t ext_mask)
{
   volatile CAN_MemMapPtr  flexcan_reg_ptr;

   flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
   if (NULL == flexcan_reg_ptr)
      return (kFlexCan_INVALID_ADDRESS);

   // Set Freeze mode
   flexcan_reg_ptr->MCR |= CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK;

   // Wait for entering the freeze mode
   while((flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK) == 0){}

    // 11 bit standard mask
   flexcan_reg_ptr->RX15MASK = CAN_ID_EXT(ext_mask);

   // De-assert Freeze Mode
   flexcan_reg_ptr->MCR &= ~(CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);

   // Wait till exit of freeze mode
   while( flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK){}

   return (kFlexCan_OK);
}

/*!
 * @brief Enable an operation mode.
 *
 * @param   instance     The FlexCAN instance number.
 * @param   mode         An operation mode to be enabled.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_enable_operation_mode(
    uint8_t instance,
    flexcan_operation_modes_t mode)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address (instance);
    if (NULL == flexcan_reg_ptr)
       return (kFlexCan_INVALID_ADDRESS);

    if (mode == kFlexCanFreezeMode)
    {
        // Debug mode, Halt and Freeze
        flexcan_reg_ptr->MCR |= (CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);
        // check for freeze Ack
        if( (flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK) != CAN_MCR_FRZACK_MASK)
            return (kFlexCan_FREEZE_FAILED);

        return (kFlexCan_OK);
    } else if (mode == kFlexCanDisableMode)
    {
        // Debug mode, Halt and Freeze
        flexcan_reg_ptr->MCR |= CAN_MCR_MDIS_MASK;
        return (kFlexCan_OK);
    }

    // Set Freeze mode
    flexcan_reg_ptr->MCR |= CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK;

    // Wait for entering the freeze mode
    while((flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK) == 0){}

    if (mode == kFlexCanNormalMode)
        flexcan_reg_ptr->MCR &= (~CAN_MCR_SUPV_MASK);
    else if (mode == kFlexCanListenOnlyMode)
        flexcan_reg_ptr->CTRL1 |= CAN_CTRL1_LOM_MASK;
    else if (mode == kFlexCanLoopBackMode)
        flexcan_reg_ptr->CTRL1 |= CAN_CTRL1_LPB_MASK;
    else
        return kFlexCan_INVALID_MODE;

    // De-assert Freeze Mode
    flexcan_reg_ptr->MCR &= ~(CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);

    // Wait till exit of freeze mode
    while( flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK){}

    return (kFlexCan_OK);
}

/*!
 * @brief Disable an operation mode.
 *
 * @param   instance     The FlexCAN instance number.
 * @param   mode         An operation mode to be disabled.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_disable_operation_mode(
    uint8_t instance,
    flexcan_operation_modes_t mode)
{
    volatile CAN_MemMapPtr  flexcan_reg_ptr;

    flexcan_reg_ptr = _bsp_get_flexcan_base_address(instance);
    if (NULL == flexcan_reg_ptr)
        return (kFlexCan_INVALID_ADDRESS);

    if (mode == kFlexCanFreezeMode)
    {
        // Exit Freeze mode
        flexcan_reg_ptr->MCR &= (~CAN_MCR_FRZ_MASK);
        // Wait till exit of freeze mode
        while( flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK){}

        return (kFlexCan_OK);
    } else if (mode == kFlexCanDisableMode)
    {
        // Enable module mode
        flexcan_reg_ptr->MCR &= ~CAN_MCR_MDIS_MASK;
        return (kFlexCan_OK);
    }

    // Set Freeze mode
    flexcan_reg_ptr->MCR |= CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK;

    // Wait for entering the freeze mode
    while((flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK) == 0){}

    if (mode == kFlexCanNormalMode)
        flexcan_reg_ptr->MCR |= CAN_MCR_SUPV_MASK;
    else if (mode == kFlexCanListenOnlyMode)
        flexcan_reg_ptr->CTRL1 &= (~CAN_CTRL1_LOM_MASK);
    else if (mode == kFlexCanLoopBackMode)
        flexcan_reg_ptr->CTRL1 &= (~CAN_CTRL1_LPB_MASK);
    else
        return kFlexCan_INVALID_MODE;

    // De-assert Freeze Mode
    flexcan_reg_ptr->MCR &= ~(CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);

    // Wait till exit of freeze mode
    while( flexcan_reg_ptr->MCR & CAN_MCR_FRZACK_MASK){}

    return (kFlexCan_OK);
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////

