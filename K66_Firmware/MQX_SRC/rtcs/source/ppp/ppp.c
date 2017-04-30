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
*   This file contains the implementation of the PPP
*   initialization routines.
*
*
*END************************************************************************/

#include <rtcs.h>

#if RTCSCFG_ENABLE_PPP && PLATFORM_SDK_ENABLED
  #error "PPP is not supported. RTCSCFG_ENABLE_PPP shall be set to FALSE."
#endif

#if RTCSCFG_ENABLE_PPP && !PLATFORM_SDK_ENABLED

#include <ppp.h>
#include "ppp_prv.h"


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPP_init
* Returned Value  :  error code
* Comments        :
*     Initializes PPP.
*
*END*-----------------------------------------------------------------*/
_ppp_handle PPP_init
   (
        PPP_PARAM_STRUCT*     params
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4 
    PPP_CFG_PTR          ppp_ptr;
    uint32_t             error;
    IPCP_DATA_STRUCT     ipcp_data;
    int stage = 0;

    /* Allocate the state structure */
    ppp_ptr = _mem_alloc_zero(sizeof(PPP_CFG));
    
    if (!ppp_ptr)
    {
        return(NULL);
    }
    ppp_ptr->DEVICE_NAME = _mem_alloc_zero(strlen(params->device)+1);
    if (ppp_ptr->DEVICE_NAME == NULL)
    {
        _mem_free(ppp_ptr);
        return(NULL);
    }
    strcpy(ppp_ptr->DEVICE_NAME, params->device);
    /* Stage 0 - Open low level device */
    ppp_ptr->IOPCB_DEVICE = fopen(params->device, NULL);
    if (ppp_ptr->IOPCB_DEVICE == NULL)
    {
        PPP_init_fail(ppp_ptr, stage);
        return(NULL);
    }
    stage++;

    /* Stage 1 - Initialize HDLC and lwsem */
    ppp_ptr->DEVICE = _iopcb_ppphdlc_init(ppp_ptr->IOPCB_DEVICE);
    if (ppp_ptr->DEVICE == NULL)
    {
        PPP_init_fail(ppp_ptr, stage);
        return(NULL);
    }

    ppp_ptr->RECV_OPTIONS = &PPP_DEFAULT_OPTIONS;
    ppp_ptr->SEND_OPTIONS = &PPP_DEFAULT_OPTIONS;

    if (_lwsem_create(&ppp_ptr->MUTEX, 1))
    {
        PPP_init_fail(ppp_ptr, stage);
        return(NULL);
    }
    stage++;

    /* Stage 2 - Initialize LCP */
    error = LCP_init(ppp_ptr);
    if (error)
    {
        PPP_init_fail(ppp_ptr, stage);
        return(NULL);
    }
    stage++;

    /* Stage 3 - Initialize and open CCP */
    /*
    error = CCP_init(ppp_ptr);
    if (error)
    {
        PPP_init_fail(ppp_ptr, stage);
        return error;
    }
    CCP_open(ppp_ptr);
    */
    stage++;

    /* Stage 4 - Create a pool of message buffers */
    ppp_ptr->MSG_POOL = RTCS_msgpool_create(sizeof(PPP_MESSAGE), PPP_MESSAGE_INITCOUNT, PPP_MESSAGE_GROWTH, PPP_MESSAGE_LIMIT);
    if (ppp_ptr->MSG_POOL == MSGPOOL_NULL_POOL_ID)
    {
        PPP_init_fail(ppp_ptr, stage);
        return(NULL);
    }
    stage++;

    /* Stage 5 - Create the Tx Task */
    error = RTCS_task_create("PPP tx", _PPPTASK_priority, _PPPTASK_stacksize + 1000, PPP_tx_task, ppp_ptr);
    if (error)
    {
        PPP_init_fail(ppp_ptr, stage);
        return(NULL);
    } 
    stage++;

    /* Stage 6 - Create the Rx Task */
    ppp_ptr->STOP_RX = FALSE; 

    error = RTCS_task_create("PPP rx", _PPPTASK_priority, _PPPTASK_stacksize + 1000, PPP_rx_task, ppp_ptr);
    if (error)
    {
        PPP_init_fail(ppp_ptr, stage);
        return(NULL);
    }
    stage++;

    /* Stage 7 - Open HDLC layer */
    error = _iopcb_open(ppp_ptr->DEVICE, PPP_lowerup, PPP_lowerdown, ppp_ptr);
    if (error != PPPHDLC_OK)
    {
        PPP_init_fail(ppp_ptr, stage);
        return(NULL);
    }
    ppp_ptr->VALID = PPP_VALID;
    stage++;

    /* Stage 8 - Add PPP interface to RTCS */
    error = RTCS_if_add(ppp_ptr, RTCS_IF_PPP, &ppp_ptr->IF_HANDLE);
    if (error != RTCS_OK)
    {
        PPP_init_fail(ppp_ptr, stage);
        return(NULL);
    }
    stage++;

    /* Stage 9 - Bind IP address to PPP interface */
    _mem_zero(&ipcp_data, sizeof(ipcp_data));
    ipcp_data.IP_UP              = params->up;
    ipcp_data.IP_DOWN            = params->down;
    ipcp_data.IP_PARAM           = params->callback_param;

    if(params->listen_flag == 0)
    {
        ipcp_data.ACCEPT_LOCAL_ADDR  = TRUE;
        ipcp_data.ACCEPT_REMOTE_ADDR = TRUE;
    }
    else
    {
        ipcp_data.ACCEPT_LOCAL_ADDR  = FALSE;
        ipcp_data.ACCEPT_REMOTE_ADDR = FALSE;
    }
    ipcp_data.LOCAL_ADDR         = params->local_addr;
    ipcp_data.REMOTE_ADDR        = params->remote_addr;
    ipcp_data.DEFAULT_NETMASK    = TRUE;
    ipcp_data.DEFAULT_ROUTE      = TRUE;

    error = RTCS_if_bind_IPCP(ppp_ptr->IF_HANDLE, &ipcp_data);
    if (error != RTCS_OK)
    {
        PPP_init_fail(ppp_ptr, stage);
        return(NULL);
    }
    params->if_handle = ppp_ptr->IF_HANDLE;
    stage++;

    /* Stage 10 - Init complete, return handle */
    return(ppp_ptr);

#else

    return(NULL);    

#endif /* RTCSCFG_ENABLE_IP4 */

} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPP_init_fail
* Returned Value  :  none
* Comments        :
*     Do cleanup when initialization fails.
*
*END*-----------------------------------------------------------------*/
void PPP_init_fail(PPP_CFG_PTR ppp_ptr, int stage)
{
    if (stage > 0)
    {
        fclose(ppp_ptr->IOPCB_DEVICE);
        _mem_free(ppp_ptr->DEVICE_NAME);
        ppp_ptr->DEVICE_NAME = NULL;
    }
    if (stage > 1)
    {
        _lwsem_destroy(&ppp_ptr->MUTEX);
    }
    if (stage > 2)
    {
        LCP_destroy(ppp_ptr);
    }
    if (stage > 3)
    {
       // CCP_close(ppp_ptr);
       // CCP_destroy(ppp_ptr);
    }
    if (stage > 5)
    {
        LWSEM_STRUCT sem;

        RTCS_sem_init(&sem);
        PPP_send_shutdown(ppp_ptr, &sem);
        RTCS_sem_wait(&sem);
        RTCS_sem_destroy(&sem);
    }
    /* Message pool must be destroyed after we send shutdown message to TX task */
    if (stage > 4)
    {
        RTCS_msgpool_destroy(ppp_ptr->MSG_POOL);
    }
    if (stage > 6)
    {
        uint32_t        wait_time  = 50; 
        uint32_t        delay_time = 100;

        ppp_ptr->STOP_RX = TRUE;
        while(ppp_ptr->STOP_RX)  
        {  
            _time_delay(delay_time);
            wait_time--;
            if(!wait_time)
            {
                RTCS_task_destroy(ppp_ptr->RX_TASKID);
                break;
            }
        }
    }
    if (stage > 7)
    {
        _iopcb_close(ppp_ptr->DEVICE);
    }
    if (stage > 8)
    {
        RTCS_if_unbind(ppp_ptr->IF_HANDLE, IPCP_get_local_addr(ppp_ptr->IF_HANDLE));
        RTCS_if_remove(ppp_ptr->IF_HANDLE);
    }
    _mem_free(ppp_ptr);
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPP_shutdown
* Returned Value  :  error code
* Comments        :
*     Destroys a PPP handle and frees all resources.
*
*END*-----------------------------------------------------------------*/

uint32_t PPP_release
   (
      _ppp_handle  handle
       /* [IN] - the PPP state structure */
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4 
    PPP_CFG_PTR    ppp_ptr = handle;
    _rtcs_sem      sem;
    uint32_t        error = RTCS_OK;
    /* wait time  in 0.1 Sec */
    uint32_t        wait_time  = 50; 
    /* delay time in mS */ 
    uint32_t        delay_time = 100; 

    if (ppp_ptr == NULL)
    {
        return(RTCS_OK);
    }

    error = RTCS_if_unbind(ppp_ptr->IF_HANDLE, IPCP_get_local_addr(ppp_ptr->IF_HANDLE));
    if (error != RTCS_OK)
    {
        return(error);
    }

    ppp_ptr->STOP_RX = TRUE;

    /* Waiting before Rx task  will be closed or kill it. */   
    while(ppp_ptr->STOP_RX)  
    {  
        _time_delay(delay_time);
        wait_time--;
        if(!wait_time)
        {
            error = RTCSERR_TIMEOUT;
            RTCS_task_destroy(ppp_ptr->RX_TASKID);
            break;
        }
    }

    error = PPP_close(ppp_ptr);
    if (error != PPP_OK)
    {
        return(error);
    }

    /* Kill Tx task */
    RTCS_sem_init(&sem);
    PPP_send_shutdown(ppp_ptr, &sem);
    RTCS_sem_wait(&sem);
    RTCS_sem_destroy(&sem);

    error = _iopcb_close(ppp_ptr->DEVICE);
    if (error != RTCS_OK)
    {
        return(error);
    }

    error = _iopcb_ppphdlc_release(ppp_ptr->DEVICE);
    if (error != RTCS_OK)
    {
        return(error);
    }
    ppp_ptr->DEVICE = NULL;
    
    error = RTCS_msgpool_destroy(ppp_ptr->MSG_POOL);
    if (error != MQX_OK)
    {
        return(error);
    }
    /*
    error = CCP_destroy(ppp_ptr);
    if (error != PPP_OK)
    {
        return(error);
    }
    */
    error = LCP_destroy(ppp_ptr);
    if (error != PPP_OK)
    {
        return(error);
    }
    
    PPP_mutex_destroy(&ppp_ptr->MUTEX);
    
    if (ppp_ptr->IOPCB_DEVICE)
    {
        fflush(ppp_ptr->IOPCB_DEVICE);
        error = fclose(ppp_ptr->IOPCB_DEVICE);
        if (error != RTCS_OK)
        {
            return(error);
        }
    }
    
    error = RTCS_if_remove(ppp_ptr->IF_HANDLE);
    if (error != RTCS_OK)
    {
        return(error);
    }
    
    PPP_memfree(handle);
    return(error);
#else

    return RTCSERR_IP_IS_DISABLED;

#endif /* RTCSCFG_ENABLE_IP4 */   

} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPP_pause
* Returned Value  :  RTCS_OK
* Comments        :
*     Pauses PPP FSM and releases HDLC layer.
*
*END*-----------------------------------------------------------------*/
uint32_t PPP_pause(_ppp_handle handle)
{
#if RTCSCFG_ENABLE_IP4 
    PPP_CFG_PTR    ppp_ptr = handle;
    
    RTCS_task_destroy(ppp_ptr->RX_TASKID);
    /*
    ** Close HDLC layer, also signalize that lower layer is down to PPP 
    ** state machine 
    */
    _iopcb_close(ppp_ptr->DEVICE);
    /* Release low-level (typycally ittyX) device */
    fflush(ppp_ptr->IOPCB_DEVICE);
    fclose(ppp_ptr->IOPCB_DEVICE);
    ppp_ptr->IOPCB_DEVICE = NULL;
    return(RTCS_OK);
#else
    return RTCSERR_IP_IS_DISABLED;
#endif
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPP_resume
* Returned Value  :  RTCS_OK
* Comments        :
*     Resume PPP FSM and open HDLC layer.
*
*END*-----------------------------------------------------------------*/
uint32_t PPP_resume(_ppp_handle handle)
{
#if RTCSCFG_ENABLE_IP4 
    PPP_CFG_PTR    ppp_ptr = handle;
    uint32_t        error = RTCS_OK;
    
    /* Reopen low-level device */
    ppp_ptr->IOPCB_DEVICE = fopen(ppp_ptr->DEVICE_NAME, NULL);
    
    /* Open HDLC and resume PPP FSM */
    _iopcb_open(ppp_ptr->DEVICE, PPP_lowerup, PPP_lowerdown, ppp_ptr);
    error = RTCS_task_create("PPP rx", _PPPTASK_priority, _PPPTASK_stacksize + 1000, PPP_rx_task, ppp_ptr);

    return(error);
#else
    return RTCSERR_IP_IS_DISABLED;
#endif
}

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPP_pcballoc
* Returned Value  :  pointer to allocated PCB
* Comments        :
*     Called by PPPFSM_send* and {P,CH}AP_send.  Dynamically
*     allocates a PCB.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

PCB_PTR PPP_pcballoc
   (
      uint16_t protocol,
            /* [IN] - PPP protocol */
      uint32_t size
            /* [IN] - max size of packet, excluding protocol */
   )
{ /* Body */
   PCB_FRAGMENT      *pcb_frag_ptr;
   PCB_PTR packet;

   packet = _mem_alloc_system(sizeof(PCB) + sizeof(PCB_FRAGMENT) + 2 + size);
   if (packet)
   {
      packet->FREE = (void (_CODE_PTR_)(PCB_PTR))_mem_free;
      pcb_frag_ptr = packet->FRAG;
      pcb_frag_ptr->LENGTH   = size + 2;
      pcb_frag_ptr->FRAGMENT = (unsigned char *)packet + sizeof(PCB) + sizeof(PCB_FRAGMENT);
      mqx_htons(pcb_frag_ptr->FRAGMENT, protocol);
      pcb_frag_ptr++;
      pcb_frag_ptr->LENGTH   = 0;
      pcb_frag_ptr->FRAGMENT = NULL;
   } /* Endif */

   return packet;

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPP_getcall
* Returned Value  :  error code
* Comments        :
*     Retrieves an application callback function.
*
*END*-----------------------------------------------------------------*/

uint32_t PPP_getcall
   (
      _ppp_handle             handle,
            /* [IN] - the PPP state structure */
      uint32_t                 callnum,
            /* [IN] - PPP callback */
      void (_CODE_PTR_   *callback)(),
            /* [OUT] - callback function */
      void              **callparam
            /* [OUT] - callback parameter */
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4 

   PPP_CFG_PTR ppp_ptr = handle;
   PPP_CALL_INTERNAL_PTR call_ptr;

      /* Do some error checking */
  if (ppp_ptr->VALID != PPP_VALID) {
      return RTCSERR_PPP_INVALID_HANDLE;
   } else if (callnum >= PPP_CALL_MAX) {
      return RTCSERR_PPP_INVALID_CALLBACK;
   } /* Endif */

   call_ptr   = &ppp_ptr->LCP_CALL[callnum];
   *callback  = call_ptr->CALLBACK;
   *callparam = call_ptr->PARAM;
   return PPP_OK;

#else

    return RTCSERR_IP_IS_DISABLED;    

#endif /* RTCSCFG_ENABLE_IP4 */    

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPP_setcall
* Returned Value  :  error code
* Comments        :
*     Sets an application callback function.
*
*END*-----------------------------------------------------------------*/

uint32_t PPP_setcall
   (
      _ppp_handle             handle,
            /* [IN] - the PPP state structure */
      uint32_t                 callnum,
            /* [IN] - PPP callback */
      void (_CODE_PTR_   *callback)(),
            /* [IN/OUT] - callback function */
      void              **callparam
            /* [IN/OUT] - callback parameter */
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4 

   PPP_CFG_PTR ppp_ptr = handle;
   PPP_CALL_INTERNAL_PTR call_ptr;
   void (_CODE_PTR_ oldfn)();
   void   *oldparam;

      /* Do some error checking */
   if (ppp_ptr->VALID != PPP_VALID) {
      return RTCSERR_PPP_INVALID_HANDLE;
   } else if (callnum >= PPP_CALL_MAX) {
      return RTCSERR_PPP_INVALID_CALLBACK;
   } /* Endif */

   call_ptr = &ppp_ptr->LCP_CALL[callnum];
   oldfn    = call_ptr->CALLBACK;
   oldparam = call_ptr->PARAM;
   call_ptr->CALLBACK = *callback;
   call_ptr->PARAM    = *callparam;
   *callback  = oldfn;
   *callparam = oldparam;
   return PPP_OK;

#else

    return RTCSERR_IP_IS_DISABLED;    

#endif /* RTCSCFG_ENABLE_IP4 */    

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPP_getmtu
* Returned Value  :  MTU
* Comments        :
*     Retrieves the negotiated MTU (i.e. the peer's MRU)
*
*END*-----------------------------------------------------------------*/

uint32_t PPP_getmtu
   (
      _ppp_handle       handle
            /* [IN] - the PPP state structure */
   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4

   PPP_CFG_PTR ppp_ptr = handle;
   return ppp_ptr->SEND_OPTIONS->MRU;

#else

    return 0;    

#endif /* RTCSCFG_ENABLE_IP4 */ 
   
} /* Endbody */


/* Start CR 2207 */
/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPP_set_secrets
* Returned Value  :  
* Comments        :
*     
*
*END*-----------------------------------------------------------------*/

bool PPP_set_secrets
   (
      _ppp_handle       handle,
            /* [IN] - the PPP state structure */
      uint32_t           secret_number,

      PPP_SECRET_PTR    secret_ptr

   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4
   #if PPP_SECRETS_NOT_SHARED
   PPP_CFG_PTR ppp_ptr = handle;
   #endif
   bool     success=TRUE;

   switch (secret_number) {
      case PPP_PAP_LSECRET:
         PPP_SECRET(ppp_ptr, _PPP_PAP_LSECRET) = secret_ptr;
         break;

      case PPP_PAP_RSECRETS:
         PPP_SECRET(ppp_ptr, _PPP_PAP_RSECRETS) = secret_ptr;
         break;

      case PPP_CHAP_LSECRETS:
         PPP_SECRET(ppp_ptr, _PPP_CHAP_LSECRETS) = secret_ptr;
         break;

      case PPP_CHAP_RSECRETS:
         PPP_SECRET(ppp_ptr, _PPP_CHAP_RSECRETS) = secret_ptr;
         break;
   
      default:
         success=FALSE;
   }

   return success;

#else

    return FALSE;    

#endif /* RTCSCFG_ENABLE_IP4 */ 
   
} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPP_set_chap_lname
* Returned Value  :  
* Comments        :
*     
*
*END*-----------------------------------------------------------------*/

bool PPP_set_chap_lname
   (
      _ppp_handle       handle,
            /* [IN] - the PPP state structure */
      char    *lname

   )
{ /* Body */

#if RTCSCFG_ENABLE_IP4
   #if PPP_SECRETS_NOT_SHARED
   PPP_CFG_PTR ppp_ptr = handle;
   #endif
   PPP_SECRET(ppp_ptr, _PPP_CHAP_LNAME) = lname;
   return TRUE;

#else

    return FALSE;

#endif /* RTCSCFG_ENABLE_IP4 */ 
   
} /* Endbody */

#endif // RTCSCFG_ENABLE_PPP
