/*HEADER**********************************************************************
*
* Copyright 2009 Freescale Semiconductor, Inc.
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
*   The file contains low level eSDHC driver functions.
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>
#include <io_prv.h>
#include <string.h>
#include "esdhc.h"
#include "esdhc_prv.h"

static void _esdhc_isr(void *parameter);

/*FUNCTION****************************************************************
*
* Function Name    : _esdhc_set_baudrate_low
* Returned Value   : None
* Comments         :
*    Find and set closest divider values for given baudrate and sets related registers.
*
*END*********************************************************************/
static void _esdhc_set_baudrate_low
    (
        /* [IN/OUT] Module registry pointer */
        ESDHC_REG_STRUCT_PTR esdhc_ptr,

        /* [IN] Module input clock in Hz */
        uint32_t             clock,

        /* [IN] Desired baudrate in Hz */
        uint32_t             baudrate
    )
{
    uint32_t pres, div, min, minpres = 0x80, mindiv = 0x0F;
    int32_t  val;

    /* Find closest setting */
    min = (uint32_t)-1;
    for (pres = 2; pres <= 256; pres <<= 1)
    {
        for (div = 1; div <= 16; div++)
        {
            val = pres * div * baudrate - clock;
            if (val >= 0)
            {
                if (min > val)
                {
                    min = val;
                    minpres = pres;
                    mindiv = div;
                }
            }
        }
    }

    /* Disable ESDHC clocks */
    esdhc_ptr->SYSCTL &= (~ SDHC_SYSCTL_SDCLKEN_MASK);

    /* Change dividers */
    div = esdhc_ptr->SYSCTL & (~ (SDHC_SYSCTL_DTOCV_MASK | SDHC_SYSCTL_SDCLKFS_MASK | SDHC_SYSCTL_DVS_MASK));
    esdhc_ptr->SYSCTL = div | (SDHC_SYSCTL_DTOCV(0x0E) | SDHC_SYSCTL_SDCLKFS(minpres >> 1) | SDHC_SYSCTL_DVS(mindiv - 1));

    /* Wait for stable clock */
    while (0 == (esdhc_ptr->PRSSTAT & SDHC_PRSSTAT_SDSTB_MASK))
    {
        /* Workaround... */
        _time_delay (BSP_ALARM_RESOLUTION);

        /* Not every controller has this bit */
#if defined (BSP_M53015EVB) || (BSP_MPC8377RDB)
        break;
#endif
    };

    /* Enable ESDHC clocks */
    esdhc_ptr->SYSCTL |= SDHC_SYSCTL_SDCLKEN_MASK;
    esdhc_ptr->IRQSTAT |= SDHC_IRQSTAT_DTOE_MASK;
}


/*FUNCTION****************************************************************
*
* Function Name    : _esdhc_set_baudrate
* Returned Value   : None
* Comments         :
*    Find and set closest divider values for given baudrate and also manages
*    the external pins to avoid any glitches.
*
*END*********************************************************************/
static int32_t _esdhc_set_baudrate
    (
        /* [IN] Device runtime information */
        ESDHC_DEVICE_STRUCT_PTR esdhc_device_ptr,

        /* [IN] Desired baudrate in Hz */
        uint32_t                baudrate
    )
{
    ESDHC_REG_STRUCT_PTR   esdhc_ptr = esdhc_device_ptr->ESDHC_REG_PTR;
    ESDHC_INIT_STRUCT_CPTR esdhc_init_ptr = esdhc_device_ptr->INIT;

    /* De-Init GPIO */
    if (_bsp_esdhc_io_init(esdhc_init_ptr->CHANNEL, 0) == -1)
    {
        return IO_ERROR_DEVICE_INVALID;
    }

    /* Set closest baudrate */
    _esdhc_set_baudrate_low(esdhc_ptr, esdhc_init_ptr->CLOCK_SPEED, baudrate);

    /* Init GPIO */
    if (_bsp_esdhc_io_init(esdhc_init_ptr->CHANNEL, 0xFFFF) == -1)
    {
        return IO_ERROR_DEVICE_INVALID;
    }

    return ESDHC_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : _esdhc_get_baudrate_low
* Returned Value   : current baudrate sets in peripheral
* Comments         :
*    Return current baudrate of ESDHC peripheral (reads out dividers from registers)
*
*END*********************************************************************/
static uint32_t _esdhc_get_baudrate_low
    (
        /* [IN] Module registry pointer */
        ESDHC_REG_STRUCT_PTR esdhc_ptr,

        /* [IN] Module input clock in Hz */
        uint32_t             clock
    )
{
    int32_t div;

    /* Get total divider */
    div = ((esdhc_ptr->SYSCTL & SDHC_SYSCTL_SDCLKFS_MASK) >> SDHC_SYSCTL_SDCLKFS_SHIFT) << 1;
    div *= ((esdhc_ptr->SYSCTL & SDHC_SYSCTL_DVS_MASK) >> SDHC_SYSCTL_DVS_SHIFT) + 1;

    return (clock/div);
}


/*FUNCTION****************************************************************
*
* Function Name    : _esdhc_get_baudrate
* Returned Value   : current baudrate sets in peripheral
* Comments         :
*    Return current baudrate of ESDHC peripheral.
*
*END*********************************************************************/
static uint32_t _esdhc_get_baudrate
    (
        /* [IN] Device runtime information */
        ESDHC_DEVICE_STRUCT_PTR esdhc_device_ptr
    )
{
    ESDHC_REG_STRUCT_PTR   esdhc_ptr = esdhc_device_ptr->ESDHC_REG_PTR;
    ESDHC_INIT_STRUCT_CPTR esdhc_init_ptr = esdhc_device_ptr->INIT;

    return _esdhc_get_baudrate_low(esdhc_ptr, esdhc_init_ptr->CLOCK_SPEED);
}


/*FUNCTION****************************************************************
*
* Function Name    : _esdhc_is_running
* Returned Value   : TRUE if running, FALSE otherwise
* Comments         :
*    Checks whether eSDHC module is currently in use.
*
*END*********************************************************************/
static bool _esdhc_is_running
    (
        /* [IN] Module registry pointer */
        ESDHC_REG_STRUCT_PTR esdhc_ptr
    )
{
    return (0 != (esdhc_ptr->PRSSTAT & (SDHC_PRSSTAT_RTA_MASK | SDHC_PRSSTAT_WTA_MASK | SDHC_PRSSTAT_DLA_MASK | SDHC_PRSSTAT_CDIHB_MASK | SDHC_PRSSTAT_CIHB_MASK)));
}


#if 0
/*FUNCTION****************************************************************
*
* Function Name    : _esdhc_status_wait
* Returned Value   : bits set for given mask
* Comments         :
*    Waits for ESDHC interrupt status register bits according to given mask.
*
*END*********************************************************************/
static uint32_t _esdhc_status_wait
    (
        /* [IN] Module registry pointer */
        ESDHC_REG_STRUCT_PTR esdhc_ptr,

        /* [IN] Mask of IRQSTAT bits to wait for */
        uint32_t         mask
    )
{
    uint32_t             result;
    do
    {
        result = esdhc_ptr->IRQSTAT & mask;
    }
    while (0 == result);
    return result;
}
#endif


/*FUNCTION****************************************************************
*
* Function Name    : _esdhc_init
* Returned Value   : MQX error code
* Comments         :
*    ESDHC registers initialization and card detection.
*
*END*********************************************************************/
static int32_t _esdhc_init
    (
        /* [IN/OUT] Device runtime information */
        ESDHC_DEVICE_STRUCT_PTR  esdhc_device_ptr
    )
{
    ESDHC_REG_STRUCT_PTR   esdhc_ptr = esdhc_device_ptr->ESDHC_REG_PTR;
    ESDHC_INIT_STRUCT_CPTR esdhc_init_ptr = esdhc_device_ptr->INIT;

    esdhc_device_ptr->CARD = ESDHC_CARD_NONE;

    /* De-init GPIO - to prevent unwanted clocks on bus */
    if (_bsp_esdhc_io_init(esdhc_init_ptr->CHANNEL, 0) == -1)
    {
        return IO_ERROR_DEVICE_INVALID;
    }

    /* Reset ESDHC */
    esdhc_ptr->SYSCTL = SDHC_SYSCTL_RSTA_MASK | SDHC_SYSCTL_SDCLKFS(0x80);

    while (esdhc_ptr->SYSCTL & SDHC_SYSCTL_RSTA_MASK)
        { };

    /* Just to keep card detection working */
    esdhc_ptr->SYSCTL = SDHC_SYSCTL_PEREN_MASK;

    /* Initial values */
    esdhc_ptr->VENDOR = 0; /* Switch off the external DMA requests */

    esdhc_ptr->BLKATTR = SDHC_BLKATTR_BLKCNT(1) | SDHC_BLKATTR_BLKSIZE(512); /* Set any default size of data block */

#if PSP_ENDIAN == MQX_BIG_ENDIAN
    esdhc_ptr->PROCTL = SDHC_PROCTL_EMODE(ESDHC_PROCTL_EMODE_BIG) | SDHC_PROCTL_D3CD_MASK | SDHC_PROCTL_DMAS(2);
#else
    esdhc_ptr->PROCTL = SDHC_PROCTL_EMODE(ESDHC_PROCTL_EMODE_LITTLE) | SDHC_PROCTL_D3CD_MASK | SDHC_PROCTL_DMAS(2);
#endif

    esdhc_ptr->WML = SDHC_WML_RDWML(1) | SDHC_WML_WRWML(1); /* To do: set up the right value */

    /* Set the ESDHC initial baud rate divider and start */
    _esdhc_set_baudrate_low(esdhc_ptr, esdhc_init_ptr->CLOCK_SPEED, ESDHC_INIT_BAUDRATE);

    /* Poll inhibit bits */
    while (esdhc_ptr->PRSSTAT & (SDHC_PRSSTAT_CIHB_MASK | SDHC_PRSSTAT_CDIHB_MASK))
        { };

    /* Init GPIO again */
    if (_bsp_esdhc_io_init(esdhc_init_ptr->CHANNEL, 0xFFFF) == -1)
    {
        return IO_ERROR_DEVICE_INVALID;
    }

    /* Enable requests */
    esdhc_ptr->IRQSTAT = 0xFFFF;


    esdhc_ptr->IRQSTATEN =  SDHC_IRQSTATEN_DMAESEN_MASK         // DMA Error Status Enable
                          | SDHC_IRQSTATEN_AC12ESEN_MASK        // Auto CMD12 Error Status Enable
                          | SDHC_IRQSTATEN_DEBESEN_MASK         // Data End Bit Error Status Enable
                          | SDHC_IRQSTATEN_DCESEN_MASK          // Data CRC Error Status Enable
                          | SDHC_IRQSTATEN_DTOESEN_MASK         // Data Timeout Error Status Enable
                          | SDHC_IRQSTATEN_CIESEN_MASK          // Command Index Error Status Enable
                          | SDHC_IRQSTATEN_CEBESEN_MASK         // Command End Bit Error Status Enable
                          | SDHC_IRQSTATEN_CCESEN_MASK          // Command CRC Error Status Enable
                          | SDHC_IRQSTATEN_CTOESEN_MASK         // Command Timeout Error Status Enable
                          //| SDHC_IRQSTATEN_CINTSEN_MASK         // Card Interrupt Status Enable
                          | SDHC_IRQSTATEN_CRMSEN_MASK          // Card Removal Status Enable
                          | SDHC_IRQSTATEN_CINSEN_MASK          // Card Insertion Status Enable
                          //| SDHC_IRQSTATEN_BRRSEN_MASK          // Buffer Read Ready Status Enable
                          //| SDHC_IRQSTATEN_BWRSEN_MASK          // Buffer Write Ready Status Enable
                          | SDHC_IRQSTATEN_DINTSEN_MASK         // DMA Interrupt Status Enable
                          | SDHC_IRQSTATEN_BGESEN_MASK          // Block Gap Event Status Enable
                          | SDHC_IRQSTATEN_TCSEN_MASK           // Transfer Complete Status Enable
                          | SDHC_IRQSTATEN_CCSEN_MASK;          // Command Complete Status Enable

    /* 80 initial clocks */
    esdhc_ptr->SYSCTL |= SDHC_SYSCTL_INITA_MASK;
    while (esdhc_ptr->SYSCTL & SDHC_SYSCTL_INITA_MASK)
        { };

    /* Check card */
    if (esdhc_ptr->PRSSTAT & SDHC_PRSSTAT_CINS_MASK)
    {
        esdhc_device_ptr->CARD = ESDHC_CARD_UNKNOWN;
    }



    esdhc_ptr->IRQSIGEN =   SDHC_IRQSIGEN_DMAEIEN_MASK          // DMA Error Status Enable
                          | SDHC_IRQSIGEN_AC12EIEN_MASK        // Auto CMD12 Error Status Enable
                          | SDHC_IRQSIGEN_DEBEIEN_MASK         // Data End Bit Error Status Enable
                          | SDHC_IRQSIGEN_DCEIEN_MASK          // Data CRC Error Status Enable
                          | SDHC_IRQSIGEN_DTOEIEN_MASK         // Data Timeout Error Status Enable
                          | SDHC_IRQSIGEN_CIEIEN_MASK          // Command Index Error Status Enable
                          | SDHC_IRQSIGEN_CEBEIEN_MASK         // Command End Bit Error Status Enable
                          | SDHC_IRQSIGEN_CCEIEN_MASK          // Command CRC Error Status Enable
                          | SDHC_IRQSIGEN_CTOEIEN_MASK         // Command Timeout Error Status Enable
                          //| SDHC_IRQSIGEN_CINTIEN_MASK         // Card Interrupt Status Enable
#if ESDHC_CARD_DETECTION_SUPPORT
                          | SDHC_IRQSIGEN_CRMIEN_MASK          // Card Removal Status Enable
                          | SDHC_IRQSIGEN_CINSIEN_MASK         // Card Insertion Status Enable
#endif
                          //| SDHC_IRQSIGEN_BRRIEN_MASK          // Buffer Read Ready Status Enable
                          //| SDHC_IRQSIGEN_BWRIEN_MASK          // Buffer Write Ready Status Enable
                          | SDHC_IRQSIGEN_DINTIEN_MASK         // DMA Interrupt Status Enable
                          | SDHC_IRQSIGEN_BGEIEN_MASK          // Block Gap Event Status Enable
                          | SDHC_IRQSIGEN_TCIEN_MASK           // Transfer Complete Status Enable
                          | SDHC_IRQSIGEN_CCIEN_MASK;          // Command Complete Status Enable

    _bsp_int_init(esdhc_device_ptr->VECTOR, BSP_ESDHC_INT_LEVEL, 0, TRUE);

    return ESDHC_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : _esdhc_send_command
* Returned Value   : 0 on success, 1 on error, -1 on timeout
* Comments         :
*    One ESDHC command transaction.
*
*END*********************************************************************/
static int32_t _esdhc_send_command
    (
        /* [IN/OUT] SDHC device pointer */
        ESDHC_DEVICE_STRUCT_PTR         esdhc_device_ptr,

        /* [IN/OUT] Command specification */
        ESDHC_COMMAND_STRUCT_PTR        command,

        /* [IN] Source/Destination data address for data transfers */
        ESDHC_ADMA2_DESC               *adma2_desc_ptr
    )
{
    uint32_t                   xfertyp;
    uint32_t                   blkattr;
    uint32_t                   mask;
    uint32_t                   temp;
    MQX_TICK_STRUCT            start_tick;
    MQX_TICK_STRUCT            end_tick;
    bool                       overflow;
    ESDHC_REG_STRUCT_PTR       esdhc_ptr = esdhc_device_ptr->ESDHC_REG_PTR;

    /* Create the command (xfertyp & blkattr) */

    /* Is it necessary to send the application command? */
    if(command->COMMAND & ESDHC_COMMAND_ACMD_FLAG)
    {
        /* Send the CMD55 to enable next command from application group ACMDxx */
        xfertyp = (SDHC_XFERTYP_CMDINX(55) | SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(2));

          /* Wait for cmd line idle */
          while (esdhc_ptr->PRSSTAT & SDHC_PRSSTAT_CIHB_MASK)
              { };

          /* Setup command */
          esdhc_ptr->CMDARG = 0;
          esdhc_ptr->BLKATTR = 0;

          /* Clear the events and issue the command in a critical section*/
          _bsp_int_disable(esdhc_device_ptr->VECTOR);

          _lwevent_clear(&esdhc_device_ptr->LWEVENT, (ESDHC_LWEVENT_CMD_DONE | ESDHC_LWEVENT_CMD_ERROR | ESDHC_LWEVENT_CMD_TIMEOUT));
          esdhc_ptr->XFERTYP = xfertyp;

          /* Exit critical section */
          _bsp_int_enable(esdhc_device_ptr->VECTOR);

          if (_lwevent_wait_ticks(&esdhc_device_ptr->LWEVENT, (ESDHC_LWEVENT_CMD_DONE | ESDHC_LWEVENT_CMD_ERROR | ESDHC_LWEVENT_CMD_TIMEOUT), FALSE, ESDHC_CMD_TICK_TIMEOUT) != MQX_OK)
          {
              /* Something is wrong - timeout or any error of light weight event */
              return -1;
          }
          else
          {
              mask = _lwevent_get_signalled();

              if(mask & (ESDHC_LWEVENT_CMD_ERROR | ESDHC_LWEVENT_CMD_TIMEOUT))
                  return -1;
          }
    }

    /* Set the command index */
    xfertyp = SDHC_XFERTYP_CMDINX(command->COMMAND & ESDHC_COMMAND_CMDIX_MASK);

    /* DPSEL shall always be set for resume type commands */
    if(((command->COMMAND & ESDHC_COMMAND_CMDTYPE_MASK) == ESDHC_COMMAND_TYPE_RESUME) || (command->COMMAND & ESDHC_COMMAND_DATACMD_FLAG))
      xfertyp |= (SDHC_XFERTYP_DPSEL_MASK | SDHC_XFERTYP_DMAEN_MASK);

    if ((0 != command->BLOCKS) && (0 != command->BLOCKSIZE))
    {
        xfertyp |= (SDHC_XFERTYP_DPSEL_MASK | SDHC_XFERTYP_DMAEN_MASK);
        if (command->BLOCKS != 1)
        {
            /* Multiple block transfer */
            xfertyp |= SDHC_XFERTYP_MSBSEL_MASK;

            if (command->BLOCKS > 1)
              xfertyp |= (SDHC_XFERTYP_AC12EN_MASK | SDHC_XFERTYP_BCEN_MASK);
        }
        if ((uint32_t)-1 == command->BLOCKS)
        {
            /* Infinite transfer */
            blkattr = SDHC_BLKATTR_BLKSIZE(command->BLOCKSIZE) | SDHC_BLKATTR_BLKCNT(0xFFFF);
            xfertyp |= SDHC_XFERTYP_BCEN_MASK;
        }
        else
        {
            blkattr = SDHC_BLKATTR_BLKSIZE(command->BLOCKSIZE) | SDHC_BLKATTR_BLKCNT(command->BLOCKS);
        }
    }
    else
    {
        blkattr = 0;
    }

    if((xfertyp & SDHC_XFERTYP_DPSEL_MASK) && (command->COMMAND & ESDHC_COMMAND_DATA_READ_FLAG))
        xfertyp  |= SDHC_XFERTYP_DTDSEL_MASK;

    /* Set up response type */
    if(command->COMMAND & (ESDHC_COMMAND_RESPONSE_R1 | ESDHC_COMMAND_RESPONSE_R5 | ESDHC_COMMAND_RESPONSE_R6))
        xfertyp |= SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(2);
    else if(command->COMMAND & ESDHC_COMMAND_RESPONSE_R2)
        xfertyp |= SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(1);
    else if(command->COMMAND & (ESDHC_COMMAND_RESPONSE_R3 | ESDHC_COMMAND_RESPONSE_R4))
        xfertyp |= SDHC_XFERTYP_RSPTYP(2);
    else if(command->COMMAND & (ESDHC_COMMAND_RESPONSE_R1b | ESDHC_COMMAND_RESPONSE_R5b))
        xfertyp |= SDHC_XFERTYP_CICEN_MASK | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_RSPTYP(3);

    /* Set up the command type */
    xfertyp |= SDHC_XFERTYP_CMDTYP((command->COMMAND & ESDHC_COMMAND_CMDTYPE_MASK) >> ESDHC_COMMAND_CMDTYPE_SHIFT);

    /* Wait for cmd line idle */
    _time_get_ticks(&start_tick);

    while (esdhc_ptr->PRSSTAT & SDHC_PRSSTAT_CIHB_MASK)
    {
        _time_get_ticks(&end_tick);

        if (ESDHC_TRANSFER_TIMEOUT_MS < _time_diff_milliseconds(&end_tick, &start_tick, &overflow))
        {
            return -1;
        }

        /* Let chance to other tasks */
        _sched_yield();
    }

    /*
    The host driver can issue CMD0, CMD12, CMD13 (for
    memory) and CMD52 (for SDIO) when the DAT lines are busy
    during a data transfer. These commands can be issued when
    Command Inhibit (CIHB) is set to zero. Other commands shall
    be issued when Command Inhibit (CDIHB) is set to zero.
    Possible changes to the SD Physical Specification may add
    other commands to this list in the future.
    */

    /* Wait for data line idle if neccesary */
    temp = (command->COMMAND & ESDHC_COMMAND_CMDIX_MASK);

    if((temp != 0) && (temp != 12) && (temp != 13) && (temp != 52))
    {
        _time_get_ticks(&start_tick);

        while (esdhc_ptr->PRSSTAT & SDHC_PRSSTAT_CDIHB_MASK)
        {
            _time_get_ticks(&end_tick);

            if(ESDHC_TRANSFER_TIMEOUT_MS < _time_diff_milliseconds(&end_tick, &start_tick, &overflow))
            {
               return -1;
            }

            /* Enable clock to SD card to allow finish the operation */
            esdhc_ptr->SYSCTL |= (SDHC_SYSCTL_PEREN_MASK | SDHC_SYSCTL_HCKEN_MASK | SDHC_SYSCTL_IPGEN_MASK);

           /* Let other tasks to do their job meanwhile */
           _sched_yield();
        }
    }

#if ESDHC_AUTO_CLK_GATING
    /*
    If a response with busy is expected or auto command CMD12 (also expects busy with response)
    then force the clock the SD card to allow card finish the operation after the transfer
    otherwise use automatic clock gating.
    */
    if (((xfertyp & SDHC_XFERTYP_RSPTYP_MASK) == SDHC_XFERTYP_RSPTYP(3)) || (xfertyp & SDHC_XFERTYP_AC12EN_MASK))
    {
        esdhc_ptr->SYSCTL |= (SDHC_SYSCTL_PEREN_MASK | SDHC_SYSCTL_HCKEN_MASK | SDHC_SYSCTL_IPGEN_MASK);
    }
    else
    {
        esdhc_ptr->SYSCTL &= ~(SDHC_SYSCTL_PEREN_MASK | SDHC_SYSCTL_HCKEN_MASK | SDHC_SYSCTL_IPGEN_MASK);
    }
#endif

    /* Setup command */
    esdhc_ptr->DSADDR = 0;
    esdhc_ptr->ADSADDR = (uint32_t)adma2_desc_ptr;
    esdhc_ptr->BLKATTR = blkattr;
    esdhc_ptr->CMDARG = command->ARGUMENT;

    /* Clear the events and do the command in critical section*/
    _bsp_int_disable(esdhc_device_ptr->VECTOR);

    _lwevent_clear(&esdhc_device_ptr->LWEVENT, (ESDHC_LWEVENT_CMD_DONE | ESDHC_LWEVENT_CMD_ERROR | ESDHC_LWEVENT_CMD_TIMEOUT));

    /* Issue command */
    esdhc_ptr->XFERTYP = xfertyp;

    /* Exit critical section */
    _bsp_int_enable(esdhc_device_ptr->VECTOR);

    /* When multiple block write is stopped by CMD12, the busy from the response of CMD12 is up to 500ms. */
    if((command->COMMAND & ESDHC_COMMAND_CMDIX_MASK) == 12)
        temp = ESDHC_CMD12_TICK_TIMEOUT;
    else
        temp = ESDHC_CMD_TICK_TIMEOUT;

    if (_lwevent_wait_ticks(&esdhc_device_ptr->LWEVENT, (ESDHC_LWEVENT_CMD_DONE | ESDHC_LWEVENT_CMD_ERROR  | ESDHC_LWEVENT_CMD_TIMEOUT), FALSE, temp) != MQX_OK)
    {
        /* Something is wrong - timeout or any error of light weight event */
        return IO_ERROR_TIMEOUT;
    }
    else
    {
        mask = _lwevent_get_signalled();

        if(mask & ESDHC_LWEVENT_CMD_ERROR)
        {
            return ESDHC_ERROR_COMMAND_FAILED;
        }
        else if(mask & ESDHC_LWEVENT_CMD_TIMEOUT)
        {
            return ESDHC_ERROR_COMMAND_TIMEOUT;
        }
    }

    if (command->COMMAND & ESDHC_COMMAND_CMDRESPONSE_MASK)
    {
        command->RESPONSE[0] = esdhc_ptr->CMDRSP[0];
        if ((xfertyp & SDHC_XFERTYP_RSPTYP_MASK) == SDHC_XFERTYP_RSPTYP(1))
        {
            command->RESPONSE[1] = esdhc_ptr->CMDRSP[1];
            command->RESPONSE[2] = esdhc_ptr->CMDRSP[2];
            command->RESPONSE[3] = esdhc_ptr->CMDRSP[3];
        }
    }

    return 0;
}


/*!
 * \brief Wait for the card to finish the operation
 *
 * \param[in] esdhc_device_ptr  Device runtime information
 *
 * \return ESDHC_OK on success, ESDHC_ERR on error
 */
static int32_t _esdhc_wait_while_busy(
    /* [IN/OUT] SDHC device pointer */
    ESDHC_DEVICE_STRUCT_PTR esdhc_device_ptr)
{
    ESDHC_REG_STRUCT_PTR esdhc_ptr = esdhc_device_ptr->ESDHC_REG_PTR;
    int result = ESDHC_OK;

    MQX_TICK_STRUCT start_tick, end_tick;
    bool overflow;

    _time_get_ticks(&start_tick);

    /* while card busy, errata e4624: test DLSL[0] instead of DLA */
    while (!(esdhc_ptr->PRSSTAT & SDHC_PRSSTAT_DLSL(1)))
    {
        if (!(esdhc_ptr->PRSSTAT & SDHC_PRSSTAT_CINS_MASK))
        {
            /* card removed */
            break;
        }

        _time_get_ticks(&end_tick);
        if (ESDHC_TRANSFER_TIMEOUT_MS < _time_diff_milliseconds(&end_tick, &start_tick, &overflow))
        {
            result = IO_ERROR_TIMEOUT;
            break;
        }

        /* Enable clock to SD card to allow it to finish the operation */
        esdhc_ptr->SYSCTL |= (SDHC_SYSCTL_PEREN_MASK | SDHC_SYSCTL_HCKEN_MASK | SDHC_SYSCTL_IPGEN_MASK);

        /* Let other tasks to do their job meanwhile */
        _sched_yield();
    }

#if ESDHC_AUTO_CLK_GATING
    /* Switch the clock settings back to the automatic clock gating mode */
    esdhc_ptr->SYSCTL &= ~(SDHC_SYSCTL_PEREN_MASK | SDHC_SYSCTL_HCKEN_MASK | SDHC_SYSCTL_IPGEN_MASK);
#endif

    return result;
}


/*FUNCTION****************************************************************
*
* Function Name    : _esdhc_getCardType
* Returned Value   : MQX error code
* Comments         : Function Fill up the CARD in the device description
*
*
*END*********************************************************************/
static _mqx_int _esdhc_get_card_type
    (
        /* [IN] The I/O init data pointer */
        ESDHC_DEVICE_STRUCT_PTR esdhc_device_ptr
    )
{
  uint32_t                card_type;
  int32_t                 val;
  ESDHC_COMMAND_STRUCT    command;

  card_type = 0;

#define CARD_MEM_MASK           0x01
#define CARD_IO_MASK            0x02
#define CARD_MMC_MASK           0x04
#define CARD_CEATA_MASK         0x08
#define CARD_HC_MASK            0x10
#define CARD_MP_MASK            0x20

  /* CMD0 - Go to idle - reset card */
  command.COMMAND = ESDHC_CREATE_CMD(0, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_NO, ESDHC_COMMAND_NONE_FLAG);
  command.ARGUMENT = 0;
  command.BLOCKS = 0;
  if (_esdhc_send_command(esdhc_device_ptr, &command, NULL))
  {
      return  ESDHC_ERROR_INIT_FAILED;
  }

  /* CMD8 - Send interface condition - check HC support */
  command.COMMAND = ESDHC_CREATE_CMD(8, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_R1, ESDHC_COMMAND_NONE_FLAG);
  command.ARGUMENT = 0x000001AA; // Voltage supply 2.7-3.6V 0x100 + 0xAA check pattern
  command.BLOCKS = 0;

  val = _esdhc_send_command(esdhc_device_ptr, &command, NULL);

  if (val == 0)
  {
      if (command.RESPONSE[0] != command.ARGUMENT)
      {
          return ESDHC_ERROR_INIT_FAILED;
      }
      card_type |= CARD_HC_MASK;
  }
  else  if (val != ESDHC_ERROR_COMMAND_TIMEOUT) // This command should not be supported by old card
  {
      return ESDHC_ERROR_INIT_FAILED;
  }


  /* CMD5 - Send operating conditions - test IO */
  command.COMMAND = ESDHC_CREATE_CMD(5, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_R4, ESDHC_COMMAND_NONE_FLAG);
  command.ARGUMENT = 0;
  command.BLOCKS = 0;
  val = _esdhc_send_command(esdhc_device_ptr, &command, NULL);

  if (val == 0)
  {
      if (((command.RESPONSE[0] >> 28) & 0x07) && (command.RESPONSE[0] & 0x300000))
      {
          /* CMD5 - Send operating conditions - init IO */
          command.COMMAND = ESDHC_CREATE_CMD(5, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_R4, ESDHC_COMMAND_NONE_FLAG);
          command.ARGUMENT = 0x300000;
          command.BLOCKS = 0;
          val = 0;
          do
          {
              _time_delay (BSP_ALARM_RESOLUTION);
              val++;
              if (_esdhc_send_command(esdhc_device_ptr, &command, NULL))
              {
                  return ESDHC_ERROR_INIT_FAILED;
              }

          } while ((0 == (command.RESPONSE[0] & 0x88000000)) && (val < BSP_ALARM_FREQUENCY)); //todo 0x80000000 or 0x88000000

          if (command.RESPONSE[0] & 0x80000000)
          {
              card_type |= CARD_IO_MASK;
          }
          if (command.RESPONSE[0] & 0x08000000)
          {
              card_type |= CARD_MP_MASK;
          }
      }
  }
  else
  {
      card_type |= CARD_MP_MASK;
  }

  if (card_type | CARD_MP_MASK)
  {
      /* CMD55 - Application specific command - check MMC */
      command.COMMAND = ESDHC_CREATE_CMD(55, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_R1, ESDHC_COMMAND_NONE_FLAG);
      command.ARGUMENT = 0;
      command.BLOCKS = 0;
      val = _esdhc_send_command(esdhc_device_ptr, &command, NULL);
      if (val > 0)
      {
          return ESDHC_ERROR_INIT_FAILED;
      }

      if (val < 0)
      {
          /* MMC or CE-ATA */
          card_type &= ~(CARD_IO_MASK | CARD_MEM_MASK | CARD_HC_MASK);

          /* CMD1 - Send operating conditions - check HC */
          command.COMMAND = ESDHC_CREATE_CMD(1, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_NO, ESDHC_COMMAND_NONE_FLAG);
          command.ARGUMENT = 0x40300000;
          command.BLOCKS = 0;
          if (_esdhc_send_command(esdhc_device_ptr, &command, NULL))
          {
              return ESDHC_ERROR_INIT_FAILED;
          }
          if (0x20000000 == (command.RESPONSE[0] & 0x60000000))
          {
              card_type |= CARD_HC_MASK;
          }
          card_type |= CARD_MMC_MASK;

          /* CMD39 - Fast IO - check CE-ATA signature CE */
          command.COMMAND = ESDHC_CREATE_CMD(39, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_R4, ESDHC_COMMAND_NONE_FLAG);
          command.ARGUMENT = 0x0C00;
          command.BLOCKS = 0;
          if (_esdhc_send_command(esdhc_device_ptr, &command, NULL))
          {
              return ESDHC_ERROR_INIT_FAILED;
          }

          if (0xCE == ((command.RESPONSE[0] >> 8) & 0xFF))
          {
              /* CMD39 - Fast IO - check CE-ATA signature AA */
              command.COMMAND = ESDHC_CREATE_CMD(39, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_R4, ESDHC_COMMAND_NONE_FLAG);
              command.ARGUMENT = 0x0D00;
              command.BLOCKS = 0;
              if (_esdhc_send_command(esdhc_device_ptr, &command, NULL))
              {
                  return ESDHC_ERROR_INIT_FAILED;
              }
              if (0xAA == ((command.RESPONSE[0] >> 8) & 0xFF))
              {
                  card_type &= ~CARD_MMC_MASK;
                  card_type |= CARD_CEATA_MASK;
              }
          }
      }
      else
      {
          /* SD */
          /* ACMD41 - Send Operating Conditions */
          command.COMMAND = ESDHC_CREATE_CMD(41, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_R3, 0/*SDHC_COMMAND_ACMD_FLAG*/);
          command.ARGUMENT = 0;
          command.BLOCKS = 0;
          if (_esdhc_send_command(esdhc_device_ptr, &command, NULL))
          {
              return ESDHC_ERROR_INIT_FAILED;
          }
          if (command.RESPONSE[0] & 0x300000)
          {
              val = 0;
              do
              {
                  _time_delay (BSP_ALARM_RESOLUTION);
                  val++;

                  command.COMMAND = ESDHC_CREATE_CMD(41, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_R3, ESDHC_COMMAND_ACMD_FLAG);
                  if (card_type | CARD_HC_MASK)
                  {
                      command.ARGUMENT = 0x40300000;
                  }
                  else
                  {
                      command.ARGUMENT = 0x00300000;
                  }
                  command.BLOCKS = 0;
                  if (_esdhc_send_command(esdhc_device_ptr, &command, NULL))
                  {
                      return ESDHC_ERROR_INIT_FAILED;
                  }
              } while ((0 == (command.RESPONSE[0] & 0x80000000)) && (val < BSP_ALARM_FREQUENCY));

              if (val >= BSP_ALARM_FREQUENCY)
              {
                  card_type &= ~CARD_HC_MASK;
              }
              else
              {
                  card_type |= CARD_MEM_MASK;

                  if (!(command.RESPONSE[0] & 0x40000000))
                  {
                    card_type &= ~CARD_HC_MASK;
                  }

              }
          }
      }
  }

  switch(card_type & ~CARD_MP_MASK)
  {
    case CARD_MMC_MASK:
      esdhc_device_ptr->CARD = ESDHC_CARD_MMC;
      break;

    case CARD_CEATA_MASK:
      esdhc_device_ptr->CARD = ESDHC_CARD_CEATA;
      break;

    case CARD_MEM_MASK:
      esdhc_device_ptr->CARD = ESDHC_CARD_SD;
      break;

    case (CARD_MEM_MASK | CARD_HC_MASK):
      esdhc_device_ptr->CARD = ESDHC_CARD_SDHC;
      break;

    case CARD_IO_MASK:
      esdhc_device_ptr->CARD = ESDHC_CARD_SDIO;
      break;

    case (CARD_MEM_MASK | CARD_IO_MASK):
      esdhc_device_ptr->CARD = ESDHC_CARD_SDCOMBO;
      break;

    case (CARD_MEM_MASK | CARD_IO_MASK | CARD_HC_MASK):
      esdhc_device_ptr->CARD = ESDHC_CARD_SDHCCOMBO;
      break;

    default:
      esdhc_device_ptr->CARD = ESDHC_CARD_UNKNOWN;
      break;
  }

  return ESDHC_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : _esdhc_install
* Returned Value   : MQX error code
* Comments         :
*    Install an ESDHC device.
*
*END*********************************************************************/
_mqx_int _esdhc_install
    (
        /* [IN] A string that identifies the device for fopen */
        char                   *identifier,

        /* [IN] The I/O init data pointer */
        ESDHC_INIT_STRUCT_CPTR esdhc_init_ptr
    )
{
    ESDHC_DEVICE_STRUCT_PTR    esdhc_device_ptr;
    ESDHC_REG_STRUCT_PTR       esdhc_reg_ptr;
    uint32_t                   vector;
    _mqx_uint                  result;

    /* Check parameters */
    if ((NULL == identifier) || (NULL == esdhc_init_ptr))
    {
        return MQX_INVALID_PARAMETER;
    }

    /* Obtain device base address and interrupt vector */
    esdhc_reg_ptr = _bsp_get_esdhc_base_address(esdhc_init_ptr->CHANNEL);
    vector = _bsp_get_esdhc_vector(esdhc_init_ptr->CHANNEL);
    if (NULL == esdhc_reg_ptr || 0 == vector)
    {
        return IO_ERROR_DEVICE_INVALID;
    }

    /* Create device context */
    esdhc_device_ptr = _mem_alloc_system_zero(sizeof(ESDHC_DEVICE_STRUCT));
    if (NULL == esdhc_device_ptr)
    {
        return MQX_OUT_OF_MEMORY;
    }
    _mem_set_type (esdhc_device_ptr, MEM_TYPE_IO_ESDHC_DEVICE_STRUCT);

#if ESDHC_IS_HANDLING_CACHE
    esdhc_device_ptr->ADMA2_DATA = _mem_alloc_system(sizeof(ESDHC_ADMA2_DATA));
    if (NULL == esdhc_device_ptr->ADMA2_DATA)
    {
        _mem_free(esdhc_device_ptr);
        return MQX_OUT_OF_MEMORY;
    }
#endif

    result = _lwevent_create(&esdhc_device_ptr->LWEVENT, 0);
    if (result != MQX_OK)
    {
        _mem_free(esdhc_device_ptr);
        return result;
    }

    esdhc_device_ptr->INIT = esdhc_init_ptr;
    esdhc_device_ptr->ESDHC_REG_PTR = esdhc_reg_ptr;
    esdhc_device_ptr->VECTOR = vector;

    /* Install the interrupt service routine */
    _int_install_isr(esdhc_device_ptr->VECTOR, _esdhc_isr, esdhc_device_ptr);
    _bsp_int_init(esdhc_device_ptr->VECTOR, BSP_ESDHC_INT_LEVEL, 0, FALSE);

    /* Install device */
    result = _io_dev_install_ext(identifier,
        _esdhc_open,
        _esdhc_close,
        _esdhc_read,
        _esdhc_write,
        _esdhc_ioctl,
        _esdhc_uninstall,
        (void *)esdhc_device_ptr
    );

    /* Cleanup if there is any error during installation */
    if(MQX_OK != result)
    {
        _bsp_int_disable(esdhc_device_ptr->VECTOR);
        _int_install_isr(esdhc_device_ptr->VECTOR, _int_get_default_isr(), NULL);
        _lwevent_destroy(&esdhc_device_ptr->LWEVENT);
        _mem_free(esdhc_device_ptr);
    }

    return result;
}


/*FUNCTION****************************************************************
*
* Function Name    : _esdhc_uninstall
* Returned Value   : MQX error code
* Comments         :
*    Uninstall an ESDHC device.
*
*END**********************************************************************/
_mqx_int _esdhc_uninstall
    (
        /* [IN/OUT] The device to uninstall */
        IO_DEVICE_STRUCT_PTR dev_ptr
    )
{
    ESDHC_DEVICE_STRUCT_PTR esdhc_device_ptr;

    /* Check parameters */
    if (NULL == dev_ptr)
    {
        return IO_DEVICE_DOES_NOT_EXIST;
    }

    esdhc_device_ptr = dev_ptr->DRIVER_INIT_PTR;
    if (NULL == esdhc_device_ptr)
    {
        return IO_ERROR_DEVICE_INVALID;
    }

    /* Uninstall only when not opened */
    if (esdhc_device_ptr->COUNT)
    {
        return IO_ERROR_DEVICE_BUSY;
    }

    /* Cleanup */
    _bsp_int_disable(esdhc_device_ptr->VECTOR);
    _int_install_isr(esdhc_device_ptr->VECTOR, _int_get_default_isr(), NULL);
    _lwevent_destroy(&esdhc_device_ptr->LWEVENT);
#if ESDHC_IS_HANDLING_CACHE
    _mem_free(esdhc_device_ptr->ADMA2_DATA);
#endif
    _mem_free(esdhc_device_ptr);

    dev_ptr->DRIVER_INIT_PTR = NULL;

    return MQX_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : _esdhc_open
* Returned Value   : MQX error code
* Comments         :
*    This function exclusively opens the ESDHC device.
*
*END*********************************************************************/
_mqx_int _esdhc_open
    (
        /* [IN/OUT] ESDHC file descriptor */
        MQX_FILE_PTR        esdhc_fd_ptr,

        /* [IN] The remaining portion of the name of the device */
        char                *open_name_ptr,

        /* [IN] The flags to be used during operation */
        char                *open_flags_ptr
    )
{
    _mqx_int result;
    ESDHC_DEVICE_STRUCT_PTR esdhc_device_ptr = esdhc_fd_ptr->DEV_PTR->DRIVER_INIT_PTR;

    /* Exclusive access till close */
    _int_disable();
    if (esdhc_device_ptr->COUNT)
    {
        /* Device is already opened */
        _int_enable();
        return MQX_IO_OPERATION_NOT_AVAILABLE;
    }
    esdhc_device_ptr->COUNT++;
    _int_enable();

    result = _esdhc_init(esdhc_device_ptr);
    if (result != MQX_OK)
    {
        _int_disable();
        esdhc_device_ptr->COUNT--;
        _int_enable();
    }

    return result;
}


/*FUNCTION****************************************************************
*
* Function Name    : _esdhc_close
* Returned Value   : MQX error code
* Comments         :
*    This function closes opened ESDHC device.
*
*END*********************************************************************/
_mqx_int _esdhc_close
    (
        /* [IN/OUT] Opened file pointer for ESDHC */
        MQX_FILE_PTR        esdhc_fd_ptr
    )
{
    ESDHC_DEVICE_STRUCT_PTR esdhc_device_ptr = esdhc_fd_ptr->DEV_PTR->DRIVER_INIT_PTR;
    ESDHC_REG_STRUCT_PTR    esdhc_ptr = esdhc_device_ptr->ESDHC_REG_PTR;

    /* Disable ESDHC device */
    _int_disable();
    if (--esdhc_device_ptr->COUNT == 0)
    {
        esdhc_ptr->SYSCTL = SDHC_SYSCTL_RSTA_MASK | SDHC_SYSCTL_SDCLKFS(0x80);
        while (esdhc_ptr->SYSCTL & SDHC_SYSCTL_RSTA_MASK)
            { };
    }
    _int_enable();

    return MQX_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : _esdhc_ioctl
* Returned Value   : MQX error code
* Comments         :
*    This function performs miscellaneous services for the ESDHC I/O device.
*
*END*********************************************************************/
_mqx_int _esdhc_ioctl
    (
        /* [IN] Opened file pointer for ESDHC */
        MQX_FILE_PTR        esdhc_fd_ptr,

        /* [IN] The command to perform */
        uint32_t            cmd,

        /* [IN/OUT] Parameters for the command */
        void                *param_ptr
    )
{
    int32_t                 val;
    int32_t                 result = ESDHC_OK;
    uint32_t                *param32_ptr = param_ptr;

    ESDHC_DEVICE_STRUCT_PTR esdhc_device_ptr = esdhc_fd_ptr->DEV_PTR->DRIVER_INIT_PTR;
    ESDHC_REG_STRUCT_PTR    esdhc_ptr = esdhc_device_ptr->ESDHC_REG_PTR;
    ESDHC_INIT_STRUCT_CPTR  esdhc_init_ptr = esdhc_device_ptr->INIT;

    switch (cmd)
    {
        case IO_IOCTL_ESDHC_INIT:
            result = _esdhc_init(esdhc_device_ptr);
            break;

        case IO_IOCTL_ESDHC_SEND_COMMAND:
            /* Check if this is data transfer command to store it and send it as part of the read/write operation (when buffer address is known) */
            if(((ESDHC_COMMAND_STRUCT_PTR)param32_ptr)->COMMAND & ESDHC_COMMAND_DATACMD_FLAG)
            {
                esdhc_device_ptr->BUFFERED_CMD = *((ESDHC_COMMAND_STRUCT_PTR)param32_ptr);
            }
            else
            {
                esdhc_device_ptr->BUFFERED_CMD.COMMAND = 0;

                val = _esdhc_send_command(esdhc_device_ptr, (ESDHC_COMMAND_STRUCT_PTR)param32_ptr, NULL);
                if (val > 0)
                {
                    result = ESDHC_ERROR_COMMAND_FAILED;
                }
                if (val < 0)
                {
                    result = ESDHC_ERROR_COMMAND_TIMEOUT;
                }
            }
            break;

        case IO_IOCTL_ESDHC_GET_BAUDRATE:
            if (NULL == param32_ptr)
            {
                result = MQX_INVALID_PARAMETER;
            }
            else
            {
                /* Get actual baudrate */
                *param32_ptr = _esdhc_get_baudrate(esdhc_device_ptr);
            }
            break;

        case IO_IOCTL_ESDHC_GET_BAUDRATE_MAX:
            if (NULL == param32_ptr)
            {
                result = MQX_INVALID_PARAMETER;
            }
            else
            {
                if(esdhc_ptr->HTCAPBLT & SDHC_HTCAPBLT_HSS_MASK)
                    val = 50000000;
                else
                    val = 25000000;

                if((esdhc_init_ptr->MAX_BAUD_RATE == 0) || (esdhc_init_ptr->MAX_BAUD_RATE > val))
                    *param32_ptr = val;
                else
                    *param32_ptr = esdhc_init_ptr->MAX_BAUD_RATE;
            }
          break;

        case IO_IOCTL_ESDHC_SET_BAUDRATE:
            if (NULL == param32_ptr)
            {
                result = MQX_INVALID_PARAMETER;
            }
            else if (0 == (*param32_ptr))
            {
                result = MQX_INVALID_PARAMETER;
            }
            else
            {
                if (! _esdhc_is_running (esdhc_ptr))
                {
                    _esdhc_set_baudrate(esdhc_device_ptr, *param32_ptr);
                }
                else
                {
                    result = IO_ERROR_DEVICE_BUSY;
                }
            }
            break;

        case IO_IOCTL_ESDHC_GET_BUS_WIDTH:
            if (NULL == param32_ptr)
            {
                result = MQX_INVALID_PARAMETER;
            }
            else
            {
                /* Get actual ESDHC bus width */
                val = (esdhc_ptr->PROCTL & SDHC_PROCTL_DTW_MASK) >> SDHC_PROCTL_DTW_SHIFT;
                if (ESDHC_PROCTL_DTW_1BIT == val)
                {
                    *param32_ptr = ESDHC_BUS_WIDTH_1BIT;
                }
                else if (ESDHC_PROCTL_DTW_4BIT == val)
                {
                    *param32_ptr = ESDHC_BUS_WIDTH_4BIT;
                }
                else if (ESDHC_PROCTL_DTW_8BIT == val)
                {
                    *param32_ptr = ESDHC_BUS_WIDTH_8BIT;
                }
                else
                {
                    result = ESDHC_ERROR_INVALID_BUS_WIDTH;
                }
            }
            break;

        case IO_IOCTL_ESDHC_SET_BUS_WIDTH:
            if (NULL == param32_ptr)
            {
                result = MQX_INVALID_PARAMETER;
            }
            else
            {
                /* Set actual ESDHC bus width */
                if (! _esdhc_is_running (esdhc_ptr))
                {
                    if (ESDHC_BUS_WIDTH_1BIT == *param32_ptr)
                    {
                        esdhc_ptr->PROCTL &= (~ SDHC_PROCTL_DTW_MASK);
                        esdhc_ptr->PROCTL |= SDHC_PROCTL_DTW(ESDHC_PROCTL_DTW_1BIT);
                    }
                    else if (ESDHC_BUS_WIDTH_4BIT == *param32_ptr)
                    {
                        esdhc_ptr->PROCTL &= (~ SDHC_PROCTL_DTW_MASK);
                        esdhc_ptr->PROCTL |= SDHC_PROCTL_DTW(ESDHC_PROCTL_DTW_4BIT);
                    }
                    else if (ESDHC_BUS_WIDTH_8BIT == *param32_ptr)
                    {
                        esdhc_ptr->PROCTL &= (~ SDHC_PROCTL_DTW_MASK);
                        esdhc_ptr->PROCTL |= SDHC_PROCTL_DTW(ESDHC_PROCTL_DTW_8BIT);
                    }
                    else
                    {
                        result = ESDHC_ERROR_INVALID_BUS_WIDTH;
                    }
                }
                else
                {
                    result = IO_ERROR_DEVICE_BUSY;
                }
            }
            break;

        case IO_IOCTL_ESDHC_GET_CARD:
            if (NULL == param32_ptr)
            {
                result = MQX_INVALID_PARAMETER;
            }
            else
            {
                /* 80 clocks to update levels */
                esdhc_ptr->SYSCTL |= SDHC_SYSCTL_INITA_MASK;
                while (esdhc_ptr->SYSCTL & SDHC_SYSCTL_INITA_MASK)
                    { };

                /* Update and return actual card status */
                if (esdhc_ptr->IRQSTAT & SDHC_IRQSTAT_CRM_MASK)
                {
                    esdhc_ptr->IRQSTAT |= SDHC_IRQSTAT_CRM_MASK;
                    esdhc_device_ptr->CARD = ESDHC_CARD_NONE;
                }

                if (esdhc_ptr->PRSSTAT & SDHC_PRSSTAT_CINS_MASK)
                {
                    if ((ESDHC_CARD_NONE == esdhc_device_ptr->CARD) || (ESDHC_CARD_UNKNOWN == esdhc_device_ptr->CARD))
                    {
                        esdhc_device_ptr->CARD = ESDHC_CARD_UNKNOWN;

                        /* Backup the current baudrate */
                        val = _esdhc_get_baudrate(esdhc_device_ptr);

                        /* Set up slow init baudrate */
                        if (ESDHC_OK != (result = _esdhc_set_baudrate(esdhc_device_ptr, ESDHC_INIT_BAUDRATE)))
                        {
                          break;
                        }

                        /* Recognize inserted card */
                        _esdhc_get_card_type(esdhc_device_ptr);

                        /* Restore original baudrate */
                        if (ESDHC_OK != (result = _esdhc_set_baudrate(esdhc_device_ptr, val)))
                        {
                          break;
                        }
                    }
                }
                else
                {
                    esdhc_device_ptr->CARD = ESDHC_CARD_NONE;
                }

                *param32_ptr = esdhc_device_ptr->CARD;
            }
            break;

        case IO_IOCTL_DEVICE_IDENTIFY:
            /* Get ESDHC device parameters */
            param32_ptr[IO_IOCTL_ID_PHY_ELEMENT]  = IO_DEV_TYPE_PHYS_ESDHC;
            param32_ptr[IO_IOCTL_ID_LOG_ELEMENT]  = IO_DEV_TYPE_LOGICAL_MFS;
            param32_ptr[IO_IOCTL_ID_ATTR_ELEMENT] = IO_ESDHC_ATTRIBS;
            if (esdhc_fd_ptr->FLAGS & IO_O_RDONLY)
            {
                param32_ptr[IO_IOCTL_ID_ATTR_ELEMENT] &= (~ IO_DEV_ATTR_WRITE);
            }
            break;

        case IO_IOCTL_FLUSH_OUTPUT:
            result = _esdhc_wait_while_busy(esdhc_device_ptr);
            break;

        case IO_IOCTL_ESDHC_SET_IO_CALLBACK:
            if (NULL == param_ptr || ((ESDHC_IO_INT_CALLBACK_STRUCT *)param_ptr)->CALLBACK == NULL)
            {
                /* Disable and clear pending IRQ */
                esdhc_ptr->IRQSIGEN &= ~SDHC_IRQSIGEN_CINTIEN_MASK;
                esdhc_ptr->IRQSTAT |= SDHC_IRQSTAT_CINT_MASK;

                /* Unregister callback */
                esdhc_device_ptr->IO_CALLBACK_STR.CALLBACK = NULL;
                esdhc_device_ptr->IO_CALLBACK_STR.USERDATA = NULL;
            }
            else
            {
                /* Register callback and enable IO card interrupt */
                esdhc_device_ptr->IO_CALLBACK_STR = *((ESDHC_IO_INT_CALLBACK_STRUCT*)param32_ptr);

                /* Clear pending IRQ and enable it */
                esdhc_ptr->IRQSTAT |= SDHC_IRQSTAT_CINT_MASK;
                esdhc_ptr->IRQSIGEN |= SDHC_IRQSIGEN_CINTIEN_MASK;
            }
            break;

#if ESDHC_CARD_DETECTION_SUPPORT
        case IO_IOCTL_ESDHC_SET_CARD_CALLBACK:
            if (NULL == param_ptr)
            {
                esdhc_device_ptr->CARD_PRESENCE_CALLBACK_STR.CALLBACK = NULL;
                esdhc_device_ptr->CARD_PRESENCE_CALLBACK_STR.USERDATA = NULL;
            }
            else
            {
                esdhc_device_ptr->CARD_PRESENCE_CALLBACK_STR = *((ESDHC_CARD_PRESENCE_CALLBACK_STRUCT_PTR)param_ptr);
            }
            break;
#endif

        case IO_IOCTL_ESDHC_GET_CARD_PRESENCE:
            if (NULL == param32_ptr)
            {
                result = MQX_INVALID_PARAMETER;
            }
            else
            {
                if(esdhc_ptr->PRSSTAT & SDHC_PRSSTAT_CINS_MASK)
                    *param32_ptr = TRUE;
                else
                    *param32_ptr = FALSE;
            }
            break;

        case IO_IOCTL_GET_REQ_ALIGNMENT:
            if (NULL == param32_ptr)
            {
                result = MQX_INVALID_PARAMETER;
                break;
            }
            *param32_ptr = 4; /* 32-bit word alignment required for ADMA2 operation */
            break;

        default:
            result = IO_ERROR_INVALID_IOCTL_CMD;
            break;
    }
    return result;
}


/*FUNCTION****************************************************************
*
* Function Name    : _esdhc_read
* Returned Value   : Returns the number of bytes received or IO_ERROR
* Comments         :
*   Reads the data into provided array.
*
*END*********************************************************************/
_mqx_int _esdhc_read
    (
        /* [IN] Opened file pointer for ESDHC */
        FILE_DEVICE_STRUCT_PTR esdhc_fd_ptr,

        /* [OUT] Where the characters are to be stored */
        char                  *data_ptr,

        /* [IN] The number of bytes to read */
        _mqx_int               n
    )
{
    ESDHC_DEVICE_STRUCT_PTR esdhc_device_ptr = esdhc_fd_ptr->DEV_PTR->DRIVER_INIT_PTR;
    ESDHC_REG_STRUCT_PTR    esdhc_ptr = esdhc_device_ptr->ESDHC_REG_PTR;

#if ESDHC_IS_HANDLING_CACHE
    ESDHC_ADMA2_DESC *adma2_desc = esdhc_device_ptr->ADMA2_DATA->DESC;
    int adma2_desc_idx;

    uint8_t *head;
    uint32_t head_len;

    uint8_t *body;
    uint32_t body_len;

    uint8_t *tail;
    uint32_t tail_len;
#else
    ESDHC_ADMA2_DESC adma2_desc[1];
#endif

    /* Check if the buffered command is ready */
    if((esdhc_device_ptr->BUFFERED_CMD.COMMAND & (ESDHC_COMMAND_DATACMD_FLAG | ESDHC_COMMAND_DATA_READ_FLAG)) != (ESDHC_COMMAND_DATACMD_FLAG | ESDHC_COMMAND_DATA_READ_FLAG))
    {
        return IO_ERROR;
    }

    /* Infinite transfers are not supported */
    if(esdhc_device_ptr->BUFFERED_CMD.BLOCKS == ((uint32_t)-1))
    {
        return IO_ERROR;
    }

    /* Check if the transfer fits into the buffer (size block * count of size must be <= n) */
    if(esdhc_device_ptr->BUFFERED_CMD.BLOCKS * esdhc_device_ptr->BUFFERED_CMD.BLOCKSIZE > n)
    {
        return IO_ERROR;
    }

    /* Zero length transfer is a no-op */
    if(n == 0)
    {
        return 0;
    }

    /* Check max transfer size */
    if(n >= 0x10000)
    {
        return IO_ERROR;
    }

    /* Check buffer alignment */
    if(((uint32_t)data_ptr) & 0x03)
    {
        return IO_ERROR;
    }

#if ESDHC_IS_HANDLING_CACHE

    /* Prepare the section lengths */
    head_len = (PSP_MEMORY_ALIGNMENT + 1) - ((uint32_t)data_ptr & PSP_MEMORY_ALIGNMENT);
    body_len = (n - head_len) & ~((uint32_t)PSP_MEMORY_ALIGNMENT);
    tail_len = (n - head_len - body_len);

    adma2_desc_idx = -1; /* Points to last valid descriptor, which is none at the moment */
    if(head_len)
    {
        head = esdhc_device_ptr->ADMA2_DATA->HEAD_BUF;
        adma2_desc_idx++;
        adma2_desc[adma2_desc_idx].LEN_ATTR = (head_len << 16) | ESDHC_ADMA2_FLAG_TRAN | ESDHC_ADMA2_FLAG_VALID;
        adma2_desc[adma2_desc_idx].DATA_ADDR = (uint32_t)head;
    }
    if (body_len)
    {
        body = ((uint8_t *)data_ptr) + head_len;
        adma2_desc_idx++;
        adma2_desc[adma2_desc_idx].LEN_ATTR = (body_len << 16) | ESDHC_ADMA2_FLAG_TRAN | ESDHC_ADMA2_FLAG_VALID;
        adma2_desc[adma2_desc_idx].DATA_ADDR = (uint32_t)body;
    }
    if(tail_len)
    {
        tail = esdhc_device_ptr->ADMA2_DATA->TAIL_BUF;
        adma2_desc_idx++;
        adma2_desc[adma2_desc_idx].LEN_ATTR = (tail_len << 16) | ESDHC_ADMA2_FLAG_TRAN | ESDHC_ADMA2_FLAG_VALID;
        adma2_desc[adma2_desc_idx].DATA_ADDR = (uint32_t)tail;
    }
    adma2_desc[adma2_desc_idx].LEN_ATTR |= ESDHC_ADMA2_FLAG_END;
    _DCACHE_FLUSH_MBYTES(adma2_desc, 3*sizeof(ESDHC_ADMA2_DESC));

#else
    adma2_desc[0].LEN_ATTR = (n << 16) | ESDHC_ADMA2_FLAG_TRAN | ESDHC_ADMA2_FLAG_VALID | ESDHC_ADMA2_FLAG_END;
    adma2_desc[0].DATA_ADDR = (uint32_t)data_ptr;
#endif

    _lwevent_clear(&esdhc_device_ptr->LWEVENT, (ESDHC_LWEVENT_TRANSFER_DONE | ESDHC_LWEVENT_TRANSFER_ERROR));

    if(_esdhc_send_command(esdhc_device_ptr, &(esdhc_device_ptr->BUFFERED_CMD), adma2_desc) != MQX_OK)
    {
        esdhc_device_ptr->BUFFERED_CMD.COMMAND = 0;
        return IO_ERROR;
    }

    /* Wait for transfer to finish. Timeout depends on number of blocks. */
    if (_lwevent_wait_ticks(&esdhc_device_ptr->LWEVENT, (ESDHC_LWEVENT_TRANSFER_DONE | ESDHC_LWEVENT_TRANSFER_ERROR), FALSE, ESDHC_CMD12_TICK_TIMEOUT) != MQX_OK)
    {
        /* Something is wrong - timeout or any error of light weight event */
        return IO_ERROR_TIMEOUT;
    }
    else
    {
       if(_lwevent_get_signalled() & ESDHC_LWEVENT_TRANSFER_ERROR)
       {
            if(esdhc_device_ptr->BUFFERED_CMD.BLOCKS > 1)
            {
                /* In this case the peripheral doesn't automatically send the CMD12, so MUST be sent manually */
                ESDHC_COMMAND_STRUCT command;

                // FIXME: This is probably wrong; Correctly compute the count of blocks actually read
                int32_t count = (int32_t)((uint32_t)esdhc_ptr->DSADDR - (uint32_t)data_ptr);
                // int32_t count = (esdhc_ptr->DSADDR & (esdhc_device_ptr->BUFFERED_CMD.BLOCKSIZE - 1) - esdhc_device_ptr->BUFFERED_CMD.BLOCKSIZE - (uint32_t)data_ptr);

                command.COMMAND = ESDHC_CREATE_CMD(12, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_R1, (ESDHC_COMMAND_NONE_FLAG));
                command.ARGUMENT = 0;
                command.BLOCKS = 0;

                if(_esdhc_send_command(esdhc_device_ptr, &command, NULL) != MQX_OK)
                {
                    return IO_ERROR;
                }

                esdhc_ptr->SYSCTL |= SDHC_SYSCTL_RSTD_MASK;
                return count;
            }
            else
            {
                esdhc_ptr->SYSCTL |= SDHC_SYSCTL_RSTD_MASK;
                return 0;
            }
        }
    }

#if ESDHC_IS_HANDLING_CACHE
    if (head_len)
    {
      _DCACHE_INVALIDATE_LINE(head);
      _mem_copy(head, data_ptr, head_len);
    }

    if (body_len)
    {
        _DCACHE_INVALIDATE_MBYTES(body, body_len);
    }

    if  (tail_len)
    {
      _DCACHE_INVALIDATE_LINE(tail);
      _mem_copy(tail, ((uint8_t *)data_ptr) + head_len + body_len, tail_len);
    }
#endif

    return (esdhc_device_ptr->BUFFERED_CMD.BLOCKS * esdhc_device_ptr->BUFFERED_CMD.BLOCKSIZE);
}


/*FUNCTION****************************************************************
*
* Function Name    : _esdhc_write
* Returned Value   : return number of byte transmitted or IO_ERROR
* Comments         :
*   Writes the provided data buffer to the device.
*
*END*********************************************************************/
_mqx_int _esdhc_write
    (
        /* [IN] Opened file pointer for ESDHC */
        FILE_DEVICE_STRUCT_PTR esdhc_fd_ptr,

        /* [OUT] Where the characters are to be taken from */
        char                  *data_ptr,

        /* [IN] The number of bytes to read */
        _mqx_int               n
    )
{
    ESDHC_DEVICE_STRUCT_PTR esdhc_device_ptr = esdhc_fd_ptr->DEV_PTR->DRIVER_INIT_PTR;
    ESDHC_REG_STRUCT_PTR    esdhc_ptr = esdhc_device_ptr->ESDHC_REG_PTR;

#if ESDHC_IS_HANDLING_CACHE
    ESDHC_ADMA2_DESC *adma2_desc = esdhc_device_ptr->ADMA2_DATA->DESC;
    int adma2_desc_idx;

    uint8_t *head;
    uint32_t head_len;

    uint8_t *body;
    uint32_t body_len;

    uint8_t *tail;
    uint32_t tail_len;
#else
    ESDHC_ADMA2_DESC adma2_desc[1];
#endif

    /* Check if the buffered command is ready */
    if((esdhc_device_ptr->BUFFERED_CMD.COMMAND & (ESDHC_COMMAND_DATACMD_FLAG | ESDHC_COMMAND_DATA_READ_FLAG)) != (ESDHC_COMMAND_DATACMD_FLAG))
    {
        return IO_ERROR;
    }

    /* Infinite transfers are not supported */
    if(esdhc_device_ptr->BUFFERED_CMD.BLOCKS == ((uint32_t)-1))
    {
      return IO_ERROR;
    }

    /* Check if the transfer fits into the buffer (size block * count of size must be <= n) */
    if((esdhc_device_ptr->BUFFERED_CMD.BLOCKS * esdhc_device_ptr->BUFFERED_CMD.BLOCKSIZE) > n)
    {
      return IO_ERROR;
    }

    /* Zero length transfer is a no-op */
    if(n == 0)
    {
      return 0;
    }

    /* Check max transfer size */
    if(n >= 0x10000)
    {
        return IO_ERROR;
    }

    /* Check buffer alignment */
    if(((uint32_t)data_ptr) & 0x03)
    {
        return IO_ERROR;
    }

#if ESDHC_IS_HANDLING_CACHE

    /* Prepare the section lengths */
    head_len = (PSP_MEMORY_ALIGNMENT + 1) - ((uint32_t)data_ptr & PSP_MEMORY_ALIGNMENT);
    body_len = (n - head_len) & ~((uint32_t)PSP_MEMORY_ALIGNMENT);
    tail_len = (n - head_len - body_len);

    adma2_desc_idx = -1; /* Points to last valid descriptor, which is none at the moment */
    if(head_len)
    {
        head = esdhc_device_ptr->ADMA2_DATA->HEAD_BUF;
        adma2_desc_idx++;
        adma2_desc[adma2_desc_idx].LEN_ATTR = (head_len << 16) | ESDHC_ADMA2_FLAG_TRAN | ESDHC_ADMA2_FLAG_VALID;
        adma2_desc[adma2_desc_idx].DATA_ADDR = (uint32_t)head;
    }
    if (body_len)
    {
        body = ((uint8_t *)data_ptr) + head_len;
        adma2_desc_idx++;
        adma2_desc[adma2_desc_idx].LEN_ATTR = (body_len << 16) | ESDHC_ADMA2_FLAG_TRAN | ESDHC_ADMA2_FLAG_VALID;
        adma2_desc[adma2_desc_idx].DATA_ADDR = (uint32_t)body;
    }
    if(tail_len)
    {
        tail = esdhc_device_ptr->ADMA2_DATA->TAIL_BUF;
        adma2_desc_idx++;
        adma2_desc[adma2_desc_idx].LEN_ATTR = (tail_len << 16) | ESDHC_ADMA2_FLAG_TRAN | ESDHC_ADMA2_FLAG_VALID;
        adma2_desc[adma2_desc_idx].DATA_ADDR = (uint32_t)tail;
    }
    adma2_desc[adma2_desc_idx].LEN_ATTR |= ESDHC_ADMA2_FLAG_END;
    _DCACHE_FLUSH_MBYTES(adma2_desc, 3*sizeof(ESDHC_ADMA2_DESC));

    /* Flush caches */
    if (head_len)
    {
        _mem_copy(data_ptr, head, head_len);
        _DCACHE_FLUSH_LINE(head);
    }

    if (body_len)
    {
        _DCACHE_FLUSH_MBYTES(body, body_len);
    }

    if (tail_len)
    {
        _mem_copy(((uint8_t*)data_ptr) + head_len + body_len, tail, tail_len);
        _DCACHE_FLUSH_LINE(tail);
    }

#else
    adma2_desc[0].LEN_ATTR = (n << 16) | ESDHC_ADMA2_FLAG_TRAN | ESDHC_ADMA2_FLAG_VALID | ESDHC_ADMA2_FLAG_END;
    adma2_desc[0].DATA_ADDR = (uint32_t)data_ptr;
#endif

    _lwevent_clear(&esdhc_device_ptr->LWEVENT, (ESDHC_LWEVENT_TRANSFER_DONE | ESDHC_LWEVENT_TRANSFER_ERROR));

    if(_esdhc_send_command(esdhc_device_ptr, &(esdhc_device_ptr->BUFFERED_CMD), adma2_desc) != MQX_OK)
    {
        esdhc_device_ptr->BUFFERED_CMD.COMMAND = 0;
        return IO_ERROR;
    }

    /* Wait for transfer to complete. Timeout depends on number of blocks. */
    if (_lwevent_wait_ticks(&esdhc_device_ptr->LWEVENT, (ESDHC_LWEVENT_TRANSFER_DONE | ESDHC_LWEVENT_TRANSFER_ERROR), FALSE, /*(esdhc_device_ptr->BUFFERED_CMD.BLOCKS > 1)? */ESDHC_CMD12_TICK_TIMEOUT /*: ESDHC_CMD_TICK_TIMEOUT*/) != MQX_OK)
    {
        return IO_ERROR_TIMEOUT;
    }
    else
    {
        if(_lwevent_get_signalled() & ESDHC_LWEVENT_TRANSFER_ERROR)
        {
            esdhc_ptr->SYSCTL |= SDHC_SYSCTL_RSTD_MASK;

            if(esdhc_device_ptr->BUFFERED_CMD.BLOCKS > 1)
            {
                /* In this case the peripheral doesn't automatically send the CMD12, so MUST be sent manually */
                ESDHC_COMMAND_STRUCT command;
                command.COMMAND = ESDHC_CREATE_CMD(12, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_R1b, (ESDHC_COMMAND_NONE_FLAG));
                command.ARGUMENT = 0;
                command.BLOCKS = 0;

                _esdhc_send_command(esdhc_device_ptr, &command, NULL);
                /* Don't care about the result because the operation ends always by IO_ERROR. */
            }

            return IO_ERROR;
        }
    }

    return (esdhc_device_ptr->BUFFERED_CMD.BLOCKS * esdhc_device_ptr->BUFFERED_CMD.BLOCKSIZE);
}


/*FUNCTION****************************************************************
*
* Function Name    : _esdhc_isr
* Returned Value   : EDHC interrupt routine
* Comments         :
*   End of operation of ESDHC handler.
*
*END*********************************************************************/
static void _esdhc_isr
    (
        /* [IN] The address of the device specific information */
        void                          *parameter
    )
{
  ESDHC_DEVICE_STRUCT_PTR esdhc_device_ptr = (ESDHC_DEVICE_STRUCT_PTR)parameter;
  ESDHC_REG_STRUCT_PTR    esdhc_ptr = esdhc_device_ptr->ESDHC_REG_PTR;
  uint32_t                sdhc_irqstat;

  /* Back up the IRQ status */
  sdhc_irqstat  = esdhc_ptr->IRQSTAT;

  // Clear the all sets IRQ status bits
  esdhc_ptr->IRQSTAT = sdhc_irqstat;

  /*
      DMA Error
    Occurs when an Internal DMA transfer has failed. This bit is set to 1, when some error occurs in the data
    transfer. This error can be caused by either Simple DMA or ADMA, depending on which DMA is in use.
    The value in DMA System Address register is the next fetch address where the error occurs. Since any
    error corrupts the whole data block, the host driver shall re-start the transfer from the corrupted block
    boundary. The address of the block boundary can be calculated either from the current DSADDR value or
    from the remaining number of blocks and the block size.
  */
  if((esdhc_ptr->IRQSIGEN & SDHC_IRQSIGEN_DMAEIEN_MASK) && (sdhc_irqstat & SDHC_IRQSTAT_DMAE_MASK))
  {
    _lwevent_set( &esdhc_device_ptr->LWEVENT, ESDHC_LWEVENT_TRANSFER_ERROR);
  }

  /*
      Auto CMD12 Error
    Occurs when detecting that one of the bits in the Auto CMD12 Error Status register has changed from 0 to
    1. This bit is set to 1, not only when the errors in Auto CMD12 occur, but also when the Auto CMD12 is
    not executed due to the previous command error.
  */
  if((esdhc_ptr->IRQSIGEN & SDHC_IRQSIGEN_AC12EIEN_MASK) && (sdhc_irqstat & SDHC_IRQSTAT_AC12E_MASK))
  {
    _lwevent_set( &esdhc_device_ptr->LWEVENT, (ESDHC_LWEVENT_CMD_ERROR | ESDHC_LWEVENT_TRANSFER_ERROR));
  }

  /*
      Data End Bit Error
    Occurs either when detecting 0 at the end bit position of read data, which uses the DAT line, or at the end
    bit position of the CRC.
  */
  if((esdhc_ptr->IRQSIGEN & SDHC_IRQSIGEN_DEBEIEN_MASK) && (sdhc_irqstat & SDHC_IRQSTAT_DEBE_MASK))
  {
    _lwevent_set( &esdhc_device_ptr->LWEVENT, ESDHC_LWEVENT_TRANSFER_ERROR);
  }

  /*
      Data CRC Error
    Occurs when detecting a CRC error when transferring read data, which uses the DAT line, or when
    detecting the Write CRC status having a value other than 010.
  */
  if((esdhc_ptr->IRQSIGEN & SDHC_IRQSIGEN_DCEIEN_MASK) && (sdhc_irqstat & SDHC_IRQSTAT_DCE_MASK))
  {
    _lwevent_set( &esdhc_device_ptr->LWEVENT, ESDHC_LWEVENT_TRANSFER_ERROR);
  }

  /*
      Data Timeout Error
    Occurs when detecting one of following time-out conditions.
      • Busy time-out for R1b,R5b type
      • Busy time-out after Write CRC status
      • Read Data time-out
  */
  if((esdhc_ptr->IRQSIGEN & SDHC_IRQSIGEN_DTOEIEN_MASK) && (sdhc_irqstat & SDHC_IRQSTAT_DTOE_MASK))
  {
    _lwevent_set( &esdhc_device_ptr->LWEVENT, ESDHC_LWEVENT_TRANSFER_ERROR);
  }

  /*
      Command Index Error
    Occurs if a Command Index error occurs in the command response.
  */
  if((esdhc_ptr->IRQSIGEN & SDHC_IRQSIGEN_CIEIEN_MASK) && (sdhc_irqstat & SDHC_IRQSTAT_CIE_MASK))
  {
    _lwevent_set( &esdhc_device_ptr->LWEVENT, ESDHC_LWEVENT_CMD_ERROR);
  }

  /*
      Command End Bit Error
    Occurs when detecting that the end bit of a command response is 0.
  */
  if((esdhc_ptr->IRQSIGEN & SDHC_IRQSIGEN_CEBEIEN_MASK) && (sdhc_irqstat & SDHC_IRQSTAT_CEBE_MASK))
  {
    _lwevent_set( &esdhc_device_ptr->LWEVENT, ESDHC_LWEVENT_CMD_ERROR);
  }



  /*
      Command CRC Error
    Command CRC Error is generated in two cases.
    • If a response is returned and the Command Timeout Error is set to 0 (indicating no time-out), this bit
    is set when detecting a CRC error in the command response.
    • The SDHC detects a CMD line conflict by monitoring the CMD line when a command is issued. If
    the SDHC drives the CMD line to 1, but detects 0 on the CMD line at the next SDCLK edge, then
    the SDHC shall abort the command (Stop driving CMD line) and set this bit to 1. The Command
    Timeout Error shall also be set to 1 to distinguish CMD line conflict.
  */
  if((esdhc_ptr->IRQSIGEN & SDHC_IRQSIGEN_CCEIEN_MASK) && (sdhc_irqstat & SDHC_IRQSTAT_CCE_MASK))
  {
    _lwevent_set( &esdhc_device_ptr->LWEVENT, ESDHC_LWEVENT_CMD_ERROR);
  }


  /*
      Command Timeout Error
    Occurs only if no response is returned within 64 SDCLK cycles from the end bit of the command. If the
    SDHC detects a CMD line conflict, in which case a Command CRC Error shall also be set, this bit shall be
    set without waiting for 64 SDCLK cycles. This is because the command will be aborted by the SDHC.
  */
  if((esdhc_ptr->IRQSIGEN & SDHC_IRQSIGEN_CTOEIEN_MASK) && (sdhc_irqstat & SDHC_IRQSTAT_CTOE_MASK))
  {
    _lwevent_set( &esdhc_device_ptr->LWEVENT, ESDHC_LWEVENT_CMD_TIMEOUT);
  }

  /*
      Card Interrupt
    This status bit is set when an interrupt signal is detected from the external card. In 1-bit mode, the SDHC
    will detect the Card Interrupt without the SD Clock to support wakeup. In 4-bit mode, the card interrupt
    signal is sampled during the interrupt cycle, so the interrupt from card can only be sampled during
    interrupt cycle, introducing some delay between the interrupt signal from the SDIO card and the interrupt
    to the host system. Writing this bit to 1 can clear this bit, but as the interrupt factor from the SDIO card
    does not clear, this bit is set again. In order to clear this bit, it is required to reset the interrupt factor from
    the external card followed by a writing 1 to this bit.
    When this status has been set, and the host driver needs to service this interrupt, the Card Interrupt
    Signal Enable in the Interrupt Signal Enable register should be 0 to stop driving the interrupt signal to the
    host system. After completion of the card interrupt service (It should reset the interrupt factors in the SDIO
    card and the interrupt signal may not be asserted), write 1 to clear this bit, set the Card Interrupt Signal
    Enable to 1, and start sampling the interrupt signal again.
  */
  if((esdhc_ptr->IRQSIGEN & SDHC_IRQSIGEN_CINTIEN_MASK) && (sdhc_irqstat & SDHC_IRQSTAT_CINT_MASK))
  {
    if(esdhc_device_ptr->IO_CALLBACK_STR.CALLBACK)
      esdhc_device_ptr->IO_CALLBACK_STR.CALLBACK(esdhc_device_ptr->IO_CALLBACK_STR.USERDATA);
  }

#if ESDHC_CARD_DETECTION_SUPPORT
  /*
      Card Removal
    This status bit is set if the Card Inserted bit in the Present State register changes from 1 to 0. When the
    host driver writes this bit to 1 to clear this status, the status of the Card Inserted in the Present State
    register should be confirmed. Because the card state may possibly be changed when the host driver
    clears this bit and the interrupt event may not be generated. When this bit is cleared, it will be set again if
    no card is inserted. In order to leave it cleared, clear the Card Removal Status Enable bit in Interrupt
    Status Enable register.
      0b Card state unstable or inserted
      1b Card removed
  */
  if((esdhc_ptr->IRQSIGEN & SDHC_IRQSIGEN_CRMIEN_MASK) && (sdhc_irqstat & SDHC_IRQSTAT_CRM_MASK) /*&& !(esdhc_ptr->PRSSTAT & SDHC_PRSSTAT_CINS_MASK)*/)
  {
    // Set card to NONE
    esdhc_device_ptr->CARD = ESDHC_CARD_NONE;

    // Switch of Card Insert interrupts
    esdhc_ptr->IRQSIGEN &= ~SDHC_IRQSIGEN_CRMIEN_MASK;
    esdhc_ptr->IRQSIGEN |= SDHC_IRQSIGEN_CINSIEN_MASK;

    // Switch the peripheral to 1 bit mode to enable correct work of Dat3 signal to card insert detection
    esdhc_ptr->PROCTL &= (~ SDHC_PROCTL_DTW_MASK);
    esdhc_ptr->PROCTL |= SDHC_PROCTL_DTW(ESDHC_PROCTL_DTW_1BIT);

    // Notify higher layer if requested
    if(esdhc_device_ptr->CARD_PRESENCE_CALLBACK_STR.CALLBACK)
      esdhc_device_ptr->CARD_PRESENCE_CALLBACK_STR.CALLBACK(esdhc_device_ptr->CARD_PRESENCE_CALLBACK_STR.USERDATA, FALSE);

    // Set the ERRORS for internal events to manage active trasfers
    _lwevent_set( &esdhc_device_ptr->LWEVENT, (ESDHC_LWEVENT_CMD_ERROR | ESDHC_LWEVENT_TRANSFER_ERROR));
  }

  /*
      Card Insertion
    This status bit is set if the Card Inserted bit in the Present State register changes from 0 to 1. When the
    host driver writes this bit to 1 to clear this status, the status of the Card Inserted in the Present State
    register should be confirmed. Because the card state may possibly be changed when the host driver
    clears this bit and the interrupt event may not be generated. When this bit is cleared, it will be set again if
    a card is inserted. In order to leave it cleared, clear the Card Inserted Status Enable bit in Interrupt Status
    Enable register.
      0b Card state unstable or removed
      1b Card inserted
  */
  else if((esdhc_ptr->IRQSIGEN & SDHC_IRQSIGEN_CINSIEN_MASK) && (sdhc_irqstat & SDHC_IRQSTAT_CINS_MASK) /*&& (esdhc_ptr->PRSSTAT & SDHC_PRSSTAT_CINS_MASK)*/)
  {
    // Switch of Card Insert interrupts


    esdhc_ptr->IRQSIGEN &= ~SDHC_IRQSIGEN_CINSIEN_MASK;
    esdhc_ptr->IRQSIGEN |= SDHC_IRQSIGEN_CRMIEN_MASK;

    // If there was none card, just change the state to Unkown card
    if(esdhc_device_ptr->CARD == ESDHC_CARD_NONE)
    {
      esdhc_device_ptr->CARD = ESDHC_CARD_UNKNOWN;

      // Notify higher layer if requested
      if(esdhc_device_ptr->CARD_PRESENCE_CALLBACK_STR.CALLBACK)
        esdhc_device_ptr->CARD_PRESENCE_CALLBACK_STR.CALLBACK(esdhc_device_ptr->CARD_PRESENCE_CALLBACK_STR.USERDATA, TRUE);
    }

    // Set the ERRORS for internal events to manage active trasfers
    _lwevent_set( &esdhc_device_ptr->LWEVENT, (ESDHC_LWEVENT_CMD_ERROR | ESDHC_LWEVENT_TRANSFER_ERROR));
  }

#endif
  /*
      Buffer Read Ready
    This status bit is set if the Buffer Read Enable bit, in the Present State register, changes from 0 to 1.
    Refer to the Buffer Read Enable bit in the Present State register for additional information.
  */
  if((esdhc_ptr->IRQSIGEN & SDHC_IRQSIGEN_BRRIEN_MASK) && (sdhc_irqstat & SDHC_IRQSTAT_BRR_MASK))
  {

  }

  /*
      Buffer Write Ready
    This status bit is set if the Buffer Write Enable bit, in the Present State register, changes from 0 to 1. Refer
    to the Buffer Write Enable bit in the Present State register for additional information.
  */
  if((esdhc_ptr->IRQSIGEN & SDHC_IRQSIGEN_BWRIEN_MASK) && (sdhc_irqstat & SDHC_IRQSTAT_BWR_MASK))
  {

  }

  /*
      DMA Interrupt
    Occurs only when the internal DMA finishes the data transfer successfully. Whenever errors occur during
    data transfer, this bit will not be set. Instead, the DMAE bit will be set. Either Simple DMA or ADMA
    finishes data transferring, this bit will be set.
  */
  if((esdhc_ptr->IRQSIGEN & SDHC_IRQSIGEN_DINTIEN_MASK) && (sdhc_irqstat & SDHC_IRQSTAT_DINT_MASK))
  {
    //_lwevent_set( &esdhc_device_ptr->LWEVENT, ESDHC_LWEVENT_TRANSFER_DONE);
  }

  /*
      Block Gap Event
    If the PROCTL[SABGREQ] is set, this bit is set when a read or write transaction is stopped at a block gap.
    If PROCTL[SABGREQ] is not set to 1, this bit is not set to 1.
    In the case of a read transaction: This bit is set at the falling edge of the DAT line active status (When the
    transaction is stopped at SD Bus timing). The read wait must be supported in order to use this function.
    In the case of write transaction: This bit is set at the falling edge of write transfer active status (After
    getting CRC status at SD bus timing).
  */
  if((esdhc_ptr->IRQSIGEN & SDHC_IRQSIGEN_BGEIEN_MASK) && (sdhc_irqstat & SDHC_IRQSTAT_BGE_MASK))
  {
    //_lwevent_set( &esdhc_device_ptr->LWEVENT, ESDHC_LWEVENT_TRANSFER_ERROR);
  }

  /*
      Transfer Complete
    This bit is set when a read or write transfer is completed.
    In the case of a read transaction: This bit is set at the falling edge of the read transfer active status. There
    are two cases in which this interrupt is generated. The first is when a data transfer is completed as
    specified by the data length (after the last data has been read to the host system). The second is when
    data has stopped at the block gap and completed the data transfer by setting the PROCTL[SABGREQ]
    (after valid data has been read to the host system).
    In the case of a write transaction: This bit is set at the falling edge of the DAT line active status. There are
    two cases in which this interrupt is generated. The first is when the last data is written to the SD card as
    specified by the data length and the busy signal is released. The second is when data transfers are
    stopped at the block gap, by setting the PROCTL[SABGREQ], and the data transfers are completed.
    (after valid data is written to the SD card and the busy signal released).
  */
  if((esdhc_ptr->IRQSIGEN & SDHC_IRQSIGEN_TCIEN_MASK) && (sdhc_irqstat & SDHC_IRQSTAT_TC_MASK))
  {
    _lwevent_set( &esdhc_device_ptr->LWEVENT, ESDHC_LWEVENT_TRANSFER_DONE);
  }

  /*
      Command Complete
    This bit is set when you receive the end bit of the command response (except Auto CMD12). Refer to the
    PRSSTAT[CIHB].
  */
  if((esdhc_ptr->IRQSIGEN & SDHC_IRQSIGEN_CCIEN_MASK) && (sdhc_irqstat & SDHC_IRQSTAT_CC_MASK))
  {
    _lwevent_set( &esdhc_device_ptr->LWEVENT, ESDHC_LWEVENT_CMD_DONE);
  }
}

/* EOF */
