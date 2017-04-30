
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
*   This file contains definitions private to the light weight
*   memory manger.
*
*
*END************************************************************************/
#ifndef __lwmem_prv_h__
#define __lwmem_prv_h__

/*--------------------------------------------------------------------------*/
/*
 *                    CONSTANT DEFINITIONS
 */

/* The correct value for the light weight memory pool VALID field */
#define LWMEM_POOL_VALID   (_mqx_uint)(0x6C6D6570)    /* "lmep" */

/* The smallest amount of memory that is allocated */
#define LWMEM_MIN_MEMORY_STORAGE_SIZE \
   ((_mem_size)(sizeof(LWMEM_BLOCK_STRUCT) + PSP_MEMORY_ALIGNMENT) & \
   PSP_MEMORY_ALIGNMENT_MASK)


/*--------------------------------------------------------------------------*/
/*
 *                      MACROS DEFINITIONS
 */

/*
 * get the location of the block pointer, given the address as provided
 * to the application by _lwmem_alloc.
 */
#define GET_LWMEMBLOCK_PTR(addr) \
   (LWMEM_BLOCK_STRUCT_PTR)((void *)((unsigned char *)(addr) - \
      sizeof(LWMEM_BLOCK_STRUCT)))

/*--------------------------------------------------------------------------*/
/*
 *                    DATATYPE DECLARATIONS
 */

/* LWMEM BLOCK STRUCT */
/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief This structure is used to define the storage blocks used by the memory
 * manager in MQX.
 */
typedef struct lwmem_block_struct
{
   /*! \brief The size of the block. */
   _mem_size      BLOCKSIZE;

   /*! \brief The pool the block came from. */
   _lwmem_pool_id POOL;

   /*!
    * \brief For an allocated block, this is the task ID of the owning task.
    * When on the free list, this points to the next block on the free list.
    */
   union {
      void       *NEXTBLOCK;
      struct {
         _task_number    TASK_NUMBER;
         _mem_type       MEM_TYPE;
      } S;
   } U;

} LWMEM_BLOCK_STRUCT, * LWMEM_BLOCK_STRUCT_PTR;
/*! \endcond */

#define _GET_LWMEMBLOCK_TYPE(ptr)      (((LWMEM_BLOCK_STRUCT_PTR)GET_LWMEMBLOCK_PTR(ptr))->U.S.MEM_TYPE)

/*--------------------------------------------------------------------------*/
/*
 *                  PROTOTYPES OF FUNCTIONS
 */

#ifdef __cplusplus
extern "C" {
#endif
#ifndef __TAD_COMPILE__

extern void     *_lwmem_alloc_internal(_mem_size, TD_STRUCT_PTR, _lwmem_pool_id, bool);
extern _mqx_uint _lwmem_alloc_extend_internal(void*, _mem_size);
extern _mqx_uint _lwmem_free_part_internal(void*, _mem_size);
extern void     *_lwmem_alloc_at_internal(_mem_size, void *, TD_STRUCT_PTR, _lwmem_pool_id, bool);
extern void     *_lwmem_alloc_align_internal(_mem_size, _mem_size, TD_STRUCT_PTR, _lwmem_pool_id, bool);
extern _mem_size _lwmem_get_free_internal(_lwmem_pool_id);
extern void      _lwmem_cleanup_internal(TD_STRUCT_PTR);
extern void      _lwmem_transfer_internal(void *, TD_STRUCT_PTR);
extern _mqx_uint _lwmem_transfer_td_internal(void *, TD_STRUCT_PTR,
   TD_STRUCT_PTR);
extern _mqx_uint _lwmem_init_internal(void);
extern void     *_lwmem_get_next_block_internal(TD_STRUCT_PTR,void *);
extern _lwmem_pool_id _lwmem_create_pool_mapped(void *, _mem_size);

#if MQX_ENABLE_USER_MODE
void            *_usr_lwmem_alloc_internal(_mem_size);
_mqx_uint        _usr_lwmem_free_internal(void *);
_lwmem_pool_id   _usr_lwmem_create_pool_internal(LWMEM_POOL_STRUCT_PTR, void *, _mem_size);
#endif

#endif
#ifdef __cplusplus
}
#endif

#endif /* __lwmem_prv_h__ */
/* EOF */
