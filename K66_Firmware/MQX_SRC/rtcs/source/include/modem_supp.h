#ifndef __MODEM_SUPP_H__
#define __MODEM_SUPP_H__
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
*   This file contains support functions for modem library
*
*END************************************************************************/

#include "modem.h"
#include "modem_prv.h"

void modem_get_next_command(MODEM_CONTEXT_STRUCT* context_ptr);
int32_t modem_send_command(MODEM_CONTEXT_STRUCT* context_ptr);
int32_t modem_read_response(MODEM_CONTEXT_STRUCT* context_ptr);
int32_t modem_rx_fflush(MQX_FILE_PTR modem_serial_handler);
void modem_debug_print (MQX_FILE_PTR terminal_handler, const char *data, ...);
int32_t modem_ppp_start(MODEM_CONTEXT_STRUCT *context_ptr);
int32_t modem_init(MODEM_CONTEXT_STRUCT *context_ptr);
MODEM_CONTEXT_STRUCT* modem_create_context(MODEM_PARAM_STRUCT* params_ptr);
#endif
