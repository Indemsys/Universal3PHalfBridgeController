
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
*
*END************************************************************************/

#define __MEMORY_MANAGER_COMPILE__
#include "mqx_inc.h"
#include "mem_prv.h"

#if !MQX_USE_TLSF_ALLOCATOR /* Cannot use full-weight allocators in parallel with tlsf */
#if MQX_USE_MEM 

#if MQX_USE_UNCACHED_MEM && PSP_HAS_DATA_CACHE
extern unsigned char __UNCACHED_DATA_START[];
extern unsigned char __UNCACHED_DATA_END[];
#endif /* MQX_USE_UNCACHED_MEM && PSP_HAS_DATA_CACHE */

/*!
 * \brief Allocates a block of memory.
 *
 * The functions allocate at least size single-addressable units; the actual
 * number might be greater. The start address of the block is aligned so that
 * tasks can use the returned pointer as a pointer to any data type without
 * causing an error.
 * \n Tasks cannot use memory blocks as messages. Tasks must use _msg_alloc() or
 * _msg_alloc_system() to allocate messages.
 * \n Only the task that allocates a memory block with one of the following
 * functions can free the memory block:
 * \li _mem_alloc()
 * \li _mem_alloc_from()
 * \li _mem_alloc_zero()
 * \li _mem_alloc_zero_from()
 * \li _mem_alloc_align()
 * \li _mem_alloc_align_from()
 * \li _mem_alloc_at()
 *
 * \n Any task can free a memory block that is allocated with one of the following
 * functions:
 * \li _mem_alloc_system()
 * \li _mem_alloc_system_from()
 * \li _mem_alloc_system_zero()
 * \li _mem_alloc_system_zero_from()
 * \li _mem_alloc_system_align()
 * \li _mem_alloc_system_align_from()
 *
 * \n Function types:
 * <table>
 *  <tr>
 *    <td></td>
 *    <td><b>Allocate this type of memory block:</b></td>
 *    <td><b>From:</b></td>
 *  </tr>
 *  <tr>
 *    <td><b>_mem_alloc()</b></td>
 *    <td>Private</td>
 *    <td>Default memory pool</td>
 *  </tr>
 *  <tr>
 *    <td><b>_mem_alloc_from()</b></td>
 *    <td>Private</td>
 *    <td>Specified memory pool</td>
 *  </tr>
 *  <tr>
 *    <td><b>_mem_alloc_system()</b></td>
 *    <td>System</td>
 *    <td>Default memory pool</td>
 *  </tr>
 *  <tr>
 *    <td><b>_mem_alloc_system_from()</b></td>
 *    <td>System</td>
 *    <td>Specified memory pool</td>
 *  </tr>
 *  <tr>
 *    <td><b>_mem_alloc_system_zero()</b></td>
 *    <td>System (zero-filled)</td>
 *    <td>Default memory pool</td>
 *  </tr>
 *  <tr>
 *    <td><b>_mem_alloc_system_zero_from()</b></td>
 *    <td>System (zero-filled)</td>
 *    <td>Specified memory pool</td>
 *  </tr>
 *  <tr>
 *    <td><b>_mem_alloc_zero()</b></td>
 *    <td>Private (zero-filled)</td>
 *    <td>Default memory pool</td>
 *  </tr>
 *  <tr>
 *    <td><b>_mem_alloc_zero_from()</b></td>
 *    <td>Private (zero-filled)</td>
 *    <td>Specified memory pool</td>
 *  </tr>
 *  <tr>
 *    <td><b>_mem_alloc_system_align()</b></td>
 *    <td>System (aligned)</td>
 *    <td>Default memory pool</td>
 *  </tr>
 *  <tr>
 *    <td><b>_mem_alloc_system_align_from()</b></td>
 *    <td>System (aligned)</td>
 *    <td>Specified memory pool</td>
 *  </tr>
 *  <tr>
 *    <td><b>_mem_alloc_align()</b></td>
 *    <td>Private (aligned)</td>
 *    <td>Default memory pool</td>
 *  </tr>
 *  <tr>
 *    <td><b>_mem_alloc_align_from()</b></td>
 *    <td>Private (aligned)</td>
 *    <td>Specified memory pool</td>
 *  </tr>
 *  <tr>
 *    <td><b>_mem_alloc_at()</b></td>
 *    <td>Private (start address defined)</td>
 *    <td>Default memory pool</td>
 *  </tr>
 * </table>
 *
 * \param[in] size Size of the memory block.
 *
 * \return Pointer to the memory block (Success.)
 * \return NULL (Failure: see task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following
 * task error codes:
 * \li MQX_CORRUPT_STORAGE_POOL_FREE_LIST (Memory pool freelist is corrupted.)
 * \li MQX_INVALID_CHECKSUM (Checksum of the current memory block header is
 * incorrect.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_INVALID_CONFIGURATION (User area not aligned on a cache line 
 * boundary.)
 * 
 * \see _mem_alloc_from()
 * \see _mem_alloc_system()
 * \see _mem_alloc_system_from()
 * \see _mem_alloc_system_zero()
 * \see _mem_alloc_system_zero_from()
 * \see _mem_alloc_system_align() 
 * \see _mem_alloc_system_align_from() 
 * \see _mem_alloc_zero()
 * \see _mem_alloc_zero_from()
 * \see _mem_alloc_align()
 * \see _mem_alloc_align_from()
 * \see _mem_alloc_at()
 * \see _mem_realloc()
 * \see _mem_create_pool
 * \see _mem_free
 * \see _mem_get_highwater
 * \see _mem_get_highwater_pool
 * \see _mem_get_size
 * \see _mem_transfer
 * \see _mem_free_part
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error
 */

void *_mem_alloc
(
    _mem_size size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    _mqx_uint error;
    void   *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_mem_alloc, size);

    _INT_DISABLE();
    result = _mem_alloc_internal(size, kernel_data->ACTIVE_PTR, (MEMPOOL_STRUCT_PTR) kernel_data->KD_POOL, &error);

    /* update the memory allocation pointer in case a lower priority
     * task was preempted inside _mem_alloc_internal
     */
    kernel_data->KD_POOL->POOL_ALLOC_CURRENT_BLOCK = kernel_data->KD_POOL->POOL_FREE_LIST_PTR;

    _INT_ENABLE();

#if MQX_CHECK_ERRORS
    if (error != MQX_OK) {
        _task_set_error(error);
    } /* Endif */
#endif

    _KLOGX3(KLOG_mem_alloc, result, kernel_data->KD_POOL->POOL_BLOCK_IN_ERROR);
    return (result);

} /* Endbody */

/*!
 * \brief Allocates a block of memory.
 *
 * See Description of _mem_alloc() function.
 *
 * \param[in] size  Size of the memory block.
 * \param[in] align Alignment of the memory block.
 *
 * \return Pointer to the memory block (Success.)
 * \return NULL (Failure: see task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following
 * task error codes:
 * \li MQX_CORRUPT_STORAGE_POOL_FREE_LIST (Memory pool freelist is corrupted.)
 * \li MQX_INVALID_CHECKSUM (Checksum of the current memory block header is
 * incorrect.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_INVALID_CONFIGURATION (User area not aligned on a cache line 
 * boundary.)
 * \li MQX_INVALID_PARAMETER (Requested alignment is not power of 2.)
 *
 * \see _mem_alloc()
 * \see _mem_alloc_from()
 * \see _mem_alloc_system()
 * \see _mem_alloc_system_from()
 * \see _mem_alloc_system_zero()
 * \see _mem_alloc_system_zero_from()
 * \see _mem_alloc_system_align() 
 * \see _mem_alloc_system_align_from() 
 * \see _mem_alloc_zero()
 * \see _mem_alloc_zero_from()
 * \see _mem_alloc_align_from()
 * \see _mem_alloc_at()
 * \see _mem_realloc()
 * \see _mem_create_pool
 * \see _mem_free
 * \see _mem_get_highwater
 * \see _mem_get_highwater_pool
 * \see _mem_get_size
 * \see _mem_transfer
 * \see _mem_free_part
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error
 */
void *_mem_alloc_align
(
    _mem_size size,
    _mem_size align
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    _mqx_uint error;
    void   *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_mem_alloc, size);

    _INT_DISABLE();
    result = _mem_alloc_internal_align(size, align, kernel_data->ACTIVE_PTR, (MEMPOOL_STRUCT_PTR)
                    kernel_data->KD_POOL, &error);

    /* update the memory allocation pointer in case a lower priority
     * task was preempted inside _mem_alloc_internal
     */
    kernel_data->KD_POOL->POOL_ALLOC_CURRENT_BLOCK = kernel_data->KD_POOL->POOL_FREE_LIST_PTR;

    _INT_ENABLE();

#if MQX_CHECK_ERRORS
    if (error != MQX_OK) {
        _task_set_error(error);
    } /* Endif */
#endif

    _KLOGX3(KLOG_mem_alloc_align, result, kernel_data->KD_POOL->POOL_BLOCK_IN_ERROR);
    return (result);
}

/*!
 * \brief Allocates a block of memory.
 *
 * See Description of _mem_alloc() function.
 *
 * \param[in] size Size of the memory block.
 * \param[in] addr Start address of the memory block.
 *
 * \return Pointer to the memory block (Success.)
 * \return NULL (Failure: see task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following
 * task error codes:
 * \li MQX_CORRUPT_STORAGE_POOL_FREE_LIST (Memory pool freelist is corrupted.)
 * \li MQX_INVALID_CHECKSUM (Checksum of the current memory block header is
 * incorrect.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_INVALID_CONFIGURATION (User area not aligned on a cache line 
 * boundary.)
 * 
 * \see _mem_alloc()
 * \see _mem_alloc_from()
 * \see _mem_alloc_system()
 * \see _mem_alloc_system_from()
 * \see _mem_alloc_system_zero()
 * \see _mem_alloc_system_zero_from()
 * \see _mem_alloc_system_align() 
 * \see _mem_alloc_system_align_from() 
 * \see _mem_alloc_zero()
 * \see _mem_alloc_zero_from()
 * \see _mem_alloc_align() 
 * \see _mem_alloc_align_from()
 * \see _mem_realloc()
 * \see _mem_create_pool
 * \see _mem_free
 * \see _mem_get_highwater
 * \see _mem_get_highwater_pool
 * \see _mem_get_size
 * \see _mem_transfer
 * \see _mem_free_part
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error                 
 */
void *_mem_alloc_at
(
    _mem_size size,
    void   *addr
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    _mqx_uint error;
    void   *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_mem_alloc, size);

    _INT_DISABLE();
    result = _mem_alloc_at_internal(size, addr, kernel_data->ACTIVE_PTR, (MEMPOOL_STRUCT_PTR) kernel_data->KD_POOL,
                    &error);

    /* update the memory allocation pointer in case a lower priority
     * task was preempted inside _mem_alloc_internal
     */
    kernel_data->KD_POOL->POOL_ALLOC_CURRENT_BLOCK = kernel_data->KD_POOL->POOL_FREE_LIST_PTR;

    _INT_ENABLE();

#if MQX_CHECK_ERRORS
    if (error != MQX_OK) {
        _task_set_error(error);
    }
#endif

    _KLOGX3(KLOG_mem_alloc_at, result, kernel_data->KD_POOL->POOL_BLOCK_IN_ERROR);
    return (result);
}

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
 * \li MQX_CORRUPT_STORAGE_POOL_FREE_LIST (Memory pool freelist is corrupted.)
 * \li MQX_INVALID_CHECKSUM (Checksum of the current memory block header is 
 * incorrect.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)     
 * \li MQX_INVALID_CONFIGURATION (User area not aligned on a cache line 
 * boundary.)
 */ 
void *_mem_alloc_uncached
(
    _mem_size size
)
{
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void   *result;

    _GET_KERNEL_DATA(kernel_data);
    //_KLOGE2(KLOG_mem_alloc_uncached, req_size);

    result = _mem_alloc_from(kernel_data->UNCACHED_POOL, size);

    //_KLOGX3(KLOG_mem_alloc_uncached, result, kernel_data->UNCACHED_POOL.POOL_BLOCK_IN_ERROR);

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
 * \li MQX_CORRUPT_STORAGE_POOL_FREE_LIST (Memory pool freelist is corrupted.)
 * \li MQX_INVALID_CHECKSUM (Checksum of the current memory block header is 
 * incorrect.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)  
 * \li MQX_INVALID_CONFIGURATION (User area not aligned on a cache line 
 * boundary.)
 */ 
void *_mem_alloc_align_uncached
(
    _mem_size size,
    _mem_size align
)
{
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void   *result;

    _GET_KERNEL_DATA(kernel_data);
    //_KLOGE2(KLOG_mem_alloc_uncached, size);

    result = _mem_alloc_align_from(kernel_data->UNCACHED_POOL, size, align);

    //_KLOGX3(KLOG_mem_alloc_uncached, result, kernel_data->UNCACHED_POOL.POOL_BLOCK_IN_ERROR);

    return result;
}

#endif /* MQX_USE_UNCACHED_MEM && PSP_HAS_DATA_CACHE */

/*!
 * \brief Allocates a block of memory.
 *
 * See Description of _mem_alloc() function.
 *
 * \param[in] pool_id Pool from which to allocate the memory block (from
 * _mem_create_pool()).
 * \param[in] size    Number of single-addressable units to allocate.
 *
 * \return Pointer to the memory block (Success.)
 * \return NULL (Failure: see task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following
 * task error codes:
 * \li MQX_CORRUPT_STORAGE_POOL_FREE_LIST (Memory pool freelist is corrupted.)
 * \li MQX_INVALID_CHECKSUM (Checksum of the current memory block header is
 * incorrect.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_INVALID_CONFIGURATION (User area not aligned on a cache line 
 * boundary.)
 *
 * \see _mem_alloc()
 * \see _mem_alloc_system()
 * \see _mem_alloc_system_from()
 * \see _mem_alloc_system_zero()
 * \see _mem_alloc_system_zero_from()
 * \see _mem_alloc_system_align() 
 * \see _mem_alloc_system_align_from() 
 * \see _mem_alloc_zero()
 * \see _mem_alloc_zero_from()
 * \see _mem_alloc_align()
 * \see _mem_alloc_align_from()
 * \see _mem_alloc_at()
 * \see _mem_realloc()
 * \see _mem_create_pool
 * \see _mem_free
 * \see _mem_get_highwater
 * \see _mem_get_highwater_pool
 * \see _mem_get_size
 * \see _mem_transfer
 * \see _mem_free_part
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error
 */
void *_mem_alloc_from
(
    _mem_pool_id pool_id,
    _mem_size size
)
{
    KERNEL_DATA_STRUCT_PTR kernel_data;
    MEMPOOL_STRUCT_PTR mem_pool_ptr;
    _mqx_uint error;
    void   *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_mem_alloc_from, size);

    mem_pool_ptr = (MEMPOOL_STRUCT_PTR) pool_id;

    _INT_DISABLE();

    result = _mem_alloc_internal(size, kernel_data->ACTIVE_PTR, mem_pool_ptr, &error);

#if MQX_CHECK_ERRORS
    if (error != MQX_OK) {
        _task_set_error(error);
    } /* Endif */
#endif

    /* update the memory allocation pointer in case a lower priority
     * task was preempted inside _mem_alloc_internal
     */
    mem_pool_ptr->POOL_ALLOC_CURRENT_BLOCK = mem_pool_ptr->POOL_FREE_LIST_PTR;

    _INT_ENABLE();

    _KLOGX3(KLOG_mem_alloc_from, result, mem_pool_ptr->POOL_BLOCK_IN_ERROR);
    return (result);

}

/*!
 * \brief Allocates a block of memory.
 *
 * See Description of _mem_alloc() function.
 *
 * \param[in] pool_id Pool from which to allocate the memory block (from
 * _mem_create_pool()).
 * \param[in] size    Number of single-addressable units to allocate.
 * \param[in] align   Alignment of the memory block.
 *
 * \return Pointer to the memory block (Success.)
 * \return NULL (Failure: see task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following
 * task error codes:
 * \li MQX_CORRUPT_STORAGE_POOL_FREE_LIST (Memory pool freelist is corrupted.)
 * \li MQX_INVALID_CHECKSUM (Checksum of the current memory block header is
 * incorrect.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_INVALID_CONFIGURATION (User area not aligned on a cache line 
 * boundary.)
 * \li MQX_INVALID_PARAMETER (Requested alignment is not power of 2.)
 *
 * \see _mem_alloc()
 * \see _mem_alloc_from()
 * \see _mem_alloc_system()
 * \see _mem_alloc_system_from()
 * \see _mem_alloc_system_zero()
 * \see _mem_alloc_system_zero_from()
 * \see _mem_alloc_system_align() 
 * \see _mem_alloc_system_align_from() 
 * \see _mem_alloc_zero()
 * \see _mem_alloc_zero_from()
 * \see _mem_alloc_align()
 * \see _mem_alloc_at()
 * \see _mem_realloc()
 * \see _mem_create_pool
 * \see _mem_free
 * \see _mem_get_highwater
 * \see _mem_get_highwater_pool
 * \see _mem_get_size
 * \see _mem_transfer
 * \see _mem_free_part
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error
 */
void *_mem_alloc_align_from
(
    _mem_pool_id pool_id,
    _mem_size size,
    _mem_size align
)
{
    KERNEL_DATA_STRUCT_PTR kernel_data;
    MEMPOOL_STRUCT_PTR mem_pool_ptr;
    _mqx_uint error;
    void   *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_mem_alloc_align_from, size);

    mem_pool_ptr = (MEMPOOL_STRUCT_PTR) pool_id;

    _INT_DISABLE();

    result = _mem_alloc_internal_align(size, align, kernel_data->ACTIVE_PTR, mem_pool_ptr, &error);

#if MQX_CHECK_ERRORS
    if (error != MQX_OK) {
        _task_set_error(error);
    } /* Endif */
#endif

    /* update the memory allocation pointer in case a lower priority
     * task was preempted inside _mem_alloc_internal
     */
    mem_pool_ptr->POOL_ALLOC_CURRENT_BLOCK = mem_pool_ptr->POOL_FREE_LIST_PTR;

    _INT_ENABLE();

    _KLOGX3(KLOG_mem_alloc_align_from, result, mem_pool_ptr->POOL_BLOCK_IN_ERROR);
    return (result);

}

/*!
 * \brief Allocates a block of memory.
 *
 * See Description of _mem_alloc() function.
 *
 * \param[in] size Number of single-addressable units to allocate.
 *
 * \return Pointer to the memory block (Success.)
 * \return NULL (Failure: see task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following
 * task error codes:
 * \li MQX_CORRUPT_STORAGE_POOL_FREE_LIST (Memory pool freelist is corrupted.)
 * \li MQX_INVALID_CHECKSUM (Checksum of the current memory block header is
 * incorrect.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_INVALID_CONFIGURATION (User area not aligned on a cache line 
 * boundary.)
 *
 * \see _mem_alloc()
 * \see _mem_alloc_from()
 * \see _mem_alloc_system()
 * \see _mem_alloc_system_from()
 * \see _mem_alloc_system_zero()
 * \see _mem_alloc_system_zero_from()
 * \see _mem_alloc_system_align() 
 * \see _mem_alloc_system_align_from() 
 * \see _mem_alloc_zero_from()
 * \see _mem_alloc_align()
 * \see _mem_alloc_align_from()
 * \see _mem_alloc_at()
 * \see _mem_realloc()
 * \see _mem_create_pool
 * \see _mem_free
 * \see _mem_get_highwater
 * \see _mem_get_highwater_pool
 * \see _mem_get_size
 * \see _mem_transfer
 * \see _mem_free_part
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error
 */
void *_mem_alloc_zero
(
    _mem_size size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    _mqx_uint error;
    void   *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_mem_alloc_zero, size);

    _INT_DISABLE();

    result = _mem_alloc_internal(size, kernel_data->ACTIVE_PTR, (MEMPOOL_STRUCT_PTR) kernel_data->KD_POOL, &error);

    /* update the memory allocation pointer in case a lower priority
     * task was preempted inside _mem_alloc_internal
     */
    kernel_data->KD_POOL->POOL_ALLOC_CURRENT_BLOCK = kernel_data->KD_POOL->POOL_FREE_LIST_PTR;

    _INT_ENABLE();

#if MQX_CHECK_ERRORS
    if (error != MQX_OK) {
        _task_set_error(error);
    } /* Endif */
#endif

    if (result != NULL) {
        _mem_zero(result, size);
    } /* Endif */

    _KLOGX3(KLOG_mem_alloc_zero, result,
                    kernel_data->KD_POOL->POOL_BLOCK_IN_ERROR);
    return (result);

} /* Endbody */

/*!
 * \brief Allocates a block of memory.
 *
 * See Description of _mem_alloc() function.
 *
 * \param[in] pool_id Pool from which to allocate the memory block (from
 * _mem_create_pool()).
 * \param[in] size    Number of single-addressable units to allocate.
 *
 * \return Pointer to the memory block (Success.)
 * \return NULL (Failure: see task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following
 * task error codes:
 * \li MQX_CORRUPT_STORAGE_POOL_FREE_LIST (Memory pool freelist is corrupted.)
 * \li MQX_INVALID_CHECKSUM (Checksum of the current memory block header is
 * incorrect.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_INVALID_CONFIGURATION (User area not aligned on a cache line 
 * boundary.)
 *
 * \see _mem_alloc()
 * \see _mem_alloc_from()
 * \see _mem_alloc_system()
 * \see _mem_alloc_system_from()
 * \see _mem_alloc_system_zero()
 * \see _mem_alloc_system_zero_from()
 * \see _mem_alloc_system_align() 
 * \see _mem_alloc_system_align_from() 
 * \see _mem_alloc_zero()
 * \see _mem_alloc_align()
 * \see _mem_alloc_align_from()
 * \see _mem_alloc_at()
 * \see _mem_realloc()
 * \see _mem_create_pool
 * \see _mem_free
 * \see _mem_get_highwater
 * \see _mem_get_highwater_pool
 * \see _mem_get_size
 * \see _mem_transfer
 * \see _mem_free_part
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error
 */
void *_mem_alloc_zero_from
(
    void   *pool_id,
    _mem_size size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    MEMPOOL_STRUCT_PTR mem_pool_ptr;
    _mqx_uint error;
    void   *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_mem_alloc_zero, size);

    mem_pool_ptr = (MEMPOOL_STRUCT_PTR) pool_id;

    _INT_DISABLE();

    result = _mem_alloc_internal(size, kernel_data->ACTIVE_PTR, mem_pool_ptr, &error);

    /*
     * update the memory allocation pointer in case a lower priority
     * task was preempted inside _mem_alloc_internal
     */
    mem_pool_ptr->POOL_ALLOC_CURRENT_BLOCK = mem_pool_ptr->POOL_FREE_LIST_PTR;

    _INT_ENABLE();

#if MQX_CHECK_ERRORS
    if (error != MQX_OK) {
        _task_set_error(error);
    } /* Endif */
#endif

    if (result != NULL) {
        _mem_zero(result, size);
    } /* Endif */

    _KLOGX3(KLOG_mem_alloc_zero, result, mem_pool_ptr->POOL_BLOCK_IN_ERROR);
    return (result);

} /* Endbody */

/*!
 * \private
 *
 * \brief Creates the memory pool from memory that is outside the default memory
 * pool.
 *
 * Tasks use the pool ID to allocate (variable-size) memory blocks from the pool.
 *
 * \param[in] start        Address of the start of the memory pool.
 * \param[in] end          The end of the memory pool.
 * \param[in] mem_pool_ptr Where to store the memory pool context info.
 *
 * \return MQX_OK
 * \return MQX_CORRUPT_MEMORY_SYSTEM (Internal data for the message component is
 * corrupted.)
 * \return MQX_MEM_POOL_TOO_SMALL (Size is less than the minimum allowable
 * message-pool size.)
 *
 * \see _mem_create_pool
 */
_mqx_uint _mem_create_pool_internal
(
    void              *start,
    void              *end,
    MEMPOOL_STRUCT_PTR mem_pool_ptr
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    STOREBLOCK_STRUCT_PTR block_ptr;
    STOREBLOCK_STRUCT_PTR end_block_ptr;

    _GET_KERNEL_DATA(kernel_data);

#if MQX_CHECK_VALIDITY
    _INT_DISABLE();
    if (kernel_data->MEM_COMP.VALID != MEMPOOL_VALID) {
        /* The RTOS memory system has been corrupted */
        _int_enable();
        return (MQX_CORRUPT_MEMORY_SYSTEM);
    } /* Endif */

    _INT_ENABLE();
#endif

    /* Align the start of the pool */
    mem_pool_ptr->POOL_PTR = (STOREBLOCK_STRUCT_PTR)
    _ALIGN_ADDR_TO_HIGHER_MEM(start);

    /* Set the end of memory (aligned) */
    mem_pool_ptr->POOL_LIMIT = (STOREBLOCK_STRUCT_PTR)
    _ALIGN_ADDR_TO_LOWER_MEM(end);

#if MQX_CHECK_ERRORS
    if ((unsigned char *) mem_pool_ptr->POOL_LIMIT <= ((unsigned char *) mem_pool_ptr->POOL_PTR + MQX_MIN_MEMORY_POOL_SIZE)) {
        return MQX_MEM_POOL_TOO_SMALL;
    } /* Endif */
#endif

    block_ptr = (STOREBLOCK_STRUCT_PTR) mem_pool_ptr->POOL_PTR;
    mem_pool_ptr->POOL_HIGHEST_MEMORY_USED = (void *) block_ptr;
    mem_pool_ptr->POOL_CHECK_POOL_PTR = (char *) mem_pool_ptr->POOL_PTR;
    mem_pool_ptr->POOL_BLOCK_IN_ERROR = NULL;

    /* Compute the pool size. */
    mem_pool_ptr->POOL_SIZE = (_mem_size) ((unsigned char *) mem_pool_ptr->POOL_LIMIT - (unsigned char *) mem_pool_ptr->POOL_PTR);

    /* Set up the first block as an idle block */
    block_ptr->BLOCKSIZE = mem_pool_ptr->POOL_SIZE - MQX_MIN_MEMORY_STORAGE_SIZE;
    block_ptr->USER_AREA = NULL;
    block_ptr->PREVBLOCK = NULL;
    block_ptr->NEXTBLOCK = NULL;
    MARK_BLOCK_AS_FREE(block_ptr);

    CALC_CHECKSUM(block_ptr);

    mem_pool_ptr->POOL_FREE_LIST_PTR = block_ptr;

    /*
     * Set up last block as an in_use block, so that the _mem_free algorithm
     * will work (block coalescing)
     */
    end_block_ptr = (STOREBLOCK_STRUCT_PTR)((unsigned char *) block_ptr + block_ptr->BLOCKSIZE);
    end_block_ptr->BLOCKSIZE = (_mem_size) (MQX_MIN_MEMORY_STORAGE_SIZE);
    end_block_ptr->USER_AREA = 0;
    end_block_ptr->PREVBLOCK = (struct storeblock_struct *) block_ptr;
    end_block_ptr->NEXTBLOCK = NULL;
    MARK_BLOCK_AS_USED(end_block_ptr, SYSTEM_TASK_ID(kernel_data));
    CALC_CHECKSUM(end_block_ptr);

    mem_pool_ptr->POOL_END_PTR = end_block_ptr;

    /* Initialize the list of extensions to this pool */
    _QUEUE_INIT(&mem_pool_ptr->EXT_LIST, 0);

    mem_pool_ptr->VALID = MEMPOOL_VALID;

    /* Protect the list of pools while adding new pool */
    _lwsem_wait((LWSEM_STRUCT_PTR) &kernel_data->MEM_COMP.SEM);
    _QUEUE_ENQUEUE(&kernel_data->MEM_COMP.POOLS, &mem_pool_ptr->LINK);
    _lwsem_post((LWSEM_STRUCT_PTR) &kernel_data->MEM_COMP.SEM);

    return MQX_OK;

} /* Endbody */

/*!
 * \brief Creates the memory pool from memory that is outside the default memory
 * pool.
 *
 * Tasks use the pool ID to allocate (variable-size) memory blocks from the pool.
 *
 * \param[in] start Address of the start of the memory pool.
 * \param[in] size  Size of the memory pool.
 *
 * \return MQX_OK
 * \return NULL
 *
 * \warning On failure, calls _task_set_error() to set one of the following
 * task error codes:
 * \li MQX_CORRUPT_MEMORY_SYSTEM (Internal data for the message component is
 * corrupted.)
 * \li MQX_MEM_POOL_TOO_SMALL (Size is less than the minimum allowable
 * message-pool size.)
 *
 * \see _mem_alloc()
 * \see _mem_alloc_from()
 * \see _mem_alloc_system()
 * \see _mem_alloc_system_from()
 * \see _mem_alloc_system_zero()
 * \see _mem_alloc_system_zero_from()
 * \see _mem_alloc_system_align() 
 * \see _mem_alloc_system_align_from() 
 * \see _mem_alloc_zero()
 * \see _mem_alloc_zero_from
 * \see _mem_alloc_align()
 * \see _mem_alloc_align_from()
 * \see _mem_alloc_at()
 * \see _mem_realloc()
 * \see _task_set_error
 */
_mem_pool_id _mem_create_pool
(
    void   *start,
    _mem_size size
)
{ /* Body */
    _KLOGM(KERNEL_DATA_STRUCT_PTR kernel_data);
    MEMPOOL_STRUCT_PTR mem_pool_ptr;
    unsigned char *end;
    _mqx_uint error;

    _KLOGM(_GET_KERNEL_DATA(kernel_data));
    _KLOGE3(KLOG_mem_create_pool, start, size);

#if MQX_CHECK_ERRORS
    if (size < MQX_MIN_MEMORY_POOL_SIZE) {
        error = MQX_MEM_POOL_TOO_SMALL;
        _task_set_error(error);
        _KLOGX4(KLOG_mem_create_pool, start, size, error);
        return NULL;
    } /* Endif */
#endif

    mem_pool_ptr = (MEMPOOL_STRUCT_PTR)_ALIGN_ADDR_TO_HIGHER_MEM(start);
    _mem_zero((void *) mem_pool_ptr, (_mem_size) sizeof(MEMPOOL_STRUCT));

    end = (unsigned char *) start + size;
    start = (void *) ((unsigned char *) mem_pool_ptr + sizeof(MEMPOOL_STRUCT));
    error = _mem_create_pool_internal(start, (void *) end, mem_pool_ptr);

#if (MQX_CHECK_ERRORS)
    if (error != MQX_OK) {
        _task_set_error(error);
        _KLOGX4(KLOG_mem_create_pool, start, size, error);
        return NULL;
    } /* Endif */
#endif

    return ((_mem_pool_id) mem_pool_ptr);

} /* Endbody */

/*!
 * \brief Adds the specified memory area for use by the memory manager.
 *
 * The function adds the specified memory to the default memory pool.
 * \n The function fails if size is less than (3 * MQX_MIN_MEMORY_STORAGE_SIZE),
 * as defined in mem_prv.h
 *
 * \param[in] start_of_pool Pointer to the start of the memory to add.
 * \param[in] size          Size of the memory pool to add.
 *
 * \return MQX_OK
 * \return MQX_INVALID_SIZE (Failure, see Description.)
 * \return MQX_INVALID_COMPONENT_HANDLE (Memory pool to extend is not valid.)
 *
 * \see _mem_get_highwater
 * \see MQX_INITIALIZATION_STRUCT
 */
_mqx_uint _mem_extend
(
    void   *start_of_pool,
    _mem_size size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    _mqx_uint result;

    _GET_KERNEL_DATA(kernel_data);

    _KLOGE3(KLOG_mem_extend, start_of_pool, size);

    result = _mem_extend_pool_internal(start_of_pool, size, (MEMPOOL_STRUCT_PTR) kernel_data->KD_POOL);

    _KLOGX2(KLOG_mem_extend, result);

    return (result);

} /* Endbody */

/*!
 * \private
 *
 * \brief Adds physical memory to the memory pool, which is outside the default
 * memory pool.
 *
 * \param[in] start_of_pool Pointer to the start of the memory to add.
 * \param[in] size          Size of the memory pool addition.
 * \param[in] mem_pool_ptr  The memory pool to extend.
 *
 * \return MQX_OK
 * \return MQX_INVALID_SIZE (Failure, see Description.)
 * \return MQX_INVALID_COMPONENT_HANDLE (Memory pool to extend is not valid.)
 *
 * \see _mem_create_pool
 * \see _mem_get_highwater_pool
 */
_mqx_uint _mem_extend_pool_internal
(
    void              *start_of_pool,
    _mem_size          size,
    MEMPOOL_STRUCT_PTR mem_pool_ptr
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    STOREBLOCK_STRUCT_PTR block_ptr;
    STOREBLOCK_STRUCT_PTR end_ptr;
    STOREBLOCK_STRUCT_PTR free_ptr;
    STOREBLOCK_STRUCT_PTR tmp_ptr;
    MEMPOOL_EXTENSION_STRUCT_PTR ext_ptr;
    unsigned char *real_start_ptr;
    unsigned char *end_of_pool;
    _mem_size block_size;
    _mem_size real_size;
    _mem_size free_block_size;

    _GET_KERNEL_DATA(kernel_data);

#if MQX_CHECK_ERRORS
    if (size < (_mem_size) (3 * MQX_MIN_MEMORY_STORAGE_SIZE)) {
        /* Pool must be big enough to hold at least 3 memory blocks */
        return (MQX_INVALID_SIZE);
    }/* Endif */
#endif

#if MQX_CHECK_VALIDITY
    if (mem_pool_ptr->VALID != MEMPOOL_VALID) {
        return (MQX_INVALID_COMPONENT_HANDLE);
    }/* Endif */
#endif

    ext_ptr = (MEMPOOL_EXTENSION_STRUCT_PTR)
    _ALIGN_ADDR_TO_HIGHER_MEM(start_of_pool);

    real_start_ptr = (unsigned char *) ext_ptr + sizeof(MEMPOOL_EXTENSION_STRUCT);
    real_start_ptr = (unsigned char *) _ALIGN_ADDR_TO_HIGHER_MEM(real_start_ptr);

    end_of_pool = (unsigned char *) start_of_pool + size;
    end_of_pool = (unsigned char *) _ALIGN_ADDR_TO_LOWER_MEM(end_of_pool);

    real_size = (_mem_size) (end_of_pool - real_start_ptr);

    ext_ptr->START = start_of_pool;
    ext_ptr->SIZE = size;
    ext_ptr->REAL_START = real_start_ptr;

    block_ptr = (STOREBLOCK_STRUCT_PTR) real_start_ptr;
    block_size = MQX_MIN_MEMORY_STORAGE_SIZE;

    free_ptr = (STOREBLOCK_STRUCT_PTR)((unsigned char *) block_ptr + block_size);
    free_block_size = real_size - (_mem_size) (2 * MQX_MIN_MEMORY_STORAGE_SIZE);

    end_ptr = (STOREBLOCK_STRUCT_PTR)((unsigned char *) free_ptr + free_block_size);

    /*
     * Make a small minimal sized memory block to be as
     * the first block in the pool.  This will be an in-use block
     * and will thus avoid problems with memory co-allescing during
     * memory frees
     */
    block_ptr->BLOCKSIZE = block_size;
    block_ptr->MEM_TYPE = 0;
    block_ptr->USER_AREA = 0;
    block_ptr->PREVBLOCK = (struct storeblock_struct *) NULL;
    block_ptr->NEXTBLOCK = free_ptr;
    MARK_BLOCK_AS_USED(block_ptr, SYSTEM_TASK_ID(kernel_data));
    CALC_CHECKSUM(block_ptr);

    /*
     * Let the next block be the actual free block that will be added
     * to the free list
     */
    free_ptr->BLOCKSIZE = free_block_size;
    free_ptr->MEM_TYPE = 0;
    free_ptr->USER_AREA = 0;
    free_ptr->PREVBLOCK = block_ptr;
    free_ptr->NEXTBLOCK = end_ptr;
    MARK_BLOCK_AS_FREE(free_ptr);
    CALC_CHECKSUM(free_ptr);

    /*
     * Set up a minimal sized block at the end of the pool, and also
     * mark it as being allocated.  Again this is to comply with the
     * _mem_free algorithm
     */
    end_ptr->BLOCKSIZE = block_size;
    end_ptr->MEM_TYPE = 0;
    end_ptr->USER_AREA = 0;
    end_ptr->PREVBLOCK = free_ptr;
    end_ptr->NEXTBLOCK = NULL;
    MARK_BLOCK_AS_USED(end_ptr, SYSTEM_TASK_ID(kernel_data));
    CALC_CHECKSUM(end_ptr);

    _int_disable(); /* Add the block to the free list */
    tmp_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR;
    mem_pool_ptr->POOL_FREE_LIST_PTR = free_ptr;
    if (tmp_ptr != NULL) {
        PREV_FREE( tmp_ptr) = free_ptr;
    } /* Endif */
    PREV_FREE( free_ptr) = NULL;
    NEXT_FREE( free_ptr) = tmp_ptr;

    /* Reset the free list queue walker for some other task */
    mem_pool_ptr->POOL_FREE_CURRENT_BLOCK = mem_pool_ptr->POOL_FREE_LIST_PTR;

    /* Link in the extension */
    _QUEUE_ENQUEUE(&mem_pool_ptr->EXT_LIST, &ext_ptr->LINK);

    _int_enable();

    return (MQX_OK);

} /* Endbody */

/*!
 * \brief Adds physical memory to the memory pool, which is outside the default
 * memory pool.
 *
 * The function adds the specified memory to the memory pool.
 * \n The function fails if size is less than (3 * MIN_MEMORY_STORAGE_SIZE), as
 * defined in mem_prv.h.
 *
 * \param[in] pool_id       Pool to which to add memory (from _mem_create_pool()).
 * \param[in] start_of_pool Pointer to the start of the memory to add.
 * \param[in] size          Size of the memory pool addition.
 *
 * \return MQX_OK
 * \return MQX_INVALID_SIZE (Failure, see Description.)
 * \return MQX_INVALID_COMPONENT_HANDLE (Memory pool to extend is not valid.)
 *
 * \see _mem_create_pool
 * \see _mem_get_highwater_pool
 */
_mqx_uint _mem_extend_pool
(
    _mem_pool_id pool_id,
    void        *start_of_pool,
    _mem_size    size
)
{ /* Body */
    _KLOGM(KERNEL_DATA_STRUCT_PTR kernel_data);
    _mqx_uint result;

    _KLOGM(_GET_KERNEL_DATA(kernel_data));

    _KLOGE4(KLOG_mem_extend_pool, start_of_pool, size, pool_id);

    result = _mem_extend_pool_internal(start_of_pool, size, (MEMPOOL_STRUCT_PTR) pool_id);

    _KLOGX2(KLOG_mem_extend_pool, result);

    return (result);

} /* Endbody */

/*!
 * \brief Free part of the memory block.
 *
 * Under the same restriction as for _mem_free(), the function trims from the
 * end of the memory block.
 * \n A successful call to the function frees memory only if requested_size is
 * sufficiently smaller than the size of the original block. To determine whether
 * the function freed memory, call _mem_get_size() before and after calling
 * _mem_free_part().
 *
 * \param[in] mem_ptr        Pointer to the memory block to trim.
 * \param[in] requested_size New size for the block.
 *
 * \return MQX_OK
 * \return MQX_INVALID_POINTER (Mem_ptr is NULL, not in the pool, or misaligned.)
 * \return MQX_INVALID_CHECKSUM (Block's checksum is not correct, indicating
 * that at least some of the block was overwritten.)
 * \return MQX_NOT_RESOURCE_OWNER (If the block was allocated with _mem_alloc()
 * or _mem_alloc_zero(), only the task that allocated it can free part of it.)
 * \return MQX_INVALID_SIZE (Size of the original block is less than requested_size
 * or requested_size is less than 0 or remaining space is less than 
 * twice the size of MQX_MIN_MEMORY_STORAGE_SIZE .).
 *
 * \warning On failure, calls _task_set_error() to set the task error code (see
 * return values)
 *
 * \see _mem_free
 * \see _mem_alloc()
 * \see _mem_alloc_from()
 * \see _mem_alloc_system()
 * \see _mem_alloc_system_from()
 * \see _mem_alloc_system_zero()
 * \see _mem_alloc_system_zero_from()
 * \see _mem_alloc_system_align() 
 * \see _mem_alloc_system_align_from() 
 * \see _mem_alloc_zero()
 * \see _mem_alloc_zero_from
 * \see _mem_alloc_align()
 * \see _mem_alloc_align_from()
 * \see _mem_alloc_at()
 * \see _mem_realloc()
 * \see _mem_get_size
 * \see _task_set_error
 */
_mqx_uint _mem_free_part
(
    void     *mem_ptr,
    _mem_size requested_size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    STOREBLOCK_STRUCT_PTR block_ptr;
    STOREBLOCK_STRUCT_PTR prev_block_ptr;
    STOREBLOCK_STRUCT_PTR next_block_ptr;
    STOREBLOCK_STRUCT_PTR new_block_ptr;
    TD_STRUCT_PTR td_ptr;

    _mem_size size;
    _mem_size block_size;
    _mem_size new_block_size;
    _mqx_uint result_code;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE3(KLOG_mem_free_part, mem_ptr, requested_size);

#if MQX_CHECK_ERRORS
    /* Make sure a correct pointer was passed in.    */
    if (mem_ptr == NULL) {
        _task_set_error(MQX_INVALID_POINTER);
        _KLOGX2(KLOG_mem_free_part, MQX_INVALID_POINTER);
        return (MQX_INVALID_POINTER);
    } /* Endif */
#endif

    /* Verify the block size */
    block_ptr = GET_MEMBLOCK_PTR(mem_ptr);

#if MQX_CHECK_ERRORS
    if (!_MEMORY_ALIGNED(block_ptr)) {
        _task_set_error(MQX_INVALID_POINTER);
        _KLOGX2(KLOG_mem_free_part, MQX_INVALID_POINTER);
        return (MQX_INVALID_POINTER);
    } /* Endif */

    if ((block_ptr->BLOCKSIZE < MQX_MIN_MEMORY_STORAGE_SIZE) || BLOCK_IS_FREE(block_ptr)) {
        _task_set_error(MQX_INVALID_POINTER);
        kernel_data->KD_POOL->POOL_BLOCK_IN_ERROR = block_ptr;
        _KLOGX3(KLOG_mem_free_part, MQX_INVALID_POINTER, block_ptr);
        return (MQX_INVALID_POINTER);
    } /* Endif */
#endif

    _INT_DISABLE();
#if MQX_CHECK_VALIDITY
    if (!VALID_CHECKSUM(block_ptr)) {
        _int_enable();
        _task_set_error(MQX_INVALID_CHECKSUM);
        kernel_data->KD_POOL->POOL_BLOCK_IN_ERROR = block_ptr;
        _KLOGX3(KLOG_mem_free_part, MQX_INVALID_CHECKSUM, block_ptr);
        return (MQX_INVALID_CHECKSUM);
    } /* Endif */
#endif

    td_ptr = SYSTEM_TD_PTR(kernel_data);
    if (block_ptr->TASK_NUMBER != (TASK_NUMBER_FROM_TASKID(td_ptr->TASK_ID))) {
        td_ptr = kernel_data->ACTIVE_PTR;
    }

    /*  Walk through the memory resources of the task descriptor.
     *  Two pointers are maintained, one to the current block
     *  and one to the previous block.
     */
    next_block_ptr = (STOREBLOCK_STRUCT_PTR) td_ptr->MEMORY_RESOURCE_LIST;
    prev_block_ptr = (STOREBLOCK_STRUCT_PTR)((unsigned char *) (&td_ptr->MEMORY_RESOURCE_LIST)
                    - FIELD_OFFSET(STOREBLOCK_STRUCT,NEXTBLOCK));

    /* Scan the task's memory resource list searching for the block to
     * free, Stop when the current pointer is equal to the block to free
     * or the end of the list is reached.
     */
    while (next_block_ptr && ((void *) next_block_ptr != mem_ptr)) {
        /* The block is not found, and the end of the list has not been
         * reached, so move down the list.
         */
        prev_block_ptr = GET_MEMBLOCK_PTR(next_block_ptr);
        next_block_ptr = (STOREBLOCK_STRUCT_PTR) prev_block_ptr->NEXTBLOCK;
    } /* Endwhile */

#if MQX_CHECK_ERRORS
    if (next_block_ptr == NULL) {
        _int_enable();
        /* The specified block does not belong to the calling task. */
        _task_set_error(MQX_NOT_RESOURCE_OWNER);
        _KLOGX2(KLOG_mem_free_part, MQX_NOT_RESOURCE_OWNER);
        return (MQX_NOT_RESOURCE_OWNER);
    } /* Endif */
#endif

    /* determine the size of the block.  */
    block_size = block_ptr->BLOCKSIZE;

    size = requested_size + (_mem_size) FIELD_OFFSET(STOREBLOCK_STRUCT,USER_AREA);
    if (size < MQX_MIN_MEMORY_STORAGE_SIZE) {
        size = MQX_MIN_MEMORY_STORAGE_SIZE;
    } /* Endif */
    _MEMORY_ALIGN_VAL_LARGER(size);

#if MQX_CHECK_ERRORS
    /* Verify that the size parameter is within range of the block size. */
    if (size <= block_size) {
#endif
        /* Adjust the size to allow for the overhead and force alignment */

        /* Compute the size of the new_ block that would be created. */
        new_block_size = block_size - size;

        /* Decide if it worthwile to split the block. If the amount of space
         * returned is not at least twice the size of the block overhead,
         * then return error code.
         */
        if (new_block_size >= (2 * MQX_MIN_MEMORY_STORAGE_SIZE)) {

            /* Create an 'inuse' block */
            new_block_ptr = (STOREBLOCK_STRUCT_PTR)((char *) block_ptr + size);
            new_block_ptr->BLOCKSIZE = new_block_size;
            PREV_PHYS( new_block_ptr) = block_ptr;
            new_block_ptr->TASK_NUMBER = block_ptr->TASK_NUMBER;
            new_block_ptr->MEM_POOL_PTR = block_ptr->MEM_POOL_PTR;
            CALC_CHECKSUM(new_block_ptr);
            /* Split the block */
            block_ptr->BLOCKSIZE = size;
            CALC_CHECKSUM(block_ptr);

            /* make sure right physical neighbour knows about it */
            block_ptr = NEXT_PHYS(new_block_ptr);
            PREV_PHYS( block_ptr) = new_block_ptr;
            CALC_CHECKSUM(block_ptr);

            /* Link the new block onto the requestor's task descriptor. */
            new_block_ptr->NEXTBLOCK = td_ptr->MEMORY_RESOURCE_LIST;
            td_ptr->MEMORY_RESOURCE_LIST = (char *) (&new_block_ptr->USER_AREA);

            result_code = _mem_free((void *) &new_block_ptr->USER_AREA);
        }
        else {
            result_code = MQX_INVALID_SIZE;
        } /* Endif */
#if MQX_CHECK_ERRORS
    }
    else {
        result_code = MQX_INVALID_SIZE;
    } /* Endif */
#endif

#if MQX_CHECK_ERRORS
    if (result_code != MQX_OK) {
        _task_set_error(result_code);
    } /* Endif */
#endif

    _INT_ENABLE();
    _KLOGX2(KLOG_mem_free_part, result_code);
    return (result_code);
}

/*!
 * \brief Frees the given memory block.
 *
 * If the memory block was allocated with one of the following functions, only
 * the task that owns the block can free it:
 * \li _mem_alloc()
 * \li _mem_alloc_from()
 * \li _mem_alloc_zero()
 * \li _mem_alloc_zero_from()
 *
 * \n Any task can free a memory block that is allocated with one of the following
 * functions:
 * \li _mem_alloc_system()
 * \li _mem_alloc_system_from()
 * \li _mem_alloc_system_zero()
 * \li _mem_alloc_system_zero_from()
 * \li _mem_alloc_system_align()
 * \li _mem_alloc_system_align_from()
 *
 * \param[in] mem_ptr Pointer to the memory block to free.
 *
 * \return MQX_OK
 * \return MQX_INVALID_POINTER (Mem_ptr is NULL, not in the pool, or misaligned.)
 * \return MQX_INVALID_CHECKSUM (Block's checksum is not correct, indicating
 * that at least some of the block was overwritten.)
 * \return MQX_NOT_RESOURCE_OWNER (If the block was allocated with _mem_alloc()
 * or _mem_alloc_zero(), only the task that allocated it can free part of it.)
 *
 * \warning On failure, calls _task_set_error() to set the task error code (see
 * return values).
 *
 * \see _mem_free_part
 * \see _mem_alloc()
 * \see _mem_alloc_from()
 * \see _mem_alloc_system()
 * \see _mem_alloc_system_from()
 * \see _mem_alloc_system_zero()
 * \see _mem_alloc_system_zero_from()
 * \see _mem_alloc_system_align() 
 * \see _mem_alloc_system_align_from() 
 * \see _mem_alloc_zero()
 * \see _mem_alloc_zero_from
 * \see _mem_alloc_align()
 * \see _mem_alloc_align_from()
 * \see _mem_alloc_at()
 * \see _mem_realloc()
 * \see _task_set_error
 */
_mqx_uint _mem_free
(
    void   *mem_ptr
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    STOREBLOCK_STRUCT_PTR block_ptr;
    STOREBLOCK_STRUCT_PTR prev_block_ptr;
    STOREBLOCK_STRUCT_PTR next_block_ptr;
    MEMPOOL_STRUCT_PTR mem_pool_ptr;
    TD_STRUCT_PTR td_ptr;

    _GET_KERNEL_DATA(kernel_data);

    _KLOGE2(KLOG_mem_free, mem_ptr);

#if MQX_CHECK_ERRORS
    /* Verify the passed in parameter */
    if (mem_ptr == NULL) {
        _task_set_error(MQX_INVALID_POINTER);
        _KLOGX2(KLOG_mem_free, MQX_INVALID_POINTER);
        return (MQX_INVALID_POINTER);
    } /* Endif */
#endif

    block_ptr = GET_MEMBLOCK_PTR(mem_ptr);

#if MQX_CHECK_ERRORS
    /* Verify pointer alignment */
    if (!_MEMORY_ALIGNED(block_ptr) || (block_ptr->BLOCKSIZE < MQX_MIN_MEMORY_STORAGE_SIZE) || BLOCK_IS_FREE(block_ptr)) {
        _task_set_error(MQX_INVALID_POINTER);
        _KLOGX2(KLOG_mem_free, MQX_INVALID_POINTER);
        return (MQX_INVALID_POINTER);
    } /* Endif */

#endif

    _INT_DISABLE();

#if MQX_CHECK_VALIDITY
    if (!VALID_CHECKSUM(block_ptr)) {
        _int_enable();
        _task_set_error(MQX_INVALID_CHECKSUM);
        _KLOGX2(KLOG_mem_free, MQX_INVALID_CHECKSUM);
        return (MQX_INVALID_CHECKSUM);
    } /* Endif */
#endif

    mem_pool_ptr = (MEMPOOL_STRUCT_PTR) block_ptr->MEM_POOL_PTR;
    td_ptr = SYSTEM_TD_PTR(kernel_data);
    if (block_ptr->TASK_NUMBER != (TASK_NUMBER_FROM_TASKID(td_ptr->TASK_ID))) {
        td_ptr = kernel_data->ACTIVE_PTR;
    } /* Endif */

    /*
     * Walk through the memory resources of the task descriptor.
     * Two pointers are maintained, one to the current block
     * and one to the previous block.
     */
    next_block_ptr = (STOREBLOCK_STRUCT_PTR) td_ptr->MEMORY_RESOURCE_LIST;
    prev_block_ptr = (STOREBLOCK_STRUCT_PTR)((unsigned char *) (&td_ptr->MEMORY_RESOURCE_LIST)
                    - FIELD_OFFSET(STOREBLOCK_STRUCT,NEXTBLOCK));

    /*
     * Scan the task's memory resource list searching for the block to
     * free, Stop when the current pointer is equal to the block to free
     * or the end of the list is reached.
     */
    while (next_block_ptr && ((void *) next_block_ptr != mem_ptr)) {
        /*
         * The block is not found, and the end of the list has not been
         * reached, so move down the list.
         */
        prev_block_ptr = GET_MEMBLOCK_PTR(next_block_ptr);
        next_block_ptr = (STOREBLOCK_STRUCT_PTR) prev_block_ptr->NEXTBLOCK;
    } /* Endwhile */

#if MQX_CHECK_ERRORS
    if (next_block_ptr == NULL) {
        _int_enable();
        /* The specified block does not belong to the calling task. */
        _task_set_error(MQX_NOT_RESOURCE_OWNER);
        _KLOGX2(KLOG_mem_free, MQX_NOT_RESOURCE_OWNER);
        return (MQX_NOT_RESOURCE_OWNER);
    } /* Endif */
#endif

    /* Remove the memory block from the resource list of the calling task. */
    prev_block_ptr->NEXTBLOCK = block_ptr->NEXTBLOCK;

#if MQX_MEM_MONITOR
    kernel_data->MEM_USED -= block_ptr->BLOCKSIZE;
#endif

#if MQX_ALLOW_TYPED_MEMORY
    block_ptr->MEM_TYPE = 0;
#endif

    /*
     * Check if the neighbouring blocks are free, so we
     * can coalesce the blocks.
     */
    if (_mem_check_coalesce_internal(block_ptr)) {
        /* No need to add block to free list if coalesced */
        _INT_ENABLE();
        _KLOGX2(KLOG_mem_free, MQX_OK);
        return (MQX_OK);
    } /* Endif */

#if MQX_MEMORY_FREE_LIST_SORTED == 1

    next_block_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR;
    if (next_block_ptr != NULL) {

        /* Insertion sort into the free list by address*/
        while (next_block_ptr < block_ptr) {
            /* This takes some time, so allow higher priority tasks
             * to interrupt us.
             */

            if (NEXT_FREE(next_block_ptr) == NULL) {
                /* At end of free list */
                break;
            } /* Endif */

            next_block_ptr = (STOREBLOCK_STRUCT_PTR) NEXT_FREE(next_block_ptr);

            /* Save the current location in case premption occurs */
            mem_pool_ptr->POOL_FREE_CURRENT_BLOCK = next_block_ptr;
            _INT_ENABLE();
            _INT_DISABLE();

            /* Reset to the start if we were preempted 
               and the preempting task did finish its free/malloc operation */
            if(mem_pool_ptr->POOL_FREE_CURRENT_BLOCK != next_block_ptr){
                next_block_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR;
                if (next_block_ptr == NULL)
                {
                    break;
                }
            }

            if (_mem_check_coalesce_internal(block_ptr)) {
                /* No need to add block to free list if coalesced */
                _INT_ENABLE();
                _KLOGX2(KLOG_mem_free, MQX_OK);
                return (MQX_OK);
            } /* Endif */
        } /* Endwhile */

    } /* Endif */

    /* We have found the correct location */

    /* Make the block a free block */
    block_ptr->NEXTBLOCK = NULL;
    block_ptr->BLOCKSIZE = block_ptr->BLOCKSIZE;
    MARK_BLOCK_AS_FREE(block_ptr);
    CALC_CHECKSUM(block_ptr);

    /* Insert current block just before next block */
    if (next_block_ptr == mem_pool_ptr->POOL_FREE_LIST_PTR) {
        /* We are inserting at the head of the free list */
        mem_pool_ptr->POOL_FREE_LIST_PTR = block_ptr;
        if (next_block_ptr != NULL) {
            PREV_FREE( next_block_ptr) = (void *) block_ptr;
        } /* Endif */
        PREV_FREE( block_ptr) = NULL;
        NEXT_FREE( block_ptr) = (void *) next_block_ptr;

    }
    else if (next_block_ptr < block_ptr) {
        /* We are inserting at the end of the free list */
        NEXT_FREE( block_ptr) = NULL;
        PREV_FREE( block_ptr) = (void *) next_block_ptr;
        NEXT_FREE( next_block_ptr) = (void *) block_ptr;

    }
    else {
        /* We are inserting into the middle of the free list */
        PREV_FREE( block_ptr) = PREV_FREE(next_block_ptr);
        PREV_FREE( next_block_ptr) = (void *) block_ptr;
        NEXT_FREE( block_ptr) = (void *) next_block_ptr;
        NEXT_FREE( PREV_FREE(
            block_ptr)) = (void *) block_ptr;
    } /* Endif */

    /*
     * Reset the freelist current block pointer in case we pre-empted
     * another task
     */
    mem_pool_ptr->POOL_FREE_CURRENT_BLOCK = mem_pool_ptr->POOL_FREE_LIST_PTR;

#else

    /* Make the block a free block */
    block_ptr->NEXTBLOCK = NULL;
    MARK_BLOCK_AS_FREE(block_ptr);
    CALC_CHECKSUM(block_ptr);

    /* Put the block at the head of the free list */
    next_block_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR;
    mem_pool_ptr->POOL_FREE_LIST_PTR = block_ptr;

    if ( next_block_ptr != NULL ) {
        PREV_FREE(next_block_ptr) = (void *)block_ptr;
    } /* Endif */
    PREV_FREE(block_ptr) = NULL;
    NEXT_FREE(block_ptr) = (void *)next_block_ptr;

#endif

    /* Reset the _mem_test pointers */
    mem_pool_ptr->POOL_PHYSICAL_CHECK_BLOCK = (STOREBLOCK_STRUCT_PTR) mem_pool_ptr->POOL_PTR;
    mem_pool_ptr->POOL_FREE_CHECK_BLOCK = mem_pool_ptr->POOL_FREE_LIST_PTR;

    _INT_ENABLE();
    _KLOGX2(KLOG_mem_free, MQX_OK);
    return (MQX_OK);

} /* Endbody */

/*!
 * \private
 *
 * \brief This function coalesces any free block found adjacent to the provided
 * block.
 *
 * \param[in] passed_block_ptr An Address of a memory block, whose neighbours
 * has to be checked to see if they are free.
 *
 * \return TRUE (Coalescing was performed), FALSE (Coalescing was not performed).
 */
bool _mem_check_coalesce_internal
(
    STOREBLOCK_STRUCT_PTR passed_block_ptr
)
{ /* Body */
    register STOREBLOCK_STRUCT_PTR block_ptr;
    register STOREBLOCK_STRUCT_PTR prev_block_ptr;
    register STOREBLOCK_STRUCT_PTR next_block_ptr;
    MEMPOOL_STRUCT_PTR mem_pool_ptr;
    bool have_coalesced = FALSE;

    block_ptr = passed_block_ptr;
    mem_pool_ptr = (MEMPOOL_STRUCT_PTR) block_ptr->MEM_POOL_PTR;

    /* Check the previous physical neighbour */
    prev_block_ptr = PREV_PHYS(block_ptr);
    if ((prev_block_ptr != NULL) && BLOCK_IS_FREE(prev_block_ptr)) {
        /* the block previous to this one is free */

#if MQX_CHECK_ERRORS
        assert(((char *)prev_block_ptr + prev_block_ptr->BLOCKSIZE) == (char *)block_ptr);
#endif

        /*  make the current block a free block so it can't be freed again */
        block_ptr->NEXTBLOCK = NULL;
        MARK_BLOCK_AS_FREE(block_ptr);

        /* Add the current block to the previous block */
        prev_block_ptr->BLOCKSIZE += block_ptr->BLOCKSIZE;

        /* Modify the next physical block to point to the previous block */
        next_block_ptr = NEXT_PHYS(block_ptr);
        PREV_PHYS( next_block_ptr) = prev_block_ptr;
        CALC_CHECKSUM(prev_block_ptr);
        CALC_CHECKSUM(next_block_ptr);

        block_ptr = prev_block_ptr; /* Set up as the current block */
        have_coalesced = TRUE;

    } /* Endif */

    /* Now, check the next block to see if it is free */
    next_block_ptr = NEXT_PHYS(block_ptr);
    if (BLOCK_IS_FREE(next_block_ptr)) {
        /* the next block is free */

        if (mem_pool_ptr->POOL_ALLOC_CURRENT_BLOCK == next_block_ptr) {
            /* We must modify the _mem_alloc pointer to not point
             ** at the next block
             */
            mem_pool_ptr->POOL_ALLOC_CURRENT_BLOCK = block_ptr;
        } /* Endif */

        if (have_coalesced) {
            /* the current block is already on the free list */

            /* Remove the block from the free list */
            if (block_ptr == mem_pool_ptr->POOL_FREE_LIST_PTR) {
                mem_pool_ptr->POOL_FREE_LIST_PTR = (STOREBLOCK_STRUCT_PTR) NEXT_FREE(block_ptr);
                if (NEXT_FREE(block_ptr) != NULL) {
                    PREV_FREE( NEXT_FREE(
                        block_ptr)) = 0;
                } /* Endif */
            }
            else {
                NEXT_FREE( PREV_FREE(
                    block_ptr)) = NEXT_FREE(block_ptr);
                if (NEXT_FREE(block_ptr) != NULL) {
                    PREV_FREE( NEXT_FREE(
                        block_ptr)) = PREV_FREE(block_ptr);
                } /* Endif */
            } /* Endif */

        }
        else {

            /* Make the block a free block */
            block_ptr->NEXTBLOCK = NULL;
            MARK_BLOCK_AS_FREE(block_ptr);

        } /* Endif */

        /*
         * The current block is now a free block not on the free list .
         * the freelist pointers have to be modified so that the next
         * block is removed from the list, replace with the current block.
         */

        /* set the next free block to be the same as the one on the free list */
        NEXT_FREE( block_ptr) = NEXT_FREE(next_block_ptr);

        /*
         * And set the back pointer of the block after the next free block
         * to point back to the current block
         */
        if (NEXT_FREE(block_ptr) != NULL) {
            PREV_FREE( NEXT_FREE(
                block_ptr)) = (STOREBLOCK_STRUCT_PTR) block_ptr;
        } /* Endif */

        if (next_block_ptr == mem_pool_ptr->POOL_FREE_LIST_PTR) {
            /*
             * If the next block pointer was at the head of the free list,
             * the kernel free list pointer must be updated
             */
            mem_pool_ptr->POOL_FREE_LIST_PTR = block_ptr;
            PREV_FREE( block_ptr) = NULL;
        }
        else {
            /*
             * Otherwise we need to adjust the pointers of the block that
             * was previous to the next block on the free list
             */
            PREV_FREE( block_ptr) = PREV_FREE(next_block_ptr);
            if (PREV_FREE(block_ptr) != NULL) {
                NEXT_FREE( PREV_FREE(
                    block_ptr)) = (void *) block_ptr;
            } /* Endif */
        } /* Endif */

        /* Add the next block onto the current block */
        block_ptr->BLOCKSIZE += next_block_ptr->BLOCKSIZE;

        /*
         * Reset the previous physical block pointer of
         * the block after the next block (ie the next next block)
         */
        prev_block_ptr = NEXT_PHYS(next_block_ptr);
        PREV_PHYS( prev_block_ptr) = block_ptr;
        CALC_CHECKSUM(prev_block_ptr);
        CALC_CHECKSUM(block_ptr);

        have_coalesced = TRUE;
    } /* Endif */

    if (have_coalesced) {

        /* Reset the _mem_test pointers */
        mem_pool_ptr->POOL_PHYSICAL_CHECK_BLOCK = (STOREBLOCK_STRUCT_PTR) mem_pool_ptr->POOL_PTR;
        mem_pool_ptr->POOL_FREE_CHECK_BLOCK = mem_pool_ptr->POOL_FREE_LIST_PTR;

#if MQX_MEMORY_FREE_LIST_SORTED == 1
        /*
         * Reset the freelist current block pointer in case we pre-empted
         * another task
         */
        mem_pool_ptr->POOL_FREE_CURRENT_BLOCK = mem_pool_ptr->POOL_FREE_LIST_PTR;
#endif
    } /* Endif */

    return (have_coalesced);

} /* Endbody */

/*!
 * \brief Allocates a block of memory.
 *
 * See Description of _mem_alloc() function.
 *
 * \param[in] pool_id Pool from which to allocate the memory block (from
 * _mem_create_pool()).
 * \param[in] size    Size of the memory block.
 *
 * \return Pointer to the memory block (Success.)
 * \return NULL (Failure: see task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following
 * task error codes:
 * \li MQX_CORRUPT_STORAGE_POOL_FREE_LIST (Memory pool freelist is corrupted.)
 * \li MQX_INVALID_CHECKSUM (Checksum of the current memory block header is
 * incorrect.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_INVALID_CONFIGURATION (User area not aligned on a cache line 
 * boundary.)
 *
 * \see _mem_alloc()
 * \see _mem_alloc_from()
 * \see _mem_alloc_system()
 * \see _mem_alloc_system_zero()
 * \see _mem_alloc_system_zero_from()
 * \see _mem_alloc_system_align() 
 * \see _mem_alloc_system_align_from() 
 * \see _mem_alloc_zero()
 * \see _mem_alloc_zero_from()
 * \see _mem_alloc_align()
 * \see _mem_alloc_align_from()
 * \see _mem_alloc_at()
 * \see _mem_realloc()
 * \see _mem_create_pool
 * \see _mem_free
 * \see _mem_get_highwater
 * \see _mem_get_highwater_pool
 * \see _mem_get_size
 * \see _mem_transfer
 * \see _mem_free_part
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error
 */
void *_mem_alloc_system_from
(
    _mem_pool_id pool_id,
    _mem_size size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    MEMPOOL_STRUCT_PTR mem_pool_ptr = (MEMPOOL_STRUCT_PTR) pool_id;
    _mqx_uint error;
    void   *result;

    _GET_KERNEL_DATA(kernel_data);

    _KLOGE2(KLOG_mem_alloc_system_from, size);

    _INT_DISABLE();
    result = _mem_alloc_internal(size, SYSTEM_TD_PTR(kernel_data), mem_pool_ptr, &error);

    /* update the memory allocation pointer in case a lower priority
     * task was preempted inside _mem_alloc_internal
     */
    mem_pool_ptr->POOL_ALLOC_CURRENT_BLOCK = mem_pool_ptr->POOL_FREE_LIST_PTR;

    _INT_ENABLE();

#if MQX_CHECK_ERRORS
    if (error != MQX_OK) {
        _task_set_error(error);
    } /* Endif */
#endif

    _KLOGX3(KLOG_mem_alloc_system_from, result, mem_pool_ptr->POOL_BLOCK_IN_ERROR);

    return (result);

} /* Endbody */

/*!
 * \brief Allocates a block of memory.
 *
 * See Description of _mem_alloc() function.
 *
 * \param[in] size Size of the memory block.
 *
 * \return Pointer to the memory block (Success.)
 * \return NULL (Failure: see task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following
 * task error codes:
 * \li MQX_CORRUPT_STORAGE_POOL_FREE_LIST (Memory pool freelist is corrupted.)
 * \li MQX_INVALID_CHECKSUM (Checksum of the current memory block header is
 * incorrect.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_INVALID_CONFIGURATION (User area not aligned on a cache line 
 * boundary.)
 *
 * \see _mem_alloc()
 * \see _mem_alloc_from()
 * \see _mem_alloc_system_from()
 * \see _mem_alloc_system_zero()
 * \see _mem_alloc_system_zero_from()
 * \see _mem_alloc_system_align() 
 * \see _mem_alloc_system_align_from() 
 * \see _mem_alloc_zero()
 * \see _mem_alloc_zero_from()
 * \see _mem_alloc_align()
 * \see _mem_alloc_align_from()
 * \see _mem_alloc_at()
 * \see _mem_realloc()
 * \see _mem_create_pool
 * \see _mem_free
 * \see _mem_get_highwater
 * \see _mem_get_highwater_pool
 * \see _mem_get_size
 * \see _mem_transfer
 * \see _mem_free_part
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error
 */
void *_mem_alloc_system
(
    _mem_size size
)
{
    KERNEL_DATA_STRUCT_PTR kernel_data;
    _mqx_uint error;
    void   *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_mem_alloc_system, size);

    _INT_DISABLE();
    result = _mem_alloc_internal(size, SYSTEM_TD_PTR(kernel_data), (MEMPOOL_STRUCT_PTR) kernel_data->KD_POOL, &error);

    /* update the memory allocation pointer in case a lower priority
     * task was preempted inside _mem_alloc_internal
     */
    kernel_data->KD_POOL->POOL_ALLOC_CURRENT_BLOCK = kernel_data->KD_POOL->POOL_FREE_LIST_PTR;

    _INT_ENABLE();

#if MQX_CHECK_ERRORS
    if (error != MQX_OK) {
        _task_set_error(error);
    } /* Endif */
#endif

    _KLOGX3(KLOG_mem_alloc_system, result, kernel_data->KD_POOL->POOL_BLOCK_IN_ERROR);

    return (result);
}
/*!
 * \brief Allocates a block of memory.
 *
 * See Description of _mem_alloc_align() function.
 *
 * \param[in] size Size of the memory block.
 * \param[in] align   Alignment of the memory block. 
 *
 * \return Pointer to the memory block (Success.)
 * \return NULL (Failure: see task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following
 * task error codes:
 * \li MQX_CORRUPT_STORAGE_POOL_FREE_LIST (Memory pool freelist is corrupted.)
 * \li MQX_INVALID_CHECKSUM (Checksum of the current memory block header is
 * incorrect.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_INVALID_CONFIGURATION (User area not aligned on a cache line 
 * boundary.)
 * \li MQX_INVALID_PARAMETER (Requested alignment is not power of 2.)
 *
 * \see _mem_alloc()
 * \see _mem_alloc_from()
 * \see _mem_alloc_system_from()
 * \see _mem_alloc_system_zero()
 * \see _mem_alloc_system_zero_from()
 * \see _mem_alloc_system_align_from() 
 * \see _mem_alloc_zero()
 * \see _mem_alloc_zero_from()
 * \see _mem_alloc_align()
 * \see _mem_alloc_align_from()
 * \see _mem_alloc_at()
 * \see _mem_realloc()
 * \see _mem_create_pool
 * \see _mem_free
 * \see _mem_get_highwater
 * \see _mem_get_highwater_pool
 * \see _mem_get_size
 * \see _mem_transfer
 * \see _mem_free_part
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error
 */
void *_mem_alloc_system_align
(
    _mem_size size,
    _mem_size align
)
{
    KERNEL_DATA_STRUCT_PTR kernel_data;
    _mqx_uint error;
    void   *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE3(KLOG_mem_alloc_system_align, size, align);

    _INT_DISABLE();
    result = _mem_alloc_internal_align(size, align, SYSTEM_TD_PTR(kernel_data), (MEMPOOL_STRUCT_PTR)
                    kernel_data->KD_POOL, &error);
    /* update the memory allocation pointer in case a lower priority
     * task was preempted inside _mem_alloc_internal
     */
    kernel_data->KD_POOL->POOL_ALLOC_CURRENT_BLOCK = kernel_data->KD_POOL->POOL_FREE_LIST_PTR;

    _INT_ENABLE();

#if MQX_CHECK_ERRORS
    if (error != MQX_OK) {
        _task_set_error(error);
    } /* Endif */
#endif

    _KLOGX3(KLOG_mem_alloc_system_align, result, kernel_data->KD_POOL->POOL_BLOCK_IN_ERROR);

    return (result);
}

/*!
 * \brief Allocates a block of memory.
 *
 * See Description of _mem_alloc() function.
 *
 * \param[in] pool_id Pool from which to allocate the memory block (from
 * _mem_create_pool()). 
 * \param[in] size Size of the memory block.
 * \param[in] align   Alignment of the memory block. 
 *
 * \return Pointer to the memory block (Success.)
 * \return NULL (Failure: see task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following
 * task error codes:
 * \li MQX_CORRUPT_STORAGE_POOL_FREE_LIST (Memory pool freelist is corrupted.)
 * \li MQX_INVALID_CHECKSUM (Checksum of the current memory block header is
 * incorrect.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_INVALID_CONFIGURATION (User area not aligned on a cache line 
 * boundary.)
 * \li MQX_INVALID_PARAMETER (Requested alignment is not power of 2.)
 *
 * \see _mem_alloc()
 * \see _mem_alloc_from()
 * \see _mem_alloc_system_from()
 * \see _mem_alloc_system_zero()
 * \see _mem_alloc_system_zero_from()
 * \see _mem_alloc_system_align()
 * \see _mem_alloc_zero()
 * \see _mem_alloc_zero_from()
 * \see _mem_alloc_align()
 * \see _mem_alloc_align_from()
 * \see _mem_alloc_at()
 * \see _mem_realloc()
 * \see _mem_create_pool
 * \see _mem_free
 * \see _mem_get_highwater
 * \see _mem_get_highwater_pool
 * \see _mem_get_size
 * \see _mem_transfer
 * \see _mem_free_part
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error
 */
void *_mem_alloc_system_align_from
(
    _mem_pool_id pool_id,
    _mem_size size,
    _mem_size align
)
{
    KERNEL_DATA_STRUCT_PTR kernel_data;
    MEMPOOL_STRUCT_PTR mem_pool_ptr;
    _mqx_uint error;
    void   *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE4(KLOG_mem_alloc_system_align_from, pool_id, size, align);

    mem_pool_ptr = (MEMPOOL_STRUCT_PTR) pool_id;

    _INT_DISABLE();
    
    result = _mem_alloc_internal_align(size, align, SYSTEM_TD_PTR(kernel_data), mem_pool_ptr, &error);

#if MQX_CHECK_ERRORS
    if (error != MQX_OK) {
        _task_set_error(error);
    } /* Endif */
#endif

    /* update the memory allocation pointer in case a lower priority
     * task was preempted inside _mem_alloc_internal
     */
    mem_pool_ptr->POOL_ALLOC_CURRENT_BLOCK = mem_pool_ptr->POOL_FREE_LIST_PTR;

    _INT_ENABLE();

    _KLOGX3(KLOG_mem_alloc_system_align_from, result, mem_pool_ptr->POOL_BLOCK_IN_ERROR);
    return (result);
    
}

#if MQX_USE_UNCACHED_MEM && PSP_HAS_DATA_CACHE

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
 * \li MQX_CORRUPT_STORAGE_POOL_FREE_LIST (Memory pool freelist is corrupted.)
 * \li MQX_INVALID_CHECKSUM (Checksum of the current memory block header is
 * incorrect.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_INVALID_CONFIGURATION (User area not aligned on a cache line 
 * boundary.)
 */
void *_mem_alloc_system_uncached
(
    _mem_size size
)
{
    KERNEL_DATA_STRUCT_PTR kernel_data;
    _mqx_uint error;
    void   *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_mem_alloc_system, size);

    _INT_DISABLE();
    result = _mem_alloc_internal(size, SYSTEM_TD_PTR(kernel_data), (MEMPOOL_STRUCT_PTR)kernel_data->UNCACHED_POOL, &error);

    /* update the memory allocation pointer in case a lower priority
     * task was preempted inside _mem_alloc_internal
     */
    kernel_data->UNCACHED_POOL->POOL_ALLOC_CURRENT_BLOCK = kernel_data->UNCACHED_POOL->POOL_FREE_LIST_PTR;

    _INT_ENABLE();

#if MQX_CHECK_ERRORS
    if (error != MQX_OK) {
        _task_set_error(error);
    } /* Endif */
#endif

    _KLOGX3(KLOG_mem_alloc_system, result, kernel_data->UNCACHED_POOL->POOL_BLOCK_IN_ERROR);

    return(result);
}

#endif /* MQX_USE_UNCACHED_MEM && PSP_HAS_DATA_CACHE */
/*!
 * \private
 *
 * \brief This function initializes the memory storage pool.
 *
 * \return MQX_OK
 * \return MQX_ERROR (Could not create system pool)
 */
_mqx_uint _mem_init_internal
(
    void
)
{ /* Body */
#if MQX_USE_UNCACHED_MEM && PSP_HAS_DATA_CACHE
    void   *__uncached_data_start = (void *)__UNCACHED_DATA_START;
    void   *__uncached_data_end   = (void *)__UNCACHED_DATA_END;
#endif /* MQX_USE_UNCACHED_MEM && PSP_HAS_DATA_CACHE */

    KERNEL_DATA_STRUCT_PTR kernel_data;
    void   *start;

    _GET_KERNEL_DATA(kernel_data);

    _QUEUE_INIT(&kernel_data->MEM_COMP.POOLS, 0);

    _lwsem_create((LWSEM_STRUCT_PTR) &kernel_data->MEM_COMP.SEM, 1);

    kernel_data->MEM_COMP.VALID = MEMPOOL_VALID;

    /*
     * Move the MQX memory pool pointer past the end of kernel data.
     */
    start = (void *) ((unsigned char *) kernel_data + sizeof(KERNEL_DATA_STRUCT));

    kernel_data->KD_POOL = (MEMPOOL_STRUCT_PTR)_mem_create_pool(start, (_mem_size)kernel_data->INIT.END_OF_KERNEL_MEMORY - (_mem_size)start);

#if MQX_USE_UNCACHED_MEM && PSP_HAS_DATA_CACHE
    if (kernel_data->KD_POOL != NULL) {
        if ((__uncached_data_start <=  start) && (start <= __uncached_data_end)) {
            kernel_data->UNCACHED_POOL = kernel_data->KD_POOL;
        }
        else {
            /* The pool state structure is created at the bottom of the pool */
            kernel_data->UNCACHED_POOL = (MEMPOOL_STRUCT_PTR)_mem_create_pool(__uncached_data_start, (_mem_size)__uncached_data_end - (_mem_size)__uncached_data_start);
        }
    }
#endif /* MQX_USE_UNCACHED_MEM && PSP_HAS_DATA_CACHE */
    if(kernel_data->KD_POOL != NULL)
    {
      return MQX_OK;
    }
    else
    {
      return MQX_ERROR;
    }
} /* Endbody */

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
void *_mem_get_next_block_internal
(
    TD_STRUCT_PTR td_ptr,
    void   *memory_ptr
)
{ /* Body */
    STOREBLOCK_STRUCT_PTR block_ptr;

    if (memory_ptr == NULL) {
        return (td_ptr->MEMORY_RESOURCE_LIST);
    }
    else {
        block_ptr = GET_MEMBLOCK_PTR(memory_ptr);
        return (block_ptr->NEXTBLOCK);
    } /* Endif */

} /* Endbody */

/*!
 * \brief Gets the allocated size (in bytes) of a block allocated using the MQX
 * storage allocator (_mem_alloc).
 *
 * The size is the actual size of the memory block and might be larger than the
 * size that a task requested.
 *
 * \param[in] mem_ptr Address of the memory block whose size is wanted.
 *
 * \return Number of single-addressable units in the block (Success.)
 * \return 0 (Failure.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following
 * task error codes:
 * \li MQX_CORRUPT_STORAGE_POOL (Memory is corrupted or mem_ptr does not point
 * to a block that was allocated with a function from the _mem_alloc family.)
 * \li MQX_INVALID_CHECKSUM (Checksum is not correct because part of the memory
 * block header was overwritten.)
 * \li MQX_INVALID_POINTER (Mem_ptr is NULL or improperly aligned.)
 *
 * \see _mem_free
 * \see _mem_alloc()
 * \see _mem_alloc_from()
 * \see _mem_alloc_system()
 * \see _mem_alloc_system_from()
 * \see _mem_alloc_system_zero()
 * \see _mem_alloc_system_zero_from()
 * \see _mem_alloc_system_align() 
 * \see _mem_alloc_system_align_from() 
 * \see _mem_alloc_zero()
 * \see _mem_alloc_zero_from()
 * \see _mem_alloc_align()
 * \see _mem_alloc_align_from()
 * \see _mem_alloc_at()
 * \see _mem_realloc()
 * \see _mem_free_part
 * \see _task_set_error
 */
_mem_size _mem_get_size
(
    void   *mem_ptr
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    STOREBLOCK_STRUCT_PTR block_ptr;
    _mem_size size;

    _GET_KERNEL_DATA(kernel_data);

#if MQX_CHECK_ERRORS
    if (mem_ptr == NULL) {
        _task_set_error(MQX_INVALID_POINTER);
        return (0);
    } /* Endif */
#endif

    /* Compute the start of the block  */
    block_ptr = GET_MEMBLOCK_PTR(mem_ptr);    
    
#if MQX_CHECK_ERRORS
    if (!_MEMORY_ALIGNED(block_ptr)) {
        _task_set_error(MQX_INVALID_POINTER);
        return (0);
    } /* Endif */
#endif

    _int_disable();
#if MQX_CHECK_VALIDITY
    if ((!VALID_CHECKSUM(block_ptr))) {
        _int_enable();
        kernel_data->KD_POOL->POOL_BLOCK_IN_ERROR = block_ptr;
        _task_set_error(MQX_INVALID_CHECKSUM);
        return (0);
    } /* Endif */
#endif

    size = block_ptr->BLOCKSIZE;

#if MQX_CHECK_ERRORS
    /* For all free blocks, a check is made to ensure that the user
     * has not corrupted the storage pool. This is done by checking the
     * 'magic value', which should not be corrupted. Alternatively, the
     * user could have passed in an invalid memory pointer.
     */
    if (BLOCK_IS_FREE(block_ptr)) {
        _int_enable();
        kernel_data->KD_POOL->POOL_BLOCK_IN_ERROR = block_ptr;
        _task_set_error(MQX_CORRUPT_STORAGE_POOL);
        return (0);
    } /* Endif */

#endif
    _int_enable();

    /* The size includes the block overhead, which the user is not
     * interested in. If the size is less than the overhead,
     * then there is a bad block or bad block pointer.
     */
#if MQX_CHECK_ERRORS
    if (size <= (_mem_size) FIELD_OFFSET(STOREBLOCK_STRUCT,USER_AREA)) {
        kernel_data->KD_POOL->POOL_BLOCK_IN_ERROR = block_ptr; 
        _task_set_error(MQX_CORRUPT_STORAGE_POOL);
        return (0);
    } /* Endif */
#endif    
    
    
    return (size - (_mem_size) FIELD_OFFSET(STOREBLOCK_STRUCT,USER_AREA));

} /* Endbody */

/*!
 * \brief This function checks the all memory pool for any errors.
 *
 * \param[out] pool_error_ptr Pointer to the memory pool in error (initialized
 * only if an error was found).
 *
 * \return MQX_OK (No errors found.)
 * \return Errors from _mem_test() (A memory pool has an error.)
 * \return Errors from _queue_test() (Memory-pool queue has an error.)
 *
 * \see _mem_test
 * \see _mem_test_pool
 * \see _queue_test
 */
_mqx_uint _mem_test_all
(
    _mem_pool_id  *pool_error_ptr
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    MEMPOOL_STRUCT_PTR pool_ptr;
    _mqx_uint result;

    _GET_KERNEL_DATA(kernel_data);

    /* Use a semaphore to protect the list of pools */
    _lwsem_wait((LWSEM_STRUCT_PTR) &kernel_data->MEM_COMP.SEM);

    /* Make sure that the queue of memory pools is ok */
    result = _queue_test((QUEUE_STRUCT_PTR) &kernel_data->MEM_COMP.POOLS, (void **) pool_error_ptr);

    _lwsem_post((LWSEM_STRUCT_PTR) &kernel_data->MEM_COMP.SEM);

    if (result != MQX_OK) {
        return (result);
    } /* Endif */

    /* Now test application pools */
    _lwsem_wait((LWSEM_STRUCT_PTR) &kernel_data->MEM_COMP.SEM);
    pool_ptr = (MEMPOOL_STRUCT_PTR)((void *) kernel_data->MEM_COMP.POOLS.NEXT);
    while (pool_ptr != (MEMPOOL_STRUCT_PTR)((void *) &kernel_data->MEM_COMP.POOLS)) {
        result = _mem_test_pool(pool_ptr);
        if (result != MQX_OK) {
            break;
        } /* Endif */
        pool_ptr = (MEMPOOL_STRUCT_PTR)((void *) pool_ptr->LINK.NEXT);
    } /* Endwhile */

    _lwsem_post((LWSEM_STRUCT_PTR) &kernel_data->MEM_COMP.SEM);
    
    if (result != MQX_OK) {
        *pool_error_ptr = (_mem_pool_id) pool_ptr;
    } else {
        *pool_error_ptr = NULL;
    }

    return (result);

} /* Endbody */

/*!
 * \brief Tests the memory in the memory pool.
 *
 * If _mem_test_pool() indicates an error, _mem_get_error_pool() indicates which
 * block has the error.
 *
 * \param[in] pool_id The pool to check.
 *
 * \return MQX_OK (No errors found.)
 * \return MQX_CORRUPT_STORAGE_POOL (A memory pool pointer is not correct.)
 * \return MQX_CORRUPT_STORAGE_POOL_FREE_LIST (Memory pool freelist is corrupted.)
 * \return MQX_INVALID_CHECKSUM (Checksum of the current memory block header is
 * incorrect (header is corrupted).)
 *
 * \see _mem_get_error_pool
 * \see _mem_test
 * \see _task_set_error
 */
_mqx_uint _mem_test_pool
(
    _mem_pool_id pool_id
)
{ /* Body */
    _KLOGM(KERNEL_DATA_STRUCT_PTR kernel_data);
    STOREBLOCK_STRUCT_PTR next_block_ptr;
    MEMPOOL_STRUCT_PTR mem_pool_ptr;
    MEMPOOL_EXTENSION_STRUCT_PTR ext_ptr;
    _mqx_uint result = MQX_OK;
    int i;
    unsigned char* end;

    _KLOGM(_GET_KERNEL_DATA(kernel_data));

    _KLOGE2(KLOG_mem_test_pool, pool_id);

    mem_pool_ptr = (MEMPOOL_STRUCT_PTR) pool_id;

    /* First check the physical blocks */
    mem_pool_ptr->POOL_PHYSICAL_CHECK_BLOCK = (STOREBLOCK_STRUCT_PTR) mem_pool_ptr->POOL_PTR;
    while (mem_pool_ptr->POOL_PHYSICAL_CHECK_BLOCK < mem_pool_ptr->POOL_END_PTR) {
        if ((!mem_pool_ptr->POOL_PHYSICAL_CHECK_BLOCK) || (!_MEMORY_ALIGNED(mem_pool_ptr->POOL_PHYSICAL_CHECK_BLOCK))) {
            mem_pool_ptr->POOL_BLOCK_IN_ERROR = (STOREBLOCK_STRUCT_PTR) mem_pool_ptr->POOL_PHYSICAL_CHECK_BLOCK;
            result = MQX_CORRUPT_STORAGE_POOL;
            break;
        } /* Endif */

        _int_disable();
        if (!VALID_CHECKSUM(mem_pool_ptr->POOL_PHYSICAL_CHECK_BLOCK)) {
            mem_pool_ptr->POOL_BLOCK_IN_ERROR = (STOREBLOCK_STRUCT_PTR) mem_pool_ptr->POOL_PHYSICAL_CHECK_BLOCK;
            _int_enable();
            result = MQX_INVALID_CHECKSUM;
            break;
        } /* Endif */

        next_block_ptr = NEXT_PHYS(mem_pool_ptr->POOL_PHYSICAL_CHECK_BLOCK);
        if (next_block_ptr->PREVBLOCK != mem_pool_ptr->POOL_PHYSICAL_CHECK_BLOCK) {
            mem_pool_ptr->POOL_BLOCK_IN_ERROR = next_block_ptr;
            _int_enable();
            result = MQX_CORRUPT_STORAGE_POOL;
            break;
        } /* Endif */
        mem_pool_ptr->POOL_PHYSICAL_CHECK_BLOCK = next_block_ptr;
        _int_enable();

    } /* Endwhile */


    /* Test blocks from this pool extensions */
    ext_ptr = (void *) mem_pool_ptr->EXT_LIST.NEXT;
    i = mem_pool_ptr->EXT_LIST.SIZE;
    while ((i--)&&(result == MQX_OK)) {
      mem_pool_ptr->POOL_PHYSICAL_CHECK_BLOCK = (STOREBLOCK_STRUCT_PTR) ext_ptr->REAL_START;
      end = (unsigned char *)_ALIGN_ADDR_TO_LOWER_MEM((unsigned char *)ext_ptr->START + ext_ptr->SIZE) - 
            (_mem_size)MQX_MIN_MEMORY_STORAGE_SIZE;
      while ( (void *)mem_pool_ptr->POOL_PHYSICAL_CHECK_BLOCK  < (void *) end) {
          if ((!mem_pool_ptr->POOL_PHYSICAL_CHECK_BLOCK) || (!_MEMORY_ALIGNED(mem_pool_ptr->POOL_PHYSICAL_CHECK_BLOCK))) {
              mem_pool_ptr->POOL_BLOCK_IN_ERROR = (STOREBLOCK_STRUCT_PTR) mem_pool_ptr->POOL_PHYSICAL_CHECK_BLOCK;
              result = MQX_CORRUPT_STORAGE_POOL;
              break;
          } /* Endif */

          _int_disable();
          if (!VALID_CHECKSUM(mem_pool_ptr->POOL_PHYSICAL_CHECK_BLOCK)) {
              mem_pool_ptr->POOL_BLOCK_IN_ERROR = (STOREBLOCK_STRUCT_PTR) mem_pool_ptr->POOL_PHYSICAL_CHECK_BLOCK;
              _int_enable();
              result = MQX_INVALID_CHECKSUM;
              break;
          } /* Endif */

          next_block_ptr = NEXT_PHYS(mem_pool_ptr->POOL_PHYSICAL_CHECK_BLOCK);
          if (next_block_ptr->PREVBLOCK != mem_pool_ptr->POOL_PHYSICAL_CHECK_BLOCK) {
              mem_pool_ptr->POOL_BLOCK_IN_ERROR = next_block_ptr;
              _int_enable();
              result = MQX_CORRUPT_STORAGE_POOL;
              break;
          } /* Endif */
          mem_pool_ptr->POOL_PHYSICAL_CHECK_BLOCK = next_block_ptr;
          _int_enable();

      } /* Endwhile */ 
      ext_ptr = (void *) ext_ptr->LINK.NEXT;
    }
    
    if (result != MQX_OK) {
        _KLOGX3(KLOG_mem_test_pool, result, mem_pool_ptr->POOL_BLOCK_IN_ERROR);
        return (result);
    } /* Endif */

    /* Now check the free list */
    _int_disable();
    if (mem_pool_ptr->POOL_FREE_LIST_PTR == NULL) { /* no free list to check */
        _int_enable();
        return MQX_OK;
    } /* Endif */

    mem_pool_ptr->POOL_FREE_CHECK_BLOCK = mem_pool_ptr->POOL_FREE_LIST_PTR;
    next_block_ptr = mem_pool_ptr->POOL_FREE_CHECK_BLOCK;
    if (next_block_ptr->USER_AREA != (void *) NULL) {
        _KLOGX3(KLOG_mem_test_pool, MQX_CORRUPT_STORAGE_POOL_FREE_LIST, next_block_ptr );
        mem_pool_ptr->POOL_BLOCK_IN_ERROR = next_block_ptr;
        _int_enable();
        return (MQX_CORRUPT_STORAGE_POOL_FREE_LIST);
    } /* Endif */

    _int_enable();

    while (mem_pool_ptr->POOL_FREE_CHECK_BLOCK < mem_pool_ptr->POOL_END_PTR) {
        if ((!mem_pool_ptr->POOL_FREE_CHECK_BLOCK) || (!_MEMORY_ALIGNED(mem_pool_ptr->POOL_FREE_CHECK_BLOCK))) {
            mem_pool_ptr->POOL_BLOCK_IN_ERROR = mem_pool_ptr->POOL_FREE_CHECK_BLOCK;
            result = MQX_CORRUPT_STORAGE_POOL_FREE_LIST;
            break;
        } /* Endif */

        _int_disable();
        if (!VALID_CHECKSUM(mem_pool_ptr->POOL_FREE_CHECK_BLOCK)) {
            mem_pool_ptr->POOL_BLOCK_IN_ERROR = mem_pool_ptr->POOL_FREE_CHECK_BLOCK;
            _int_enable();
            result = MQX_INVALID_CHECKSUM;
            break;
        } /* Endif */
        _int_enable();

        _int_disable();
        if (BLOCK_IS_USED(mem_pool_ptr->POOL_FREE_CHECK_BLOCK)) {
            /* An allocated block on the free list */
            mem_pool_ptr->POOL_BLOCK_IN_ERROR = mem_pool_ptr->POOL_FREE_CHECK_BLOCK;
            _int_enable();
            result = MQX_CORRUPT_STORAGE_POOL_FREE_LIST;
            break;
        } /* Endif */

        next_block_ptr = (STOREBLOCK_STRUCT_PTR) NEXT_FREE(mem_pool_ptr->POOL_FREE_CHECK_BLOCK);
        if (!next_block_ptr) {
            _int_enable(); /* If zero, free list has been completed */
            break;
        } /* Endif */
        if (next_block_ptr->USER_AREA != (char *) mem_pool_ptr->POOL_FREE_CHECK_BLOCK) {
            mem_pool_ptr->POOL_BLOCK_IN_ERROR = mem_pool_ptr->POOL_FREE_CHECK_BLOCK;
            _int_enable();
            result = MQX_CORRUPT_STORAGE_POOL_FREE_LIST;
            break;
        } /* Endif */
        mem_pool_ptr->POOL_FREE_CHECK_BLOCK = next_block_ptr;
        _int_enable();

    } /* Endwhile */

    if (result == MQX_OK) {
        _KLOGX2(KLOG_mem_test_pool, result);
    }
    else {
        _KLOGX3(KLOG_mem_test_pool, result, mem_pool_ptr->POOL_BLOCK_IN_ERROR);
    } /* Endif */
    return (result);

} /* Endbody */

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
 * \return MQX_CORRUPT_STORAGE_POOL (A memory pool pointer is not correct.)
 * \return MQX_CORRUPT_STORAGE_POOL_FREE_LIST (Memory pool freelist is corrupted.)
 * \return MQX_INVALID_CHECKSUM (Checksum of the current memory block header is
 * incorrect (header is corrupted).)
 *
 * \warning Can be called by only one task at a time (see Description).
 * \warning Disables and enables interrupts.
 *
 * \see _mem_alloc()
 * \see _mem_alloc_from()
 * \see _mem_alloc_system()
 * \see _mem_alloc_system_from()
 * \see _mem_alloc_system_zero()
 * \see _mem_alloc_system_zero_from()
 * \see _mem_alloc_system_align() 
 * \see _mem_alloc_system_align_from() 
 * \see _mem_alloc_zero()
 * \see _mem_alloc_zero_from()
 * \see _mem_alloc_align()
 * \see _mem_alloc_align_from()
 * \see _mem_alloc_at()
 * \see _mem_realloc()
 * \see _mem_get_error
 * \see _mem_test_pool
 */
_mqx_uint _mem_test
(
    void
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    _mqx_uint result;

    _GET_KERNEL_DATA(kernel_data);

    result = _mem_test_pool((_mem_pool_id) kernel_data->KD_POOL);

    return (result);

} /* Endbody */

/*!
 * \brief Gets the highest memory address that MQX has allocated in the default
 * memory pool.
 *
 * The function gets the highwater mark; that is, the highest memory address
 * ever allocated by MQX in the default memory pool. The mark does not decrease
 * if tasks free memory in the default memory pool.
 * \n If a task extends the default memory pool (_mem_extend()) with an area
 * above the highwater mark and MQX subsequently allocates memory from the
 * extended memory, the function returns an address from the extended memory.
 *
 * \return Highest address allocated in the default memory pool.
 *
 * \see _mem_alloc()
 * \see _mem_alloc_from()
 * \see _mem_alloc_system()
 * \see _mem_alloc_system_from()
 * \see _mem_alloc_system_zero()
 * \see _mem_alloc_system_zero_from()
 * \see _mem_alloc_system_align() 
 * \see _mem_alloc_system_align_from() 
 * \see _mem_alloc_zero()
 * \see _mem_alloc_zero_from()
 * \see _mem_alloc_align()
 * \see _mem_alloc_align_from()
 * \see _mem_alloc_at()
 * \see _mem_realloc()
 * \see _mem_extend
 * \see _mem_get_highwater_pool
 */
void *_mem_get_highwater
(
    void
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);

    return (kernel_data->KD_POOL->POOL_HIGHEST_MEMORY_USED);

} /* Endbody */

/*!
 * \brief Gets the highest memory address that MQX has allocated in the pool.
 *
 * The function gets the highwater mark; that is, the highest memory address
 * ever allocated in the memory pool. The mark does not decrease if tasks free
 * blocks in the pool.
 * \n If a task extends the memory pool (_mem_extend_pool()) with an area above
 * the highwater mark and MQX subsequently allocates memory from the extended
 * memory, the function returns an address from the extended memory.
 *
 * \param[in] pool_id Pool for which to get the highwater mark (from
 * _mem_create_pool()).
 *
 * \return Highest address allocated in the memory pool.
 *
 * \see _mem_alloc()
 * \see _mem_alloc_from()
 * \see _mem_alloc_system()
 * \see _mem_alloc_system_from()
 * \see _mem_alloc_system_zero()
 * \see _mem_alloc_system_zero_from()
 * \see _mem_alloc_system_align() 
 * \see _mem_alloc_system_align_from() 
 * \see _mem_alloc_zero()
 * \see _mem_alloc_zero_from()
 * \see _mem_alloc_align()
 * \see _mem_alloc_align_from()
 * \see _mem_alloc_at()
 * \see _mem_realloc()
 * \see _mem_create_pool
 * \see _mem_extend_pool
 * \see _mem_get_highwater
 */
void *_mem_get_highwater_pool
(
    _mem_pool_id pool_id
)
{ /* Body */
    MEMPOOL_STRUCT_PTR mem_pool_ptr = (MEMPOOL_STRUCT_PTR) pool_id;

    return (mem_pool_ptr->POOL_HIGHEST_MEMORY_USED);

} /* Endbody */

/*!
 * \brief Gets the last memory block which caused a corrupt memory pool error in
 * kernel data.
 *
 * If _mem_test() indicates an error in the default memory pool, _mem_get_error()
 * indicates which block has the error.
 * \n In each memory block header, MQX maintains internal information, including
 * a checksum of the information. As tasks call functions from the _mem family,
 * MQX recalculates the checksum and compares it with the original. If the
 * checksums do not match, MQX marks the block as corrupted.
 * \n A block will be corrupted if:
 * \n - A task writes past the end of an allocated memory block and into the
 * header information in the next block. This can occur if:
 * \li The task allocated a block smaller than it needed.
 * \li A task overflows its stack.
 * \li A pointer is out of range.
 *
 * \n - A task randomly overwrites memory in the default memory pool.
 *
 * \return Pointer to the memory block that is corrupted.
 *
 * \see _mem_test
 */
void *_mem_get_error
(
    void
)
{ /* Body */
    register KERNEL_DATA_STRUCT_PTR kernel_data;
    register void   *user_ptr;

    _GET_KERNEL_DATA(kernel_data);

    user_ptr = (void *) ((unsigned char *) kernel_data->KD_POOL->POOL_BLOCK_IN_ERROR
                    + FIELD_OFFSET(STOREBLOCK_STRUCT,USER_AREA));
    return (user_ptr);

} /* Endbody */

/*!
 * \brief Gets the last memory block which caused a corrupt memory pool error in
 * the specified pool.
 *
 * \param[in] pool_id Memory pool from which to get the block.
 *
 * \return Pointer to the memory block.
 *
 * \see _mem_test_pool
 */
void *_mem_get_error_pool
(
    _mem_pool_id pool_id
)
{ /* Body */
    MEMPOOL_STRUCT_PTR mem_pool_ptr = (MEMPOOL_STRUCT_PTR) pool_id;
    register void   *user_ptr;

    user_ptr = (void *) ((unsigned char *) mem_pool_ptr->POOL_BLOCK_IN_ERROR + FIELD_OFFSET(STOREBLOCK_STRUCT,USER_AREA));
    return (user_ptr);

} /* Endbody */

/*!
 * \brief Sets the value of the default memory pool.
 *
 * Because MQX allocates memory blocks from the default
 * memory pool when an application calls _mem_alloc(), _mem_alloc_system(),
 * _mem_alloc_system_zero() or _mem_alloc_zero(), the application must
 * call _mem_set_default_pool() to change the default pool if needed.
 *
 * \param[in] pool_id New pool ID.
 *
 * \return Previous pool ID.
*/
_mem_pool_id _mem_set_default_pool
(
    _mem_pool_id pool_id
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    _mem_pool_id         old_pool_id;

    _GET_KERNEL_DATA(kernel_data);

    old_pool_id = (_mem_pool_id)kernel_data->KD_POOL;
    kernel_data->KD_POOL = pool_id;
    return (old_pool_id);

} /* Endbody */

/*!
 * \brief Gets the pool ID of the system pool.
 *
 * \return system pool ID.
 */
_mem_pool_id _mem_get_system_pool_id
(
    void
)
{
    register KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);

    return (_mem_pool_id) kernel_data->KD_POOL;
}

#if MQX_ALLOW_TYPED_MEMORY
/*!
 * \brief Gets the type of the specified block.
 *
 * \param[in] mem_ptr Address of the memory block to get type of.
 *
 * \return Pointer to the memory block type.
 */
_mem_type _mem_get_type
(
    void   *mem_ptr
)
{
    STOREBLOCK_STRUCT_PTR block_ptr;

    block_ptr = GET_MEMBLOCK_PTR(mem_ptr);
    return block_ptr->MEM_TYPE;
}

/*!
 * \brief Sets type of the specified block.
 *
 * \param[in] mem_ptr  Address of the memory block to set type.
 * \param[in] mem_type Memory type to set.
 *
 * \return TRUE (Type has been set.), FALSE (Type has not been set.).
 */
bool _mem_set_type
(
    void     *mem_ptr,
    _mem_type mem_type
)
{
    STOREBLOCK_STRUCT_PTR block_ptr;

    if (mem_ptr != NULL) {
        block_ptr = GET_MEMBLOCK_PTR(mem_ptr);
        _int_disable();
        block_ptr->MEM_TYPE = mem_type;
        CALC_CHECKSUM(block_ptr);
        _int_enable();
        return TRUE;
    }
    else {
        return FALSE;
    }
}

#endif /* MQX_ALLOW_TYPED_MEMORY */

/*!
 * \brief Transfers the ownership of a block of memory from an owner task to
 * another task.
 *
 * \param[in] memory_ptr A memory block whose ownership is to be transferred.
 * \param[in] source_id  Task ID of the current owner.
 * \param[in] target_id  Task ID of the new owner.
 *
 * \return MQX_OK
 * \return MQX_INVALID_CHECKSUM (Block's checksum is not correct, indicating
 * that at least some of the block was overwritten.)
 * \return MQX_INVALID_POINTER (Block_ptr is NULL or misaligned.)
 * \return MQX_INVALID_TASK_ID (Source or target does not represent a valid task.)
 * \return MQX_NOT_RESOURCE_OWNER (Memory block is not a resource of the task
 * represented by source.)
 *
 * \warning On failure, calls _task_set_error() to set the error code (see Return).
 *
 * \see _mem_alloc()
 * \see _mem_alloc_from()
 * \see _mem_alloc_system()
 * \see _mem_alloc_system_from()
 * \see _mem_alloc_system_zero()
 * \see _mem_alloc_system_zero_from()
 * \see _mem_alloc_system_align() 
 * \see _mem_alloc_system_align_from() 
 * \see _mem_alloc_zero()
 * \see _mem_alloc_zero_from()
 * \see _mem_alloc_align()
 * \see _mem_alloc_align_from()
 * \see _mem_alloc_at()
 * \see _mem_realloc()
 * \see _mqx_get_system_task_id
 * \see _task_set_error
 */
_mqx_uint _mem_transfer
(
    void    *memory_ptr,
    _task_id source_id,
    _task_id target_id
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    STOREBLOCK_STRUCT_PTR block_ptr;
    TD_STRUCT_PTR source_td;
    TD_STRUCT_PTR target_td;
    _mqx_uint result;

    _GET_KERNEL_DATA(kernel_data);

    _KLOGE4(KLOG_mem_transfer, memory_ptr, source_id, target_id);

#if MQX_CHECK_ERRORS
    if (memory_ptr == NULL) {
        _task_set_error(MQX_INVALID_POINTER);
        _KLOGX2(KLOG_mem_transfer, MQX_INVALID_POINTER);
        return (MQX_INVALID_POINTER);
    } /* Endif */
#endif

    /* Verify the block */
    block_ptr = GET_MEMBLOCK_PTR(memory_ptr);

#if MQX_CHECK_ERRORS
    if (!_MEMORY_ALIGNED(block_ptr)) {
        _task_set_error(MQX_INVALID_POINTER);
        _KLOGX2(KLOG_mem_transfer, MQX_INVALID_POINTER);
        return (MQX_INVALID_POINTER);
    } /* Endif */
#endif

#if MQX_CHECK_VALIDITY
    if (!VALID_CHECKSUM(block_ptr)) {
        _task_set_error(MQX_INVALID_CHECKSUM);
        kernel_data->KD_POOL->POOL_BLOCK_IN_ERROR = block_ptr;
        _KLOGX2(KLOG_mem_transfer, MQX_INVALID_CHECKSUM);
        return (MQX_INVALID_CHECKSUM);
    } /* Endif */
#endif

    source_td = (TD_STRUCT_PTR) _task_get_td(source_id);
    target_td = (TD_STRUCT_PTR) _task_get_td(target_id);

#if MQX_CHECK_ERRORS
    if ((source_td == NULL) || (target_td == NULL)) {
        _task_set_error(MQX_INVALID_TASK_ID);
        _KLOGX2(KLOG_mem_transfer, MQX_INVALID_TASK_ID);
        return (MQX_INVALID_TASK_ID);
    } /* Endif */
#endif

#if MQX_CHECK_ERRORS
    if (block_ptr->TASK_NUMBER != TASK_NUMBER_FROM_TASKID(source_id))
    {
        _task_set_error(MQX_NOT_RESOURCE_OWNER);
        return (MQX_NOT_RESOURCE_OWNER);
    } /* Endif */
#endif

    _INT_DISABLE();

    result = _mem_transfer_td_internal(memory_ptr, source_td, target_td);
    if (result != MQX_OK) _task_set_error(result);

    _INT_ENABLE();

    _KLOGX2(KLOG_mem_transfer, result);
    return (result);

} /* Endbody */

/*!
 * \private
 *
 * \brief Transfers the ownership of a block of memory from an owner task to
 * another task.
 *
 * \param[in] memory_ptr Address of the USER_AREA in the memory block to transfer.
 * \param[in] target_td  Target task descriptor.
 */
void _mem_transfer_internal
(
    void         *memory_ptr,
    TD_STRUCT_PTR target_td
)
{ /* Body */
    STOREBLOCK_STRUCT_PTR block_ptr;

    /* Verify the block */
    block_ptr = GET_MEMBLOCK_PTR(memory_ptr);
    
    /* Transfering already owned block causes linked list loop creation!
       If the target task already owns the memory block, do nothing. */
    if(block_ptr->TASK_NUMBER != TASK_NUMBER_FROM_TASKID(target_td->TASK_ID))
    {
        /* Link the block onto the target's task descriptor. */
        block_ptr->NEXTBLOCK = target_td->MEMORY_RESOURCE_LIST;
        target_td->MEMORY_RESOURCE_LIST = (char *) (&block_ptr->USER_AREA);

        block_ptr->TASK_NUMBER = TASK_NUMBER_FROM_TASKID(target_td->TASK_ID);
        CALC_CHECKSUM(block_ptr);
    }
    
} /* Endbody */

/*!
 * \private
 *
 * \brief Transfers the ownership of a block of memory from an owner task to
 * another task.
 *
 * \param[in] memory_ptr A memory block whose ownership is to be transferred.
 * \param[in] source_td  Task ID of the current owner.
 * \param[in] target_td  Task ID of the new owner.
 *
 * \return MQX_OK
 * \return MQX_NOT_RESOURCE_OWNER
 */
_mqx_uint _mem_transfer_td_internal
(
    void         *memory_ptr,
    TD_STRUCT_PTR source_td,
    TD_STRUCT_PTR target_td
)
{ /* Body */
    STOREBLOCK_STRUCT_PTR block_ptr;
    STOREBLOCK_STRUCT_PTR prev_block_ptr = NULL; /* Initialize to supress compiler warning */
    STOREBLOCK_STRUCT_PTR next_block_ptr;

    block_ptr = GET_MEMBLOCK_PTR(memory_ptr);
    
     /* Transfering already owned block causes linked list loop creation!
       If the target task already owns the memory block, do nothing. */
    if(block_ptr->TASK_NUMBER == TASK_NUMBER_FROM_TASKID(target_td->TASK_ID))
    {
       /* Do nothing, target already owns the block */
       return (MQX_OK);
    }

    next_block_ptr = (STOREBLOCK_STRUCT_PTR) source_td->MEMORY_RESOURCE_LIST;

    /* prev_block_ptr = GET_MEMBLOCK_PTR(&source_td->MEMORY_RESOURCE_LIST); */
    if (memory_ptr == next_block_ptr) {
        /*
         * This is the last item on the resource list of the
         * source task. Remove it from the resource list by
         * pointing the resource list to the next item.
         */
        source_td->MEMORY_RESOURCE_LIST = block_ptr->NEXTBLOCK;
    }
    else {
        /* Scan the task's memory resource list searching for the block.
         * Stop when the current pointer is equal to the block,
         * or the end of the list is reached.
         */
        while (next_block_ptr && ((void *) next_block_ptr != memory_ptr)) {
            prev_block_ptr = GET_MEMBLOCK_PTR(next_block_ptr);
            next_block_ptr = (STOREBLOCK_STRUCT_PTR) prev_block_ptr->NEXTBLOCK;
        } /* Endwhile */

        if (!next_block_ptr) {
            /* The specified block does not belong to the source task. */
            return (MQX_NOT_RESOURCE_OWNER);
        } /* Endif */

        /* Remove the memory block from the resource list of the
         * source task.
         */
        prev_block_ptr->NEXTBLOCK = block_ptr->NEXTBLOCK;
    } /* Endif */

    _mem_transfer_internal(memory_ptr, target_td);

    return (MQX_OK);

} /* Endbody */

/*!
 * \brief Allocates a block of memory.
 *
 * See Description of _mem_alloc() function.
 *
 * \param[in] pool_id Pool from which to allocate the memory block (from
 * _mem_create_pool()).
 * \param[in] size Size of the memory block.
 *
 * \return Pointer to the memory block (Success.)
 * \return NULL (Failure: see task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following
 * task error codes:
 * \li MQX_CORRUPT_STORAGE_POOL_FREE_LIST (Memory pool freelist is corrupted.)
 * \li MQX_INVALID_CHECKSUM (Checksum of the current memory block header is
 * incorrect.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_INVALID_CONFIGURATION (User area not aligned on a cache line 
 * boundary.)
 *
 * \see _mem_alloc()
 * \see _mem_alloc_from()
 * \see _mem_alloc_system()
 * \see _mem_alloc_system_from()
 * \see _mem_alloc_system_zero()
 * \see _mem_alloc_system_align() 
 * \see _mem_alloc_system_align_from() 
 * \see _mem_alloc_zero()
 * \see _mem_alloc_zero_form()
 * \see _mem_alloc_align()
 * \see _mem_alloc_align_from()
 * \see _mem_alloc_at()
 * \see _mem_realloc()
 * \see _mem_create_pool
 * \see _mem_free
 * \see _mem_get_highwater
 * \see _mem_get_highwater_pool
 * \see _mem_get_size
 * \see _mem_transfer
 * \see _mem_free_part
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error
 */
void *_mem_alloc_system_zero_from
(
    _mem_pool_id pool_id,
    _mem_size size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    MEMPOOL_STRUCT_PTR mem_pool_ptr;
    void   *result;
    _mqx_uint error;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_mem_alloc_system_zero_from, size);

    mem_pool_ptr = (MEMPOOL_STRUCT_PTR) pool_id;

    _INT_DISABLE();
    result = _mem_alloc_internal(size, SYSTEM_TD_PTR(kernel_data), mem_pool_ptr, &error);

    /* update the memory allocation pointer in case a lower priority
     * task was preempted inside _mem_alloc_internal
     */
    mem_pool_ptr->POOL_ALLOC_CURRENT_BLOCK = mem_pool_ptr->POOL_FREE_LIST_PTR;

    _INT_ENABLE();

#if MQX_CHECK_ERRORS
    if (error != MQX_OK) {
        _task_set_error(error);
    } /* Endif */
#endif

    if (result != NULL) {
        _mem_zero(result, size);
    } /* Endif */

    _KLOGX3(KLOG_mem_alloc_system_zero_from, result, mem_pool_ptr->POOL_BLOCK_IN_ERROR);

    return (result);

} /* Endbody */

/*!
 * \brief Allocates a block of memory.
 *
 * See Description of _mem_alloc() function.
 *
 * \param[in] size Size of the memory block.
 *
 * \return Pointer to the memory block (Success.)
 * \return NULL (Failure: see task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following
 * task error codes:
 * \li MQX_CORRUPT_STORAGE_POOL_FREE_LIST (Memory pool freelist is corrupted.)
 * \li MQX_INVALID_CHECKSUM (Checksum of the current memory block header is
 * incorrect.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_INVALID_CONFIGURATION (User area not aligned on a cache line 
 * boundary.)
 *
 * \see _mem_alloc()
 * \see _mem_alloc_from()
 * \see _mem_alloc_system()
 * \see _mem_alloc_system_from()
 * \see _mem_alloc_system_zero_from()
 * \see _mem_alloc_system_align() 
 * \see _mem_alloc_system_align_from() 
 * \see _mem_alloc_zero()
 * \see _mem_alloc_zero_form()
 * \see _mem_alloc_align()
 * \see _mem_alloc_align_from()
 * \see _mem_alloc_at()
 * \see _mem_realloc()
 * \see _mem_create_pool
 * \see _mem_free
 * \see _mem_get_highwater
 * \see _mem_get_highwater_pool
 * \see _mem_get_size
 * \see _mem_transfer
 * \see _mem_free_part
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error
 */
void *_mem_alloc_system_zero
(
    _mem_size size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void   *result;
    _mqx_uint error;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_mem_alloc_system_zero, size);

    _INT_DISABLE();
    result = _mem_alloc_internal(size, SYSTEM_TD_PTR(kernel_data), (MEMPOOL_STRUCT_PTR) kernel_data->KD_POOL, &error);

    /* update the memory allocation pointer in case a lower priority
     * task was preempted inside _mem_alloc_internal
     */
    kernel_data->KD_POOL->POOL_ALLOC_CURRENT_BLOCK = kernel_data->KD_POOL->POOL_FREE_LIST_PTR;

    _INT_ENABLE();

#if MQX_CHECK_ERRORS
    if (error != MQX_OK) {
        _task_set_error(error);
    } /* Endif */
#endif

    if (result != NULL) {
        _mem_zero(result, size);
    } /* Endif */

    _KLOGX3(KLOG_mem_alloc_system_zero, result, kernel_data->KD_POOL->POOL_BLOCK_IN_ERROR);
    return (result);

}

#if MQX_USE_UNCACHED_MEM && PSP_HAS_DATA_CACHE

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
 * \li MQX_CORRUPT_STORAGE_POOL_FREE_LIST (Memory pool freelist is corrupted.)
 * \li MQX_INVALID_CHECKSUM (Checksum of the current memory block header is
 * incorrect.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_INVALID_CONFIGURATION (User area not aligned on a cache line 
 * boundary.)
 */
void *_mem_alloc_system_zero_uncached
(
    _mem_size size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void   *result;
    _mqx_uint error;

    _GET_KERNEL_DATA(kernel_data);
    /*_KLOGE2(KLOG_mem_alloc_system_zero_uncached, size);*/

    _INT_DISABLE();
    result = _mem_alloc_internal(size, SYSTEM_TD_PTR(kernel_data), (MEMPOOL_STRUCT_PTR)kernel_data->UNCACHED_POOL, &error);

    /* update the memory allocation pointer in case a lower priority
     * task was preempted inside _mem_alloc_internal
     */
    kernel_data->UNCACHED_POOL->POOL_ALLOC_CURRENT_BLOCK = kernel_data->UNCACHED_POOL->POOL_FREE_LIST_PTR;

    _INT_ENABLE();

#if MQX_CHECK_ERRORS
    if (error != MQX_OK) {
        _task_set_error(error);
    } /* Endif */
#endif

    if (result != NULL) {
        _mem_zero(result, size);
    } /* Endif */

    /*_KLOGX3(KLOG_mem_alloc_system_zero_uncached, result, kernel_data->UNCACHED_POOL.POOL_BLOCK_IN_ERROR);*/
    return(result);

}

#endif /* MQX_USE_UNCACHED_MEM && PSP_HAS_DATA_CACHE */

/*!
 * \private
 *
 * \brief Internal function for memory allocation.
 *
 * This function must be called disabled.
 * \n A pointer to memory that can be used is returned. _mem_alloc_internal
 * walks down the free list, looking for a block big enough to satisfy the
 * request. If the found block is bigger than needed, then a new free block is
 * created from the remnants, and added into the free list.
 * \n A global memory pointer is used, this allows higher priority tasks to
 * pre-empt the current task in _mem_alloc_internal. Care is taken to make sure
 * that when the higher priority task is completed, the memory pool search
 * pointer is reset to the beginning of the free list.
 * \n Since the memory test function may be running concurrently with the
 * allocator, the memory test pointers are reset.
 *
 * \param[in]  requested_size Size of the memory block.
 * \param[in]  td_ptr         Owner's TD.
 * \param[in]  mem_pool_ptr   Which pool to allocate from.
 * \param[out] error_ptr      Error code for operation:
 * \li MQX_OK
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_CORRUPT_STORAGE_POOL_FREE_LIST (Memory pool freelist is corrupted.)
 * \li MQX_INVALID_CHECKSUM (Checksum of the current memory block header is
 * incorrect.)
 * \li MQX_INVALID_CONFIGURATION (User area not aligned on a cache line 
 * boundary.)
 *
 * \return Pointer to the memory block (success)
 * \return NULL (Failure: see task error codes.)
 * \return (void *) NULL (Failure: see task error codes.)
 */
void *_mem_alloc_internal
(
    _mem_size          requested_size,
    TD_STRUCT_PTR      td_ptr,
    MEMPOOL_STRUCT_PTR mem_pool_ptr,
    _mqx_uint_ptr      error_ptr
)
{ /* Body */
    STOREBLOCK_STRUCT_PTR block_ptr;
    STOREBLOCK_STRUCT_PTR next_block_ptr;
    STOREBLOCK_STRUCT_PTR next_next_block_ptr;
    _mem_size block_size;
    _mem_size next_block_size;

    *error_ptr = MQX_OK;

    /*
     * Adjust message size to allow for block management overhead
     * and force size to be aligned.
     */
    requested_size += (_mem_size) (FIELD_OFFSET(STOREBLOCK_STRUCT,USER_AREA));
#if MQX_CHECK_ERRORS
    if (requested_size < MQX_MIN_MEMORY_STORAGE_SIZE) {
        requested_size = MQX_MIN_MEMORY_STORAGE_SIZE;
    } /* Endif */
#endif

    _MEMORY_ALIGN_VAL_LARGER(requested_size);

    block_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR;

    while (TRUE) {

        /*
         * Save the current block pointer.
         * We will be enabling access to higher priority tasks.
         * A higher priority task may pre-empt the current task
         * and may do a memory allocation.  If this is true,
         * the higher priority task will reset the POOL_ALLOC_CURRENT_BLOCK
         * upon exit, and the current task will start the search over again.
         */
        mem_pool_ptr->POOL_ALLOC_CURRENT_BLOCK = block_ptr;

        /* allow pending interrupts */
        _int_enable();
        _int_disable();

        /* Reset to the start if we were preempted 
           and the preempting task did finish its free/malloc operation */
        if(mem_pool_ptr->POOL_ALLOC_CURRENT_BLOCK != block_ptr){
            block_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR;
        }

        if (block_ptr == NULL) { /* Null pointer */
            mem_pool_ptr->POOL_BLOCK_IN_ERROR = block_ptr;
            *error_ptr = MQX_OUT_OF_MEMORY;
            return (NULL); /* request failed */
        } /* Endif */

#if MQX_CHECK_VALIDITY
        if (!_MEMORY_ALIGNED(block_ptr) || BLOCK_IS_USED(block_ptr)) {
            mem_pool_ptr->POOL_BLOCK_IN_ERROR = block_ptr;
            *error_ptr = MQX_CORRUPT_STORAGE_POOL_FREE_LIST;
            return ((void *) NULL);
        } /* Endif */
#endif

#if MQX_CHECK_VALIDITY
        if (!VALID_CHECKSUM(block_ptr)) {
            mem_pool_ptr->POOL_BLOCK_IN_ERROR = block_ptr;
            *error_ptr = MQX_INVALID_CHECKSUM;
            return ((void *) NULL);
        } /* Endif */
#endif

        block_size = block_ptr->BLOCKSIZE;

        if (block_size >= requested_size) {
            /* request fits into this block */

#if MQX_CHECK_VALIDITY
            /* Check that user area is aligned on a cache line boundary */
            if (!_MEMORY_ALIGNED(&block_ptr->USER_AREA)) {
                *error_ptr = MQX_INVALID_CONFIGURATION;
                return ((void *) NULL);
            } /* Endif */
#endif

            next_block_size = block_size - requested_size;
            if (next_block_size >= (2 * MQX_MIN_MEMORY_STORAGE_SIZE)) {
                /*
                 * The current block is big enough to split.
                 * into 2 blocks.... the part to be allocated is one block,
                 * and the rest remains as a free block on the free list.
                 */

                next_block_ptr = (STOREBLOCK_STRUCT_PTR)((char *) block_ptr + requested_size);

                /* Initialize the new physical block values */
                next_block_ptr->BLOCKSIZE = next_block_size;
                next_block_ptr->PREVBLOCK = (STOREBLOCK_STRUCT_PTR) block_ptr;
                MARK_BLOCK_AS_FREE(next_block_ptr);

                CALC_CHECKSUM(next_block_ptr);

                /* Link new block into the free list */
                next_block_ptr->NEXTBLOCK = block_ptr->NEXTBLOCK;
                block_ptr->NEXTBLOCK = (void *) next_block_ptr;

                next_block_ptr->USER_AREA = (void *) block_ptr;
                if (next_block_ptr->NEXTBLOCK != NULL) {
                    ((STOREBLOCK_STRUCT_PTR) next_block_ptr->NEXTBLOCK)-> USER_AREA = (void *) next_block_ptr;
                } /* Endif */

                /*
                 * Modify the block on the other side of the next block
                 * (the next next block) so that it's previous block pointer
                 * correctly point to the next block.
                 */
                next_next_block_ptr = (STOREBLOCK_STRUCT_PTR) NEXT_PHYS(next_block_ptr);
                PREV_PHYS( next_next_block_ptr) = next_block_ptr;
                CALC_CHECKSUM(next_next_block_ptr);

            }
            else {

                /* Take the entire block */
                requested_size = block_size;

            } /* Endif */

            /* Set the size of the block */
            block_ptr->BLOCKSIZE = requested_size;

            /* Indicate the block is in use */
            MARK_BLOCK_AS_USED(block_ptr, td_ptr->TASK_ID);
            block_ptr->MEM_TYPE = 0;

            CALC_CHECKSUM(block_ptr);

            /* Unlink the block from the free list */
            if (block_ptr == mem_pool_ptr->POOL_FREE_LIST_PTR) {
                /* At the head of the free list */

                mem_pool_ptr->POOL_FREE_LIST_PTR = (STOREBLOCK_STRUCT_PTR) NEXT_FREE(block_ptr);
                if (mem_pool_ptr->POOL_FREE_LIST_PTR != NULL) {
                    PREV_FREE(mem_pool_ptr->POOL_FREE_LIST_PTR) = 0;
                } /* Endif */

            }
            else {

                /*
                 * NOTE: PREV_FREE guaranteed to be non-zero
                 * Have to make the PREV_FREE of this block
                 * point to the NEXT_FREE of this block
                 */
                NEXT_FREE( PREV_FREE(
                    block_ptr)) = NEXT_FREE(block_ptr);
                if (NEXT_FREE(block_ptr) != NULL) {
                    /*
                     * Now have to make the NEXT_FREE of this block
                     * point to the PREV_FREE of this block
                     */
                    PREV_FREE( NEXT_FREE(
                        block_ptr)) = PREV_FREE(block_ptr);
                } /* Endif */

            } /* Endif */

#if MQX_MEMORY_FREE_LIST_SORTED == 1
            if (block_ptr == mem_pool_ptr->POOL_FREE_CURRENT_BLOCK) {
                /* Reset the freelist insertion sort by _mem_free */
                mem_pool_ptr->POOL_FREE_CURRENT_BLOCK = mem_pool_ptr->POOL_FREE_LIST_PTR;
            } /* Endif */
#endif

            /* Reset the __mem_test freelist pointer */
            mem_pool_ptr->POOL_FREE_CHECK_BLOCK = mem_pool_ptr->POOL_FREE_LIST_PTR;

            /*
             * Set the curent pool block to the start of the free list, so
             * that if this task pre-empted another that was performing a
             * _mem_alloc, the other task will restart it's search for a block
             */
            mem_pool_ptr->POOL_ALLOC_CURRENT_BLOCK = mem_pool_ptr->POOL_FREE_LIST_PTR;

            /* Remember some statistics - only for blocks which fall into the
             * main pool area. Ignore blocks in the extended areas (EXT_LIST)
             */
            next_block_ptr = NEXT_PHYS(block_ptr);
            if (((char *)(next_block_ptr) > (char *) mem_pool_ptr->POOL_HIGHEST_MEMORY_USED) &&
                ((char *)(next_block_ptr) > (char *) mem_pool_ptr->POOL_PTR) &&
                ((char *)(next_block_ptr) < (char *) mem_pool_ptr->POOL_END_PTR)
                )
            {
               mem_pool_ptr->POOL_HIGHEST_MEMORY_USED = ((char *)(next_block_ptr) - 1);
            } /* Endif */

            /* Link the block onto the task descriptor. */
            block_ptr->NEXTBLOCK = td_ptr->MEMORY_RESOURCE_LIST;
            td_ptr->MEMORY_RESOURCE_LIST = (void *) (&block_ptr->USER_AREA);

            block_ptr->MEM_POOL_PTR = (void *) mem_pool_ptr;

#if MQX_MEM_MONITOR
            do
            {
                KERNEL_DATA_STRUCT_PTR kernel_data;
                _GET_KERNEL_DATA(kernel_data);
                kernel_data->MEM_USED += block_ptr->BLOCKSIZE;
                if (kernel_data->MEM_USED > kernel_data->MEM_HIGHEST_USED)
                    kernel_data->MEM_HIGHEST_USED = kernel_data->MEM_USED;
            } while(0);
#endif

            return ((void *) (&block_ptr->USER_AREA));

        }
        else {
            block_ptr = (STOREBLOCK_STRUCT_PTR) NEXT_FREE(block_ptr);
        } /* Endif */

    } /* Endwhile */

#ifdef lint
    return( NULL ); /* to satisfy lint */
#endif

} /* Endbody */

/*!
 * \private
 *
 * \brief Internal function for memory allocation at specified position.
 *
 * \param[in]  requested_size Size of the memory block.
 * \param[in]  requested_addr Address of the memory block to allocate.
 * \param[in]  td_ptr         Owner TD.
 * \param[in]  mem_pool_ptr   Which pool to allocate from.
 * \param[out] error_ptr      Error code for operation:
 * \li MQX_OK
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_CORRUPT_STORAGE_POOL_FREE_LIST (Memory pool freelist is corrupted.)
 * \li MQX_INVALID_CHECKSUM (Checksum of the current memory block header is
 * incorrect.)
 * \li MQX_INVALID_CONFIGURATION (User area not aligned on a cache line 
 * boundary.)
 *
 * \return Pointer to the memory block (success)
 * \return NULL (Failure: see task error codes.)
 * \return (void *) NULL (Failure: see task error codes.)
 */
void *_mem_alloc_at_internal
(
    _mem_size          requested_size,
    void              *requested_addr,
    TD_STRUCT_PTR      td_ptr,
    MEMPOOL_STRUCT_PTR mem_pool_ptr,
    _mqx_uint_ptr      error_ptr
)
{ /* Body */
    STOREBLOCK_STRUCT_PTR block_ptr;
	STOREBLOCK_STRUCT_PTR free_block_ptr;
    STOREBLOCK_STRUCT_PTR next_block_ptr;
    STOREBLOCK_STRUCT_PTR next_next_block_ptr;
    _mem_size block_size, next_block_size, free_block_size;

    *error_ptr = MQX_OK;

    /*
     * Adjust message size to allow for block management overhead
     * and force size to be aligned.
     */
    requested_size += (_mem_size) (FIELD_OFFSET(STOREBLOCK_STRUCT,USER_AREA));
#if MQX_CHECK_ERRORS
    if (requested_size < MQX_MIN_MEMORY_STORAGE_SIZE) {
        requested_size = MQX_MIN_MEMORY_STORAGE_SIZE;
    } /* Endif */
#endif

    _MEMORY_ALIGN_VAL_LARGER(requested_size);

    block_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR;

    while (TRUE) {

        /*
         * Save the current block pointer.
         * We will be enabling access to higher priority tasks.
         * A higher priority task may pre-empt the current task
         * and may do a memory allocation.  If this is true,
         * the higher priority task will reset the POOL_ALLOC_CURRENT_BLOCK
         * upon exit, and the current task will start the search over
         * again.
         */
        mem_pool_ptr->POOL_ALLOC_CURRENT_BLOCK = block_ptr;

        /* allow pending interrupts */
        _int_enable();
        _int_disable();

        /* Reset to the start if we were preempted 
           and the preempting task did finish its free/malloc operation */
        if(mem_pool_ptr->POOL_ALLOC_CURRENT_BLOCK != block_ptr){
            block_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR;
        }

        if (block_ptr == NULL) { /* Null pointer */
            mem_pool_ptr->POOL_BLOCK_IN_ERROR = block_ptr;
            *error_ptr = MQX_OUT_OF_MEMORY;
            return (NULL); /* request failed */
        } /* Endif */

#if MQX_CHECK_VALIDITY
        if (!_MEMORY_ALIGNED(block_ptr) || BLOCK_IS_USED(block_ptr)) {
            mem_pool_ptr->POOL_BLOCK_IN_ERROR = block_ptr;
            *error_ptr = MQX_CORRUPT_STORAGE_POOL_FREE_LIST;
            return ((void *) NULL);
        } /* Endif */
#endif

#if MQX_CHECK_VALIDITY
        if (!VALID_CHECKSUM(block_ptr)) {
            mem_pool_ptr->POOL_BLOCK_IN_ERROR = block_ptr;
            *error_ptr = MQX_INVALID_CHECKSUM;
            return ((void *) NULL);
        } /* Endif */
#endif

        block_size = block_ptr->BLOCKSIZE;

        if ((unsigned char *) &block_ptr->USER_AREA <= (unsigned char *) requested_addr && (unsigned char *) &block_ptr->USER_AREA
                        + block_size >= (unsigned char *) requested_addr + requested_size && block_size >= requested_size) {
            /* request fits into this block */

            /* create new free block */
            free_block_ptr = (STOREBLOCK_STRUCT_PTR)((char *) block_ptr);

            block_ptr = (STOREBLOCK_STRUCT_PTR)((unsigned char *) requested_addr
                            - (_mem_size) (FIELD_OFFSET(STOREBLOCK_STRUCT,USER_AREA)));

            free_block_size = (unsigned char *) block_ptr - (unsigned char *) free_block_ptr;

#if MQX_CHECK_VALIDITY
            /* Check that user area is aligned on a cache line boundary */
            if (!_MEMORY_ALIGNED(&block_ptr->USER_AREA)) {
                *error_ptr = MQX_INVALID_CONFIGURATION;
                return ((void *) NULL);
            } /* Endif */
#endif

            next_block_size = block_size - requested_size - free_block_size;
            if (next_block_size >= (2 * MQX_MIN_MEMORY_STORAGE_SIZE)) {
                /*
                 * The current block is big enough to split.
                 * into 2 blocks.... the part to be allocated is one block,
                 * and the rest remains as a free block on the free list.
                 */

                next_block_ptr = (STOREBLOCK_STRUCT_PTR)((char *) block_ptr + requested_size);

                /* Initialize the new physical block values */
                next_block_ptr->BLOCKSIZE = next_block_size;
                next_block_ptr->PREVBLOCK = (STOREBLOCK_STRUCT_PTR) block_ptr;
                MARK_BLOCK_AS_FREE(next_block_ptr);

                CALC_CHECKSUM(next_block_ptr);

                /* Link new block into the free list */
                next_block_ptr->NEXTBLOCK = free_block_ptr->NEXTBLOCK;
                block_ptr->NEXTBLOCK = (void *) next_block_ptr;

                next_block_ptr->USER_AREA = (void *) block_ptr;
                if (next_block_ptr->NEXTBLOCK != NULL) {
                    ((STOREBLOCK_STRUCT_PTR) next_block_ptr->NEXTBLOCK)->USER_AREA = (void *) next_block_ptr;
                }

                /*
                 * Modify the current block, to point to this newly created
                 * block which is now the next physical block.
                 */
                block_ptr->BLOCKSIZE = requested_size;

                /*
                 * Modify the block on the other side of the next block
                 * (the next next block) so that it's previous block pointer
                 * correctly point to the next block.
                 */
                next_next_block_ptr = (STOREBLOCK_STRUCT_PTR) NEXT_PHYS(next_block_ptr);
                PREV_PHYS( next_next_block_ptr) = next_block_ptr;
                CALC_CHECKSUM(next_next_block_ptr);
            }
            else {
                /* Take the entire block */
                requested_size += next_block_size;
                next_block_ptr = free_block_ptr->NEXTBLOCK;
            }

            if (free_block_size >= (2 * MQX_MIN_MEMORY_STORAGE_SIZE)) {
                /* modify the new physical block values */
                free_block_ptr->BLOCKSIZE = free_block_size;
                free_block_ptr->NEXTBLOCK = (void *) next_block_ptr;
                MARK_BLOCK_AS_FREE(free_block_ptr);

                CALC_CHECKSUM(free_block_ptr);

                /* Set the size of the block */
                block_ptr->BLOCKSIZE = requested_size;
                block_ptr->PREVBLOCK = (STOREBLOCK_STRUCT_PTR) free_block_ptr;
                block_ptr->MEM_TYPE = 0;
                MARK_BLOCK_AS_USED(block_ptr, td_ptr->TASK_ID);

                CALC_CHECKSUM(block_ptr);
            }
            else {
                /* Set the size of the block */
                block_ptr->PREVBLOCK = PREV_PHYS(free_block_ptr);/*->PREVBLOCK;*/
                block_ptr->BLOCKSIZE = requested_size;
                block_ptr->MEM_TYPE = 0;
                MARK_BLOCK_AS_USED(block_ptr, td_ptr->TASK_ID);
                CALC_CHECKSUM(block_ptr);

                (block_ptr->PREVBLOCK)->BLOCKSIZE += free_block_size;
                CALC_CHECKSUM(block_ptr->PREVBLOCK);

                /* Unlink the block from the free list */
                if (free_block_ptr == mem_pool_ptr->POOL_FREE_LIST_PTR) {
                    /* At the head of the free list */

                    mem_pool_ptr->POOL_FREE_LIST_PTR = next_block_ptr;
                    if (mem_pool_ptr->POOL_FREE_LIST_PTR != NULL) {
                        PREV_FREE(mem_pool_ptr->POOL_FREE_LIST_PTR) = 0;
                    }
                }
                else {
                    /*
                     * NOTE: PREV_FREE guaranteed to be non-zero
                     * Have to make the PREV_FREE of this block
                     * point to the NEXT_FREE of this block
                     */
                    NEXT_FREE( PREV_FREE(
                        free_block_ptr)) = NEXT_FREE(free_block_ptr);
                    if (NEXT_FREE(free_block_ptr) != NULL) {
                        /*
                         * Now have to make the NEXT_FREE of this block
                         * point to the PREV_FREE of this block
                         */
                        PREV_FREE( NEXT_FREE(
                            free_block_ptr)) = PREV_FREE(free_block_ptr);
                    }
                }
#if MQX_MEMORY_FREE_LIST_SORTED == 1
                if (free_block_ptr == mem_pool_ptr->POOL_FREE_CURRENT_BLOCK) {
                    /* Reset the freelist insertion sort by _mem_free */
                    mem_pool_ptr->POOL_FREE_CURRENT_BLOCK = mem_pool_ptr->POOL_FREE_LIST_PTR;
                }
#endif
            }

            /* Reset the __mem_test freelist pointer */
            mem_pool_ptr->POOL_FREE_CHECK_BLOCK = mem_pool_ptr->POOL_FREE_LIST_PTR;

            /*
             * Set the curent pool block to the start of the free list, so
             * that if this task pre-empted another that was performing a
             * _mem_alloc, the other task will restart it's search for a block
             */
            mem_pool_ptr->POOL_ALLOC_CURRENT_BLOCK = mem_pool_ptr->POOL_FREE_LIST_PTR;

            /* Remember some statistics */
            next_block_ptr = NEXT_PHYS(block_ptr);
            if (((char *)(next_block_ptr) > (char *) mem_pool_ptr->POOL_HIGHEST_MEMORY_USED) &&
                ((char *)(next_block_ptr) > (char *) mem_pool_ptr->POOL_PTR) &&
                ((char *)(next_block_ptr) < (char *) mem_pool_ptr->POOL_END_PTR)
                )
            {
               mem_pool_ptr->POOL_HIGHEST_MEMORY_USED = ((char *)(next_block_ptr) - 1);
            }

            /* Link the block onto the task descriptor. */
            block_ptr->NEXTBLOCK = td_ptr->MEMORY_RESOURCE_LIST;
            td_ptr->MEMORY_RESOURCE_LIST = (void *) (&block_ptr->USER_AREA);

            block_ptr->MEM_POOL_PTR = (void *) mem_pool_ptr;

            return ((void *) (&block_ptr->USER_AREA));
        }
        else {
            block_ptr = (STOREBLOCK_STRUCT_PTR) NEXT_FREE(block_ptr);
        }
    }

#ifdef lint
    return( NULL ); /* to satisfy lint */
#endif
}

/*!
 * \private
 *
 * \brief Allocate an aligned block of memory for a task from the free list.
 *
 * \param[in]  requested_size Size of the memory block.
 * \param[in]  req_align      Requested alignment.
 * \param[in]  td_ptr         Owner TD.
 * \param[in]  mem_pool_ptr   Which pool to allocate from.
 * \param[out] error_ptr      Error code for operation:
 * \li MQX_OK
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_CORRUPT_STORAGE_POOL_FREE_LIST (Memory pool freelist is corrupted.)
 * \li MQX_INVALID_CHECKSUM (Checksum of the current memory block header is 
 * incorrect.)
 * \li MQX_INVALID_CONFIGURATION (User area not aligned on a cache line 
 * boundary.)
 * \li MQX_INVALID_PARAMETER (Requested alignment is not power of 2.)
 *
 * \return Pointer to the memory block (success)
 * \return NULL (Failure: see task error codes.)
 * \return (void *) NULL (Failure: see task error codes.)
 */
void *_mem_alloc_internal_align
(
    _mem_size          requested_size,
    _mem_size          req_align,
    TD_STRUCT_PTR      td_ptr,
    MEMPOOL_STRUCT_PTR mem_pool_ptr,
    _mqx_uint_ptr      error_ptr
)
{ /* Body */
	STOREBLOCK_STRUCT_PTR block_ptr;
	STOREBLOCK_STRUCT_PTR free_block_ptr;
    STOREBLOCK_STRUCT_PTR next_block_ptr = NULL;
    STOREBLOCK_STRUCT_PTR next_next_block_ptr;
    _mem_size block_size;
    _mem_size next_block_size;
    _mem_size shift;
    *error_ptr = MQX_OK;

    /*
     * Adjust message size to allow for block management overhead
     * and force size to be aligned.
     */
    requested_size += (_mem_size) (FIELD_OFFSET(STOREBLOCK_STRUCT,USER_AREA));
#if MQX_CHECK_ERRORS
    if (requested_size < MQX_MIN_MEMORY_STORAGE_SIZE) {
        requested_size = MQX_MIN_MEMORY_STORAGE_SIZE;
    } /* Endif */
    /* Check if reg_align is power of 2 */
    if ((req_align != 0) && (req_align & (req_align - 1))) {
        *error_ptr = MQX_INVALID_PARAMETER;
        return (NULL); /* request failed */
    } /* Endif */
    /* If aligment is less than PSP_MEMORY_ALIGNMENT correction is needed. */
    if (req_align <= PSP_MEMORY_ALIGNMENT) {
        req_align = (PSP_MEMORY_ALIGNMENT + 1);
    } /* Endif */
#endif

    _MEMORY_ALIGN_VAL_LARGER(requested_size);

    block_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR;

    while (TRUE) {

        /*
         * Save the current block pointer.
         * We will be enabling access to higher priority tasks.
         * A higher priority task may pre-empt the current task
         * and may do a memory allocation.  If this is true,
         * the higher priority task will reset the POOL_ALLOC_CURRENT_BLOCK
         * upon exit, and the current task will start the search over
         * again.
         */
        mem_pool_ptr->POOL_ALLOC_CURRENT_BLOCK = block_ptr;

        /* allow pending interrupts */
        _int_enable();
        _int_disable();


        /* Reset to the start if we were preempted 
           and the preempting task did finish its free/malloc operation */
        if(mem_pool_ptr->POOL_ALLOC_CURRENT_BLOCK != block_ptr){
            block_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR;
        }
        if (block_ptr == NULL) { /* Null pointer */
            mem_pool_ptr->POOL_BLOCK_IN_ERROR = block_ptr;
            *error_ptr = MQX_OUT_OF_MEMORY;
            return (NULL); /* request failed */
        } /* Endif */

#if MQX_CHECK_VALIDITY
        if (!_MEMORY_ALIGNED(block_ptr) || BLOCK_IS_USED(block_ptr)) {
            mem_pool_ptr->POOL_BLOCK_IN_ERROR = block_ptr;
            *error_ptr = MQX_CORRUPT_STORAGE_POOL_FREE_LIST;
            return ((void *) NULL);
        }

        if (!VALID_CHECKSUM(block_ptr)) {
            mem_pool_ptr->POOL_BLOCK_IN_ERROR = block_ptr;
            *error_ptr = MQX_INVALID_CHECKSUM;
            return ((void *) NULL);
        }
#endif

        block_size = block_ptr->BLOCKSIZE;
        shift = (((_mem_size) &block_ptr->USER_AREA + req_align) & ~(req_align - 1))
                        - (_mem_size) &block_ptr->USER_AREA;

        if (shift < (2 * MQX_MIN_MEMORY_STORAGE_SIZE)) {
            shift = (((_mem_size) &block_ptr->USER_AREA + (3 * MQX_MIN_MEMORY_STORAGE_SIZE) + req_align) & ~(req_align
                            - 1)) - (_mem_size) &block_ptr->USER_AREA;
        }

        if (block_size >= requested_size + shift) {
            /* request fits into this block */

            /* create new free block */
            free_block_ptr = (STOREBLOCK_STRUCT_PTR)((char *) block_ptr);
            block_ptr = (STOREBLOCK_STRUCT_PTR)(((char *) block_ptr) + shift);

#if MQX_CHECK_VALIDITY
            /* Assert that user area is aligned on a cache line boundary */
            if (!_MEMORY_ALIGNED(&block_ptr->USER_AREA)) {
                *error_ptr = MQX_INVALID_CONFIGURATION;
                return ((void *) NULL);
            } /* Endif */
#endif
            block_ptr->PREVBLOCK = (STOREBLOCK_STRUCT_PTR) free_block_ptr;

            next_block_size = block_size - requested_size - shift;
            if (next_block_size >= (2 * MQX_MIN_MEMORY_STORAGE_SIZE)) {
                /*
                 * The current block is big enough to split.
                 * into 2 blocks.... the part to be allocated is one block,
                 * and the rest remains as a free block on the free list.
                 */

                next_block_ptr = (STOREBLOCK_STRUCT_PTR)((char *) block_ptr + requested_size);

                /* Initialize the new physical block values */
                next_block_ptr->BLOCKSIZE = next_block_size;
                next_block_ptr->PREVBLOCK = (STOREBLOCK_STRUCT_PTR) block_ptr;
                MARK_BLOCK_AS_FREE(next_block_ptr);

                CALC_CHECKSUM(next_block_ptr);

                /* Link new block into the free list */
                next_block_ptr->NEXTBLOCK = free_block_ptr->NEXTBLOCK;
                block_ptr->NEXTBLOCK = (void *) next_block_ptr;

                next_block_ptr->USER_AREA = (void *) free_block_ptr;
                if (next_block_ptr->NEXTBLOCK != NULL) {
                    ((STOREBLOCK_STRUCT_PTR) next_block_ptr->NEXTBLOCK)->USER_AREA = (void *) next_block_ptr;
                }

                /*
                 * Modify the current block, to point to this newly created
                 * block which is now the next physical block.
                 */
                block_ptr->BLOCKSIZE = requested_size;

                /*
                 * Modify the block on the other side of the next block
                 * (the next next block) so that it's previous block pointer
                 * correctly point to the next block.
                 */
                next_next_block_ptr = (STOREBLOCK_STRUCT_PTR) NEXT_PHYS(next_block_ptr);
                PREV_PHYS( next_next_block_ptr) = next_block_ptr;
                CALC_CHECKSUM(next_next_block_ptr);
            }
            else {
                /*
                 * Modify the block on the other side of the next block
                 * (the next next block) so that it's previous block pointer
                 * correctly point to the next block.
                 */
                next_next_block_ptr = (STOREBLOCK_STRUCT_PTR) NEXT_PHYS(free_block_ptr);
                PREV_PHYS( next_next_block_ptr) = block_ptr;
                CALC_CHECKSUM(next_next_block_ptr);

                /* Take the entire block */
                requested_size += next_block_size;
                next_block_ptr = free_block_ptr->NEXTBLOCK;

            } /* Endif */

            /* modify the new physical block values */
            free_block_ptr->BLOCKSIZE = shift;
            free_block_ptr->NEXTBLOCK = (void *) next_block_ptr;
            MARK_BLOCK_AS_FREE(free_block_ptr);

            CALC_CHECKSUM(free_block_ptr);

            /* Set the size of the block */
            block_ptr->BLOCKSIZE = requested_size;
            block_ptr->PREVBLOCK = (STOREBLOCK_STRUCT_PTR) free_block_ptr;
            block_ptr->MEM_TYPE = 0;
            MARK_BLOCK_AS_USED(block_ptr, td_ptr->TASK_ID);

            CALC_CHECKSUM(block_ptr);

            /* Remember some statistics */
            next_block_ptr = NEXT_PHYS(block_ptr);
            if (((char *)(next_block_ptr) > (char *) mem_pool_ptr->POOL_HIGHEST_MEMORY_USED) &&
                ((char *)(next_block_ptr) > (char *) mem_pool_ptr->POOL_PTR) &&
                ((char *)(next_block_ptr) < (char *) mem_pool_ptr->POOL_END_PTR)
                )
            {
               mem_pool_ptr->POOL_HIGHEST_MEMORY_USED = ((char *)(next_block_ptr) - 1);
            } /* Endif */

            /* Link the block onto the task descriptor. */
            block_ptr->NEXTBLOCK = td_ptr->MEMORY_RESOURCE_LIST;
            td_ptr->MEMORY_RESOURCE_LIST = (void *) (&block_ptr->USER_AREA);

            block_ptr->MEM_POOL_PTR = (void *) mem_pool_ptr;


            return ((void *) (&block_ptr->USER_AREA));

        }
        else {
            block_ptr = (STOREBLOCK_STRUCT_PTR) NEXT_FREE(block_ptr);
        }

    }

#ifdef lint
    return( NULL ); /* to satisfy lint */
#endif
}

/*!
 * \private
 *
 * \brief Free part of the memory block.
 *
 * Under the same restriction as for _mem_free(), the function trims from the
 * end of the memory block.
 * \n A successful call to the function frees memory only if requested_size is
 * sufficiently smaller than the size of the original block. To determine whether
 * the function freed memory, call _mem_get_size() before and after calling
 * _mem_free_part().
 *
 * \param[in] mem_ptr        Pointer to the memory block to trim.
 * \param[in] requested_size New size for the block.
 *
 * \return MQX_OK
 * \return MQX_INVALID_POINTER (Mem_ptr is NULL, not in the pool, or misaligned.)
 * \return MQX_INVALID_CHECKSUM (Block's checksum is not correct, indicating
 * that at least some of the block was overwritten.)
 * \return MQX_NOT_RESOURCE_OWNER (If the block was allocated with _mem_alloc()
 * or _mem_alloc_zero(), only the task that allocated it can free part of it.)
 * \return MQX_INVALID_SIZE (Size of the original block is less than requested_size
 * or requested_size is less than 0 or remaining space is less than 
 * twice the size of MQX_MIN_MEMORY_STORAGE_SIZE .).
 *
 * \see _mem_free
 * \see _mem_alloc()
 * \see _mem_alloc_from()
 * \see _mem_alloc_system()
 * \see _mem_alloc_system_from()
 * \see _mem_alloc_system_zero()
 * \see _mem_alloc_system_zero_from()
 * \see _mem_alloc_system_align() 
 * \see _mem_alloc_system_align_from() 
 * \see _mem_alloc_zero()
 * \see _mem_alloc_zero_from
 * \see _mem_alloc_align()
 * \see _mem_alloc_align_from()
 * \see _mem_alloc_at()
 * \see _mem_realloc()
 * \see _mem_get_size
 * \see _task_set_error
 */
_mqx_uint _mem_free_part_internal
(
    void     *mem_ptr,
    _mem_size requested_size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    STOREBLOCK_STRUCT_PTR block_ptr;
    STOREBLOCK_STRUCT_PTR prev_block_ptr;
    STOREBLOCK_STRUCT_PTR next_block_ptr;
    STOREBLOCK_STRUCT_PTR new_block_ptr;
    TD_STRUCT_PTR td_ptr;

    _mem_size size;
    _mem_size block_size;
    _mem_size new_block_size;
    _mqx_uint result_code;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE3(KLOG_mem_free_part_internal, mem_ptr, requested_size);
    
#if MQX_CHECK_ERRORS
    /* Make sure a correct pointer was passed in.    */
    if (mem_ptr == NULL) {
        _KLOGX2(KLOG_mem_free_part_internal, MQX_INVALID_POINTER);
        return (MQX_INVALID_POINTER);
    } /* Endif */
#endif

    /* Verify the block size */
    block_ptr = GET_MEMBLOCK_PTR(mem_ptr);

#if MQX_CHECK_ERRORS
    if (!_MEMORY_ALIGNED(block_ptr)) {
        _KLOGX2(KLOG_mem_free_part_internal, MQX_INVALID_POINTER);
        return (MQX_INVALID_POINTER);
    } /* Endif */

    if ((block_ptr->BLOCKSIZE < MQX_MIN_MEMORY_STORAGE_SIZE) || BLOCK_IS_FREE(block_ptr)) {
        kernel_data->KD_POOL->POOL_BLOCK_IN_ERROR = block_ptr;
        _KLOGX3(KLOG_mem_free_part_internal, MQX_INVALID_POINTER, block_ptr);
        return (MQX_INVALID_POINTER);
    } /* Endif */
#endif

    _INT_DISABLE();
#if MQX_CHECK_VALIDITY
    if (!VALID_CHECKSUM(block_ptr)) {
        kernel_data->KD_POOL->POOL_BLOCK_IN_ERROR = block_ptr;
        _INT_ENABLE();
        _KLOGX3(KLOG_mem_free_part_internal, MQX_INVALID_CHECKSUM, block_ptr);
        return (MQX_INVALID_CHECKSUM);
    } /* Endif */
#endif

    td_ptr = SYSTEM_TD_PTR(kernel_data);
    if (block_ptr->TASK_NUMBER != (TASK_NUMBER_FROM_TASKID(td_ptr->TASK_ID))) {
        td_ptr = kernel_data->ACTIVE_PTR;
    }

    /*  Walk through the memory resources of the task descriptor.
     *  Two pointers are maintained, one to the current block
     *  and one to the previous block.
     */
    next_block_ptr = (STOREBLOCK_STRUCT_PTR) td_ptr->MEMORY_RESOURCE_LIST;
    prev_block_ptr = (STOREBLOCK_STRUCT_PTR)((unsigned char *) (&td_ptr->MEMORY_RESOURCE_LIST)
                    - FIELD_OFFSET(STOREBLOCK_STRUCT,NEXTBLOCK));

    /* Scan the task's memory resource list searching for the block to
     * free, Stop when the current pointer is equal to the block to free
     * or the end of the list is reached.
     */
    while (next_block_ptr && ((void *) next_block_ptr != mem_ptr)) {
        /* The block is not found, and the end of the list has not been
         * reached, so move down the list.
         */
        prev_block_ptr = GET_MEMBLOCK_PTR(next_block_ptr);
        next_block_ptr = (STOREBLOCK_STRUCT_PTR) prev_block_ptr->NEXTBLOCK;
    } /* Endwhile */

#if MQX_CHECK_ERRORS
    if (next_block_ptr == NULL) {
        _INT_ENABLE();
        _KLOGX2(KLOG_mem_free_part_internal, MQX_NOT_RESOURCE_OWNER);
        /* The specified block does not belong to the calling task. */
        return (MQX_NOT_RESOURCE_OWNER);
    } /* Endif */
#endif

    /* determine the size of the block.  */
    block_size = block_ptr->BLOCKSIZE;

    size = requested_size + (_mem_size) FIELD_OFFSET(STOREBLOCK_STRUCT,USER_AREA);
    if (size < MQX_MIN_MEMORY_STORAGE_SIZE) {
        size = MQX_MIN_MEMORY_STORAGE_SIZE;
    } /* Endif */
    _MEMORY_ALIGN_VAL_LARGER(size);

#if MQX_CHECK_ERRORS
    /* Verify that the size parameter is within range of the block size. */
    if (size <= block_size) {
#endif
        /* Adjust the size to allow for the overhead and force alignment */

        /* Compute the size of the new_ block that would be created. */
        new_block_size = block_size - size;

        /* Decide if it worthwile to split the block. If the amount of space
         * returned is not at least twice the size of the block overhead,
         * then return error code.
         */
        if (new_block_size >= (2 * MQX_MIN_MEMORY_STORAGE_SIZE)) {

            /* Create an 'inuse' block */
            new_block_ptr = (STOREBLOCK_STRUCT_PTR)((char *) block_ptr + size);
            new_block_ptr->BLOCKSIZE = new_block_size;
            PREV_PHYS( new_block_ptr) = block_ptr;
            new_block_ptr->TASK_NUMBER = block_ptr->TASK_NUMBER;
            new_block_ptr->MEM_POOL_PTR = block_ptr->MEM_POOL_PTR;
            CALC_CHECKSUM(new_block_ptr);
            /* Split the block */
            block_ptr->BLOCKSIZE = size;
            CALC_CHECKSUM(block_ptr);

            /* make sure right physical neighbour knows about it */
            block_ptr = NEXT_PHYS(new_block_ptr);
            PREV_PHYS( block_ptr) = new_block_ptr;
            CALC_CHECKSUM(block_ptr);

            /* Link the new block onto the requestor's task descriptor. */
            new_block_ptr->NEXTBLOCK = td_ptr->MEMORY_RESOURCE_LIST;
            td_ptr->MEMORY_RESOURCE_LIST = (char *) (&new_block_ptr->USER_AREA);

            result_code = _mem_free((void *) &new_block_ptr->USER_AREA);
        }
        else {
            result_code = MQX_INVALID_SIZE;
        } /* Endif */
#if MQX_CHECK_ERRORS
    }
    else {
        result_code = MQX_INVALID_SIZE;
    } /* Endif */
#endif
    
    _INT_ENABLE();
    _KLOGX2(_mem_free_part_internal, result_code);
    return (result_code);
}

/*! 
 * \private
 *
 * \brief If it is possible, extend given memory block.
 *
 * This function extend given memory block, if free memory block is
 * right behind given block and is big enough. This function merge whole
 * or part of free block with given memory block.
 *
 * \param[in] mem_ptr Pointer to given memory block.
 * \param[in] size New size for given memory block.
 *
 * \return MQX_OK
 * \return MQX_INVALID_POINTER (Mem_ptr is NULL, not in the pool, or misaligned.)
 * \return MQX_INVALID_CHECKSUM (Block's checksum is not correct, indicating
 * that at least some of the block was overwritten.)
 * \return MQX_NOT_RESOURCE_OWNER (If the block was allocated with _mem_alloc()
 * or _mem_alloc_zero(), only the task that allocated it can free part of it.)
 * \return MQX_INVALID_SIZE (Size of the original block is bigger than requested 
 *  size, or no free block with enough size after original block.)
 * \return MQX_CORRUPT_STORAGE_POOL_FREE_LIST (Memory pool freelist is corrupted.)
 * \return MQX_INVALID_CONFIGURATION (User area of free block not aligned on a cache line 
 * boundary.)
 */
_mqx_uint _mem_alloc_extend_internal
(
   void         *mem_ptr,
   _mem_size    size
      
)
{/* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    STOREBLOCK_STRUCT_PTR  block_ptr;
    STOREBLOCK_STRUCT_PTR  prev_block_ptr;
    STOREBLOCK_STRUCT_PTR  next_block_ptr;
    STOREBLOCK_STRUCT_PTR  next_next_block_ptr;
    STOREBLOCK_STRUCT_PTR  next_ptr;
    volatile STOREBLOCK_STRUCT_PTR  free_ptr;
    MEMPOOL_STRUCT_PTR     mem_pool_ptr;
    _mem_size              block_size;
    _mem_size              requested_size;
    TD_STRUCT_PTR          td_ptr;
    void                   *user_are_ptr;
    _mem_size              next_block_size;
    _mqx_uint              result_code;
      
    _GET_KERNEL_DATA(kernel_data);
    _KLOGE3(_mem_alloc_extend_internal, mem_ptr, size);

#if MQX_CHECK_ERRORS
    /* Make sure a correct pointer was passed in.    */
    if (mem_ptr == NULL) {
        _KLOGX2(KLOG_mem_alloc_extend_internal, MQX_INVALID_POINTER);
        return (MQX_INVALID_POINTER);
    } /* Endif */
#endif
    
    /* Verify the block size */
    block_ptr = GET_MEMBLOCK_PTR(mem_ptr);

#if MQX_CHECK_ERRORS
    /* Verify pointer alignment */
    if (!_MEMORY_ALIGNED(block_ptr) || (block_ptr->BLOCKSIZE < MQX_MIN_MEMORY_STORAGE_SIZE) || BLOCK_IS_FREE(block_ptr)) {
        _KLOGX2(KLOG_mem_alloc_extend_internal, MQX_INVALID_POINTER);
        return (MQX_INVALID_POINTER);
    } /* Endif */
#endif
    
    /* determine the size of the block.  */
    block_size = block_ptr->BLOCKSIZE;
    requested_size = size + (_mem_size) FIELD_OFFSET(STOREBLOCK_STRUCT,USER_AREA);
    
#if MQX_CHECK_ERRORS
    /* Verify the passed in parameter */
    if (requested_size < block_size)
    {
        _KLOGX2(KLOG_mem_alloc_extend_internal, MQX_INVALID_SIZE);
        return (MQX_INVALID_SIZE);
    } /* Endif */
#endif   
    
    _MEMORY_ALIGN_VAL_LARGER(requested_size);
    
    _INT_DISABLE();
    
#if MQX_CHECK_VALIDITY
    if (!VALID_CHECKSUM(block_ptr)) {
        kernel_data->KD_POOL->POOL_BLOCK_IN_ERROR = block_ptr;
        _INT_ENABLE();
        _KLOGX3(KLOG_mem_alloc_extend_internal, MQX_INVALID_CHECKSUM, block_ptr);
        return (MQX_INVALID_CHECKSUM);
    } /* Endif */
#endif
    
    mem_pool_ptr = (MEMPOOL_STRUCT_PTR) block_ptr->MEM_POOL_PTR;
    td_ptr = SYSTEM_TD_PTR(kernel_data);
    if (block_ptr->TASK_NUMBER != (TASK_NUMBER_FROM_TASKID(td_ptr->TASK_ID))) {
        td_ptr = kernel_data->ACTIVE_PTR;
    }

    /*  Walk through the memory resources of the task descriptor.
     *  Two pointers are maintained, one to the current block
     *  and one to the previous block.
     */
    next_block_ptr = (STOREBLOCK_STRUCT_PTR) td_ptr->MEMORY_RESOURCE_LIST;
    prev_block_ptr = (STOREBLOCK_STRUCT_PTR)((unsigned char *) (&td_ptr->MEMORY_RESOURCE_LIST)
                    - FIELD_OFFSET(STOREBLOCK_STRUCT,NEXTBLOCK));

    /* Scan the task's memory resource list searching for the block to
     * free, Stop when the current pointer is equal to the block to free
     * or the end of the list is reached.
     */
    while (next_block_ptr && ((void *) next_block_ptr != mem_ptr)) {
        /* The block is not found, and the end of the list has not been
         * reached, so move down the list.
         */
        prev_block_ptr = GET_MEMBLOCK_PTR(next_block_ptr);
        next_block_ptr = (STOREBLOCK_STRUCT_PTR) prev_block_ptr->NEXTBLOCK;
    } /* Endwhile */

#if MQX_CHECK_ERRORS
    if (next_block_ptr == NULL) {
        _INT_ENABLE();
        /* The specified block does not belong to the calling task. */
        _KLOGX2(KLOG_mem_alloc_extend_internal, MQX_NOT_RESOURCE_OWNER);
        return (MQX_NOT_RESOURCE_OWNER);
    } /* Endif */
#endif
        
    /* No need extend */
    if(requested_size == block_ptr->BLOCKSIZE){
      _INT_ENABLE();
      _KLOGX2(KLOG_mem_alloc_extend_internal, MQX_OK);
      return (MQX_OK);
    }/* Endif */
        
    free_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR;
    
    while (TRUE){
            
      if (free_ptr == NULL){
          block_ptr = NULL;
          break;      
      }
      
#if MQX_CHECK_VALIDITY
        if (!_MEMORY_ALIGNED(free_ptr) || BLOCK_IS_USED(free_ptr)) {
            mem_pool_ptr->POOL_BLOCK_IN_ERROR = free_ptr;
            _INT_ENABLE();
            _KLOGX3(KLOG_mem_alloc_extend_internal, MQX_CORRUPT_STORAGE_POOL_FREE_LIST, free_ptr);
            return (MQX_CORRUPT_STORAGE_POOL_FREE_LIST);
        } /* Endif */
      
        if (!VALID_CHECKSUM(free_ptr)) {
            mem_pool_ptr->POOL_BLOCK_IN_ERROR = free_ptr;
            _INT_ENABLE();
            _KLOGX3(KLOG_mem_alloc_extend_internal, MQX_INVALID_CHECKSUM, free_ptr);
            return (MQX_INVALID_CHECKSUM);
        } /* Endif */
#endif
  
#if MQX_MEMORY_FREE_LIST_SORTED == 1
      /* Searching for free memory block behind allocated memory block */
      if (((void *) block_ptr < free_ptr)){
#endif       
        /* Check if free block is behind allocated block */
        if (((unsigned char *) block_ptr + block_ptr->BLOCKSIZE) == (unsigned char *) free_ptr){
          
          /* If free memory block is not big enough set block_ptr as NULL*/
          size = requested_size-block_ptr->BLOCKSIZE;  
          if(size > free_ptr->BLOCKSIZE){
            block_ptr = NULL;
          }
          break;
        }
        else{
          block_ptr = NULL;
          break;
        }/* Endif */
        
 #if MQX_MEMORY_FREE_LIST_SORTED == 1     
      }/* Endif */
#endif
      /*
       * Save the next block pointer.
       * We will be enabling access to higher priority tasks.
       * A higher priority task may pre-empt the current task
       * and may do a memory allocation.  If this is true,
       * the higher priority task will reset the POOL_ALLOC_CURRENT_BLOCK
       * upon exit, and the current task will start the search over again.
       */
      
      prev_block_ptr = free_ptr;
      free_ptr = free_ptr->NEXTBLOCK;
      mem_pool_ptr->POOL_ALLOC_CURRENT_BLOCK = free_ptr;

      /* allow pending interrupts */
      _INT_ENABLE();
      _INT_DISABLE();

      /* Reset to the start if we were preempted 
         and the preempting task did finish its free/malloc operation */
      if(mem_pool_ptr->POOL_ALLOC_CURRENT_BLOCK != free_ptr){
          free_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR;
      }/* Endif */
      
    }/* Endwhile */
    
    if(block_ptr != NULL){
      
#if MQX_CHECK_VALIDITY
      /* Check that user area is aligned on a cache line boundary */
      if (!_MEMORY_ALIGNED(&free_ptr->USER_AREA)) {
           _INT_ENABLE();
           _KLOGX2(KLOG_mem_alloc_extend_internal, MQX_INVALID_CONFIGURATION);
          return (MQX_INVALID_CONFIGURATION);
      } /* Endif */
#endif
            
      next_block_size = free_ptr->BLOCKSIZE-size;
      if (next_block_size >= (2 * MQX_MIN_MEMORY_STORAGE_SIZE)){ 
        /*
         * The current block is big enough to split.
         * into 2 blocks.... the part to be added to given block,
         * and the rest remains as a free block on the free list.
         */
        /* back up */
        next_ptr = free_ptr->NEXTBLOCK;
        user_are_ptr = free_ptr->USER_AREA;
        prev_block_ptr = free_ptr->PREVBLOCK;
        
        next_block_ptr = (STOREBLOCK_STRUCT_PTR)((char *) free_ptr + size);
        
        /* Initialize the new physical block values */
        next_block_ptr->BLOCKSIZE = next_block_size;
        next_block_ptr->PREVBLOCK = (STOREBLOCK_STRUCT_PTR) prev_block_ptr;
        MARK_BLOCK_AS_FREE(next_block_ptr);
        
        CALC_CHECKSUM(next_block_ptr);
        
        /* Link new block into the free list */
        next_block_ptr->NEXTBLOCK = next_ptr;
        
        next_block_ptr->USER_AREA = user_are_ptr;
        
        if (next_block_ptr->NEXTBLOCK != NULL) {
            ((STOREBLOCK_STRUCT_PTR) next_block_ptr->NEXTBLOCK)-> USER_AREA = (void *) next_block_ptr;
        } /* Endif */
        
        if (next_block_ptr->USER_AREA != NULL) {
            ((STOREBLOCK_STRUCT_PTR) next_block_ptr->USER_AREA)-> NEXTBLOCK = (void *) next_block_ptr;
        } /* Endif */
        /*
         * Modify the block on the other side of the next block
         * (the next next block) so that it's previous block pointer
         * correctly point to the next block.
         */
        next_next_block_ptr = (STOREBLOCK_STRUCT_PTR) NEXT_PHYS(next_block_ptr);
        PREV_PHYS( next_next_block_ptr) = next_block_ptr;
        CALC_CHECKSUM(next_next_block_ptr);
      }
      else{
        /* Take the entire block */
        size = free_ptr->BLOCKSIZE;
        
        if (free_ptr->NEXTBLOCK != NULL) {
            ((STOREBLOCK_STRUCT_PTR) free_ptr->NEXTBLOCK)-> USER_AREA = (void *) free_ptr->USER_AREA;
        } /* Endif */
        
        if (free_ptr->USER_AREA != NULL) {
            ((STOREBLOCK_STRUCT_PTR) free_ptr->USER_AREA)-> NEXTBLOCK = (void *) free_ptr->NEXTBLOCK;
        } /* Endif */
        
        next_next_block_ptr = (STOREBLOCK_STRUCT_PTR) NEXT_PHYS(free_ptr);
        PREV_PHYS( next_next_block_ptr) = block_ptr;
        CALC_CHECKSUM(next_next_block_ptr);        
        
        next_block_ptr = free_ptr->NEXTBLOCK;
      }/* Endif */
      
      block_ptr->BLOCKSIZE += size;
      CALC_CHECKSUM(block_ptr);
      
      if(free_ptr == mem_pool_ptr->POOL_FREE_LIST_PTR){
        
        /* At the head of the free list */
        mem_pool_ptr->POOL_FREE_LIST_PTR = next_block_ptr;
        if (mem_pool_ptr->POOL_FREE_LIST_PTR != NULL) {
            PREV_FREE(mem_pool_ptr->POOL_FREE_LIST_PTR) = 0;
        } /* Endif */
      }/* Endif */
      
#if MQX_MEMORY_FREE_LIST_SORTED == 1
            if (free_ptr == mem_pool_ptr->POOL_FREE_CURRENT_BLOCK) {
                /* Reset the freelist insertion sort by _mem_free */
                mem_pool_ptr->POOL_FREE_CURRENT_BLOCK = mem_pool_ptr->POOL_FREE_LIST_PTR;
            } /* Endif */
#endif
      
      /* Reset the __mem_test freelist pointer */
      mem_pool_ptr->POOL_FREE_CHECK_BLOCK = mem_pool_ptr->POOL_FREE_LIST_PTR;

      /*
       * Set the curent pool block to the start of the free list, so
       * that if this task pre-empted another that was performing a
       * _mem_alloc, the other task will restart it's search for a block
       */
      mem_pool_ptr->POOL_ALLOC_CURRENT_BLOCK = mem_pool_ptr->POOL_FREE_LIST_PTR;

      /* Remember some statistics - only for blocks which fall into the
       * main pool area. Ignore blocks in the extended areas (EXT_LIST)
       */
      next_block_ptr = NEXT_PHYS(block_ptr);
      if (((char *)(next_block_ptr) > (char *) mem_pool_ptr->POOL_HIGHEST_MEMORY_USED) &&
          ((char *)(next_block_ptr) > (char *) mem_pool_ptr->POOL_PTR) &&
          ((char *)(next_block_ptr) < (char *) mem_pool_ptr->POOL_END_PTR)
          )
      {
         mem_pool_ptr->POOL_HIGHEST_MEMORY_USED = ((char *)(next_block_ptr) - 1);
      } /* Endif */
      
      result_code = MQX_OK;      
    }
    else{
      result_code = MQX_INVALID_SIZE;
    }/* Endif */  
    
    _INT_ENABLE();
    
    _KLOGX2(KLOG_mem_alloc_extend_internal, result_code);
    return result_code;
    
} /* Endbody */

/*!
 * \brief Resize the memomory block that was previously allocated.
 *
 * This function can allocate new memory block with a specific size, or 
 * resize given memory block. When create new memory block, then copy
 * data from memory block pointed to by the pointer to new memory block.
 * Then memory block, that was previosly allocated, is freed.
 * 
 * When given pointer is NULL, then function has same behavior as malloc(size).
 * When size is 0, then function has same behavior as malloc(0), and given
 * memory block is freed.
 *
 * \param[in] mem_ptr Pointer to given memory block.
 * \param[in] size Size for the new memory block.
 *
 * \return Pointer to the new memory block (Success.)
 * \return NULL (Failure: see task error codes.)
 *
 * \li MQX_INVALID_POINTER (Block not in the pool, or misaligned.)
 * \li MQX_INVALID_CHECKSUM (Block's checksum is not correct, indicating
 *  that at least some of the block was overwritten.)
 * \li MQX_NOT_RESOURCE_OWNER (If the block was allocated with _mem_alloc()
 * or _mem_alloc_zero(), only the task that allocated it can free part of it.)
 * \li MQX_INVALID_SIZE (Size of given memory block was changed while 
 *  realloc was executed)
 * \li MQX_CORRUPT_STORAGE_POOL_FREE_LIST (Memory pool freelist is corrupted.)
 * \li MQX_INVALID_CONFIGURATION (User area of free block not aligned on a cache line 
 *  boundary.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a free block of the requested size.)
 *
 * \see _mem_alloc()
 * \see _mem_alloc_from()
 * \see _mem_alloc_system()
 * \see _mem_alloc_system_from()
 * \see _mem_alloc_system_zero_from()
 * \see _mem_alloc_system_align() 
 * \see _mem_alloc_system_align_from() 
 * \see _mem_alloc_zero()
 * \see _mem_alloc_zero_form()
 * \see _mem_alloc_align()
 * \see _mem_alloc_align_from()
 * \see _mem_alloc_at()
 * \see _mem_create_pool
 * \see _mem_free
 * \see _mem_get_highwater
 * \see _mem_get_highwater_pool
 * \see _mem_get_size
 * \see _mem_transfer
 * \see _mem_free_part
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error
 */
void *_mem_realloc
(
   void         *mem_ptr,
   _mem_size    size
      
)
{/* Body */
  void                   *result = NULL;
  _mem_size              old_size;
  _mem_size              copy_size;
#if MQX_CHECK_ERRORS && !NDEBUG
  _mqx_uint              error;
#endif
  _mqx_uint              realloc_error;
  STOREBLOCK_STRUCT_PTR  block_ptr;
  STOREBLOCK_STRUCT_PTR  prev_block_ptr;
  STOREBLOCK_STRUCT_PTR  next_block_ptr;
  KERNEL_DATA_STRUCT_PTR kernel_data;
  TD_STRUCT_PTR          td_ptr;
  
  _GET_KERNEL_DATA(kernel_data);
  _KLOGE3(KLOG_mem_realloc, mem_ptr, size);
    
  /* Behavior as malloc */
  if(mem_ptr == NULL){    
    result = (TASK_NUMBER_FROM_TASKID(_task_get_id()) == SYSTEM_TASK_NUMBER) ? _mem_alloc_system(size) : _mem_alloc(size);   
    _KLOGX2(KLOG_mem_realloc, result);
    return result;
  }/* Endif */
  
  block_ptr = GET_MEMBLOCK_PTR(mem_ptr);

#if MQX_CHECK_ERRORS
    /* Verify pointer alignment */
    if (!_MEMORY_ALIGNED(block_ptr) || (block_ptr->BLOCKSIZE < MQX_MIN_MEMORY_STORAGE_SIZE) || BLOCK_IS_FREE(block_ptr)) {
        _task_set_error(MQX_INVALID_POINTER);
        _KLOGX2(KLOG_mem_realloc, MQX_INVALID_POINTER);
        return result;
    } /* Endif */

#endif

  _INT_DISABLE();

#if MQX_CHECK_VALIDITY
    if (!VALID_CHECKSUM(block_ptr)) {
        _INT_ENABLE();
        _task_set_error(MQX_INVALID_CHECKSUM);
        _KLOGX2(KLOG_mem_realloc, MQX_INVALID_CHECKSUM);
        return result;
    } /* Endif */
#endif
  
  td_ptr = SYSTEM_TD_PTR(kernel_data);
  if (block_ptr->TASK_NUMBER != (TASK_NUMBER_FROM_TASKID(td_ptr->TASK_ID))) {
      td_ptr = kernel_data->ACTIVE_PTR;
  } /* Endif */

  /*
   * Walk through the memory resources of the task descriptor.
   * Two pointers are maintained, one to the current block
   * and one to the previous block.
   */
  next_block_ptr = (STOREBLOCK_STRUCT_PTR) td_ptr->MEMORY_RESOURCE_LIST;
  prev_block_ptr = (STOREBLOCK_STRUCT_PTR)((unsigned char *) (&td_ptr->MEMORY_RESOURCE_LIST)
                  - FIELD_OFFSET(STOREBLOCK_STRUCT,NEXTBLOCK));

  /*
   * Scan the task's memory resource list searching for the block.
   * Stop when the current pointer is equal to the block
   * or the end of the list is reached.
   */
  while (next_block_ptr && ((void *) next_block_ptr != mem_ptr)) {
      /*
       * The block is not found, and the end of the list has not been
       * reached, so move down the list.
       */
      prev_block_ptr = GET_MEMBLOCK_PTR(next_block_ptr);
      next_block_ptr = (STOREBLOCK_STRUCT_PTR) prev_block_ptr->NEXTBLOCK;
  } /* Endwhile */

#if MQX_CHECK_ERRORS
    if (next_block_ptr == NULL) {
        _INT_ENABLE();
        /* The specified block does not belong to the calling task. */
        _task_set_error(MQX_NOT_RESOURCE_OWNER);
        _KLOGX2(KLOG_mem_realloc, MQX_NOT_RESOURCE_OWNER);
        return result;
    } /* Endif */
#endif
  
  /* size of given memory block */
  old_size = block_ptr->BLOCKSIZE - (_mem_size) (FIELD_OFFSET(STOREBLOCK_STRUCT,USER_AREA));   
  
  _INT_ENABLE();
  
  if(size > 0){       
 
    realloc_error = (size < old_size) ? _mem_free_part_internal(mem_ptr, size) : _mem_alloc_extend_internal(mem_ptr, size);
  
#if MQX_CHECK_ERRORS
    if(realloc_error != MQX_OK && realloc_error != MQX_INVALID_SIZE){
      _task_set_error(realloc_error);
      _KLOGX2(KLOG_mem_realloc, realloc_error);
      return result;
    }
#endif
         
    if(realloc_error == MQX_OK){
        result = mem_ptr;
    }
    else{
      copy_size = (size < old_size) ? size : old_size;
    }/* Endif */
  }
  else{     
    copy_size = size;
  }/* Endif */ 
         
  /* Create new memory block, copy data from given memory block to new memory block and free given memory block */
  if(result != mem_ptr){
    result = (block_ptr->TASK_NUMBER == SYSTEM_TASK_NUMBER) ? _mem_alloc_system(size) : _mem_alloc(size);
    
#if MQX_CHECK_ERRORS
  if(result == NULL){
    _KLOGX2(KLOG_mem_realloc, result);
    return result;
  }/* Endif */
#endif
  
    _mem_copy(mem_ptr, result, copy_size);
#if MQX_CHECK_ERRORS && !NDEBUG
    error = _mem_free(mem_ptr);
    assert(error == MQX_OK);
#else
    _mem_free(mem_ptr);
#endif
  
  }/* Endif */
  
  _KLOGX2(KLOG_mem_realloc, result);
  return (result);
}/* Endbody */

#endif /* MQX_USE_MEM */
#endif /* !MQX_USE_TLSF_ALLOCATOR */

#if MQX_USE_MEM || MQX_USE_LWMEM || MQX_USE_TLSF_ALLOCATOR

/*!
 * \brief Converts data to the other endian format.
 *
 * Converts data from Big to Little Endian byte order (or vice versa). The size
 * of the fields in the data are defined by the null terminated array of 8-bit
 * numbers.
 *
 * \param[in] definition Pointer to a NULL-terminated array, each element of
 * which defines the size (in single-addressable units) of each field in the
 * data structure that defines the data to convert.
 * \param[in] data       Pointer to the data to convert.
 *
 * \see _mem_swap_endian_len
 * \see _msg_swap_endian_data
 * \see _msg_swap_endian_header
 */
void _mem_swap_endian
(
    register unsigned char  *definition,
    void                *data
)
{ /* Body */
    register unsigned char      *data_ptr;
    register unsigned char      *next_ptr;
    unsigned char      *b_ptr;
    _mqx_uint i;
    register _mqx_uint size;
    unsigned char c;

    data_ptr = (unsigned char *) data;
    size = (_mqx_uint) *definition++;
    while (size) {
        switch (size)
        {
            case 0: /* For compiler optimizations */
                break;
            case 1: /* No need to swap */
                ++data_ptr;
                break;
                /* Cases 2 & 4 are common sizes */
            case 2:
                c = data_ptr[0];
                data_ptr[0] = data_ptr[1];
                data_ptr[1] = c;
                data_ptr += 2;
                break;
            case 4:
                c = data_ptr[0];
                data_ptr[0] = data_ptr[3];
                data_ptr[3] = c;
                c = data_ptr[1];
                data_ptr[1] = data_ptr[2];
                data_ptr[2] = c;
                data_ptr += 4;
                break;
                /* All others done long hand */
            default:
                next_ptr = data_ptr + size;
                b_ptr = data_ptr + size - 1;
                i = (size / 2) + 1;
                while (--i) {
                    c = *data_ptr;
                    *data_ptr++ = *b_ptr;
                    *b_ptr-- = c;
                } /* Endwhile */
                data_ptr = next_ptr;
                break;
        } /* Endswitch */
        size = *definition++;
    } /* Endwhile */

} /* Endbody */

/*!
 * \brief Converts data to the other endian format.
 *
 * Converts data from Big to Little Endian byte order (or vice versa). The size
 * of the fields in the data are defined by the null terminated array of 8-bit
 * numbers.
 *
 * \param[in] definition Pointer to a NULL-terminated array, each element of
 * which defines the size (in single-addressable units) of each field in the
 * data structure that defines the data to convert.
 * \param[in] data       Pointer to the data to convert.
 * \param[in] len        The fields in the definition array to process.
 *
 * \see _mem_swap_endian
 * \see _msg_swap_endian_data
 * \see _msg_swap_endian_header
 */
void _mem_swap_endian_len
(
    register unsigned char  *definition,
    void                *data,
    _mqx_uint            len
)
{ /* Body */
    register unsigned char      *data_ptr;
    register unsigned char      *next_ptr;
    unsigned char      *b_ptr;
    _mqx_uint i;
    register _mqx_uint size;
    unsigned char c;

    data_ptr = (unsigned char *) data;
    size = (_mqx_uint) *definition++;
    while (size && len--) {
        switch (size)
        {
            case 0: /* For compiler optimizations */
                break;
            case 1: /* No need to swap */
                ++data_ptr;
                break;
                /* Cases 2 & 4 are common sizes */
            case 2:
                c = data_ptr[0];
                data_ptr[0] = data_ptr[1];
                data_ptr[1] = c;
                data_ptr += 2;
                break;
            case 4:
                c = data_ptr[0];
                data_ptr[0] = data_ptr[3];
                data_ptr[3] = c;
                c = data_ptr[1];
                data_ptr[1] = data_ptr[2];
                data_ptr[2] = c;
                data_ptr += 4;
                break;
                /* All others done long hand */
            default:
                next_ptr = data_ptr + size;
                b_ptr = data_ptr + size - 1;
                i = (size / 2) + 1;
                while (--i) {
                    c = *data_ptr;
                    *data_ptr++ = *b_ptr;
                    *b_ptr-- = c;
                } /* Endwhile */
                data_ptr = next_ptr;
                break;
        } /* Endswitch */
        size = (_mqx_uint) *definition++;
    } /* Endwhile */

} /* Endbody */

#endif /* MQX_USE_MEM || MQX_USE_LWMEM || MQX_USE_TLSF_ALLOCATOR */

/*!
 * \brief Checks whether memory can be read and written without corruption.
 *
 * It aligns the addresses it's given to a PSP specified value and then writes
 * every 32-bit element in the memory range once with values that change every
 * bit. Then it reads back the data and returns an error if the value read back
 * does not equal the value written.
 *
 * \param[in] base
 * \param[in] extent
 *
 * \return MQX_OK
 * \return MQX_INVALID_SIZE
 * \return MQX_CORRUPT_MEMORY_SYSTEM
 */
_mqx_uint _mem_verify
(
    void   *base,
    void   *extent
)
{ /* Body */

    _mqx_uint result = MQX_INVALID_SIZE;

    if (extent > base) {
        unsigned char *cbase = (unsigned char *) _ALIGN_ADDR_TO_HIGHER_MEM(base);
        unsigned char *cextent = (unsigned char *) _ALIGN_ADDR_TO_LOWER_MEM(extent);

        if (cextent > cbase) {
            uint32_t length = cextent - cbase;
            uint32_t *p, *p1 = (uint32_t *) cbase;
            uint32_t *eom = (uint32_t *) (cbase + length);
            uint32_t v = 0x12345678;

            for (p = p1; p < eom; p++) {
                *p = v;
                v += 0x11111111;
            } /* Endfor */

            result = MQX_OK;

            v = 0x12345678;
            for (p = p1; p < eom; p++) {
                if (*p != v) {
                    result = MQX_CORRUPT_MEMORY_SYSTEM;
                    break;
                } /* Endif */
                v += 0x11111111;
            } /* Endfor */

        } /* Endif */

    } /* Endif */

    return (result);

} /* Endbody */

#if MQX_MEM_MONITOR && MQX_USE_MEM
_mem_size _mem_get_size_pools()
{
   KERNEL_DATA_STRUCT_PTR kernel_data;
   MEMPOOL_STRUCT_PTR     mem_pool_ptr;
   _mem_size              total_size = 0;

   _GET_KERNEL_DATA(kernel_data);
   mem_pool_ptr = &kernel_data->KD_POOL;

   do
   {
      total_size += mem_pool_ptr->POOL_SIZE;
      mem_pool_ptr = (MEMPOOL_STRUCT_PTR)mem_pool_ptr->LINK.NEXT;
   } while ((QUEUE_ELEMENT_STRUCT*)mem_pool_ptr != (&kernel_data->KD_POOL)->LINK.PREV);

   return total_size;
}

_mem_size _mem_highest_used()
{
    KERNEL_DATA_STRUCT_PTR kernel_data;
    _GET_KERNEL_DATA(kernel_data);

    return kernel_data->MEM_HIGHEST_USED;
}
#endif

/* EOF */
