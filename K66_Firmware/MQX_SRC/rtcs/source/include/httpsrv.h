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
*   Header for HTTPSRV.
*
*
*END************************************************************************/

#ifndef HTTPSRV_H_
#define HTTPSRV_H_

#include <rtcs.h>

/*
** HTTP server status codes
*/
#define HTTPSRV_OK           0
#define HTTPSRV_BIND_FAIL    1 
#define HTTPSRV_LISTEN_FAIL  2
#define HTTPSRV_ERR          3
#define HTTPSRV_CREATE_FAIL  4
#define HTTPSRV_BAD_FAMILY   5
#define HTTPSRV_SOCKOPT_FAIL 6
#define HTTPSRV_SES_INVALID  7
#define HTTPSRV_FAIL         8

#define HTTPSRV_CODE_CONTINUE            (100) /* Continue */
#define HTTPSRV_CODE_UPGRADE             (101) /* Switching Protocols */
#define HTTPSRV_CODE_OK                  (200) /* OK */
#define HTTPSRV_CODE_CREATED             (201) /* Created */
#define HTTPSRV_CODE_ACCEPTED            (202) /* Accepted */
#define HTTPSRV_CODE_NON_AUTHORITATIVE   (203) /* Non-Authoritative Information */
#define HTTPSRV_CODE_NO_CONTENT          (204) /* No Content */
#define HTTPSRV_CODE_RESET_CONTENT       (205) /* Reset Content */
#define HTTPSRV_CODE_PARTIAL_CONTENT     (206) /* Partial Content */
#define HTTPSRV_CODE_MULTIPLE_CHOICES    (300) /* Multiple Choices */
#define HTTPSRV_CODE_MOVED_PERMANENTLY   (301) /* Moved Permanently */
#define HTTPSRV_CODE_FOUND               (302) /* Found */
#define HTTPSRV_CODE_SEE_OTHER           (303) /* See Other */
#define HTTPSRV_CODE_NOT_MODIFIED        (304) /* Not Modified */
#define HTTPSRV_CODE_USE_PROXY           (305) /* Use Proxy */
#define HTTPSRV_CODE_TEMPORARY_REDIRECT  (307) /* Temporary Redirect */
#define HTTPSRV_CODE_BAD_REQ             (400) /* Bad Request */
#define HTTPSRV_CODE_UNAUTHORIZED        (401) /* Unauthorized */
#define HTTPSRV_CODE_PAYMENT_REQUIRED    (402) /* Payment Required */
#define HTTPSRV_CODE_FORBIDDEN           (403) /* Forbidden */
#define HTTPSRV_CODE_NOT_FOUND           (404) /* Not Found */
#define HTTPSRV_CODE_METHOD_NOT_ALLOWED    (405) /* Method Not Allowed */
#define HTTPSRV_CODE_NOT_ACCEPTABLE        (406) /* Not Acceptable */
#define HTTPSRV_CODE_PROXY_AUTH_REQUIRED   (407) /* Proxy Authentication Required */
#define HTTPSRV_CODE_REQUEST_TIMEOUT       (408) /* Request Time-out */
#define HTTPSRV_CODE_CONFLICT              (409) /* Conflict */
#define HTTPSRV_CODE_GONE                  (410) /* Gone */
#define HTTPSRV_CODE_NO_LENGTH             (411) /* Length Required */
#define HTTPSRV_CODE_PRECONDITION_FAILED   (412) /* Precondition Failed */
#define HTTPSRV_CODE_ENTITY_TOO_LARGE      (413) /* Request Entity Too Large */
#define HTTPSRV_CODE_URI_TOO_LONG          (414) /* Request-URI Too Large */
#define HTTPSRV_CODE_UNSUPPORTED_MEDIA     (415) /* Unsupported Media Type */
#define HTTPSRV_CODE_RANGE_NOT_SATISFIABLE (416) /* Requested range not satisfiable */
#define HTTPSRV_CODE_EXPECTATION_FAILED    (417) /* Expectation Failed */
#define HTTPSRV_CODE_UPGRADE_REQUIRED      (426) /* Upgrade Required */
#define HTTPSRV_CODE_FIELD_TOO_LARGE       (431) /* Request Header Fields Too Large */
#define HTTPSRV_CODE_INTERNAL_ERROR        (500) /* Internal Server Error */
#define HTTPSRV_CODE_NOT_IMPLEMENTED       (501) /* Not Implemented */
#define HTTPSRV_CODE_BAD_GATEWAY           (502) /* Bad Gateway */
#define HTTPSRV_CODE_SERVICE_UNAVAILABLE   (503) /* Service Unavailable */
#define HTTPSRV_CODE_GATEWAY_TIMEOUT       (504) /* Gateway Time-out */
#define HTTPSRV_CODE_VERSION_NOT_SUPPORTED (505) /* HTTP Version not supported */

/*
** Authentication types
*/
typedef enum httpstv_auth_type
{
    HTTPSRV_AUTH_INVALID,
    HTTPSRV_AUTH_BASIC,
    HTTPSRV_AUTH_DIGEST /* Not supported yet! */
} HTTPSRV_AUTH_TYPE;

/*
 * HTTP request method type
 */
typedef enum httpsrv_req_method
{
    HTTPSRV_REQ_UNKNOWN,
    HTTPSRV_REQ_GET,
    HTTPSRV_REQ_POST,
    HTTPSRV_REQ_HEAD
} HTTPSRV_REQ_METHOD;

/*
 * Server plugin types
 */
typedef enum httpsrv_plugin_type
{
    HTTPSRV_INVALID_PLUGIN,
    HTTPSRV_WS_PLUGIN
}HTTPSRV_PLUGIN_TYPE;

/*
* HTTP content type
*/
typedef enum httpsrv_content_type
{
    HTTPSRV_CONTENT_TYPE_OCTETSTREAM = 1,
    HTTPSRV_CONTENT_TYPE_PLAIN,
    HTTPSRV_CONTENT_TYPE_HTML,
    HTTPSRV_CONTENT_TYPE_CSS,
    HTTPSRV_CONTENT_TYPE_GIF,
    HTTPSRV_CONTENT_TYPE_JPG,
    HTTPSRV_CONTENT_TYPE_PNG,
    HTTPSRV_CONTENT_TYPE_SVG,
    HTTPSRV_CONTENT_TYPE_JS,
    HTTPSRV_CONTENT_TYPE_ZIP,
    HTTPSRV_CONTENT_TYPE_XML,
    HTTPSRV_CONTENT_TYPE_PDF,
} HTTPSRV_CONTENT_TYPE;

/*
** Authentication user structure
*/
typedef struct httpsrv_auth_user_struct
{
    char* user_id;   /* User ID - usually name*/
    char* password;  /* Password */
}HTTPSRV_AUTH_USER_STRUCT;

/*
** Authentication realm structure
*/
typedef struct httpsrv_auth_realm_struct
{
    const char*                     name;       /* Name of realm. Send to client so user know which login/pass should be used. */
    const char*                     path;       /* Path to file/directory to protect. Relative to root directory */
    const HTTPSRV_AUTH_TYPE         auth_type;  /* Authentication type to use. */
    const HTTPSRV_AUTH_USER_STRUCT* users;      /* Table of allowed users. */
} HTTPSRV_AUTH_REALM_STRUCT;

/*
** CGI request structure. Contains variables specified in RFC3875 (The Common Gateway Interface (CGI) Version 1.1).
** Structure is extended by session handle.
*/
typedef struct httpsrv_cgi_request_struct
{
    uint32_t              ses_handle;         /* Session handle required for various read/write operations*/
    /* 
    *  Following is subset of variables from RFC3875. 
    ** Please see http://tools.ietf.org/html/rfc3875#section-4.1 for details
    */
    HTTPSRV_REQ_METHOD   request_method;     /* Request method (GET, POST, HEAD) see HTTPSRV_REQ_METHOD enum */
    HTTPSRV_CONTENT_TYPE content_type;       /* Content type */
    uint32_t             content_length;     /* Content length */
    uint32_t             server_port;        /* Local connection port */
    char*                remote_addr;        /* Remote client address */
    char*                server_name;        /* Server hostname/IP */
    char*                script_name;        /* CGI name */
    char*                server_protocol;    /* Server protocol name and version (HTTP/1.0) */
    char*                server_software;    /* Server software identification string */
    char*                query_string;       /* Request query string */
    char*                gateway_interface;  /* Gateway interface type and version (CGI/1.1)*/
    char*                remote_user;        /* Remote user name  */
    HTTPSRV_AUTH_TYPE    auth_type;          /* Auth type */
}HTTPSRV_CGI_REQ_STRUCT;

/*
** CGI response struct. This structure is filled by CGI function.
*/
typedef struct httpsrv_cgi_response_struct
{
    uint32_t              ses_handle;              /* Session handle for reading/writing */
    HTTPSRV_CONTENT_TYPE  content_type;            /* Response content type */
    int32_t               content_length;          /* Response content length */
    uint32_t              status_code;             /* Status code (200, 404, etc.)*/
    char*                 data;                    /* Pointer to data to write */
    uint32_t              data_length;             /* Length of data in bytes */
}HTTPSRV_CGI_RES_STRUCT;

/*
** Directory aliases
*/
typedef struct httpsrv_alias
{
    const char* alias;
    const char* path;
}HTTPSRV_ALIAS;

/*
** Server side include parameter structure.
** Passed to user SSI function.
*/
typedef struct httpsrv_ssi_param_struct
{
    uint32_t ses_handle;         /* Session handle for reading/writing */
    char*    com_param;          /* Server side include command parameter (separated from command by ":") */
}HTTPSRV_SSI_PARAM_STRUCT;

/*
** Server side include callback prototype
*/
typedef int32_t(*HTTPSRV_SSI_CALLBACK_FN)(HTTPSRV_SSI_PARAM_STRUCT* param);

/*
** SSI callback link structure
*/
typedef struct httpsrv_ssi_link_struct
{
    char*                   fn_name;            /* Function name */
    HTTPSRV_SSI_CALLBACK_FN callback;           /* Pointer to user function */
    uint32_t                stack;              /* Stack size for SSI. If set to zero, default task will be used */
} HTTPSRV_SSI_LINK_STRUCT;

/*
** CGI callback prototype
*/
typedef int32_t(*HTTPSRV_CGI_CALLBACK_FN)(HTTPSRV_CGI_REQ_STRUCT* param);

/*
** CGI callback link structure
*/
typedef struct httpsrv_cgi_link_struct
{
    char*                   fn_name;            /* Function name */
    HTTPSRV_CGI_CALLBACK_FN callback;           /* Pointer to user function */
    uint32_t                stack;              /* Stack size for CGI. If set to zero, default task will be used */
} HTTPSRV_CGI_LINK_STRUCT;

/*
 * Structure defining plugin
 */
typedef struct httpsrv_plugin_struct
{
    /* Type of plugin. */
    HTTPSRV_PLUGIN_TYPE type;
    /* Pointer to plugin data. */
    void                *data;
}HTTPSRV_PLUGIN_STRUCT;

/*
 * Structure for linking resource (URI) to plugin.
 */
typedef struct httpsrv_plugin_link_struct
{
    /* Path of resource causing plugin invocation. */
    char                  *resource;
    HTTPSRV_PLUGIN_STRUCT *plugin;
}HTTPSRV_PLUGIN_LINK_STRUCT;

/*
 * Structure for SSL initialization parameters.
 */
typedef struct httpsrv_ssl_struct
{
    char*              cert_file;       /* HTTPS Server Certificate file.*/
    char*              priv_key_file;   /* HTTPS Server private key file.*/
}HTTPSRV_SSL_STRUCT;


/*
** HTTP server parameters
*/
typedef struct httpsrv_param_struct
{
    uint16_t                          af;             /* Inet protocol family */
    unsigned short                    port;           /* Listening port */
  #if RTCSCFG_ENABLE_IP4
    in_addr                           ipv4_address;   /* Listening IPv4 address */
  #endif
  #if RTCSCFG_ENABLE_IP6    
    in6_addr                          ipv6_address;   /* Listening IPv6 address */
    uint32_t                          ipv6_scope_id;  /* Scope ID for IPv6 */
  #endif
    uint32_t                          max_uri;        /* maximal URI length */
    uint32_t                          max_ses;        /* maximal sessions count */
    bool                              use_nagle;      /* enable/disable nagle algorithm for server sockets */
    const HTTPSRV_CGI_LINK_STRUCT     *cgi_lnk_tbl;   /* cgi callback table */
    const HTTPSRV_SSI_LINK_STRUCT     *ssi_lnk_tbl;   /* function callback table (dynamic web pages) */
    const HTTPSRV_PLUGIN_LINK_STRUCT  *plugins;       /* Table of plugins */
    const HTTPSRV_ALIAS               *alias_tbl;     /* table of directory aliases */
    uint32_t                          server_prio;    /* server main task priority */
    uint32_t                          script_prio;    /* script handler priority */
    uint32_t                          script_stack;   /* script handler stack */
    char*                             root_dir;       /* root directory */
    char*                             index_page;     /* index page full path and name */
    const HTTPSRV_AUTH_REALM_STRUCT   *auth_table;    /* Table of authentication realms */
    const HTTPSRV_SSL_STRUCT          *ssl_params;    /* Pointer to SSL parameters (optional).*/
} HTTPSRV_PARAM_STRUCT;


#ifdef __cplusplus
extern "C" {
#endif

/*
** Initialize and run HTTP server
** Returns server handle when successful, zero otherwise.
*/
uint32_t HTTPSRV_init(HTTPSRV_PARAM_STRUCT *params);

/*
** Stop and release HTTP server
** Returns RTCS_OK when successful, RTCS_ERR otherwise.
*/
uint32_t HTTPSRV_release(uint32_t server_h);

uint32_t HTTPSRV_cgi_write(HTTPSRV_CGI_RES_STRUCT* response);
uint32_t HTTPSRV_cgi_read(uint32_t ses_handle, char* buffer, uint32_t length);
uint32_t HTTPSRV_ssi_write(uint32_t ses_handle, char* data, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif /* HTTP_H_ */
