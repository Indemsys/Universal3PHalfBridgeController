/**HEADER********************************************************************
* 
* Copyright (c) 2013 Freescale Semiconductor;
* All Rights Reserved                       
*
*************************************************************************** 
*
* THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR 
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  
* IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
* THE POSSIBILITY OF SUCH DAMAGE.
*
**************************************************************************
*
* Comments:
*
*   HTTPSRV script functions.
*
*END************************************************************************/

#include "httpsrv.h"
#include "httpsrv_prv.h"
#include "httpsrv_supp.h"
#include "httpsrv_script.h"
#include <lwmsgq.h>

#define HTTPSRV_DETACHED_SCRIPT_TASK_NAME "HTTP server detached callback task"
/*
** Wait for session CGI unlock. Wait until session timeout.
**
** IN:
**      HTTPSRV_SESSION_STRUCT* session - session structure pointer.
**
** OUT:
**      none
**
** Return Value: 
**      MQX_OK if CGI processign finished during wait, error code otherwise.
*/
uint32_t httpsrv_wait_for_cgi(HTTPSRV_SESSION_STRUCT *session)
{
    MQX_TICK_STRUCT time_ticks;
    uint32_t        time_msec;
    TIME_STRUCT     time;

    RTCS_ASSERT(session != NULL);
    if (session->script_tid == 0)
    {
        return(HTTPSRV_OK);
    }

    time_msec = session->time + session->timeout;
    /* Prepare information about time in appropriate format */
    time.SECONDS = time_msec/1000;
    time.MILLISECONDS = time_msec%1000;
    _time_to_ticks(&time, &time_ticks);

    return(_lwsem_wait_until(&session->lock, &time_ticks));
}

/*
** Detach script processing to separate task
**
** IN:
**      HTTPSRV_SESSION_STRUCT* session - session structure pointer.
**      HTTPSRV_STRUCT *server - pointer to server structure (needed for session parameters).
**
** OUT:
**      none
**
** Return Value: 
**      none
*/
void httpsrv_detach_script(HTTPSRV_DET_TASK_PARAM* task_params)
{
    _mqx_uint  error;
    _mqx_uint  priority;

    RTCS_ASSERT(task_params != NULL);
    error = _task_get_priority(MQX_NULL_TASK_ID, &priority);
    if (error != MQX_OK)
    {
        return;
    }

    error = RTCS_task_create(HTTPSRV_DETACHED_SCRIPT_TASK_NAME, priority, task_params->stack_size, httpsrv_detached_task, (void *)task_params);
    if (error != MQX_OK)
    {
        return;
    }
}

/*
** Function for searching callback for name in function table (SSI/CGI)
**
** IN:
**      HTTPSRV_FN_LINK_STRUCT* table - table to search in.
**
**       char* name - name to search.
** OUT:
**      stack_size - pointer to variable to store callback stack size to.
**
** Return Value: 
**      HTTPSRV_FN_CALLBACK - function callback if successfull, NULL if not found
*/
HTTPSRV_FN_CALLBACK httpsrv_find_callback(HTTPSRV_FN_LINK_STRUCT* table, char* name, uint32_t* stack_size)
{
    HTTPSRV_FN_CALLBACK retval = NULL;

    RTCS_ASSERT(table != NULL);
    RTCS_ASSERT(name != NULL);
    if ((table == NULL) || (name == NULL))
    {
        goto EXIT;
    }

    while ((table->callback != NULL) && (*(table->callback) != NULL))
    {
        if (0 == strcmp(name, table->fn_name))
        {
            retval = (HTTPSRV_FN_CALLBACK) table->callback;
            if (stack_size != NULL)
            {
                *stack_size = table->stack_size;
            }
            break;
        }
        table++;
    }
    EXIT:
    return(retval);
}

/*
** Function for CGI calling
**
** IN:
**      HTTPSRV_CGI_CALLBACK_FN function - pointer to user function to be called as CGI
**
**      HTTPSRV_SCRIPT_MSG* msg_ptr - pointer to message containing data required for CGI parameter
** OUT:
**      none
**
** Return Value: 
**      none
*/
void httpsrv_call_cgi(HTTPSRV_CGI_CALLBACK_FN function, HTTPSRV_SCRIPT_MSG* msg_ptr)
{
    HTTPSRV_SESSION_STRUCT *session;
    HTTPSRV_CGI_REQ_STRUCT cgi_param;
    char server_ip[RTCS_IP_ADDR_STR_SIZE];
    char remote_ip[RTCS_IP_ADDR_STR_SIZE];
    struct sockaddr l_address;
    struct sockaddr r_address;
    uint16_t length = sizeof(sockaddr);

    RTCS_ASSERT(function != NULL);
    RTCS_ASSERT(msg_ptr != NULL);
    session = msg_ptr->session;
    RTCS_ASSERT(session != NULL);
    
    /* Fill callback parameter */
    cgi_param.ses_handle = (uint32_t) session;
    cgi_param.request_method = session->request.method;
    cgi_param.content_type = (HTTPSRV_CONTENT_TYPE) session->request.content_type;
    cgi_param.content_length = session->request.content_length;
    
    getsockname(session->sock, &l_address, &length);
    getpeername(session->sock, &r_address, &length);

    if (l_address.sa_family == AF_INET)
    {
        inet_ntop(l_address.sa_family, &((struct sockaddr_in*) &l_address)->sin_addr.s_addr, server_ip, sizeof(server_ip));
        inet_ntop(r_address.sa_family, &((struct sockaddr_in*) &r_address)->sin_addr.s_addr, remote_ip, sizeof(remote_ip));
        cgi_param.server_port = ((struct sockaddr_in*) &l_address)->sin_port;
    }
    else if (l_address.sa_family == AF_INET6)
    {
        inet_ntop(l_address.sa_family, ((struct sockaddr_in6*) &l_address)->sin6_addr.s6_addr, server_ip, sizeof(server_ip));
        inet_ntop(r_address.sa_family, ((struct sockaddr_in6*) &r_address)->sin6_addr.s6_addr, remote_ip, sizeof(remote_ip));
        cgi_param.server_port = ((struct sockaddr_in6*) &l_address)->sin6_port;
    }
    
    cgi_param.auth_type = HTTPSRV_AUTH_BASIC;
    cgi_param.remote_user = session->request.auth.user_id;
    cgi_param.remote_addr = remote_ip;
    cgi_param.server_name = server_ip;
    cgi_param.script_name = msg_ptr->name;
    cgi_param.server_protocol = HTTPSRV_PROTOCOL_STRING;
    cgi_param.server_software = HTTPSRV_PRODUCT_STRING;
    cgi_param.query_string = session->request.query;
    cgi_param.gateway_interface = HTTPSRV_CGI_VERSION_STRING;

    /* Call the function */
    function(&cgi_param);
}

/*
** Function for SSI calling
**
** IN:
**      HTTPSRV_SSI_CALLBACK_FN function - pointer to user function to be called as SSI
**
**      HTTPSRV_SCRIPT_MSG* msg_ptr - pointer to message containing data required for SSI parameter
** OUT:
**      none
**
** Return Value: 
**      none
*/
void httpsrv_call_ssi(HTTPSRV_SSI_CALLBACK_FN function, HTTPSRV_SCRIPT_MSG* msg_ptr)
{
    char* tmp;
    HTTPSRV_SSI_PARAM_STRUCT ssi_param;

    RTCS_ASSERT(function != NULL);
    RTCS_ASSERT(msg_ptr != NULL);
    ssi_param.ses_handle = (uint32_t) msg_ptr->session;
    tmp = strchr(msg_ptr->name, ':');
    if (tmp != NULL)
    {
        *tmp++ = '\0';
    }
    else
    {
        tmp = "";
    }
    ssi_param.com_param = tmp;

    function(&ssi_param);
}

/*
** Function for CGI request processing
**
** IN:
**      HTTPSRV_SESSION_STRUCT* session - session structure pointer.
**      HTTPSRV_STRUCT*         server - pointer to server structure (needed for session parameters).
**      char*                   cgi_name - name of cgi function.
**
** OUT:
**      none
**
** Return Value: 
**      none
*/
void httpsrv_process_cgi(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session, char* cgi_name)
{
    HTTPSRV_SCRIPT_MSG message;

    RTCS_ASSERT(session != NULL);
    RTCS_ASSERT(server != NULL);
    RTCS_ASSERT(cgi_name != NULL);

    message.session = session;
    message.type = HTTPSRV_CGI_CALLBACK;
    message.name = cgi_name;
    message.ses_tid = _task_get_id();
    _lwmsgq_send(server->script_msgq, (_mqx_max_type*) &message, LWMSGQ_SEND_BLOCK_ON_FULL);
    /* wait until CGI is processed */
    _task_block();

    /*
    ** There is some unread content from client after CGI finished. 
    ** It must be read and discarded if we have keep-alive enabled
    ** so it does not affect next request.
    */
    if (session->request.content_length)
    {
        char *tmp = NULL;

        tmp = _mem_alloc(HTTPSRV_TMP_BUFFER_SIZE);
        if (tmp != NULL)
        {
            uint32_t length = session->request.content_length;

            while(length)
            {
                int32_t retval;

                retval = httpsrv_read(session, tmp, HTTPSRV_TMP_BUFFER_SIZE);
                if ((retval == 0) || (retval == RTCS_ERROR))
                {
                    break;
                }
                length -= retval;
            }
            _mem_free(tmp);
            session->request.content_length = 0;
        } 
    }
    return;
}
