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
*   This file contains Low Power Manager functionality specific to Kinetis.
*
*
*END************************************************************************/


#include "mqx.h"
#include "bsp.h"


#if MQX_ENABLE_LOW_POWER


#ifndef PE_LDD_VERSION


static const LPM_CPU_POWER_MODE LPM_CPU_POWER_MODES_KINETIS[LPM_CPU_POWER_MODES] =
{
    /* Kinetis RUN*/
    {
        SMC_PMCTRL_LPWUI_MASK,                      /* Mode PMCTRL register == voltage regulator ON after wakeup*/
        0,                                          /* Mode flags == clear settings*/
    },
    /* Kinetis WAIT*/
    {
        SMC_PMCTRL_LPWUI_MASK,                      /* Mode PMCTRL register == voltage regulator ON after wakeup*/
        LPM_CPU_POWER_MODE_FLAG_USE_WFI,            /* Mode flags == execute WFI*/
    },
    /* Kinetis STOP*/
    {
        SMC_PMCTRL_LPWUI_MASK,                      /* Mode PMCTRL register == voltage regulator ON after wakeup*/
        LPM_CPU_POWER_MODE_FLAG_DEEP_SLEEP | LPM_CPU_POWER_MODE_FLAG_USE_WFI,   /* Mode flags == deepsleep, execute WFI*/
    },
    /* Kinetis VLPR*/
    {
        SMC_PMCTRL_RUNM(2),                         /* Mode PMCTRL register == VLPR*/
        0,                                          /* Mode flags == clear settings*/
    },
    /* Kinetis VLPW*/
    {
        SMC_PMCTRL_RUNM(2),                         /* Mode PMCTRL register == VLPW*/
        LPM_CPU_POWER_MODE_FLAG_USE_WFI,            /* Mode flags == execute WFI*/
    },
    /* Kinetis VLPS*/
    {
        SMC_PMCTRL_STOPM(2),                                                    /* Mode PMCTRL register == VLPS*/
        LPM_CPU_POWER_MODE_FLAG_DEEP_SLEEP | LPM_CPU_POWER_MODE_FLAG_USE_WFI,   /* Mode flags == deepsleep, execute WFI*/
    },
    /* Kinetis LLS*/
    {
        SMC_PMCTRL_LPWUI_MASK | SMC_PMCTRL_STOPM(3), /* Mode PMCTRL register == voltage regulator ON after wakeup, LLS*/
        LPM_CPU_POWER_MODE_FLAG_DEEP_SLEEP | LPM_CPU_POWER_MODE_FLAG_USE_WFI,   /* Mode flags == deepsleep, execute WFI*/
    },
    /* Kinetis VLLS3*/
    {
        SMC_PMCTRL_LPWUI_MASK | SMC_PMCTRL_STOPM(4),
        LPM_CPU_POWER_MODE_FLAG_DEEP_SLEEP | LPM_CPU_POWER_MODE_FLAG_USE_WFI
    },
    /* Kinetis VLLS2*/
    {
        SMC_PMCTRL_LPWUI_MASK | SMC_PMCTRL_STOPM(4),
        LPM_CPU_POWER_MODE_FLAG_DEEP_SLEEP | LPM_CPU_POWER_MODE_FLAG_USE_WFI
    },
    /* Kinetis VLLS1*/
    {
        SMC_PMCTRL_LPWUI_MASK | SMC_PMCTRL_STOPM(4),
        LPM_CPU_POWER_MODE_FLAG_DEEP_SLEEP | LPM_CPU_POWER_MODE_FLAG_USE_WFI
    }
#if MQX_ENABLE_HSRUN
    ,
    /* Kinetis HSRUN*/
    {
        SMC_PMCTRL_RUNM(3),  /* Mode PMCTRL register == HSRUN */                                        
        0,                   /* Mode flags == clear settings*/
    }
#endif
};


#else


static const LDD_TDriverOperationMode LPM_PE_OPERATION_MODE_MAP[LPM_OPERATION_MODES] =
{
    DOM_RUN,
    DOM_WAIT,
    DOM_SLEEP,
    DOM_STOP
#if MQX_ENABLE_HSRUN
    ,
    DOM_HSRUN
#endif
};


#endif


/*FUNCTION*------------------------------------------------------------------
*
* Function Name    : _lpm_set_cpu_operation_mode
* Returned Value   : MQX error code
* Comments         :
*    This function changes operation mode of the CPU core.
*
*END*----------------------------------------------------------------------*/

_mqx_uint _lpm_set_cpu_operation_mode
    (
        /* [IN] Specification of CPU core low power operation modes available */
        const LPM_CPU_OPERATION_MODE  *operation_modes,

        /* [IN] Low power operation mode identifier */
        LPM_OPERATION_MODE                 target_mode
    )
{

#ifndef PE_LDD_VERSION

    const LPM_CPU_POWER_MODE              *mode_ptr;
    _mqx_uint                              scr, flags, mcg, index, cme;

    /* Check parameters */
    if ((NULL == operation_modes) || (LPM_OPERATION_MODES <= (_mqx_uint)target_mode))
    {
        return MQX_INVALID_PARAMETER;
    }
    index = operation_modes[target_mode].MODE_INDEX;

    if (LPM_CPU_POWER_MODES <= index)
    {
        return MQX_INVALID_CONFIGURATION;
    }
    mode_ptr = &(LPM_CPU_POWER_MODES_KINETIS[index]);
    flags = mode_ptr->FLAGS | (operation_modes[target_mode].FLAGS & LPM_CPU_POWER_MODE_FLAG_USER_MASK);

    /* Go through Kinetis Run */
    scr = (SCB_SCR & (~ (SCB_SCR_SLEEPDEEP_MASK | SCB_SCR_SLEEPONEXIT_MASK)));
    SCB_SCR = scr;
    SMC_PMCTRL = LPM_CPU_POWER_MODES_KINETIS[LPM_CPU_POWER_MODE_RUN].PMCTRL;
    while (0 == (PMC_REGSC & PMC_REGSC_REGONS_MASK))
        { };
    while (SMC_PMSTAT_PMSTAT(1) != SMC_PMSTAT)
        { };
#if MQX_ENABLE_HSRUN
    /* Go to HSRUN through RUN */
    if (LPM_CPU_POWER_MODE_HSRUN == index)
    {
        SMC_PMCTRL = LPM_CPU_POWER_MODES_KINETIS[LPM_CPU_POWER_MODE_HSRUN].PMCTRL;
        while (SMC_PMSTAT_PMSTAT(128) != SMC_PMSTAT)
            { };
    }
#endif
    /* Go to VLPW through VLPR */
    if (LPM_CPU_POWER_MODE_VLPW == index)
    {
        SMC_PMCTRL = LPM_CPU_POWER_MODES_KINETIS[LPM_CPU_POWER_MODE_VLPR].PMCTRL;
        while (SMC_PMSTAT_PMSTAT(4) != SMC_PMSTAT)
            { };
    }

    /* Setup ARM System control register */
    if (flags & LPM_CPU_POWER_MODE_FLAG_DEEP_SLEEP)
    {
        scr |= SCB_SCR_SLEEPDEEP_MASK;
    }
    if (flags & LPM_CPU_POWER_MODE_FLAG_SLEEP_ON_EXIT)
    {
        scr |= SCB_SCR_SLEEPONEXIT_MASK;
    }
    SCB_SCR = scr;

    /* Setup wakeup unit for LLS mode */
    if (LPM_CPU_POWER_MODE_LLS == index)
    {
        LLWU_PE1 = operation_modes[target_mode].PE1;
        LLWU_PE2 = operation_modes[target_mode].PE2;
        LLWU_PE3 = operation_modes[target_mode].PE3;
        LLWU_PE4 = operation_modes[target_mode].PE4;
        LLWU_ME = operation_modes[target_mode].ME;
#if BSP_TWR_K65F180M
        LLWU_PF1 = 0xFF;
        LLWU_PF2 = 0xFF;
        LLWU_PF3 = 0xFF;
#else
        LLWU_F1 = 0xFF;
        LLWU_F2 = 0xFF;
        LLWU_F3 = 0xFF;
#endif
    }

    /* Setup wake up unit for VLLSx mode */
    if ((LPM_CPU_POWER_MODE_VLLS3 == index)
        || (LPM_CPU_POWER_MODE_VLLS2 == index)
        || (LPM_CPU_POWER_MODE_VLLS1 == index))
    {
        LLWU_PE1 = operation_modes[target_mode].PE1;
        LLWU_PE2 = operation_modes[target_mode].PE2;
        LLWU_PE3 = operation_modes[target_mode].PE3;
        LLWU_PE4 = operation_modes[target_mode].PE4;
        LLWU_ME = operation_modes[target_mode].ME;
#if BSP_TWR_K65F180M
        LLWU_PF1 = 0xFF;
        LLWU_PF2 = 0xFF;
        LLWU_PF3 = 0xFF;
#else
        LLWU_F1 = 0xFF;
        LLWU_F2 = 0xFF;
        LLWU_F3 = 0xFF;
#endif
        
#if LLWU_RST_LLRSTE_MASK
        LLWU_RST |= LLWU_RST_LLRSTE_MASK;
#endif
    }

    /* Keep status of MCG before mode change */
    mcg = MCG_S & MCG_S_CLKST_MASK;

    /* Disable CME if enabled before entering changing Power mode */
    cme = MCG_C6 & MCG_C6_CME0_MASK;            /* Save CME state */
    MCG_C6 &= ~(MCG_C6_CME0_MASK);              /* Clear CME */

    /* Operation mode setup */
    SMC_PMCTRL = mode_ptr->PMCTRL;

    /* VLLSx setup */
    switch (index)
    {
#ifdef SMC_STOPCTRL_LLSM_MASK
        case LPM_CPU_POWER_MODE_VLLS3:
            #if BSP_TWR_K65F180M
            SMC_STOPCTRL = ((SMC_STOPCTRL & ~SMC_STOPCTRL_LLSM_MASK) | SMC_STOPCTRL_LLSM(3));
			#else 
			SMC_STOPCTRL = SMC_STOPCTRL_LLSM(3);
			#endif
            break;
        case LPM_CPU_POWER_MODE_VLLS2:
			#if BSP_TWR_K65F180M
            SMC_STOPCTRL = ((SMC_STOPCTRL & ~SMC_STOPCTRL_LLSM_MASK) | SMC_STOPCTRL_LLSM(2));
			#else 
            SMC_STOPCTRL = SMC_STOPCTRL_LLSM(2);
			#endif
            break;
        case LPM_CPU_POWER_MODE_VLLS1:
			#if BSP_TWR_K65F180M
            SMC_STOPCTRL = ((SMC_STOPCTRL & ~SMC_STOPCTRL_LLSM_MASK) | SMC_STOPCTRL_LLSM(1));
			#else 
            SMC_STOPCTRL = SMC_STOPCTRL_LLSM(1);
			#endif
            break;
        default:
            break;
#else
        case LPM_CPU_POWER_MODE_VLLS3:
            SMC_VLLSCTRL = SMC_VLLSCTRL_VLLSM(3);
            break;
        case LPM_CPU_POWER_MODE_VLLS2:
            SMC_VLLSCTRL = SMC_VLLSCTRL_VLLSM(2);
            break;
        case LPM_CPU_POWER_MODE_VLLS1:
            SMC_VLLSCTRL = SMC_VLLSCTRL_VLLSM(1);
            break;
        default:
            break;
#endif
    }

    /* Wait for proper setup of VLPR */
    if (LPM_CPU_POWER_MODE_VLPR == index)
    {
        while (SMC_PMSTAT_PMSTAT(4) != SMC_PMSTAT)
            { };
    }

    /* Go to sleep if required */
    if (flags & LPM_CPU_POWER_MODE_FLAG_USE_WFI)
    {
/* ENGR00178898 workaround  Shut down SPI0, SPI1 pripheral. Preventing entering stop mode for some reason */
#ifdef SIM_SCGC6_SPI0_MASK
        SIM_SCGC6 &= ~SIM_SCGC6_SPI0_MASK;
#endif
#ifdef SIM_SCGC6_SPI1_MASK
        SIM_SCGC6 &= ~SIM_SCGC6_SPI1_MASK;
#endif
        _ASM_SLEEP(NULL);
#ifdef SIM_SCGC6_SPI0_MASK
        SIM_SCGC6 |= SIM_SCGC6_SPI0_MASK;
#endif
#ifdef SIM_SCGC6_SPI1_MASK
        SIM_SCGC6 |= SIM_SCGC6_SPI1_MASK;
#endif
    }

    /* After stop modes, reconfigure MCG if needed */
    if ( (LPM_CPU_POWER_MODE_STOP == index)
      || (LPM_CPU_POWER_MODE_VLPS == index)
      || (LPM_CPU_POWER_MODE_LLS == index)
      || (LPM_CPU_POWER_MODE_VLLS3 == index)
      || (LPM_CPU_POWER_MODE_VLLS2 == index)
      || (LPM_CPU_POWER_MODE_VLLS1 == index) )
    {
#ifdef BSP_CLOCK_USE_FLL
        if ((MCG_S_CLKST(0) == mcg) && (MCG_S_CLKST(1) == (MCG_S & MCG_S_CLKST_MASK)))
        {
            MCG_C1 &= (~ (MCG_C1_CLKS_MASK | MCG_C1_IREFS_MASK));
            while (0 != (MCG_S & MCG_S_CLKST(3)))
                { };
        }
#else
        if ((MCG_S_CLKST(3) == mcg) && (MCG_S_CLKST(2) == (MCG_S & MCG_S_CLKST_MASK)))
        {
            MCG_C1 &= (~ (MCG_C1_CLKS_MASK | MCG_C1_IREFS_MASK));
            while (0 == (MCG_S & MCG_S_LOCK0_MASK))
                { };
        }
#endif
    }

    /* Restore CME */
    MCG_C6 |= cme;

    return MQX_OK;

#else

#ifdef Cpu_SetOperationMode_METHOD_ENABLED
    if (LPM_OPERATION_MODES <= (_mqx_uint)target_mode)
    {
        return MQX_INVALID_PARAMETER;
    }
    return Cpu_SetOperationMode (LPM_PE_OPERATION_MODE_MAP[target_mode], NULL, NULL);
#else
    #error Undefined method Cpu_SetOperationMode() in PE CPU component!
#endif

#endif

}

/*FUNCTION*------------------------------------------------------------------
*
* Function Name    : _lpm_wakeup_core
* Returned Value   : None
* Comments         :
*    This function must be called from ISR not to let the core to return to sleep again.
*
*END*----------------------------------------------------------------------*/

void _lpm_wakeup_core
    (
        void
    )
{
    SCB_SCR &= (~ (SCB_SCR_SLEEPONEXIT_MASK));
}


/*FUNCTION*------------------------------------------------------------------
*
* Function Name    : _lpm_idle_sleep_check
* Returned Value   : TRUE if idle sleep possible, FALSE otherwise
* Comments         :
*    This function checks whether cpu core can sleep during execution of idle task
*    in current power mode.
*
*END*----------------------------------------------------------------------*/

bool _lpm_idle_sleep_check
    (
        void
    )
{
    _mqx_uint pmctrl, stop;

    pmctrl = SMC_PMCTRL;
    stop = SCB_SCR & SCB_SCR_SLEEPDEEP_MASK;

    /* Idle sleep is available only in normal RUN/WAIT and VLPR/VLPW with LPWUI disabled */
    if ((0 == stop) && (0 == (pmctrl & SMC_PMCTRL_STOPM_MASK)) && (! ((SMC_PMCTRL_LPWUI_MASK | SMC_PMCTRL_RUNM(2)) == (pmctrl & (SMC_PMCTRL_LPWUI_MASK | SMC_PMCTRL_RUNM_MASK)))))
    {
        return TRUE;
    }

    return FALSE;
}

/*FUNCTION*------------------------------------------------------------------
*
* Function Name    : _lpm_llwu_clear_flag
* Returned Value   : Void
* Comments         :
*    This function is used to clear LLWU flags and pass them to caller
*
*END*----------------------------------------------------------------------*/

void _lpm_llwu_clear_flag
    (
        /* [OUT] Pointer stores value of LLWU_Fx flags */
        uint32_t *llwu_fx_ptr
    )
{
    volatile uint8_t LLWU_F1_TMP;
    volatile uint8_t LLWU_F2_TMP;
    volatile uint8_t LLWU_F3_TMP;

    /* Read LLWU_Fx into temporary LLWU_Fx_TMP variables */
    #if BSP_TWR_K65F180M
    LLWU_F1_TMP = LLWU_PF1;
    LLWU_F2_TMP = LLWU_PF2;
    LLWU_F3_TMP = LLWU_PF3;
    #else
    LLWU_F1_TMP = LLWU_F1;
    LLWU_F2_TMP = LLWU_F2;
    LLWU_F3_TMP = LLWU_F3;
    #endif

    /* clean wakeup flags */
    #if BSP_TWR_K65F180M
    LLWU_PF1 = LLWU_F1_TMP;
    LLWU_PF2 = LLWU_F2_TMP;
    #else
    LLWU_F1 = LLWU_F1_TMP;
    LLWU_F2 = LLWU_F2_TMP;
    #endif

    if(LLWU_FILT1 & LLWU_FILT1_FILTF_MASK) {
        LLWU_FILT1 |= LLWU_FILT1_FILTF_MASK;
    }

    if(LLWU_FILT2 & LLWU_FILT2_FILTF_MASK) {
        LLWU_FILT2 |= LLWU_FILT2_FILTF_MASK;
    }

    *llwu_fx_ptr = (uint32_t)(LLWU_F3_TMP << 16) | (LLWU_F2_TMP << 8) | (LLWU_F1_TMP);
}


/*FUNCTION*---------------------------------------------------------------------
*
* Function Name    : _lpm_get_reset_source
* Returned Value   : void
* Comments         :
*   Return reset source of Kinetis.
*
*END*-------------------------------------------------------------------------*/

uint32_t _lpm_get_reset_source()
{
    if (RCM_SRS0 & RCM_SRS0_WAKEUP_MASK)
        return MQX_RESET_SOURCE_LLWU;
    else if (RCM_SRS0 & RCM_SRS0_LVD_MASK)
        return MQX_RESET_SOURCE_LOW_VOLTAGE_DETECT;
    else if (RCM_SRS0 & RCM_SRS0_LOC_MASK)
        return MQX_RESET_SOURCE_LOSS_OF_CLOCK;
    else if (RCM_SRS0 & RCM_SRS0_WDOG_MASK)
        return MQX_RESET_SOURCE_WATCHDOG;
    else if (RCM_SRS0 & RCM_SRS0_PIN_MASK)
        return MQX_RESET_SOURCE_EXTERNAL_PIN;
    else if (RCM_SRS0 & RCM_SRS0_POR_MASK)
        return MQX_RESET_SOURCE_POWER_ON;
    else if (RCM_SRS1 & RCM_SRS1_JTAG_MASK)
        return MQX_RESET_SOURCE_JTAG;
    else if (RCM_SRS1 & RCM_SRS1_LOCKUP_MASK)
        return MQX_RESET_SOURCE_CORE_LOCKUP;
    else if (RCM_SRS1 & RCM_SRS1_SW_MASK)
        return MQX_RESET_SOURCE_SOFTWARE;
    else if (RCM_SRS1 & RCM_SRS1_MDM_AP_MASK)
        return MQX_RESET_SOURCE_MDM_AP;
    else if (RCM_SRS1 & RCM_SRS1_EZPT_MASK)
        return MQX_RESET_SOURCE_EZPT;
#if defined (RCM_SRS1_TAMPER_MASK)
    else if (RCM_SRS1 & RCM_SRS1_TAMPER_MASK)
        return MQX_RESET_SOURCE_TAMPER;
#endif
    else
        return MQX_RESET_SOURCE_INVALID;
}


/*FUNCTION*------------------------------------------------------------------
*
* Function Name    : _lpm_write_rfvbat
* Returned Value   : MQX error code
* Comments         :
*    This function write data to a specific RFVBAT register file.
*
*END*----------------------------------------------------------------------*/

_mqx_uint _lpm_write_rfvbat(uint8_t channel, uint32_t data)
{
    if (channel > 7)
        return MQX_INVALID_PARAMETER;

    RFVBAT_REG(channel) = data;

    return MQX_OK;
}


/*FUNCTION*------------------------------------------------------------------
*
* Function Name    : _lpm_read_rfvbat
* Returned Value   : MQX error code
* Comments         :
*    This function read data from a specific RFVBAT register file.
*
*END*----------------------------------------------------------------------*/

uint32_t _lpm_read_rfvbat(uint8_t channel, uint32_t *data_ptr)
{
    if (channel > 7)
        return MQX_INVALID_PARAMETER;

    *data_ptr = RFVBAT_REG(channel);

    return MQX_OK;
}


/*FUNCTION*------------------------------------------------------------------
*
* Function Name    : _lpm_write_rfsys
* Returned Value   : MQX error code
* Comments         :
*    This function write data to a specific SYSTEM register file.
*
*END*----------------------------------------------------------------------*/

_mqx_uint _lpm_write_rfsys(uint8_t channel, uint32_t data)
{
    if (channel > 7)
        return MQX_INVALID_PARAMETER;

    RFSYS_REG(channel) = data;

    return MQX_OK;
}

/*FUNCTION*------------------------------------------------------------------
*
* Function Name    : _lpm_read_rfsys
* Returned Value   : MQX error code
* Comments         :
*    This function read data from a specific SYSTEM register file.
*
*END*----------------------------------------------------------------------*/

uint32_t _lpm_read_rfsys(uint8_t channel, uint32_t *data_ptr)
{
    if (channel > 7)
        return MQX_INVALID_PARAMETER;

    *data_ptr = RFSYS_REG(channel);

    return MQX_OK;
}


#endif /* MQX_ENABLE_LOW_POWER */

/* EOF */
