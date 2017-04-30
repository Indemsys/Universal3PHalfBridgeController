/*HEADER**********************************************************************
*
* Copyright 2012 Freescale Semiconductor, Inc.
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
*   This file contains definitions private to the SPI driver.
*
*
*END************************************************************************/

#ifndef __spi_dspi_prv_h__
#define __spi_dspi_prv_h__

#include "spi.h"
#include "spi_dspi_common.h"


/*--------------------------------------------------------------------------*/
/*
**                    DATATYPE DECLARATIONS
*/

/*
** DSPI_INFO_STRUCT
** Run time state information for each spi channel
*/
typedef struct dspi_info_struct
{
    /* SPI channel number */
    uint32_t                           CHANNEL;

    /* The input clock source of the module */
    CM_CLOCK_SOURCE                   CLOCK_SOURCE;

    /* Most recently used clock configuration (cached value) */
    BSP_CLOCK_CONFIGURATION           CLOCK_CONFIG;

    /* Most recently used baudrate */
    uint32_t                           BAUDRATE;

    /* Most recently calculated timing parameters for CTAR register */
    uint32_t                           CTAR_TIMING;

    /* The spi device registers */
    VDSPI_REG_STRUCT_PTR              DSPI_PTR;

    /* Pattern to transmit during half-duplex rx transfer */
    uint32_t                           DUMMY_PATTERN;

    /* Additional attributes for the transfer */
    uint32_t                           ATTR;

    /* Pointer to the buffer to use for RX data in ISR */
    uint8_t                        *RX_BUF;

    /* Remaining RX byte count for ISR */
    uint32_t                           RX_LEN;

    /* Pointer to the buffer to use for TX data in ISR*/
    uint8_t                        *TX_BUF;

    /* Remaining TX byte count for ISR */
    uint32_t                           TX_LEN;

    /* Event to signal that background job is done */
    LWSEM_STRUCT                      EVENT_IO_FINISHED;

    /* Number of interrupt vectors supported by the peripheral */
    uint32_t                           NUM_VECTORS;

} DSPI_INFO_STRUCT, * DSPI_INFO_STRUCT_PTR;


/*--------------------------------------------------------------------------*/
/*
**                        FUNCTION PROTOTYPES
*/

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif
