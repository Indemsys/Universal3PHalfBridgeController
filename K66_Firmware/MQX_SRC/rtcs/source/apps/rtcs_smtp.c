/*HEADER**********************************************************************
*
* Copyright 2013 Freescale Semiconductor, Inc.
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
*   This file contains an implementation of a SMTP client.
*
*
*END************************************************************************/

#include <rtcs.h>


#include <rtcs_smtp.h>
#if MQX_USE_IO_OLD
#include <fio.h>
#else
#include <stdio.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <rtcs_base64.h>

typedef struct smtp_find_line_context
{
    char *last_start;
    char *last_end;
    uint32_t first;
}SMTP_FIND_LINE_CONTEXT;

static uint32_t SMTP_send_command (uint32_t socket, char *command, char *response, uint32_t max_size);
static uint32_t SMTP_send_string(uint32_t socket, char *s);
static uint32_t SMTP_get_response_code(char *response);
static uint32_t SMTP_connect (struct sockaddr* server);
static void SMTP_cleanup(uint32_t socket, void *a, ...);
static char *SMTP_findline(char *s, char **line_start, uint32_t *line_length, SMTP_FIND_LINE_CONTEXT* context);
static void SMTP_findline_init(SMTP_FIND_LINE_CONTEXT* context);

/*
** Function for sending email
** IN:
**      SMTP_PARAM_STRUCT_PTR params - Pointer to structure with all required params set up
**                                      (email envelope, email text etc).
**
** OUT:
**      char *err_string - Pointer to string in which error string should be saved (can be NULL -
**                            error string is thrown away in that case).
**
** Return value:
**      _mqx_int - Error code or RTCS_SMTP_OK.
*/
_mqx_int SMTP_send_email (SMTP_PARAM_STRUCT_PTR params, char *err_string, uint32_t err_string_size)
{
    char *response = NULL;
    char *command = NULL;
    char *location = NULL;
    uint32_t code = 0;
    uint32_t socket = 0;

    /* Check params and envelope content for NULL */
    if ((params == NULL) || (params->envelope.from == NULL) || (params->envelope.to == NULL))
    {
        return(SMTP_ERR_BAD_PARAM);
    }

    /* Allocate buffers */
    response = (char *) _mem_alloc_system(sizeof(char)*SMTP_RESPONSE_BUFFER_SIZE);
    if (response == NULL)
    {
        return(MQX_OUT_OF_MEMORY);
    }
    command = (char *) _mem_alloc_system(sizeof(char)*SMTP_COMMAND_BUFFER_SIZE);
    if (command == NULL)
    {
        SMTP_cleanup(socket, response, NULL);
        return(MQX_OUT_OF_MEMORY);
    }
    
    /* Connect to server */
    socket = SMTP_connect(&params->server);
    if (socket == 0)
    {
        SMTP_cleanup(socket, response, command, NULL);
        return(SMTP_ERR_CONN_FAILED);
    }
    
    /* Read greeting message */
    recv(socket, response, SMTP_RESPONSE_BUFFER_SIZE, 0);
    
    /* Get response code */
    code = SMTP_get_response_code(response);
    if (code > 299)
    {
        SET_ERR_STR(err_string, response, err_string_size);
        SMTP_cleanup(socket, response, command, NULL);
        return(SMTP_WRONG_RESPONSE);
    }
    
    /* Get server extensions */
    sprintf(command, "EHLO FreescaleTower");
    code = SMTP_send_command(socket, command, response, SMTP_RESPONSE_BUFFER_SIZE);
    
    /* If server does not support EHLO, try HELO */
    if (code > 399)
    {
        sprintf(command, "HELO FreescaleTower");
        code = SMTP_send_command(socket, command, response, SMTP_RESPONSE_BUFFER_SIZE);
        if (code != 399)
        {
            SET_ERR_STR(err_string, response, err_string_size);
            SMTP_cleanup(socket, response, command, NULL);
            return(SMTP_WRONG_RESPONSE);
        }
    }
    /* Try to determine if authentication is supported, authenticate if needed */

    location = strstr(response, "AUTH");
    if ((location != NULL) && strstr(location, "LOGIN") && (params->login != NULL))
    {
        char *b64_data = NULL;
        uint32_t b64_length = 0;

        /* Send AUTH command */
        sprintf(command, "AUTH LOGIN");
        code = SMTP_send_command(socket, command, response, SMTP_RESPONSE_BUFFER_SIZE);
        if ((code > 399) || (code == 0))
        {
            SET_ERR_STR(err_string, response, err_string_size);
            SMTP_cleanup(socket, response, command, NULL);
            return(SMTP_WRONG_RESPONSE);
        }

        /* Send base64 encoded username */
        b64_length = (strlen(params->login) / 3) * 4 + ((strlen(params->login) % 3) ? (1) : (0)) + 1;
        b64_data = (char *) _mem_alloc_system(sizeof(char)*b64_length);
        if (b64_data == NULL)
        {
            SMTP_cleanup(socket, response, command, NULL);
            return(MQX_OUT_OF_MEMORY);
        }
        sprintf(command, "%s", base64_encode(params->login, b64_data));
        _mem_free(b64_data);
        code = SMTP_send_command(socket, command, response, SMTP_RESPONSE_BUFFER_SIZE);
        if ((code > 399) || (code == 0))
        {
            SET_ERR_STR(err_string, response, err_string_size);
            SMTP_cleanup(socket, response, command, NULL);
            return(SMTP_WRONG_RESPONSE);
        }

        /* Send base64 encoded password */
        b64_length = (strlen(params->login) / 3) * 4 + ((strlen(params->pass) % 3) ? (1) : (0)) + 1;
        b64_data = (char *) _mem_alloc_system(sizeof(char)*b64_length);
        if (b64_data == NULL)
        {
            SMTP_cleanup(socket, response, command, NULL);
            return(MQX_OUT_OF_MEMORY);
        }
        sprintf(command, "%s", base64_encode(params->pass, b64_data));
        _mem_free(b64_data);
        code = SMTP_send_command(socket, command, response, SMTP_RESPONSE_BUFFER_SIZE);
        if ((code > 299) || (code == 0))
        {
            SET_ERR_STR(err_string, response, err_string_size);
            SMTP_cleanup(socket, response, command, NULL);
            return(SMTP_WRONG_RESPONSE);
        }
    }
    /* Send Email */
    sprintf(command, "MAIL FROM:<%s>", params->envelope.from);
    code = SMTP_send_command(socket, command, response, SMTP_RESPONSE_BUFFER_SIZE);
    if ((code > 299) || (code == 0))
    {
        SET_ERR_STR(err_string, response, err_string_size);
        SMTP_cleanup(socket, response, command, NULL);
        return(SMTP_WRONG_RESPONSE);
    }
    sprintf(command, "RCPT TO:<%s>", params->envelope.to);
    code = SMTP_send_command(socket, command, response, SMTP_RESPONSE_BUFFER_SIZE);
    
    /* Mail receiver not OK nor server will forward */
    if ((code > 299) || (code == 0))
    {
        SET_ERR_STR(err_string, response, err_string_size);
        SMTP_cleanup(socket, response, command, NULL);
        return(SMTP_WRONG_RESPONSE);
    }
    
    /* Send message data */
    sprintf(command, "DATA");
    code = SMTP_send_command(socket, command, response, SMTP_RESPONSE_BUFFER_SIZE);
    if ((code > 399) || (code == 0))
    {
        SET_ERR_STR(err_string, response, err_string_size);
        SMTP_cleanup(socket, response, command, NULL);
        return(SMTP_WRONG_RESPONSE);
    }
    /* Send email text */
    code = SMTP_send_string(socket, params->text);
    
    /* Send terminating sequence for DATA command */
    code = SMTP_send_command(socket, "\r\n.", response, SMTP_RESPONSE_BUFFER_SIZE);
    if ((code > 299) || (code == 0))
    {
        SET_ERR_STR(err_string, response, err_string_size);
        SMTP_cleanup(socket, response, command, NULL);
        return(SMTP_WRONG_RESPONSE);
    }
   
    /* Write response to user buffer */
    SET_ERR_STR(err_string, response, err_string_size);
    
    /* Disconnect from server */
    sprintf(command, "QUIT");
    code = SMTP_send_command(socket, command, response, SMTP_RESPONSE_BUFFER_SIZE);
    
    /* Cleanup */
    SMTP_cleanup(socket, response, command, NULL);
    return(SMTP_OK);
}

/*
** Function for reading numeric server response to command
** IN:
**      char* response - response string from server. 
**
** OUT:
**      none
**
** Return value:
**      uint32 - numeric response code if valid, zero otherwise
*/
static uint32_t SMTP_get_response_code(char *response)
{
    char code_str[] = "000";
    if (response != NULL)
    {
        strncpy(code_str, response, strlen(code_str));
    }
    return (strtol(code_str, NULL, 10));
}

/*
** Function for sending string to SMTP server
** IN:
**      int socket - socket used for communication with server.
**      char* s- string to send. 
**
** OUT:
**      none
**
** Return value:
**      uint32 - number of bytes send
*/
static uint32_t SMTP_send_string(uint32_t socket, char *s)
{
    uint32_t               send_total = 0;
    int32_t                send_step;
    char                   *line = NULL;
    uint32_t               line_length = 0;
    char                   *last_loc = s;
    uint32_t               last_length = 0;
    int                    dot = '.';
    SMTP_FIND_LINE_CONTEXT context;

    if (s == NULL) return(0);

    SMTP_findline_init(&context); 

    /* Send all lines of text */
    while (SMTP_findline(s, &line, &line_length, &context))
    {
        /* If first character is dot, send another dot to ensure email transparency */
        /* See RFC 5321 section 4.5.2 for details why this must be done */
        if (line[0] == '.')
        {
            send_step = send(socket, &dot, 1, 0);
        }
        send_step = send(socket, line, line_length, 0);
        if (send_step > 0)
        {
            send_total += send_step;
        }
        else
        {
            break;
        }
        last_loc = line;
        last_length = line_length;
    }

    /* Send rest which might not end with \n\r sequence */
    if (send_step > 0)
    {
        if (send_total < strlen(s))
        {
            send_step = send(socket, last_loc + last_length, strlen(s) - send_total, 0);
            if (send_step > 0)
            {
                send_total += send_step;
            }
        }
    }

    return(send_total);
}

/*
** Function for sending single command to SMTP server
** IN:
**      int socket - socket used for communication with server.
**      char* command - command to send. 
**      char* response - response string from server
**      uint32_t max_size - size of response buffer
**
** OUT:
**      char **- pointer to string in which full server response will be saved (can be NULL).
**
** Return value:
**      uint32 - numeric response value
*/
static uint32_t SMTP_send_command (uint32_t socket, char *command, char *response, uint32_t max_size)
{
    char *out_string;
    uint32_t rec_len = 0;

    if ((response == NULL) || (command == NULL))
    {
        return(0);
    }
    /* Allocate buffer for output text */
    out_string = (char *) _mem_alloc_system_zero(strlen(command)+3);
    if (out_string == NULL)
    {
        return(0);
    }
    /* Add terminating sequence and send command to server */
    sprintf(out_string, "%s\r\n", command);
    send(socket, out_string, strlen(out_string), 0);
    /* Read response */
    rec_len = recv(socket, response, max_size, 0);
    response[rec_len] = '\0';
    /* Cleanup and return */
    _mem_free(out_string);
    return(SMTP_get_response_code(response));
}

/*
** Function for connecting to to SMTP server.
** IN:
**      char *server - server to connect to. 
**
** OUT:
**      none
**
** Return value:
**      uint32 - socket created and connected to server on port 25 or zero.
*/

static uint32_t SMTP_connect (struct sockaddr* server)
{
    int32_t   retval = 0;
    uint32_t  sfd = 0;

    /* Create socket */
    sfd = socket(server->sa_family, SOCK_STREAM, 0);
    if (sfd == RTCS_SOCKET_ERROR)
    {
        return(0);
    }
    /* Set port for connection */
    switch(server->sa_family)
    {
        case AF_INET:
            ((struct sockaddr_in*) server)->sin_port = RTCS_SMTP_PORT;
            break;
        case AF_INET6:
            ((struct sockaddr_in6*) server)->sin6_port = RTCS_SMTP_PORT;
            break;
    }
    /* Connect socket */
    retval = connect(sfd, server, sizeof(*server));
    if (retval != RTCS_OK)
    {
        struct linger l_options;

        fprintf(stderr, "SMTPClient - Connection failed. Error: 0x%X\n", retval);

        /* Set linger options for RST flag sending. */
        l_options.l_onoff = 1;
        l_options.l_linger_ms = 0;
        setsockopt(sfd, SOL_SOCKET, SO_LINGER, &l_options, sizeof(l_options));
        closesocket(sfd);
        return(0);
    }
    return(sfd);
}

/*
** Function for SMTP cleanup - free memory, close sockets etc.
** IN:
**      int socket - Socket to shutdown.
**      pointer a ... - Pointers to deallocate.
**
** OUT:
**      none
**
** Return value:
**      None
*/

static void SMTP_cleanup(uint32_t socket, void *a, ...)
{
    va_list ap;
    /* Close socket */
    if (socket != 0)
    {
        closesocket(socket);
    }
    
    /* Free pointers */
    va_start(ap, a);
    while(a != NULL)
    {
        _mem_free(a);
        a = va_arg(ap, void *);
    }
    va_end(ap);
}

/*
** Function for line searching lines in strings. After each call pointer to next line start
** is returned. When no next line can be found NULL is returned.
**
** IN:
**      char *s - email text to search. 
**
** OUT:
**      char *line_start - pointer to start of line 
**      uint32_t *line_length - pointer to variable i which length of line should be saved
**
** Return value:
**      char *- pointer to start of line
*/

static char *SMTP_findline(char *s, char **line_start, uint32_t *line_length, SMTP_FIND_LINE_CONTEXT* context)
{
    char *line_end;

    /* Check parameters */
    if (line_length == NULL)
    {
        return(NULL);
    }
    /* First run on string */
    if (!context->first)
    {
        context->first = TRUE;
        context->last_start = s;
        context->last_end = s;
        *line_start = s;
    }
    else
    {
        *line_start = context->last_end;
    }
    /* Find line end */
    line_end = strstr(*line_start, "\n\r");
    /* If end of string is reached */
    if (line_end == NULL) 
    {
        *line_start = NULL;
        *line_length = 0;
        context->first = FALSE;
    }
    else
    {
        line_end += 2;
        *line_length = line_end - *line_start;
    }
    /* Update line ending position */
    context->last_end = line_end;
    return(*line_start);
}

/*
 * Initialize context for line searching.
 */

static void SMTP_findline_init(SMTP_FIND_LINE_CONTEXT* context)
{
    context->last_start = NULL;
    context->last_end = NULL;
    context->first = FALSE;
}


