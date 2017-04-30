/*HEADER**********************************************************************
*
* Copyright 2010 Freescale Semiconductor, Inc.
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
*            Kinetis RTC driver functions
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>

static uint32_t alarm_time    = 0;
static uint32_t alarm_period  = 0;

static volatile RTC_HANDLER rtc_handler = {NULL, NULL};


/*FUNCTION****************************************************************
*
* Function Name    : _rtc_init
* Returned Value   : MQX error code
* Comments         :
*    This function (re)initializes/clears/enables RTC module.
*
*END*********************************************************************/
int32_t _rtc_init
(
    /* [IN] param identifying init operations */
    void* param
)
{ /* Body */
    int32_t       result = MQX_OK;
    RTC_MemMapPtr rtc    = RTC_BASE_PTR;

    _int_disable();
    /* Check if the time is invalid */
    if (rtc->SR & RTC_SR_TIF_MASK)
    {
        rtc->SR &= ~RTC_SR_TCE_MASK;    /* disable rtc timer - enable write access */
        rtc->TAR = 0xFFFFFFFF;          /* e2574: RTC: Writing RTC_TAR[TAR] = 0 does not disable RTC alarm */

        /* if TIF reading TSR return zero and we must set TSR to one */
        rtc->TSR = 1;                   /* this clear SR flags TIF, TOF */
        rtc->SR |= RTC_SR_TCE_MASK;
        /* Update alarm time & alarm period */

        alarm_time = 0;
        alarm_period = 0;
    } /* End if*/
    _int_enable();

    /* Check if RTC counter or OSC for RTC is not started */
    if( !(rtc->SR & RTC_SR_TCE_MASK) || !(rtc->CR & RTC_CR_OSCE_MASK) )
    {
        rtc->CR |= RTC_CR_OSCE_MASK;
        /* recommended 125 ms delay for oscillator start */
        _time_delay(125);
        rtc->SR |= RTC_SR_TCE_MASK;
    } /* End if */

    return result;
} /* Endbody */


/*FUNCTION****************************************************************
*
* Function Name    : _rtc_isr
* Returned Value   : none
* Comments         :
*    This is ISR for RTC module. Interrupt occurs when one
*    of these bits(TIF, TOF, TAF) is set.
*
*END*********************************************************************/
void _rtc_isr
(
    /* [IN] rtc module pointer passed to interrupt */
    void* ptr
)
{ /* Body */
    RTC_MemMapPtr rtc = RTC_BASE_PTR;
    uint32_t      tar_reg, status;

    /* TAR can be changed, store register to variable */
    tar_reg = rtc->TAR;
    status = rtc->SR;

    /* Time Invalid Flag, Time Overflow Flag */
    if (status & (RTC_SR_TIF_MASK | RTC_SR_TOF_MASK))
    {
        rtc->SR &= ~RTC_SR_TCE_MASK;    /* disable rtc timer - enable write access */
        rtc->TAR = 0xFFFFFFFF;          /* e2574: RTC: Writing RTC_TAR[TAR] = 0 does not disable RTC alarm */

        /* if TIF or TOF flag is set, reading TSR return zero and we must set TSR to one */
        rtc->TSR = 1;                   /* this clear SR flags TIF, TOF */
        rtc->SR |= RTC_SR_TCE_MASK;
    }

    if (status & RTC_SR_TAF_MASK) /* Time Alarm Flag */
    {
        if (alarm_period == 0)
        {
            /* e2574: RTC: Writing RTC_TAR[TAR] = 0 does not disable RTC alarm */
            rtc->TAR = tar_reg -1;
            alarm_time = 0;
        }
        else
        {
            /* Update next alarm time */
            alarm_time += alarm_period;
            rtc->TAR = (uint32_t)(tar_reg + alarm_period);
        }

        /* call alarm callback */
        if ((INT_ISR_FPTR)NULL != rtc_handler.isr_func)
        {
            rtc_handler.isr_func (rtc_handler.isr_dat);
        }
    }
} /* Endbody */

/*FUNCTION****************************************************************
*
* Function Name    : _rtc_callback_install
* Returned Value   : MQX error code
* Comments         :
*    This function installs given user callback for RTC module.
*
*END*********************************************************************/
int32_t  _rtc_callback_reg
(
        /* [IN] pointer to user ISR code */
        INT_ISR_FPTR func,
        /* [IN] Type of callback (Alarm or stop watch) */
        void* data
)
{ /* Body */
    int32_t       result = MQX_OK;
    RTC_MemMapPtr rtc    = RTC_BASE_PTR;

    if  (func == NULL) /* Unregister alarm callback */
    {
        /* Disable Alarm interrupt */
        rtc->IER &= ~RTC_IER_TAIE_MASK;

        /* Install default ISR for RTC interrupt */
        if (NULL == _int_install_isr (INT_RTC, _int_get_default_isr(), NULL))
        {
            result = _task_get_error ();
        } /* Endif */

        _bsp_int_init(INT_RTC, BSP_RTC_INT_LEVEL, 0, FALSE);
        rtc_handler.isr_func = NULL;
    }
    else
    {
        /* Update call back func & call back dat */
        rtc_handler.isr_func = NULL;
        rtc_handler.isr_dat  = data;
        rtc_handler.isr_func = func;

        /* Install rtc isr */
        if (NULL == _int_install_isr (INT_RTC, _rtc_isr, (void*)rtc))
        {
            result = _task_get_error ();
        } /* Endif */

        if (MQX_OK == result)
        {
            /* Set INT level */
            _bsp_int_init(INT_RTC, BSP_RTC_INT_LEVEL, 0, TRUE);
            /* Enable RTC interrupt - only Time Alarm Interrupt Enable */
            rtc->IER = RTC_IER_TAIE_MASK;
        } /* Endif */
    } /* Endelse */

    return result;
} /* Endbody */

/*FUNCTION****************************************************************
*
* Function Name    : _rtc_set_time
* Returned Value   : MQX_OK
* Comments         :
*    This function sets the RTC time according to given time struct.
*
*END*********************************************************************/
int32_t _rtc_set_time
(
    /* [IN] given time to be set as RTC time */
    uint32_t time
)
{ /* Body */
    RTC_MemMapPtr rtc = RTC_BASE_PTR;

    /* Disable RTC */
    rtc->SR &= ~RTC_SR_TCE_MASK;

    /* zero prescaler */
    rtc->TPR = 0;
    
    /* set new time */
    rtc->TSR = time;    
       
    /* Enable RTC */
    rtc->SR |= RTC_SR_TCE_MASK;

    return MQX_OK;
} /* Endbody */

/*FUNCTION****************************************************************
*
* Function Name    : _rtc_get_time
* Returned Value   : MQX_OK
* Comments         :
*    This function gets the RTC time and stores it in given time struct.
*
*END*********************************************************************/
int32_t _rtc_get_time
(
    uint32_t *timep
)
{ /* Body */
    RTC_MemMapPtr rtc = RTC_BASE_PTR;
    *timep = (uint32_t)rtc->TSR;

    return MQX_OK;
} /* Endbody */

/*FUNCTION****************************************************************
*
* Function Name    : _rtc_set_alarm
* Returned Value   : RTC_INVALID_TIME if alarm time is larger than current 
*    time; MQX_OK if successful.
* Comments         :
*    This function sets RTC alarm time according to given time struct.
*    Alarm happens immediately after match.
*
*END*********************************************************************/
int32_t _rtc_set_alarm
(
    /* [IN] time to be set as RTC alarm time */
    uint32_t   time,
    uint32_t   period
)
{ /* Body */
    RTC_MemMapPtr rtc = RTC_BASE_PTR;

    if (time < (uint32_t)rtc->TSR) /* Alarm time must be larger than current time */
    {
        return RTC_INVALID_TIME;
    }

    alarm_time     = time;
    alarm_period   = period;
    rtc->TAR = alarm_time;

    return MQX_OK;
} /* Endbody */

/*FUNCTION****************************************************************
*
* Function Name    : _rtc_get_alarm
* Returned Value   : MQX_OK if successful, MQX_ERROR if fail
* Comments         :
*    This function gets the RTC alarm time.
*
*END*********************************************************************/
int32_t _rtc_get_alarm
(
    uint32_t *timep
)
{
    *timep = alarm_time;
    return MQX_OK;
}

