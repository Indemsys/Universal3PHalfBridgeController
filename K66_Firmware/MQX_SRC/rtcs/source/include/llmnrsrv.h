/*HEADER**********************************************************************
*
* Copyright 2014 Freescale Semiconductor, Inc.
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
*   Header for LLMNRSRV.
*
*
*END************************************************************************/

#ifndef LLMNRSRV_H_
#define LLMNRSRV_H_

#include <rtcs.h>

/*
** LLMNR server parameters
*/
typedef struct llmnrsrv_host_name_struct
{
    char                    *host_name;         /* Link-local host name advertised by LLMNR server (null-terminated). */
    uint32_t                host_name_ttl;      /* TTL value that indicates for how many seconds link-local host name is valid for LLMNR querier, in seconds (OPTIONAL).
                                                 * Default value is defined by RTCSCFG_LLMNRSRV_HOSTNAME_TTL. */
} LLMNRSRV_HOST_NAME_STRUCT;

/*
** LLMNR server parameters
*/
typedef struct llmnrsrv_param_struct
{
    _rtcs_if_handle             interface;          /* Interface to configure LLMNR for. */
    LLMNRSRV_HOST_NAME_STRUCT   *host_name_table;   /* Pointer to host name table (null terminated).*/
    uint16_t                    af;                 /* Inet protocol family (IPv6 or IPv4 or both) the server will listen for LLMNR query (OPTIONAL).*/
    uint32_t                    task_prio;          /* Server main task priority (OPTIONAL)*/
} LLMNRSRV_PARAM_STRUCT;

#ifdef __cplusplus
extern "C" {
#endif

/*
** Initialize and run LLMNR server
** Returns server handle when successful, zero otherwise.
*/
uint32_t LLMNRSRV_init(LLMNRSRV_PARAM_STRUCT *);

/*
** Stop and release LLMNR server
** Returns RTCS_OK when successful, error code otherwise.
*/
uint32_t LLMNRSRV_release(uint32_t);

#ifdef __cplusplus
}
#endif


#endif /* LLMNRSRV_H_ */
