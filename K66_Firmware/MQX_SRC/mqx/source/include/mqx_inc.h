
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
*   This file contains include statements for all header files
*   required when compiling the kernel.
*
*
*END************************************************************************/

#ifndef __mqx_inc_h__
#define __mqx_inc_h__

#ifdef MQX_REDUCE_DEBUG /* CR1446 & CR1434 */
#if defined(__HIGHC__) && !defined(MQX_CRIPPLED_EVALUATION)
   /* Minimize symbolic debug info when MetaWare compiler */
#  pragma on(nodebug)
# endif
#endif

/*
** These are the 'MQX' include files
*/
#include "mqx.h"
#include "mqx_ioc.h"
#include "psp.h"
#include "psp_comp.h"
#include "mem_prv.h"
#include "mqx_prv.h"
#include "psp_prv.h"
#include "psp_abi.h"
#include "fio.h"

#ifdef MQX_REDUCE_DEBUG /* CR1446 & CR1434 */
# if defined(__HIGHC__) && !defined(MQX_CRIPPLED_EVALUATION)
   /* Minimize symbolic debug info when MetaWare compiler */
#  pragma pop(nodebug)
# endif
#endif

#if MQX_KERNEL_LOGGING
#include "lwlog.h"
#include "klog.h"
#endif
#if MQX_USE_LWMEM_ALLOCATOR
#ifndef __MEMORY_MANAGER_COMPILE__
#include "lwmem_prv.h"
#endif
#endif
#if MQX_USE_TLSF_ALLOCATOR
#ifndef __MEMORY_MANAGER_COMPILE__
#include "tlsf_adaptation_prv.h"
#endif
#endif

#endif /* __mqx_inc_h__ */
/* EOF */
