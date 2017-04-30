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

#include <mqx.h>
#include <bsp.h>
#include <lwevent.h>
#include "fsl_flexcan_driver.h"
#include "fsl_flexcan_int.h"

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

bool int_mb;
bool int_fifo;
static bool err_int_flag = 0;
uint32_t rx_mb_idx;

LWEVENT_STRUCT event;

//! The following table is based on the assumption that BSP_CANPE_CLOCK is defined. The CAN bit
//! timing parameters are calculated by using the method outlined in AN1798, section 4.1.
//! A maximum time for PROP_SEG will be used, the remaining TQ will be split equally between PSEG1
//! and PSEG2, if PSEG2 >=2.
//! RJW is set to the minimum of 4 or PSEG1.
//! The table contains bit_rate (Hz), propseg, pseg1, pseg2, pre_divider, and rjw.
#if (MQX_CPU == PSP_CPU_MK70F120M) || (MQX_CPU == PSP_CPU_MK65F180M)
const flexcan_bitrate_table_t bit_rate_table[] = {
    { 125000, 6, 7, 7, 19, 3},  /* 125 kHz */
    { 250000, 6, 7, 7,  9, 3},  /* 250 kHz */
    { 500000, 6, 7, 7,  4, 3},  /* 500 kHz */
    { 750000, 6, 5, 5,  3, 3},  /* 750 kHz */
    {1000000, 6, 5, 5,  2, 3},  /* 1   MHz */
};
#elif (MQX_CPU == PSP_CPU_MK60DN512Z)
const flexcan_bitrate_table_t bit_rate_table[] = {
    { 125000, 6, 7, 7, 15, 3},  /* 125 kHz */
    { 250000, 6, 7, 7,  7, 3},  /* 250 kHz */
    { 500000, 6, 7, 7,  3, 3},  /* 500 kHz */
    { 750000, 6, 3, 3,  3, 3},  /* 750 kHz */
    {1000000, 6, 7, 7,  1, 3},  /* 1   MHz */
};
#elif PSP_MQX_CPU_IS_VYBRID
const flexcan_bitrate_table_t bit_rate_table[] = {
    { 125000, 6, 7, 7, 21, 3},  /* 125 kHz on 66 MHz IPG*/
    { 250000, 6, 7, 7, 10, 2},  /* 250 kHz */
    { 500000, 6, 6, 6, 5,  3},  /* 500 kHz */
    {1000000, 3, 2, 2, 5,  1},  /* 1   MHz */
};
#endif

////////////////////////////////////////////////////////////////////////////////
// Prototypes
////////////////////////////////////////////////////////////////////////////////

void flexcan_irq_handler(void * can_ptr);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

/*!
 * @brief Set FlexCAN bit rate
 *
 * @param   instance    The FlexCAN instance number.
 * @param   bitrate     Select a FlexCAN bit rate in the bit_rate_table.
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_set_bitrate(uint8_t instance, uint32_t bitrate)
{
    uint32_t num_bitrate_table;
    uint32_t i;
    flexcan_time_segment_t time_seg;

    // Find the time segments from the table bit_rate_table
    num_bitrate_table = sizeof(bit_rate_table)/sizeof(bit_rate_table[0]);
    for (i = 0; i < num_bitrate_table; i++)
    {
        if (bit_rate_table[i].bit_rate == bitrate)
        {
            time_seg.propseg = bit_rate_table[i].propseg;
            time_seg.pseg1 = bit_rate_table[i].pseg1;
            time_seg.pseg2 = bit_rate_table[i].pseg2;
            time_seg.pre_divider = bit_rate_table[i].pre_divider;
            time_seg.rjw = bit_rate_table[i].rjw;
            break;
        }
    }

    if (i == num_bitrate_table)
    {
        return kFlexCan_INVALID_FREQUENCY;
    }

    // Set time segments
    return (flexcan_hal_set_time_segments(instance, &time_seg));
}

/*!
 * @brief Get FlexCAN bitrate
 *
 * @param   instance    The FlexCAN instance number.
 * @param   bitrate     Pointer to a variable for returing the FlexCAN bit rate
 *                      in the bit_rate_table.
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_get_bitrate(uint8_t instance, uint32_t *bitrate)
{
    uint32_t i;
    flexcan_time_segment_t time_seg;
    uint32_t num_bitrate_table;
    uint32_t result;

    //result = _cm_get_clock(_cm_get_clock_configuration(), CM_CLOCK_SOURCE_BUS);
    //printf("\nbus clock: %ld", result);

    //result = _cm_get_clock(_cm_get_clock_configuration(), CM_CLOCK_SOURCE_IPG);
    //printf("\nIPG clock: %ld", result);

    // Get the time segments
    result = flexcan_hal_get_time_segments(instance, &time_seg);
    if (result)
    {
        return result;
    }

    // Find out the corresponding bit rate in the table bit_rate_table
    num_bitrate_table = sizeof(bit_rate_table)/sizeof(bit_rate_table[0]);
    for (i = 0; i < num_bitrate_table; i++)
    {
        if (bit_rate_table[i].pre_divider == time_seg.pre_divider)
        {
            if (bit_rate_table[i].pseg2 == time_seg.pseg2)
                {
                *bitrate = bit_rate_table[i].bit_rate;
                return kFlexCan_OK;
                }
        }
    }

    return kFlexCan_INVALID_FREQUENCY;
}

/*!
 * @brief Set RX masking type
 *
 * @param   instance     The FlexCAN instance number.
 * @param   type         The FlexCAN RX mask type.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_set_mask_type(uint8_t instance, flexcan_rx_mask_type_t type)
{
    return flexcan_hal_set_mask_type(instance, type);
}

/*!
 * @brief Set FlexCAN RX FIFO global standard or extended mask.
 *
 * @param   instance    The FlexCAN instance number.
 * @param   id_type     Standard ID or extended ID.
 * @param   mask        Mask value.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_set_rx_fifo_global_mask(
    uint8_t instance,
    flexcan_mb_id_type_t id_type,
    uint32_t mask)
{
    uint32_t result;

    if (id_type == kFlexCanMbId_Std)
    {
        // Set standard global mask for RX FIOF
        result = flexcan_hal_set_rx_fifo_global_std_mask(instance, mask);
        if (result)
            return result;
    }
    else if (id_type == kFlexCanMbId_Ext)
    {
        // Set extended global mask for RX FIFO
        result = flexcan_hal_set_rx_fifo_global_ext_mask(instance, mask);
        if (result)
            return result;
    }
    else
    {
        return kFlexCan_INVALID_ID_TYPE;
    }

    return kFlexCan_OK;
}

/*!
 * @brief Set FlexCAN RX MB global standard or extended mask.
 *
 * @param   instance    The FlexCAN instance number.
 * @param   id_type     Standard ID or extended ID.
 * @param   mask        Mask value.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_set_rx_mb_global_mask(
    uint8_t instance,
    flexcan_mb_id_type_t id_type,
    uint32_t mask)
{
    uint32_t result;

    if (id_type == kFlexCanMbId_Std)
    {
        // Set standard global mask for RX MB
        result = flexcan_hal_set_rx_mb_global_std_mask(instance, mask);
        if (result)
            return result;
    }
    else if (id_type == kFlexCanMbId_Ext)
    {
        // Set extended global mask for RX MB
        result = flexcan_hal_set_rx_mb_global_ext_mask(instance, mask);
        if (result)
            return result;
    }
    else
    {
        return kFlexCan_INVALID_ID_TYPE;
    }

    return kFlexCan_OK;
}

/*!
 * @brief Set FlexCAN RX individual standard or extended mask.
 *
 * @param   instance                   The FlexCAN instance number.
 * @param   id_type                    Standard ID or extended ID.
 * @param   mb_idx                     Index of the message buffer
 * @param   mask                       Mask value.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_set_rx_individual_mask(
    uint8_t instance,
    flexcan_mb_id_type_t id_type,
    uint32_t mb_idx,
    uint32_t mask)
{
    uint32_t result;

    if (id_type == kFlexCanMbId_Std)
    {
        // Set standard individual mask
        result = flexcan_hal_set_rx_individual_std_mask(instance, mb_idx, mask);
        if (result)
            return result;
    }
    else if (id_type == kFlexCanMbId_Ext)
    {
        // Set extended individual mask
        result = flexcan_hal_set_rx_individual_ext_mask(instance, mb_idx, mask);
        if (result)
            return result;
    }
    else
    {
        return kFlexCan_INVALID_ID_TYPE;
    }

    return kFlexCan_OK;
}

/*!
 * @brief Initialize FlexCAN peripheral
 *
 * This function initializes
 * @param   instance                   The FlexCAN instance number.
 * @param   data                       The FlexCAN platform data.
 * @param   enable_err_interrupts      1 if enable it, 0 if not.
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_init(
   uint8_t instance,
   const flexcan_config_t *data,
   bool enable_err_interrupts)
{
    uint32_t result;

    if (!data)
    {
        return kFlexCan_UNDEF_ERROR;
    }

    // Initialize pins for FlexCAN device
    result = _bsp_flexcan_io_init(instance);
    if (result)
    {
        return result;
    }

    // Initialize FLEXCAN device
    result = flexcan_hal_init(instance, data);
    if (result)
        return result;

    // Select mode
    result = flexcan_hal_enable_operation_mode(instance, kFlexCanNormalMode);
    if (result)
        return result;

    // Enable error interrupts
    if(enable_err_interrupts)
    {
#if !(PSP_MQX_CPU_IS_VYBRID)
        result = flexcan_install_isr_err_int(instance, flexcan_irq_handler);
        if (result)
            return result;

        result = flexcan_install_isr_boff_int(instance, flexcan_irq_handler);
        if (result)
            return result;

        result = flexcan_error_int_enable(instance);
        if (result)
            return result;
#endif

        err_int_flag = enable_err_interrupts;

        result = flexcan_hal_enable_error_interrupt(instance);
        if (result)
            return result;

        result = flexcan_hal_enable_tx_rx_warning_interrupt(instance);
        if (result)
            return result;
    }

    // Set up an event group
    result = _lwevent_create(&event, LWEVENT_AUTO_CLEAR);
    if (result != MQX_OK) {
        printf("\nCannot create lwevent");
        return result;
    }

   return (kFlexCan_OK);
}

/*!
 * @brief FlexCAN transmit message buffer field configuration.
 *
 * @param   instance                   The FlexCAN instance number.
 * @param   data                       The FlexCAN platform data.
 * @param   mb_idx                     Index of the message buffer
 * @param   cs                         CODE/status values (TX)
 * @param   msg_id                     ID of the message to transmit
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_tx_mb_config(
    uint8_t instance,
    const flexcan_config_t *data,
    uint32_t mb_idx,
    flexcan_mb_code_status_tx_t *cs,
    uint32_t msg_id)
{
    uint32_t result;

    // Initialize transmit mb
    cs->code = kFlexCanTX_Inactive;
    result = flexcan_hal_inactive_mb_tx(instance, data, mb_idx);
    if (result)
        return result;

    // Install ISR
    result = flexcan_install_isr(instance, mb_idx, flexcan_irq_handler);
    if (result)
        return result;

    // Enable interrupt line
    result = flexcan_int_enable(instance, mb_idx);
    if (result)
        return result;

    // Enable message buffer interrupt
    return flexcan_hal_enable_mb_interrupt(instance, data, mb_idx);
}

/*!
 * @brief Send FlexCAN messages
 *
 * @param   instance                   The FlexCAN instance number.
 * @param   data                       The FlexCAN platform data.
 * @param   mb_idx                     Index of the message buffer
 * @param   cs                         CODE/status values (TX)
 * @param   msg_id                     ID of the message to transmit
 * @param   num_bytes                  The number of bytes in mb_data
 * @param   mb_data                    Bytes of the FlexCAN message
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_send(
    uint8_t instance,
    const flexcan_config_t *data,
    uint32_t mb_idx,
    flexcan_mb_code_status_tx_t *cs,
    uint32_t msg_id,
    uint32_t num_bytes,
    uint8_t *mb_data)
{
    uint32_t result;
    uint32_t num_byte_left;
    uint32_t total_byte_data_transmitted;

    if (!mb_data)
    {
        return kFlexCan_UNDEF_ERROR;
    }

    // If the byte count of the data for transmitting is zero, then return immediately.
    if (num_bytes == 0)
    {
        return kFlexCan_NO_MESSAGE;
    }

    // Start transmitting data
    num_byte_left = num_bytes;
    total_byte_data_transmitted = 0;

    while (num_byte_left)
    {
        if (num_byte_left < cs->data_length)
            cs->data_length = num_byte_left;

        // Set up FlexCAN message buffer for transmitting data
        cs->code = kFlexCanTX_Data;
        result = flexcan_hal_set_mb_tx(instance, data, mb_idx, cs, msg_id,
                                       (mb_data + total_byte_data_transmitted));

        if(result == kFlexCan_OK)
        {
            // Wait for the event
            if (_lwevent_wait_ticks(&event, 1 << mb_idx, FALSE, 0) != MQX_OK) {
                printf("\nEvent Wait for send failed");
            }
        }
        else
        {
#ifdef DEBUG
            printf("\nTransmit error. Error Code: 0x%lx", result);
#endif
        }

        total_byte_data_transmitted += cs->data_length;
        num_byte_left -= cs->data_length;
    }

    return (kFlexCan_OK);
}

/*!
 * @brief FlexCAN receive message buffer field configuration.
 *
 * @param   instance                   The FlexCAN instance number.
 * @param   data                       The FlexCAN platform data.
 * @param   mb_idx                     Index of the message buffer
 * @param   cs                         CODE/status values (RX)
 * @param   msg_id                     ID of the message to transmit
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_rx_mb_config(
    uint8_t instance,
    const flexcan_config_t *data,
    uint32_t mb_idx,
    flexcan_mb_code_status_rx_t *cs,
    uint32_t msg_id)
{
    uint32_t result;

    rx_mb_idx = mb_idx;

    // Initialize rx mb
    cs->code = kFlexCanRX_NotUsed;
    result = flexcan_hal_set_mb_rx(instance, data, mb_idx, cs, msg_id);
    if (result)
         return result;

    // Install ISR
    result = flexcan_install_isr(instance, mb_idx, flexcan_irq_handler);
    if (result)
        return result;

    // Enable interrupt line
    result = flexcan_int_enable(instance, mb_idx);
    if(result)
         return result;

    // Enable MB interrupt
    result = flexcan_hal_enable_mb_interrupt(instance, data, mb_idx);
    if (result)
         return result;

    // Initialize receive MB
    cs->code = kFlexCanRX_Inactive;
    result = flexcan_hal_set_mb_rx(instance, data, mb_idx, cs, msg_id);
    if (result)
         return result;

    // Set up FlexCAN message buffer fields for receiving data
    cs->code = kFlexCanRX_Empty;
    return flexcan_hal_set_mb_rx(instance, data, mb_idx, cs, msg_id);
}

/*!
 * @brief FlexCAN RX FIFO field configuration.
 *
 * @param   instance           The FlexCAN instance number.
 * @param   data               The FlexCAN platform data.
 * @param   id_format          The format of the Rx FIFO ID Filter Table Elements.
 * @param   id_filter_table    The ID filter table elements which contain RTR bit, IDE bit,
 *                             and RX message ID.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_rx_fifo_config(
    uint8_t instance,
    const flexcan_config_t *data,
    flexcan_rx_fifo_id_element_format_t id_format,
    flexcan_id_table_t *id_filter_table)
{
    uint32_t result;

    // Initialize rx fifo
    result = flexcan_hal_set_rx_fifo(instance, data, id_format, id_filter_table);
    if(result)
         return result;

    // Install ISR
    result = flexcan_install_isr(instance, 5, flexcan_irq_handler);
    if (result)
        return result;

    // Enable interrupt line
    result = flexcan_int_enable(instance, 5);
    if(result)
         return result;

    // Enable RX FIFO interrupts
    result = flexcan_hal_enable_mb_interrupt(instance, data, 5);
    if(result)
         return result;

    result = flexcan_int_enable(instance, 6);
    if(result)
         return result;

    result = flexcan_hal_enable_mb_interrupt(instance, data, 6);
    if(result)
         return result;

    result = flexcan_int_enable(instance, 7);
    if(result)
         return result;

    result = flexcan_hal_enable_mb_interrupt(instance, data, 7);
    if(result)
         return result;

   return (kFlexCan_OK);
}

/*!
 * @brief FlexCAN is waiting for receiving data.
 *
 * @param   instance                   The FlexCAN instance number.
 * @param   data                       The FlexCAN platform data.
 * @param   mb_idx                     Index of the message buffer
 * @param   msg_id                     ID of the message to transmit
 * @param   receiveDataCount           The number of data to be received
 * @param   rx_mb                      The FlexCAN receive message buffer data.
 * @param   rx_fifo                    The FlexCAN receive FIFO data.
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_start_receive(
    uint8_t instance,
    const flexcan_config_t *data,
    uint32_t mb_idx,
    uint32_t msg_id,
    uint32_t receiveDataCount,
    bool *is_rx_mb_data,
    bool *is_rx_fifo_data,
    flexcan_mb_t *rx_mb,
    flexcan_mb_t *rx_fifo)
{
    uint32_t result;
    uint32_t bit_mask = 0;

    // Find out bit mask. Each set bit represents an event bit to wait for
    if (data->is_rx_mb_needed)
    {
        bit_mask |= 1 << mb_idx;
    }

    if (data->is_rx_fifo_needed)
    {
        bit_mask |= 1 << 5;
    }

    if (bit_mask == 0)
        return kFlexCan_UNDEF_ERROR;

    while (receiveDataCount)
    {
        int_mb = FALSE;
        int_fifo = FALSE;

        // Wait for the event
        if (_lwevent_wait_ticks(&event, bit_mask, FALSE, 0) != MQX_OK) {
#ifdef DEBUG
            printf("\nEvent Wait failed");
#endif
        }

        if (int_mb)
        {
            *is_rx_mb_data = TRUE;

            // Lock RX MB
            result = flexcan_hal_lock_rx_mb(instance, data, mb_idx);
            if(result != kFlexCan_OK)
            {
                return result;
            }

            // Get RX MB field values
            result = flexcan_hal_get_mb(instance, data, mb_idx, rx_mb);
            if(result != kFlexCan_OK)
                return result;
        }

        if (int_fifo)
        {
            *is_rx_fifo_data = TRUE;

            // Lock RX FIFO
            result = flexcan_hal_lock_rx_mb(instance, data, 0);
            if(result != kFlexCan_OK)
            {
                return result;
            }

            // Get RX FIFO field values
            result = flexcan_hal_read_fifo(instance,rx_fifo);
            if(result != kFlexCan_OK)
                return result;
        }

        // Unlock RX message buffer and RX FIFO
        result = flexcan_hal_unlock_rx_mb(instance);
        if(result != kFlexCan_OK)
        {
            return result;
        }

        receiveDataCount--;
    }

   return (kFlexCan_OK);
}

/*!
 * @brief Shutdown a FlexCAN instance.
 *
 * @param   instance    The FlexCAN instance number.
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_shutdown(uint8_t instance)
{
    uint32_t result;
    if (!err_int_flag){
        result = _lwevent_destroy(&event);
        if(result != MQX_OK)
            return result;
    }
    else
    {
#if !(PSP_MQX_CPU_IS_VYBRID)
        /* Disable error interrupts*/
        result =  flexcan_hal_disable_error_interrupt(instance);
        if (result)
            return result;

        /*Disables error, wake up & Bus off interrupts.*/
        result = flexcan_error_int_disable(instance);
        if (result)
            return result;
        /*Uninstalls interrupt handler for a flexcan error.*/
        result = flexcan_uninstall_isr_err_int(instance);
        if (result)
            return result;
        /*Uninstalls interrupt handler for a flexcan bus off.*/
        result = flexcan_uninstall_isr_boff_int(instance);
        if (result)
            return result;
#endif
        /* Disable interrupt line*/
        result = flexcan_int_disable(instance, 1);
        if (result)
            return result;
        /* Uninstall ISR.*/
        result = flexcan_uninstall_isr(instance);
        if (result)
            return result;

        /* Clear error interrupt flag */
        err_int_flag = 0;
    }
    return (flexcan_hal_disable(instance));
}

/*!
 * @brief Interrupt handler for a FlexCAN instance.
 *
 * @param   can_ptr                   point to a FlexCAN instance.
 */
void flexcan_irq_handler(void * can_ptr)
{
    volatile CAN_MemMapPtr        can_reg_ptr;
    volatile uint32_t             tmp_reg;

    uint8_t instance;

    can_reg_ptr = (CAN_MemMapPtr)can_ptr;

    if (can_reg_ptr == CAN0_BASE_PTR)
        instance = 0;
    else if (can_reg_ptr == CAN1_BASE_PTR)
        instance = 1;
    else
        return;

    // get the interrupt flag
    tmp_reg = (flexcan_hal_get_all_mb_int_flags(instance)) & CAN_IMASK1_BUFLM_MASK;

    // check Tx/Rx interrupt flag and clear the interrupt
    if(tmp_reg){
        if (tmp_reg & 0x20)
        {
            int_fifo = TRUE;
        }

        if (tmp_reg & (1 << rx_mb_idx))
        {
            int_mb = TRUE;
        }

        // clear the interrupt and unlock message buffer
        _lwevent_set(&event, tmp_reg);
        flexcan_hal_clear_mb_int_flag(instance, tmp_reg);
        flexcan_hal_unlock_rx_mb(instance);
   }

    // Clear all other interrupts in ERRSTAT register (Error, Busoff, Wakeup)
    flexcan_hal_clear_err_interrupt_status(instance);
#if PSP_MQX_CPU_IS_VYBRID
    flexcan_hal_clear_err_int_flag(instance);
#endif

    return;
}

