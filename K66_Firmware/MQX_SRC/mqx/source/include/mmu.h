
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
*   This file contains the type definitions for the generic MMU interface
*   functions.
*
*
*END************************************************************************/
#ifndef __mmu_h__
#define __mmu_h__

/* When a region is added to the mmu, the default is to make this region 
 * cachable, copyback, writeable, for both code and data accesses
 */

/* Write accesses to this page are not allowed. */
#define PSP_MMU_WRITE_PROTECTED         (0x00000001)

/* This page is NOT to be cached in the code cache */
#define PSP_MMU_CODE_CACHE_INHIBITED    (0x00000002)

/* This page is NOT to be cached in the data cache */
#define PSP_MMU_DATA_CACHE_INHIBITED    (0x00000004)

/* This page is NOT to be cached in the code or data cache */
#define PSP_MMU_CACHE_INHIBITED         (0x00000006)

/* Write accesses immediately propagate to physical memory. */
#define PSP_MMU_WRITE_THROUGH           (0x00000008)

/* On some MMUs, a write can be made without updating the cache. */
#define PSP_MMU_WRITE_NO_UPDATE         (0x00000020)

/* On some MMUs, a write can be stored into a write buffer, and 
 * stores to memory performed at some future time. 
 */
#define PSP_MMU_WRITE_BUFFERED          (0x00000040)

/* This page is shared with an external hardware device that can snoop. */
#define PSP_MMU_COHERENT                (0x00000080)

/* This page requires that memory accesses are not out-of-order, 
 * or that memory not specifically requested by the software be accessed. 
 */
#define PSP_MMU_GUARDED                 (0x00000100)

/* Read accesses to this page are not allowed. */
#define PSP_MMU_READ_PROTECTED          (0x00000200)

/* Page can contain executable instructions. */
#define PSP_MMU_EXEC_ALLOWED            (0x00000400)

/* Page is locked. Note: Cannot be used for a virtual context */
#define PSP_MMU_PAGE_LOCKED             (0x00000800)

/* Page contains VLE code */
#define PSP_MMU_PAGE_VLE             (0x00001000)

#ifndef __ASM__

/*--------------------------------------------------------------------------*/

/* PSP MMU VINIT STRUCT */
/*!
 * \brief This structure is used to initialize the virtual memory support.
 */
typedef struct psp_mmu_vinit_struct
{
   /*! \brief Where MMU pages can exist. */
   unsigned char *MMU_PAGE_TABLE_BASE;

   /*! \brief How much memory allocated for the tables. */
   uint32_t   MMU_PAGE_TABLE_SIZE;

   /*! \brief Where does unmapped memory start. */
   unsigned char *UNMAPPED_MEMORY_BASE;

   /*! \brief How much memory available. */
   uint32_t   UNMAPPED_MEMORY_SIZE;

} PSP_MMU_VINIT_STRUCT, * PSP_MMU_VINIT_STRUCT_PTR;


/*--------------------------------------------------------------------------*/
/*
 *                  FUNCTION PROTOTYPES
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TAD_COMPILE__

extern _mqx_uint _mmu_add_region(unsigned char *, _mem_size, _mqx_uint);
extern void      _mmu_disable(void);
extern void      _mmu_enable(void);
extern void      _mmu_init(void *);

extern _mqx_uint _mmu_add_vregion(void *, void *, _mem_size, _mqx_uint);
extern _mem_size _mmu_get_vpage_size(void);
extern _mqx_uint _mmu_get_vmem_attributes(void *, void **,
   void   **, _mem_size_ptr, _mqx_uint_ptr);
extern _mqx_uint _mmu_set_vmem_attributes(void *, _mqx_uint, _mem_size);
extern _mqx_uint _mmu_vinit(_mqx_uint, void *);
extern void      _mmu_venable(void);
extern void      _mmu_vdisable(void);
extern _mqx_uint _mmu_vtop(void *, void **);

extern _mqx_uint _mmu_add_vcontext(_task_id, void *, _mem_size, _mqx_uint);
extern _mqx_uint _mmu_create_vcontext(_task_id);
extern _task_id  _mmu_create_vtask(_mqx_uint, _mqx_uint, void *, void *, 
   _mem_size, _mqx_uint);
extern _mqx_uint _mmu_destroy_vcontext(_task_id);
#endif

#ifdef __cplusplus
}
#endif

#endif  /* __ASM__ */

#endif 
/* EOF */
