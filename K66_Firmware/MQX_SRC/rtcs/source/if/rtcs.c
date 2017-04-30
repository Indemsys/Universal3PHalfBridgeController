/*HEADER**********************************************************************
*
* Copyright 2008, 2014 Freescale Semiconductor, Inc.
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
#include "rtcs_prv.h"
#include "tcpip.h"
#include "socket.h"

#define         RTCS_DNS_SUFFIX_MAX_SIZE   (255)   /* Maximum length of DNS suffix, including terminating null.*/
static char    *rtcs_dns_suffix = NULL;            /* Primary DNS suffix.*/

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_socket
* Returned Value  : socket handle
* Comments  :  Routine to open a socket.
*
*END*-----------------------------------------------------------------*/

uint32_t  RTCS_socket
   (
      uint32_t     pf,
         /*[IN] specifies the protocol family */
      uint32_t     type,
         /*[IN] specifies the type of communication */
      uint32_t     protocol
         /*[IN] select a specific protocol */
   )
{ /* Body */
  SOCKET_STRUCT_PTR socket_ptr;
  uint32_t           error;

  RTCSLOG_API(RTCSLOG_TYPE_FNENTRY, RTCSLOG_FN_SOCKET, type);

  socket_ptr = SOCK_Get_sock_struct((RTCS_SOCKET_CALL_STRUCT_PTR)type,
                                     RTCS_task_getid());
  if (socket_ptr == NULL) 
  {
    RTCSLOG_API(RTCSLOG_TYPE_FNEXIT, RTCSLOG_FN_SOCKET, RTCS_SOCKET_ERROR);
    return RTCS_SOCKET_ERROR;
  }
   
  /* Setup internet family */
  if(
    #if RTCSCFG_ENABLE_IP4
      (AF_INET == pf)
      #if RTCSCFG_ENABLE_IP6
      ||
      #endif
    #endif
    #if RTCSCFG_ENABLE_IP6
      (AF_INET6 == pf)
    #endif
    )
  {
    socket_ptr->AF = pf;
  }
  else
  {
    RTCSLOG_API(RTCSLOG_TYPE_FNEXIT, RTCSLOG_FN_SOCKET, RTCS_SOCKET_ERROR);
    return RTCS_SOCKET_ERROR;
  }

  /* call protocol specific socket() function */
  if (socket_ptr->PROTOCOL->SOCK_SOCKET) 
  {
    error = socket_ptr->PROTOCOL->SOCK_SOCKET((uint32_t)socket_ptr);
    if (error) 
    {
      SOCK_Free_sock_struct(socket_ptr);
      RTCSLOG_API(RTCSLOG_TYPE_FNEXIT, RTCSLOG_FN_SOCKET, RTCS_SOCKET_ERROR);
      return RTCS_SOCKET_ERROR;
    }
  }

  RTCSLOG_API(RTCSLOG_TYPE_FNEXIT, RTCSLOG_FN_SOCKET, (uint32_t)socket_ptr);
  return (uint32_t)socket_ptr;
}


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_shutdown
* Returned Value  : OK or error code
* Comments  :  Routine to close a socket.
*
*END*-----------------------------------------------------------------*/

uint32_t  RTCS_shutdown
   (
      uint32_t     sock,
         /*[IN] socket handle for this socket */
      uint32_t     how
         /*[IN] specifies whether or not connection should be immediate */
   )
{
  struct linger so_linger = {0}; /* l_onoff = 0; l_linger = 0; */
  int32_t status;
  
  if(how == FLAG_ABORT_CONNECTION)
  {
    so_linger.l_onoff = 1; /* l_onoff = 1; l_linger = 0; */
  }
  
  status = setsockopt(sock, SOL_SOCKET, SO_LINGER, (const void*)&so_linger, sizeof(so_linger));
  
  if(status == RTCS_OK)
  {
    status = closesocket(sock);
  }
  
  return (uint32_t)status;
}

#if RTCSCFG_BACKWARD_COMPATIBILITY_RTCSSELECT==1
/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_attachsock
* Returned Value  : socket handle or RTCS_ERROR
* Comments  :  Routine to take ownership of a socket.
*
*END*-----------------------------------------------------------------*/

uint32_t  RTCS_attachsock
   (
      uint32_t     sock
         /*[IN] specifies the handle of the existing socket */
   )
{ /* Body */
   register SOCKET_STRUCT_PTR socket_ptr = (SOCKET_STRUCT_PTR)sock;

   if (!SOCK_Add_owner(socket_ptr, RTCS_task_getid())) {
      RTCS_setsockerror(sock, RTCSERR_OUT_OF_MEMORY);
      return RTCS_SOCKET_ERROR;
   } /* Endif */

   return sock;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_detachsock
* Returned Value  : error code
* Comments  :  Routine to relinquish ownership of a socket.
*
*END*-----------------------------------------------------------------*/

uint32_t  RTCS_detachsock
   (
      uint32_t     sock
         /*[IN] specifies the handle of the existing socket */
   )
{ /* Body */
   register SOCKET_STRUCT_PTR socket_ptr = (SOCKET_STRUCT_PTR)sock;

   if (!SOCK_Remove_owner(socket_ptr, RTCS_task_getid())) {
      return RTCSERR_SOCK_NOT_OWNER;
   } /* Endif */

   return RTCS_OK;

} /*EndBody*/

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_transfersock
* Returned Value  : socket handle or RTCS_ERROR
* Comments  :  Routine to take ownership of a socket.
*
*END*-----------------------------------------------------------------*/
uint32_t  RTCS_transfersock
   (
      uint32_t     in_sock,
         /*[IN] specifies the handle of the existing socket */
      _task_id    new_owner
   )
{
   SOCKET_STRUCT_PTR socket_ptr = (SOCKET_STRUCT_PTR)in_sock;
   _rtcs_taskid      old_owner_id;
   _rtcs_taskid      new_owner_id;
   uint32_t           out_sock = RTCS_SOCKET_ERROR;

   old_owner_id = RTCS_task_getid();

   if (SOCK_Is_owner(socket_ptr, old_owner_id)) {

      new_owner_id = _task_get_td(new_owner);
      if (new_owner_id) {
         if (old_owner_id==new_owner_id) {
            out_sock=in_sock;
         } else if (SOCK_Add_owner(socket_ptr, new_owner_id)) {
            out_sock=in_sock;
            SOCK_Remove_owner(socket_ptr, old_owner_id);
         } else {
            RTCS_setsockerror(in_sock, RTCSERR_OUT_OF_MEMORY);
         } /* Endif */
      } else {
         RTCS_setsockerror(in_sock, RTCSERR_INVALID_PARAMETER);
      }
   } else {
      RTCS_setsockerror(in_sock, RTCSERR_SOCK_NOT_OWNER);
   } /* Endif */

   return out_sock;
}
#endif

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_geterror
* Returned Value  : error code
* Comments  :  Routine is used to get the last error code associated
*              with a socket.
*
*END*-----------------------------------------------------------------*/

uint32_t  RTCS_geterror
   (
      uint32_t     sock
         /*[IN] socket handle for this socket */
   )
{ /* Body */
   register SOCKET_STRUCT_PTR socket_ptr = (SOCKET_STRUCT_PTR)sock;

   /*
   ** Verify the socket
   */
   if(!SOCK_check_valid(sock))
   {
      return RTCSERR_SOCK_INVALID;
   }

   return socket_ptr->ERROR_CODE;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_setsockerror
* Returned Value  : error code
* Comments  :  Routine is used to set the error code on a socket.
*              Used internally by RTCS.
*
*END*-----------------------------------------------------------------*/

void RTCS_setsockerror
   (
      uint32_t     sock,
         /* [IN] socket handle for this socket */
      uint32_t     error
   )
{ /* Body */
   register SOCKET_STRUCT_PTR socket_ptr = (SOCKET_STRUCT_PTR)sock;

   socket_ptr->ERROR_CODE = error;

} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_set_primary_dns_suffix
* Returned Value  : RTCS_OK (success), Error code (failure)
* Comments  :  Sets Primary DNS Suffix.
*              To clear DNS suffix, call RTCS_set_primary_dns_suffix(NULL);
*END*-----------------------------------------------------------------*/
uint32_t RTCS_set_primary_dns_suffix(char *dns_suffix)
{
    uint32_t    result = RTCS_OK;
    size_t      dns_suffix_length = (dns_suffix!=NULL) ? strlen(dns_suffix) : 0; 

    if(dns_suffix_length <= RTCS_DNS_SUFFIX_MAX_SIZE) /* Control maximum length */
    {
        if(rtcs_dns_suffix != NULL) /* Clear old DNS suffix.*/
        {
            _mem_free(rtcs_dns_suffix);
            rtcs_dns_suffix = NULL;
        }

        /* Allocate and copy new DNS Suffix string.*/
        if(dns_suffix_length != 0)
        {
            dns_suffix_length++; /*Count terminating null.*/

            rtcs_dns_suffix = (char*)_mem_alloc_system(dns_suffix_length);

            if (rtcs_dns_suffix != NULL)
            {
                _mem_copy((void*)dns_suffix, (void*)rtcs_dns_suffix, dns_suffix_length);
            }
            else
            {
                result = RTCSERR_OUT_OF_MEMORY;
            }
        }
    }
    else
    {
        result = RTCSERR_INVALID_PARAMETER;
    }
    
    return result;
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_get_primary_dns_suffix
* Returned Value  : Pointer Primary DNS Suffix, or NULL if not set.
* Comments  :  Gets Primary DNS Suffix.
*
*END*-----------------------------------------------------------------*/
const char * RTCS_get_primary_dns_suffix(void)
{
    return rtcs_dns_suffix;
}


/* EOF */
