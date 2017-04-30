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
*   This file contains functions of the Two Level Segregate Fit adaptation
*   to a common MQX _mem_* interface.
*
*
*END************************************************************************/
#include <stdint.h>

#include "mqx_inc.h"
#include "mqx_prv.h"

#if MQX_USE_TLSF_ALLOCATOR
#include "tlsf_adaptation.h"
#include "tlsf_adaptation_prv.h"
#include "tlsf.h"
#include "tlsfbits.h"

#ifndef MQX_TLSF_MEASURE_HIGHWATER
    #define MQX_TLSF_MEASURE_HIGHWATER 0
#endif

#if MQX_ALLOCATOR_ALLOW_IN_ISR
#define TLSF_MEM_PROT_ENTER() _int_disable()
#define TLSF_MEM_PROT_EXIT()  _int_enable()
#else
#define TLSF_MEM_PROT_ENTER() _task_stop_preemption()
#define TLSF_MEM_PROT_EXIT()  _task_allow_preemption()
#endif

#if MQX_USE_UNCACHED_MEM && PSP_HAS_DATA_CACHE

extern unsigned char __UNCACHED_DATA_START[];
extern unsigned char __UNCACHED_DATA_END[];

#endif /* MQX_USE_UNCACHED_MEM && PSP_HAS_DATA_CACHE */

void* _tlsf_pool_create_limited_internal(void* tlsf_pool_ptr, unsigned char* start, unsigned char* end);

void* tlsf_get_adaptation_from_ptr(void* userData)
{
    return (void*)((unsigned char*)userData - sizeof(tlsf_adaptation_structure_t));
}

void* tlsf_get_ptr_from_adaptation(void* adapt)
{
    return (void*)((unsigned char*)adapt + sizeof(tlsf_adaptation_structure_t));
}

/*!
 * \private
 *
 * \brief This routine returns what the next memory block is for a given memory
 * block (where the memory block is on a tasks resource list).
 *
 * \param[in] td_ptr     The task descriptor being checked.
 * \param[in] memory_ptr The address (USERS_AREA) of the memory block.
 *
 * return Pointer to next block.
 */
void *_tlsf_get_next_block_internal
(
    TD_STRUCT_PTR td_ptr,
    void   *memory_ptr
)
{ /* Body */
 
#if MQX_ALLOCATOR_GARBAGE_COLLECTING 
    if (memory_ptr == NULL) {
        if(td_ptr != NULL)
        {
          return tlsf_get_ptr_from_adaptation(td_ptr->FIRST_OWNED_BLOCK);
        }
        else
        {
          return NULL;
        }
    }
    else {
        memory_ptr = ((tlsf_adaptation_structure_t*)tlsf_get_adaptation_from_ptr(memory_ptr))->next;
        if(memory_ptr != NULL)
        {
          return tlsf_get_ptr_from_adaptation(memory_ptr);
        }
        else
        {
          return NULL;
        }
        
    } /* Endif */
    
#else
    return NULL; /* for compatibility when MQX_ALLOCATOR_GARBAGE_COLLECTING==0 */
#endif
} /* Endbody */



/*!
 * \brief Adds the specified memory area for use by the memory manager.
 *
 * The function adds the specified memory to the default memory pool.
 * \n The function fails if size is less than block_size_min,
 * as defined in tlsf.c
 *
 * \param[in] start_of_pool Pointer to the start of the memory to add.
 * \param[in] size          Size of the memory pool to add.
 *
 * \return MQX_OK
 * \return MQX_INVALID_SIZE (Failure, see Description.)
 * \return MQX_INVALID_COMPONENT_HANDLE (memory pool to extend is not valid.)
 *
 * \see _tlsf_get_highwater
 * \see MQX_INITIALIZATION_STRUCT
 */
_mqx_uint _tlsf_extend
(
    void   *start_of_pool,
    _mem_size size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    _mqx_uint result;

    _GET_KERNEL_DATA(kernel_data);

    _KLOGE3(KLOG_tlsf_extend, start_of_pool, size);

    result = _tlsf_extend_pool((tlsf_t) kernel_data->KERNEL_TLSF_POOL, start_of_pool, size);

    _KLOGX2(KLOG_tlsf_extend, result);

    return (result);

} /* Endbody */

/*!
 * \brief Adds physical memory to the memory pool, which is outside the default
 * memory pool.
 *
 * The function adds the specified memory to the memory pool.
 * \n The function fails if size is less than block_size_min,
 * as defined in tlsf.c
 *
 * \param[in] pool_id       Pool to which to add memory (from _tlsf_create_pool()).
 * \param[in] start_of_pool Pointer to the start of the memory to add.
 * \param[in] size          Size of the memory pool addition.
 *
 * \return MQX_OK
 * \return MQX_INVALID_SIZE (Failure, see Description.)
 * \return MQX_INVALID_COMPONENT_HANDLE (memory pool to extend is not valid.)
 *
 * \see _tlsf_create_pool
 * \see _tlsf_get_highwater_pool
 */
_mqx_uint _tlsf_extend_pool
(
    tlsf_t pool_id,
    void        *start_of_pool,
    _mem_size    size
)
{ /* Body */
     KERNEL_DATA_STRUCT_PTR kernel_data;
    _mqx_uint result = MQX_OK;
    _GET_KERNEL_DATA(kernel_data);
    (void)kernel_data; /* Warning suppression with KLOG disabled */

    _KLOGE4(KLOG_tlsf_extend_pool, start_of_pool, size, pool_id);
    
#if MQX_CHECK_VALIDITY
    if (_tlsf_pool_is_valid(pool_id) != MQX_OK)
    {
        _task_set_error(MQX_TLSF_POOL_INVALID);
        _KLOGX2(KLOG_tlsf_extend_pool, MQX_TLSF_POOL_INVALID);
        return MQX_TLSF_POOL_INVALID;
    } /* Endif */
#endif

#ifdef MQX_CHECK_ERRORS
#if !MQX_ALLOCATOR_ALLOW_IN_ISR  
    if (kernel_data->IN_ISR)
    {
#if defined(_DEBUG)
      assert(0 && "If you want to use allocator in ISR, set MQX_ALLOCATOR_ALLOW_IN_ISR to 1");
#endif
      _KLOGX2(_tlsf_extend_pool, MQX_CANNOT_CALL_FUNCTION_FROM_ISR);
      return (MQX_CANNOT_CALL_FUNCTION_FROM_ISR);
    }

#endif
#endif

    if((start_of_pool == NULL)||(pool_id == NULL))
    {
      result =  MQX_INVALID_COMPONENT_HANDLE;
    }
    else
    {
      TLSF_MEM_PROT_ENTER();
      if(NULL == _tlsf_pool_create_limited_internal(pool_id, (unsigned char*)start_of_pool, ((unsigned char*)start_of_pool) + size))
      {
        result = MQX_INVALID_SIZE;
      }
      TLSF_MEM_PROT_EXIT();
    }
    _KLOGX2(KLOG_tlsf_extend_pool, result);

    return (result);

} /* Endbody */


/*!
 * \brief Allocates a private block function kept for compatibility,
 * it actually calls _tlsf_alloc function and allocates the 
 * requested size of memory.
 *
 * See Description of _tlsf_alloc() function.
 *
 * \param[in] requested_size Number of single-addressable units to allocate.
 * \param[in] requested_addr Start address of the memory block.
 *
 * \return Pointer to the TLSF memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_LWMEM_POOL_INVALID (Memory pool to allocate from is invalid.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 *
 */
void *_tlsf_alloc_at
(
    _mem_size requested_size,
    void     *requested_addr
)
{ /* Body */
    (void)requested_addr;
    return _tlsf_alloc(requested_size);
}

/*!
 * \brief Gets the highest size memory address used,
 * function kept for API compatibility, unimplemented.
 *
 * \return Highest memory address used
 */
void *_tlsf_get_highwater(void)
{

  return NULL;
}

/*!
 * \brief Gets the size of unallocated (free) memory in system pool.
 *
 * \return Size of free memory (success).
 * \return 0 (failure)
 *
 * \warning On failure, calls _task_set_error() to set the following task error code:
 * \li MQX_LWMEM_POOL_INVALID (Pool that contains the block is not valid.)
 */
_mem_size _tlsf_get_free
(
    void
)
{ 
    KERNEL_DATA_STRUCT_PTR kernel_data;
    _GET_KERNEL_DATA(kernel_data);
    return _tlsf_get_free_from(kernel_data->KERNEL_TLSF_POOL);
}

/*!
 * \brief Gets the size of unallocated (free) memory from a specified pool.
 *
 * \param[in] pool_id The pool to get free size from. (instance of the allocator)
 *
 * \return Size of free memory (success).
 * \return 0 (failure)
 *
 * \warning On failure, calls _task_set_error() to set the following task error code:
 * \li MQX_LWMEM_POOL_INVALID (Pool that contains the block is not valid.)
 */
_mem_size _tlsf_get_free_from
(
    void* pool_id
)
{
    uint16_t i,j;
    _mem_size retval = 0;
    control_t* ctrl = pool_id;
    block_header_t* block;
    
#if MQX_CHECK_VALIDITY
    if (_tlsf_pool_is_valid(pool_id) != MQX_OK)
    {
        _task_set_error(MQX_TLSF_POOL_INVALID);
        return 0;
    } /* Endif */
#endif

#ifdef MQX_CHECK_ERRORS
    {
#if !MQX_ALLOCATOR_ALLOW_IN_ISR  
        KERNEL_DATA_STRUCT_PTR kernel_data;
        _GET_KERNEL_DATA(kernel_data);
        if (kernel_data->IN_ISR)
        {
#if defined(_DEBUG)
          assert(0 && "If you want to use allocator in ISR, set MQX_ALLOCATOR_ALLOW_IN_ISR to 1");
#endif
          _KLOGX2(_tlsf_get_free_from, MQX_CANNOT_CALL_FUNCTION_FROM_ISR);
          return 0;
        }
#endif
    }
#endif
    TLSF_MEM_PROT_ENTER();
    for(i = 0; i < FL_INDEX_COUNT; i++)
    {
        for(j = 0; j < SL_INDEX_COUNT; j++)
        {
            block = ctrl->blocks[i][j];
            while(block != (&ctrl->block_null))
            {
                /* first two bits contain information about the state of the block and its predecessor, mask them */
                retval += ((block->size)&(0xFFFFFFFC)); 
                block = block->next_free;
            }
        }
    }
    TLSF_MEM_PROT_EXIT();
    return retval;
}

/*!
 * \private
 *
 * \brief Allocates a block of memory for a task from the pool_id instance.
 *
 * \param[in] requested_size The size of the memory block.
 * \param[in] td_ptr         The owner TD.
 * \param[in] pool_id        Which pool to allocate from.
 * \param[in] zero           Zero the memory after it is allocated.
 *
 * \return Pointer to the TLSF memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 */
void *_tlsf_alloc_internal
(
    _mem_size      requested_size,
    TD_STRUCT_PTR  td_ptr,
    tlsf_t         pool_id,
    bool           zero
)
{ /* Body */
    tlsf_adaptation_structure_t* adapt;
    void* retval;
    
#if MQX_CHECK_VALIDITY
    if (_tlsf_pool_is_valid(pool_id) != MQX_OK)
    {
        _task_set_error(MQX_TLSF_POOL_INVALID);
        return NULL;
    } /* Endif */
#endif

#if PSP_HAS_DATA_CACHE
    /* Make sure the size of aligned block is also aligned */
    requested_size = (requested_size + PSP_MEMORY_ALIGNMENT) & (~PSP_MEMORY_ALIGNMENT);
#endif

#ifdef MQX_CHECK_ERRORS
    {
        KERNEL_DATA_STRUCT_PTR kernel_data;
        _GET_KERNEL_DATA(kernel_data);
        (void)kernel_data; /* Warning suppression with KLOG disabled */
        
#if !MQX_ALLOCATOR_ALLOW_IN_ISR  
        if (kernel_data->IN_ISR)
        {
#if defined(_DEBUG)
            assert(0 && "If you want to use allocator in ISR, set MQX_ALLOCATOR_ALLOW_IN_ISR to 1");
#endif
	    _task_set_error(MQX_CANNOT_CALL_FUNCTION_FROM_ISR);
            _KLOGX2(_tlsf_alloc_internal, MQX_CANNOT_CALL_FUNCTION_FROM_ISR);
            return NULL;
        }
#endif
				
        if(((size_t)requested_size + sizeof(tlsf_adaptation_structure_t)) > tlsf_block_size_max())
        {
            _task_set_error(MQX_TLSF_TOO_LARGE_BLOCK);
            _KLOGX2(_tlsf_alloc_internal, MQX_TLSF_TOO_LARGE_BLOCK);
            return NULL;
        }
    }
#endif
    
#if PSP_HAS_DATA_CACHE
    adapt = (tlsf_adaptation_structure_t*)tlsf_memalign(pool_id, (PSP_MEMORY_ALIGNMENT + 1), (size_t)requested_size + sizeof(tlsf_adaptation_structure_t), sizeof(tlsf_adaptation_structure_t), (PSP_MEMORY_ALIGNMENT + 1));
#else
    adapt = (tlsf_adaptation_structure_t*)tlsf_malloc(pool_id, requested_size  + sizeof(tlsf_adaptation_structure_t));
#endif    
    if (adapt == NULL)
    {
        _task_set_error(MQX_OUT_OF_MEMORY);
        return NULL;
    }
   
#if MQX_ALLOW_TYPED_MEMORY
    adapt->U.S.TYPE = 0;
#endif
    
    adapt->pool = pool_id;
    
#if MQX_ALLOCATOR_GARBAGE_COLLECTING
    adapt->owner = (void*)td_ptr;
    adapt->prev = NULL;
    if(td_ptr->FIRST_OWNED_BLOCK == NULL)
    {
        td_ptr->FIRST_OWNED_BLOCK = (void*)adapt;
        adapt->next = NULL;
    }
    else
    {
       ((tlsf_adaptation_structure_t*)(td_ptr->FIRST_OWNED_BLOCK))->prev = adapt;
       adapt->next = td_ptr->FIRST_OWNED_BLOCK;
       td_ptr->FIRST_OWNED_BLOCK = (void*)adapt;     
    }
#endif
    
    retval = (void*)((unsigned char*)adapt + sizeof(tlsf_adaptation_structure_t));
    
    if (zero)
    {
        _mem_zero((void*)retval, requested_size);
    } /* Endif */

    return retval;

} /* Endbody */


/*!
 * \private
 *
 * \brief Allocate an aligned block of memory for a task from the pool_id instance.
 *
 * \param[in] requested_size The size of the memory block.
 * \param[in] req_align      Requested alignement (e.g. 8 for alignement to 8 bytes).
 * \param[in] td_ptr         The owner TD.
 * \param[in] pool_id        Which pool to allocate from.
 * \param[in] zero           Zero the memory after it is allocated.
 *
 * \return Pointer to the TLSF memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_INVALID_PARAMETER (Requested alignment is not power of 2.)
 */
void *_tlsf_alloc_align_internal
(
    _mem_size      requested_size,
    _mem_size      req_align,
    TD_STRUCT_PTR  td_ptr,
    tlsf_t pool_id,
    bool        zero
)
{ /* Body */
     tlsf_adaptation_structure_t* adapt;
     void* retval;
     
     if(!requested_size)
     {
        _task_set_error(MQX_INVALID_PARAMETER);
        return NULL;
     }
          
     /* Check if reg_align is power of 2 */
    if ((req_align != 0) && (req_align & (req_align - 1)))
    {
        _task_set_error(MQX_INVALID_PARAMETER);
        return (NULL); /* request failed */
    }

#if PSP_HAS_DATA_CACHE
    if(req_align < (PSP_MEMORY_ALIGNMENT + 1))
    {
        req_align = (PSP_MEMORY_ALIGNMENT + 1);
    }
    
    /* Make sure the size of aligned block is also aligned */
    requested_size = (requested_size + req_align-1) & (~(req_align-1));
#else
    if(req_align < 4)
    {
        req_align = 4;
    }
#endif


#if MQX_CHECK_VALIDITY
    if (_tlsf_pool_is_valid(pool_id) != MQX_OK)
    {
        _task_set_error(MQX_TLSF_POOL_INVALID);
        return (NULL);
    } /* Endif */
#endif
#ifdef MQX_CHECK_ERRORS
    {
        KERNEL_DATA_STRUCT_PTR kernel_data;
        _GET_KERNEL_DATA(kernel_data);
        (void)kernel_data; /* Warning suppression with KLOG disabled */
        
#if !MQX_ALLOCATOR_ALLOW_IN_ISR  
        if (kernel_data->IN_ISR)
        {
#if defined(_DEBUG)
            assert(0 && "If you want to use allocator in ISR, set MQX_ALLOCATOR_ALLOW_IN_ISR to 1");
#endif
            _KLOGX2(_tlsf_alloc_align_internal, MQX_CANNOT_CALL_FUNCTION_FROM_ISR);
            return NULL;
        }
#endif			
        if(((size_t)requested_size + sizeof(tlsf_adaptation_structure_t)) > tlsf_block_size_max())
        {
           _task_set_error(MQX_TLSF_TOO_LARGE_BLOCK);
           _KLOGX2(_tlsf_alloc_align_internal, MQX_TLSF_TOO_LARGE_BLOCK);
           return NULL;
        }
    }
#endif

#if PSP_HAS_DATA_CACHE
     adapt = (tlsf_adaptation_structure_t*)tlsf_memalign(pool_id, (size_t)req_align, (size_t)requested_size + sizeof(tlsf_adaptation_structure_t), sizeof(tlsf_adaptation_structure_t), (PSP_MEMORY_ALIGNMENT + 1));
#else
     adapt = (tlsf_adaptation_structure_t*)tlsf_memalign(pool_id, (size_t)req_align, (size_t)requested_size + sizeof(tlsf_adaptation_structure_t), sizeof(tlsf_adaptation_structure_t), 4);
#endif
     
    if (adapt == NULL)
    {
        _task_set_error(MQX_OUT_OF_MEMORY);
        return NULL;
    }

#if MQX_ALLOW_TYPED_MEMORY
    adapt->U.S.TYPE = 0;
#endif
    
    adapt->pool = pool_id;
        
#if MQX_ALLOCATOR_GARBAGE_COLLECTING
    adapt->owner = (void*)td_ptr;
    adapt->prev = NULL;
    if(td_ptr->FIRST_OWNED_BLOCK == NULL)
    {
        td_ptr->FIRST_OWNED_BLOCK = (void*)adapt;
        adapt->next = NULL;
    }
    else
    {
       ((tlsf_adaptation_structure_t*)(td_ptr->FIRST_OWNED_BLOCK))->prev = adapt;
       adapt->next = td_ptr->FIRST_OWNED_BLOCK;
       td_ptr->FIRST_OWNED_BLOCK = (void*)adapt;     
    }
#endif

    retval =  (void*)((unsigned char*)adapt + sizeof(tlsf_adaptation_structure_t)); 
    
    if (zero) 
    {
        _mem_zero(retval, requested_size);
    } /* Endif */
    

    return retval;
}

/*!
 * \brief Allocates a private block of TLSF memory block from the default
 * memory pool.
 *
 * The application must first set a value for the default TLSF memory pool
 * by calling _tlsf_set_default_pool().
 * \n The _tlsf_alloc functions allocate at least size single-addressable units;
 * the actual number might be greater. The start address of the block is aligned
 * so that tasks can use the returned pointer as a pointer to any data type without
 * causing an error.
 * \n Tasks cannot use TLSF memory blocks as messages. Tasks must use
 * _msg_alloc() or _msg_alloc_system() to allocate messages.
 * \n Only the task that owns a TLSF memory block that was allocated with
 * one of the following functions can free the block:
 * \li _tlsf_alloc()
 * \li _tlsf_alloc_zero()
 * \li _tlsf_alloc_at()
 *
 * \n Any task can free a TLSF memory block that is allocated with one of
 * the following functions:
 * \li _tlsf_alloc_system()
 * \li _tlsf_alloc_system_zero()
 * \li _tlsf_alloc_system_align()
 *
 * \n Function types:
 * <table>
 *  <tr>
 *    <td></td>
 *    <td><b>Allocate this type of TLSF memory block form the default memory
 *    pool:</b></td>
 *  </tr>
 *  <tr>
 *    <td><b>_tlsf_alloc()</b></td>
 *    <td>Private</td>
 *  </tr>
 *  <tr>
 *    <td><b>_tlsf_alloc_system()</b></td>
 *    <td>System</td>
 *  </tr>
 *  <tr>
 *    <td><b>_tlsf_alloc_system_zero()</b></td>
 *    <td>System (zero-filled)</td>
 *  </tr>
 *  <tr>
 *    <td><b>_tlsf_alloc_zero()</b></td>
 *    <td>Private (zero-filled)</td>
 *  </tr>
 *  <tr>
 *    <td><b>_tlsf_alloc_at()</b></td>
 *    <td>Private (start address defined)</td>
 *  </tr>
 *  <tr>
 *    <td><b>_tlsf_alloc_system_align()</b></td>
 *    <td>System (aligned) </td>
 *  </tr>
 *  <tr>
 *    <td><b>_tlsf_alloc_align()</b></td>
 *    <td>Private (aligned) </td>
 *  </tr>
 * </table>
 *
 * \param[in] requested_size Number of single-addressable units to allocate.
 *
 * \return Pointer to the TLSF memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 *
 */
void *_tlsf_alloc
(
    _mem_size requested_size
)
{
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;
    
    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_tlsf_alloc, requested_size);
    
    TLSF_MEM_PROT_ENTER();
    
    result = _tlsf_alloc_internal(requested_size, kernel_data->ACTIVE_PTR,
                    (tlsf_t) kernel_data->KERNEL_TLSF_POOL, FALSE);

    TLSF_MEM_PROT_EXIT();
    
    _KLOGX2(KLOG_tlsf_alloc, result);
    return (result);
}

/*!
 * \brief Allocates an aligned block of TLSF memory block from the default
 * memory pool.
 *
 * See Description of _tlsf_alloc() function.
 *
 * \param[in] requested_size Number of single-addressable units to allocate.
 * \param[in] req_align      Align requested value.
 *
 * \return Pointer to the TLSF memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_INVALID_PARAMETER (Requested alignment is not power of 2.)
 *
 */
void *_tlsf_alloc_align
(
    _mem_size requested_size,
    _mem_size req_align
)
{
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE3(KLOG_tlsf_alloc_align, requested_size, req_align);

    TLSF_MEM_PROT_ENTER();
    
    result = _tlsf_alloc_align_internal(requested_size, req_align, kernel_data->ACTIVE_PTR,
                    (tlsf_t) kernel_data->KERNEL_TLSF_POOL, FALSE);

    TLSF_MEM_PROT_EXIT();
    
    _KLOGX2(KLOG_tlsf_alloc_align, result);

    return (result);
}

/*!
 * \brief Gets default system tlsf pool.
 *
 * \return Pointer to default szstem tlsf pool.
 * \return NULL (failure)
 */
tlsf_t _tlsf_get_system_pool_id(void)
{
    register KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);

    return (tlsf_t) kernel_data->KERNEL_TLSF_POOL;
}

/*!
 * \brief Allocates a private block of TLSF memory block from the specified
 * memory pool.
 *
 * The function is similar to _tlsf_alloc(), _tlsf_alloc_system(),
 * _tlsf_alloc_system_zero() and _tlsf_alloc_zero() except that the application
 * does not call _tlsf_set_default_pool() first.
 * \n Only the task that owns a TLSF memory block that was allocated with
 * one of the following functions can free the block:
 * \li _tlsf_alloc_from()
 * \li _tlsf_alloc_zero_from()
 *
 * \n Any task can free a TLSF memory block that is allocated with one of
 * the following functions:
 * \li _tlsf_alloc_system_from()
 * \li _tlsf_alloc_system_zero_from()
 * \li _tlsf_alloc_system_align_from()
 *
 * \n Function types:
 * <table>
 *  <tr>
 *    <td></td>
 *    <td><b>Allocate this type of TLSF memory block form the specified
 *    TLSF memory pool:</b></td>
 *  </tr>
 *  <tr>
 *    <td><b>_tlsf_alloc_from()</b></td>
 *    <td>Private</td>
 *  </tr>
 *  <tr>
 *    <td><b>_tlsf_alloc_system_from()</b></td>
 *    <td>System</td>
 *  </tr>
 *  <tr>
 *    <td><b>_tlsf_alloc_system_zero_from()</b></td>
 *    <td>System (zero-filled)</td>
 *  </tr>
 *  <tr>
 *    <td><b>_tlsf_alloc_zero_from()</b></td>
 *    <td>Private (zero-filled)</td>
 *  </tr>
 *  <tr>
 *    <td><b>_tlsf_alloc_system_align_from()</b></td>
 *    <td>System (aligned) </td>
 *  </tr>
 *  <tr>
 *    <td><b>_tlsf_alloc_align_from()</b></td>
 *    <td>Private (aligned) </td>
 *  </tr>
 * </table>
 *
 * \param[in] pool_id        TLSF memory pool from which to allocate the
 * TLSF memory block (from _tlsf_create_pool()).
 * \param[in] requested_size Number of single-addressable units to allocate.
 *
 * \return Pointer to the TLSF memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_TLSF_POOL_INVALID (Memory pool to allocate from is invalid.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 *
 */
void *_tlsf_alloc_from
(
    tlsf_t pool_id,
    _mem_size      requested_size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE3(KLOG_tlsf_alloc_from, pool_id, requested_size);

    TLSF_MEM_PROT_ENTER();
    
    result = _tlsf_alloc_internal(requested_size, kernel_data->ACTIVE_PTR, pool_id, FALSE);

    TLSF_MEM_PROT_EXIT();
    
    _KLOGX2(KLOG_tlsf_alloc_from, result);
    return (result);
}

/*!
 * \brief Allocates a private block of TLSF memory block from the specified
 * memory pool.
 *
 * See Description of _tlsf_alloc_from() function.
 *
 * \param[in] pool_id        TLSF memory pool from which to allocate the
 * TLSF memory block (from _tlsf_create_pool()).
 * \param[in] requested_size Number of single-addressable units to allocate.
 * \param[in] req_align      Align requested value.
 *
 * \return Pointer to the TLSF memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_INVALID_PARAMETER (Requested alignment is not power of 2.)
 *
 */
void *_tlsf_alloc_align_from
(
    tlsf_t pool_id,
    _mem_size      requested_size,
    _mem_size      req_align
)
{
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE4(KLOG_tlsf_alloc_align_from, pool_id, requested_size, req_align);

    result = _tlsf_alloc_align_internal(requested_size, req_align, kernel_data->ACTIVE_PTR, pool_id, FALSE);

    _KLOGX2(KLOG_tlsf_alloc_align_from, result);

    return (result);
}

/*!
 * \brief Allocates a private (zero-filled) block of TLSF memory block from
 * the default memory pool.
 *
 * See Description of _tlsf_alloc() function.
 *
 * \param[in] size Number of single-addressable units to allocate.
 *
 * \return Pointer to the TLSF memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 *
 */
void *_tlsf_alloc_zero
(
    _mem_size size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_tlsf_alloc_zero, size);

    TLSF_MEM_PROT_ENTER();
    
    result = _tlsf_alloc_internal(
        size, kernel_data->ACTIVE_PTR, (tlsf_t) kernel_data->KERNEL_TLSF_POOL, TRUE
    );

    TLSF_MEM_PROT_EXIT();
    
    _KLOGX2(KLOG_tlsf_alloc_zero, result);
    return (result);

} /* Endbody */



/*!
 * \brief Allocates a private (zero-filled) block of TLSF memory block from the specified
 * memory pool.
 *
 * See Description of _tlsf_alloc_from() function.
 *
 * \param[in] pool_id        TLSF memory pool from which to allocate the
 * TLSF memory block (from _tlsf_create_pool()).
 * \param[in] size Number of single-addressable units to allocate.
 *
 * \return Pointer to the TLSF memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 *
 */
void *_tlsf_alloc_zero_from
(
    void     *pool_id,
    _mem_size size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE3(KLOG_tlsf_alloc_zero_from, pool_id, size);

    TLSF_MEM_PROT_ENTER();
    
    result = _tlsf_alloc_internal(size, kernel_data->ACTIVE_PTR, (tlsf_t)pool_id, TRUE);

    TLSF_MEM_PROT_EXIT();
    
    _KLOGX2(KLOG_tlsf_alloc_zero_from, result);
    return (result);

} /* Endbody */

/*!
 * \brief Creates the TLSF memory pool.
 *
 * Tasks use the pool ID (a pointer) to allocate (variable-size) TLSF memory blocks
 * from the pool.
 *
 * \param[in] start        Start of the memory for the pool.
 * \param[in] size         Number of single-addressable units in the pool.
 *
 * \return Pool ID
 *
 */
tlsf_t _tlsf_create_pool
(
    void          *start,
    _mem_size     size
)
{ /* Body */
    tlsf_t mem_pool_ptr;

    KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);
    (void)kernel_data; /* Warning suppression with KLOG disabled */

    _KLOGE3(KLOG_tlsf_create_pool, start, size);

    mem_pool_ptr = _tlsf_pool_create_limited_internal(NULL, (unsigned char*)start, (unsigned char*)((unsigned char*)start + size));
    
    if(mem_pool_ptr == NULL)
    {
        _task_set_error(MQX_INVALID_SIZE);
        _KLOGX2(KLOG_tlsf_create_pool, MQX_INVALID_SIZE);
        return NULL;
    }
    else
    {
        _KLOGX2(KLOG_tlsf_create_pool, mem_pool_ptr);
        return  mem_pool_ptr;
    }
}


/*!
 * \brief Frees the TLSF memory block.
 *
 * If the block was allocated with one of the following functions, only the task
 * that owns the block can free it:
 * \li _tlsf_alloc()
 * \li _tlsf_alloc_from()
 * \li _tlsf_alloc_zero()
 * \li _tlsf_alloc_zero_from()
 *
 * \n Any task can free a block that was allocated with one of the following functions:
 * \li _tlsf_alloc_system()
 * \li _tlsf_alloc_system_from()
 * \li _tlsf_alloc_system_zero()
 * \li _tlsf_alloc_system_zero_from()
 * \li _tlsf_alloc_system_align()
 * \li _tlsf_alloc_system_align_from()
 *
 * \n The function also coalesces any free block found physically on either side
 * of the block being freed. 
 *
 * \param[in] mem_ptr Pointer to the block to free.
 *
 * \return MQX_OK
 * \return MQX_INVALID_POINTER (mem_ptr is NULL.)
 * \return MQX_TLSF_POOL_INVALID (Pool that contains the block is not valid.)
 * \return MQX_NOT_RESOURCE_OWNER (If the block was allocated with _tlsf_alloc()
 * or _tlsf_alloc_zero(), only the task that allocated it can free part of it.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following
 * task error codes:
 * \li MQX_INVALID_POINTER (mem_ptr is NULL.)
 * \li MQX_NOT_RESOURCE_OWNER (If the block was allocated with _tlsf_alloc() or
 * _tlsf_alloc_zero(), only the task that allocated it can free part of it.)
 *
 */
_mqx_uint _tlsf_free
(
    void   *mem_ptr
)
{ /* Body */
    tlsf_adaptation_structure_t* adapt;
#if (MQX_ALLOCATOR_GARBAGE_COLLECTING && MQX_CHECK_ERRORS || MQX_KERNEL_LOGGING)   
    KERNEL_DATA_STRUCT_PTR kernel_data;
    _GET_KERNEL_DATA(kernel_data);
#endif
#ifdef MQX_CHECK_ERRORS
#if !MQX_ALLOCATOR_ALLOW_IN_ISR  
    if (kernel_data->IN_ISR)
    {
#if defined(_DEBUG)
        assert(0 && "If you want to use allocator in ISR, set MQX_ALLOCATOR_ALLOW_IN_ISR to 1");
#endif
        _KLOGX2(_tlsf_free, MQX_CANNOT_CALL_FUNCTION_FROM_ISR);
        return MQX_CANNOT_CALL_FUNCTION_FROM_ISR;
    }
#endif
#endif
    
#if MQX_CHECK_ERRORS
    if (mem_ptr == NULL)
    {
        _task_set_error(MQX_INVALID_POINTER);
        return (MQX_INVALID_POINTER);
    } /* Endif */
#endif
      
    _KLOGE2(KLOG_tlsf_free, mem_ptr);

    adapt = (tlsf_adaptation_structure_t*)tlsf_get_adaptation_from_ptr(mem_ptr);
    
#if MQX_CHECK_VALIDITY
    if (_tlsf_pool_is_valid(adapt->pool) != MQX_OK)
    {
        _task_set_error(MQX_TLSF_POOL_INVALID);
        _KLOGX2(KLOG_tlsf_free, MQX_TLSF_POOL_INVALID);
        return MQX_TLSF_POOL_INVALID;
    } /* Endif */
#endif
    
    TLSF_MEM_PROT_ENTER();
    
#if MQX_ALLOCATOR_GARBAGE_COLLECTING
    
#if MQX_CHECK_ERRORS
    /* Verify the passed in parameter */
    if ((adapt->owner != (void*)kernel_data->ACTIVE_PTR)
                    && (adapt->owner != (void*)SYSTEM_TD_PTR(kernel_data)))
    {
        _task_set_error(MQX_NOT_RESOURCE_OWNER);
        _KLOGE2(KLOG_tlsf_free, MQX_NOT_RESOURCE_OWNER);
        TLSF_MEM_PROT_EXIT();
        return (MQX_NOT_RESOURCE_OWNER);
    } /* Endif */
#endif
    
    /* remove this block from the td's linked list */
    if(adapt->prev == NULL)
    {
       ((TD_STRUCT_PTR)(adapt->owner))->FIRST_OWNED_BLOCK = adapt->next;
       if(adapt->next != NULL)
       {
          ((tlsf_adaptation_structure_t*)(adapt->next))->prev = NULL;
       }
       
    }
    else
    {
       ((tlsf_adaptation_structure_t*)(adapt->prev))->next = adapt->next;
       if(adapt->next != NULL)
       {
          ((tlsf_adaptation_structure_t*)(adapt->next))->prev = adapt->prev;
       }
    }
#endif
   
    tlsf_free(adapt->pool, (void*)adapt);
    
    TLSF_MEM_PROT_EXIT();

    _KLOGX2(KLOG_tlsf_free, MQX_OK);
    return (MQX_OK);

} /* Endbody */

/*!
 * \brief Allocates a system block of TLSF memory block from the specified
 * memory pool that is available system-wide.
 *
 * See Description of _tlsf_alloc_from() function.
 *
 * \param[in] pool_id TLSF memory pool from which to allocate the
 * TLSF memory block (from _tlsf_create_pool()).
 * \param[in] size    Number of single-addressable units to allocate.
 *
 * \return Pointer to the TLSF memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 *
 */
void *_tlsf_alloc_system_from
(
    tlsf_t pool_id,
    _mem_size      size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_tlsf_alloc_system_from, size);

    TLSF_MEM_PROT_ENTER();
    
    result = _tlsf_alloc_internal(size, SYSTEM_TD_PTR(kernel_data), pool_id, FALSE);

    TLSF_MEM_PROT_EXIT();
    
    _KLOGX2(KLOG_tlsf_alloc_system_from, result);
    return (result);

} /* Endbody */

/*!
 * \brief Allocates a system block of TLSF memory block from the default
 * memory pool that is available system-wide.
 *
 * See Description of _tlsf_alloc() function.
 *
 * \param[in] size Number of single-addressable units to allocate.
 *
 * \return Pointer to the TLSF memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 *
 */
void *_tlsf_alloc_system
(
    _mem_size size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_tlsf_alloc_system, size);

    TLSF_MEM_PROT_ENTER();
    
    result = _tlsf_alloc_internal(
        size, SYSTEM_TD_PTR(kernel_data), (tlsf_t) kernel_data->KERNEL_TLSF_POOL, FALSE
    );
    
    TLSF_MEM_PROT_EXIT();

    _KLOGX2(KLOG_tlsf_alloc_system, result);
    return (result);

} /* Endbody */

/*!
 * \brief Allocates a system aligned block of TLSF memory block from the default
 * memory pool that is available system-wide.
 *
 * See Description of _tlsf_alloc() function.
 *
 * \param[in] size Number of single-addressable units to allocate.
 * \param[in] req_align      Align requested value.
 *
 * \return Pointer to the TLSF memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_INVALID_PARAMETER (Requested alignment is not power of 2.)
 *
 */
void *_tlsf_alloc_system_align
(
    _mem_size size,
    _mem_size req_align
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE3(KLOG_tlsf_alloc_system_align, size, req_align);

    TLSF_MEM_PROT_ENTER();
    
    result = _tlsf_alloc_align_internal(
        size, req_align, SYSTEM_TD_PTR(kernel_data), (tlsf_t) kernel_data->KERNEL_TLSF_POOL, FALSE
    );
    
    TLSF_MEM_PROT_EXIT();

    _KLOGX2(KLOG_tlsf_alloc_system_align, result);
    return (result);

} /* Endbody */


/*!
 * \brief Allocates a system aligned block of TLSF memory block from from the specified
 * memory pool. system-wide.
 *
 * See Description of _tlsf_alloc_from() function.
 *
 * \param[in] pool_id        TLSF memory pool from which to allocate the
 * TLSF memory block (from _tlsf_create_pool()).
 * \param[in] requested_size Number of single-addressable units to allocate.
 * \param[in] req_align      Align requested value.
 *
 * \return Pointer to the TLSF memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_INVALID_PARAMETER (Requested alignment is not power of 2.)
 *
 */
void *_tlsf_alloc_system_align_from
(
    tlsf_t pool_id,
    _mem_size      requested_size,
    _mem_size      req_align
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE4(KLOG_tlsf_alloc_system_align_from, pool_id, requested_size, req_align);

    TLSF_MEM_PROT_ENTER();
    
    result = _tlsf_alloc_align_internal(requested_size, req_align, SYSTEM_TD_PTR(kernel_data), pool_id, FALSE);

    TLSF_MEM_PROT_EXIT();
    
    _KLOGX2(KLOG_tlsf_alloc_system_align_from, result);
    return (result);

} /* Endbody */

/*!
 * \private
 *
 * \brief Creates a TLSF allocator instance or adds memory to existing instance, helper function
 *
 * \return pointer to TLSF allocator instance
 */
void* _tlsf_pool_create_limited_internal(void* tlsf_pool_ptr, unsigned char* start, unsigned char* end)
{
 #define CORTEX_MEMORY_BARRIER_ADDR ((unsigned char*)0x20000000)
    uint32_t      totlen;

    totlen = (unsigned char *) end - (unsigned char *) start - 4;

    if(tlsf_pool_ptr == NULL)
    {
        if(totlen > tlsf_block_size_max())
        {
            end = start + tlsf_block_size_max() + 4;
        }

        if((start < CORTEX_MEMORY_BARRIER_ADDR)&&(end >= CORTEX_MEMORY_BARRIER_ADDR))
        {
#if PSP_HAS_DATA_CACHE
            tlsf_pool_ptr = tlsf_create_with_pool(start + block_header_overhead, (unsigned char *) CORTEX_MEMORY_BARRIER_ADDR - (unsigned char *) start - (PSP_MEMORY_ALIGNMENT + 1) - block_header_overhead, (PSP_MEMORY_ALIGNMENT + 1));
#else          
            tlsf_pool_ptr = tlsf_create_with_pool(start + block_header_overhead, (unsigned char *) CORTEX_MEMORY_BARRIER_ADDR - (unsigned char *) start - 4 - block_header_overhead, 4);
#endif            
#if PSP_HAS_DATA_CACHE
            tlsf_add_pool(tlsf_pool_ptr, CORTEX_MEMORY_BARRIER_ADDR + block_header_overhead, (unsigned char *) end - (unsigned char *) CORTEX_MEMORY_BARRIER_ADDR - (PSP_MEMORY_ALIGNMENT + 1) - block_header_overhead, (PSP_MEMORY_ALIGNMENT + 1));  
#else
            tlsf_add_pool(tlsf_pool_ptr, CORTEX_MEMORY_BARRIER_ADDR + block_header_overhead, (unsigned char *) end - (unsigned char *) CORTEX_MEMORY_BARRIER_ADDR - 4 - block_header_overhead, 4);
#endif
        }
        else
        {
#if PSP_HAS_DATA_CACHE
            tlsf_pool_ptr = tlsf_create_with_pool(start + block_header_overhead, (unsigned char *) end - (unsigned char *) start - (PSP_MEMORY_ALIGNMENT + 1) - block_header_overhead, (PSP_MEMORY_ALIGNMENT + 1));
#else          
            tlsf_pool_ptr = tlsf_create_with_pool(start + block_header_overhead, (unsigned char *) end - (unsigned char *) start - 4 - block_header_overhead, 4);
#endif            
        }

        if(tlsf_pool_ptr == NULL)
        {
            return NULL;
        }
    }

    totlen -= (unsigned char *) end - (unsigned char *) start - 4;
    start += (unsigned char *) end - (unsigned char *) start - 4;
    

    while(totlen > 0)
    {
        if(totlen >= tlsf_block_size_max())
        {
            end = start + tlsf_block_size_max() + 4;
        }
        else
        {
            end = start + totlen + 4;
        }

        if((start < CORTEX_MEMORY_BARRIER_ADDR)&&(end >= CORTEX_MEMORY_BARRIER_ADDR))
        {
#if PSP_HAS_DATA_CACHE
            tlsf_add_pool(tlsf_pool_ptr, start + block_header_overhead, (unsigned char *) CORTEX_MEMORY_BARRIER_ADDR - (unsigned char *) start - (PSP_MEMORY_ALIGNMENT + 1) - block_header_overhead, (PSP_MEMORY_ALIGNMENT + 1));
            tlsf_add_pool(tlsf_pool_ptr, CORTEX_MEMORY_BARRIER_ADDR + block_header_overhead, (unsigned char *) end - (unsigned char *) CORTEX_MEMORY_BARRIER_ADDR - (PSP_MEMORY_ALIGNMENT + 1) - block_header_overhead, (PSP_MEMORY_ALIGNMENT + 1));
#else
            tlsf_add_pool(tlsf_pool_ptr, start + block_header_overhead, (unsigned char *) CORTEX_MEMORY_BARRIER_ADDR - (unsigned char *) start - 4 - block_header_overhead, 4);
            tlsf_add_pool(tlsf_pool_ptr, CORTEX_MEMORY_BARRIER_ADDR + block_header_overhead, (unsigned char *) end - (unsigned char *) CORTEX_MEMORY_BARRIER_ADDR - 4 - block_header_overhead, 4);
#endif
        }
        else
        {
#if PSP_HAS_DATA_CACHE
            tlsf_add_pool(tlsf_pool_ptr, start + block_header_overhead, (unsigned char *) end - (unsigned char *) start - (PSP_MEMORY_ALIGNMENT + 1) - block_header_overhead, (PSP_MEMORY_ALIGNMENT + 1));
#else            
            tlsf_add_pool(tlsf_pool_ptr, start + block_header_overhead, (unsigned char *) end - (unsigned char *) start - 4 - block_header_overhead, 4);
#endif            
        }
        totlen -= (unsigned char *) end - (unsigned char *) start - 4;
        start += tlsf_block_size_max();
    }

    return tlsf_pool_ptr;
}

/*!
 * \private
 *
 * \brief Initializes MQX to use the tlsf manager.
 *
 * \return MQX_OK
 */
_mqx_uint _tlsf_init_internal(void)
{ /* Body */

#if MQX_USE_UNCACHED_MEM && PSP_HAS_DATA_CACHE
    void   *__uncached_data_start = (void *)__UNCACHED_DATA_START;
    void   *__uncached_data_end   = (void *)__UNCACHED_DATA_END;
#endif /* MQX_USE_UNCACHED_MEM && PSP_HAS_DATA_CACHE */
  
    KERNEL_DATA_STRUCT_PTR kernel_data;
    unsigned char              *start;
    unsigned char              *end;
    

    _GET_KERNEL_DATA(kernel_data);

    /*
     * Move the MQX memory pool pointer past the end of kernel data.
     */
    start = (void *) ((unsigned char *) kernel_data + sizeof(KERNEL_DATA_STRUCT));
    end = (void *) ((unsigned char *) kernel_data->INIT.END_OF_KERNEL_MEMORY);
   
    kernel_data->KERNEL_TLSF_POOL = _tlsf_pool_create_limited_internal(NULL, (unsigned char*)start, (unsigned char*)end);

#if MQX_USE_UNCACHED_MEM && PSP_HAS_DATA_CACHE

    if ((__uncached_data_start <=  start) && (start <= __uncached_data_end)) 
    {
        kernel_data->UNCACHED_POOL = kernel_data->KERNEL_TLSF_POOL;
    }
    else 
    {
        /* The pool state structure is created at the top of the pool */
        kernel_data->UNCACHED_POOL = _tlsf_pool_create_limited_internal(NULL, (unsigned char*)__uncached_data_start, (unsigned char*)__uncached_data_end);
    }   
#endif /* MQX_USE_UNCACHED_MEM && PSP_HAS_DATA_CACHE */

    return (MQX_OK);

} /* Endbody */


/*!
 * \brief Sets the value of the default TLSF memory pool.
 *
 * Because MQX allocates TLSF memory blocks from the default TLSF
 * memory pool when an application calls _tlsf_alloc(), _tlsf_alloc_system(),
 * _tlsf_alloc_system_zero() or _tlsf_alloc_zero(), the application must first
 * call _tlsf_set_default_pool().
 *
 * \param[in] pool_id New pool ID.
 *
 * \return Previous pool ID.
 *
 */
tlsf_t _tlsf_set_default_pool
(
    tlsf_t pool_id
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    tlsf_t         old_pool_id;

    _GET_KERNEL_DATA(kernel_data);
    
#if MQX_CHECK_VALIDITY
    if (_tlsf_pool_is_valid(pool_id) != MQX_OK)
    {
        _task_set_error(MQX_TLSF_POOL_INVALID);
        return NULL;
    } /* Endif */
#endif
#ifdef MQX_CHECK_ERRORS
#if !MQX_ALLOCATOR_ALLOW_IN_ISR  
    if (kernel_data->IN_ISR)
    {
#if defined(_DEBUG)
        assert(0 && "If you want to use allocator in ISR, set MQX_ALLOCATOR_ALLOW_IN_ISR to 1");
#endif
        _KLOGX2(_tlsf_set_default_pool, MQX_CANNOT_CALL_FUNCTION_FROM_ISR);
        return NULL;
    }
#endif
#endif
    
    TLSF_MEM_PROT_ENTER();
    
    old_pool_id = kernel_data->KERNEL_TLSF_POOL;
    kernel_data->KERNEL_TLSF_POOL = pool_id;
    
    TLSF_MEM_PROT_EXIT();
    
    return (old_pool_id);

} /* Endbody */

/*!
 * \brief Gets the size of the TLSF memory block.
 *
 * The size is the actual size of the block and might be larger than the size
 * that a task requested.
 *
 * \param[in] mem_ptr Pointer to the TLSF memory block.
 *
 * \return Number of single-addressable units in the block (success).
 * \return 0 (failure)
 */
_mem_size _tlsf_get_size
(
    void   *mem_ptr
)
{ /* Body */

#if MQX_CHECK_ERRORS
    if (mem_ptr == NULL)
    {
        _task_set_error(MQX_INVALID_POINTER);
        return (0);
    } /* Endif */
#endif
    
    return (tlsf_block_size(tlsf_get_adaptation_from_ptr(mem_ptr)) - sizeof(tlsf_adaptation_structure_t));
} /* Endbody */

/* Move to standalone file */

#if MQX_ALLOW_TYPED_MEMORY
/*!
 * \brief Gets type of the specified block.
 *
 * \param[in] mem_ptr Pointer to the TLSF memory block.
 *
 * \return Type of memory block.
 */
_mem_type _tlsf_get_type
(
    void   *mem_ptr
)
{
    return ((tlsf_adaptation_structure_t*)tlsf_get_adaptation_from_ptr(mem_ptr))->U.S.TYPE;
}

/*!
 * \brief Sets the type of the specified block.
 *
 * \param[in] mem_ptr  Pointer to the TLSF memory block.
 * \param[in] mem_type Type of TLSF memory block to set.
 *
 * \return TRUE (success) or FALSE (failure: mem_ptr is NULL).
 */
bool _tlsf_set_type
(
    void     *mem_ptr,
    _mem_type mem_type
)
{

    if (mem_ptr != NULL)
    {
        TLSF_MEM_PROT_ENTER();
        
        ((tlsf_adaptation_structure_t*)tlsf_get_adaptation_from_ptr(mem_ptr))->U.S.TYPE = mem_type;
        
        TLSF_MEM_PROT_EXIT();
        
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


#endif /* MQX_ALLOW_TYPED_MEMORY */

/*!
 * \brief Transfers the ownership of the TLSF memory block from one task
 * to another. Call ignored if GC is not enabled. 
 *
 * \param[in] memory_ptr The memory block whose ownership is to be transferred.
 * \param[in] source_id  Task ID of the current owner.
 * \param[in] target_id  Task ID of the new owner.
 *
 * \return MQX_OK
 * \return MQX_INVALID_POINTER (Block_ptr is NULL.)
 * \return MQX_INVALID_TASK_ID (Source or target does not represent a valid task.)
 * \return MQX_NOT_RESOURCE_OWNER (Block is not a resource of the task represented
 * by source.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_INVALID_POINTER (Block_ptr is NULL.)
 * \li MQX_INVALID_TASK_ID (Source or target does not represent a valid task.)
 * \li MQX_NOT_RESOURCE_OWNER (Block is not a resource of the task represented
 * by source.)
 *
 */
_mqx_uint _tlsf_transfer
(
    void    *memory_ptr,
    _task_id source_id,
    _task_id target_id
)
{
#if MQX_ALLOCATOR_GARBAGE_COLLECTING
  /* Body */
    _KLOGM(KERNEL_DATA_STRUCT_PTR kernel_data);
    tlsf_adaptation_structure_t* block_ptr;
    TD_STRUCT_PTR          source_td;
    TD_STRUCT_PTR          target_td;

    _KLOGM(_GET_KERNEL_DATA(kernel_data));

    _KLOGE4(KLOG_tlsf_transfer, memory_ptr, source_id, target_id);

#if MQX_CHECK_ERRORS
    if (memory_ptr == NULL)
    {
        _task_set_error(MQX_INVALID_POINTER);
        _KLOGX2(KLOG_tlsf_transfer, MQX_INVALID_POINTER);
        return (MQX_INVALID_POINTER);
    } /* Endif */
#endif

    /* Verify the block */
    block_ptr = (tlsf_adaptation_structure_t*)tlsf_get_adaptation_from_ptr(memory_ptr);

    TLSF_MEM_PROT_ENTER();
    
    source_td = (TD_STRUCT_PTR) _task_get_td(source_id);
    target_td = (TD_STRUCT_PTR) _task_get_td(target_id);
    
#if MQX_CHECK_ERRORS
    if ((source_td == NULL) || (target_td == NULL))
    {
        _task_set_error(MQX_INVALID_TASK_ID);
        _KLOGX2(KLOG_tlsf_transfer, MQX_INVALID_TASK_ID);
        TLSF_MEM_PROT_EXIT();
        return (MQX_INVALID_TASK_ID);
    } /* Endif */
#endif
    
#if MQX_CHECK_ERRORS
    if (block_ptr->owner != (void*)source_td) /* source must be the owner! */
    {
        _task_set_error(MQX_NOT_RESOURCE_OWNER);
        TLSF_MEM_PROT_EXIT();
        _KLOGX2(KLOG_tlsf_transfer, MQX_NOT_RESOURCE_OWNER);
        return (MQX_NOT_RESOURCE_OWNER);
    } /* Endif */
#endif
    
    

    /* remove this block from the source_td's linked list */
    if(block_ptr->prev == NULL)
    {
       ((TD_STRUCT_PTR)(block_ptr->owner))->FIRST_OWNED_BLOCK = block_ptr->next;
       if(block_ptr->next != NULL)
       {
          ((tlsf_adaptation_structure_t*)(block_ptr->next))->prev = NULL;
       }
       
    }
    else
    {
       ((tlsf_adaptation_structure_t*)(block_ptr->prev))->next = block_ptr->next;
       if(block_ptr->next != NULL)
       {
          ((tlsf_adaptation_structure_t*)(block_ptr->next))->prev = block_ptr->prev;
       }
    }

    /* add this block to the head of target_td's linked list */
    block_ptr->owner = (void*)target_td;
    block_ptr->prev = NULL;
    if(target_td->FIRST_OWNED_BLOCK == NULL)
    {
        target_td->FIRST_OWNED_BLOCK = block_ptr;
        block_ptr->next = NULL;
    }
    else
    {
       ((tlsf_adaptation_structure_t*)(target_td->FIRST_OWNED_BLOCK))->prev = block_ptr;
       block_ptr->next = target_td->FIRST_OWNED_BLOCK;
       target_td->FIRST_OWNED_BLOCK = block_ptr;     
    }

    TLSF_MEM_PROT_EXIT();
    
    _KLOGX2(KLOG_tlsf_transfer, MQX_OK);
#else
    (void)memory_ptr;
    (void)source_id;
    (void)target_id;
#endif
    return (MQX_OK);

} /* Endbody */

/*!
 * \private
 *
 * \brief This function transfers the ownership of a block of memory from an owner
 * task to another task.
 *
 * \param[in] memory_ptr The address of the user data in the memory block to transfer.
 * \param[in] target_td  The target task descriptor.
 */
void _tlsf_transfer_internal
(
    void         *memory_ptr,
    TD_STRUCT_PTR target_td
)
{ 
#if MQX_ALLOCATOR_GARBAGE_COLLECTING
  /* Body */
    tlsf_adaptation_structure_t* block_ptr;
    
    block_ptr = (tlsf_adaptation_structure_t*)tlsf_get_adaptation_from_ptr(memory_ptr);
     
    TLSF_MEM_PROT_ENTER();

    /* remove this block from the source_td's linked list */
    if(block_ptr->prev == NULL)
    {
       ((TD_STRUCT_PTR)(block_ptr->owner))->FIRST_OWNED_BLOCK = block_ptr->next;
       if(block_ptr->next != NULL)
       {
          ((tlsf_adaptation_structure_t*)(block_ptr->next))->prev = NULL;
       }
       
    }
    else
    {
       ((tlsf_adaptation_structure_t*)(block_ptr->prev))->next = block_ptr->next;
       if(block_ptr->next != NULL)
       {
          ((tlsf_adaptation_structure_t*)(block_ptr->next))->prev = block_ptr->prev;
       }
    }

    /* add this block to the head of target_td's linked list */
    block_ptr->owner = (void*)target_td;
    block_ptr->prev = NULL;
    if(target_td->FIRST_OWNED_BLOCK == NULL)
    {
        target_td->FIRST_OWNED_BLOCK = block_ptr;
        block_ptr->next = NULL;
    }
    else
    {
       ((tlsf_adaptation_structure_t*)(target_td->FIRST_OWNED_BLOCK))->prev = block_ptr;
       block_ptr->next = target_td->FIRST_OWNED_BLOCK;
       target_td->FIRST_OWNED_BLOCK = block_ptr;     
    }
    
    TLSF_MEM_PROT_EXIT();
    
#else
    (void)memory_ptr;
    (void)target_td;
#endif

} /* Endbody */

/*!
 * \private
 *
 * \brief This routine transfers the ownership of a block of memory from an owner
 * task to another task.
 *
 * \param[in] memory_ptr The memory block whose ownership is to be transferred.
 * \param[in] source_td  The source (owner) TD.
 * \param[in] target_td  The target (new owner) TD.
 *
 * \return MQX_OK
 */
_mqx_uint _tlsf_transfer_td_internal
(
    void         *memory_ptr,
    TD_STRUCT_PTR source_td,
    TD_STRUCT_PTR target_td
)
{
#if MQX_ALLOCATOR_GARBAGE_COLLECTING
  /* Body */
    tlsf_adaptation_structure_t* block_ptr;
    
    block_ptr = (tlsf_adaptation_structure_t*)tlsf_get_adaptation_from_ptr(memory_ptr);
    
    TLSF_MEM_PROT_ENTER();
      
    /* remove this block from the source_td's linked list */
    if(block_ptr->prev == NULL)
    {
       ((TD_STRUCT_PTR)(block_ptr->owner))->FIRST_OWNED_BLOCK = block_ptr->next;
       if(block_ptr->next != NULL)
       {
          ((tlsf_adaptation_structure_t*)(block_ptr->next))->prev = NULL;
       }
       
    }
    else
    {
       ((tlsf_adaptation_structure_t*)(block_ptr->prev))->next = block_ptr->next;
       if(block_ptr->next != NULL)
       {
          ((tlsf_adaptation_structure_t*)(block_ptr->next))->prev = block_ptr->prev;
       }
    }

    /* add this block to the head of target_td's linked list */
    block_ptr->owner = (void*)target_td;
    block_ptr->prev = NULL;
    if(target_td->FIRST_OWNED_BLOCK == NULL)
    {
        target_td->FIRST_OWNED_BLOCK = block_ptr;
        block_ptr->next = NULL;
    }
    else
    {
       ((tlsf_adaptation_structure_t*)(target_td->FIRST_OWNED_BLOCK))->prev = block_ptr;
       block_ptr->next = target_td->FIRST_OWNED_BLOCK;
       target_td->FIRST_OWNED_BLOCK = block_ptr;     
    }
    
    TLSF_MEM_PROT_EXIT();
    
#else
    (void)memory_ptr;
    (void)source_td;
    (void)target_td;
#endif
    return (MQX_OK);
} /* Endbody */

/*!
 * \brief Allocates a system(zero-filled) block of TLSF memory block from
 * the specified memory pool.
 *
 * See Description of _tlsf_alloc_from() function.
 *
 * \param[in] pool_id        TLSF memory pool from which to allocate the
 * TLSF memory block (from _tlsf_create_pool()).
 * \param[in] size           Number of single-addressable units to allocate.
 *
 * \return Pointer to the TLSF memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 *
 */
void *_tlsf_alloc_system_zero_from
(
    tlsf_t pool_id,
    _mem_size      size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_tlsf_alloc_system_zero_from, size);
    
    TLSF_MEM_PROT_ENTER();

    result = _tlsf_alloc_internal(size, SYSTEM_TD_PTR(kernel_data), pool_id, TRUE);

    TLSF_MEM_PROT_EXIT();
    
    _KLOGX2(KLOG_tlsf_alloc_system_zero_from, result);
    return (result);

} /* Endbody */

/*!
 * \brief Allocates a system (zero-filled) block of TLSF memory block from
 * the default memory pool.
 *
 * See Description of _tlsf_alloc() function.
 *
 * \param[in] size Number of single-addressable units to allocate.
 *
 * \return Pointer to the TLSF memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 *
 */
void *_tlsf_alloc_system_zero
(
    _mem_size size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_tlsf_alloc_system_zero, size);

    TLSF_MEM_PROT_ENTER();
    
    result = _tlsf_alloc_internal(
        size, SYSTEM_TD_PTR(kernel_data), (tlsf_t) kernel_data->KERNEL_TLSF_POOL, TRUE
    );
    
    TLSF_MEM_PROT_EXIT();

    _KLOGX2(KLOG_tlsf_alloc_system_zero, result);
    return (result);

} /* Endbody */




#if MQX_USE_UNCACHED_MEM && PSP_HAS_DATA_CACHE

/*!
 * \brief Allocates a block of memory.
 *
 * \param[in] size Size of the memory block.
 *
 * \return Pointer to the memory block (Success.)
 * \return NULL (Failure: see task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following
 * task error codes:
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)     
 */ 
void *_tlsf_alloc_uncached
(
    _mem_size size
)
{
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void   *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_tlsf_alloc_uncached, size);

    result = _tlsf_alloc_from(kernel_data->UNCACHED_POOL, size);

    _KLOGX2(KLOG_tlsf_alloc_uncached, result);

    return result;
}

/*!
 * \brief Allocates an aligned block of memory.
 *
 * \param[in] size  Size of the memory block.
 * \param[in] align Alignment of the memory block.
 *
 * \return Pointer to the memory block (Success.)
 * \return NULL (Failure: see task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following
 * task error codes:
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)  
 */ 
void *_tlsf_alloc_align_uncached
(
    _mem_size size,
    _mem_size align
)
{
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void   *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_tlsf_alloc_align_uncached, size);
    
    TLSF_MEM_PROT_ENTER();
 
    result = _tlsf_alloc_align_from(kernel_data->UNCACHED_POOL, size, align);

     TLSF_MEM_PROT_EXIT();
    
    _KLOGX2(KLOG_tlsf_alloc_align_uncached, result);

    return result;
}


/*!
 * \brief Allocates uncached memory that is available system wide.
 *
 * \param[in] size Size of the memory block.
 *
 * \return Pointer to the memory block (Success.)
 * \return NULL (Failure: see task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following
 * task error codes:
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 */
void *_tlsf_alloc_system_uncached
(
    _mem_size size
)
{    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_tlsf_alloc_system_uncached, size);

    TLSF_MEM_PROT_ENTER();
    
    result = _tlsf_alloc_internal(size, SYSTEM_TD_PTR(kernel_data), kernel_data->UNCACHED_POOL, FALSE);

    TLSF_MEM_PROT_EXIT();
    
    _KLOGX2(KLOG_tlsf_alloc_system_uncached, result);
    return (result);
}


/*!
 * \brief Allocates uncached memory from the system, zeroed.
 *
 * \param[in] size Size of the memory block.
 *
 * \return Pointer to the memory block (Success.)
 * \return NULL (Failure: see task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following
 * task error codes:
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 */
void *_tlsf_alloc_system_zero_uncached
(
    _mem_size size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_tlsf_alloc_system_zero_uncached, size);

    TLSF_MEM_PROT_ENTER();
    
    result = _tlsf_alloc_internal(size, SYSTEM_TD_PTR(kernel_data), kernel_data->UNCACHED_POOL, TRUE);

    TLSF_MEM_PROT_EXIT();
    
    _KLOGX2(KLOG_tlsf_alloc_system_zero_uncached, result);
    return (result);

}


#endif /* MQX_USE_UNCACHED_MEM && PSP_HAS_DATA_CACHE */


/*!
 * \brief Resize the memory block that was previously allocated.
 *
 * This function can allocate new memory block with a specific size, or 
 * resize given memory block. When create new memory block, then copy
 * data from memory block pointed to by the pointer to new memory block.
 * Then memory block, that was previosly allocated, is freed.
 * 
 * When given pointer is NULL, then function has same behavior as malloc(size).
 * When size is 0, then function has same behavior as malloc(0) or free(), and given
 * memory block is freed.
 *
 * See Description of _tlsf_alloc() function.
 *
 * \param[in] mem_ptr Pointer to given memory block.
 * \param[in] size Size for the new memory block.
 *
 * \return Pointer to the TLSF memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_NOT_RESOURCE_OWNER
 *
 */

void *_tlsf_realloc
(
    void* mem_ptr,
    _mem_size requested_size
)
{ /* Body */
    tlsf_adaptation_structure_t* adapt;
    tlsf_adaptation_structure_t* newadapt;
    void* retval;
    tlsf_t pool_id;
    TD_STRUCT_PTR td_ptr;

#if (MQX_ALLOCATOR_GARBAGE_COLLECTING && MQX_CHECK_ERRORS || MQX_KERNEL_LOGGING || defined(_DEBUG))   
    KERNEL_DATA_STRUCT_PTR kernel_data;
    _GET_KERNEL_DATA(kernel_data);
#endif
#ifdef MQX_CHECK_ERRORS
#if !MQX_ALLOCATOR_ALLOW_IN_ISR  
    if (kernel_data->IN_ISR)
    {
#if defined(_DEBUG)
        assert(0 && "If you want to use allocator in ISR, set MQX_ALLOCATOR_ALLOW_IN_ISR to 1");
#endif
        _KLOGX2(_tlsf_realloc, MQX_CANNOT_CALL_FUNCTION_FROM_ISR);
        return NULL;
    }
#endif
		
    if(((size_t)requested_size + sizeof(tlsf_adaptation_structure_t)) > tlsf_block_size_max())
    {
        _task_set_error(MQX_TLSF_TOO_LARGE_BLOCK);
        _KLOGX2(_tlsf_realloc, MQX_TLSF_TOO_LARGE_BLOCK);
        return NULL;
    }
#endif
    
      
    _KLOGE2(KLOG_tlsf_realloc, mem_ptr);

    /* Zero-size requests are treated as free. */
    if (mem_ptr && requested_size == 0)
    {
        _tlsf_free(mem_ptr);
        _KLOGE2(KLOG_tlsf_realloc, NULL);
        return NULL;
    }
    /* Requests with NULL pointers are treated as _tlsf_alloc_system. */
    else if (!mem_ptr)
    {
        retval = _tlsf_alloc_system(requested_size);
        _KLOGE2(KLOG_tlsf_realloc, retval);
        return retval;
    }
    else
    {

        adapt = (tlsf_adaptation_structure_t*)tlsf_get_adaptation_from_ptr(mem_ptr);
        pool_id = adapt->pool;
#if MQX_CHECK_VALIDITY
    if (_tlsf_pool_is_valid(pool_id) != MQX_OK)
    {
        _task_set_error(MQX_TLSF_POOL_INVALID);
        return NULL;
    } /* Endif */
#endif

#if MQX_ALLOCATOR_GARBAGE_COLLECTING
        td_ptr = (TD_STRUCT_PTR)adapt->owner;

        TLSF_MEM_PROT_ENTER();
        
    #if MQX_CHECK_ERRORS
        /* Verify the passed in parameter */
        if ((adapt->owner != (void*)kernel_data->ACTIVE_PTR)
                        && (adapt->owner != (void*)SYSTEM_TD_PTR(kernel_data)))
        {
            TLSF_MEM_PROT_EXIT();
            _task_set_error(MQX_NOT_RESOURCE_OWNER);
            _KLOGE2(KLOG_tlsf_realloc, MQX_NOT_RESOURCE_OWNER);
            return NULL;
        } /* Endif */
    #endif
        
        /* remove this block from the td's linked list */
        if(adapt->prev == NULL)
        {
           ((TD_STRUCT_PTR)(adapt->owner))->FIRST_OWNED_BLOCK = adapt->next;
           if(adapt->next != NULL)
           {
              ((tlsf_adaptation_structure_t*)(adapt->next))->prev = NULL;
           }
           
        }
        else
        {
           ((tlsf_adaptation_structure_t*)(adapt->prev))->next = adapt->next;
           if(adapt->next != NULL)
           {
              ((tlsf_adaptation_structure_t*)(adapt->next))->prev = adapt->prev;
           }
        }
#else
        TLSF_MEM_PROT_ENTER();
#endif
        
#if PSP_HAS_DATA_CACHE
        newadapt = (tlsf_adaptation_structure_t*)tlsf_realloc_aligned(pool_id, adapt, requested_size  + sizeof(tlsf_adaptation_structure_t), (PSP_MEMORY_ALIGNMENT + 1), sizeof(tlsf_adaptation_structure_t));
#else
        newadapt = (tlsf_adaptation_structure_t*)tlsf_realloc(pool_id, adapt, requested_size  + sizeof(tlsf_adaptation_structure_t));
#endif
        

        if (newadapt == NULL)
        {
#if MQX_ALLOCATOR_GARBAGE_COLLECTING
            /* Add block back to the block list of the owner */
            adapt->prev = NULL;
            if(td_ptr->FIRST_OWNED_BLOCK == NULL)
            {
                td_ptr->FIRST_OWNED_BLOCK = (void*)adapt;
                adapt->next = NULL;
            }
            else
            {
               ((tlsf_adaptation_structure_t*)(td_ptr->FIRST_OWNED_BLOCK))->prev = adapt;
               adapt->next = td_ptr->FIRST_OWNED_BLOCK;
               td_ptr->FIRST_OWNED_BLOCK = (void*)adapt;     
            }
#endif
            TLSF_MEM_PROT_EXIT();
            _task_set_error(MQX_OUT_OF_MEMORY);
            return NULL;
        }
       
#if MQX_ALLOW_TYPED_MEMORY
        newadapt->U.S.TYPE = 0;
#endif

        newadapt->pool = pool_id;
        
#if MQX_ALLOCATOR_GARBAGE_COLLECTING
        /* Add block back to the block list of the owner */
        newadapt->owner = (void*)td_ptr;
        newadapt->prev = NULL;
        if(td_ptr->FIRST_OWNED_BLOCK == NULL)
        {
            td_ptr->FIRST_OWNED_BLOCK = (void*)newadapt;
            newadapt->next = NULL;
        }
        else
        {
           ((tlsf_adaptation_structure_t*)(td_ptr->FIRST_OWNED_BLOCK))->prev = newadapt;
           newadapt->next = td_ptr->FIRST_OWNED_BLOCK;
           td_ptr->FIRST_OWNED_BLOCK = (void*)newadapt;     
        }
#endif

        TLSF_MEM_PROT_EXIT();
        
        retval = (void*)((unsigned char*)newadapt + sizeof(tlsf_adaptation_structure_t));

        return retval;
    }

} /* Endbody */


/*!
 * \brief Tests the memory in the memory pool.
 *
 * _tlsf_test_pool() tests the integrity of a TLSF allocator instance
 *
 * \param[in] pool_id The pool to check. (allocator instance)
 *
 * \return MQX_OK (No errors found.)
 * \return MQX_TLSF_POOL_INVALID (A memory pool pointer is not correct.)
 *
 */
_mqx_uint _tlsf_test_pool
(
    void* pool_id
)
{ /* Body */
   
    TLSF_MEM_PROT_ENTER();
    if(tlsf_check(pool_id)!=0)
    {
        TLSF_MEM_PROT_EXIT();
        return MQX_TLSF_POOL_INVALID;
    }
    else
    {
        TLSF_MEM_PROT_EXIT();
        return MQX_OK;
    }
} /* Endbody */

/*!
 * \brief Tests the memory pool header validity
 *
 *  _tlsf_pool_is_valid() tests the validity of the header
 *  of a pool, this function is safe and can be used to check,
 *  if the allocator has been initialized.
 *
 * \param[in] pool_id The pool to check. (allocator instance)
 *
 * \return MQX_OK (No errors found.)
 * \return MQX_TLSF_POOL_INVALID (A memory pool pointer is not correct.)
 *
 */
_mqx_uint _tlsf_pool_is_valid
(
    void* pool_id
)
{
  if(pool_id == NULL)
  {
    return MQX_TLSF_POOL_INVALID;
  }
  
  if(((control_t*)pool_id)->block_null.size != TLSF_CHECK_VALUE)  
  {
    return MQX_TLSF_POOL_INVALID;
  }
  else
  {
    return MQX_OK;
  }
}

/*!
 * \brief Tests memory that the memory component uses to allocate memory from
 * the default memory pool.
 *
 * The function checks the checksums of all memory-block headers. If the
 * function detects an error, _mem_get_error() gets the block in error.
 * \n The function can be called by only one task at a time because it keeps
 * state-in-progress variables that MQX controls. This mechanism lets other
 * tasks allocate and free memory while _mem_test() runs.
 *
 * \return MQX_OK (No errors found.)
 * \return MQX_TLSF_POOL_INVALID (A memory pool pointer is not correct.)
 *
 * \warning Can be called by only one task at a time (see Description).
 * \warning Disables and enables interrupts.
 */
_mqx_uint _tlsf_test
(
    void
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    _GET_KERNEL_DATA(kernel_data);

    return _mem_test_pool(kernel_data->KERNEL_TLSF_POOL);

} /* Endbody */




#endif /* MQX_USE_TLSF_ALLOCATOR */
