
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


#ifndef __tlsf_h__
#define __tlsf_h__

#include "mqx_cnfg.h"

#include "tlsf.h"
#if (! MQX_USE_TLSF_ALLOCATOR) && (! defined (MQX_DISABLE_CONFIG_CHECK))
#error tlsf component is currently disabled in MQX kernel. Please set MQX_USE_TLSF_ALLOCATOR to 1 in user_config.h and recompile kernel.
#endif

#ifndef __ASM__
/*--------------------------------------------------------------------------*/
/*
 *                    DATATYPE DECLARATIONS
 */

#define POOL_USER_RW_ACCESS     (MPU_UM_RW)
#define POOL_USER_RO_ACCESS     (MPU_UM_R)
#define POOL_USER_NO_ACCESS     (0)

/* TLSF BLOCK STRUCT */
/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief This structure is used to define the storage blocks used by the memory
 * manager in MQX.
 */
typedef struct
{
#if MQX_ALLOW_TYPED_MEMORY
    union 
    {
      uint32_t MQX_DATA;
      struct 
      {
         _mem_type TYPE;
         uint16_t RSVD;
      }S;
    }U;
#endif
    
#if MQX_ALLOCATOR_GARBAGE_COLLECTING
/* Ownership used for garbage collection and error checking */
    void* owner; 
/* When using garbage collection, prev and next pointers are used for faster implementation */
    void* prev; 
    void* next;
#endif
  tlsf_t pool;
}tlsf_adaptation_structure_t;
/*! \endcond */

/*--------------------------------------------------------------------------*/
/*
 *                  PROTOTYPES OF FUNCTIONS
 */

#ifdef __cplusplus
extern "C" {
#endif
#ifndef __TAD_COMPILE__

extern void            *_tlsf_alloc(_mem_size);
extern void            *_tlsf_alloc_at(_mem_size, void *);
extern void            *_tlsf_alloc_align(_mem_size, _mem_size);
extern void            *_tlsf_alloc_align_from(tlsf_t, _mem_size, _mem_size);
extern void            *_tlsf_alloc_zero(_mem_size);
extern void            *_tlsf_alloc_from(tlsf_t, _mem_size);
extern void            *_tlsf_alloc_zero_from(tlsf_t, _mem_size);
extern _mem_size        _tlsf_get_free(void);
extern _mem_size        _tlsf_get_free_from(tlsf_t);

extern void            *_tlsf_alloc_system(_mem_size);
extern void            *_tlsf_alloc_system_align(_mem_size, _mem_size);
extern void            *_tlsf_alloc_system_align_from(tlsf_t, _mem_size, _mem_size);
extern void            *_tlsf_alloc_system_zero(_mem_size);
extern void            *_tlsf_alloc_system_from(tlsf_t, _mem_size);
extern void            *_tlsf_alloc_system_zero_from(tlsf_t, _mem_size);
extern tlsf_t   _tlsf_create_pool(void *start, _mem_size  size);
extern _mqx_uint        _tlsf_free(void *);
extern _mqx_uint        _tlsf_get_size(void *);
extern tlsf_t   _tlsf_set_default_pool(tlsf_t);
extern _mqx_uint        _tlsf_transfer(void *, _task_id, _task_id);

extern tlsf_t   _tlsf_get_system_pool_id(void);
extern _mem_type        _tlsf_get_type(void *);
extern bool          _tlsf_set_type(void *,_mem_type);
extern void            *_tlsf_get_highwater(void) ;

extern void *_tlsf_alloc_uncached(_mem_size size);
extern void *_tlsf_alloc_align_uncached(_mem_size size,_mem_size align);
extern void *_tlsf_alloc_system_uncached(_mem_size size);
extern void *_tlsf_alloc_system_zero_uncached(_mem_size size);
extern void* tlsf_get_adaptation_from_ptr(void* userData);
extern void* tlsf_get_ptr_from_adaptation(void* adapt);
extern _mqx_uint _tlsf_extend(void* start_of_pool, _mem_size size);
extern _mqx_uint _tlsf_extend_pool(tlsf_t pool_id, void* start_of_pool, _mem_size size);
extern void* _tlsf_realloc(void* mem_ptr, _mem_size requested_size);
extern _mqx_uint _tlsf_test_pool(void* pool_id);
extern _mqx_uint _tlsf_test(void);
extern _mqx_uint _tlsf_pool_is_valid(void* pool_id);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _ASM_ */


#endif /* __tlsf_h__ */
/* EOF */

