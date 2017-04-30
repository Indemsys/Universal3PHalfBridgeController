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

#include <string.h>
#include <mqx.h>
#include <bsp.h>
#include <rtcs.h>

#if RTCSCFG_ENABLE_IP4 && RTCSCFG_ENABLE_PPP && !PLATFORM_SDK_ENABLED
#include "modem_supp.h"

static char *modem_find_char(char *string, char find);
static bool modem_char_is_in_response(char c, char* response, uint32_t *index);
void modem_wait_for_ppp(MODEM_CONTEXT_STRUCT *context_ptr);
static void modem_ppp_linkup(void *param);
static void modem_ppp_linkdown(void *param);

char localname[]    = "1";
char localsecret[]  = "1";
char remotename[]   = "guest";
char remotesecret[] = "anonymous";

PPP_SECRET lsecret ={sizeof(localname)-1, sizeof(localsecret)-1, localname, localsecret};

/* create context structure and allocates resources */
MODEM_CONTEXT_STRUCT* modem_create_context(MODEM_PARAM_STRUCT* params_ptr)
{
    MODEM_CONTEXT_STRUCT* context_ptr;

    context_ptr = NULL;
    context_ptr = _mem_alloc_zero(sizeof(MODEM_CONTEXT_STRUCT));
    if (context_ptr != NULL)
    {
        context_ptr->params = *params_ptr;
    }
    return context_ptr;
}

/* Get next command from commands list. Pair it with responses. */
void modem_get_next_command(MODEM_CONTEXT_STRUCT* context_ptr)
{
    char     *command_end;
    char     *response_end;
    uint32_t i;

    if (context_ptr->next_command_ptr == NULL)
    {
        context_ptr->next_command_ptr = context_ptr->params.commands;
        context_ptr->next_response_ptr = context_ptr->params.responses;
    }

    command_end = NULL;
    response_end = NULL;

    /* Beginning of command is end of previous command. Same applies for responses. */
    context_ptr->command_ptr = context_ptr->next_command_ptr;
    context_ptr->response_ptr = context_ptr->next_response_ptr;

    /* Find command end so we know where to begin in next step. */
    for(i = 0; i <= context_ptr->skip; i++)
    {
        /* Update command start address */
        if (context_ptr->skip > 0)
        {
            context_ptr->command_ptr = command_end+1;
            context_ptr->response_ptr = response_end+1;
        }

        if (command_end == NULL)
        {
            command_end = context_ptr->next_command_ptr;
            response_end = context_ptr->next_response_ptr;
        }

        command_end = strchr(context_ptr->next_command_ptr, '\v');
        if (command_end == NULL)
        {
            context_ptr->command_ptr = NULL;
            break;
        }
        response_end = strchr(context_ptr->next_response_ptr, '\v');
        /* +1 to move to next character after '\v'. */
        context_ptr->next_command_ptr = command_end+1;
        context_ptr->next_response_ptr = response_end+1;
    }

    /* Find start of expected response in user responses. */
    return;
}

/* Send single command to modem */
int32_t modem_send_command(MODEM_CONTEXT_STRUCT* context_ptr)
{
    uint32_t size;
    uint32_t offset;
    uint32_t l_prefix;
    uint32_t l_suffix;
    uint32_t l_command;
    uint32_t wrote;
    int32_t  retval;

    if (context_ptr->command_ptr == NULL)
    {
        return(MODEM_DONE);
    }

    retval = MODEM_OK;
    l_prefix = strlen(MODEM_COMMAND_PREFIX);
    l_command = context_ptr->next_command_ptr - context_ptr->command_ptr - 1;
    l_suffix = strlen(MODEM_COMMAND_SUFFIX);
    size = l_prefix+l_command+l_suffix;

    /* Copy all data to buffer and send it. */
    char data[size];

    offset = 0;
    _mem_copy(MODEM_COMMAND_PREFIX, data+offset, l_prefix);
    offset += l_prefix;
    _mem_copy(context_ptr->command_ptr, data+offset, l_command);
    offset += l_command;
    _mem_copy(MODEM_COMMAND_SUFFIX, data+offset, l_suffix);

    wrote = fwrite(data, 1, size, context_ptr->modem_serial_handler);
    if (wrote != size)
    {
        retval = MODEM_FAIL;
    }
    else
    {
        if (context_ptr->terminal_serial_handler)
        {
            wrote = fwrite(data, 1, size, context_ptr->terminal_serial_handler);
        }
    }

    return(retval);
}

/* find character in commands/responses. */
static char *modem_find_char(char *string, char find)
{
    while((*string != '\v') && (*string != '\0'))
    {
        string++;
        if (*string == find)
        {
            return(string);
        }
    }
    return(NULL);
}

/* Read response from modem. */
int32_t modem_read_response(MODEM_CONTEXT_STRUCT* context_ptr)
{
    uint32_t r_count;
    uint32_t i;
    char     *response_end;
    char     *search_start;
    uint32_t timeout;

    r_count = 0;

    if (context_ptr->response_ptr == NULL)
    {
        return(MODEM_FAIL);
    }

    /* Init searching to actual responses pointer. */
    search_start = context_ptr->response_ptr;

    /* Count responses for command. */
    while((response_end = modem_find_char(search_start, ';')) != NULL)
    {
        search_start = response_end+1;
        r_count++;
    }

    char *sub_responses[r_count+1];
    uint32_t response_indexes[r_count+1];

    sub_responses[0] = context_ptr->response_ptr;

    memset(response_indexes, 0, sizeof(uint32_t)*(r_count+1));
    for(i = 1; i <= r_count; i++)
    {
        response_end = modem_find_char(context_ptr->response_ptr, ';');
        sub_responses[i] = response_end+1;
    }

    timeout = MODEM_RESPONSE_TIMEOUT_MS;
    while(1)
    {
        if(fstatus(context_ptr->modem_serial_handler))
        {
            int chr;

            chr = fgetc(context_ptr->modem_serial_handler);
            if (chr == -1)
            {
                return(MODEM_FAIL);
            }
            /* Match character in all (multiple) responses. */
            for(i = 0; i <= r_count; i++)
            {
                if (!modem_char_is_in_response(chr, sub_responses[i], response_indexes+i))
                {
                    response_indexes[i] = 0;
                }
                else
                {
                    char     *sub_response;
                    uint32_t index;

                    sub_response = sub_responses[i];
                    index = response_indexes[i] + 1;
                    /* If end is encountered, break. */
                    if((sub_response[index] == '\v') || (sub_response[index] == ';'))
                    {
                        /* Set how many commands we have to skip in next step. */
                        context_ptr->skip = i;
                        return(MODEM_OK);
                    }
                }
            }
        }
        else
        {
            timeout = timeout - MODEM_TIMEOUT_TIMESTEP_MS;
            if(!timeout)
            {
                break;
            }
            RTCS_time_delay(MODEM_TIMEOUT_TIMESTEP_MS);
        }
    }
    return(MODEM_FAIL);
}

/* Check if character is in response. */
static bool modem_char_is_in_response(char c, char* response, uint32_t *index)
{
    if(c == response[*index])
    {
        *index = (*index)+1;
        return(true);
    }
    else
    {
        *index = 0;
        return(false);
    }
}

/* Read all remaining characters from modem. */
int32_t modem_rx_fflush(MQX_FILE_PTR modem_serial_handler)
{
    uint32_t c = 0;
    while(fstatus(modem_serial_handler))
    {
        fgetc(modem_serial_handler);
        c++;
    }
    return(c);
}

/* Print debug message. */
void modem_debug_print(MQX_FILE_PTR terminal_serial_handler, const char *data, ...)
{
    if(terminal_serial_handler)
    {
        va_list vl;
        va_start(vl,data);
        vfprintf(terminal_serial_handler, data, vl);
        va_end(vl);
    }
}

/* initialize serial modem using predefined AT commands */
int32_t modem_init(MODEM_CONTEXT_STRUCT* context_ptr)
{
    uint32_t  command_result;
    int32_t   retval;

    command_result = MODEM_OK;
    do
    {
        if (command_result == MODEM_OK)
        {
            modem_get_next_command(context_ptr);
        }
        retval = modem_send_command(context_ptr);
        if (retval != MODEM_OK)
        {
            break;
        }
        _time_delay(100);
        command_result = modem_read_response(context_ptr);
    }while(context_ptr->command_ptr != NULL);///Reach the last command

    fclose(context_ptr->modem_serial_handler);
    context_ptr->modem_serial_handler = NULL;
    return (MODEM_INIT_OK);
}

/* initialize and startup PPP connection*/
int32_t modem_ppp_start(MODEM_CONTEXT_STRUCT* context_ptr)
{
    LWSEM_STRUCT_PTR   ppp_sem;
    MODEM_PARAM_STRUCT *modem_params_ptr;
    PPP_PARAM_STRUCT   *ppp_params_ptr;
    MQX_FILE_PTR       debug_output;
    IPCP_DATA_STRUCT   ipcp_data;

    debug_output = context_ptr->terminal_serial_handler;
    modem_params_ptr = &context_ptr->params;
    ppp_params_ptr = modem_params_ptr->ppp_init_params;

    /* Set pointer to pap_lsecret to give remote system know we will use PAP. */
    _PPP_PAP_LSECRET = &lsecret;
    _PPP_ACCM = 0;

    _mem_zero(&ipcp_data, sizeof(ipcp_data));
    ipcp_data.IP_UP              = modem_ppp_linkup;
    ipcp_data.IP_DOWN            = modem_ppp_linkdown;
    ipcp_data.IP_PARAM           = &ppp_sem;
    ipcp_data.ACCEPT_LOCAL_ADDR  = TRUE;
    ipcp_data.ACCEPT_REMOTE_ADDR = TRUE;
    ipcp_data.LOCAL_ADDR         = 0;
    ipcp_data.REMOTE_ADDR        = 0;
    ipcp_data.DEFAULT_NETMASK    = TRUE;

    /* Init PPP */
    context_ptr->ppp_handle = PPP_init(ppp_params_ptr);
    if (context_ptr->ppp_handle == NULL)
    {
        return(MODEM_PPP_INIT_ERROR);
    }

    if (ppp_params_ptr->listen_flag == 0)
    {
        _ip_address       local_address;
        _ip_address       remote_address = 0;
        _ip_address       dns_server_addr;

        struct addrinfo   *addrinfo_result;
        modem_debug_print(debug_output, "\nAttempting to establish connection. Please wait...");
        modem_wait_for_ppp(context_ptr);

        if (!getaddrinfo(modem_params_ptr->dns_server_address, 0, 0, &addrinfo_result))
        {
            dns_server_addr = ((sockaddr_in *)(addrinfo_result->ai_addr))->sin_addr.s_addr;
            uint32_t result = 0;
            result = RTCS_if_add_dns_addr(ppp_params_ptr->if_handle, dns_server_addr);
            if (result == RTCS_OK)
            {
                modem_debug_print(debug_output, "DNS server added successfuly.\n");
            }
            else
            {
                modem_debug_print(debug_output, "DNS server address fail.\n");
            }
            freeaddrinfo(addrinfo_result);
        }
        else
        {
            modem_debug_print(debug_output, "Unable to resolve ip address\n");
        }

        local_address = IPCP_get_local_addr(ppp_params_ptr->if_handle);
        remote_address = IPCP_get_peer_addr(ppp_params_ptr->if_handle);
        modem_debug_print(debug_output, "Connection established\n");
        modem_debug_print(debug_output, "Local address:  %d.%d.%d.%d\n", IPBYTES(local_address));
        modem_debug_print(debug_output, "Remote address: %d.%d.%d.%d\n", IPBYTES(remote_address));
    }
    else
    {
        modem_debug_print(debug_output, "Waiting for incoming connection.\n");
    }
    return(MODEM_PPP_INIT_OK);
}

/* waits for PPP connection establishment */
void modem_wait_for_ppp(MODEM_CONTEXT_STRUCT* context_ptr)
{
    MQX_FILE_PTR debug_output;
    uint32_t     timeout;

    timeout = context_ptr->params.ppp_connection_timeout;
    debug_output = context_ptr->terminal_serial_handler;

    for (int count = 0; count < timeout; count++)
    {
        if (context_ptr->ppp_link == FALSE)
        {
            modem_debug_print(debug_output, ".");
            _time_delay(1000);
        }
        else
        {
            modem_debug_print(debug_output,"Link_up");
            break;
        }
    }
}

/* Callback invoked on PPP link up event. */
static void modem_ppp_linkup(void *param)
{
    MODEM_CONTEXT_STRUCT* context_ptr;

    context_ptr = (MODEM_CONTEXT_STRUCT *) param;
    context_ptr->ppp_link = TRUE;
}

/* Callback invoked on PPP link down event. */
static void modem_ppp_linkdown(void *param)
{
    MODEM_CONTEXT_STRUCT* context_ptr;

    context_ptr = (MODEM_CONTEXT_STRUCT *) param;
    context_ptr->ppp_link = FALSE;
}
#endif

