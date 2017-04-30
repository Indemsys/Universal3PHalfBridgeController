
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
* See license agreement file for full license terms including other restrictions.
*****************************************************************************
*
* Comments:
*
*   This include file is used to define constants and data types for the
*   support functions for MQX I/O Components.
*
*
*END************************************************************************/
#ifndef __mqx_ioc_h__
#define __mqx_ioc_h__ 1

/*--------------------------------------------------------------------------*/
/*                        CONSTANT DEFINITIONS                              */


/*
 * The IO component indexes, used to index into the component
 * arrays to access component specific data
 */
#define IO_SUBSYSTEM_COMPONENT        (0)
#define IO_RTCS_COMPONENT             (1)
#define IO_SNMP_COMPONENT             (2)
#define IO_MFS_COMPONENT              (3)
#define IO_USB_COMPONENT              (4)
#define IO_SHELL_COMPONENT            (5)
#define IO_ENET_COMPONENT             (6)
#define IO_EDS_COMPONENT              (7)
//#define IO_LAPB_COMPONENT             (2)  // obsolete
//#define IO_LAPD_COMPONENT             (3)  // obsolete
//#define IO_SDLC_COMPONENT             (4)  // obsolete
//#define IO_HDLC_COMPONENT             (5)
//#define IO_MFS_COMPONENT              (6)
//#define IO_CAN_COMPONENT              (7)
//#define IO_PPP_COMPONENT              (8)
//#define IO_SNMP_COMPONENT             (9)
//#define IO_EDS_COMPONENT              (10)
//#define IO_USB_COMPONENT              (11)
//#define IO_SHELL_COMPONENT            (12)

/* The maximum number of IO components */
#define MAX_IO_COMPONENTS                  (8)

/*--------------------------------------------------------------------------*/
/*                        DATATYPE DECLARATIONS                             */


/*--------------------------------------------------------------------------*/
/*                           EXTERNAL DECLARATIONS                          */

#ifndef __ASM__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TAD_COMPILE__
extern void   *_mqx_get_io_component_handle(_mqx_uint);
extern void   *_mqx_set_io_component_handle(_mqx_uint, void *);
extern uint32_t _mqx_link_io_component_handle(_mqx_uint, void *, void **);
extern uint32_t _mqx_unlink_io_component_handle(_mqx_uint, void *, void **);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __ASM__ */

#endif
/* EOF */
