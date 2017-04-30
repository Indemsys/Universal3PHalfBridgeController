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
*   This file contains FTP server commands for file transfers. 
*
*
*END************************************************************************/

#include "ftpsrv_prv.h"
#include "ftpsrv_msg.h"
#include "rtcs_util.h"
#include <stdlib.h>
#include <lwmsgq.h>

#define FTPSRV_TRANSFER_TASK_NAME "FTP server transfer"

static int32_t ftpsrv_transfer(FTPSRV_SESSION_STRUCT* session, uint32_t mode);

int32_t ftpsrv_abor(FTPSRV_SESSION_STRUCT* session)
{
    FTPSRV_TRANSFER_MSG message;

    if (session->state == FTPSRV_STATE_TRANSFER)
    {
        message.command = FTPSRV_CMD_ABORT;
        _lwmsgq_send((void *)session->msg_queue, (uint32_t *)&message, LWMSGQ_SEND_BLOCK_ON_FULL);
    }
    else
    {
        session->message = (char*) ftpsrvmsg_ok;
    }

    return(FTPSRV_OK);
}

int32_t ftpsrv_appe(FTPSRV_SESSION_STRUCT* session)
{ 
    return(ftpsrv_transfer(session, FTPSRV_MODE_APPEND));
}
/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   ftpsrv_retr
* Returned Value   :  int32_t error code
* Comments  :  
*
* Usage:  
*
*END*---------------------------------------------------------------------*/

int32_t ftpsrv_retr(FTPSRV_SESSION_STRUCT* session)
{ 
    return(ftpsrv_transfer(session, FTPSRV_MODE_READ));
}

int32_t ftpsrv_stor(FTPSRV_SESSION_STRUCT* session)
{
    return(ftpsrv_transfer(session, FTPSRV_MODE_WRITE));
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   ftpsrv_retr
* Returned Value   :  int32_t error code
* Comments  :  
*
* Usage:  
*
*END*---------------------------------------------------------------------*/
int32_t ftpsrv_transfer(FTPSRV_SESSION_STRUCT* session, uint32_t mode)
{
    FTPSRV_TRANSFER_PARAM trans_param;
    #if MQX_USE_IO_OLD
    MQX_FILE_PTR          file;
    #else
    FILE                  *file;
    #endif
    uint32_t              sock;
    char*                 path;
    char*                 full_path;
    _mqx_uint             err_code;
    char                  file_param[3] = {0};
    uint32_t              wrong_path;

    if (session->cmd_arg == NULL)
    {
        session->message = (char*) ftpsrvmsg_badsyntax;
        return(FTPSRV_ERROR);
    }

    /* Get full path of file */
    rtcs_url_decode(session->cmd_arg);
    path = rtcs_path_strip_delimiters(session->cmd_arg);

    full_path = ftpsrv_get_full_path(session, path, &wrong_path);
    if (full_path == NULL)
    {
        if (wrong_path)
        {
            session->message = (char*) ftpsrvmsg_no_file;
        }
        else
        {
            session->message = (char*) ftpsrvmsg_no_memory;
        }
        return(FTPSRV_ERROR);
    }

    if (mode & FTPSRV_MODE_READ)
    {
        file_param[0] = 'r';
    }
    else if (mode & FTPSRV_MODE_WRITE)
    {
        file_param[0] = 'w';
    }
    else if (mode & FTPSRV_MODE_APPEND)
    {
        file_param[1] = 'a';
    }

    file = fopen(full_path, file_param);
    _mem_free(full_path);
    if (file == NULL)
    {
        session->message = (char*) ftpsrvmsg_no_file;
        return(FTPSRV_ERROR);
    }

    /* Send initialization message */
    ftpsrv_send_msg(session, ftpsrvmsg_opening_datacon);
    
    /* Use connection based on session type (active/passive). */
    sock = ftpsrv_open_data_connection(session);
    if (sock == RTCS_SOCKET_ERROR)
    {
        session->message = (char*) ftpsrvmsg_locerr;
        return(FTPSRV_ERROR);
    }
    trans_param.sock = sock;
    trans_param.file = file;
    trans_param.mode = mode;
    trans_param.session = session;
    err_code = RTCS_task_create(FTPSRV_TRANSFER_TASK_NAME, FTPSRV_TRANSFER_TASK_PRIO, FTPSRV_SESSION_STACK_SIZE, ftpsrv_transfer_task, &trans_param);
    if (MQX_OK != err_code)
    {
        ftpsrv_abort(sock);
        fclose(file);
        session->message = (char*) ftpsrvmsg_locerr;
        return(FTPSRV_ERROR);
    }
    session->message = "";
    return(FTPSRV_OK);
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   ftpsrv_epsv
* Returned Value   :  int32_t error code
* Comments  :  extended (IPv6) PASV command, see RFC2428.
*
* Usage:  
*
*END*---------------------------------------------------------------------*/

int32_t ftpsrv_epsv(FTPSRV_SESSION_STRUCT* session)
{
    uint16_t port;
    uint16_t family;
    char*    prots;
    uint32_t error;
    uint16_t size;
    
    size = sizeof(session->data_sockaddr);
    /* Get socket info for data socket from control socket */
    error = getsockname(session->control_sock, &session->data_sockaddr, &size);
    if (error != RTCS_OK)
    {
        session->message = (char*) ftpsrvmsg_locerr;
        return(FTPSRV_ERROR);
    }

    /* Translate socket address family to numeric value. */
    if (session->cmd_arg)
    {
        family = strtoul(session->cmd_arg, NULL, 10);
    }
    else
    {
        family = session->data_sockaddr.sa_family;
        #if RTCSCFG_ENABLE_IP4
        if (family == AF_INET)
        {
            family = FTPSRV_PROT_IPV4;
        }
        else
        #endif
        #if RTCSCFG_ENABLE_IP6
        if (family == AF_INET6)
        {
            family = FTPSRV_PROT_IPV6;
        }
        else
        #endif
        {
            family = FTPSRV_PROT_INVALID;
        }
    }

    /* Get supported protocols as string and number. */
    ftpsrv_get_prots_str(&prots, &family);

    /* Open the connection. */
    port = ftpsrv_open_passive_conn(session, family);
    if (port == 0)
    {
        if (session->state == FTPSRV_STATE_BAD_PROT)
        {
            snprintf(session->buffer, FTPSRV_BUF_SIZE, ftpsrvmsg_badprot, prots);
            session->message = session->buffer;
        }
        else
        {
            session->message = (char*) ftpsrvmsg_locerr;
        }
        return(FTPSRV_ERROR);
    }

    /* Mark this session as passive and set correct state */
    session->passive = TRUE;
    session->state = FTPSRV_STATE_LISTEN;

    snprintf(session->buffer, FTPSRV_BUF_SIZE, ftpsrvmsg_epsvok, port);
    session->message = session->buffer;
    return(FTPSRV_OK);
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   ftpsrv_eprt
* Returned Value   :  int32_t error code
* Comments  :  extended (IPv6) PORT command, see RFC2428.
*
* Usage:  
*
*END*---------------------------------------------------------------------*/
int32_t ftpsrv_eprt(FTPSRV_SESSION_STRUCT* session)
{
    char*         net_ptr;   /* family  */
    uint16_t      family;
    char*         net_addr;  /* address */
    char*         tcp_port;  /* port    */
    uint16_t      port;
    char          token[2];  /* token for parameter splitting */
    uint32_t      error;
    char*         prots;

    /* read delimiter char */
    token[0] = *(session->cmd_arg);

    /* Set default error message */
    session->message = (char*) ftpsrvmsg_badport;

    /* The delimiter character MUST be one of the ASCII characters in range 33-126 inclusive. See RFC 2428. */
    /* The character "|" (ASCII 124) is recommended */
    if ((token[0] > 126) || (token[0] < 33))
    {
        goto ERR_EXIT;
    }
    token[1] = '\0';

    /* Get components of parameter */
    net_ptr = strtok(session->cmd_arg, token);
    family = strtoul(net_ptr, NULL, 10);
    net_addr = strtok(NULL, token);
    tcp_port = strtok(NULL, token);
    port = strtoul(tcp_port, NULL, 10);

    /* Check port validity */
    if (port < 1024)
    {
        goto ERR_EXIT;
    }

    /* Get supported protocols as string and number */
    ftpsrv_get_prots_str(&prots, &family);

    /* Fill sockaddr structure depending on address family */
    switch(family)
    {
        case 1: /* IPv4 */
            session->data_sockaddr.sa_family = AF_INET;
            ((sockaddr_in*) &session->data_sockaddr)->sin_port = port;
            error = inet_pton(AF_INET,
                              net_addr,
                              &(((sockaddr_in*) &session->data_sockaddr)->sin_addr.s_addr),
                              sizeof(((sockaddr_in*) &session->data_sockaddr)->sin_addr.s_addr));
            break;
        case 2: /* IPv6 */
            session->data_sockaddr.sa_family = AF_INET6;
            ((sockaddr_in6*) &session->data_sockaddr)->sin6_port = port;
            error = inet_pton(AF_INET6,
                              net_addr,
                              &(((sockaddr_in6*) &session->data_sockaddr)->sin6_addr.s6_addr),
                              sizeof(((sockaddr_in6*) &session->data_sockaddr)->sin6_addr.s6_addr));
            break;
        default:
            snprintf(session->buffer, FTPSRV_BUF_SIZE, ftpsrvmsg_badprot, prots);
            session->message = session->buffer;
            error = (uint32_t)RTCS_ERROR;
            break;
    }

    /* Check if address was valid */
    if (error != RTCS_OK)
    {
        goto ERR_EXIT;
    }

    session->passive = FALSE;

    session->message = (char*) ftpsrvmsg_portok;
    return(FTPSRV_OK);
  ERR_EXIT:
    return(FTPSRV_ERROR);
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   ftpsrv_pasv
* Returned Value   :  int32_t error code
* Comments  :  
*
* Usage:  
*
*END*---------------------------------------------------------------------*/
   
int32_t ftpsrv_pasv(FTPSRV_SESSION_STRUCT* session)
{
    uint16_t port;
    uint32_t ip_addr;
    _mqx_int error;
    uint16_t size;
    
    size = sizeof(session->data_sockaddr);
    /* Get socket info for data socket from control socket */
    error = getsockname(session->control_sock, &session->data_sockaddr, &size);
    if (error != RTCS_OK)
    {
        session->message = (char*) ftpsrvmsg_locerr;
        return(FTPSRV_ERROR);
    }

    if (session->data_sockaddr.sa_family != AF_INET)
    {
        session->message = (char*) ftpsrvmsg_unimp;
        return(FTPSRV_ERROR);
    }

    /* Try to open passive connection */
    port = ftpsrv_open_passive_conn(session, FTPSRV_PROT_IPV4);
    if (port == 0)
    {
        session->message = (char*) ftpsrvmsg_locerr;
        return(FTPSRV_ERROR);
    }

    /* Mark this session as passive and set correct state */
    session->passive = TRUE;
    session->state = FTPSRV_STATE_LISTEN;

    ip_addr = ((sockaddr_in*) &session->data_sockaddr)->sin_addr.s_addr;
    snprintf(session->buffer, FTPSRV_BUF_SIZE, ftpsrvmsg_pasv_mode, IPBYTES(ip_addr), ((port >> 8) & 0xff), (port & 0xff));
    session->message = session->buffer;
    return(FTPSRV_OK);
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   ftpsrv_port
* Returned Value   :  int32_t error code
* Comments  :  
*
* Usage:  
*
*END*---------------------------------------------------------------------*/
   
int32_t ftpsrv_port(FTPSRV_SESSION_STRUCT* session)
{
    uint32_t     i;
    uint16_t     port;
    char*        arg = session->cmd_arg;
    char         addr[6];

    if (session->cmd_arg == NULL)
    {
        session->message = (char*) ftpsrvmsg_badsyntax;
        return(FTPSRV_ERROR);
    }

    for (i = 0; i < 6; i++)
    {
        char* tmp = strtok(arg, ",");
        addr[i] = strtoul(tmp, NULL, 10);
        arg = NULL;
    }

    port = ((uint16_t) addr[4] << 8) + addr[5];

    if (port < 1024)
    {
        session->message = (char*) ftpsrvmsg_badport;
        return(FTPSRV_ERROR);
    }

   ((sockaddr_in*) &session->data_sockaddr)->sin_family = AF_INET;
   ((sockaddr_in*) &session->data_sockaddr)->sin_addr.s_addr = IPADDR(addr[0],addr[1],addr[2],addr[3]);
   ((sockaddr_in*) &session->data_sockaddr)->sin_port = port;

   session->passive = FALSE;
   session->state = FTPSRV_STATE_CONNECT;

   session->message = (char*) ftpsrvmsg_portok;
   return FTPSRV_OK;
}
