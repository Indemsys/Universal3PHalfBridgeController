#ifndef __rtcs_smtp_h__
#define __rtcs_smtp_h__
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
*   Simple Mail Transfer Protocol definitions.
*
*
*END************************************************************************/

#include <mqx.h>
#include <rtcs.h>

#define SMTP_OK                   (0)
#define SMTP_ERR_BAD_PARAM        (1)
#define SMTP_ERR_CONN_FAILED      (2)
#define SMTP_WRONG_RESPONSE       (3)
#define SMTP_RESPONSE_BUFFER_SIZE (512)
#define SMTP_COMMAND_BUFFER_SIZE  (128)

#define SET_ERR_STR(x,y,l) if(x != NULL) snprintf(x, l, "%s", y+4);

typedef struct smtp_email_envelope
{
    char    *from;
    char    *to;
}SMTP_EMAIL_ENVELOPE, * SMTP_EMAIL_ENVELOPE_PTR;

typedef struct smtp_param_struct 
{
    SMTP_EMAIL_ENVELOPE envelope;
    char *text;
    struct sockaddr server;
    char *login;
    char *pass;
}SMTP_PARAM_STRUCT, * SMTP_PARAM_STRUCT_PTR;

#ifdef __cplusplus
extern "C" {
#endif

_mqx_int SMTP_send_email (SMTP_PARAM_STRUCT_PTR param, char *err_string, uint32_t err_string_size);

#ifdef __cplusplus
}
#endif

#endif
