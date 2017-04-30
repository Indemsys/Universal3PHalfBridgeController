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
#ifndef __FSL_FLEXCAN_HAL_H__
#define __FSL_FLEXCAN_HAL_H__

#include <stdint.h>
#include <mqx.h>
#include <psp.h>

//! @addtogroup flexcan_hal
//! @{

//! @file

/////////////////////////////////////////////////////////////////////////////
// Definitions
/////////////////////////////////////////////////////////////////////////////

//! @brief FlexCAN constants
enum _flexcan_constants
{
    kFlexCanMessageSize = 8,        //!< The FlexCAN message size
#if PSP_MQX_CPU_IS_KINETIS
    kFlexCanTotalRxImrNumber = 16   //!< The number of total RXIMR registers
#else
    kFlexCanTotalRxImrNumber = 64   //!< The number of total RXIMR registers
#endif
};

//! @brief FlexCAN return error codes
enum _flexcan_return_error_codes
{
    kFlexCan_OK                       = (0x00),
    kFlexCan_UNDEF_ERROR              = (FLEXCAN_ERROR_BASE | 0x01),
    kFlexCan_MESSAGE14_TX             = (FLEXCAN_ERROR_BASE | 0x02),
    kFlexCan_MESSAGE15_TX             = (FLEXCAN_ERROR_BASE | 0x03),
    kFlexCan_MESSAGE_OVERWRITTEN      = (FLEXCAN_ERROR_BASE | 0x04),
    kFlexCan_NO_MESSAGE               = (FLEXCAN_ERROR_BASE | 0x05),
    kFlexCan_MESSAGE_LOST             = (FLEXCAN_ERROR_BASE | 0x06),
    kFlexCan_MESSAGE_BUSY             = (FLEXCAN_ERROR_BASE | 0x07),
    kFlexCan_MESSAGE_ID_MISSMATCH     = (FLEXCAN_ERROR_BASE | 0x08),
    kFlexCan_MESSAGE14_START          = (FLEXCAN_ERROR_BASE | 0x09),
    kFlexCan_MESSAGE15_START          = (FLEXCAN_ERROR_BASE | 0x0A),
    kFlexCan_INVALID_ADDRESS          = (FLEXCAN_ERROR_BASE | 0x0B),
    kFlexCan_INVALID_MAILBOX          = (FLEXCAN_ERROR_BASE | 0x0C),
    kFlexCan_TIMEOUT                  = (FLEXCAN_ERROR_BASE | 0x0D),
    kFlexCan_INVALID_FREQUENCY        = (FLEXCAN_ERROR_BASE | 0x0E),
    kFlexCan_INT_ENABLE_FAILED        = (FLEXCAN_ERROR_BASE | 0x0F),
    kFlexCan_INT_DISABLE_FAILED       = (FLEXCAN_ERROR_BASE | 0x10),
    kFlexCan_INT_INSTALL_FAILED       = (FLEXCAN_ERROR_BASE | 0x11),
    kFlexCan_REQ_MAILBOX_FAILED       = (FLEXCAN_ERROR_BASE | 0x12),
    kFlexCan_DATA_SIZE_ERROR          = (FLEXCAN_ERROR_BASE | 0x13),
    kFlexCan_MESSAGE_FORMAT_UNKNOWN   = (FLEXCAN_ERROR_BASE | 0x14),
    kFlexCan_INVALID_DIRECTION        = (FLEXCAN_ERROR_BASE | 0x15),
    kFlexCan_RTR_NOT_SET              = (FLEXCAN_ERROR_BASE | 0x16),
    kFlexCan_SOFTRESET_FAILED         = (FLEXCAN_ERROR_BASE | 0x17),
    kFlexCan_INVALID_MODE             = (FLEXCAN_ERROR_BASE | 0x18),
    kFlexCan_START_FAILED             = (FLEXCAN_ERROR_BASE | 0x19),
    kFlexCan_CLOCK_SOURCE_INVALID     = (FLEXCAN_ERROR_BASE | 0x1A),
    kFlexCan_INIT_FAILED              = (FLEXCAN_ERROR_BASE | 0x1B),
    kFlexCan_ERROR_INT_ENABLE_FAILED  = (FLEXCAN_ERROR_BASE | 0x1C),
    kFlexCan_ERROR_INT_DISABLE_FAILED = (FLEXCAN_ERROR_BASE | 0x1D),
    kFlexCan_FREEZE_FAILED            = (FLEXCAN_ERROR_BASE | 0x1E),
    kFlexCan_INVALID_ID_TYPE          = (FLEXCAN_ERROR_BASE | 0x1F)
};

//! @brief FlexCAN interrupt levels
enum _flexcan_int_level
{
    kFlexCan_MESSBUF_INT_LEVEL       = (3),
    kFlexCan_MESSBUF_INT_SUBLEVEL    = (4),

    kFlexCan_ERROR_INT_LEVEL         = (3),
    kFlexCan_ERROR_INT_SUBLEVEL      = (2),

    kFlexCan_BUSOFF_INT_LEVEL        = (3),
    kFlexCan_BUSOFF_INT_SUBLEVEL     = (3),

    kFlexCan_WAKEUP_INT_LEVEL        = (3),
    kFlexCan_WAKEUP_INT_SUBLEVEL     = (1)
};

//! @brief The Status enum is used to report current status of the CAN interface.
enum _flexcan_err_status
{
    kFlexCan_RxWrn   = 0x0080, //!< Reached warning level for RX errors
    kFlexCan_TxWrn   = 0x0100, //!< Reached warning level for TX errors
    kFlexCan_StfErr  = 0x0200, //!< Stuffing Error
    kFlexCan_FrmErr  = 0x0400, //!< Form Error
    kFlexCan_CrcErr  = 0x0800, //!< Cyclic Redundancy Check Error
    kFlexCan_AckErr  = 0x1000, //!< Received no ACK on transmission
    kFlexCan_Bit0Err = 0x2000, //!< Unable to send dominant bit
    kFlexCan_Bit1Err = 0x4000, //!< Unable to send recessive bit
};

//! @brief FlexCAN operation modes
typedef enum _flexcan_operation_modes {
    kFlexCanNormalMode,        //!< normal mode or user mode
    kFlexCanListenOnlyMode,    //!< listen-only mode
    kFlexCanLoopBackMode,      //!< loop-back mode
    kFlexCanFreezeMode,        //!< Freeze mode
    kFlexCanDisableMode,       //!< Module disable mode
} flexcan_operation_modes_t;

//! @brief FlexCAN message buffer CODE for Rx buffers.
typedef enum _flexcan_mb_code_rx {
    kFlexCanRX_Inactive  = 0x0, //!< MB is not active
    kFlexCanRX_Full      = 0x2, //!< MB is full
    kFlexCanRX_Empty     = 0x4, //!< MB is active and empty
    kFlexCanRX_Overrun   = 0x6, //!< MB is being overwritten into a full buffer
    kFlexCanRX_Busy      = 0x8, //!< FlexCAN is updating the contents of the MB.
                                //!  The CPU must not access the MB.
    kFlexCanRX_Ranswer   = 0xA, //!< A frame was configured to recognize a Remote Request Frame
                                //!  and transmit a Response Frame in return
    kFlexCanRX_NotUsed   = 0xF, //!< Not used
} flexcan_mb_code_rx_t;

//! @brief FlexCAN message buffer CODE FOR Tx buffers.
typedef enum _flexcan_mb_code_tx {
    kFlexCanTX_Inactive  = 0x08, //!< MB is not active
    kFlexCanTX_Abort     = 0x09, //!< MB is aborted
    kFlexCanTX_Data      = 0x0C, //!< MB is a TX Data Frame(MB RTR must be 0)
    kFlexCanTX_Remote    = 0x1C, //!< MB is a TX Remote Request Frame (MB RTR must be 1)
    kFlexCanTX_Tanswer   = 0x0E, //!< MB is a TX Response Request Frame from
                                 //!  an incoming Remote Request Frame
    kFlexCanTX_NotUsed   = 0xF,  //!< Not used
} flexcan_mb_code_tx_t;

//! @brief FlexCAN message buffer transmission types.
typedef enum _flexcan_mb_transmission_type {
    kFlexCanMBStatusType_TX,          //!< transmit MB
    kFlexCanMBStatusType_TXRemote,    //!< transmit remote request MB
    kFlexCanMBStatusType_RX,          //!< receive MB
    kFlexCanMBStatusType_RXRemote,    //!< receive remote request MB
    kFlexCanMBStatusType_RXTXRemote,  //!< FlexCAN remote frame receives remote request,
                                      //!  then transmit MB
} flexcan_mb_transmission_type_t;

typedef enum _flexcan_rx_fifo_id_element_format {
    kFlexCanRxFifoIdElementFormat_A, //!< One full ID (standard and extended) per ID Filter Table
                                     //!  element.
    kFlexCanRxFifoIdElementFormat_B, //!< Two full standard IDs or two partial 14-bit (standard and
                                     //!  extended) IDs per ID Filter Table element.
    kFlexCanRxFifoIdElementFormat_C, //!< Four partial 8-bit Standard IDs per ID Filter Table
                                     //!  element.
    kFlexCanRxFifoIdElementFormat_D, //!< All frames rejected.
} flexcan_rx_fifo_id_element_format_t;

//! @brief FlexCAN Rx FIFO filters number.
typedef enum _flexcan_rx_fifo_id_filter_number {
    kFlexCanRxFifoIDFilters_8   = 0x0,         //!<   8 Rx FIFO Filters
    kFlexCanRxFifoIDFilters_16  = 0x1,         //!<  16 Rx FIFO Filters
    kFlexCanRxFifoIDFilters_24  = 0x2,         //!<  24 Rx FIFO Filters
    kFlexCanRxFifoIDFilters_32  = 0x3,         //!<  32 Rx FIFO Filters
    kFlexCanRxFifoIDFilters_40  = 0x4,         //!<  40 Rx FIFO Filters
    kFlexCanRxFifoIDFilters_48  = 0x5,         //!<  48 Rx FIFO Filters
    kFlexCanRxFifoIDFilters_56  = 0x6,         //!<  56 Rx FIFO Filters
    kFlexCanRxFifoIDFilters_64  = 0x7,         //!<  64 Rx FIFO Filters
    kFlexCanRxFifoIDFilters_72  = 0x8,         //!<  72 Rx FIFO Filters
    kFlexCanRxFifoIDFilters_80  = 0x9,         //!<  80 Rx FIFO Filters
    kFlexCanRxFifoIDFilters_88  = 0xA,         //!<  88 Rx FIFO Filters
    kFlexCanRxFifoIDFilters_96  = 0xB,         //!<  96 Rx FIFO Filters
    kFlexCanRxFifoIDFilters_104 = 0xC,         //!< 104 Rx FIFO Filters
    kFlexCanRxFifoIDFilters_112 = 0xD,         //!< 112 Rx FIFO Filters
    kFlexCanRxFifoIDFilters_120 = 0xE,         //!< 120 Rx FIFO Filters
    kFlexCanRxFifoIDFilters_128 = 0xF          //!< 128 Rx FIFO Filters
} flexcan_rx_fifo_id_filter_num_t;

//! @brief FlexCAN RX FIFO ID filter table structure
typedef struct flexcan_id_table {
    bool is_remote_mb;      //!< Remote frame
    bool is_extended_mb;    //!< Extended frame
    uint32_t *id_filter;    //!< RX FITO ID filter elements
} flexcan_id_table_t;

//! @brief FlexCAN RX mask type.
typedef enum _flexcan_rx_mask_type {
    kFlexCanRxMask_Global,      //!< RX global mask
    kFlexCanRxMask_Individual,  //!< RX individual mask
} flexcan_rx_mask_type_t;

//! @brief FlexCAN MB ID type
typedef enum _flexcan_mb_id_type {
    kFlexCanMbId_Std,           //!< Standard ID
    kFlexCanMbId_Ext,           //!< Extended ID
} flexcan_mb_id_type_t;

//! @brief FlexCAN clock source
typedef enum _flexcan_clk_source {
    kFlexCanClkSource_Osc,           //!< the Oscillator clock
    kFlexCanClkSource_Ipbus,         //!< the peripheral clock
} flexcan_clk_source_t;

//! @brief FlexCAN error interrupt types
typedef enum _flexcan_int_type {
    kFlexCanInt_Buf,           //!< OR'd message buffers interrupt
    kFlexCanInt_Err,           //!< Error interrupt
    kFlexCanInt_Boff,          //!< Bus off interrupt
    kFlexCanInt_Wakeup,        //!< Wakeup interrupt
    kFlexCanInt_Txwarning,     //!< TX warning interrupt
    kFlexCanInt_Rxwarning,     //!< RX warning interrupt
} flexcan_int_type_t;

//! @brief FlexCAN bus error counters
typedef struct flexcan_berr_counter {
    uint16_t txerr;           //!< Transmit error counter
    uint16_t rxerr;           //!< Receive error counter
} flexcan_berr_counter_t;

//! @brief FlexCAN MB CODE and status for transmitting
typedef struct flexcan_mb_code_status_tx {
    flexcan_mb_code_tx_t code;                   //!< MB code for TX buffers
    flexcan_mb_id_type_t msg_id_type;            //!< Type of message ID (standard or extended)
    uint32_t data_length;                        //!< Length of Data in Bytes
    uint32_t substitute_remote;                  //!< Substitute remote request (used only in
                                                 //!  extended format)
    uint32_t remote_transmission;                //!< Remote transmission request
    bool local_priority_enable;                  //!< 1 if enable it; 0 if disable it
    uint32_t local_priority_val;                 //!< Local priority value [0..2]
} flexcan_mb_code_status_tx_t;

//! @brief FlexCAN MB CODE and status for receiving
typedef struct flexcan_mb_code_status_rx {
    flexcan_mb_code_rx_t code;                        //!< MB code for RX buffers
    flexcan_mb_id_type_t msg_id_type;            //!< Type of message ID (standard or extended)
    uint32_t data_length;                        //!< Length of Data in Bytes
    uint32_t substitute_remote;                  //!< Substitute remote request (used only in
                                                 //!  extended format)
    uint32_t remote_transmission;                //!< Remote transmission request
    bool local_priority_enable;                  //!< 1 if enable it; 0 if disable it
    uint32_t local_priority_val;                 //!< Local priority value [0..2]
} flexcan_mb_code_status_rx_t;

//! @brief FlexCAN RX FIFO config
typedef struct flexcan_rx_fifo_config {
    flexcan_mb_id_type_t msg_id_type;                     //!< Type of message ID
                                                          //!  (standard or extended)
    uint32_t data_length;                                 //!< Length of Data in Bytes
    uint32_t substitute_remote;                           //!< Substitute remote request (used
                                                          //!  only in extended format)
    uint32_t remote_transmission;                         //!< Remote transmission request
    flexcan_rx_fifo_id_element_format_t id_filter_number; //!< The number of RX FIFO ID filters
} flexcan_rx_fifo_config_t;

//! @brief FlexCAN message buffer structure
typedef struct flexcan_mb {
    uint32_t cs;                        //!< Code and Status
    uint32_t msg_id;                    //!< Message Buffer ID
    uint8_t data[kFlexCanMessageSize];  //!< bytes of the FlexCAN message
} flexcan_mb_t;

//! @brief FlexCAN configuration
typedef struct flexcan_config {
    uint32_t num_mb;                                //!< The number of Message Buffers needed
    uint32_t max_num_mb;                            //!< The maximum number of Message Buffers
    uint32_t num_rximr;                             //!< The number of total RXIMR registers
    flexcan_rx_fifo_id_filter_num_t num_id_filters; //!< The number of RX FIFO ID filters needed
    bool is_rx_fifo_needed;                         //!< 1 if need it; 0 if not
    bool is_rx_mb_needed;                           //!< 1 if need it; 0 if not
} flexcan_config_t;

//! @brief FlexCAN timing related structures
typedef struct flexcan_time_segment {
    uint32_t propseg;     //!< Propagation segment
    uint32_t pseg1;       //!< Phase segment 1
    uint32_t pseg2;       //!< Phase segment 2
    uint32_t pre_divider; //!< Clock pre divider
    uint32_t rjw;         //!< Resync jump width
} flexcan_time_segment_t;


/////////////////////////////////////////////////////////////////////////////
// API
/////////////////////////////////////////////////////////////////////////////

#if defined(__cplusplus)
extern "C" {
#endif

//! @name FlexCAN Driver
//@{

/*!
 * @brief Enable FlexCAN controller
 *
 * @param   instance    The FlexCAN instance number.
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_hal_enable(uint8_t instance);

/*!
 * @brief Disable FlexCAN controller
 *
 * @param   instance    The FlexCAN instance number.
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_hal_disable(uint8_t instance);

/*!
 * @brief Reset FlexCAN controller
 *
 * @param   instance    The FlexCAN instance number.
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_hal_sw_reset(uint8_t instance);

/*!
 * @brief Select the clock source for FlexCAN
 *
 * @param   instance    The FlexCAN instance number.
 * @param   clk         The FlexCAN clock source.
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_hal_select_clk(uint8_t instance, flexcan_clk_source_t clk);

/*!
 * @brief Initialize FlexCAN controller
 *
 * @param   instance    The FlexCAN instance number.
 * @param   data   The FlexCAN platform data.
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_hal_init(uint8_t instance, const flexcan_config_t *data);

/*!
 * @brief Set FlexCAN time segments for setting up bit rate
 *
 * @param   instance    The FlexCAN instance number.
 * @param   time_seg    FlexCAN time segments need to be set for bit rate.
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_hal_set_time_segments(uint8_t instance, flexcan_time_segment_t *time_seg);

/*!
 * @brief Get FlexCAN time segments for calculating the bitrate.
 *
 * @param   instance    The FlexCAN instance number.
 * @param   time_seg    FlexCAN time segments read for bitrate.
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_hal_get_time_segments(uint8_t instance, flexcan_time_segment_t *time_seg);

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
    uint32_t mb_idx);

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
    uint8_t *mb_data);

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
    uint32_t msg_id);

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
    flexcan_mb_t *mb);

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
    uint32_t mb_idx);

/*!
 * @brief Unlock FlexCAN RX message buffer
 *
 * @param   instance     The FlexCAN instance number.
 * @return  0 if successful; non-zero failed
 */
uint32_t flexcan_hal_unlock_rx_mb(uint8_t instance);

/*!
 * @brief Enable Rx FIFO.
 *
 * @param   instance     The FlexCAN instance number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_enable_rx_fifo(uint8_t instance);

/*!
 * @brief Disable Rx FIFO.
 *
 * @param   instance     The FlexCAN instance number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_disable_rx_fifo(uint8_t instance);

/*!
 * @brief Set the number of Rx FIFO filters.
 *
 * @param   instance     The FlexCAN instance number.
 * @param   number       The number of Rx FIFO filters.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_set_rx_fifo_filters_number(uint8_t instance, uint32_t number);

/*!
 * @brief Set the maximum number of Message Buffers.
 *
 * @param   instance     The FlexCAN instance number.
 * @param   data         The FlexCAN platform data.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_set_max_mb_number(
    uint8_t instance,
    const flexcan_config_t *data);

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
    flexcan_id_table_t *id_filter_table);

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
    flexcan_id_table_t *id_filter_table);

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
    uint32_t mb_idx);

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
    uint32_t mb_idx);

/*!
 * @brief Enable all error interrupts of the FlexCAN module
 *
 * It enables the related HW module interrupt.
 *
 * @param   instance     The FlexCAN instance number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_enable_error_interrupt(uint8_t instance);

/*!
 * @brief Disable all error interrupts of the FlexCAN module
 *
 * It disables the related HW module interrupt.
 *
 * @param   instance     The FlexCAN instance number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_disable_error_interrupt(uint8_t instance);

/*!
 * @brief Enable Tx warning and Rx warning interrupts of the FlexCAN module
 *
 * It enables Tx warning interrupt and Rx warning interrupt.
 *
 * @param   instance     The FlexCAN instance number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_enable_tx_rx_warning_interrupt(uint8_t instance);

/*!
 * @brief Disable Tx warning and Rx warning interrupts of the FlexCAN module
 *
 * It disables Tx warning interrupt and Rx warning interrupt.
 *
 * @param   instance     The FlexCAN instance number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_disable_tx_rx_warning_interrupt(uint8_t instance);

/*!
 * @brief Un-freeze the FlexCAN module
 *
 * @param   instance     The FlexCAN instance number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_exit_freeze_mode(uint8_t instance);

/*!
 * @brief Freeze the FlexCAN module
 *
 * @param   instance     The FlexCAN instance number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_enter_freeze_mode(uint8_t instance);

/*!
 * @brief Get freeze ack.
 *
 * @param   instance     The FlexCAN instance number.
 * @return  freeze ack state (1-freeze mode, 0-not in freeze mode, otherwise failed).
 */
uint32_t flexcan_hal_get_freeze_ack(uint8_t instance);

/*!
 * @brief Get the individual FlexCAN MB interrupt flag
 *
 * @param   instance     The FlexCAN instance number.
 * @param   mb_idx      Index of the message buffer.
 * @return  the individual MB interrupt flag (0 and 1 are the flag value; otherwise failed).
 */
uint32_t flexcan_hal_get_mb_int_flag(
    uint8_t instance,
    uint32_t mb_idx);

/*!
 * @brief Get all FlexCAN MB interrupt flags.
 *
 * @param   instance     The FlexCAN instance number.
 * @return  all MB interrupt flags
 */
uint32_t flexcan_hal_get_all_mb_int_flags(uint8_t instance);

/*!
 * @brief Clear the interrupt flag of the message buffers
 *
 * @param   instance     The FlexCAN instance number.
 * @param   reg_val     The value to be writen to the interrupt flag1 register.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_clear_mb_int_flag(
    uint8_t instance,
    uint32_t reg_val);

/*!
 * @brief Get transmit error counter and receive error counter
 *
 * @param   instance     The FlexCAN instance number.
 * @param   err_cnt      Transmit error counter and receive error counter.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_get_err_counter(
    uint8_t instance,
    flexcan_berr_counter_t *err_cnt);

/*!
 * @brief Get error and status
 *
 * @param   instance     The FlexCAN instance number.
 * @return  The current error and status.
 */
uint32_t flexcan_hal_get_err_status(uint8_t instance);

/*!
 * @brief Clear all other interrupts in ERRSTAT register (Error, Busoff, Wakeup
 *
 * @param   instance     The FlexCAN instance number.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_clear_err_interrupt_status(uint8_t instance);

/*!
 * @brief Get FlexCAN RX FIFO data
 *
 * @param   instance     The FlexCAN instance number.
 * @param   rx_fifo      The FlexCAN receive FIFO data.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_read_fifo(
    uint8_t instance,
    flexcan_mb_t *rx_fifo);

/*!
 * @brief Set RX masking type
 *
 * @param   instance     The FlexCAN instance number.
 * @param   type         The FlexCAN RX mask type.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_set_mask_type(uint8_t instance, flexcan_rx_mask_type_t type);

/*!
 * @brief Set FlexCAN RX FIFO global standard mask.
 *
 * @param   instance     The FlexCAN instance number.
 * @param   std_mask     Standard mask.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_set_rx_fifo_global_std_mask(
    uint8_t instance,
    uint32_t std_mask);

/*!
 * @brief Set FlexCAN RX FIFO global extended mask.
 *
 * @param   instance     The FlexCAN instance number.
 * @param   ext_mask     Extended mask.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_set_rx_fifo_global_ext_mask(
    uint8_t instance,
    uint32_t ext_mask);

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
    uint32_t std_mask);

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
    uint32_t ext_mask);

/*!
 * @brief Get the FlexCAN ID acceptance filter hit indicator on Rx FIFO.
 *
 * @param   instance     The FlexCAN instance number.
 * @return  RX FIFO information.
 */
uint32_t  flexcan_hal_get_rx_fifo_id_acceptance_filter(uint8_t instance);

/*!
 * @brief Set FlexCAN RX MB global standard mask.
 *
 * @param   instance     The FlexCAN instance number.
 * @param   std_mask     Standard mask.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_set_rx_mb_global_std_mask(
    uint8_t instance,
    uint32_t std_mask);

/*!
 * @brief Set FlexCAN RX MB BUF14 standard mask.
 *
 * @param   instance     The FlexCAN instance number.
 * @param   std_mask     Standard mask.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_set_rx_mb_buf14_std_mask(
    uint8_t instance,
    uint32_t std_mask);

/*!
 * @brief Set FlexCAN RX MB BUF15 standard mask.
 *
 * @param   instance     The FlexCAN instance number.
 * @param   std_mask     Standard mask.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_set_rx_mb_buf15_std_mask(
    uint8_t instance,
    uint32_t std_mask);

/*!
 * @brief Set FlexCAN RX MB global extended mask.
 *
 * @param   instance     The FlexCAN instance number.
 * @param   ext_mask     Extended mask.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_set_rx_mb_global_ext_mask(
    uint8_t instance,
    uint32_t ext_mask);

/*!
 * @brief Set FlexCAN RX MB BUF14 extended mask.
 *
 * @param   instance     The FlexCAN instance number.
 * @param   ext_mask     Extended mask.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_set_rx_mb_buf14_ext_mask(
    uint8_t instance,
    uint32_t ext_mask);

/*!
 * @brief Set FlexCAN RX MB BUF15 extended mask.
 *
 * @param   instance     The FlexCAN instance number.
 * @param   ext_mask     Extended mask.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_set_rx_mb_buf15_ext_mask(
    uint8_t instance,
    uint32_t ext_mask);

/*!
 * @brief Enable an operation mode.
 *
 * @param   instance     The FlexCAN instance number.
 * @param   mode         An operation mode to be enabled.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_enable_operation_mode(
    uint8_t instance,
    flexcan_operation_modes_t mode);

/*!
 * @brief Disable an operation mode.
 *
 * @param   instance     The FlexCAN instance number.
 * @param   mode         An operation mode to be disabled.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_disable_operation_mode(
    uint8_t instance,
    flexcan_operation_modes_t mode);

#if PSP_MQX_CPU_IS_VYBRID
/*!
 * @brief Get all error interrupt flags.
 *
 * @param   instance     The FlexCAN instance number.
 * @return  all error interrupt flags
 */
uint32_t flexcan_hal_get_err_int_flags(uint8_t instance);

/*!
 * @brief Clear the error interrupt flags
 *
 * @param   instance     The FlexCAN instance number.
 * @param   reg_val     The value to be writen to the error status register.
 * @return  0 if successful; non-zero failed.
 */
uint32_t flexcan_hal_clear_err_int_flag(uint8_t instance);
#endif

//@}

#if defined(__cplusplus)
}
#endif

//! @}

#endif // __FSL_FLEXCAN_HAL_H__
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////

