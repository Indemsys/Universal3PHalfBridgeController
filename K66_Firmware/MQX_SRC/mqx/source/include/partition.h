
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
*   This file contains public definitions for the memory partition
*   component
*
*
*END************************************************************************/
#ifndef __partition_h__
#define __partition_h__

#include <mqx_cnfg.h>
#if (! MQX_USE_PARTITIONS) && (! defined (MQX_DISABLE_CONFIG_CHECK))
#error PARTITION component is currently disabled in MQX kernel. Please set MQX_USE_PARTITIONS to 1 in user_config.h and recompile kernel.
#endif

/* Partition error return value */
#define PARTITION_NULL_ID       ((_partition_id)0)

/* Partition error codes */

#define PARTITION_INVALID                 (PART_ERROR_BASE|0x01)
#define PARTITION_OUT_OF_BLOCKS           (PART_ERROR_BASE|0x02)
#define PARTITION_BLOCK_INVALID_ALIGNMENT (PART_ERROR_BASE|0x03)
#define PARTITION_TOO_SMALL               (PART_ERROR_BASE|0x04)
#define PARTITION_INVALID_TASK_ID         (PART_ERROR_BASE|0x05)
#define PARTITION_BLOCK_INVALID_CHECKSUM  (PART_ERROR_BASE|0x06)
#define PARTITION_INVALID_TYPE            (PART_ERROR_BASE|0x07)
#define PARTITION_ALL_BLOCKS_NOT_FREE     (PART_ERROR_BASE|0x08)

/* The definition of a partition id */
typedef void *_partition_id;

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TAD_COMPILE__
extern void         *_partition_alloc(_partition_id);
extern void         *_partition_alloc_system(_partition_id);
extern void         *_partition_alloc_system_zero(_partition_id);
extern void         *_partition_alloc_zero(_partition_id);
extern _mem_size     _partition_calculate_size(_mqx_uint, _mem_size);
extern _mqx_uint     _partition_calculate_blocks(_mem_size, _mem_size);
extern _mqx_uint     _partition_create_component(void);
extern _partition_id _partition_create(_mem_size, _mqx_uint, _mqx_uint, _mqx_uint);
extern _partition_id _partition_create_at(void *, _mem_size, _mem_size);
extern _mqx_uint     _partition_destroy(_partition_id);
extern _mqx_uint     _partition_extend(_partition_id, void *, _mem_size);
extern _mqx_uint     _partition_free(void *);
extern _mqx_uint     _partition_get_free_blocks(_partition_id);
extern _mqx_uint     _partition_get_max_used_blocks(_partition_id);
extern _mqx_uint     _partition_get_number_of(void);
extern _mqx_uint     _partition_get_total_blocks(_partition_id);
extern _mem_size     _partition_get_block_size(_partition_id);
extern _mem_size     _partition_get_total_size(_partition_id);
extern _mqx_uint     _partition_test(_partition_id *, void **, 
   void   **);
extern _mqx_uint     _partition_transfer(void *, _task_id);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __partition_h__ */
/* EOF */
