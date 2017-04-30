#ifndef __modem_prv_h__
#define __modem_prv_h__
/*HEADER**********************************************************************
*
* Copyright 2014 Freescale Semiconductor, Inc.
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
*   This file contains private definition for modem library
*
*END************************************************************************/

#include "modem.h"

#define MODEM_RESPONSE_TIMEOUT_MS	20000
#define MODEM_TIMEOUT_TIMESTEP_MS	100

typedef enum modem_error
{
	MODEM_OK,
	MODEM_FAIL,
	MODEM_DONE
}MODEM_ERROR;

typedef struct modem_context_struct
{
    MODEM_PARAM_STRUCT  params;                 /* modem initialization parameters structure */
    _ppp_handle         ppp_handle;             /* PPP comunication device handler */
    MQX_FILE_PTR        modem_serial_handler;   /* modem serial communiciaton device handler */
    MQX_FILE_PTR        terminal_serial_handler;/*terminal device handler */
    uint32_t            modem_connected;        /* modem init flag */
    char                *next_command_ptr;      /* next command to be send pointer */
    char                *next_response_ptr;     /* next response to be expected pointer */
    char                *command_ptr;           /* command to be sent to modem pointer.*/
    char                *response_ptr;          /* expected response to command pointer. */
    uint32_t            skip;                   /* Number of commands to skip. */
    bool                ppp_link;               /* PPP link status. */
}MODEM_CONTEXT_STRUCT;

#endif // __modem_prv_h__
