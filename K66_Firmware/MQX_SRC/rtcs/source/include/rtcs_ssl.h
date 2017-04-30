#ifndef __rtcs_ssl_h__
#define __rtcs_ssl_h__

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
*   SSL adaptation layer for RTCS
*
*
*END************************************************************************/

#include <stdint.h>

typedef enum rtcs_ssl_init_type
{
    RTCS_SSL_SERVER,
    RTCS_SSL_CLIENT
}RTCS_SSL_INIT_TYPE;

typedef struct rtcs_ssl_params_struct
{
    char*              cert_file;       /* Client or Server Certificate file.*/
    char*              priv_key_file;   /* Client or Server private key file.*/
    char*              ca_file;         /* CA (Certificate Authority) certificate file.*/
    RTCS_SSL_INIT_TYPE init_type;
}RTCS_SSL_PARAMS_STRUCT;

#ifdef __cplusplus
extern "C" {
#endif

extern void* RTCS_ssl_init(RTCS_SSL_PARAMS_STRUCT *params);
extern void RTCS_ssl_release(void *ctx);
extern uint32_t RTCS_ssl_socket(void* ctx, uint32_t sock);
extern uint32_t RTCS_ssl_shutdown(uint32_t ssl_sock);
extern int32_t RTCS_ssl_recv(uint32_t ssl_sock, void *buf, uint32_t len, uint32_t flags);
extern int32_t RTCS_ssl_send(uint32_t ssl_sock, void *buf, uint32_t len, uint32_t flags);

#ifdef __cplusplus
}
#endif

#endif /* __rtcs_ssl_h__ */
