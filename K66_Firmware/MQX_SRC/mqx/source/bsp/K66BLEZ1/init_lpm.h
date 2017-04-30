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
*   Low Power Manager initialization definitions.
*
*
*END************************************************************************/

#ifndef __init_lpm_h__
    #define __init_lpm_h__

/*-------------------------------------------------------------------------*/
/*
**                            CONSTANT DEFINITIONS
*/

typedef enum 
{
    LPM_OPERATION_MODE_RUN = 0,
    LPM_OPERATION_MODE_WAIT,
    LPM_OPERATION_MODE_SLEEP,
    LPM_OPERATION_MODE_STOP,
    LPM_OPERATION_MODE_HSRUN,
    LPM_OPERATION_MODES          /* Number of operation modes available */
} LPM_OPERATION_MODE;


/*-------------------------------------------------------------------------*/
/*
**                            MACRO DECLARATIONS
*/


/*-------------------------------------------------------------------------*/
/*
**                            DATATYPE DECLARATIONS
*/


/*-------------------------------------------------------------------------*/
/*
**                            FUNCTION PROTOTYPES
*/


#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif


#endif

/* EOF */
