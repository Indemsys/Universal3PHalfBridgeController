/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
* Copyright 2004-2008 Embedded Access Inc.
* Copyright 1989-2008 ARC International
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
*   This file contains the interface functions to the
*   RTCS Communication Library.
*
*
*END************************************************************************/

#include <rtcs.h>
#if MQX_USE_IO_OLD
#include <fio.h>
#else
#include <stdio.h>
#endif
#include "rtcs_prv.h"
#include "tcpip.h"
#include "socket.h"
#include "ip_prv.h"


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_create
* Returned Value  : RTCS_OK or error code
* Comments        :
*        Initialize RTCS.
*
*END*-----------------------------------------------------------------*/

uint32_t RTCS_create
   (
      void
   )
{ /* Body */
   RTCS_DATA_PTR              RTCS_data_ptr;
   SOCKET_CONFIG_STRUCT_PTR   socket_cfg_ptr;
   uint32_t                    error;

   /*
   ** Check and see if this is the first time we have initialized,
   */
   if (RTCS_get_data() != NULL) {
      return RTCSERR_INITIALIZED;
   } 
//
   RTCS_data_ptr = RTCS_mem_alloc_system_zero(sizeof(RTCS_DATA));

   if (RTCS_data_ptr == NULL)  {
      error = RTCSERR_OUT_OF_MEMORY;
   } else {

   _mem_set_type(RTCS_data_ptr, MEM_TYPE_RTCS_DATA);

   RTCS_set_data(RTCS_data_ptr);

   /*
   ** Initialize socket state
   */
   socket_cfg_ptr = RTCS_mem_alloc_system_zero(sizeof(SOCKET_CONFIG_STRUCT));

      if (socket_cfg_ptr == NULL)  {
         error =  RTCSERR_OUT_OF_MEMORY;
      } else {
         _mem_set_type(socket_cfg_ptr, MEM_TYPE_SOCKET_CONFIG_STRUCT);
      
         socket_cfg_ptr->INITIALIZED = TRUE;
         socket_cfg_ptr->SOCKET_HEAD = NULL;
         socket_cfg_ptr->SOCKET_TAIL = NULL;
         RTCS_mutex_init(&socket_cfg_ptr->SOCK_MUTEX);
         RTCS_setcfg(SOCKET, socket_cfg_ptr);

   /*
   ** Initialize global data
   */
   _IP_forward    = FALSE;
   _TCP_bypass_rx = FALSE;
   _TCP_bypass_tx = FALSE;

   RTCS_data_ptr->VERSION            = RTCS_VERSION;
#if RTCSCFG_LOG_SOCKET_API||RTCSCFG_LOG_PCB
   RTCS_data_ptr->RTCS_LOG_PROTCOUNT = RTCSLOG_PROT_MAX;
   RTCSLOG_disable(RTCS_LOGCTRL_ALL);
#endif
   RTCS_data_ptr->TCPIP_qid = RTCS_msgq_get_id(0,TCPIP_QUEUE);

         /*
         ** Create a pool of buffers for use in communicating to the TCP/IP task.
         */
         RTCS_data_ptr->TCPIP_msg_pool = RTCS_msgpool_create(sizeof(TCPIP_MESSAGE),
            _RTCS_msgpool_init, _RTCS_msgpool_grow, _RTCS_msgpool_max);
         if (RTCS_data_ptr->TCPIP_msg_pool == MSGPOOL_NULL_POOL_ID) {
            RTCS_log_error( ERROR_TCPIP, RTCSERR_CREATE_POOL_FAILED, 0, 0, 0);
            error =  RTCSERR_CREATE_POOL_FAILED;
         } else {
            /*
            ** Create the socket partition
            */
            RTCS_data_ptr->RTCS_socket_partition = RTCS_part_create(sizeof(SOCKET_STRUCT),
              _RTCS_socket_part_init, _RTCS_socket_part_grow, _RTCS_socket_part_max, NULL, NULL);
            if (RTCS_data_ptr->RTCS_socket_partition == 0) {
               RTCS_log_error(ERROR_RTCS, RTCSERR_CREATE_PARTITION_FAILED, 0, 0, 0);
               error = RTCSERR_CREATE_PARTITION_FAILED;
            } else {
               /*
               ** Create TCPIP task
               */
               error = RTCS_task_create("TCP/IP", _RTCSTASK_priority, _RTCSTASK_stacksize, TCPIP_task, NULL);
               if (error) {
                  RTCS_part_destroy(RTCS_data_ptr->RTCS_socket_partition);
               }
            }
            if (error) {
               RTCS_msgpool_destroy( RTCS_data_ptr->TCPIP_msg_pool );
            }     
         }
         if (error) {
            RTCS_setcfg(SOCKET, NULL);
            _mem_free(socket_cfg_ptr);
         }
      }
      if (error) {
         RTCS_set_data(NULL);
         _mem_free(RTCS_data_ptr);
      }
   }
   
   return error;
            
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_cmd_issue
* Returned Values : uint32_t (error code)
* Comments        :
*     Issue a command to the TCP/IP task
*
*END*-----------------------------------------------------------------*/

uint32_t RTCS_cmd_issue
   (
      TCPIP_PARM_PTR    parm_ptr,
      void (_CODE_PTR_  command)(void *)
   )
{ /* Body */
   RTCS_DATA_PTR        RTCS_data_ptr;
   TCPIP_MESSAGE_PTR    message;

   RTCS_data_ptr = RTCS_get_data();

   message = RTCS_msg_alloc(RTCS_data_ptr->TCPIP_msg_pool);
   if (message == NULL) {
      parm_ptr->ERROR = RTCSERR_OUT_OF_BUFFERS;
   } else {
      parm_ptr->ERROR = RTCS_OK;
      parm_ptr->SYNC  = _task_get_id_from_td(_task_get_td(0));

      message->HEAD.TARGET_QID = RTCS_data_ptr->TCPIP_qid;
      message->HEAD.SOURCE_QID = 0;
      message->HEAD.SIZE = sizeof(*message);
      message->COMMAND = command;
      message->DATA = parm_ptr;

      if (!RTCS_msgq_send_blocked(message, RTCS_data_ptr->TCPIP_msg_pool)) {
         parm_ptr->ERROR = RTCSERR_SEND_FAILED;
      } /* Endif */

   } /* Endif */

   return parm_ptr->ERROR;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_cmd_internal
* Returned Values : uint32_t (error code)
* Comments        :
*     Issue a command from within the TCP/IP task
*
*END*-----------------------------------------------------------------*/

uint32_t RTCS_cmd_internal
   (
      TCPIP_PARM_PTR    parm_ptr,
      void (_CODE_PTR_  command)(void *)
   )
{ /* Body */

   parm_ptr->ERROR = RTCS_OK;
   parm_ptr->SYNC  = MQX_NULL_TASK_ID;

   command(parm_ptr);

   return parm_ptr->ERROR;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_cmd_smartissue
* Returned Values : uint32_t (error code)
* Comments        :
*     Issue a command from any context
*
*END*-----------------------------------------------------------------*/

uint32_t RTCS_cmd_smartissue
   (
      TCPIP_PARM_PTR    parm_ptr,
      void (_CODE_PTR_  command)(void *)
   )
{ /* Body */
   RTCS_DATA_PTR        RTCS_data_ptr;

   RTCS_data_ptr = RTCS_get_data();

   if (RTCS_task_getid() == RTCS_data_ptr->TCPIP_TASKID) {
      return RTCS_cmd_internal(parm_ptr, command);
   } else {
      return RTCS_cmd_issue(parm_ptr, command);
   } /* Endif */

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_cmd_complete
* Returned Values : void
* Comments        :
*     Unblocks an application following successful completion of a command.
*
*END*-----------------------------------------------------------------*/

void RTCS_cmd_complete
   (
      TCPIP_PARM_PTR    parm_ptr,
      uint32_t           error
   )
{ /* Body */
   void * td = NULL;
   if (error) 
   {
      parm_ptr->ERROR = error;
   } /* Endif */
   /*
    * SYNC can be NULL by RTCS_cmd_internal. In such a case we don't call
    * _task_ready().
    * Next, we try _test_get_td(), this would return NULL if the SYNC task 
    * does not exist. 
    * This may happen when the SYNC task was aborted/destroyed.
    * It makes no sense to _task_ready() a task that does not exist.
    */
   if((MQX_NULL_TASK_ID != parm_ptr->SYNC) && (NULL != (td=_task_get_td(parm_ptr->SYNC)))) 
   {
      _task_ready(td);
      if(MQX_INVALID_TASK_STATE == _task_get_error())
      {
        /* task is already on Ready queue. 
         * it is possible that RTCS task calls RTCS_cmd_complete
         * multiple times on the same task. just clear the error code so that
         * it does not confuse users.
         */
         _task_set_error(MQX_OK);
      }
   } /* Endif */

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_cmd_service
* Returned Values : TRUE on success
* Comments        :
*     Issue a nonblocking command to the TCP/IP task
*
*END*-----------------------------------------------------------------*/

bool RTCS_cmd_service
   (
      RTCSPCB_PTR       pcb,
      void (_CODE_PTR_  command)(void *)
   )
{ /* Body */
   RTCS_DATA_PTR        RTCS_data_ptr;
   TCPIP_MESSAGE_PTR    message;
   bool b_retval;

   RTCS_data_ptr = RTCS_get_data();

   message = RTCS_msg_alloc(RTCS_data_ptr->TCPIP_msg_pool);
   if (message == NULL) {
      return FALSE;
   } /* Endif */

   message->HEAD.TARGET_QID = RTCS_data_ptr->TCPIP_qid;
   message->HEAD.SOURCE_QID = 0;
   message->HEAD.SIZE = sizeof(*message);
   message->COMMAND = command;
   message->DATA = pcb;
#if 1
#include <message.h>
  b_retval = _msgq_send_priority(message, MSG_MAX_PRIORITY);
  return b_retval;
#else
   return RTCS_msgq_send(message, RTCS_data_ptr->TCPIP_msg_pool);
#endif

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  RTCS_seterror
*  Returned Value:  void
*  Comments      :  Called to record an error condition in a
*                   provided error structure.
*
*END*-----------------------------------------------------------------*/

void RTCS_seterror
   (
      RTCS_ERROR_STRUCT_PTR   errptr,
         /* [IN/OUT] where to record the error */
      uint32_t                 errcode,
         /* [IN] the error that occurred */
      uint32_t                 errparm
         /* [IN] additional information */
   )
{ /* Body */

   /* Only record the first error that occurs */
   if (errptr->ERROR == RTCS_OK) {
      errptr->ERROR    = errcode;
      errptr->PARM     = errparm;
#ifdef __MQX__
      errptr->TASK_ID   = _task_get_id();
      errptr->TASKCODE = _task_get_error();
      errptr->MEMPTR   = _mem_get_highwater();
      errptr->STACK    = _task_check_stack();
#endif
   } /* Endif */

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name    : RTCS_log_error
* Returned Value   : none
* Comments  :  Routine to handle non-fatal errors.
*
*END*-----------------------------------------------------------------*/

void  RTCS_log_error
   (
      uint32_t  module,
      uint32_t  error,
      uint32_t  p1,
      uint32_t  p2,
      uint32_t  p3
   )
{ /* Body */
#if RTCSCFG_LOG_SOCKET_API || RTCSCFG_LOG_PCB
   RTCS_log(RTCSLOG_TYPE_ERROR,module,error,p1,p2,p3);
#endif
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_io_open
* Returned Value  : open file descriptor
* Comments        : RTOS-specific wrapper for fopen().
*
*END*-----------------------------------------------------------------*/

void *RTCS_io_open
   (
      char       *filename,
      char       *filemode,
      uint32_t   *error_ptr
   )
{ /* Body */
#if MQX_USE_IO_OLD
   MQX_FILE_PTR file;
#else
   FILE * file;
#endif
   uint32_t  i;

   /* Scan for device name delimiter */
   for (i = 0; filename[i] && filename[i] != ':'; i++) {};

   /* 
   ** If the next char is not nul then a file
   ** on the resident file system is probably
   ** being opened.
   */
   if (filename[i] == ':' && filename[i+1] != '\0') {
      /* Assume we are opening a file */
      file = fopen(filename, filemode);
   } else {
      file = fopen(filename, 0);
   } /* Endif */

   if (error_ptr) {
      if (file) {
         *error_ptr = RTCS_OK;
      } else {
         *error_ptr = _task_get_error();
         if (!*error_ptr) {
            *error_ptr = RTCS_ENOENT;
         } /* Endif */
      } /* Endif */
   } /* Endif */

   return file;

} /* Endbody */

