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
*   This file contains the implementation of getsockopt()
*   and setsockopt() at the SOL_LINK level.
*
*
*END************************************************************************/

#include <rtcs.h>
#include "rtcs_prv.h"
#include "tcpip.h"

#define RTCS_ENTER(f,a)

#define RTCS_EXIT(f,a)  return a

uint32_t SOL_LINK_getsockopt (uint32_t, uint32_t, uint32_t, void *, uint32_t *);
uint32_t SOL_LINK_setsockopt (uint32_t, uint32_t, uint32_t, const void *, uint32_t);

const RTCS_SOCKOPT_CALL_STRUCT SOL_LINK_CALL = {
   SOL_LINK_getsockopt,
   SOL_LINK_setsockopt
};


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOL_LINK_getsockopt
* Returned Value  : error code
* Comments  :  Obtain the current value for a socket option.
*
*END*-----------------------------------------------------------------*/

uint32_t  SOL_LINK_getsockopt
   (
      uint32_t        sock,
         /* [IN] socket handle */
      uint32_t        level,
         /* [IN] protocol level for the option */
      uint32_t        optname,
         /* [IN] name of the option */
      void          *optval,
         /* [IN] buffer for the current value of the option */
      uint32_t   *optlen
         /* [IN/OUT] length of the option value, in bytes */
   )
{

#if RTCSCFG_LINKOPT_8023 || RTCSCFG_ENABLE_8021Q
    SOCKET_STRUCT_PTR    socket_ptr = (SOCKET_STRUCT_PTR)sock;
#endif

    RTCS_ENTER(GETSOCKOPT, sock);
     
    if (*optlen < sizeof(uint32_t))
    {
        RTCS_EXIT(GETSOCKOPT, RTCSERR_SOCK_SHORT_OPTION);
    }

    switch (optname)
    {

#if RTCSCFG_LINKOPT_8023
    case RTCS_SO_LINK_RX_8023:
        *(uint32_t *)optval = socket_ptr->LINK_OPTIONS.RX.OPT_8023;
        *optlen = sizeof(uint32_t);
        RTCS_EXIT(GETSOCKOPT, RTCS_OK);
    case RTCS_SO_LINK_TX_8023:
        *(uint32_t *)optval = socket_ptr->LINK_OPTIONS.TX.OPT_8023;
        *optlen = sizeof(uint32_t);
        RTCS_EXIT(GETSOCKOPT, RTCS_OK);
#endif

#if RTCSCFG_ENABLE_8021Q
    case RTCS_SO_LINK_RX_8021Q_PRIO:
        *(int32_t *)optval = socket_ptr->LINK_OPTIONS.RX.OPT_PRIO ? socket_ptr->LINK_OPTIONS.RX.PRIO : -1;
        *optlen = sizeof(uint32_t);
        RTCS_EXIT(GETSOCKOPT, RTCS_OK);
    case RTCS_SO_LINK_TX_8021Q_PRIO:
        *(int32_t *)optval = socket_ptr->LINK_OPTIONS.TX.OPT_PRIO ? socket_ptr->LINK_OPTIONS.TX.PRIO : -1;
        *optlen = sizeof(uint32_t);
        RTCS_EXIT(GETSOCKOPT, RTCS_OK);
    case RTCS_SO_LINK_RX_8021Q_VID:
        *(int32_t *)optval = socket_ptr->LINK_OPTIONS.RX.OPT_VID ? socket_ptr->LINK_OPTIONS.RX.VID : -1;
        *optlen = sizeof(uint32_t);
        RTCS_EXIT(GETSOCKOPT, RTCS_OK);
    case RTCS_SO_LINK_TX_8021Q_VID:
        *(int32_t *)optval = socket_ptr->LINK_OPTIONS.TX.OPT_VID ? socket_ptr->LINK_OPTIONS.TX.VID : -1;
        *optlen = sizeof(uint32_t);
        RTCS_EXIT(GETSOCKOPT, RTCS_OK);
#endif

    default:
        RTCS_EXIT(GETSOCKOPT, RTCSERR_SOCK_INVALID_OPTION);
   }
} 


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : SOL_LINK_setsockopt
* Returned Value  : error code
* Comments  :  Modify the current value for a socket option.
*
*END*-----------------------------------------------------------------*/

uint32_t  SOL_LINK_setsockopt
   (
      uint32_t        sock,
         /* [IN] socket handle */
      uint32_t        level,
         /* [IN] protocol level for the option */
      uint32_t        optname,
         /* [IN] name of the option */
      const void          *optval,
         /* [IN] new value for the option */
      uint32_t        optlen
         /* [IN] length of the option value, in bytes */
   )
{

#if RTCSCFG_LINKOPT_8023 || RTCSCFG_ENABLE_8021Q
    SOCKET_STRUCT_PTR    socket_ptr = (SOCKET_STRUCT_PTR)sock;
#endif

    RTCS_ENTER(SETSOCKOPT, sock);
    
    if (optlen < sizeof(uint32_t))
    {
        RTCS_EXIT(GETSOCKOPT, RTCSERR_SOCK_SHORT_OPTION);
    }

    switch (optname)
    {

#if RTCSCFG_LINKOPT_8023
    case RTCS_SO_LINK_TX_8023:
        socket_ptr->LINK_OPTIONS.TX.OPT_8023 = *(uint32_t *)optval ? TRUE : FALSE;
        RTCS_EXIT(SETSOCKOPT, RTCS_OK);
#endif

#if RTCSCFG_ENABLE_8021Q
    case RTCS_SO_LINK_TX_8021Q_PRIO:
        if (*(int32_t *)optval == -1)
        {
            socket_ptr->LINK_OPTIONS.TX.OPT_PRIO = 0;
        }
        else
        {
            socket_ptr->LINK_OPTIONS.TX.OPT_PRIO = 1;
            socket_ptr->LINK_OPTIONS.TX.PRIO = *(int32_t *)optval;
        }
        /* Propogate the option to the TCB */
        if (socket_ptr->TCB_PTR != NULL)
        {
            socket_ptr->TCB_PTR->TX.OPT_PRIO = socket_ptr->LINK_OPTIONS.TX.OPT_PRIO;
            socket_ptr->TCB_PTR->TX.PRIO = socket_ptr->LINK_OPTIONS.TX.PRIO;
        }
        RTCS_EXIT(SETSOCKOPT, RTCS_OK);
    case RTCS_SO_LINK_TX_8021Q_VID:
        if (*(int32_t *)optval == -1)
        {
            socket_ptr->LINK_OPTIONS.TX.OPT_VID = 0;
        }
        else
        {
            socket_ptr->LINK_OPTIONS.TX.OPT_VID = 1;
            socket_ptr->LINK_OPTIONS.TX.VID = *(int32_t *)optval;
        }
        /* Propogate the option to the TCB */
        if (socket_ptr->TCB_PTR != NULL)
        {
            socket_ptr->TCB_PTR->TX.OPT_VID = socket_ptr->LINK_OPTIONS.TX.OPT_VID;
            socket_ptr->TCB_PTR->TX.VID = socket_ptr->LINK_OPTIONS.TX.VID;
        }
        RTCS_EXIT(SETSOCKOPT, RTCS_OK);
#endif

   default:
        RTCS_EXIT(SETSOCKOPT, RTCSERR_SOCK_INVALID_OPTION);
   } 

}


/* EOF */
