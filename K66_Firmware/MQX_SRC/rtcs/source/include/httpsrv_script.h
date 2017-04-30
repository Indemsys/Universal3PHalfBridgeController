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
* $FileName: httpsrv_prv.h$
*
* Comments:
*
*   Header for HTTPSRV.
*
*END************************************************************************/

#ifndef HTTPSRV_SCRIPT_H_
#define HTTPSRV_SCRIPT_H_


#include "httpsrv_prv.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t httpsrv_wait_for_cgi(HTTPSRV_SESSION_STRUCT *session);
void httpsrv_detach_script(HTTPSRV_DET_TASK_PARAM* task_params);
HTTPSRV_FN_CALLBACK httpsrv_find_callback(HTTPSRV_FN_LINK_STRUCT* table, char* name, uint32_t* stack_size);
void httpsrv_call_cgi(HTTPSRV_CGI_CALLBACK_FN function, HTTPSRV_SCRIPT_MSG* msg_ptr);
void httpsrv_call_ssi(HTTPSRV_SSI_CALLBACK_FN function, HTTPSRV_SCRIPT_MSG* msg_ptr);
void httpsrv_process_cgi(HTTPSRV_STRUCT *server, HTTPSRV_SESSION_STRUCT *session, char* cgi_name);

#ifdef __cplusplus
}
#endif

#endif
