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
*   This file contains the RTCS shell SMTP command.
*
*
*END************************************************************************/

#include <string.h>
#include <mqx.h>
#include "shell.h"
#if SHELLCFG_USES_RTCS
#include <rtcs.h>
#include <rtcs_smtp.h>
#include <sh_rtcs.h>

#define DATE_LENGTH 128
#define ERR_MSG_BUFF_SIZE 512

const char *wday[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
const char *months[] =  
{
   "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" 
};

void print_usage(FILE_PTR outstream, uint32_t short_help);

int32_t Shell_smtp (int32_t argc, char *argv[])
{
    struct addrinfo            hints;
    struct addrinfo           *result, *rp;
    int32_t                    retval = 0;
    int32_t                    err_code = 0;
    bool                       print_help;
    bool                       shorthelp = FALSE;
    SMTP_PARAM_STRUCT          params={0};
    char                      *errstr = NULL;
    char                      *server = NULL;
    char                      *optstring = ":f:t:s:u:p:m:j:h";
    int32_t                    next_option;
    char                      *email_text = NULL;
    uint32_t                   email_size = 0;
    TIME_STRUCT                time;
    DATE_STRUCT                date;
    char                       date_str[DATE_LENGTH];
    SHELL_GETOPT_CONTEXT       gopt_context;
    char*                      subject = NULL;

    params.login = NULL;
    params.pass = NULL;

    print_help = Shell_check_help_request(argc, argv, &shorthelp);
    
    if (print_help)
    {
        if (!shorthelp)
        {
            fprintf(stdout,"Usage:"); 
        }
        fprintf(stdout, "%s", argv[0]);
        print_usage(stdout, shorthelp);
        err_code = SHELL_EXIT_SUCCESS;
        return(err_code);
    }
    
    /* Parse command line options */
    Shell_getopt_init(&gopt_context);
    do
    {
        next_option = Shell_getopt(argc, argv, optstring, &gopt_context);
        switch (next_option)
        {
            case 'f':
                params.envelope.from = gopt_context.optarg;
                break;
            case 't':
                params.envelope.to = gopt_context.optarg;
                break;
            case 'm':
                params.text = gopt_context.optarg;
                break;
            case 's':
                server = gopt_context.optarg;
                break;
            case 'u':
                params.login = gopt_context.optarg;
                break;
            case 'p':
                params.pass = gopt_context.optarg;
                break;
            case 'j':
                subject = gopt_context.optarg;
                break;
            case 'h':
                print_usage (stdout, FALSE);
                return(SHELL_EXIT_SUCCESS);
            case '?': /* User specified an invalid option. */
                print_usage(stderr, TRUE);
                return(SHELL_EXIT_ERROR);
            case ':': /* Option has a missing parameter. */
                printf("Option -%c requires a parameter.\n", next_option);
                return(SHELL_EXIT_ERROR);
            case -1: /* Done with options. */
                break;
            default:
                break;
        }
    }while(next_option != -1);
    
    if ((server == NULL) || (params.envelope.from == NULL) || (params.envelope.to == NULL))
    {
        printf("Required parameters not set.\n");
        print_usage(stdout, FALSE);
    }
    
    /* Set sane values for unset parameters. */
    if (subject == NULL)
    {
        subject = "";
    }
    if (params.text == NULL)
    {
        params.text = "";
    }
    
    /* Get time and store it to string in correct format. */
    _time_get(&time);
    _time_to_date(&time,&date);

    snprintf(date_str, DATE_LENGTH, "%s, %d %s %d %02d:%02d:%02d -0000",
        wday[date.WDAY],date.DAY, months[date.MONTH-1],date.YEAR, date.HOUR, date.MINUTE, date.SECOND);

    /* Evaluate email size */
    email_size = strlen(params.envelope.from) + 
                 strlen(params.envelope.to) +
                 strlen(params.text) +
                 strlen(subject) +
                 strlen(date_str) +
                 strlen("From: <>\r\n") +
                 strlen("To: <>\r\n") + 
                 strlen("Subject: \r\n") +
                 strlen("Date: \r\n\r\n") +   
                 strlen("\r\n") + 1;
    /* Allocate space */
    email_text = (char *) _mem_alloc_zero(email_size);
    if (email_text == NULL)
    {
        fprintf(stderr, "  Unable to allocate memory for email message.\n");
        err_code = SHELL_EXIT_ERROR;
        return(err_code);
    }

    /* Prepare email message */
    snprintf(email_text, email_size, "From: <%s>\r\n"
                                     "To: <%s>\r\n"
                                     "Subject: %s\r\n"
                                     "Date: %s\r\n\r\n"
                                     "%s\r\n",
                                     params.envelope.from,
                                     params.envelope.to,
                                     subject,
                                     date_str,
                                     params.text);
    params.text = email_text;
    
    _mem_zero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM; /* TCP socket */
    hints.ai_flags = AI_PASSIVE;     /* For wildcard IP address */
    hints.ai_protocol = 0;           /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    retval = getaddrinfo(server, NULL, &hints, &result);
    if (retval != 0)
    {
        fprintf(stderr, "  getaddrinfo failed. Error: 0x%X\n", retval);
        err_code = SHELL_EXIT_ERROR;
        return(err_code);
    }
    /* Allocate buffer for error message */
    errstr = (char *) _mem_alloc_system_zero(ERR_MSG_BUFF_SIZE);
    /* Try to send email using one of addresses. If it fails try another one. */
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        _mem_copy(rp->ai_addr, &params.server, sizeof(params.server));
        /* Try to send email */
        retval = SMTP_send_email(&params, errstr, ERR_MSG_BUFF_SIZE);
        /* If connection failed try another address */
        if (retval != SMTP_ERR_CONN_FAILED)
        {
            break;
        }
    }
    /* No address succeeded */
    if (rp == NULL)
    {
        fprintf(stderr, "  Unable to connect to %s.\n", server);
        err_code = SHELL_EXIT_ERROR;
    }

    if (retval != SMTP_OK)
    {
        printf("  Email sending failed%s %s\n", (strlen(errstr) > 0) ? ":":".", errstr);
        err_code = SHELL_EXIT_ERROR;
    }
    else
    {
        printf("  Email send. Server response: %s", errstr);
    }
    /* Cleanup */
    freeaddrinfo(result);
    _mem_free(errstr);
    _mem_free(email_text);
    return(err_code);
} 

void print_usage(FILE_PTR outstream, uint32_t short_help)
{
    fprintf (outstream, " -f <sender@email.com> -t <recipient@email.com> -s <www.mail.server.com> [-u <Username>] [-p <Password>] [-j <\"email subject\">] [-m <\"text of email message\"]>\n");
    if (!short_help)
    {
        fprintf (outstream,
        " -f - Sender email address.\n"
        " -t - Recipient email address.\n"
        " -s - Email server that should be used for sending of email.\n"
        " -u - Username for authentication.\n"
        " -p - Password for authentication.\n"
        " -j - Subject of email.\n"
        " -m - Email text.\n"
        " -h - Display this usage information.\n");
    }
    return;
}

#endif
