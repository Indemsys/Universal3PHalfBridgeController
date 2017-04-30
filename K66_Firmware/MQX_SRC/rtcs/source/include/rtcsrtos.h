#ifndef __rtcsrtos_h__
#define __rtcsrtos_h__
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
*   This file contains the definitions required to
*   use RTCS with MQX.
*
*
*END************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/* MQX is the only one supported */
#if PLATFORM_SDK_ENABLED
//#include <mqx_sdk_config.h>
  #include <mqx_cnfg.h>
#else
#include <user_config.h>
#endif
#include <rtcs25x.h>


/***************************************
**
** Random Number Generation
*/

void     RTCS_rand_seed (uint32_t);
uint32_t  RTCS_rand      (void);


/***************************************
**
** The _iopcb_handle
*/

#ifndef __IOPCB__
#define __IOPCB__

#define IOPCB_IOCTL_S_ACCM       4
#define IOPCB_IOCTL_R_ACCM       5
#define IOPCB_IOCTL_GET_IFTYPE   6
#define IOPCB_IOCTL_S_PFC        14
#define IOPCB_IOCTL_R_PFC        15
#define IOPCB_IOCTL_S_ACFC       16
#define IOPCB_IOCTL_R_ACFC       17

typedef struct _iopcb_table {
   uint32_t (_CODE_PTR_ OPEN)  (struct _iopcb_table *, void (_CODE_PTR_)(void *), void (_CODE_PTR_)(void *), void *);
   uint32_t (_CODE_PTR_ CLOSE) (struct _iopcb_table *);
   PCB_PTR (_CODE_PTR_ READ)  (struct _iopcb_table *, uint32_t);
   void    (_CODE_PTR_ WRITE) (struct _iopcb_table *, PCB_PTR, uint32_t);
   uint32_t (_CODE_PTR_ IOCTL) (struct _iopcb_table *, uint32_t, void *);
}      *_iopcb_handle;

#define _iopcb_open(h,up,down,param)   ((h)->OPEN(h,up,down,param))
#define _iopcb_close(h)                ((h)->CLOSE(h))
#define _iopcb_read(h,flags)           ((h)->READ(h,flags))
#define _iopcb_write(h,pcb,flags)      ((h)->WRITE(h,pcb,flags))
#define _iopcb_ioctl(h,opt,val)        ((h)->IOCTL(h,opt,val))

#endif




#ifdef __cplusplus
}
#endif


#endif
/* EOF */
