/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
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


#ifndef INCLUDED_tlsf
#define INCLUDED_tlsf
/*
** Two Level Segregated Fit memory allocator, version 3.0.
** Written by Matthew Conte, and placed in the Public Domain.
**	http://tlsf.baisoku.org
**
** Based on the original documentation by Miguel Masmano:
**	http://rtportal.upv.es/rtmalloc/allocators/tlsf/index.shtml
**
** Please see the accompanying Readme.txt for implementation
** notes and caveats.
**
** This implementation was written to the specification
** of the document, therefore no GPL restrictions apply.
*/
#include "mqx_cnfg.h"
#include <stddef.h>
#if defined(__cplusplus)
extern "C" {
#endif

/*
** Constants.
*/


/* 
** Please define this constant in user_config.h to optimize your	**
**	allocator footprint                                           **/
#ifndef MEM_TLSF_MAX_SINGLE_BLOCK_SIZE
/* Provide default value */
#define MEM_TLSF_MAX_SINGLE_BLOCK_SIZE 0x00100000 /* Defaults to 1024kBytes */
#endif


/* Public constants: may be modified. */
enum tlsf_public
{
  /* log2 of number of linear subdivisions of block sizes. */
  /* Tune this if necessary (for small/large memory device) */
#if (MEM_TLSF_MAX_SINGLE_BLOCK_SIZE > 0x00008000)
	SL_INDEX_COUNT_LOG2 = 5,  /* x > 32k */
#elif (MEM_TLSF_MAX_SINGLE_BLOCK_SIZE <= 0x00004000)
        SL_INDEX_COUNT_LOG2 = 3, /* 16k and less*/
#else
        SL_INDEX_COUNT_LOG2 = 4, /* 16k to 32k */
#endif
};

/* Internal constants, do not modify, read-only */
enum tlsf_private
{
	/* All allocation sizes and addresses needs to be aligned to 4 bytes at least*/	
        ALIGN_SIZE_LOG2 = 2,
	ALIGN_SIZE = (1 << ALIGN_SIZE_LOG2),

	/*
	** We support allocations of sizes up to (1 << FL_INDEX_MAX) bits.
	** However, because we linearly subdivide the second-level lists, and
	** our minimum size granularity is 4 bytes, it doesn't make sense to
	** create first-level lists for sizes smaller than SL_INDEX_COUNT * 4,
	** or (1 << (SL_INDEX_COUNT_LOG2 + 2)) bytes, as there we will be
	** trying to split size ranges into more slots than we have available.
	** Instead, we calculate the minimum threshold size, and place all
	** blocks below that size into the 0th first-level list.
	*/


	/*
	** We can increase this to support larger sizes, at the expense
	** of more overhead in the TLSF structure.
	*/
    /* Tune this if necessary (for small/large memory device) */
#if   (MEM_TLSF_MAX_SINGLE_BLOCK_SIZE <= 0x00004000)
        FL_INDEX_MAX = 14, /* Max block = 16k */
#elif (MEM_TLSF_MAX_SINGLE_BLOCK_SIZE <= 0x00010000)
        FL_INDEX_MAX = 16, /* Max block = 64k */
#elif (MEM_TLSF_MAX_SINGLE_BLOCK_SIZE <= 0x00020000)
        FL_INDEX_MAX = 17, /* Max block = 128k */
#elif (MEM_TLSF_MAX_SINGLE_BLOCK_SIZE <= 0x00040000)
        FL_INDEX_MAX = 18, /* Max block = 256k */
#elif (MEM_TLSF_MAX_SINGLE_BLOCK_SIZE <= 0x00080000)
        FL_INDEX_MAX = 19,  /* Max block = 512k */
#elif (MEM_TLSF_MAX_SINGLE_BLOCK_SIZE <= 0x00100000)
        FL_INDEX_MAX = 20,  /* Max block = 1024k */
#elif (MEM_TLSF_MAX_SINGLE_BLOCK_SIZE <= 0x40000000)
		FL_INDEX_MAX = 30, /* Max block = 1G */
#else
		FL_INDEX_MAX = 31, /* Max block = 2G */
#endif

	SL_INDEX_COUNT = (1 << SL_INDEX_COUNT_LOG2),
	FL_INDEX_SHIFT = (SL_INDEX_COUNT_LOG2 + ALIGN_SIZE_LOG2),
	FL_INDEX_COUNT = (FL_INDEX_MAX - FL_INDEX_SHIFT + 1),

	SMALL_BLOCK_SIZE = (1 << FL_INDEX_SHIFT),
};

/* tlsf_t: a TLSF structure. Can contain 1 to N pools. */
/* pool_t: a block of memory that TLSF can manage. */
typedef void* tlsf_t;
typedef void* pool_t;

extern const size_t block_size_min;
extern const size_t block_size_max;

/*
** Size of the TLSF structures in a given memory block passed to
** tlsf_create, equal to the size of a control_t
*/
#define tlsf_size() (sizeof(control_t))

#define tlsf_align_size() (ALIGN_SIZE)

#define tlsf_block_size_min() (block_size_min)

#define tlsf_block_size_max()  (block_size_max)


/*
** Enable/Disable debugging of TLSF module
*/
#define _DEBUG_TLSF 0

#define TLSF_CHECK_VALUE (0xA5A55A5A)

/*
** Block header structure.
**
** There are several implementation subtleties involved:
** - The prev_phys_block field is only valid if the previous block is free.
** - The prev_phys_block field is actually stored at the end of the
**   previous block. It appears at the beginning of this structure only to
**   simplify the implementation.
** - The next_free / prev_free fields are only valid if the block is free.
*/
typedef struct block_header_t
{
	/* Points to the previous physical block. */
	struct block_header_t* prev_phys_block;

	/* The size of this block, excluding the block header. */
	size_t size;

	/* Next and previous free blocks. */
	struct block_header_t* next_free;
	struct block_header_t* prev_free;
} block_header_t;

/* The TLSF control structure. */
typedef struct control_t
{
	/* Empty lists point at this block to indicate they are free. */
	block_header_t block_null;

	/* Bitmaps for free lists. */
	unsigned int fl_bitmap;
	unsigned int sl_bitmap[FL_INDEX_COUNT];

	/* Head of free lists. */
	block_header_t* blocks[FL_INDEX_COUNT][SL_INDEX_COUNT];
} control_t;


/* Create/destroy a memory pool. */
tlsf_t tlsf_create(void* mem);
tlsf_t tlsf_create_with_pool(void* mem, size_t bytes, size_t alignment);
void tlsf_destroy(tlsf_t tlsf);
pool_t tlsf_get_pool(tlsf_t tlsf);

/* Add/remove memory pools. */
pool_t tlsf_add_pool(tlsf_t tlsf, void* mem, size_t bytes, size_t alignment);
void tlsf_remove_pool(tlsf_t tlsf, pool_t pool);

/* malloc/memalign/realloc/free replacements. */
void* tlsf_malloc(tlsf_t tlsf, size_t bytes);
void* tlsf_memalign(tlsf_t tlsf, size_t align, size_t size, size_t bytes_to_aligned, size_t header_align);
void* tlsf_realloc(tlsf_t tlsf, void* ptr, size_t size);
void* tlsf_realloc_aligned(tlsf_t tlsf, void* ptr, size_t size, size_t alignment, size_t bytes_to_aligned);
void tlsf_free(tlsf_t tlsf, void* ptr);

/* Returns internal block size, not original request size */
size_t tlsf_block_size(void* ptr);
size_t tlsf_min_alloc_size(void);
size_t tlsf_allocator_overhead(void);

/* Debugging. */
typedef void (*tlsf_walker)(void* ptr, size_t size, int used, void* user);
void tlsf_walk_pool(pool_t pool, tlsf_walker walker, void* user);
/* Returns nonzero if any internal consistency check fails. */
int tlsf_check(tlsf_t tlsf);
int tlsf_check_pool(pool_t pool);
void* tlsf_get_block_end(void* ptr);

extern const size_t tlsf_alignment_size;
extern const size_t block_header_overhead;

#if defined(__cplusplus)
};
#endif

#endif
