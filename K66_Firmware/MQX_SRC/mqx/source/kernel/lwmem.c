
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
*   This file contains functions of the Lightweight Memory component.
*
*
*END************************************************************************/

#include "mqx_inc.h"
#if MQX_USE_LWMEM
#include "lwmem.h"
#include "lwmem_prv.h"

#if MQX_MEM_MONITOR
_mem_size _lwmem_get_size_pools()
{
   KERNEL_DATA_STRUCT_PTR       kernel_data;
   LWMEM_POOL_STRUCT_PTR        pool_ptr;
   _mem_size                    total_size = 0;

   _GET_KERNEL_DATA(kernel_data);

   /* initialize the memory alloc_info_struct */
   pool_ptr = (LWMEM_POOL_STRUCT*)kernel_data->LWMEM_POOLS.NEXT;
   do
   {
      total_size += (_mem_size)pool_ptr->POOL_ALLOC_END_PTR - (_mem_size)pool_ptr;
      pool_ptr = (LWMEM_POOL_STRUCT_PTR)pool_ptr->LINK.NEXT;
   } while ((QUEUE_ELEMENT_STRUCT**)pool_ptr != &kernel_data->LWMEM_POOLS.NEXT);

   return total_size;
}

_mem_size _lwmem_highest_used()
{
    KERNEL_DATA_STRUCT_PTR kernel_data;
    _GET_KERNEL_DATA(kernel_data);

    return kernel_data->MEM_HIGHEST_USED;
}
#endif /* MQX_MEM_MONITOR */

/*!
 * \private
 *
 * \brief Allocates a block of memory for a task from the free list.
 *
 * \param[in] requested_size The size of the memory block.
 * \param[in] td_ptr         The owner TD.
 * \param[in] pool_id        Which pool to allocate from.
 * \param[in] zero           Zero the memory after it is allocated.
 *
 * \return Pointer to the lightweight-memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_LWMEM_POOL_INVALID (Memory pool to allocate from is invalid.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 */
void *_lwmem_alloc_internal
(
    _mem_size      requested_size,
    TD_STRUCT_PTR  td_ptr,
    _lwmem_pool_id pool_id,
    bool        zero
)
{ /* Body */
    LWMEM_BLOCK_STRUCT_PTR block_ptr;
    LWMEM_BLOCK_STRUCT_PTR next_block_ptr;
    LWMEM_BLOCK_STRUCT_PTR prev_block_ptr = NULL;
    _mem_size              block_size;
    _mem_size              next_block_size;
    LWMEM_POOL_STRUCT_PTR  mem_pool_ptr = (LWMEM_POOL_STRUCT_PTR) pool_id;
    uint32_t               highwater;
    void*                  retval = NULL;

#if MQX_CHECK_VALIDITY
    if (mem_pool_ptr->VALID != LWMEM_POOL_VALID)
    {
        _task_set_error(MQX_LWMEM_POOL_INVALID);
        return (NULL);
    } /* Endif */
#endif

    /*
     * Adjust message size to allow for block management overhead
     * and force size to be aligned.
     */
    requested_size += (_mem_size) sizeof(LWMEM_BLOCK_STRUCT);
    if (requested_size < LWMEM_MIN_MEMORY_STORAGE_SIZE)
    {
        requested_size = LWMEM_MIN_MEMORY_STORAGE_SIZE;
    } /* Endif */

    _MEMORY_ALIGN_VAL_LARGER(requested_size);

    _int_disable();
    /* Start search for block from the beginning of list of free memory blocks */
    block_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR;

    prev_block_ptr =  mem_pool_ptr->POOL_FREE_LIST_PTR;
    while (block_ptr != NULL)
    {
        /* Provide window for higher priority tasks */
        mem_pool_ptr->POOL_ALLOC_PTR = block_ptr;

        _int_enable();
        _int_disable();



        /* Reset to the start if we were preempted
           and the preempting task did finish its free/malloc operation,
           a safe solution, even with time-slicing. */
        if(block_ptr != mem_pool_ptr->POOL_ALLOC_PTR)
        {
            block_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR;
            prev_block_ptr = block_ptr;
        }

        if (block_ptr == NULL)
        {
            /* The pool became full during preemption, no free block exists anymore */
            break;

        }

        block_size = block_ptr->BLOCKSIZE;
        if (block_size >= requested_size)
        {


            /* request fits into this block */
            next_block_size = block_size - requested_size;
            if (next_block_size >= LWMEM_MIN_MEMORY_STORAGE_SIZE)
            {
                /*
                 * The current block is big enough to split.
                 * into 2 blocks.... the part to be allocated is one block,
                 * and the rest remains as a free block on the free list.
                 */
                next_block_ptr = (LWMEM_BLOCK_STRUCT_PTR) (void *) ((unsigned char *) block_ptr + requested_size);
                /* Initialize the new physical block values */
                next_block_ptr->BLOCKSIZE = next_block_size;
                /* Link new block into the free list */
                next_block_ptr->POOL = (void *) mem_pool_ptr;
                next_block_ptr->U.NEXTBLOCK = block_ptr->U.NEXTBLOCK;

                /* Modify the current block, to point to this newly created block*/
                block_ptr->BLOCKSIZE = requested_size;
            }
            else
            {
                /* Take the entire block */
                requested_size = block_size;
                next_block_ptr = block_ptr->U.NEXTBLOCK;
            } /* Endif */

            if (block_ptr == mem_pool_ptr->POOL_FREE_LIST_PTR)
            {
                /* At the head of the free list */
                mem_pool_ptr->POOL_FREE_LIST_PTR = next_block_ptr;
            }
            else
            {
                prev_block_ptr->U.NEXTBLOCK = next_block_ptr;
            } /* Endif */

            /* Indicate the block is in use */
            block_ptr->U.S.TASK_NUMBER = TASK_NUMBER_FROM_TASKID(td_ptr->TASK_ID);
            block_ptr->U.S.MEM_TYPE = 0;
            block_ptr->POOL = (_lwmem_pool_id) mem_pool_ptr;

            /* Indicate the highest memory address used */
            highwater = ((uint32_t) block_ptr) + block_ptr->BLOCKSIZE - 1;
            if (highwater > (uint32_t) mem_pool_ptr->HIGHWATER)
            {
                mem_pool_ptr->HIGHWATER = (void*) highwater;
            }

            if (zero)
            {
                _mem_zero((void *) ((unsigned char *) block_ptr + sizeof(LWMEM_BLOCK_STRUCT)), requested_size
                                - sizeof(LWMEM_BLOCK_STRUCT));
            } /* Endif */
            retval = (void *) ((unsigned char *) block_ptr + sizeof(LWMEM_BLOCK_STRUCT));
            break;
        }
        else
        {
            prev_block_ptr = block_ptr;
            block_ptr = block_ptr->U.NEXTBLOCK;
        } /* Endif */
    } /* Endwhile */

    mem_pool_ptr->POOL_ALLOC_PTR = mem_pool_ptr->POOL_FREE_LIST_PTR;
    mem_pool_ptr->POOL_FREE_PTR = mem_pool_ptr->POOL_FREE_LIST_PTR;
    mem_pool_ptr->POOL_TEST_PTR = mem_pool_ptr->POOL_FREE_LIST_PTR;
    mem_pool_ptr->POOL_TEST2_PTR = mem_pool_ptr->POOL_ALLOC_START_PTR;
    #if MQX_TASK_DESTRUCTION
    mem_pool_ptr->POOL_DESTROY_PTR = mem_pool_ptr->POOL_ALLOC_START_PTR;
    #endif

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

    _int_enable();

    if (retval == NULL)
    {
        _task_set_error(MQX_OUT_OF_MEMORY);
    }

    return (retval);

} /* Endbody */

/*!
 * \private
 *
 * \brief Allocates a block of memory for a task from the free list.
 *
 * \param[in] requested_size The size of the memory block.
 * \param[in] requested_addr The address of the memory block.
 * \param[in] td_ptr         The owner TD.
 * \param[in] pool_id        Which pool to allocate from.
 * \param[in] zero           Zero the memory after it is allocated.
 *
 * \return Pointer to the lightweight memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_LWMEM_POOL_INVALID (Memory pool to allocate from is invalid.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 */
void *_lwmem_alloc_at_internal
(
    _mem_size      requested_size,
    void          *requested_addr,
    TD_STRUCT_PTR  td_ptr,
    _lwmem_pool_id pool_id,
    bool        zero
)
{ /* Body */
    LWMEM_BLOCK_STRUCT_PTR  block_ptr;
	LWMEM_BLOCK_STRUCT_PTR  requested_block_ptr;
    LWMEM_BLOCK_STRUCT_PTR  next_block_ptr;
    LWMEM_BLOCK_STRUCT_PTR  prev_block_ptr = NULL;
    (void)                  prev_block_ptr; /* disable 'unused variable' warning */
    _mem_size               block_size;
    _mem_size               next_block_size, free_block_size;
    LWMEM_POOL_STRUCT_PTR   mem_pool_ptr = (LWMEM_POOL_STRUCT_PTR) pool_id;
    uint32_t                highwater;
    void*                   retval = NULL;

#if MQX_CHECK_VALIDITY
    if (mem_pool_ptr->VALID != LWMEM_POOL_VALID)
    {
        _task_set_error(MQX_LWMEM_POOL_INVALID);
        return (NULL);
    } /* Endif */
#endif

    /*
     * Adjust message size to allow for block management overhead
     * and force size to be aligned.
     */
    requested_size += (_mem_size) sizeof(LWMEM_BLOCK_STRUCT);
    if (requested_size < LWMEM_MIN_MEMORY_STORAGE_SIZE)
    {
        requested_size = LWMEM_MIN_MEMORY_STORAGE_SIZE;
    } /* Endif */

    _MEMORY_ALIGN_VAL_LARGER(requested_size);

    _int_disable();
    block_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR;
    while (block_ptr != NULL)
    {
        /* Provide window for higher priority tasks */
        mem_pool_ptr->POOL_ALLOC_PTR = block_ptr;

        _int_enable();
        _int_disable();



        /* Reset to the start if we were preempted
           and the preempting task did finish its free/malloc operation,
           a safe solution, even with time-slicing. */
        if(block_ptr != mem_pool_ptr->POOL_ALLOC_PTR)
        {
            block_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR;
            prev_block_ptr = block_ptr;
        }

        if (block_ptr == NULL)
        {
            /* The pool became full during preemption, no free block exists anymore */
            break;
        }

        block_size = block_ptr->BLOCKSIZE;
        if ((unsigned char *) block_ptr + LWMEM_MIN_MEMORY_STORAGE_SIZE <= (unsigned char *) requested_addr
                        - sizeof(LWMEM_BLOCK_STRUCT) && (unsigned char *) block_ptr + block_size
                        >= (unsigned char *) requested_addr - sizeof(LWMEM_BLOCK_STRUCT) + requested_size)
        {
            /* request fits into this block */

            requested_block_ptr = (LWMEM_BLOCK_STRUCT_PTR)((unsigned char *) requested_addr - sizeof(LWMEM_BLOCK_STRUCT));

            free_block_size = (unsigned char *) requested_block_ptr - (unsigned char *) block_ptr;
            /* requested block size is known */
            next_block_size = block_size - requested_size - free_block_size;

            /* free_block_size is always > LWMEM_MIN_MEMORY_STORAGE_SIZE */
            block_ptr->BLOCKSIZE = free_block_size;

            /* chek and prepare free block after requested block */
            if (next_block_size >= LWMEM_MIN_MEMORY_STORAGE_SIZE)
            {
                /*
                 * The current block is big enough to split.
                 * into 2 blocks.... the part to be allocated is one block,
                 * and the rest remains as a free block on the free list.
                 */
                next_block_ptr = (LWMEM_BLOCK_STRUCT_PTR) (void *) ((unsigned char *) requested_block_ptr + requested_size);
                /* Initialize the new physical block values */
                next_block_ptr->BLOCKSIZE = next_block_size;
                /* Link new block into the free list */
                next_block_ptr->POOL = (void *) mem_pool_ptr;
                next_block_ptr->U.NEXTBLOCK = block_ptr->U.NEXTBLOCK;
                block_ptr->U.NEXTBLOCK = (void *) next_block_ptr;
            }
            else
            {
                /* Take the entire block */
                requested_size += next_block_size;
                next_block_ptr = block_ptr->U.NEXTBLOCK;
            }

            /* first free block always stay first */

            /* add next free block to list */
            //prev_block_ptr->U.NEXTBLOCK = next_block_ptr;

            /* Modify the allocated block information to point to next block */
            requested_block_ptr->BLOCKSIZE = requested_size;
            requested_block_ptr->U.NEXTBLOCK = next_block_ptr;

            /* Indicate the block is in use */
            requested_block_ptr->U.S.TASK_NUMBER = TASK_NUMBER_FROM_TASKID(td_ptr->TASK_ID);
            requested_block_ptr->U.S.MEM_TYPE = 0;
            requested_block_ptr->POOL = (_lwmem_pool_id) mem_pool_ptr;

            /* Indicate the highest memory address used */
            highwater = ((uint32_t) block_ptr) + block_ptr->BLOCKSIZE - 1;
            if (highwater > (uint32_t) mem_pool_ptr->HIGHWATER)
            {
                mem_pool_ptr->HIGHWATER = (void*) highwater;
            }

            if (zero)
            {
                _mem_zero((void *) requested_addr, requested_size - sizeof(LWMEM_BLOCK_STRUCT));
            } /* Endif */
            retval = requested_addr;
            break;
        }
        else
        {
            prev_block_ptr = block_ptr;
            block_ptr = block_ptr->U.NEXTBLOCK;
        } /* Endif */
    } /* Endwhile */

    mem_pool_ptr->POOL_ALLOC_PTR = mem_pool_ptr->POOL_FREE_LIST_PTR;
    mem_pool_ptr->POOL_FREE_PTR = mem_pool_ptr->POOL_FREE_LIST_PTR;
    mem_pool_ptr->POOL_TEST_PTR = mem_pool_ptr->POOL_FREE_LIST_PTR;
    mem_pool_ptr->POOL_TEST2_PTR = mem_pool_ptr->POOL_ALLOC_START_PTR;
    #if MQX_TASK_DESTRUCTION
    mem_pool_ptr->POOL_DESTROY_PTR = mem_pool_ptr->POOL_ALLOC_START_PTR;
    #endif
    _int_enable();

    if (retval == NULL)
    {
        _task_set_error(MQX_OUT_OF_MEMORY);
    }
    return (retval);
}

/*!
 * \private
 *
 * \brief Allocate an aligned block of memory for a task from the free list.
 *
 * \param[in] requested_size The size of the memory block.
 * \param[in] req_align      Requested alignement (e.g. 8 for alignement to 8 bytes).
 * \param[in] td_ptr         The owner TD.
 * \param[in] pool_id        Which pool to allocate from.
 * \param[in] zero           Zero the memory after it is allocated.
 *
 * \return Pointer to the lightweight memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_LWMEM_POOL_INVALID (Memory pool to allocate from is invalid.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_INVALID_PARAMETER (Requested alignment is not power of 2.)
 */
void *_lwmem_alloc_align_internal
(
    _mem_size      requested_size,
    _mem_size      req_align,
    TD_STRUCT_PTR  td_ptr,
    _lwmem_pool_id pool_id,
    bool        zero
)
{ /* Body */
    LWMEM_BLOCK_STRUCT_PTR block_ptr;
    LWMEM_BLOCK_STRUCT_PTR next_block_ptr;
    LWMEM_BLOCK_STRUCT_PTR prev_block_ptr;
    _mem_size              block_size;
    _mem_size              next_block_size;
    _mem_size              shift;
    LWMEM_POOL_STRUCT_PTR  mem_pool_ptr = (LWMEM_POOL_STRUCT_PTR) pool_id;
    uint32_t               highwater;
    void*                  retval = NULL;

#if MQX_CHECK_ERRORS
    if (requested_size < LWMEM_MIN_MEMORY_STORAGE_SIZE) {
        requested_size = LWMEM_MIN_MEMORY_STORAGE_SIZE;
    }
   /* Check if reg_align is power of 2 */
    if ((req_align != 0) && (req_align & (req_align - 1))) {
        _task_set_error(MQX_INVALID_PARAMETER);
        return (NULL); /* request failed */
    }
    /* If aligment is less than PSP_MEMORY_ALIGNMENT correction is needed. */
    if (req_align <= PSP_MEMORY_ALIGNMENT) {
        req_align = (PSP_MEMORY_ALIGNMENT + 1);
    }
#endif

#if MQX_CHECK_VALIDITY
    if (mem_pool_ptr->VALID != LWMEM_POOL_VALID)
    {
        _task_set_error(MQX_LWMEM_POOL_INVALID);
        return (NULL);
    }
#endif

    /*
     * Adjust message size to allow for block management overhead
     * and force size to be aligned.
     */
    requested_size += (_mem_size) sizeof(LWMEM_BLOCK_STRUCT);
    if (requested_size < LWMEM_MIN_MEMORY_STORAGE_SIZE)
    {
        requested_size = LWMEM_MIN_MEMORY_STORAGE_SIZE;
    }

    _MEMORY_ALIGN_VAL_LARGER(requested_size);

    _int_disable();
    block_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR;
    while (block_ptr != NULL)
    {
        /* Provide window for higher priority tasks */
        mem_pool_ptr->POOL_ALLOC_PTR = block_ptr;
        _int_enable();
        _int_disable();


        /* Reset to the start if we were preempted
           and the preempting task did finish its free/malloc operation,
           a safe solution, even with time-slicing. */
        if(block_ptr != mem_pool_ptr->POOL_ALLOC_PTR)
        {
            block_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR;
            prev_block_ptr = block_ptr;
        }

        if (block_ptr == NULL)
        {
            /* The pool became full during preemption, no free block exists anymore */
            break;
        }

        shift = (((_mem_size) block_ptr + sizeof(LWMEM_BLOCK_STRUCT) + req_align) & ~(req_align - 1))
                        - ((_mem_size) block_ptr + sizeof(LWMEM_BLOCK_STRUCT));

        if (shift < (2 * LWMEM_MIN_MEMORY_STORAGE_SIZE))
        {
            shift = (((_mem_size) block_ptr + sizeof(LWMEM_BLOCK_STRUCT) + (3 * LWMEM_MIN_MEMORY_STORAGE_SIZE)
                            + req_align) & ~(req_align - 1)) - ((_mem_size) block_ptr + sizeof(LWMEM_BLOCK_STRUCT));
        }

        block_size = block_ptr->BLOCKSIZE;
        /* request fits into this block */
        if (block_size >= requested_size + shift)
        {
            /* create new free block */
            prev_block_ptr = block_ptr;
            block_ptr = (LWMEM_BLOCK_STRUCT_PTR)(((unsigned char *) block_ptr) + shift);
            block_size -= shift;
            prev_block_ptr->BLOCKSIZE = shift;
            block_ptr->U.NEXTBLOCK = prev_block_ptr->U.NEXTBLOCK;

            next_block_size = block_size - requested_size;
            if (next_block_size >= LWMEM_MIN_MEMORY_STORAGE_SIZE)
            {
                /*
                 * The current block is big enough to split.
                 * into 2 blocks.... the part to be allocated is one block,
                 * and the rest remains as a free block on the free list.
                 */
                next_block_ptr = (LWMEM_BLOCK_STRUCT_PTR) (void *) ((unsigned char *) block_ptr + requested_size);
                /* Initialize the new physical block values */
                next_block_ptr->BLOCKSIZE = next_block_size;
                /* Link new block into the free list */
                next_block_ptr->POOL = (void *) mem_pool_ptr;
                next_block_ptr->U.NEXTBLOCK = block_ptr->U.NEXTBLOCK;
                block_ptr->U.NEXTBLOCK = (void *) next_block_ptr;
            }
            else
            {
                /* Take the entire block */
                requested_size = block_size;
                next_block_ptr = block_ptr->U.NEXTBLOCK;
            } /* Endif */

            /* Modify the current block, to point to this newly created block*/
            block_ptr->BLOCKSIZE = requested_size;

            if (block_ptr == mem_pool_ptr->POOL_FREE_LIST_PTR)
            {
                /* At the head of the free list */
                mem_pool_ptr->POOL_FREE_LIST_PTR = next_block_ptr;
            }
            else
            {
                prev_block_ptr->U.NEXTBLOCK = next_block_ptr;
            } /* Endif */

            /* Indicate the block is in use */
            block_ptr->U.S.TASK_NUMBER = TASK_NUMBER_FROM_TASKID(td_ptr->TASK_ID);
            block_ptr->U.S.MEM_TYPE = 0;
            block_ptr->POOL = (_lwmem_pool_id) mem_pool_ptr;

            /* Indicate the highest memory address used */
            highwater = ((uint32_t) block_ptr) + block_ptr->BLOCKSIZE - 1;
            if (highwater > (uint32_t) mem_pool_ptr->HIGHWATER)
            {
                mem_pool_ptr->HIGHWATER = (void*) highwater;
            }

            if (zero)
            {
                _mem_zero((void *) ((unsigned char *) block_ptr + sizeof(LWMEM_BLOCK_STRUCT)), requested_size
                                - sizeof(LWMEM_BLOCK_STRUCT));
            }

            retval = (void *) ((unsigned char *) block_ptr + sizeof(LWMEM_BLOCK_STRUCT));
            break;
        }
        else
        {
            prev_block_ptr = block_ptr;
            block_ptr = block_ptr->U.NEXTBLOCK;
        }
    }

    mem_pool_ptr->POOL_ALLOC_PTR = mem_pool_ptr->POOL_FREE_LIST_PTR;
    mem_pool_ptr->POOL_FREE_PTR = mem_pool_ptr->POOL_FREE_LIST_PTR;
    mem_pool_ptr->POOL_TEST_PTR = mem_pool_ptr->POOL_FREE_LIST_PTR;
    mem_pool_ptr->POOL_TEST2_PTR = mem_pool_ptr->POOL_ALLOC_START_PTR;
    #if MQX_TASK_DESTRUCTION
    mem_pool_ptr->POOL_DESTROY_PTR = mem_pool_ptr->POOL_ALLOC_START_PTR;
    #endif
    _int_enable();

    if (retval == NULL)
    {
        _task_set_error(MQX_OUT_OF_MEMORY);
    }

    return (retval);
}

#if MQX_ENABLE_USER_MODE
/*!
 * \private
 *
 * \brief Allocates a block of memory.
 *
 * \param[in] requested_size The size of the memory block.
 *
 * \return Pointer to the lightweight memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_LWMEM_POOL_INVALID (Memory pool to allocate from is invalid.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 */
void *_usr_lwmem_alloc_internal
(
    _mem_size requested_size
)
{
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_usr_lwmem_alloc, requested_size);

    result = _lwmem_alloc_internal(requested_size, kernel_data->ACTIVE_PTR,
                    (_lwmem_pool_id)kernel_data->KD_USER_POOL, FALSE);

    _KLOGX2(KLOG_usr_lwmem_alloc, result);
    return(result);
}

#endif /* MQX_ENABLE_USER_MODE */

/*!
 * \brief Allocates a private block of lightweight memory block from the default
 * memory pool.
 *
 * The application must first set a value for the default lightweight memory pool
 * by calling _lwmem_set_default_pool().
 * \n The _lwmem_alloc functions allocate at least size single-addressable units;
 * the actual number might be greater. The start address of the block is aligned
 * so that tasks can use the returned pointer as a pointer to any data type without
 * causing an error.
 * \n Tasks cannot use lightweight memory blocks as messages. Tasks must use
 * _msg_alloc() or _msg_alloc_system() to allocate messages.
 * \n Only the task that owns a lightweight memory block that was allocated with
 * one of the following functions can free the block:
 * \li _lwmem_alloc()
 * \li _lwmem_alloc_zero()
 * \li _lwmem_alloc_at()
 *
 * \n Any task can free a lightweight memory block that is allocated with one of
 * the following functions:
 * \li _lwmem_alloc_system()
 * \li _lwmem_alloc_system_zero()
 * \li _lwmem_alloc_system_align()
 *
 * \n Function types:
 * <table>
 *  <tr>
 *    <td></td>
 *    <td><b>Allocate this type of lighweight memory block form the default memory
 *    pool:</b></td>
 *  </tr>
 *  <tr>
 *    <td><b>_lwmem_alloc()</b></td>
 *    <td>Private</td>
 *  </tr>
 *  <tr>
 *    <td><b>_lwmem_alloc_system()</b></td>
 *    <td>System</td>
 *  </tr>
 *  <tr>
 *    <td><b>_lwmem_alloc_system_zero()</b></td>
 *    <td>System (zero-filled)</td>
 *  </tr>
 *  <tr>
 *    <td><b>_lwmem_alloc_zero()</b></td>
 *    <td>Private (zero-filled)</td>
 *  </tr>
 *  <tr>
 *    <td><b>_lwmem_alloc_at()</b></td>
 *    <td>Private (start address defined)</td>
 *  </tr>
 *  <tr>
 *    <td><b>_lwmem_alloc_system_align()</b></td>
 *    <td>System (aligned) </td>
 *  </tr>
 *  <tr>
 *    <td><b>_lwmem_alloc_align()</b></td>
 *    <td>Private (aligned) </td>
 *  </tr>
 * </table>
 *
 * \param[in] requested_size Number of single-addressable units to allocate.
 *
 * \return Pointer to the lightweight memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_LWMEM_POOL_INVALID (Memory pool to allocate from is invalid.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 *
 * \see _lwmem_alloc_system
 * \see _lwmem_alloc_system_zero
 * \see _lwmem_alloc_zero
 * \see _lwmem_alloc_at
 * \see _lwmem_alloc_system_align
 * \see _lwmem_alloc_align
 * \see _lwmem_alloc_system_align_from
 * \see _lwmem_alloc_align_from
 * \see _lwmem_alloc_from
 * \see _lwmem_alloc_system_from
 * \see _lwmem_alloc_system_zero_from
 * \see _lwmem_alloc_zero_from
 * \see _lwmem_realloc
 * \see _lwmem_create_pool
 * \see _lwmem_free
 * \see _lwmem_get_size
 * \see _lwmem_set_default_pool
 * \see _lwmem_transfer
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error
 */
void *_lwmem_alloc
(
    _mem_size requested_size
)
{
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

#if MQX_ENABLE_USER_MODE && MQX_ENABLE_USER_STDAPI
    if (MQX_RUN_IN_USER_MODE)
    {
        return _usr_lwmem_alloc(requested_size);
    }
#endif

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_lwmem_alloc, requested_size);

    result = _lwmem_alloc_internal(requested_size, kernel_data->ACTIVE_PTR,
                    (_lwmem_pool_id) kernel_data->KERNEL_LWMEM_POOL, FALSE);

    _KLOGX2(KLOG_lwmem_alloc, result);
    return (result);
}

#if MQX_ENABLE_USER_MODE
/*!
 * /brief Allocates a private block of lightweight memory block from the default
 * memory pool.
 *
 * This function is an equivalent to the _lwmem_alloc() API call but it can be
 * executed from within the User task or other code running in the CPU User mode.
 * Parameters passed to this function by pointer are required to meet the memory
 * protection requirements as described in the parameter list below.
 *
 * \param[in] requested_size Number of single-addressable units to allocate.
 *
 * \return Pointer to the lightweight memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_LWMEM_POOL_INVALID (Memory pool to allocate from is invalid.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 *
 * \see _lwmem_alloc
 * \see _usr_lwmem_alloc_from
 * \see _usr_lwmem_create_pool
 * \see _usr_lwmem_free
 */
void *_usr_lwmem_alloc
(
    _mem_size requested_size
)
{
    MQX_API_CALL_PARAMS params =
    {   (uint32_t)requested_size, 0, 0, 0, 0};
    return (void *)_mqx_api_call(MQX_API_LWMEM_ALLOC, &params);
}

#endif /* MQX_ENABLE_USER_MODE */

/*!
 * \brief Allocates a private block (with defined start address) of
 * lightweight memory block from the default memory pool.
 *
 * See Description of _lwmem_alloc() function.
 *
 * \param[in] requested_size Number of single-addressable units to allocate.
 * \param[in] requested_addr Start address of the memory block.
 *
 * \return Pointer to the lightweight memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_LWMEM_POOL_INVALID (Memory pool to allocate from is invalid.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 *
 * \see _lwmem_alloc
 * \see _lwmem_alloc_system
 * \see _lwmem_alloc_system_zero
 * \see _lwmem_alloc_zero
 * \see _lwmem_alloc_system_align
 * \see _lwmem_alloc_align
 * \see _lwmem_alloc_system_align_from
 * \see _lwmem_alloc_align_from
 * \see _lwmem_alloc_from
 * \see _lwmem_alloc_system_from
 * \see _lwmem_alloc_system_zero_from
 * \see _lwmem_alloc_zero_from
 * \see _lwmem_realloc
 * \see _lwmem_create_pool
 * \see _lwmem_free
 * \see _lwmem_get_size
 * \see _lwmem_set_default_pool
 * \see _lwmem_transfer
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error
 */
void *_lwmem_alloc_at
(
    _mem_size requested_size,
    void     *requested_addr
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_lwmem_alloc_at, requested_size);

    result = _lwmem_alloc_at_internal(requested_size, requested_addr, kernel_data->ACTIVE_PTR,
                    (_lwmem_pool_id) kernel_data->KERNEL_LWMEM_POOL, FALSE);

    _KLOGX2(KLOG_lwmem_alloc_at, result);
    return (result);
}

/*!
 * \brief Allocates an aligned block of lightweight memory block from the default
 * memory pool.
 *
 * See Description of _lwmem_alloc() function.
 *
 * \param[in] requested_size Number of single-addressable units to allocate.
 * \param[in] req_align      Align requested value.
 *
 * \return Pointer to the lightweight memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_LWMEM_POOL_INVALID (Memory pool to allocate from is invalid.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_INVALID_PARAMETER (Requested alignment is not power of 2.)
 *
 * \see _lwmem_alloc
 * \see _lwmem_alloc_system
 * \see _lwmem_alloc_system_zero
 * \see _lwmem_alloc_zero
 * \see _lwmem_alloc_at
 * \see _lwmem_alloc_system_align
 * \see _lwmem_alloc_system_align_from
 * \see _lwmem_alloc_align_from
 * \see _lwmem_alloc_from
 * \see _lwmem_alloc_system_from
 * \see _lwmem_alloc_system_zero_from
 * \see _lwmem_alloc_zero_from
 * \see _lwmem_realloc
 * \see _lwmem_create_pool
 * \see _lwmem_free
 * \see _lwmem_get_size
 * \see _lwmem_set_default_pool
 * \see _lwmem_transfer
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error
 */
void *_lwmem_alloc_align
(
    _mem_size requested_size,
    _mem_size req_align
)
{
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE3(KLOG_lwmem_alloc_align, requested_size, req_align);

    result = _lwmem_alloc_align_internal(requested_size, req_align, kernel_data->ACTIVE_PTR,
                    (_lwmem_pool_id) kernel_data->KERNEL_LWMEM_POOL, FALSE);

    _KLOGX2(KLOG_lwmem_alloc_align, result);

    return (result);
}

/*!
 * \brief Gets default system lwmem pool.
 *
 * \return Pointer to default szstem lwmem pool.
 * \return NULL (failure)
 */
_lwmem_pool_id _lwmem_get_system_pool_id(void)
{
    register KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);

    return (_lwmem_pool_id) kernel_data->KERNEL_LWMEM_POOL;
}

/*!
 * \brief Allocates a private block of lightweight memory block from the specified
 * memory pool.
 *
 * The function is similar to _lwmem_alloc(), _lwmem_alloc_system(),
 * _lwmem_alloc_system_zero() and _lwmem_alloc_zero() except that the application
 * does not call _lwmem_set_default_pool() first.
 * \n Only the task that owns a lightweight memory block that was allocated with
 * one of the following functions can free the block:
 * \li _lwmem_alloc_from()
 * \li _lwmem_alloc_zero_from()
 *
 * \n Any task can free a lightweight memory block that is allocated with one of
 * the following functions:
 * \li _lwmem_alloc_system_from()
 * \li _lwmem_alloc_system_zero_from()
 * \li _lwmem_alloc_system_align_from()
 *
 * \n Function types:
 * <table>
 *  <tr>
 *    <td></td>
 *    <td><b>Allocate this type of lighweight memory block form the specified
 *    lightweight memory pool:</b></td>
 *  </tr>
 *  <tr>
 *    <td><b>_lwmem_alloc_from()</b></td>
 *    <td>Private</td>
 *  </tr>
 *  <tr>
 *    <td><b>_lwmem_alloc_system_from()</b></td>
 *    <td>System</td>
 *  </tr>
 *  <tr>
 *    <td><b>_lwmem_alloc_system_zero_from()</b></td>
 *    <td>System (zero-filled)</td>
 *  </tr>
 *  <tr>
 *    <td><b>_lwmem_alloc_zero_from()</b></td>
 *    <td>Private (zero-filled)</td>
 *  </tr>
 *  <tr>
 *    <td><b>_lwmem_alloc_system_align_from()</b></td>
 *    <td>System (aligned) </td>
 *  </tr>
 *  <tr>
 *    <td><b>_lwmem_alloc_align_from()</b></td>
 *    <td>Private (aligned) </td>
 *  </tr>
 * </table>
 *
 * \param[in] pool_id        Lightweight memory pool from which to allocate the
 * lightweight memory block (from _lwmem_create_pool()).
 * \param[in] requested_size Number of single-addressable units to allocate.
 *
 * \return Pointer to the lightweight memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_LWMEM_POOL_INVALID (Memory pool to allocate from is invalid.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 *
 * \see _lwmem_alloc
 * \see _lwmem_alloc_system
 * \see _lwmem_alloc_system_zero
 * \see _lwmem_alloc_zero
 * \see _lwmem_alloc_at
 * \see _lwmem_alloc_system_align
 * \see _lwmem_alloc_align
 * \see _lwmem_alloc_system_align_from
 * \see _lwmem_alloc_align_from
 * \see _lwmem_alloc_system_from
 * \see _lwmem_alloc_system_zero_from
 * \see _lwmem_alloc_zero_from
 * \see _lwmem_realloc
 * \see _lwmem_create_pool
 * \see _lwmem_free
 * \see _lwmem_transfer
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error
 */
void *_lwmem_alloc_from
(
    _lwmem_pool_id pool_id,
    _mem_size      requested_size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

#if MQX_ENABLE_USER_MODE && MQX_ENABLE_USER_STDAPI
    if (MQX_RUN_IN_USER_MODE)
    {
        return _usr_lwmem_alloc_from(pool_id, requested_size);
    }
#endif

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE3(KLOG_lwmem_alloc_from, pool_id, requested_size);

    result = _lwmem_alloc_internal(requested_size, kernel_data->ACTIVE_PTR, pool_id, FALSE);

    _KLOGX2(KLOG_lwmem_alloc_from, result);
    return (result);
}

#if MQX_ENABLE_USER_MODE
/*!
 * \brief Allocates a private block of lightweight memory block from the specified
 * memory pool.
 *
 * This function is an equivalent to the _lwmem_alloc_from() API call but it can
 * be executed from within the User task or other code running in the CPU User
 * mode. Parameters passed to this function by pointer are required to meet the
 * memory protection requirements as described in the parameter list below.
 *
 * \param[in] pool_id        Read/write. Lightweight memory pool from which to
 * allocate the lightweight memory block (pool created with _usr_lwmem_create_pool()
 * or ordinary lightweight memory pool for which the user-mode access has been
 * enabled by calling _watchdog_create_component()).
 * \param[in] requested_size Number of single-addressable units to allocate.
 *
 * \return Pointer to the lightweight memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_LWMEM_POOL_INVALID (Memory pool to allocate from is invalid.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 *
 * \see _lwmem_alloc_from
 * \see _watchdog_create_component
 * \see _usr_lwmem_alloc
 * \see _usr_lwmem_create_pool
 * \see _usr_lwmem_free
 */
void *_usr_lwmem_alloc_from
(
    _lwmem_pool_id pool_id,
    _mem_size      requested_size
)
{
    MQX_API_CALL_PARAMS params =
    {   (uint32_t)pool_id, (uint32_t)requested_size, 0, 0, 0};
    return (void *)_mqx_api_call(MQX_API_LWMEM_ALLOC_FROM, &params);
}

#endif /* MQX_ENABLE_USER_MODE */

/*!
 * \brief Allocates a private block of lightweight memory block from the specified
 * memory pool.
 *
 * See Description of _lwmem_alloc_from() function.
 *
 * \param[in] pool_id        Lightweight memory pool from which to allocate the
 * lightweight memory block (from _lwmem_create_pool()).
 * \param[in] requested_size Number of single-addressable units to allocate.
 * \param[in] req_align      Align requested value.
 *
 * \return Pointer to the lightweight memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_LWMEM_POOL_INVALID (Memory pool to allocate from is invalid.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_INVALID_PARAMETER (Requested alignment is not power of 2.)
 *
 * \see _lwmem_alloc
 * \see _lwmem_alloc_system
 * \see _lwmem_alloc_system_zero
 * \see _lwmem_alloc_zero
 * \see _lwmem_alloc_at
 * \see _lwmem_alloc_system_align
 * \see _lwmem_alloc_align
 * \see _lwmem_alloc_system_align_from
 * \see _lwmem_alloc_from
 * \see _lwmem_alloc_system_from
 * \see _lwmem_alloc_system_zero_from
 * \see _lwmem_alloc_zero_from
 * \see _lwmem_realloc
 * \see _lwmem_create_pool
 * \see _lwmem_free
 * \see _lwmem_transfer
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error
 */
void *_lwmem_alloc_align_from
(
    _lwmem_pool_id pool_id,
    _mem_size      requested_size,
    _mem_size      req_align
)
{
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE4(KLOG_lwmem_alloc_align_from, pool_id, requested_size, req_align);

    result = _lwmem_alloc_align_internal(requested_size, req_align, kernel_data->ACTIVE_PTR, pool_id, FALSE);

    _KLOGX2(KLOG_lwmem_alloc_align_from, result);

    return (result);
}

/*!
 * \brief Allocates a private (zero-filled) block of lightweight memory block from
 * the default memory pool.
 *
 * See Description of _lwmem_alloc() function.
 *
 * \param[in] size Number of single-addressable units to allocate.
 *
 * \return Pointer to the lightweight memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_LWMEM_POOL_INVALID (Memory pool to allocate from is invalid.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 *
 * \see _lwmem_alloc
 * \see _lwmem_alloc_system
 * \see _lwmem_alloc_system_zero
 * \see _lwmem_alloc_at
 * \see _lwmem_alloc_system_align
 * \see _lwmem_alloc_align
 * \see _lwmem_alloc_system_align_from
 * \see _lwmem_alloc_align_from
 * \see _lwmem_alloc_from
 * \see _lwmem_alloc_system_from
 * \see _lwmem_alloc_system_zero_from
 * \see _lwmem_alloc_zero_from
 * \see _lwmem_realloc
 * \see _lwmem_create_pool
 * \see _lwmem_free
 * \see _lwmem_get_size
 * \see _lwmem_set_default_pool
 * \see _lwmem_transfer
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error
 */
void *_lwmem_alloc_zero
(
    _mem_size size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_lwmem_alloc_zero, size);

    result = _lwmem_alloc_internal(
        size, kernel_data->ACTIVE_PTR, (_lwmem_pool_id) kernel_data->KERNEL_LWMEM_POOL, TRUE
    );

    _KLOGX2(KLOG_lwmem_alloc_zero, result);
    return (result);

} /* Endbody */

/*!
 * \brief Allocates a private (zero-filled) block of lightweight memory block from the specified
 * memory pool.
 *
 * See Description of _lwmem_alloc_from() function.
 *
 * \param[in] pool_id        Lightweight memory pool from which to allocate the
 * lightweight memory block (from _lwmem_create_pool()).
 * \param[in] size Number of single-addressable units to allocate.
 *
 * \return Pointer to the lightweight memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_LWMEM_POOL_INVALID (Memory pool to allocate from is invalid.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 *
 * \see _lwmem_alloc
 * \see _lwmem_alloc_system
 * \see _lwmem_alloc_system_zero
 * \see _lwmem_alloc_zero
 * \see _lwmem_alloc_at
 * \see _lwmem_alloc_system_align
 * \see _lwmem_alloc_align
 * \see _lwmem_alloc_system_align_from
 * \see _lwmem_alloc_align_from
 * \see _lwmem_alloc_from
 * \see _lwmem_alloc_system_from
 * \see _lwmem_alloc_system_zero_from
 * \see _lwmem_realloc
 * \see _lwmem_create_pool
 * \see _lwmem_free
 * \see _lwmem_transfer
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error
 */
void *_lwmem_alloc_zero_from
(
    void     *pool_id,
    _mem_size size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE3(KLOG_lwmem_alloc_zero_from, pool_id, size);

    result = _lwmem_alloc_internal(size, kernel_data->ACTIVE_PTR, pool_id, TRUE);

    _KLOGX2(KLOG_lwmem_alloc_zero_from, result);
    return (result);

} /* Endbody */

/*!
 * \brief Creates the lightweight memory pool from memory that is outside the
 * default memory pool.
 *
 * Tasks use the pool ID to allocate (variable-size) lightweight memory blocks
 * from the pool.
 *
 * \param[in] mem_pool_ptr Pointer to the definition of the pool.
 * \param[in] start        Start of the memory for the pool.
 * \param[in] size         Number of single-addressable units in the pool.
 *
 * \return Pool ID
 *
 * \see _lwmem_alloc
 * \see _lwmem_alloc_system
 * \see _lwmem_alloc_system_zero
 * \see _lwmem_alloc_zero
 * \see _lwmem_alloc_at
 * \see _lwmem_alloc_system_align
 * \see _lwmem_alloc_align
 * \see _lwmem_alloc_system_align_from
 * \see _lwmem_alloc_align_from
 * \see _lwmem_alloc_from
 * \see _lwmem_alloc_system_from
 * \see _lwmem_alloc_system_zero_from
 * \see _lwmem_alloc_zero_from
 * \see _lwmem_realloc
 */
_lwmem_pool_id _lwmem_create_pool
(
    LWMEM_POOL_STRUCT_PTR mem_pool_ptr,
    void                 *start,
    _mem_size             size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    LWMEM_BLOCK_STRUCT_PTR block_ptr;
    unsigned char              *end;

#if MQX_ENABLE_USER_MODE && MQX_ENABLE_USER_STDAPI
    if (MQX_RUN_IN_USER_MODE)
    {
        return _usr_lwmem_create_pool(mem_pool_ptr, start, size);
    }
#endif

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE3(KLOG_lwmem_create_pool, start, size);

    /* Set the end of memory (aligned) */
    end = (unsigned char *) start + size;
    mem_pool_ptr->POOL_ALLOC_END_PTR = (void *) _ALIGN_ADDR_TO_LOWER_MEM(end);

    /* Align the start of the pool */
    block_ptr = (LWMEM_BLOCK_STRUCT_PTR)_ALIGN_ADDR_TO_HIGHER_MEM(start);
    mem_pool_ptr->POOL_ALLOC_START_PTR = (void *) block_ptr;
    mem_pool_ptr->HIGHWATER = (void *) block_ptr;

    /* Set up the first block as an idle block */
    block_ptr->BLOCKSIZE = (unsigned char *) mem_pool_ptr->POOL_ALLOC_END_PTR - (unsigned char *) block_ptr;
    block_ptr->U.NEXTBLOCK = NULL;
    block_ptr->POOL = (void *) mem_pool_ptr;
    mem_pool_ptr->POOL_FREE_LIST_PTR = block_ptr;
    mem_pool_ptr->POOL_ALLOC_PTR = block_ptr;
    mem_pool_ptr->POOL_FREE_PTR = block_ptr;
    mem_pool_ptr->POOL_TEST_PTR = block_ptr;

    /* Protect the list of pools while adding new pool */
    _int_disable();
    if (kernel_data->LWMEM_POOLS.NEXT == NULL)
    {
        /* Initialize the light weight memory */
        _QUEUE_INIT(&kernel_data->LWMEM_POOLS, 0);
    } /* Endif */
    _QUEUE_ENQUEUE(&kernel_data->LWMEM_POOLS, &mem_pool_ptr->LINK);
    _int_enable();
    mem_pool_ptr->VALID = LWMEM_POOL_VALID;

    _KLOGX2(KLOG_lwmem_create_pool, mem_pool_ptr);
    return ((_lwmem_pool_id) mem_pool_ptr);

}

#if MQX_ENABLE_USER_MODE
/*!
 * \brief Sets lightweight memory pool access rights for User-mode tasks.
 *
 * This function sets access rights for a (lightweight) memory pool. Setting
 * correct access rights is important for tasks and other code running in the
 * User-mode. User-mode access to a memory pool whose access rights are not set
 * properly causes memory protection exception to be risen.
 *
 * \param[in] mem_pool_id Lightweight memory pool for access rights to set
 * (returned by _lwmem_create_pool()).
 * \param[in] access      Access rights to set. Possible values:
 * \li POOL_USER_RW_ACCESS
 * \li POOL_USER_RO_ACCESS
 * \li POOL_USER_NO_ACCESS
 *
 * \return MQX_OK
 */

_mqx_uint _mem_set_pool_access
(
    _lwmem_pool_id mem_pool_id,
    uint32_t        access
)
{
    _mqx_uint res = MQX_LWMEM_POOL_INVALID;
    LWMEM_POOL_STRUCT_PTR mem_pool_ptr = (_lwmem_pool_id)mem_pool_id;

    if (LWMEM_POOL_VALID == mem_pool_ptr->VALID)
    {
        res = _psp_mpu_add_region(mem_pool_ptr->POOL_ALLOC_START_PTR, mem_pool_ptr->POOL_ALLOC_END_PTR, access);
    }

    return res;
}

/*!
 * \brief Creates the lightweight memory pool from memory that is outside the
 * default memory pool.
 *
 * This function is an equivalent to the _lwmem_create_pool() API call but
 * it can be executed from within the User task or other code running in the CPU
 * User mode. Parameters passed to this function by pointer are required to meet
 * the memory protection requirements as described in the parameter list below.
 *
 * \param[in] mem_pool_ptr Read/write. Pointer to the definition of the pool.
 * \param[in] start        Start of the memory for the pool.
 * \param[in] size         Number of single-addressable units in the pool.
 *
 * \return Pool ID
 *
 * \see _lwmem_create_pool
 * \see _usr_lwmem_alloc
 * \see _usr_lwmem_alloc_from
 * \see _usr_lwmem_free
 */
_lwmem_pool_id _usr_lwmem_create_pool
(
    LWMEM_POOL_STRUCT_PTR mem_pool_ptr,
    void                 *start,
    _mem_size             size
)
{
    MQX_API_CALL_PARAMS params =
    {   (uint32_t)mem_pool_ptr, (uint32_t)start, (uint32_t)size, 0, 0};
    return (_lwmem_pool_id)_mqx_api_call(MQX_API_LWMEM_CREATE_POOL, &params);
}

#endif /* MQX_ENABLE_USER_MODE */

/*!
 * \brief Initializes a memory storage pool. Will set task error code if error occurs.
 *
 * \param[in] start The start of the memory pool.
 * \param[in] size  The size of the memory pool.
 *
 * \return A handle to the memory pool.
 */
_lwmem_pool_id _lwmem_create_pool_mapped
(
    void     *start,
    _mem_size size
)
{ /* Body */
    LWMEM_POOL_STRUCT_PTR mem_pool_ptr;

    mem_pool_ptr = (LWMEM_POOL_STRUCT_PTR)_ALIGN_ADDR_TO_HIGHER_MEM(start);
    _mem_zero((void *) mem_pool_ptr, (_mem_size) sizeof(LWMEM_POOL_STRUCT));

    start = (void *) ((unsigned char *) mem_pool_ptr + sizeof(LWMEM_POOL_STRUCT));
    _lwmem_create_pool(mem_pool_ptr, start, size - sizeof(LWMEM_POOL_STRUCT)); /* real pool size is decreased by pool header struct (must be - memory overwrite) */

    return ((_mem_pool_id) mem_pool_ptr);

} /* Endbody */

/*!
 * \brief Frees the lightweight memory block.
 *
 * If the block was allocated with one of the following functions, only the task
 * that owns the block can free it:
 * \li _lwmem_alloc()
 * \li _lwmem_alloc_from()
 * \li _lwmem_alloc_zero()
 * \li _lwmem_alloc_zero_from()
 *
 * \n Any task can free a block that was allocated with one of the following functions:
 * \li _lwmem_alloc_system()
 * \li _lwmem_alloc_system_from()
 * \li _lwmem_alloc_system_zero()
 * \li _lwmem_alloc_system_zero_from()
 * \li _lwmem_alloc_system_align()
 * \li _lwmem_alloc_system_align_from()
 *
 * \n The function also coalesces any free block found physically on either side
 * of the block being freed. If coalescing is not possible, then the block is
 * placed onto the free list.
 *
 * \param[in] mem_ptr Pointer to the block to free.
 *
 * \return MQX_OK
 * \return MQX_INVALID_POINTER (mem_ptr is NULL.)
 * \return MQX_LWMEM_POOL_INVALID (Pool that contains the block is not valid.)
 * \return MQX_NOT_RESOURCE_OWNER (If the block was allocated with _lwmem_alloc()
 * or _lwmem_alloc_zero(), only the task that allocated it can free part of it.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following
 * task error codes:
 * \li MQX_INVALID_POINTER (mem_ptr is NULL.)
 * \li MQX_LWMEM_POOL_INVALID (Pool that contains the block is not valid.)
 * \li MQX_NOT_RESOURCE_OWNER (If the block was allocated with _lwmem_alloc() or
 * _lwmem_alloc_zero(), only the task that allocated it can free part of it.)
 *
 * \see _lwmem_alloc
 * \see _lwmem_alloc_system
 * \see _lwmem_alloc_system_zero
 * \see _lwmem_alloc_zero
 * \see _lwmem_alloc_at
 * \see _lwmem_alloc_system_align
 * \see _lwmem_alloc_align
 * \see _lwmem_alloc_system_align_from
 * \see _lwmem_alloc_align_from
 * \see _lwmem_alloc_from
 * \see _lwmem_alloc_system_from
 * \see _lwmem_alloc_system_zero_from
 * \see _lwmem_alloc_zero_from
 * \see _lwmem_realloc
 * \see _lwmem_free
 * \see _task_set_error
 */
_mqx_uint _lwmem_free
(
    void   *mem_ptr
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR  kernel_data = NULL;
    (void)                  kernel_data; /* suppress 'unused variable' warning */
    LWMEM_BLOCK_STRUCT_PTR  block_ptr;
    LWMEM_BLOCK_STRUCT_PTR  free_list_ptr;
    LWMEM_POOL_STRUCT_PTR   mem_pool_ptr;
    bool                    insert;

#if MQX_ENABLE_USER_MODE && MQX_ENABLE_USER_STDAPI
    if (MQX_RUN_IN_USER_MODE)
    {
        return _usr_lwmem_free(mem_ptr);
    }
#endif

    _GET_KERNEL_DATA(kernel_data);

    _KLOGE2(KLOG_lwmem_free, mem_ptr);

#if MQX_CHECK_ERRORS
    /* Verify the passed in parameter */
    if (mem_ptr == NULL)
    {
        _task_set_error(MQX_INVALID_POINTER);
        _KLOGX2(KLOG_lwmem_free, MQX_INVALID_POINTER);
        return (MQX_INVALID_POINTER);
    } /* Endif */
#endif

    block_ptr = GET_LWMEMBLOCK_PTR(mem_ptr);
    mem_pool_ptr = (LWMEM_POOL_STRUCT_PTR) block_ptr->POOL;
#if MQX_CHECK_VALIDITY
    if (mem_pool_ptr->VALID != LWMEM_POOL_VALID)
    {
        _task_set_error(MQX_LWMEM_POOL_INVALID);
        _KLOGX2(KLOG_lwmem_free, MQX_LWMEM_POOL_INVALID);
        return (MQX_LWMEM_POOL_INVALID);
    } /* Endif */
#endif

#if MQX_CHECK_ERRORS
    /* Verify the passed in parameter */
    if (!((block_ptr->U.S.TASK_NUMBER == TASK_NUMBER_FROM_TASKID(kernel_data->ACTIVE_PTR->TASK_ID))
                    || (block_ptr->U.S.TASK_NUMBER == SYSTEM_TASK_NUMBER)))
    {
        _task_set_error(MQX_NOT_RESOURCE_OWNER);
        return (MQX_NOT_RESOURCE_OWNER);
    } /* Endif */
#endif

    _int_disable();

#if MQX_MEM_MONITOR
    kernel_data->MEM_USED -= block_ptr->BLOCKSIZE;
#endif

    free_list_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR;
    while (TRUE)
    {
        /*  We are at the beginning */
        if ((mem_pool_ptr->POOL_FREE_LIST_PTR == NULL) || ((void *) block_ptr < mem_pool_ptr->POOL_FREE_LIST_PTR))
        {
            free_list_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR;
            /* Working with block just before the free list? */
            if (((unsigned char *) block_ptr + block_ptr->BLOCKSIZE) == (unsigned char *) free_list_ptr)
            {
                /* Join with the next block on the list */
                block_ptr->BLOCKSIZE += free_list_ptr->BLOCKSIZE;
                block_ptr->U.NEXTBLOCK = free_list_ptr->U.NEXTBLOCK;
                /* The merged block should have cleared header to assure failure at multiple freeing with the same pointer */
                _mem_zero(free_list_ptr, sizeof(LWMEM_BLOCK_STRUCT));
            }
            else
            {
                block_ptr->U.NEXTBLOCK = free_list_ptr;
            } /* Endif */
            mem_pool_ptr->POOL_FREE_LIST_PTR = block_ptr;
            insert = FALSE;
            break;
        }/* Endif */
        if (((void *) block_ptr < free_list_ptr->U.NEXTBLOCK) || (free_list_ptr->U.NEXTBLOCK == NULL))
        {
            insert = TRUE;
            break;
        }/* Endif */
        free_list_ptr = free_list_ptr->U.NEXTBLOCK;
        /* Provide window for higher priority tasks */
        mem_pool_ptr->POOL_FREE_PTR = free_list_ptr;
        _int_enable();
        _int_disable();
        if(free_list_ptr != mem_pool_ptr->POOL_FREE_PTR)
        {
           free_list_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR;
        }
    } /* Endwhile */

    if (insert)
    {
        /*
         * We are between the two blocks where we are to be inserted,
         * free_list_ptr is before block_ptr,
         * block_ptr may be at end of list.
         */
        if (((unsigned char *) block_ptr + block_ptr->BLOCKSIZE) == free_list_ptr->U.NEXTBLOCK)
        {
            /* Join with the next block on the list */
            block_ptr->BLOCKSIZE += ((LWMEM_BLOCK_STRUCT_PTR)(free_list_ptr->U.NEXTBLOCK))->BLOCKSIZE;
            block_ptr->U.NEXTBLOCK = ((LWMEM_BLOCK_STRUCT_PTR)(free_list_ptr->U.NEXTBLOCK))->U.NEXTBLOCK;
            /* The merged block should have cleared header to assure failure at multiple freeing with the same pointer */
            _mem_zero(free_list_ptr->U.NEXTBLOCK, sizeof(LWMEM_BLOCK_STRUCT));
        }
        else
        {
            /* this block is to be inserted */
            block_ptr->U.NEXTBLOCK = free_list_ptr->U.NEXTBLOCK;
        } /* Endif */

        if (((unsigned char *) free_list_ptr + free_list_ptr->BLOCKSIZE == (unsigned char *) block_ptr))
        {
            free_list_ptr->BLOCKSIZE += block_ptr->BLOCKSIZE;
            free_list_ptr->U.NEXTBLOCK = block_ptr->U.NEXTBLOCK;
            /* The merged block should have cleared header to assure failure at multiple freeing with the same pointer */
            _mem_zero(block_ptr, sizeof(LWMEM_BLOCK_STRUCT));
        }
        else
        {
            free_list_ptr->U.NEXTBLOCK = block_ptr;
        } /* Endif */

    } /* Endif */

    mem_pool_ptr->POOL_ALLOC_PTR = mem_pool_ptr->POOL_FREE_LIST_PTR;
    mem_pool_ptr->POOL_FREE_PTR = mem_pool_ptr->POOL_FREE_LIST_PTR;
    mem_pool_ptr->POOL_TEST_PTR = mem_pool_ptr->POOL_FREE_LIST_PTR;
    mem_pool_ptr->POOL_TEST2_PTR = mem_pool_ptr->POOL_ALLOC_START_PTR;
#if MQX_TASK_DESTRUCTION
    mem_pool_ptr->POOL_DESTROY_PTR = mem_pool_ptr->POOL_ALLOC_START_PTR;
#endif
    _int_enable();

    _KLOGX2(KLOG_lwmem_free, MQX_OK);
    return (MQX_OK);

} /* Endbody */

#if MQX_ENABLE_USER_MODE
/*!
 * \brief Frees the given block of memory.
 *
 * This function is an equivalent to the _lwmem_free() API call but it can be
 * executed from within the User task or other code running in the CPU User mode.
 * Parameters passed to this function by pointer are required to meet the memory
 * protection requirements as described in the parameter list below.
 *
 * \param[in] mem_ptr Pointer to the block to free.
 *
 * \return MQX_OK
 * \return MQX_INVALID_POINTER (mem_ptr is NULL.)
 * \return MQX_LWMEM_POOL_INVALID (Pool that contains the block is not valid.)
 * \return MQX_NOT_RESOURCE_OWNER (If the block was allocated with _lwmem_alloc()
 * or _lwmem_alloc_zero(), only the task that allocated it can free part of it.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following
 * task error codes:
 * \li MQX_INVALID_POINTER (mem_ptr is NULL.)
 * \li MQX_LWMEM_POOL_INVALID (Pool that contains the block is not valid.)
 * \li MQX_NOT_RESOURCE_OWNER (If the block was allocated with _lwmem_alloc() or
 * _lwmem_alloc_zero(), only the task that allocated it can free part of it.)
 *
 * \see _lwmem_free
 * \see _usr_lwmem_alloc
 * \see _usr_lwmem_alloc_from
 * \see _usr_lwmem_create_pool
 */
_mqx_uint _usr_lwmem_free
(
    void   *mem_ptr
)
{
    MQX_API_CALL_PARAMS params =
    {   (uint32_t)mem_ptr, 0, 0, 0, 0};
    return _mqx_api_call(MQX_API_LWMEM_FREE, &params);
}

#endif /* MQX_ENABLE_USER_MODE */

/*!
 * \brief Gets the size of unallocated (free) memory.
 *
 * \return Size of free memory (success).
 * \return 0 (failure)
 *
 * \warning On failure, calls _task_set_error() to set the following task error code:
 * \li MQX_LWMEM_POOL_INVALID (Pool that contains the block is not valid.)
 */
_mem_size _lwmem_get_free()
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    _mem_size              result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE1(KLOG_lwmem_get_free);

    result = _lwmem_get_free_internal((_lwmem_pool_id) kernel_data->KERNEL_LWMEM_POOL);

    _KLOGX2(KLOG_lwmem_get_free, result);
    return (result);

} /* Endbody */

/*!
 * \brief Gets the size of unallocated (free) memory from a specified pool.
 *
 * \param[in] pool_id The pool to get free size from.
 *
 * \return Size of free memory (success).
 * \return 0 (failure)
 *
 * \warning On failure, calls _task_set_error() to set the following task error code:
 * \li MQX_LWMEM_POOL_INVALID (Pool that contains the block is not valid.)
 */
_mem_size _lwmem_get_free_from
(
    void   *pool_id
)
{ /* Body */
    _KLOGM(KERNEL_DATA_STRUCT_PTR kernel_data);
    _mem_size              result;

    _KLOGM(_GET_KERNEL_DATA(kernel_data));
    _KLOGE2(KLOG_lwmem_get_free_from, pool_id);

    result = _lwmem_get_free_internal((_lwmem_pool_id) pool_id);

    _KLOGX2(KLOG_lwmem_get_free_from, result);
    return (result);

} /* Endbody */

/*!
 * \private
 *
 * \brief Returns summarized size of free memory blocks in the pool.
 *
 * \param[in] pool_id Which pool to get free size from.
 *
 * \return Size of free memory (success).
 * \return 0 (failure)
 *
 * \warning On failure, calls _task_set_error() to set the following task error code:
 * \li MQX_LWMEM_POOL_INVALID (Pool that contains the block is not valid.)
 */
_mem_size _lwmem_get_free_internal
(
    _lwmem_pool_id pool_id
)
{ /* Body */
    LWMEM_BLOCK_STRUCT_PTR block_ptr;
    _mem_size              total_size = 0;
    LWMEM_POOL_STRUCT_PTR  mem_pool_ptr = (LWMEM_POOL_STRUCT_PTR) pool_id;

#if MQX_CHECK_VALIDITY
    if (mem_pool_ptr->VALID != LWMEM_POOL_VALID)
    {
        _task_set_error(MQX_LWMEM_POOL_INVALID);
        return (0);
    } /* Endif */
#endif

    _int_disable();
    block_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR;
    while (block_ptr != NULL)
    {
        /* Provide window for higher priority tasks */
        mem_pool_ptr->POOL_ALLOC_PTR = block_ptr;
        _int_enable();
        _int_disable();

        /* Reset to the start if we were preempted
           and the preempting task did finish its free/malloc operation,
           a safe solution, even with time-slicing. */
        if (block_ptr != mem_pool_ptr->POOL_ALLOC_PTR)
        {
            total_size = 0; /* some task with higher priority did reset our loop pointer */
            block_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR; /* Restart, the free-list has changed! */
            continue;

        }

        total_size += block_ptr->BLOCKSIZE;
        block_ptr = block_ptr->U.NEXTBLOCK;
    } /* Endwhile */

    _int_enable();
    return (total_size);
} /* Endbody */

/*!
 * \brief Allocates a system block of lightweight memory block from the specified
 * memory pool that is available system-wide.
 *
 * See Description of _lwmem_alloc_from() function.
 *
 * \param[in] pool_id Lightweight memory pool from which to allocate the
 * lightweight memory block (from _lwmem_create_pool()).
 * \param[in] size    Number of single-addressable units to allocate.
 *
 * \return Pointer to the lightweight memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_LWMEM_POOL_INVALID (Memory pool to allocate from is invalid.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 *
 * \see _lwmem_alloc
 * \see _lwmem_alloc_system
 * \see _lwmem_alloc_system_zero
 * \see _lwmem_alloc_zero
 * \see _lwmem_alloc_at
 * \see _lwmem_alloc_system_align
 * \see _lwmem_alloc_align
 * \see _lwmem_alloc_system_align_from
 * \see _lwmem_alloc_align_from
 * \see _lwmem_alloc_from
 * \see _lwmem_alloc_system_zero_from
 * \see _lwmem_alloc_zero_from
 * \see _lwmem_realloc
 * \see _lwmem_create_pool
 * \see _lwmem_free
 * \see _lwmem_transfer
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error
 */
void *_lwmem_alloc_system_from
(
    _lwmem_pool_id pool_id,
    _mem_size      size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_lwmem_alloc_system_from, size);

    result = _lwmem_alloc_internal(size, SYSTEM_TD_PTR(kernel_data), pool_id, FALSE);

    _KLOGX2(KLOG_lwmem_alloc_system_from, result);
    return (result);

} /* Endbody */

/*!
 * \brief Allocates a system block of lightweight memory block from the default
 * memory pool that is available system-wide.
 *
 * See Description of _lwmem_alloc() function.
 *
 * \param[in] size Number of single-addressable units to allocate.
 *
 * \return Pointer to the lightweight memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_LWMEM_POOL_INVALID (Memory pool to allocate from is invalid.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 *
 * \see _lwmem_alloc
 * \see _lwmem_alloc_system_zero
 * \see _lwmem_alloc_zero
 * \see _lwmem_alloc_at
 * \see _lwmem_alloc_system_align
 * \see _lwmem_alloc_align
 * \see _lwmem_alloc_system_align_from
 * \see _lwmem_alloc_align_from
 * \see _lwmem_alloc_from
 * \see _lwmem_alloc_system_from
 * \see _lwmem_alloc_system_zero_from
 * \see _lwmem_alloc_zero_from
 * \see _lwmem_realloc
 * \see _lwmem_create_pool
 * \see _lwmem_free
 * \see _lwmem_get_size
 * \see _lwmem_set_default_pool
 * \see _lwmem_transfer
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error
 */
void *_lwmem_alloc_system
(
    _mem_size size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_lwmem_alloc_system, size);

    result = _lwmem_alloc_internal(
        size, SYSTEM_TD_PTR(kernel_data), (_lwmem_pool_id) kernel_data->KERNEL_LWMEM_POOL, FALSE
    );

    _KLOGX2(KLOG_lwmem_alloc_system, result);
    return (result);

} /* Endbody */

/*!
 * \brief Allocates a system aligned block of lightweight memory block from the default
 * memory pool that is available system-wide.
 *
 * See Description of _lwmem_alloc() function.
 *
 * \param[in] size Number of single-addressable units to allocate.
 * \param[in] req_align      Align requested value.
 *
 * \return Pointer to the lightweight memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_LWMEM_POOL_INVALID (Memory pool to allocate from is invalid.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_INVALID_PARAMETER (Requested alignment is not power of 2.)
 *
 * \see _lwmem_alloc
 * \see _lwmem_alloc_system_zero
 * \see _lwmem_alloc_zero
 * \see _lwmem_alloc_at
 * \see _lwmem_alloc_align
 * \see _lwmem_alloc_system_align_from
 * \see _lwmem_alloc_align_from
 * \see _lwmem_alloc_from
 * \see _lwmem_alloc_system_from
 * \see _lwmem_alloc_system_zero_from
 * \see _lwmem_alloc_zero_from
 * \see _lwmem_realloc
 * \see _lwmem_create_pool
 * \see _lwmem_free
 * \see _lwmem_get_size
 * \see _lwmem_set_default_pool
 * \see _lwmem_transfer
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error
 */
void *_lwmem_alloc_system_align
(
    _mem_size size,
    _mem_size req_align
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE3(KLOG_lwmem_alloc_system_align, size, req_align);

    result = _lwmem_alloc_align_internal(
        size, req_align, SYSTEM_TD_PTR(kernel_data), (_lwmem_pool_id) kernel_data->KERNEL_LWMEM_POOL, FALSE
    );

    _KLOGX2(KLOG_lwmem_alloc_system_align, result);
    return (result);

} /* Endbody */


/*!
 * \brief Allocates a system aligned block of lightweight memory block from from the specified
 * memory pool. system-wide.
 *
 * See Description of _lwmem_alloc_from() function.
 *
 * \param[in] pool_id        Lightweight memory pool from which to allocate the
 * lightweight memory block (from _lwmem_create_pool()).
 * \param[in] requested_size Number of single-addressable units to allocate.
 * \param[in] req_align      Align requested value.
 *
 * \return Pointer to the lightweight memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_LWMEM_POOL_INVALID (Memory pool to allocate from is invalid.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_INVALID_PARAMETER (Requested alignment is not power of 2.)
 *
 * \see _lwmem_alloc
 * \see _lwmem_alloc_system_zero
 * \see _lwmem_alloc_zero
 * \see _lwmem_alloc_at
 * \see _lwmem_alloc_system_align
 * \see _lwmem_alloc_align
 * \see _lwmem_alloc_align_from
 * \see _lwmem_alloc_from
 * \see _lwmem_alloc_system_from
 * \see _lwmem_alloc_system_zero_from
 * \see _lwmem_alloc_zero_from
 * \see _lwmem_realloc
 * \see _lwmem_create_pool
 * \see _lwmem_free
 * \see _lwmem_get_size
 * \see _lwmem_set_default_pool
 * \see _lwmem_transfer
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error
 */
void *_lwmem_alloc_system_align_from
(
    _lwmem_pool_id pool_id,
    _mem_size      requested_size,
    _mem_size      req_align
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE4(KLOG_lwmem_alloc_system_align_from, pool_id, requested_size, req_align);

    result = _lwmem_alloc_align_internal(requested_size, req_align, SYSTEM_TD_PTR(kernel_data), pool_id, FALSE);

    _KLOGX2(KLOG_lwmem_alloc_system_align_from, result);
    return (result);

} /* Endbody */

/*!
 * \private
 *
 * \brief Initializes MQX to use the lwmem manager.
 *
 * \return MQX_OK
 */
_mqx_uint _lwmem_init_internal(void)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    LWMEM_POOL_STRUCT_PTR  lwmem_pool_ptr;
    unsigned char              *start;

    _GET_KERNEL_DATA(kernel_data);

    /*
     * Move the MQX memory pool pointer past the end of kernel data.
     */
    start = (void *) ((unsigned char *) kernel_data + sizeof(KERNEL_DATA_STRUCT));
    lwmem_pool_ptr = (LWMEM_POOL_STRUCT_PTR) start;
    kernel_data->KERNEL_LWMEM_POOL = (void *) lwmem_pool_ptr;

    start = (void *) ((unsigned char *) start + sizeof(LWMEM_POOL_STRUCT));

    _lwmem_create_pool(lwmem_pool_ptr, start, (unsigned char *) kernel_data->INIT.END_OF_KERNEL_MEMORY - (unsigned char *) start);

    return (MQX_OK);

} /* Endbody */

/*!
 * \private
 *
 * \brief Find the next block associated with the specified Task.
 *
 * \param[in] td_ptr       The TD whose blocks are being looked for.
 * \param[in] in_block_ptr The block last obtained.
 *
 * \return Pointer to the next block.
 * \return NULL (failure)
 */
void *_lwmem_get_next_block_internal
(
    TD_STRUCT_PTR td_ptr,
    void         *in_block_ptr
)
{
    KERNEL_DATA_STRUCT_PTR kernel_data;
    LWMEM_POOL_STRUCT_PTR  lwmem_pool_ptr;
    LWMEM_BLOCK_STRUCT_PTR block_ptr = in_block_ptr;
    LWMEM_BLOCK_STRUCT_PTR free_ptr;

    _GET_KERNEL_DATA(kernel_data);

    if (block_ptr == NULL)
    {
        /* first item, start on first item in first pool */
        lwmem_pool_ptr = (LWMEM_POOL_STRUCT*)kernel_data->LWMEM_POOLS.NEXT;
        block_ptr = lwmem_pool_ptr->POOL_ALLOC_START_PTR;
    }
    else
    {
        /* continued, get lwmem pool from provided in_block_ptr */
        block_ptr = GET_LWMEMBLOCK_PTR(in_block_ptr);
        lwmem_pool_ptr = block_ptr->POOL;

        block_ptr = (LWMEM_BLOCK_STRUCT_PTR)((unsigned char *) block_ptr + block_ptr->BLOCKSIZE);
    }

    _int_disable();

    do
    {
        free_ptr = lwmem_pool_ptr->POOL_FREE_LIST_PTR;

        while ((unsigned char *) block_ptr < (unsigned char *) lwmem_pool_ptr->POOL_ALLOC_END_PTR)
        {
            if (block_ptr->U.S.TASK_NUMBER == TASK_NUMBER_FROM_TASKID(td_ptr->TASK_ID))
            {
                /* check for block is not free block */
                while (free_ptr && free_ptr < block_ptr)
                {
                    free_ptr = free_ptr->U.NEXTBLOCK;
                }

                if (free_ptr != block_ptr)
                {
                    /* This block is owned by the target task and it's not free block*/
                    _int_enable();

                    return ((void *) ((unsigned char *) block_ptr + sizeof(LWMEM_BLOCK_STRUCT)));
                }
            }
            block_ptr = (LWMEM_BLOCK_STRUCT_PTR)((unsigned char *) block_ptr + block_ptr->BLOCKSIZE);
        }

        /* continue in next lwmem pool */
        lwmem_pool_ptr = (LWMEM_POOL_STRUCT_PTR)(lwmem_pool_ptr->LINK.NEXT);
        block_ptr = lwmem_pool_ptr->POOL_ALLOC_START_PTR;
    } while ((QUEUE_ELEMENT_STRUCT**)lwmem_pool_ptr != &kernel_data->LWMEM_POOLS.NEXT); /* repeat until processed lwmem pool is not first pool (pool list is circular list) */

    _int_enable();

    return (NULL);
}

/*!
 * \brief Sets the value of the default lightweight memory pool.
 *
 * Because MQX allocates lightweight memory blocks from the default lightweight
 * memory pool when an application calls _lwmem_alloc(), _lwmem_alloc_system(),
 * _lwmem_alloc_system_zero() or _lwmem_alloc_zero(), the application must first
 * call _lwmem_set_default_pool().
 *
 * \param[in] pool_id New pool ID.
 *
 * \return Previous pool ID.
 *
 * \see _lwmem_alloc
 * \see _lwmem_alloc_system
 * \see _lwmem_alloc_system_zero
 * \see _lwmem_alloc_zero
 * \see _lwmem_alloc_at
 * \see _lwmem_alloc_system_align
 * \see _lwmem_alloc_align
 * \see _lwmem_alloc_system_align_from
 * \see _lwmem_alloc_align_from
 * \see _lwmem_alloc_from
 * \see _lwmem_alloc_system_from
 * \see _lwmem_alloc_system_zero_from
 * \see _lwmem_alloc_zero_from
 * \see _lwmem_realloc
 * \see _lwsem_destroy
 * \see _lwsem_post
 * \see _lwsem_test
 * \see _lwsem_wait
 * \see _lwsem_wait_for
 * \see _lwsem_wait_ticks
 * \see _lwsem_wait_until
 */
_lwmem_pool_id _lwmem_set_default_pool
(
    _lwmem_pool_id pool_id
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    _lwmem_pool_id         old_pool_id;

    _GET_KERNEL_DATA(kernel_data);

    old_pool_id = kernel_data->KERNEL_LWMEM_POOL;
    kernel_data->KERNEL_LWMEM_POOL = pool_id;
    return (old_pool_id);

} /* Endbody */

/*!
 * \brief Gets the size of the lightweight memory block.
 *
 * The size is the actual size of the block and might be larger than the size
 * that a task requested.
 *
 * \param[in] mem_ptr Pointer to the lightweight memory block.
 *
 * \return Number of single-addressable units in the block (success).
 * \return 0 (failure)
 */
_mem_size _lwmem_get_size
(
    void   *mem_ptr
)
{ /* Body */
    LWMEM_BLOCK_STRUCT_PTR block_ptr;

#if MQX_CHECK_ERRORS
    if (mem_ptr == NULL)
    {
        _task_set_error(MQX_INVALID_POINTER);
        return (0);
    } /* Endif */
#endif

    /* Compute the start of the block  */
    block_ptr = GET_LWMEMBLOCK_PTR(mem_ptr);
    /* The size includes the block overhead, which the user is not
     * interested in. If the size is less than the overhead,
     * then there is a bad block or bad block pointer.
     */
#if MQX_CHECK_ERRORS
    if (block_ptr->BLOCKSIZE <= (_mem_size) sizeof(LWMEM_BLOCK_STRUCT))
    {
        _task_set_error(MQX_INVALID_POINTER);
        return (0);
    } /* Endif */
#endif

    return (block_ptr->BLOCKSIZE - (_mem_size) sizeof(LWMEM_BLOCK_STRUCT));

} /* Endbody */

/* Move to standalone file */

#if MQX_ALLOW_TYPED_MEMORY
/*!
 * \brief Gets type of the specified block.
 *
 * \param[in] mem_ptr Pointer to the lightweight memory block.
 *
 * \return Type of memory block.
 */
_mem_type _lwmem_get_type
(
    void   *mem_ptr
)
{
    LWMEM_BLOCK_STRUCT_PTR block_ptr;

    block_ptr = GET_LWMEMBLOCK_PTR(mem_ptr);
    return block_ptr->U.S.MEM_TYPE;
}

/*!
 * \brief Sets the type of the specified block.
 *
 * \param[in] mem_ptr  Pointer to the lightweight memory block.
 * \param[in] mem_type Type of lightweight memory block to set.
 *
 * \return TRUE (success) or FALSE (failure: mem_ptr ic NULL).
 */
bool _lwmem_set_type
(
    void     *mem_ptr,
    _mem_type mem_type
)
{
    LWMEM_BLOCK_STRUCT_PTR block_ptr;

    if (mem_ptr != NULL)
    {
        block_ptr = GET_LWMEMBLOCK_PTR(mem_ptr);
        block_ptr->U.S.MEM_TYPE = mem_type;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
#endif

/*!
 * \brief Sets the type of the specified block.
 *
 * \return Highest size of used memory.
 */
void *_lwmem_get_highwater(void)
{
    KERNEL_DATA_STRUCT_PTR kernel_data;

    _GET_KERNEL_DATA(kernel_data);

    return (((LWMEM_POOL_STRUCT_PTR)(kernel_data->KERNEL_LWMEM_POOL))->HIGHWATER);

}

/*!
 * \brief Tests all lightweight memory for errors.
 *
 * The function checks the checksums in the headers of all lightweight memory blocks.
 * \n The function can be called by only one task at a time because it keeps
 * state-in-progress variables that MQX controls. This mechanism lets other tasks
 * allocate and free lightweight memory while _lwmem_test() runs.
 *
 * \param[out] pool_error_ptr  Pointer to the pool in error (points to NULL if no
 * error was found).
 * \param[out] block_error_ptr Pointer to the block in error (points to NULL if
 * no error was found).
 *
 * \return MQX_OK
 * \return MQX_LWMEM_POOL_INVALID (Lightweight memory pool is corrupted.)
 * \return MQX_CORRUPT_STORAGE_POOL (A memory pool pointer is not correct.)
 * \return MQX_CORRUPT_STORAGE_POOL_FREE_LIST (Memory pool freelist is corrupted.)
 * \return MQX_CORRUPT_QUEUE (An error was found.)
 * \return MQX_CANNOT_CALL_FUNCTION_FROM_ISR (Function is not called inside an interrupt.)
 *
 * \warning Can be called by only one task at a time (see description).
 * \warning Disables and enables interrupts.
 *
 * \see _lwmem_alloc
 * \see _lwmem_alloc_system
 * \see _lwmem_alloc_system_zero
 * \see _lwmem_alloc_zero
 * \see _lwmem_alloc_at
 * \see _lwmem_alloc_system_align
 * \see _lwmem_alloc_align
 * \see _lwmem_alloc_system_align_from
 * \see _lwmem_alloc_align_from
 * \see _lwmem_alloc_from
 * \see _lwmem_alloc_system_from
 * \see _lwmem_alloc_system_zero_from
 * \see _lwmem_alloc_zero_from
 * \see _lwmem_realloc
 */
_mqx_uint _lwmem_test
(
    _lwmem_pool_id  *pool_error_ptr,
    void           **block_error_ptr
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR  kernel_data;
    LWMEM_POOL_STRUCT_PTR   mem_pool_ptr;
    LWMEM_BLOCK_STRUCT_PTR  queue_ptr = NULL;
    (void)                  queue_ptr; /* disable 'unused variable' warning */
    LWMEM_BLOCK_STRUCT_PTR  block_ptr;
    _mqx_uint               i;
    _mqx_uint               result;

    _GET_KERNEL_DATA(kernel_data);

#if MQX_CHECK_ERRORS
    if (kernel_data->IN_ISR)
    {
        _KLOGX2(KLOG_lwmem_test, MQX_CANNOT_CALL_FUNCTION_FROM_ISR);
        return (MQX_CANNOT_CALL_FUNCTION_FROM_ISR);
    } /* Endif */
#endif /* MQX_CHECK_ERRORS */

    /*
     * It is not considered an error if the lwmem component has not been
     * created yet
     */
    if (kernel_data->LWMEM_POOLS.NEXT == NULL)
    {
        return (MQX_OK);
    } /* Endif */

    result = _queue_test(&kernel_data->LWMEM_POOLS, pool_error_ptr);
    if (result != MQX_OK)
    {
        _KLOGX3(KLOG_lwmem_test, result, *pool_error_ptr);
        return (result);
    } /* Endif */
    _int_disable();
    i = _QUEUE_GET_SIZE(&kernel_data->LWMEM_POOLS);
    mem_pool_ptr = (LWMEM_POOL_STRUCT_PTR) (void *) kernel_data->LWMEM_POOLS.NEXT;
    while (i--)
    {
        if (mem_pool_ptr->VALID != LWMEM_POOL_VALID)
        {
            _int_enable();
            *pool_error_ptr = (void *) mem_pool_ptr;
            *block_error_ptr = NULL;
            _KLOGX3(KLOG_lwmem_test, MQX_LWMEM_POOL_INVALID, *pool_error_ptr);
            return (MQX_LWMEM_POOL_INVALID);
        } /* Endif */

        /* Make sure pool is ok */
        block_ptr = mem_pool_ptr->POOL_ALLOC_START_PTR;
        while ((unsigned char *) block_ptr < (unsigned char *) mem_pool_ptr->POOL_ALLOC_END_PTR)
        {
            mem_pool_ptr->POOL_TEST2_PTR = block_ptr;
            _int_enable();
            _int_disable();
            block_ptr = mem_pool_ptr->POOL_TEST2_PTR;
            if (block_ptr->POOL != mem_pool_ptr)
            {
                _int_enable();
                *pool_error_ptr = (void *) mem_pool_ptr;
                *block_error_ptr = (void *) block_ptr;
                _KLOGX3(KLOG_lwmem_test, MQX_CORRUPT_STORAGE_POOL, *pool_error_ptr);
                return (MQX_CORRUPT_STORAGE_POOL);
            } /* Endif */
            block_ptr = (LWMEM_BLOCK_STRUCT_PTR)((unsigned char *) block_ptr + block_ptr->BLOCKSIZE);
        } /* Endwhile */

        /* Make sure Freelist is ok */
        block_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR;
        while (block_ptr)
        {
            /* Provide window for higher priority tasks */
            mem_pool_ptr->POOL_TEST_PTR = block_ptr;
            _int_enable();
            _int_disable();
            block_ptr = mem_pool_ptr->POOL_TEST_PTR;
            if (((void *) block_ptr < mem_pool_ptr->POOL_ALLOC_START_PTR) || ((void *) block_ptr
                            > mem_pool_ptr->POOL_ALLOC_END_PTR) || (block_ptr->POOL != mem_pool_ptr)
                            || (block_ptr->U.NEXTBLOCK && (block_ptr->U.NEXTBLOCK <= (void *) ((unsigned char *) block_ptr
                                            + block_ptr->BLOCKSIZE))))
            {
                /* This block is in error */
                _int_enable();
                *pool_error_ptr = (void *) mem_pool_ptr;
                *block_error_ptr = block_ptr;
                _KLOGX3(KLOG_lwmem_test, MQX_CORRUPT_STORAGE_POOL_FREE_LIST, *pool_error_ptr);
                return (MQX_CORRUPT_STORAGE_POOL_FREE_LIST);
            } /* Endif */
            block_ptr = block_ptr->U.NEXTBLOCK;
        } /* Endwhile */
        mem_pool_ptr = (void *) mem_pool_ptr->LINK.NEXT;
    } /* Endwhile */
    _int_enable();

    *pool_error_ptr = NULL;
    *block_error_ptr = NULL;
    return (MQX_OK);
} /* Endbody */

/*!
 * \brief Transfers the ownership of the lightweight memory block from one task
 * to another.
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
 * \see _lwmem_alloc
 * \see _lwmem_alloc_system
 * \see _lwmem_alloc_system_zero
 * \see _lwmem_alloc_zero
 * \see _lwmem_alloc_at
 * \see _lwmem_alloc_system_align
 * \see _lwmem_alloc_align
 * \see _lwmem_alloc_system_align_from
 * \see _lwmem_alloc_align_from
 * \see _lwmem_alloc_from
 * \see _lwmem_alloc_system_from
 * \see _lwmem_alloc_system_zero_from
 * \see _lwmem_alloc_zero_from
 * \see _lwmem_realloc
 * \see _task_set_error
 */
_mqx_uint _lwmem_transfer
(
    void    *memory_ptr,
    _task_id source_id,
    _task_id target_id
)
{ /* Body */
    _KLOGM(KERNEL_DATA_STRUCT_PTR kernel_data);
    LWMEM_BLOCK_STRUCT_PTR block_ptr;
    TD_STRUCT_PTR          source_td;
    TD_STRUCT_PTR          target_td;

    _KLOGM(_GET_KERNEL_DATA(kernel_data));

    _KLOGE4(KLOG_lwmem_transfer, memory_ptr, source_id, target_id);

#if MQX_CHECK_ERRORS
    if (memory_ptr == NULL)
    {
        _task_set_error(MQX_INVALID_POINTER);
        _KLOGX2(KLOG_lwmem_transfer, MQX_INVALID_POINTER);
        return (MQX_INVALID_POINTER);
    } /* Endif */
#endif

    /* Verify the block */
    block_ptr = GET_LWMEMBLOCK_PTR(memory_ptr);

    source_td = (TD_STRUCT_PTR) _task_get_td(source_id);
    target_td = (TD_STRUCT_PTR) _task_get_td(target_id);
#if MQX_CHECK_ERRORS
    if ((source_td == NULL) || (target_td == NULL))
    {
        _task_set_error(MQX_INVALID_TASK_ID);
        _KLOGX2(KLOG_lwmem_transfer, MQX_INVALID_TASK_ID);
        return (MQX_INVALID_TASK_ID);
    } /* Endif */
#endif
#if MQX_CHECK_ERRORS
    if (block_ptr->U.S.TASK_NUMBER != TASK_NUMBER_FROM_TASKID(source_td->TASK_ID))
    {
        _task_set_error(MQX_NOT_RESOURCE_OWNER);
        return (MQX_NOT_RESOURCE_OWNER);
    } /* Endif */
#endif

    block_ptr->U.S.TASK_NUMBER = TASK_NUMBER_FROM_TASKID(target_td->TASK_ID);

    _KLOGX2(KLOG_lwmem_transfer, MQX_OK);
    return (MQX_OK);

} /* Endbody */

/*!
 * \private
 *
 * \brief This function transfers the ownership of a block of memory from an owner
 * task to another task.
 *
 * \param[in] memory_ptr The address of the USER_AREA in the memory block to transfer.
 * \param[in] target_td  The target task descriptor.
 */
void _lwmem_transfer_internal
(
    void         *memory_ptr,
    TD_STRUCT_PTR target_td
)
{ /* Body */
    LWMEM_BLOCK_STRUCT_PTR block_ptr;

    /* Verify the block */
    block_ptr = GET_LWMEMBLOCK_PTR(memory_ptr);
    block_ptr->U.S.TASK_NUMBER = TASK_NUMBER_FROM_TASKID(target_td->TASK_ID);

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
_mqx_uint _lwmem_transfer_td_internal
(
    void         *memory_ptr,
    TD_STRUCT_PTR source_td,
    TD_STRUCT_PTR target_td
)
{ /* Body */
    LWMEM_BLOCK_STRUCT_PTR block_ptr;

    block_ptr = GET_LWMEMBLOCK_PTR(memory_ptr);
    block_ptr->U.S.TASK_NUMBER = TASK_NUMBER_FROM_TASKID(target_td->TASK_ID);

    return (MQX_OK);

} /* Endbody */

/*!
 * \brief Allocates a system(zero-filled) block of lightweight memory block from
 * the specified memory pool.
 *
 * See Description of _lwmem_alloc_from() function.
 *
 * \param[in] pool_id        Lightweight memory pool from which to allocate the
 * lightweight memory block (from _lwmem_create_pool()).
 * \param[in] size           Number of single-addressable units to allocate.
 *
 * \return Pointer to the lightweight memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_LWMEM_POOL_INVALID (Memory pool to allocate from is invalid.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 *
 * \see _lwmem_alloc
 * \see _lwmem_alloc_system
 * \see _lwmem_alloc_system_zero
 * \see _lwmem_alloc_zero
 * \see _lwmem_alloc_at
 * \see _lwmem_alloc_system_align
 * \see _lwmem_alloc_align
 * \see _lwmem_alloc_system_align_from
 * \see _lwmem_alloc_align_from
 * \see _lwmem_alloc_from
 * \see _lwmem_alloc_system_from
 * \see _lwmem_alloc_zero_from
 * \see _lwmem_realloc
 * \see _lwmem_create_pool
 * \see _lwmem_free
 * \see _lwmem_transfer
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error
 */
void *_lwmem_alloc_system_zero_from
(
    _lwmem_pool_id pool_id,
    _mem_size      size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_lwmem_alloc_system_zero_from, size);

    result = _lwmem_alloc_internal(size, SYSTEM_TD_PTR(kernel_data), pool_id, TRUE);

    _KLOGX2(KLOG_lwmem_alloc_system_zero_from, result);
    return (result);

} /* Endbody */

/*!
 * \brief Allocates a system (zero-filled) block of lightweight memory block from
 * the default memory pool.
 *
 * See Description of _lwmem_alloc() function.
 *
 * \param[in] size Number of single-addressable units to allocate.
 *
 * \return Pointer to the lightweight memory block (success).
 * \return NULL (Failure: see Task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following task
 * error codes:
 * \li MQX_LWMEM_POOL_INVALID (Memory pool to allocate from is invalid.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 *
 * \see _lwmem_alloc
 * \see _lwmem_alloc_system
 * \see _lwmem_alloc_zero
 * \see _lwmem_alloc_at
 * \see _lwmem_alloc_system_align
 * \see _lwmem_alloc_align
 * \see _lwmem_alloc_system_align_from
 * \see _lwmem_alloc_align_from
 * \see _lwmem_alloc_from
 * \see _lwmem_alloc_system_from
 * \see _lwmem_alloc_system_zero_from
 * \see _lwmem_alloc_zero_from
 * \see _lwmem_realloc
 * \see _lwmem_create_pool
 * \see _lwmem_free
 * \see _lwmem_get_size
 * \see _lwmem_set_default_pool
 * \see _lwmem_transfer
 * \see _msg_alloc
 * \see _msg_alloc_system
 * \see _task_set_error
 */
void *_lwmem_alloc_system_zero
(
    _mem_size size
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR kernel_data;
    void                  *result;

    _GET_KERNEL_DATA(kernel_data);
    _KLOGE2(KLOG_lwmem_alloc_system_zero, size);

    result = _lwmem_alloc_internal(
        size, SYSTEM_TD_PTR(kernel_data), (_lwmem_pool_id) kernel_data->KERNEL_LWMEM_POOL, TRUE
    );

    _KLOGX2(KLOG_lwmem_alloc_system_zero, result);
    return (result);

} /* Endbody */

/*!
 * \private
 *
 * \brief If it is possible, free part of given memory block.
 *
 * This function free part of given memory block, if free memory block is
 * right behind given memory block, or freed part is bigger as minimal size.
 * Freed part can merge with free memory block, or can create new
 * free memory block.
 *
 * \param[in] mem_ptr Pointer to given memory block.
 * \param[in] size New size for given memory block.
 *
 * \return MQX_OK
 * \return MQX_INVALID_POINTER (mem_ptr is NULL.)
 * \return MQX_INVALID_SIZE (Size of the original block is less than requested
 *  size or remaining space is less than the LWMEM_MIN_MEMORY_STORAGE_SIZE)
 * \return MQX_LWMEM_POOL_INVALID (Pool that contains the block is not valid.)
 * \return MQX_NOT_RESOURCE_OWNER (If the block was allocated with _lwmem_alloc()
 * or _lwmem_alloc_zero(), only the task that allocated it can free part of it.)
 *
 */
_mqx_uint _lwmem_free_part_internal
(
   void*    mem_ptr,
  _mem_size size
)
{
    KERNEL_DATA_STRUCT_PTR  kernel_data = NULL;
    LWMEM_BLOCK_STRUCT_PTR  block_ptr;
    volatile LWMEM_BLOCK_STRUCT_PTR  free_ptr;
    LWMEM_BLOCK_STRUCT_PTR  next_ptr;
    LWMEM_BLOCK_STRUCT_PTR  next_block_ptr;
    LWMEM_BLOCK_STRUCT_PTR  prev_block_ptr = NULL; /* If free list is damaged, the NULL value is used for easier debugging. */
    LWMEM_POOL_STRUCT_PTR   mem_pool_ptr;
    bool                    insert;
    _mqx_uint               request_block_size;
    _mqx_uint               result_code;

    _KLOGE3(KLOG_lwmem_free_part_internal, mem_ptr, size);

#if MQX_CHECK_ERRORS
    /* Verify the passed in parameter */
    if (mem_ptr == NULL)
    {
        _task_set_error(MQX_INVALID_POINTER);
        _KLOGX2(KLOG_lwmem_free_part_internal, MQX_INVALID_POINTER);
        return (MQX_INVALID_POINTER);
    } /* Endif */
#endif

    block_ptr = GET_LWMEMBLOCK_PTR(mem_ptr);
    request_block_size = size + (_mem_size) sizeof(LWMEM_BLOCK_STRUCT);

#if MQX_CHECK_ERRORS
    /* Verify the passed in parameter */
    if (request_block_size > block_ptr->BLOCKSIZE)
    {
        _KLOGX2(KLOG_lwmem_free_part_internal, MQX_INVALID_SIZE);
        return (MQX_INVALID_SIZE);
    } /* Endif */
#endif

    if(LWMEM_MIN_MEMORY_STORAGE_SIZE>request_block_size){
      request_block_size=LWMEM_MIN_MEMORY_STORAGE_SIZE;
    }/* Endif */

     _MEMORY_ALIGN_VAL_LARGER(request_block_size);

     /* No need freed part of memory */
    if(request_block_size >= block_ptr->BLOCKSIZE){
      _KLOGX2(KLOG_lwmem_free_part_internal, MQX_OK);
      return MQX_OK;
    }/* Endif */

    _GET_KERNEL_DATA(kernel_data);

    _int_disable();
    mem_pool_ptr = (LWMEM_POOL_STRUCT_PTR) block_ptr->POOL;

#if MQX_CHECK_VALIDITY
    if (mem_pool_ptr->VALID != LWMEM_POOL_VALID)
    {
        _int_enable();
        _KLOGX2(KLOG_lwmem_free_part_internal, MQX_LWMEM_POOL_INVALID);
        return (MQX_LWMEM_POOL_INVALID);
    } /* Endif */
#endif

#if MQX_CHECK_ERRORS
    /* Verify the passed in parameter */
    if (!((block_ptr->U.S.TASK_NUMBER == TASK_NUMBER_FROM_TASKID(kernel_data->ACTIVE_PTR->TASK_ID))
                    || (block_ptr->U.S.TASK_NUMBER == SYSTEM_TASK_NUMBER)))
    {
        _int_enable();
        _KLOGX2(KLOG_lwmem_free_part_internal, MQX_NOT_RESOURCE_OWNER);
        return (MQX_NOT_RESOURCE_OWNER);
    } /* Endif */
#endif

    free_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR;
    /* Searching for free memory block behind allocated memory block */
    while (TRUE){

      if (free_ptr == NULL || ((void *) block_ptr < (void *)free_ptr)){

         size = block_ptr->BLOCKSIZE-request_block_size;
         if(((unsigned char *) block_ptr + block_ptr->BLOCKSIZE) == (unsigned char *) free_ptr){

            /* Freed part join with the next block on the list */
            /* back up */
            next_ptr = free_ptr->U.NEXTBLOCK;
            size += free_ptr->BLOCKSIZE;


            /* The merged block should have cleared header to assure failure at multiple freeing with the same pointer */
            _mem_zero(free_ptr, sizeof(LWMEM_BLOCK_STRUCT));
         }
         else if(size >= LWMEM_MIN_MEMORY_STORAGE_SIZE){
          /* back up */
            next_ptr = free_ptr;
         }/* Endif */
         else{
          insert = FALSE;
          break;
         }
         insert = TRUE;
         break;
      }/* Endif */

      /* Provide window for higher priority tasks */
      prev_block_ptr=free_ptr;
      free_ptr = free_ptr->U.NEXTBLOCK;
      mem_pool_ptr->POOL_FREE_PTR = free_ptr;
      _int_enable();
      _int_disable();
      if(free_ptr != mem_pool_ptr->POOL_FREE_PTR){
        free_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR;
      }
    }/* Endwhile */

    if(insert){
      block_ptr->BLOCKSIZE = request_block_size;
      next_block_ptr = (LWMEM_BLOCK_STRUCT_PTR) (void *) ((unsigned char *) block_ptr + request_block_size);
      next_block_ptr->BLOCKSIZE = size;
      next_block_ptr->POOL = (void *) mem_pool_ptr;
      next_block_ptr->U.NEXTBLOCK = next_ptr;
      if(free_ptr == mem_pool_ptr->POOL_FREE_LIST_PTR){
          mem_pool_ptr->POOL_FREE_LIST_PTR = next_block_ptr;
      }
      else{
#if MQX_CHECK_ERRORS
          assert(prev_block_ptr != NULL);
#endif
          prev_block_ptr->U.NEXTBLOCK = next_block_ptr;
      }/* Endif */
      result_code = MQX_OK;
    }
    else {
      result_code = MQX_INVALID_SIZE;
    }/* Endif */

    mem_pool_ptr->POOL_ALLOC_PTR = mem_pool_ptr->POOL_FREE_LIST_PTR;
    mem_pool_ptr->POOL_FREE_PTR = mem_pool_ptr->POOL_FREE_LIST_PTR;
    mem_pool_ptr->POOL_TEST_PTR = mem_pool_ptr->POOL_FREE_LIST_PTR;
    mem_pool_ptr->POOL_TEST2_PTR = mem_pool_ptr->POOL_ALLOC_START_PTR;
#if MQX_TASK_DESTRUCTION
    mem_pool_ptr->POOL_DESTROY_PTR = mem_pool_ptr->POOL_ALLOC_START_PTR;
#endif
    _int_enable();

    _KLOGX2(KLOG_lwmem_free_part_internal, result_code);

    return result_code;
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
 * \return MQX_INVALID_POINTER (mem_ptr is NULL.)
 * \return MQX_INVALID_SIZE (Size of the original block is bigger than requested
 *  size, or no free block with enough size after original block.)
 * \return MQX_LWMEM_POOL_INVALID (Pool that contains the block is not valid.)
 * \return MQX_NOT_RESOURCE_OWNER (If the block was allocated with _lwmem_alloc()
 * or _lwmem_alloc_zero(), only the task that allocated it can free part of it.)
 *
 */

_mqx_uint _lwmem_alloc_extend_internal
(
  void*     mem_ptr,
  _mem_size size
)
{/* Body */
  KERNEL_DATA_STRUCT_PTR kernel_data;
  LWMEM_BLOCK_STRUCT_PTR block_ptr;
  LWMEM_BLOCK_STRUCT_PTR next_block_ptr;
  LWMEM_BLOCK_STRUCT_PTR prev_block_ptr = NULL;
  _mem_size              next_block_size;
  LWMEM_POOL_STRUCT_PTR  mem_pool_ptr;
  uint32_t               highwater;
  _mem_size              request_block_size;
  volatile LWMEM_BLOCK_STRUCT_PTR free_ptr;
  LWMEM_BLOCK_STRUCT_PTR next_ptr;
  _mqx_uint              result_code;

  _GET_KERNEL_DATA(kernel_data);

  _KLOGE3(KLOG_lwmem_alloc_extend_internal, mem_ptr, size);

#if MQX_CHECK_ERRORS
    /* Verify the passed in parameter */
    if (mem_ptr == NULL)
    {
      _KLOGX2(KLOG_lwmem_alloc_extend_internal, MQX_INVALID_POINTER);
      return (MQX_INVALID_POINTER);
    } /* Endif */
#endif

    block_ptr = GET_LWMEMBLOCK_PTR(mem_ptr);
    request_block_size = size + (_mem_size) sizeof(LWMEM_BLOCK_STRUCT);

#if MQX_CHECK_ERRORS
    /* Verify the passed in parameter */
    if (request_block_size < block_ptr->BLOCKSIZE)
    {
        _KLOGX2(KLOG_lwmem_alloc_extend_internal, MQX_INVALID_SIZE);
        return (MQX_INVALID_SIZE);
    } /* Endif */
#endif

    _MEMORY_ALIGN_VAL_LARGER(request_block_size);

    /* No need extend */
    if(request_block_size == block_ptr->BLOCKSIZE){
      _KLOGX2(KLOG_lwmem_alloc_extend_internal, MQX_OK);
      return MQX_OK;
    }/* Endif */

    _int_disable();
    mem_pool_ptr = (LWMEM_POOL_STRUCT_PTR) block_ptr->POOL;

#if MQX_CHECK_VALIDITY
    /* Verify the passed in parameter */
    if (mem_pool_ptr->VALID != LWMEM_POOL_VALID)
    {
        _int_enable();
        _KLOGX2(KLOG_lwmem_alloc_extend_internal, MQX_LWMEM_POOL_INVALID);
        return (MQX_LWMEM_POOL_INVALID);
    } /* Endif */
#endif

#if MQX_CHECK_ERRORS
    /* Verify the passed in parameter */
    if (!((block_ptr->U.S.TASK_NUMBER == TASK_NUMBER_FROM_TASKID(kernel_data->ACTIVE_PTR->TASK_ID))
                    || (block_ptr->U.S.TASK_NUMBER == SYSTEM_TASK_NUMBER)))
    {
        _int_enable();
        _KLOGX2(KLOG_lwmem_alloc_extend_internal, MQX_NOT_RESOURCE_OWNER);
        return(MQX_NOT_RESOURCE_OWNER);
    } /* Endif */
#endif

    free_ptr = mem_pool_ptr->POOL_FREE_LIST_PTR;

    while (TRUE){

      /* Searching for free memory block behind allocated memory block */
      if (free_ptr == NULL || ((void *) block_ptr < (void *)free_ptr)){

        /* Check if free block is behind allocated block */
        if (((unsigned char *) block_ptr + block_ptr->BLOCKSIZE) == (unsigned char *) free_ptr){

          /* If free memory block is not big enough set block_ptr as NULL */
          size = request_block_size-block_ptr->BLOCKSIZE;
          if(size > free_ptr->BLOCKSIZE){
            block_ptr = NULL;
          }/* Endif */
        }
        else{
          block_ptr = NULL;
        }/* Endif */

        break;
      }/* Endif */

      prev_block_ptr = free_ptr;
      /* Provide window for higher priority tasks */
      free_ptr = free_ptr->U.NEXTBLOCK;
      mem_pool_ptr->POOL_ALLOC_PTR = free_ptr;
      _int_enable();
      _int_disable();
      if(free_ptr != mem_pool_ptr->POOL_ALLOC_PTR){
        free_ptr=mem_pool_ptr->POOL_FREE_LIST_PTR;
      }/* Endif */

    }/* Endwhile */

    /* Extend memory block*/
    if(block_ptr != NULL) {

      /* If free memory can split */
      next_block_size = free_ptr->BLOCKSIZE-size;
      if (next_block_size >= LWMEM_MIN_MEMORY_STORAGE_SIZE){
         /* back up */
         next_ptr = free_ptr->U.NEXTBLOCK;
         /*
           * The current block is big enough to split.
           * into 2 blocks.... the part to be allocated is one block,
           * and the rest remains as a free block on the free list.
           */
         next_block_ptr = (LWMEM_BLOCK_STRUCT_PTR) (void *) ((unsigned char *) free_ptr + size);
         /* Initialize the new physical block values */
         next_block_ptr->BLOCKSIZE = next_block_size;
         /* Link new block into the free list */
         next_block_ptr->POOL = (void *) mem_pool_ptr;
         next_block_ptr->U.NEXTBLOCK = next_ptr;

       }
       else{
          /* Take the entire block */
          size = free_ptr->BLOCKSIZE;
          next_block_ptr = free_ptr->U.NEXTBLOCK;
       }/* Endif */

      if (free_ptr == mem_pool_ptr->POOL_FREE_LIST_PTR)
      {
          /* At the head of the free list */
          mem_pool_ptr->POOL_FREE_LIST_PTR = next_block_ptr;
      }
      else
      {
          prev_block_ptr->U.NEXTBLOCK = next_block_ptr;
      } /* Endif */
      block_ptr->BLOCKSIZE += size;

      /* Indicate the highest memory address used */
      highwater = ((uint32_t) block_ptr) + block_ptr->BLOCKSIZE - 1;
      if (highwater > (uint32_t) mem_pool_ptr->HIGHWATER)
      {
          mem_pool_ptr->HIGHWATER = (void*) highwater;
      }/* Endif */
      result_code = MQX_OK;
    }
    else {
      result_code = MQX_INVALID_SIZE;
    }/* Endif */

    mem_pool_ptr->POOL_ALLOC_PTR = mem_pool_ptr->POOL_FREE_LIST_PTR;
    mem_pool_ptr->POOL_FREE_PTR = mem_pool_ptr->POOL_FREE_LIST_PTR;
    mem_pool_ptr->POOL_TEST_PTR = mem_pool_ptr->POOL_FREE_LIST_PTR;
    mem_pool_ptr->POOL_TEST2_PTR = mem_pool_ptr->POOL_ALLOC_START_PTR;
    #if MQX_TASK_DESTRUCTION
    mem_pool_ptr->POOL_DESTROY_PTR = mem_pool_ptr->POOL_ALLOC_START_PTR;
    #endif

    _int_enable();

    _KLOGX2(KLOG_lwmem_alloc_extend_internal, result_code);

  return result_code;

}/* Endbody */

#if MQX_ENABLE_USER_MODE
/*!
 * /brief Resize the lightweight memomory block that was previously allocated.
 *
 * This function is an equivalent to the _lwmem_realloc() API call but it can be
 * executed from within the User task or other code running in the CPU User mode.
 * Parameters passed to this function by pointer are required to meet the memory
 * protection requirements as described in the parameter list below.
 *
 * \param[in] mem_ptr Pointer to given memory block.
 * \param[in] size Request size for the new memory block.
 *
 * \return Pointer to the new memory block (Success.)
 * \return NULL (Failure: see task error codes.)
 *
 * \warning On failure, calls _task_set_error() to set one of the following
 *  task error codes:
 * \li MQX_LWMEM_POOL_INVALID (Memory pool to allocate from or pool that
 *  contains the block is invalid.)
 * \li MQX_NOT_RESOURCE_OWNER (If the block was allocated with _lwmem_alloc()
 *  or _lwmem_alloc_zero(), only the task that allocated it can resize it.)
 * \li MQX_INVALID_POINTER (Bad block or bad block pointer.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a free block of the requested size.)
 * \li MQX_INVALID_SIZE (Size of given memory block was changed while
 *  realloc was executed)
 */
void *_usr_lwmem_realloc
(
   void *mem_ptr,
   _mem_size size
)
{
    MQX_API_CALL_PARAMS params =
    { (uint32_t) mem_ptr ,(uint32_t)size, 0, 0, 0};

    return (void *)_mqx_api_call(MQX_API_LWMEM_REALLOC, &params);
}

#endif /* MQX_ENABLE_USER_MODE */

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
 * \warning On failure, calls _task_set_error() to set one of the following
 *  task error codes:
 * \li MQX_LWMEM_POOL_INVALID (Memory pool to allocate from or pool that
 *  contains the block is invalid.)
 * \li MQX_NOT_RESOURCE_OWNER (If the block was allocated with _lwmem_alloc()
 *  or _lwmem_alloc_zero(), only the task that allocated it can resize it.)
 * \li MQX_INVALID_POINTER (Bad block or bad block pointer.)
 * \li MQX_OUT_OF_MEMORY (MQX cannot find a block of the requested size.)
 * \li MQX_INVALID_SIZE (Size of given memory block was changed while
 *  realloc was executed)
 *
 * \see _lwmem_alloc
 * \see _lwmem_alloc_system
 * \see _lwmem_alloc_system_zero
 * \see _lwmem_alloc_zero
 * \see _lwmem_alloc_at
 * \see _lwmem_alloc_system_align
 * \see _lwmem_alloc_align
 * \see _lwmem_alloc_system_align_from
 * \see _lwmem_alloc_align_from
 * \see _lwmem_alloc_from
 * \see _lwmem_alloc_system_from
 * \see _lwmem_alloc_system_zero_from
 * \see _lwmem_alloc_zero_from
 * \see _lwmem_free
 * \see _task_set_error
 */
void *_lwmem_realloc
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
  LWMEM_BLOCK_STRUCT_PTR block_ptr;
  KERNEL_DATA_STRUCT_PTR kernel_data;
  LWMEM_POOL_STRUCT_PTR  mem_pool_ptr;


#if MQX_ENABLE_USER_MODE && MQX_ENABLE_USER_STDAPI
    if (MQX_RUN_IN_USER_MODE)
    {
        return _usr_lwmem_realloc(mem_ptr, size);
    }/* Endif */
#endif

  _GET_KERNEL_DATA(kernel_data);
  _KLOGE3(KLOG_lwmem_realloc, mem_ptr, size);

  /* Behavior as malloc */
  if(mem_ptr == NULL){
    result = (TASK_NUMBER_FROM_TASKID(_task_get_id()) == SYSTEM_TASK_NUMBER) ? _lwmem_alloc_system(size) : _lwmem_alloc(size);
    return result;
  }/* Endif */

  block_ptr = GET_LWMEMBLOCK_PTR(mem_ptr);
  _int_disable();
  mem_pool_ptr = (LWMEM_POOL_STRUCT_PTR) block_ptr->POOL;

#if MQX_CHECK_VALIDITY
  /* Verify the passed in parameter */
  if (mem_pool_ptr->VALID != LWMEM_POOL_VALID)
  {
      _int_enable();
      _task_set_error(MQX_LWMEM_POOL_INVALID);
      _KLOGX2(KLOG_lwmem_realloc, MQX_LWMEM_POOL_INVALID);
      return NULL;
  } /* Endif */
#endif

#if MQX_CHECK_ERRORS
  /* Verify the passed in parameter */
  if (!((block_ptr->U.S.TASK_NUMBER == TASK_NUMBER_FROM_TASKID(kernel_data->ACTIVE_PTR->TASK_ID))
                  || (block_ptr->U.S.TASK_NUMBER == SYSTEM_TASK_NUMBER)))
  {
      _int_enable();
      _task_set_error(MQX_NOT_RESOURCE_OWNER);
      _KLOGX2(KLOG_lwmem_realloc, MQX_NOT_RESOURCE_OWNER);
      return NULL;
  } /* Endif */
#endif

#if MQX_CHECK_ERRORS
    if (block_ptr->BLOCKSIZE <= (_mem_size) sizeof(LWMEM_BLOCK_STRUCT))
    {
        _int_enable();
        _task_set_error(MQX_INVALID_POINTER);
        _KLOGX2(KLOG_lwmem_realloc, MQX_INVALID_POINTER);
        return NULL;
    } /* Endif */
#endif

  /* size of given memory block */
  old_size = block_ptr->BLOCKSIZE - (_mem_size) sizeof(LWMEM_BLOCK_STRUCT);

  _int_enable();

  if(size > 0){

    realloc_error = (size < old_size) ? _lwmem_free_part_internal(mem_ptr, size) : _lwmem_alloc_extend_internal(mem_ptr, size);

#if MQX_CHECK_ERRORS
    if(realloc_error != MQX_OK && realloc_error != MQX_INVALID_SIZE){
      _task_set_error(realloc_error);
      _KLOGX2(KLOG_lwmem_realloc, realloc_error);
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
    copy_size=size;
  }/* Endif */

  /* Create new memory block, copy data from given memory block to new memory block and free given memory block */
  if(result != mem_ptr){
    result = (block_ptr->U.S.TASK_NUMBER == SYSTEM_TASK_NUMBER) ? _lwmem_alloc_system(size) : _lwmem_alloc(size);

#if MQX_CHECK_ERRORS
  if(result == NULL){
    _KLOGX2(KLOG_lwmem_realloc, result);
    return result;
  }/* Endif */
#endif

    _mem_copy(mem_ptr, result, copy_size);
#if MQX_CHECK_ERRORS && !NDEBUG
    error = _lwmem_free(mem_ptr);
    assert(error == MQX_OK);
#else
    _lwmem_free(mem_ptr);
#endif

  }/* Endif */

  _KLOGX2(KLOG_lwmem_realloc, result);
  return (result);
}/* Endbody */

#endif /* MQX_USE_LWMEM */
