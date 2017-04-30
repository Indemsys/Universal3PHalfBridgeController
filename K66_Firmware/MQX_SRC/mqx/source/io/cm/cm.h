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
*   Clock manager header file.
*
*
*END************************************************************************/

#ifndef __cm_h__
    #define __cm_h__

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
**
**  Clock manager public types and constants
**
***************************************************************************/
#define CM_ERR_OK           0x00U /* OK */
#define CM_ERR_SPEED        0x01U /* This device does not work in the active speed mode. */
#define CM_ERR_RANGE        0x02U /* Parameter out of range. */
#define CM_ERR_VALUE        0x03U /* Parameter of incorrect value. */
#define CM_ERR_FAILED       0x1BU /* Requested functionality or process failed. */
#define CM_ERR_PARAM_MODE   0x81U /* Invalid mode. */
    
_mqx_int _cm_set_clock_configuration
(
    /* [IN] runtime clock configuration */
    const BSP_CLOCK_CONFIGURATION clock_configuration
);

BSP_CLOCK_CONFIGURATION _cm_get_clock_configuration
(
    void
);

uint32_t _cm_get_clock
(
    /* [IN] clock configuration */
    const BSP_CLOCK_CONFIGURATION   clock_configuration,
    /* [IN] clock source index */
    const CM_CLOCK_SOURCE           clock_source
);


#ifdef __cplusplus
}
#endif


#endif /* __cm_h__ */
