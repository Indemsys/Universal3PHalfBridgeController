/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
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
*   Clock manager generic functions
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>

/*FUNCTION*********************************************************************
* 
* Function Name   : _cm_set_clock_configuration
*
* Input Params    : clock_configuration – runtime clock configuration 
*
* Returned Value  : CM_OK       – if clock mode change was successful
*                   CM_ERROR    – if clock mode change was not successful
*
* Comments        : Function changes all internal clock configurations. 
*                   It calls underlying PE method  SetClockConfiguration() 
*                   that manages switching to clock configuration predefined 
*                   by in PE CPU wizard and calls. 
*
*END**************************************************************************/

_mqx_int _cm_set_clock_configuration
(
    /* [IN] runtime clock configuration */
    const BSP_CLOCK_CONFIGURATION clock_configuration
)
{
    return (_mqx_int)_bsp_set_clock_configuration(clock_configuration);
}


/*FUNCTION*********************************************************************
* 
* Function Name   : _cm_get_clock_configuration
*
* Input Params    : clock_configuration – runtime clock configuration 
*
* Returned Value  : number of active  clock configuration
*
* Comments        : Function changes all internal clock configurations. 
*                   It calls underlying PE method  GetClockConfiguration() 
*                   that returns active clock configuration predefined 
*                   by in PE CPU wizard and calls. 
*
*END**************************************************************************/

BSP_CLOCK_CONFIGURATION _cm_get_clock_configuration
(
    void
)
{
    return _bsp_get_clock_configuration();
}

/*FUNCTION*********************************************************************
* 
* Function Name   : _cm_get_clock
*
* Input Params    : clock_configuration – runtime clock configuration
*                   clock_source        – clock source 
*
* Returned Value  : input frequency in [Hz]
*
* Comments        : Function returns clock frequency specified by clock_source 
*                   and clock_configuration. 
*                   It looks up in CpuClockConfigurations field 
*                   defined in PE_LDD.c for relevant clock.
*
*END**************************************************************************/

uint32_t _cm_get_clock
(
    /* [IN] runtime clock configuration */
    const BSP_CLOCK_CONFIGURATION   clock_configuration,
    /* [IN] clock source index */
    const CM_CLOCK_SOURCE           clock_source
)
{
    return _bsp_get_clock(clock_configuration, clock_source);
}

