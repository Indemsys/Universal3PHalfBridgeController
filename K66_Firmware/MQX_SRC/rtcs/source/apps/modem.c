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
*   This file contains modem library for RTCS.
*
*END************************************************************************/

#include <string.h>
#include <mqx.h>
#include <bsp.h>
#include <rtcs.h>

#if  RTCSCFG_ENABLE_IP4 && RTCSCFG_ENABLE_PPP && !PLATFORM_SDK_ENABLED
#include <ppp.h>
#include "modem.h"
#include "modem_supp.h"
#include "modem_prv.h"

/*set up modem and ppp connection*/
uint32_t MODEM_connect(MODEM_PARAM_STRUCT* init_params)
{
    uint32_t error;

    MODEM_CONTEXT_STRUCT* context_ptr;
    MODEM_PARAM_STRUCT*   modem_params_ptr;

    context_ptr = modem_create_context(init_params);
    if (context_ptr == NULL)
    {
        return(0);
    }
    modem_params_ptr = &context_ptr->params;
    context_ptr->terminal_serial_handler = fopen(modem_params_ptr->terminal_device,0);
    if(!context_ptr->terminal_serial_handler)
    {
        _mem_free(context_ptr);
        return(0); 
    }
    context_ptr->modem_serial_handler = fopen(modem_params_ptr->modem_device,0);
    if(!context_ptr->modem_serial_handler)
    {
        modem_debug_print(context_ptr->terminal_serial_handler," Can't open PPP device\n");
        fclose(context_ptr->terminal_serial_handler);
        _mem_free(context_ptr);
        return(0);
    }
    modem_debug_print(context_ptr->terminal_serial_handler, "Serial modem initialization OK\n");
    modem_rx_fflush(context_ptr->modem_serial_handler);
    
    error = modem_init(context_ptr);
    if(error == MODEM_INIT_OK)
    {
        if (modem_params_ptr->ppp_enable)
        {
            error = modem_ppp_start(context_ptr);
            if(error == MODEM_PPP_INIT_ERROR)
            {   
                modem_debug_print(context_ptr->terminal_serial_handler,"\nPPP connection error\n");
            }
            else if (error == MODEM_PPP_INIT_OK)
            {  
                modem_debug_print(context_ptr->terminal_serial_handler, "\nPPP connection ok\n");
                context_ptr->modem_connected = 1;
            }
        }
        else
        {
            context_ptr->modem_connected = 1;
        }
        return((uint32_t) context_ptr);
    }
    else
    {
        modem_debug_print(context_ptr->terminal_serial_handler,"serial modem init error = %d\n",error);  
        fclose(context_ptr->terminal_serial_handler);
        fclose(context_ptr->modem_serial_handler);
        _mem_free(context_ptr);
        return(0);
    }
}

/* release ppp connection and free all alocated resources*/
void MODEM_disconnect(uint32_t handle)
{
    MODEM_CONTEXT_STRUCT* context_ptr;
    
    context_ptr = (MODEM_CONTEXT_STRUCT*) handle;
    PPP_release(context_ptr->ppp_handle); 
    modem_debug_print(context_ptr->terminal_serial_handler, "PPP connection closed\n");
    fclose(context_ptr->terminal_serial_handler);
    fclose(context_ptr->modem_serial_handler);
    _mem_free(context_ptr);
}
#endif
