
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
*   This file contains the structure definitions and constants for an
*   application which will be using MQX.
*   All compiler provided header files must be included before mqx.h.
*
*
*END************************************************************************/
#ifndef __lwmem_h__
#define __lwmem_h__

#include <mqx_cnfg.h>
#if (! MQX_USE_LWMEM) && (! defined (MQX_DISABLE_CONFIG_CHECK))
#error LWMEM component is currently disabled in MQX kernel. Please set MQX_USE_LWMEM to 1 in user_config.h and recompile kernel.
#endif

#ifndef __ASM__
/*--------------------------------------------------------------------------*/
/*
 *                    DATATYPE DECLARATIONS
 */

typedef void *_lwmem_pool_id;

/* LWMEM POOL STRUCT */
/*!
 * \brief This structure is used to define the information that defines what
 * defines a light weight memory pool.
 */
typedef volatile struct lwmem_pool_struct
{
   /*! \brief Links lightweight memory pools together. */
   QUEUE_ELEMENT_STRUCT LINK;

   /*! \brief Handle validation stamp. */
   _mqx_uint            VALID;

   /*! \brief The address of the start of Memory Pool blocks. */
   void                *POOL_ALLOC_START_PTR;

   /*! \brief The address of the end of the Memory Pool. */
   void                *POOL_ALLOC_END_PTR;

   /*! \brief The address of the head of the memory pool free list. */
   void                *POOL_FREE_LIST_PTR;

   /*! \brief Pointer used when walking through free list by lwmem_alloc. */
   volatile void                *POOL_ALLOC_PTR;

   /*! \brief Pointer used when freeing memory by lwmem_free. */
   volatile void                *POOL_FREE_PTR;

   /*! \brief Pointer used when testing memory by lwmem_test. */
   void                *POOL_TEST_PTR;

   /*! \brief Pointer used when testing memory by lwmem_test. */
   void                *POOL_TEST2_PTR;

   /*! \brief Pointer used by lwmem_cleanup_internal. */
   void                *POOL_DESTROY_PTR;

   /*! \brief Pointer to highwater mark. */
   void                *HIGHWATER;

} LWMEM_POOL_STRUCT, * LWMEM_POOL_STRUCT_PTR;


#define POOL_USER_RW_ACCESS     (MPU_UM_RW)
#define POOL_USER_RO_ACCESS     (MPU_UM_R)
#define POOL_USER_NO_ACCESS     (0)
/*--------------------------------------------------------------------------*/
/*
 *                  PROTOTYPES OF FUNCTIONS
 */

#ifdef __cplusplus
extern "C" {
#endif
#ifndef __TAD_COMPILE__

extern void            *_lwmem_alloc(_mem_size);
extern void            *_lwmem_alloc_at(_mem_size, void *);
extern void            *_lwmem_alloc_align(_mem_size, _mem_size);
extern void            *_lwmem_alloc_align_from(_lwmem_pool_id, _mem_size, _mem_size);
extern void            *_lwmem_alloc_zero(_mem_size);
extern void            *_lwmem_alloc_from(_lwmem_pool_id, _mem_size);
extern void            *_lwmem_alloc_zero_from(_lwmem_pool_id, _mem_size);
extern void            *_lwmem_realloc(void *,_mem_size);
extern _mem_size        _lwmem_get_free(void);
extern _mem_size        _lwmem_get_free_from(_lwmem_pool_id);

extern void            *_lwmem_alloc_system(_mem_size);
extern void            *_lwmem_alloc_system_align(_mem_size, _mem_size);
extern void            *_lwmem_alloc_system_align_from(_lwmem_pool_id, _mem_size, _mem_size);
extern void            *_lwmem_alloc_system_zero(_mem_size);
extern void            *_lwmem_alloc_system_from(_lwmem_pool_id, _mem_size);
extern void            *_lwmem_alloc_system_zero_from(_lwmem_pool_id, _mem_size);

extern _lwmem_pool_id   _lwmem_create_pool(LWMEM_POOL_STRUCT_PTR, void *, _mem_size);
extern _lwmem_pool_id   _lwmem_create_pool_mapped(void *start, _mem_size  size);
extern _mqx_uint        _lwmem_free(void *);
extern _mqx_uint        _lwmem_get_size(void *);
extern _lwmem_pool_id   _lwmem_set_default_pool(_lwmem_pool_id);
extern _mqx_uint        _lwmem_test(_lwmem_pool_id *, void **);
extern _mqx_uint        _lwmem_transfer(void *, _task_id, _task_id);

extern _lwmem_pool_id   _lwmem_get_system_pool_id(void);
extern _mem_type        _lwmem_get_type(void *);
extern bool             _lwmem_set_type(void *,_mem_type);
extern void            *_lwmem_get_highwater(void) ;

#if MQX_MEM_MONITOR
extern _mem_size       _lwmem_get_size_pools(void);
extern _mem_size       _lwmem_highest_used(void);
#endif

#if MQX_ENABLE_USER_MODE
extern void            *_usr_lwmem_alloc(_mem_size);
extern void            *_usr_lwmem_alloc_from(_lwmem_pool_id, _mem_size);
extern _mqx_uint        _usr_lwmem_free(void *);
extern void            *_usr_lwmem_realloc(void *,_mem_size);
extern _lwmem_pool_id   _usr_lwmem_create_pool(LWMEM_POOL_STRUCT_PTR, void *, _mem_size);
extern _mqx_uint        _mem_set_pool_access(_lwmem_pool_id, uint32_t);
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif /* _ASM_ */
#endif /* __lwmem_h__ */
/* EOF */
