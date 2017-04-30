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
*   The file contains low level SPI driver functions for DSPI module
*
*
*END************************************************************************/


#include <mqx.h>
#include <bsp.h>
#include <io_prv.h>

#include "spi.h"
#include "spi_dspi_common.h"


static const uint32_t BAUDRATE_PRESCALER[] = { 2, 3, 5, 7 };
static const uint32_t BAUDRATE_SCALER[] = { 2, 4, 6, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768 };

static const uint32_t DELAY_PRESCALER[] = { 1, 3, 5, 7 };

/*FUNCTION****************************************************************
*
* Function Name    : _dspi_find_baudrate
* Returned Value   : Actual baudrate obtained by programming the CTAR register
* Comments         :
*    Find closest setting of divider register for given baudrate.
*    Value of CTAR register is modified via pointer.
*
*END*********************************************************************/

#define DSPI_CTAR_BAUDRATE_MASK ( \
      DSPI_CTAR_BR_MASK \
    | DSPI_CTAR_DT_MASK \
    | DSPI_CTAR_ASC_MASK \
    | DSPI_CTAR_CSSCK_MASK \
    | DSPI_CTAR_PBR_MASK \
    | DSPI_CTAR_PDT_MASK \
    | DSPI_CTAR_PASC_MASK \
    | DSPI_CTAR_PCSSCK_MASK \
    | DSPI_CTAR_DBR_MASK \
)

uint32_t _dspi_find_baudrate
    (
        /* [IN] Module input clock in Hz */
        uint32_t clock,

        /* [IN] Desired baudrate in Hz */
        uint32_t baudrate,

        /* [OUT] Timing parameters of CTAR register to configure use */
        uint32_t volatile *ctar_ptr
    )
{
    uint32_t pres, best_pres;
    uint32_t scaler, best_scaler;
    uint32_t dbr, best_dbr;
    uint32_t real_baudrate, best_baudrate;

    uint32_t delayrate;

    int32_t  diff;
    uint32_t min_diff;

    uint32_t ctar;

    /* find combination of prescaler and scaler resulting in baudrate closest to the requested value */
    min_diff = (uint32_t)-1;
    best_pres = 0;
    best_scaler = 0;
    best_dbr = 1;
    best_baudrate = 0; /* required to avoid compilation warning */
    for (pres = 0; (pres < 4) && min_diff; pres++)
    {
        for (scaler = 0; (scaler < 16) && min_diff; scaler++)
        {
            for (dbr = 1; (dbr < 3) && min_diff; dbr++)
            {
                real_baudrate = ((clock * dbr) / (BAUDRATE_PRESCALER[pres] * (BAUDRATE_SCALER[scaler])));

                diff = baudrate - real_baudrate;
                if (diff < 0)
                    diff = -diff;

                if (min_diff > diff)
                {
                    /* a better match found */
                    min_diff = diff;
                    best_pres = pres;
                    best_scaler = scaler;
                    best_baudrate = real_baudrate;
                    best_dbr = dbr;
                }
            }
        }
    }

    if (ctar_ptr == NULL)
        return best_baudrate;

    /* store baudrate scaler and prescaler settings to ctar */
    ctar = DSPI_CTAR_PBR(best_pres) | DSPI_CTAR_BR(best_scaler);
    ctar |= ((best_dbr - 1) * DSPI_CTAR_DBR_MASK);

    /* similar lookup for delay prescalers */
    min_diff = (uint32_t)-1;
    delayrate = baudrate * 4; /* double the baudrate (half period delay is sufficient) and divisor is (2<<scaler), thus times 4 */
    /* initialize prescaler and scaler to slowest possible rate to cover the case when no better match is found */
    best_pres = ELEMENTS_OF(BAUDRATE_PRESCALER)-1;
    best_scaler = ELEMENTS_OF(BAUDRATE_SCALER)-1;
    for (pres = 0; (pres < 4) && min_diff; pres++)
    {
        for (scaler = 0; scaler < 16; scaler++)
        {
            diff = delayrate - ((clock / DELAY_PRESCALER[pres])>>scaler);
            if (diff >= 0) {  /* To ensure that the delay time is larger than half period of baudrate */
               if (min_diff > diff)
               {
                  /* a better match found */
                  min_diff = diff;
                  best_pres = pres;
                  best_scaler = scaler;
               }
               else break; /* higher scaler value cannot provide with better match */
            }
        }
    }

    /* add delay scalers and prescaler settings to ctar */
    ctar |= DSPI_CTAR_CSSCK(best_scaler) | DSPI_CTAR_PCSSCK(best_pres);
    ctar |= DSPI_CTAR_ASC(best_scaler) | DSPI_CTAR_PASC(best_pres);

    /* CS controlled by sw, assuming it is slow enough */
    //ctar |= DSPI_CTAR_DT(best_scaler) | DSPI_CTAR_PDT(best_pres);

    *ctar_ptr = (*ctar_ptr & ~(uint32_t)DSPI_CTAR_BAUDRATE_MASK) | ctar;

    return best_baudrate;
}


/*FUNCTION****************************************************************
*
* Function Name    : _dspi_ctar_params
* Returned Value   :
* Comments         :
*    Alters CTAR value according to SPI parameters.
*
*END*********************************************************************/

#define DSPI_CTAR_PARAMS_MASK (DSPI_CTAR_LSBFE_MASK | DSPI_CTAR_CPHA_MASK | DSPI_CTAR_CPOL_MASK | DSPI_CTAR_FMSZ_MASK)

_mqx_int _dspi_ctar_params
   (
        /* [IN] Parameters to set */
        SPI_PARAM_STRUCT_PTR params,

        /* [IN] Timing parameters of CTAR register to configure use */
        uint32_t volatile *ctar_ptr

   )
{
    uint32_t ctar = 0;

    if (ctar_ptr == NULL)
        return MQX_INVALID_POINTER;

    /* Set up SPI clock polarity and phase */
    if (params->MODE & SPI_CPHA_MASK)
        ctar |= DSPI_CTAR_CPHA_MASK;

    if (params->MODE & SPI_CPOL_MASK)
        ctar |= DSPI_CTAR_CPOL_MASK;

    /* Set up frame size */
    if (params->FRAMESIZE < 3 || params->FRAMESIZE > 16)
        return SPI_ERROR_FRAMESIZE_INVALID;
    ctar |= DSPI_CTAR_FMSZ(params->FRAMESIZE - 1);

    /* Endianess */
    if ((params->ATTR & SPI_ATTR_ENDIAN_MASK) == SPI_ATTR_LITTLE_ENDIAN)
        ctar |= DSPI_CTAR_LSBFE_MASK;

    *ctar_ptr = (*ctar_ptr & ~(uint32_t)DSPI_CTAR_PARAMS_MASK) | ctar;

    return SPI_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : _dspi_init_low
* Returned Value   :
* Comments         :
*    This function initializes the DSPI module and resets it to default state
*
*END*********************************************************************/
_mqx_int _dspi_init_low
    (
        /* [IN] Pointer to DSPI peripheral */
        VDSPI_REG_STRUCT_PTR dspi_ptr
    )
{
    /* Put DSPI into enabled but halted state */
    dspi_ptr->MCR = DSPI_MCR_HALT_MASK;

    /* Clear FIFOs */
    dspi_ptr->MCR = DSPI_MCR_CLR_TXF_MASK | DSPI_MCR_CLR_RXF_MASK;

    #if DSPI_FIFO_DEPTH == 1
    /* Disable FIFO */
    dspi_ptr->MCR |= DSPI_MCR_DIS_RXF_MASK | DSPI_MCR_DIS_TXF_MASK;
    #endif

    /* Receive FIFO overflow disable */
    dspi_ptr->MCR |= DSPI_MCR_ROOE_MASK;

    /* Set CS0-7 inactive high */
    dspi_ptr->MCR |= DSPI_MCR_PCSIS(0xFF);

    /* Disable interrupts */
    dspi_ptr->RSER = 0;

    /* Clear all flags */
    dspi_ptr->SR = ~ DSPI_SR_TFFF_MASK;

    /* Enable SPI */
    dspi_ptr->MCR &= (~DSPI_MCR_HALT_MASK);

    return SPI_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : _dspi_deinit_low
* Returned Value   :
* Comments         :
*    This function de-initializes the DSPI module by disabling it
*
*END*********************************************************************/
_mqx_int _dspi_deinit_low
    (
        /* [IN] Pointer to DSPI peripheral */
        VDSPI_REG_STRUCT_PTR dspi_ptr
    )
{
    /* Halt and disable the DSPI */
    dspi_ptr->MCR |= DSPI_MCR_HALT_MASK | DSPI_MCR_MDIS_MASK;

    /* Disable interrupts for sure */
    dspi_ptr->RSER = 0;

    return SPI_OK;
}
