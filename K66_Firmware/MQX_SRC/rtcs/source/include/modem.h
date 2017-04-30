#ifndef __modem_h__
#define __modem_h__
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
*   This file contains modem library definitions.
*
*
*END************************************************************************/
#include <rtcs.h>
#include <ppp.h>

#ifndef MODEM_COMMAND_PREFIX
#define MODEM_COMMAND_PREFIX "\r"
#endif

#ifndef MODEM_COMMAND_SUFFIX
#define MODEM_COMMAND_SUFFIX "\r"
#endif

typedef enum modem_ret_code
{
    MODEM_NOT_CONNECTED         ,
    MODEM_CONNECTED             ,
    MODEM_DISCONNECTED          ,                  
    MODEM_PPP_INIT_ERROR        ,                
    MODEM_PPP_INIT_OK           ,                   
    MODEM_COMMAND_NOT_SEND      ,              
    MODEM_COMMAND_SEND          ,                  
    MODEM_DEVICE_ERROR          ,                 
    MODEM_STATUS_INPUT_BUFF_OVERFLOW,    
    MODEM_INIT_OK               ,                       
    MODEM_INIT_ERROR            ,                    
    MODEM_DIAL_UP_ERROR         ,                 
    MODEM_CONTEXT_ERROR         ,                 
    MODEM_TERMINAL_ERROR                
} MODEM_RET_CODE;

/*
** Modem initialization structure
*/
typedef struct modem_param_struct
{
    char*                   terminal_device;        /* terminal communication device name */
    char*                   modem_device;           /* modem communication device name*/
    uint32_t                commands_count;         /* number of init commands */
    char*                   commands;               /* Init commands */
    char*                   *commands_ptr;          /* Init commands pointer*/
    char*                   responses;              /* Responses on init commands */
    char*                   dns_server_address;     /* IP address of DNS server, used for domain name resolving*/
    uint32_t                ppp_enable;             /* Flag for determining if modem will use PPP */
    uint32_t                ppp_connection_timeout; /* Time to wait until PPP connection is established*/
    PPP_PARAM_STRUCT*       ppp_init_params;        /* PPP initialization structure pointer*/
}MODEM_PARAM_STRUCT;


uint32_t MODEM_connect( MODEM_PARAM_STRUCT* modem_params );
void MODEM_disconnect( uint32_t handle ); 

#endif
