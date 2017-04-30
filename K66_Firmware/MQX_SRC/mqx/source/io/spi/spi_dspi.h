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
*   This file contains the definitions of constants and structures
*   required for the DSPI driver
*
*
*END************************************************************************/

#ifndef __spi_dspi_h__
#define __spi_dspi_h__

#include <bsp.h>


/*--------------------------------------------------------------------------*/
/*
**                    CONSTANT DEFINITIONS
*/

#define DSPI_CS_COUNT       8

#define DSPI_ATTR_USE_ISR   (1u<<31)


/*--------------------------------------------------------------------------*/
/*
**                    DATATYPE DECLARATIONS
*/

/*
** DSPI_INIT_STRUCT
**
** This structure defines the initialization parameters to be used
** when a spi port is initialized.
*/
typedef struct dspi_init_struct
{
   /* SPI channel number */
   uint32_t CHANNEL;

   /* The input clock source of the module */
   CM_CLOCK_SOURCE CLOCK_SOURCE;

} DSPI_INIT_STRUCT, * DSPI_INIT_STRUCT_PTR;

typedef const DSPI_INIT_STRUCT * DSPI_INIT_STRUCT_CPTR;


/*----------------------------------------------------------------------*/
/*
**              DEFINED VARIABLES
*/

extern const SPI_DEVIF_STRUCT _spi_dspi_devif;


/*--------------------------------------------------------------------------*/
/*
**                        FUNCTION PROTOTYPES
*/

#ifdef __cplusplus
extern "C" {
#endif

extern void     *_bsp_get_dspi_base_address(uint8_t);
extern uint32_t   _bsp_get_dspi_vectors(uint32_t dev_num, const uint32_t  **vectors_ptr);
extern bool   _bsp_dspi_enable_access(uint32_t device);

#ifdef __cplusplus
}
#endif

#endif
