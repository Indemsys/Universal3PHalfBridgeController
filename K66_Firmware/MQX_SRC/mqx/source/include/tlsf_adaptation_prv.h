
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
*   This file contains functions of the Two Level Segregate Fit, 
*   based on public domain licensed implementation from http://tlsf.baisoku.org/ .
*
*
*END************************************************************************/
#ifndef __tlsf_prv_h__
#define __tlsf_prv_h__
#include "tlsf.h"
/*--------------------------------------------------------------------------*/
/*
 *                    CONSTANT DEFINITIONS
 */


/*--------------------------------------------------------------------------*/
/*
 *                    DATATYPE DECLARATIONS
 */


/*--------------------------------------------------------------------------*/
/*
 *                      MACROS DEFINITIONS
 */

/*
 * get the location of the block pointer, given the address as provided
 * to the application by _tlsf_alloc.
 */
#define GET_TLSFBLOCK_PTR(addr)       tlsf_get_adaptation_from_ptr(addr)

#define _GET_TLSFBLOCK_TYPE(ptr)      _tlsf_get_type(ptr)

/*--------------------------------------------------------------------------*/
/*
 *                  PROTOTYPES OF FUNCTIONS
 */

#ifdef __cplusplus
extern "C" {
#endif
#ifndef __TAD_COMPILE__

extern void     *_tlsf_alloc_internal(_mem_size, TD_STRUCT_PTR, tlsf_t, bool);
extern void     *_tlsf_alloc_at_internal(_mem_size, void *, TD_STRUCT_PTR, tlsf_t, bool);
extern void     *_tlsf_alloc_align_internal(_mem_size, _mem_size, TD_STRUCT_PTR, tlsf_t, bool);
extern _mem_size _tlsf_get_free_internal(tlsf_t);
extern void      _tlsf_transfer_internal(void *, TD_STRUCT_PTR);
extern _mqx_uint _tlsf_transfer_td_internal(void *, TD_STRUCT_PTR,
   TD_STRUCT_PTR);
extern _mqx_uint _tlsf_init_internal(void);
extern tlsf_t _tlsf_create_pool(void *, _mem_size);
extern void* tlsf_get_adaptation_from_ptr(void* userData);
extern void* tlsf_get_ptr_from_adaptation(void* adapt);
extern void *_tlsf_get_next_block_internal(TD_STRUCT_PTR td_ptr, void* memory_ptr);

#endif
#ifdef __cplusplus
}
#endif

#endif /* __tlsf_prv_h__ */
/* EOF */
