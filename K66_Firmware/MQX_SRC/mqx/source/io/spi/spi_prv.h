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

#ifndef __spi_prv_h__
#define __spi_prv_h__

#include "spi.h"


/*--------------------------------------------------------------------------*/
/*
**                    DATATYPE DECLARATIONS
*/

/*
** Run time state information for SPI driver (shared for all fds)
*/
typedef struct spi_driver_data_struct
{
    /* LWSEM for locking for concurrent access from several tasks */
    LWSEM_STRUCT          BUS_LOCK;

    /* Pointer to low level driver */
    SPI_DEVIF_STRUCT_CPTR DEVIF;

    /* Pointer to runtime data specific for low level driver */
    void                 *DEVIF_DATA;

    /* Default transfer parameters for low level driver */
    SPI_PARAM_STRUCT      PARAMS;

    /* Callback function for external CS handling */
    SPI_CS_CALLBACK       CS_CALLBACK;

    /* Context passed to CS callback function */
    void                 *CS_USERDATA;

} SPI_DRIVER_DATA_STRUCT, * SPI_DRIVER_DATA_STRUCT_PTR;


/*
** Context information for open fd
*/
typedef struct spi_dev_data_struct
{
    /* Inidicates that BUS_LOCK is being held by this fd */
    bool               BUS_OWNER;

    /* Transfer parameters for low level driver */
    SPI_PARAM_STRUCT      PARAMS;

    /* Indicates necessity to re-set parameters before transfer */
    bool               PARAMS_DIRTY;

    /* Open flags for this channel */
    uint32_t               FLAGS;

#if BSPCFG_ENABLE_SPI_STATS
    /* Statistical information */
    SPI_STATISTICS_STRUCT STATS;
#endif

} SPI_DEV_DATA_STRUCT, * SPI_DEV_DATA_STRUCT_PTR;


#endif
