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
*   HTTPSRV support functions header.
*
*
*END************************************************************************/

#ifndef HTTP_SUPP_H_
#define HTTP_SUPP_H_

#define ERR_PAGE_FORMAT "<HTML><HEAD><TITLE>%s</TITLE></HEAD>\n<BODY><H1>%s</H1>\n</BODY></HTML>\n"
#include "httpsrv_prv.h"
#include "httpsrv.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t httpsrv_read(HTTPSRV_SESSION_STRUCT *session, char *dst, int32_t len);
int32_t httpsrv_write(HTTPSRV_SESSION_STRUCT *session, char *src, int32_t len);
int32_t httpsrv_ses_flush(HTTPSRV_SESSION_STRUCT *session);

void httpsrv_sendhdr(HTTPSRV_SESSION_STRUCT *session, int32_t content_len, bool has_entity);
HTTPSRV_SES_STATE httpsrv_sendfile(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session);
void httpsrv_send_err_page(HTTPSRV_SESSION_STRUCT *session, const char* title, const char* text);

int32_t httpsrv_req_hdr(HTTPSRV_SESSION_STRUCT* session, char* buffer);
int32_t httpsrv_req_line(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session, char* buffer);
uint32_t httpsrv_req_check(HTTPSRV_SESSION_STRUCT *session);

const HTTPSRV_AUTH_REALM_STRUCT* httpsrv_req_realm(HTTPSRV_STRUCT *server, char* path);
int httpsrv_check_auth(const HTTPSRV_AUTH_REALM_STRUCT* realm, const HTTPSRV_AUTH_USER_STRUCT* user);
char* httpsrv_unalias(const HTTPSRV_ALIAS* table, char* path, const char** new_root);
int32_t httpsrv_destroy_server(HTTPSRV_STRUCT* server);
HTTPSRV_STRUCT* httpsrv_create_server(HTTPSRV_PARAM_STRUCT* params);
HTTPSRV_PLUGIN_STRUCT* httpsrv_get_plugin(const HTTPSRV_PLUGIN_LINK_STRUCT* table, char* resource);
HTTPSRV_SES_STATE httpsrv_process_plugin(HTTPSRV_SESSION_STRUCT *session);
uint32_t httpsrv_recv(HTTPSRV_SESSION_STRUCT *session, char *buffer, uint32_t length, uint32_t flags);
uint32_t httpsrv_send(HTTPSRV_SESSION_STRUCT *session, char *buffer, uint32_t length, uint32_t flags);
char * httpsrv_get_query(char* src);
uint32_t httpsrv_wait_for_conn(HTTPSRV_STRUCT *server);
uint32_t httpsrv_accept(uint32_t sock);
void httpsrv_abort(uint32_t sock);
#if MQX_USE_IO_OLD == 0
size_t httpsrv_fsize(FILE *file);
#endif

#ifdef __cplusplus
}
#endif

#endif /* HTTP_SUPP_H_ */
