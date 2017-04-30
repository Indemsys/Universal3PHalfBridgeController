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
*   HTTPSRV support functions.
*
*
*END************************************************************************/

#include "httpsrv.h"
#include "httpsrv_prv.h"
#include "httpsrv_supp.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
#include <lwmsgq.h>
#include "rtcs_ssl.h"
#include "rtcs_base64.h"
#include "rtcs_util.h"

#if MQX_USE_IO_OLD
#include <fio.h>
#include <posix.h>
#else
#include <nio.h>
#include "errno.h"
#include <strings.h>
#endif

#define HTTPSRV_CALLBACK_TASK_NAME "HTTP server callback handler"

/*
* string table item
*/
typedef struct httpsrv_table_row
{
    int  id;
    char *str;
}HTTPSRV_TABLE_ROW;

/* Structure defining MIME types table row */
typedef struct httpsrv_content_table_row
{
    uint32_t length;         /* extension string length (strlen) */
    char*    ext;            /* extension string */
    int      content_type;   /* extension MIME type string */
    bool     use_cache;      /* cache use */
}HTTPSRV_CONTENT_TABLE_ROW;

/*
* content type
*/
static const HTTPSRV_TABLE_ROW content_type[] = {
        { HTTPSRV_CONTENT_TYPE_PLAIN,       "text/plain" },
        { HTTPSRV_CONTENT_TYPE_HTML,        "text/html" },
        { HTTPSRV_CONTENT_TYPE_CSS,         "text/css" },
        { HTTPSRV_CONTENT_TYPE_GIF,         "image/gif" },
        { HTTPSRV_CONTENT_TYPE_JPG,         "image/jpeg" },
        { HTTPSRV_CONTENT_TYPE_PNG,         "image/png" },
        { HTTPSRV_CONTENT_TYPE_SVG,         "image/svg+xml"},
        { HTTPSRV_CONTENT_TYPE_JS,          "application/javascript" },
        { HTTPSRV_CONTENT_TYPE_XML,         "application/xml" },
        { HTTPSRV_CONTENT_TYPE_ZIP,         "application/zip" },
        { HTTPSRV_CONTENT_TYPE_PDF,         "application/pdf" },
        { HTTPSRV_CONTENT_TYPE_OCTETSTREAM, "application/octet-stream" },
        { 0,                                0}
};

/*
* Response status to reason conversion table
*/
static const HTTPSRV_TABLE_ROW reason_phrase[] = {
    { HTTPSRV_CODE_CONTINUE, "Continue"},
    { HTTPSRV_CODE_UPGRADE, "Switching Protocols"},
    { HTTPSRV_CODE_OK, "OK"},
    { HTTPSRV_CODE_CREATED, "Created"},
    { HTTPSRV_CODE_ACCEPTED, "Accepted"},
    { HTTPSRV_CODE_NON_AUTHORITATIVE, "Non-Authoritative Information"},
    { HTTPSRV_CODE_NO_CONTENT, "No Content"},
    { HTTPSRV_CODE_RESET_CONTENT, "Reset Content"},
    { HTTPSRV_CODE_PARTIAL_CONTENT, "Partial Content"},
    { HTTPSRV_CODE_MULTIPLE_CHOICES, "Multiple Choices"},
    { HTTPSRV_CODE_MOVED_PERMANENTLY, "Moved Permanently"},
    { HTTPSRV_CODE_FOUND, "Found"},
    { HTTPSRV_CODE_SEE_OTHER, "See Other"},
    { HTTPSRV_CODE_NOT_MODIFIED, "Not Modified"},
    { HTTPSRV_CODE_USE_PROXY, "Use Proxy"},
    { HTTPSRV_CODE_TEMPORARY_REDIRECT, "Temporary Redirect"},
    { HTTPSRV_CODE_BAD_REQ, "Bad Request"},
    { HTTPSRV_CODE_UNAUTHORIZED, "Unauthorized"},
    { HTTPSRV_CODE_PAYMENT_REQUIRED, "Payment Required"},
    { HTTPSRV_CODE_FORBIDDEN, "Forbidden"},
    { HTTPSRV_CODE_NOT_FOUND, "Not Found"},
    { HTTPSRV_CODE_METHOD_NOT_ALLOWED, "Method Not Allowed"},
    { HTTPSRV_CODE_NOT_ACCEPTABLE, "Not Acceptable"},
    { HTTPSRV_CODE_PROXY_AUTH_REQUIRED, "Proxy Authentication Required"},
    { HTTPSRV_CODE_REQUEST_TIMEOUT, "Request Time-out"},
    { HTTPSRV_CODE_CONFLICT, "Conflict"},
    { HTTPSRV_CODE_GONE, "Gone"},
    { HTTPSRV_CODE_NO_LENGTH, "Length Required"},
    { HTTPSRV_CODE_PRECONDITION_FAILED, "Precondition Failed"},
    { HTTPSRV_CODE_ENTITY_TOO_LARGE, "Request Entity Too Large"},
    { HTTPSRV_CODE_URI_TOO_LONG, "Request-URI Too Large"},
    { HTTPSRV_CODE_UNSUPPORTED_MEDIA, "Unsupported Media Type"},
    { HTTPSRV_CODE_RANGE_NOT_SATISFIABLE, "Requested range not satisfiable"},
    { HTTPSRV_CODE_EXPECTATION_FAILED, "Expectation Failed"},
    { HTTPSRV_CODE_UPGRADE_REQUIRED, "Upgrade Required"},
    { HTTPSRV_CODE_FIELD_TOO_LARGE, "Request Header Fields Too Large"},
    { HTTPSRV_CODE_INTERNAL_ERROR, "Internal Server Error"},
    { HTTPSRV_CODE_NOT_IMPLEMENTED, "Not Implemented"},
    { HTTPSRV_CODE_BAD_GATEWAY, "Bad Gateway"},
    { HTTPSRV_CODE_SERVICE_UNAVAILABLE, "Service Unavailable"},
    { HTTPSRV_CODE_GATEWAY_TIMEOUT, "Gateway Time-out"},
    { HTTPSRV_CODE_VERSION_NOT_SUPPORTED, "HTTP Version not supported"},
    { 0,   0 }
};

/*
** Extension -> content type conversion table.
** This table rows MUST be ordered by size and alphabetically
** so we can list through it quickly
*/
static const HTTPSRV_CONTENT_TABLE_ROW content_tbl[] = {
    /* Size,          extension, MIME type,                        Cache? */
    {sizeof("js")-1 ,   "js",    HTTPSRV_CONTENT_TYPE_JS,          TRUE},
    {sizeof("css")-1,   "css",   HTTPSRV_CONTENT_TYPE_CSS,         TRUE},
    {sizeof("gif")-1,   "gif",   HTTPSRV_CONTENT_TYPE_GIF,         TRUE}, 
    {sizeof("htm")-1,   "htm",   HTTPSRV_CONTENT_TYPE_HTML,        TRUE},
    {sizeof("jpg")-1,   "jpg",   HTTPSRV_CONTENT_TYPE_JPG,         TRUE},
    {sizeof("pdf")-1,   "pdf",   HTTPSRV_CONTENT_TYPE_PDF,         FALSE}, 
    {sizeof("png")-1,   "png",   HTTPSRV_CONTENT_TYPE_PNG,         TRUE},
    {sizeof("svg")-1,   "svg",   HTTPSRV_CONTENT_TYPE_SVG,         TRUE},
    {sizeof("txt")-1,   "txt",   HTTPSRV_CONTENT_TYPE_PLAIN,       FALSE},
    {sizeof("xml")-1,   "xml",   HTTPSRV_CONTENT_TYPE_XML,         FALSE},
    {sizeof("zip")-1,   "zip",   HTTPSRV_CONTENT_TYPE_ZIP,         FALSE},
    {sizeof("html")-1,  "html",  HTTPSRV_CONTENT_TYPE_HTML,        TRUE},
    {sizeof("shtm")-1,  "shtm",  HTTPSRV_CONTENT_TYPE_HTML,        FALSE},
    {sizeof("shtml")-1, "shtml", HTTPSRV_CONTENT_TYPE_HTML,        FALSE},
    /* Following row MUST have length set to zero so we have proper array termination */
    {0,                      "", HTTPSRV_CONTENT_TYPE_OCTETSTREAM, FALSE}
};

static uint32_t httpsrv_sendextstr(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session, uint32_t length);
static void httpsrv_print(HTTPSRV_SESSION_STRUCT *session, char* format, ...);
static char* httpsrv_get_table_str(HTTPSRV_TABLE_ROW *table, const int32_t id);
static int httpsrv_get_table_int(HTTPSRV_TABLE_ROW *table, char* str);
static void httpsrv_process_file_type(char* extension, HTTPSRV_SESSION_STRUCT* session);
static uint32_t httpsrv_set_params(HTTPSRV_STRUCT *server, HTTPSRV_PARAM_STRUCT* params);
static uint32_t httpsrv_init_socket(HTTPSRV_STRUCT *server, uint16_t family);
static uint32_t httpsrv_basic_auth(char *auth_string, char **user_ptr, char **pass_ptr);
static void* httpsrv_ws_alloc(HTTPSRV_SESSION_STRUCT *session);

#if MQX_USE_IO_OLD == 0
size_t httpsrv_fsize(FILE *file)
{
    fseek (file, 0, SEEK_END);
    return ftell(file);
}
#endif

/*
** Send extended string to socket (dynamic web pages).
**
** IN:
**      HTTPSRV_STRUCT         *server - server structure.
**      HTTPSRV_SESSION_STRUCT *session - session for sending.
**      char                   *str - string to send.
**      uint32_t                length - length of source string.
**
** OUT:
**      none
**
** Return Value:
**      int - number of bytes processed.
*/
static uint32_t httpsrv_sendextstr(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session, uint32_t length)
{
    char * src;
    char * t_buffer;
    uint32_t n;
    uint32_t retval;

    RTCS_ASSERT(server != NULL);
    RTCS_ASSERT(session != NULL);

    t_buffer = session->response.script_buffer;
    src = session->buffer.data + session->buffer.offset;
    
    n = strlen(t_buffer);

    if (n == 0) /* Start searching for script token. */
    {
        uint32_t n_send;
        uint32_t max;
        uint32_t i;
        char     token[] = "<%";

        i = 0;
        max = length;
        
        for (n_send = 0; (n_send < max) && (token[i]); n_send++)
        {
            if (src[n_send] == token[i])
            {
                t_buffer[i] = token[i];
                i++;
                max = ((n_send+2) < length) ? n_send+2 : length;
            }
            else
            {
                i = 0;
                max = length;
                session->response.script_buffer[0] = 0;
            }
        }
        session->buffer.offset = n_send - i;
        retval = i;
    }
    else if (n == 1) /* There was already the less-than sign.*/
    {
        if (*src == '%')
        {
            /* There is script token spanning over two buffers. */
            t_buffer[n] = *src;
            retval = 1;
        }
        else
        {
            /* There was no script token, send missing less-than sign. */
            httpsrv_send(session, session->response.script_buffer, n, 0);
            memset(session->response.script_buffer, 0, sizeof(session->response.script_buffer));
            retval = 0;
        }
    }
    else /* Script token beginning found, find end. */
    {
        uint32_t name_length;
        char     fname[HTTPSRVCFG_MAX_SCRIPT_LN] = {0};
        uint32_t term_length;

        name_length = strcspn(src, " ;%<>\r\n\t\f");

        if ((src[name_length] == '%') && (src[name_length + 1] == '>'))
        {
            term_length = 2;
        }
        else
        {
            term_length = 1;
        }

        if ((name_length > 1) && (name_length < HTTPSRVCFG_MAX_SCRIPT_LN))
        {
            HTTPSRV_SCRIPT_MSG message;

            snprintf(fname, name_length+1, "%s", src);
            /* Form up message for handler task and send it. */
            message.session = session;
            message.type = HTTPSRV_SSI_CALLBACK;
            message.name = fname;
            message.ses_tid = _task_get_id();
            _lwmsgq_send(server->script_msgq, (_mqx_max_type*) &message, LWMSGQ_SEND_BLOCK_ON_FULL);
            /* Wait until SSI is processed. */
            _task_block();
            memset(session->response.script_buffer, 0, sizeof(session->response.script_buffer));
        }
        retval = name_length + term_length;
    }
    return(retval);
}

/*
** Read data from HTTP server.
**
** First copy data from session buffer if there are any and then read rest from socket if required.
**
** IN:
**      HTTPSRV_SESSION_STRUCT *session - session to use for reading.
**      char                   *dst - user buffer to read to.
**      int32_t               len - size of user buffer.
**
** OUT:
**      none
**
** Return Value:
**      int - number of bytes read.
*/
int32_t httpsrv_read(HTTPSRV_SESSION_STRUCT *session, char *dst, int32_t len)
{
    int read = 0;
    uint32_t data_size = session->buffer.offset;

    RTCS_ASSERT(dst != NULL);
    RTCS_ASSERT(session != NULL);

    /* If there are any data in buffer copy them to user buffer */
    if (data_size > 0)
    {
        uint32_t length = (data_size < len) ? data_size : len;
        uint32_t tail = HTTPSRV_SES_BUF_SIZE_PRV-length;
        
        _mem_copy(session->buffer.data, dst, length);
        memmove(session->buffer.data, session->buffer.data+length, tail);
        _mem_zero(session->buffer.data+tail, length);
        session->buffer.offset -= length;
        read = length;
    }

    /* If there is some space remaining in user buffer try to read from socket */
    while (read < len)
    {
        uint32_t received;
        
        received = httpsrv_recv(session, dst+read, len-read, 0);
        if ((received != 0) && ((uint32_t)RTCS_ERROR != received))
        {
            read += received;
        }
        else
        {
            break;
        }
    }
    
    return(read);
}

/*
 * Receive data from socket.
 */
uint32_t httpsrv_recv(HTTPSRV_SESSION_STRUCT *session, char *buffer, uint32_t length, uint32_t flags)
{
    RTCS_ASSERT(session != NULL);
    RTCS_ASSERT(buffer != NULL);    

    #if RTCSCFG_ENABLE_SSL
    if (session->ssl_sock != 0)
    {
        return RTCS_ssl_recv(session->ssl_sock, buffer, length, flags);
    }    
    #endif
    return recv(session->sock, buffer, length, flags);
}

/*
 * Send data to socket
 */
uint32_t httpsrv_send(HTTPSRV_SESSION_STRUCT *session, char *buffer, uint32_t length, uint32_t flags)
{
    RTCS_ASSERT(session != NULL);
    RTCS_ASSERT(buffer != NULL);

    #if RTCSCFG_ENABLE_SSL
    if (session->ssl_sock != 0)
    {
        return RTCS_ssl_send(session->ssl_sock, buffer, length, flags);
    }    
    #endif
    return send(session->sock, buffer, length, flags);
}

/*
** Write data to buffer. If buffer is full during write flush it to client.
**
** IN:
**      HTTPSRV_SESSION_STRUCT *session - session used for write.
**      char*                   src     - pointer to data to send.
**      int32_t                length     - length of data in bytes.
**
** OUT:
**      none
**
** Return Value:
**      int32_t - number of bytes written.
*/
int32_t httpsrv_write(HTTPSRV_SESSION_STRUCT *session, char *src, int32_t length)
{
    uint32_t space = HTTPSRV_SES_BUF_SIZE_PRV - session->buffer.offset;
    int32_t retval = length;
    uint32_t n_send;

    RTCS_ASSERT(session != NULL);
    RTCS_ASSERT(src != NULL);

    /* User buffer is bigger than session buffer - send user data directly */
    if (length > HTTPSRV_SES_BUF_SIZE_PRV)
    {
        /* If there are some data already buffered send them to client first */
        if (httpsrv_ses_flush(session) == RTCS_ERROR)
        {
            return(RTCS_ERROR);
        }
        else
        {
            return(httpsrv_send(session, src, length, 0));
        }
    }

    /* No space in buffer - make some */
    if 
        (
            (space < length) && 
            (httpsrv_ses_flush(session) == RTCS_ERROR)
        )
    {
        return(RTCS_ERROR);
    }

    /* Now we can save user data to buffer and eventually send them to client */
    space = HTTPSRV_SES_BUF_SIZE_PRV - session->buffer.offset;
    n_send = (space < length) ? space : length;
    _mem_copy(src, session->buffer.data+session->buffer.offset, n_send);
    session->buffer.offset += n_send;
    
    if (session->buffer.offset >= HTTPSRV_SES_BUF_SIZE_PRV)
    {
        if (httpsrv_ses_flush(session) == RTCS_ERROR)
        {
            return(RTCS_ERROR);
        }
        else
        {
            retval = n_send;
        }
    }

    return(retval);
}

/*
** Send data from session buffer to client.
**
** IN:
**      HTTPSRV_SESSION_STRUCT *session - session to use.
**
** OUT:
**      none
**
** Return Value:
**      int - number of bytes send.
*/
int32_t httpsrv_ses_flush(HTTPSRV_SESSION_STRUCT *session)
{ 
    int32_t  send_total;
    uint32_t data_length;
    char     *data;
    
    RTCS_ASSERT(session != NULL);

    send_total = 0;
    data_length = session->buffer.offset;
    data = session->buffer.data;
    
    while (send_total < data_length)
    {
        uint32_t send_now;

        send_now = httpsrv_send(session, data+send_total, data_length-send_total, 0);
        if (send_now == (uint32_t) RTCS_ERROR)
        {
            send_total = RTCS_ERROR;
            break;
        }
        send_total += send_now;
    }
    session->buffer.offset = 0;

    return(send_total);
}

/*
** Get string for ID from table.
**
** IN:
**      HTTPSRV_TABLE_ROW  *table - table to be searched
**      int32_t          id - search ID
**
** OUT:
**      none
**
** Return Value:
**      char* - pointer to result. NULL if not found.
*/
static char* httpsrv_get_table_str(HTTPSRV_TABLE_ROW *table, const int32_t id)
{
    HTTPSRV_TABLE_ROW *ptr = table;

    RTCS_ASSERT(ptr != NULL);
    while ((ptr->str) && (id != ptr->id))
    {
        ptr++;
    }
    return (ptr->str);
}

/*
** Get ID for string from table
**
** IN:
**      HTTPSRV_TABLE_ROW     *tbl - table to be searched
**      char*             str - search string
**      uint32_t           len - length of string
**
** OUT:
**      none
**
** Return Value:
**      ID corresponding to searched string. Zero if not found.
*/
static int httpsrv_get_table_int(HTTPSRV_TABLE_ROW *table, char* str)
{
    HTTPSRV_TABLE_ROW *ptr = table;

    RTCS_ASSERT(ptr != NULL);
    while ((ptr->id) && (!strstr(ptr->str, str)))
    {
        ptr++;
    }
    return (ptr->id);
}

/*
** Send HTTP header according to the session response structure.
**
** IN:
**      HTTPSRV_SESSION_STRUCT* session - session used for transmission
**      int32_t                content_len - content length
**      bool                 has_entity - flag indicating if HTTP entity is going to be send following header.
**
** OUT:
**      none
**
** Return Value:
**      none
*/
void httpsrv_sendhdr(HTTPSRV_SESSION_STRUCT *session, int32_t content_len, bool has_entity)
{ 
    char *connection_state;
    char *phrase;
    
    RTCS_ASSERT(session != NULL);

    if (session->flags & HTTPSRV_FLAG_HEADER_SENT)
    {
        return;
    }
    phrase = httpsrv_get_table_str((HTTPSRV_TABLE_ROW*)reason_phrase, session->response.status_code);
    if (phrase == NULL)
    {
        phrase = "-";
    }
    httpsrv_print(session, HTTPSRV_PROTOCOL_STRING" %d %s\r\n", session->response.status_code, phrase);
    
    if (session->flags & HTTPSRV_FLAG_DO_UPGRADE)
    {
        connection_state = "Upgrade";
    }
    else
    {
        connection_state = (session->flags & HTTPSRV_FLAG_IS_KEEP_ALIVE) ? "Keep-Alive" : "close";
    }
    httpsrv_print(session, "Connection: %s\r\n", connection_state);
    
    /* Send WebSocket protocol handshake response if there was WebSocket upgrade request. */
    if (session->request.upgrade_to == HTTPSRV_WS_PROTOCOL)
    {
        httpsrv_print(session, "Upgrade: WebSocket\r\n");
        
        if (session->ws_handshake->version != WS_PROTOCOL_VERSION)
        {
            httpsrv_print(session, "Sec-WebSocket-Version: %d\r\n", WS_PROTOCOL_VERSION);
        }
        if (session->ws_handshake->protocols & WS_CHAT_PROTOCOL_MASK)
        {
            httpsrv_print(session, "Sec-WebSocket-Protocol: %s\r\n", WS_AVAIL_PROTOCOL_STRING);
        }
        if (strlen(session->ws_handshake->accept) == WS_ACCEPT_LENGTH)
        {
            httpsrv_print(session, "Sec-WebSocket-Accept: %s\r\n", session->ws_handshake->accept);
        }
    }
       
    httpsrv_print(session, "Server: %s\r\n", HTTPSRV_PRODUCT_STRING);
    
    /* Check authorization */
    if (session->response.status_code == HTTPSRV_CODE_UNAUTHORIZED)
    {
        httpsrv_print(session, "WWW-Authenticate: Basic realm=\"%s\"\r\n", session->response.auth_realm->name);
    }

    /* If there will be entity body send content type */
    if (has_entity)
    {
        httpsrv_print(session, "Content-Type: %s\r\n", httpsrv_get_table_str((HTTPSRV_TABLE_ROW*)content_type, session->response.content_type));
    }

    if (session->response.status_code != HTTPSRV_CODE_UPGRADE)
    {
        httpsrv_print(session, "Cache-Control: ");
        if (session->flags & HTTPSRV_FLAG_IS_CACHEABLE)
        {
            httpsrv_print(session, "max-age=%d\r\n", HTTPSRVCFG_CACHE_MAXAGE);
        }
        else
        {
            if (session->response.auth_realm != NULL)
            {
                httpsrv_print(session, "no-store\r\n");
            }
            else
            {
                httpsrv_print(session, "no-cache\r\n");
            }
        }
    }
    /* Only non zero length cause sending Content-Length header field */
    if (content_len > 0)
    {
        httpsrv_print(session, "Content-Length: %d\r\n", content_len);
    }

    /* Handle transfer encoding. */
    if (session->flags & HTTPSRV_FLAG_IS_TRANSCODED)
    {
        httpsrv_print(session, "Transfer-Encoding: chunked\r\n");
    }
    /* End of header */
    httpsrv_print(session, "\r\n");
    
    /* Commented out to prevent problems with file system on KHCI USB */
    //if ((content_len == 0) && (!has_entity))
    {
        httpsrv_ses_flush(session);
    }
    session->flags |= HTTPSRV_FLAG_HEADER_SENT;
}

/*
** Print data to session
**
** IN:
**      HTTPSRV_SESSION_STRUCT* session - session used for transmission
**      char*                   format - format for snprintf function
**      void                    ...    - parameters to print
**
** OUT:
**      none
**
** Return Value:
**      none
*/
static void httpsrv_print(HTTPSRV_SESSION_STRUCT *session, char* format, ...)
{
    va_list ap;
    uint32_t req_space;
    char* buffer;
    int buffer_space;

    RTCS_ASSERT(session != NULL);
    req_space = 0;
    buffer = session->buffer.data;
    buffer_space = HTTPSRV_SES_BUF_SIZE_PRV - session->buffer.offset;
    va_start(ap, format);
    /*
    ** First we always test if there is enough space in buffer. If there is not, 
    ** we flush it first and then write.
    */
    req_space = vsnprintf(NULL, 0, format, ap);
    va_end(ap);
 
    va_start(ap, format);
    if (req_space > buffer_space)
    {
        httpsrv_ses_flush(session);
        buffer_space = HTTPSRV_SES_BUF_SIZE_PRV;
    }
    session->buffer.offset += vsnprintf(buffer+session->buffer.offset, buffer_space, format, ap);
    va_end(ap);
}

/*
** Convert file extension to content type and determine what kind of cache control should be used.
**
** IN:
**      char* extension - extension to convert
**
** IN/OUT:
**      HTTPSRV_SESSION_STRUCT* session - session pointer
**
** Return Value:
**      none
*/
static void httpsrv_process_file_type(char* extension, HTTPSRV_SESSION_STRUCT* session)
{
    const HTTPSRV_CONTENT_TABLE_ROW* row = content_tbl;
    uint32_t length = 0;

    RTCS_ASSERT(session != NULL);

    if (extension != NULL)
    {
        length = strlen(extension)-1;
        /* Move pointer after the dot. */
        extension++;
    }
    else
    {
        goto NO_EXT;
    }

    /* List through table rows until length match */
    while ((row->length) && (row->length < length))
    {
        row++;
    }

    /* Do a search in valid rows */
    while (row->length == length)
    {
        if (strcasecmp(extension, row->ext) == 0)
        {
            session->response.content_type = row->content_type;
            if (row->use_cache)
            {
                session->flags |= HTTPSRV_FLAG_IS_CACHEABLE;
            }

            if (session->response.auth_realm != NULL)
            {
                /* If authentication is required, then response MUST NOT be cached */
                session->flags &= ~HTTPSRV_FLAG_IS_CACHEABLE;
            }
            return;
        }
        row++;
    }

    NO_EXT:
    session->response.content_type = HTTPSRV_CONTENT_TYPE_OCTETSTREAM;
    session->flags &= ~HTTPSRV_FLAG_IS_CACHEABLE;
}

/*
** Send file to client
**
** IN:
**      HTTPSRV_SESSION_STRUCT* session - session used for transmission
**      HTTPSRV_STRUCT*         server - server structure
**
** OUT:
**      none
**
** Return Value:
**      none
*/
HTTPSRV_SES_STATE httpsrv_sendfile(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session)
{
    char              *ext;
    int               length;
    char              *buffer;
    HTTPSRV_SES_STATE retval;

    RTCS_ASSERT(server != NULL);
    RTCS_ASSERT(session != NULL);
    buffer = session->buffer.data;
    RTCS_ASSERT(buffer != NULL);

    ext = strrchr(session->request.path, '.');
    httpsrv_process_file_type(ext, session);

    /* Check if file has server side includes */
    if ((0 == strcasecmp(ext, ".shtml")) || (0 == strcasecmp(ext, ".shtm")))
    {
        /* 
         * Disable keep-alive for this session otherwise we would have to 
         * wait for session timeout.
         */
        session->flags &= ~HTTPSRV_FLAG_IS_KEEP_ALIVE;
        httpsrv_sendhdr(session, 0, 1);
        #if MQX_USE_IO_OLD
        fseek(session->response.file, session->response.length, IO_SEEK_SET);
        #else
        fseek(session->response.file, session->response.length, SEEK_SET);
        #endif
        length = fread(buffer+session->buffer.offset, 1, HTTPSRV_SES_BUF_SIZE_PRV-session->buffer.offset, session->response.file);
        if (length > 0)
        {
            uint32_t offset;

            offset = httpsrv_sendextstr(server, session, length);
            session->response.length += offset;
            session->response.length += session->buffer.offset;
            httpsrv_ses_flush(session);
        }
    }
    else
    {
        #if MQX_USE_IO_OLD
        httpsrv_sendhdr(session, session->response.file->SIZE, 1);
        fseek(session->response.file, session->response.length, IO_SEEK_SET);
        #else
        httpsrv_sendhdr(session, httpsrv_fsize(session->response.file), 1);
        fseek(session->response.file, session->response.length, SEEK_SET);
        #endif
        length = fread(buffer+session->buffer.offset, 1, HTTPSRV_SES_BUF_SIZE_PRV-session->buffer.offset, session->response.file);
        if (length > 0)
        {
            session->buffer.offset += length;
            length = httpsrv_ses_flush(session);
            if (length != RTCS_ERROR)
            {
                session->response.length += length;
            }
        }
    }

    if (length <= 0)
    {
        retval = HTTPSRV_SES_END_REQ;
    }
    else
    {
        retval = HTTPSRV_SES_RESP; 
    }
    return(retval);
}

/*
** Send error page to client
**
** IN:
**      HTTPSRV_SESSION_STRUCT* session - session used for transmission
**      const char*             title - title of error page
**      const char*             text - text displayed on error page
**
** OUT:
**      none
**
** Return Value:
**      none
*/
void httpsrv_send_err_page(HTTPSRV_SESSION_STRUCT *session, const char* title, const char* text)
{
    uint32_t length;
    char* page;
    
    RTCS_ASSERT(session != NULL);
    length = snprintf(NULL, 0, ERR_PAGE_FORMAT, title, text);
    length++;
    page = _mem_alloc(length*sizeof(char));

    session->response.content_type = HTTPSRV_CONTENT_TYPE_HTML;

    if (page != NULL)
    {
        snprintf(page, length, ERR_PAGE_FORMAT, title, text);
        httpsrv_sendhdr(session, strlen(page), 1);
        httpsrv_write(session, page, strlen(page));
        httpsrv_ses_flush(session);
        _mem_free(page);
    }
    else
    {
        httpsrv_sendhdr(session, 0, 0);
    }
}

/*
** Process one line of HTTP request header
**
** IN:
**      HTTPSRV_SESSION_STRUCT* session - session structure pointer.
**      HTTPSRV_STRUCT *server - pointer to server structure (needed for session parameters).
**      char* buffer - pointer to beginning of line with request.
**
** OUT:
**      none
**
** Return Value:
**      none
*/
int32_t httpsrv_req_hdr(HTTPSRV_SESSION_STRUCT* session, char* buffer)
{
    char* param_ptr = NULL;
    int32_t retval;

    retval = HTTPSRV_OK;
    RTCS_ASSERT(session != NULL);
    RTCS_ASSERT(buffer != NULL);

    if (strncmp(buffer, "Host: ", 6) == 0)
    {
        
        /*
         * We only do this to check request validity.
         * Virtual hosts are not supported.
         */
        /*
        param_ptr = buffer+6;
        strcpy(session->host, param_ptr);
        */
        session->flags |= HTTPSRV_FLAG_HAS_HOST;
    }
    else if (strncmp(buffer, "Connection: ", 12) == 0)
    {
        param_ptr = buffer+12;

        if 
            (
                (session->flags & HTTPSRV_FLAG_KEEP_ALIVE_ENABLED) &&
                (
                    (strstr(param_ptr, "Keep-Alive")) || 
                    (strstr(param_ptr, "keep-alive"))
                )
            )
        {
            session->flags |= HTTPSRV_FLAG_IS_KEEP_ALIVE;
        }
        else
        {
            session->flags &= ~HTTPSRV_FLAG_IS_KEEP_ALIVE;
        }
        if (strstr(param_ptr, "Upgrade"))
        {
            session->flags |= HTTPSRV_FLAG_DO_UPGRADE;
        }
    }
    else if (strncmp(buffer, "Content-Length: ", 16) == 0)
    {
        unsigned long int value;

        param_ptr = buffer+16;
        value = strtoul(param_ptr, NULL, 10);
        if (value == ULONG_MAX)
        {
            retval = HTTPSRV_ERR;
            goto EXIT;
        }
        session->request.content_length = (uint32_t) value;
        session->flags |= HTTPSRV_FLAG_HAS_CONTENT_LENGTH;
    }
    else if (strncmp(buffer, "Content-Type: ", 14) == 0)
    {
        param_ptr = buffer+14;

        session->request.content_type = httpsrv_get_table_int((HTTPSRV_TABLE_ROW*) content_type, param_ptr);
        /* If content type is unknown, treat it as octet/stream */
        if (session->request.content_type == 0)
        {
            session->request.content_type = HTTPSRV_CONTENT_TYPE_OCTETSTREAM;
        }
    }
    else if (strncmp(buffer, "Upgrade: ", 9) == 0)
    {
        param_ptr = buffer+9;
        if (strncasecmp(param_ptr, "websocket", 9) == 0)
        {
            session->request.upgrade_to = HTTPSRV_WS_PROTOCOL;
            session->ws_handshake = httpsrv_ws_alloc(session);
            if (session->ws_handshake == NULL)
            {
                retval = HTTPSRV_ERR;
                goto EXIT;
            }
        }
    }
    else if (strncmp(buffer, "Authorization: ", 15) == 0)
    {
        param_ptr = buffer+15;
        if (strncmp(param_ptr, "Basic ", 6) == 0)
        {
            char *user;
            char *pass;

            user = NULL;
            pass = NULL;
            if (httpsrv_basic_auth(param_ptr+6, &user, &pass) == HTTPSRV_OK)
            {
                session->request.auth.user_id = user;
                session->request.auth.password = pass;
            }
        }
    }
    else if (strncmp(buffer, "Sec-WebSocket-Key: ", 19) == 0)
    {
        param_ptr = buffer+19;
        if (strlen(param_ptr) == WS_KEY_LENGTH)
        {
            session->ws_handshake = httpsrv_ws_alloc(session);
            if (session->ws_handshake == NULL)
            {
                retval = HTTPSRV_ERR;
                goto EXIT;
            }
            _mem_copy(param_ptr, session->ws_handshake->key, WS_KEY_LENGTH);
        }
    }
    else if (strncmp(buffer, "Sec-WebSocket-Protocol: ", 24) == 0)
    {
        char* substring;

        param_ptr = buffer+24;
        substring = strtok(param_ptr, ", ;\t");
        session->ws_handshake = httpsrv_ws_alloc(session);
        if (session->ws_handshake == NULL)
        {
            retval = HTTPSRV_ERR;
            goto EXIT;
        }
        while (substring != NULL)
        {
            if (strcmp(substring, "chat") == 0)
            {
                session->ws_handshake->protocols |= WS_CHAT_PROTOCOL_MASK;
            }
            else if (strcmp(substring, "superchat") == 0)
            {
                session->ws_handshake->protocols |= WS_SUPERCHAT_PROTOCOL_MASK;
            }
            substring = strtok(NULL, ", ;");
        }
    }
    else if (strncmp(buffer, "Sec-WebSocket-Version: ", 23) == 0)
    {
        param_ptr = buffer+23;
        session->ws_handshake = httpsrv_ws_alloc(session);
        if (session->ws_handshake == NULL)
        {
            retval = HTTPSRV_ERR;
            goto EXIT;
        }
        session->ws_handshake->version = strtoul(param_ptr, NULL, 10);
    }
    EXIT:
    return(retval);
}

static uint32_t httpsrv_basic_auth(char *auth_string, char **user_ptr, char **pass_ptr)
{
    uint32_t decoded_length;
    uint32_t length;
    uint32_t retval;
    char* user;
    char* pass;

    pass = NULL;
    user = NULL;
    *user_ptr = NULL;
    *pass_ptr = NULL;
    retval = HTTPSRV_OK;
    decoded_length = 0;

    length = strlen(auth_string);
    if ((length == 0) || (length % 4))
    {
        retval = HTTPSRV_ERR;
        goto EXIT;
    }
    if (!isbase64((const char *) auth_string))
    {
        retval = HTTPSRV_ERR;
        goto EXIT;
    }
    /* evaluate number of bytes required for worst case (no padding) */
    decoded_length = (length/4) * 3 + 1;
    user = _mem_alloc_zero(sizeof(char)*decoded_length);
    if (user != NULL)
    {
        _mem_set_type(user, MEM_TYPE_HTTPSRV_AUTH);
        base64_decode(user, auth_string, decoded_length);
        *user_ptr = user;
    }
    else
    {
        retval = HTTPSRV_ERR;
        goto EXIT;
    }

    pass = strchr(user, ':');
    if (pass != NULL)
    {
        *pass = '\0';
        pass = pass + 1;
        *pass_ptr = pass;
    }
    EXIT:
    return(retval);
}

/*
** Read HTTP method 
**
** IN:
**      HTTPSRV_SESSION_STRUCT* session - session structure pointer.
**      HTTPSRV_STRUCT *server - pointer to server structure (needed for session parameters).
**      char* buffer - pointer to beginning of line with request.
**
** OUT:
**      none
**
** Return Value:
**      none
*/
int32_t httpsrv_req_line(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session, char* buffer)
{
    char* uri_begin  = NULL;
    char* uri_end = NULL;
    uint32_t written;
    
    RTCS_ASSERT(session != NULL);
    RTCS_ASSERT(server != NULL);
    RTCS_ASSERT(buffer != NULL);

    if (strncmp(buffer, "GET ", 4) == 0)
    {
        session->request.method = HTTPSRV_REQ_GET;
    }
    else if (strncmp(buffer, "POST ", 5) == 0)
    {
        session->request.method = HTTPSRV_REQ_POST;
    }
    else if (strncmp(buffer, "HEAD ", 5) == 0)
    {
        session->request.method = HTTPSRV_REQ_HEAD;
    }
    else /* Unknown method - not implemented response */
    {
        session->response.status_code = HTTPSRV_CODE_NOT_IMPLEMENTED;
        return(HTTPSRV_ERR);
    }

    /* Parse remaining part of line */
    uri_begin = strchr(buffer, ' ');

    if (uri_begin != NULL)
    {
        uri_begin++;
        uri_end = strchr(uri_begin, ' ');
        if (uri_end != NULL)
        {
            *uri_end = '\0';
        }
    }
    else
    {
        session->request.path[0] = '\0';
        session->response.status_code = HTTPSRV_CODE_BAD_REQ;
        return(HTTPSRV_ERR);
    }

    /* Pre-process URI */
    rtcs_url_decode(uri_begin);
    rtcs_url_cleanup(uri_begin);

    written = snprintf(session->request.path, server->params.max_uri, "%s", uri_begin);
    /* Check if whole URI is saved in buffer */
    if (written > server->params.max_uri-1)
    {
        session->request.path[0] = '\0';
        /* URI is too long so we set proper status code for response */
        session->response.status_code = HTTPSRV_CODE_URI_TOO_LONG;
        return(HTTPSRV_ERR);
    }
    return(HTTPSRV_OK);
}

/*
** Get realm for requested path
**
** IN:
**      char*            path - search path.
**      HTTPSRV_STRUCT*  server - pointer to server structure (needed for session parameters).
**
** OUT:
**      none
**
** Return Value: 
**      HTTPSRV_AUTH_REALM_STRUCT* - authentication realm for requested path. Null if not found.
*/
const HTTPSRV_AUTH_REALM_STRUCT* httpsrv_req_realm(HTTPSRV_STRUCT *server, char* path)
{
    const HTTPSRV_AUTH_REALM_STRUCT* table;

    RTCS_ASSERT(server != NULL);
    RTCS_ASSERT(path != NULL);

    table = server->params.auth_table;
    
    if (table == NULL)
    {
        return(NULL);
    }

    while((table->path != NULL) && (strstr(path, table->path) == NULL))
    {
        table++;
    }

    return(table->path ? table : NULL);
}

/*
** Find plugin for selected resource in table.
**
** IN:
**      HTTPSRV_PLUGIN_LINK_STRUCT* table - table to search in.
**      char*                   resource - resource name.
**
** OUT:
**      none
**
** Return Value: 
**      HTTPSRV_PLUGIN_STRUCT* plugin structure. NULL if not found.
*/
HTTPSRV_PLUGIN_STRUCT* httpsrv_get_plugin(const HTTPSRV_PLUGIN_LINK_STRUCT* table, char* resource)
{
    HTTPSRV_PLUGIN_STRUCT* retval = NULL;

    if ((table != NULL) && (resource != NULL))
    {
        while (table->resource != NULL)
        {
            if (strcmp(resource, table->resource) == 0)
            {
                retval = table->plugin;
                break;
            }
            table++;
        }
    }
    return(retval);
}

/*
** Process plugin according to its type.
**
** IN:
**      HTTPSRV_SESSION_STRUCT* session - session structure pointer.
**
** OUT:
**      none
**
** Return Value: 
**      none
*/
HTTPSRV_SES_STATE httpsrv_process_plugin(HTTPSRV_SESSION_STRUCT *session)
{
    HTTPSRV_SES_STATE retval;

    RTCS_ASSERT(session != NULL);
    /* Check plugin type. */
    switch (session->plugin->type)
    {
        case HTTPSRV_WS_PLUGIN:
            retval = HTTPSRV_SES_RESP;
            session->response.status_code = HTTPSRV_CODE_UPGRADE;
            break;
        default:
            retval = HTTPSRV_SES_CLOSE;
            break;
    }
    return(retval);
}

/*
** Check user authentication credentials
**
** IN:
**      HTTPSRV_AUTH_REALM_STRUCT* realm - search realm.
**      HTTPSRV_AUTH_USER_STRUCT*  user - user to authenticate.
**
** OUT:
**      none
**
** Return Value: 
**      int - 1 if user is successfully authenticated, zero otherwise.
*/
int httpsrv_check_auth(const HTTPSRV_AUTH_REALM_STRUCT* realm, const HTTPSRV_AUTH_USER_STRUCT* user)
{
    const HTTPSRV_AUTH_USER_STRUCT* users = NULL;
    
    if ((realm == NULL) || (user == NULL))
    {
        return(0);
    }

    users = realm->users;

    while (users->user_id != NULL)
    {
        if (!strcmp(users->user_id, user->user_id) && !strcmp(users->password, user->password))
        {
            return(1);
        }
        users++;
    }
    return(0);
}

/*
** Internal function for server parameters initialization
**
** IN:
**      HTTPSRV_STRUCT* server - server structure pointer
**
**      HTTPSRV_PARAM_STRUCT* params - pointer to user parameters if there are any
** OUT:
**      none
**
** Return Value: 
**      uint32_t      error code. HTTPSRV_OK if everything went right, positive number otherwise
*/
static uint32_t httpsrv_set_params(HTTPSRV_STRUCT *server, HTTPSRV_PARAM_STRUCT *params)
{
    uint32_t l_root = 0;
    uint32_t l_index = 0;
    char*    temp;

    RTCS_ASSERT(server != NULL);

    server->params.port = HTTPSRVCFG_DEF_PORT;
    #if RTCSCFG_ENABLE_IP4
    server->params.ipv4_address.s_addr = HTTPSRVCFG_DEF_ADDR;
    server->params.af |= AF_INET;
    #endif
    #if RTCSCFG_ENABLE_IP6  
    server->params.ipv6_address = in6addr_any;
    server->params.ipv6_scope_id = 0;
    server->params.af |= AF_INET6;
    #endif
    server->params.max_uri = HTTPSRVCFG_DEF_URL_LEN;
    server->params.max_ses = HTTPSRVCFG_DEF_SES_CNT;
    server->params.root_dir = "tfs:";
    server->params.index_page = HTTPSRVCFG_DEF_INDEX_PAGE;
    server->params.cgi_lnk_tbl = NULL;
    server->params.ssi_lnk_tbl = NULL;
    server->params.server_prio = HTTPSRVCFG_DEF_SERVER_PRIO;
    server->params.script_prio = HTTPSRVCFG_DEF_SERVER_PRIO;
    server->params.script_stack = 0;
    server->params.auth_table = NULL;
    server->params.use_nagle = 0;
    server->params.alias_tbl = NULL;
    #if RTCSCFG_ENABLE_SSL
    server->ssl_ctx = NULL;
    #endif

    /* If there is parameters structure copy nonzero values to server */
    if (params != NULL)
    {
        if (params->port)
            server->params.port = params->port;
        #if RTCSCFG_ENABLE_IP4
        if (params->ipv4_address.s_addr != 0)
            server->params.ipv4_address = params->ipv4_address;
        #endif
        #if RTCSCFG_ENABLE_IP6
        if (params->ipv6_address.s6_addr != NULL)
            server->params.ipv6_address = params->ipv6_address;
        if (params->ipv6_scope_id)
            server->params.ipv6_scope_id = params->ipv6_scope_id;
        #endif
        if (params->af)
            server->params.af = params->af;
        if (params->max_uri)
            server->params.max_uri = params->max_uri;
        if (params->max_ses)
            server->params.max_ses = params->max_ses;
        if (params->root_dir)
        {
            server->params.root_dir = params->root_dir;  
        }
        if (params->index_page)
        {
            server->params.index_page = params->index_page;
        } 
        if (params->cgi_lnk_tbl)
            server->params.cgi_lnk_tbl = params->cgi_lnk_tbl;
        if (params->ssi_lnk_tbl)
            server->params.ssi_lnk_tbl = params->ssi_lnk_tbl;
        if (params->server_prio)
            server->params.server_prio = params->server_prio;
        if (params->script_prio)
            server->params.script_prio = params->script_prio;
        if (params->script_stack)
            server->params.script_stack = (params->script_stack < HTTPSRV_CGI_HANDLER_STACK) ? HTTPSRV_CGI_HANDLER_STACK : params->script_stack;  
        if (params->auth_table)
            server->params.auth_table = params->auth_table;
        if (params->use_nagle)
            server->params.use_nagle = params->use_nagle;
        if (params->alias_tbl)
            server->params.alias_tbl = params->alias_tbl;
        if (params->plugins)
            server->params.plugins = params->plugins;

        if(params->ssl_params)
        {
            #if RTCSCFG_ENABLE_SSL
            RTCS_SSL_PARAMS_STRUCT ssl_params={0};
            
            /* Initialize SSL server.*/
            ssl_params.cert_file = params->ssl_params->cert_file;         /* Server Certificate file.*/
            ssl_params.priv_key_file = params->ssl_params->priv_key_file;  /* Server private key file.*/
            ssl_params.init_type = RTCS_SSL_SERVER;
            
            server->ssl_ctx = RTCS_ssl_init(&ssl_params);
            if(server->ssl_ctx == NULL)
            {
                return(HTTPSRV_ERR);
            }

            /* Set default HTTPS port.*/
            if (params->port == 0)
                server->params.port = IPPORT_HTTPS;
            #else
            return(HTTPSRV_ERR);
            #endif    
        }
    }

    /* Copy root directory and index page to dynamic memory. */
    l_root = strlen(server->params.root_dir); 
    l_index = strlen(server->params.index_page);

    temp = _mem_alloc_zero(l_root+l_index+2);
    if (temp == NULL)
    {
        return(HTTPSRV_ERR);
    }
    _mem_copy(server->params.root_dir, temp, l_root);
    _mem_copy(server->params.index_page, temp+l_root+1, l_index);

    server->params.root_dir = temp; 
    server->params.index_page = temp+l_root+1;
    
    /* If there is some callback table and no handler stack is set, use default stack size */
    if ((server->params.cgi_lnk_tbl || server->params.ssi_lnk_tbl) && !server->params.script_stack)
    {
        server->params.script_stack = HTTPSRV_CGI_HANDLER_STACK;
    }

    return(HTTPSRV_OK);
}

/*
** Function for socket initialization (both IPv4 and IPv6)
**
** IN:
**      HTTPSRV_STRUCT* server - server structure pointer
**
**      uint16_t      family - IP protocol family
** OUT:
**      none
**
** Return Value: 
**      uint32_t      error code. HTTPSRV_OK if everything went right, positive number otherwise
*/
static uint32_t httpsrv_init_socket(HTTPSRV_STRUCT *server, uint16_t family)
{
    uint32_t option;
    uint32_t error;
    sockaddr sin_sock;
    uint32_t sock = 0;
    uint32_t is_error = 0;

    RTCS_ASSERT(server != NULL);

    _mem_zero(&sin_sock, sizeof(sockaddr));
    #if RTCSCFG_ENABLE_IP4
    if (family == AF_INET) /* IPv4 */
    {
       
        if ((server->sock_v4 = socket(AF_INET, SOCK_STREAM, 0)) == (uint32_t)RTCS_ERROR)
        {
            return(HTTPSRV_CREATE_FAIL);
        }
        ((sockaddr_in *)&sin_sock)->sin_port   = server->params.port;
        ((sockaddr_in *)&sin_sock)->sin_addr   = server->params.ipv4_address;
        ((sockaddr_in *)&sin_sock)->sin_family = AF_INET;
        sock = server->sock_v4;
    }
    else
    #endif    
    #if RTCSCFG_ENABLE_IP6   
    if (family == AF_INET6) /* IPv6 */
    {
        if ((server->sock_v6 = socket(AF_INET6, SOCK_STREAM, 0)) == (uint32_t)RTCS_ERROR)
        {
            return(HTTPSRV_CREATE_FAIL);
        }
        ((sockaddr_in6 *)&sin_sock)->sin6_port      = server->params.port;
        ((sockaddr_in6 *)&sin_sock)->sin6_family    = AF_INET6;
        ((sockaddr_in6 *)&sin_sock)->sin6_scope_id  = server->params.ipv6_scope_id;
        ((sockaddr_in6 *)&sin_sock)->sin6_addr      = server->params.ipv6_address;
        sock = server->sock_v6;
    }
    else
    #endif    
    {
        return(HTTPSRV_BAD_FAMILY);
    }
    /* Set socket options */
    option = HTTPSRVCFG_SEND_TIMEOUT;
    is_error = is_error || setsockopt(sock, SOL_TCP, OPT_SEND_TIMEOUT, &option, sizeof(option));
    option = HTTPSRVCFG_CONNECT_TIMEOUT;
    is_error = is_error || setsockopt(sock, SOL_TCP, OPT_CONNECT_TIMEOUT, &option, sizeof(option));
    option = HTTPSRVCFG_TIMEWAIT_TIMEOUT;
    is_error = is_error || setsockopt(sock, SOL_TCP, OPT_TIMEWAIT_TIMEOUT, &option, sizeof(option));
    option = HTTPSRVCFG_RECEIVE_TIMEOUT;     
    is_error = is_error || setsockopt(sock, SOL_TCP, OPT_RECEIVE_TIMEOUT, &option, sizeof(option));
    option = FALSE; 
    is_error = is_error || setsockopt(sock, SOL_TCP, OPT_RECEIVE_NOWAIT, &option, sizeof(option));
    option = TRUE;  
    is_error = is_error || setsockopt(sock, SOL_TCP, OPT_RECEIVE_PUSH, &option, sizeof(option));
    option = !server->params.use_nagle;
    is_error = is_error || setsockopt(sock, SOL_TCP, OPT_NO_NAGLE_ALGORITHM, &option, sizeof(option));
    option = HTTPSRVCFG_TX_BUFFER_SIZE;
    is_error = is_error || setsockopt(sock, SOL_TCP, OPT_TBSIZE, &option, sizeof(option));
    option = HTTPSRVCFG_RX_BUFFER_SIZE;
    is_error = is_error || setsockopt(sock, SOL_TCP, OPT_RBSIZE, &option, sizeof(option));

    if (is_error)
    {
        return(HTTPSRV_SOCKOPT_FAIL);
    }

    /* Bind socket */
    error = bind(sock, &sin_sock, sizeof(sin_sock));
    if(error != RTCS_OK)
    {
        return(HTTPSRV_BIND_FAIL);
    }

    /* Listen */
    error = listen(sock, server->params.max_ses);
    if (error != RTCS_OK)
    {
        return(HTTPSRV_LISTEN_FAIL);
    }
    return(HTTPSRV_OK);
}

/*
** Function for checking of HTTP request validity
**
** IN:
**       HTTPSRV_SESSION_STRUCT *session - session to check
** OUT:
**      none
**
** Return Value: 
**      uint32_t - Status code.
*/
uint32_t httpsrv_req_check(HTTPSRV_SESSION_STRUCT *session)
{
    char               *backslash;
    HTTPSRV_REQ_STRUCT *request;

    RTCS_ASSERT(session != NULL);

    request = &session->request;
    /* If method is not implemented return without request processing */
    if (request->method == HTTPSRV_REQ_UNKNOWN)
    {
        return(HTTPSRV_CODE_NOT_IMPLEMENTED);
    }
    /* We need content length for post requests */
    else if 
        (
            (request->method == HTTPSRV_REQ_POST) && 
            !(session->flags & HTTPSRV_FLAG_HAS_CONTENT_LENGTH)
        )
    {
        return(HTTPSRV_CODE_NO_LENGTH);
    }
    /* Check request path and if there was host field in request header */
    backslash = strrchr(request->path, '/');
    if 
        (
            (backslash == NULL) || 
            !(session->flags & HTTPSRV_FLAG_HAS_HOST)
        )
    {
        /* We have invalid request */
        return(HTTPSRV_CODE_BAD_REQ);
    }
    /* Check if upgrade request is valid */
    if (session->flags & HTTPSRV_FLAG_DO_UPGRADE)
    {
        #if HTTPSRVCFG_WEBSOCKET_ENABLED
        /* Only WebSocket upgrade is supported */
        if 
            (
                (request->upgrade_to != HTTPSRV_WS_PROTOCOL) ||
                (session->ws_handshake->version > WS_PROTOCOL_VERSION) ||
                (strlen(session->ws_handshake->key) != WS_KEY_LENGTH)
            )
        {
            return(HTTPSRV_CODE_BAD_REQ);
        }
        if (session->ws_handshake->version < WS_PROTOCOL_VERSION)
        {
            return(HTTPSRV_CODE_UPGRADE_REQUIRED);
        }
        #else
        return(HTTPSRV_CODE_BAD_REQ);
        #endif
    }
    return(HTTPSRV_CODE_OK);
}

/*
** Function for resolving aliases
**
** IN:
**      HTTPSRV_ALIAS* table - table of aliases
**
**       char* path - path to unalias
** OUT:
**      char** new_root - new root directory for request, NULL if alias is not present
**
** Return Value: 
**      char* - unaliased request path
*/
char* httpsrv_unalias(const HTTPSRV_ALIAS *table, char *path, const char **new_root)
{
    char* retval = path;

    RTCS_ASSERT(new_root != NULL);
    RTCS_ASSERT(path != NULL);

    *new_root = NULL;

    while (table && (table->path != NULL) && (table->alias != NULL))
    {
        if (!strncmp(table->alias, path, strlen(table->alias)))
        {
            *new_root = table->path;
            retval = path + strlen(table->alias);
            break;
        }
        table++;
    }
    return(retval);
}

/*
 * Allocate server structure, init sockets, etc.
 */
HTTPSRV_STRUCT* httpsrv_create_server(HTTPSRV_PARAM_STRUCT* params)
{
    HTTPSRV_STRUCT *server = NULL;
    uint32_t error;
    uint32_t error4 = HTTPSRV_OK;
    uint32_t error6 = HTTPSRV_OK;

    
    if ((server = _mem_alloc_system_zero(sizeof(HTTPSRV_STRUCT))) == NULL)
    {
        return(NULL);
    }
    _mem_set_type(server, MEM_TYPE_HTTPSRV_STRUCT);

    error = _lwsem_create(&server->tid_sem, 1);
    if (error != MQX_OK)
    {
        goto EXIT;
    }

    error = httpsrv_set_params(server, params);
    if (error != HTTPSRV_OK)
    {
        goto EXIT;
    }
    
    /* Allocate space for session pointers */
    server->session = _mem_alloc_zero(sizeof(HTTPSRV_SESSION_STRUCT*) * server->params.max_ses);
    if (server->session == NULL)
    {
        goto EXIT;
    }

    /* Allocate space for session task IDs */
    server->ses_tid = _mem_alloc_zero(sizeof(_rtcs_taskid) * server->params.max_ses);
    if (server->ses_tid == NULL)
    {
        goto EXIT;
    }

    /* Init sockets. */
    if (params->af & AF_INET)
    {
        /* Setup IPv4 server socket */
        error4 = httpsrv_init_socket(server, AF_INET);
    }
    if (params->af & AF_INET6)
    {
        /* Setup IPv6 server socket */
        error6 = httpsrv_init_socket(server, AF_INET6);
    }

    if ((error4 != HTTPSRV_OK) || (error6 != HTTPSRV_OK))
    {
        goto EXIT;
    }

    /* Create script handler if required */
    if ((server->params.cgi_lnk_tbl != NULL) || (server->params.ssi_lnk_tbl != NULL))
    {
        error = RTCS_task_create(HTTPSRV_CALLBACK_TASK_NAME, server->params.script_prio, server->params.script_stack, httpsrv_script_task, server);
        if (error != RTCS_OK)
        {
            goto EXIT;
        }
    }
    return(server);
    
    EXIT:
    httpsrv_destroy_server(server);
    return(server);
}

/*
 * Close sockets, free memory etc.
 */
int32_t httpsrv_destroy_server(HTTPSRV_STRUCT* server)
{
    uint32_t           n = 0;
    bool               wait = FALSE;
    
    RTCS_ASSERT(server != NULL);
    if (server == NULL)
    {
        return(RTCS_ERROR);
    }

    if (server->sock_v4)
    {
        closesocket(server->sock_v4);
    }
    if (server->sock_v6)
    {
        closesocket(server->sock_v6);
    }
    /* Invalidate sessions (this is signal for session tasks to end them) */
    for (n = 0; n < server->params.max_ses; n++)
    {
        if (server->session[n])
        {
            server->session[n]->valid = FALSE;
        }
    }
    /* Wait until all session tasks end */
    do
    {
        wait = FALSE;
        for (n = 0; n < server->params.max_ses; n++)
        {
            if (server->ses_tid[n])
            {
                wait = TRUE;
                break;
            }
        }
        _sched_yield();
    }while(wait);

    /* Shutdown script handler if there is any */
    if (server->script_tid)
    {
        HTTPSRV_SCRIPT_MSG message;

        message.session = NULL;
        message.name = NULL;
        _lwmsgq_send(server->script_msgq, (_mqx_max_type*) &message, LWMSGQ_SEND_BLOCK_ON_FULL);
    }
    while(server->script_tid)
    {
        _sched_yield();
    }
    
    _lwsem_destroy(&server->tid_sem);
    
    #if RTCSCFG_ENABLE_SSL
    if(server->ssl_ctx)
    {
        RTCS_ssl_release(server->ssl_ctx);
    }
    #endif

    /* Free memory */
    _mem_free(server->params.root_dir);
    server->params.root_dir = NULL;
    server->params.index_page = NULL;
    _mem_free((void*) server->ses_tid);
    server->ses_tid = NULL;
    _mem_free(server->session);
    server->session = NULL;
    return(RTCS_OK);
}

/* Get query string */
char * httpsrv_get_query(char* src)
{
    char *retval = NULL;

    retval = strchr(src, '?');
    if (NULL != retval)
    {
        *retval = '\0';
        retval++;
    }
    return(retval);
}

/*
 * Wait for incoming connection, return socket with activity or error.
 */
uint32_t httpsrv_wait_for_conn(HTTPSRV_STRUCT *server)
{
    rtcs_fd_set       rfds;
    int32_t           retval;

    RTCS_ASSERT(server != NULL);

    RTCS_FD_ZERO(&rfds);
    RTCS_FD_SET(server->sock_v4, &rfds);
    RTCS_FD_SET(server->sock_v6, &rfds);

    retval = select(2, &rfds, NULL, NULL, 0);
    if (retval == RTCS_ERROR)
    {
        retval = RTCS_SOCKET_ERROR;
        goto EXIT;
    }

    if(RTCS_FD_ISSET(server->sock_v4, &rfds))
    {
        retval = server->sock_v4;
    }
    else if(RTCS_FD_ISSET(server->sock_v6, &rfds))
    {
        retval = server->sock_v6;
    }
    EXIT:
    return(retval);
}

/*
 * Accept connection from client.
 */
uint32_t httpsrv_accept(uint32_t sock)
{
    struct sockaddr  remote_addr;
    unsigned short   length;

    RTCS_ASSERT(sock != RTCS_SOCKET_ERROR);
    _mem_zero(&remote_addr, sizeof(remote_addr));
    length = sizeof(remote_addr);
    return(accept(sock, (sockaddr *) &remote_addr, &length));
}

/*
 * Abort connection on socket.
 */
void httpsrv_abort(uint32_t sock)
{
    struct linger l_options;

    /* Set linger options for RST flag sending. */
    l_options.l_onoff = 1;
    l_options.l_linger_ms = 0;
    setsockopt(sock, SOL_SOCKET, SO_LINGER, &l_options, sizeof(l_options));
    closesocket(sock);
}

/*
 * Allocate WebSocket handshake structure.
 */
void* httpsrv_ws_alloc(HTTPSRV_SESSION_STRUCT *session)
{
    RTCS_ASSERT(session != NULL);

    if (session->ws_handshake == NULL)
    {
        session->ws_handshake = _mem_alloc_zero(sizeof(WS_HANDSHAKE_STRUCT));
    }
    if (session->ws_handshake == NULL)
    {
        session->response.status_code = HTTPSRV_CODE_INTERNAL_ERROR;
    }
    return(session->ws_handshake);
}
