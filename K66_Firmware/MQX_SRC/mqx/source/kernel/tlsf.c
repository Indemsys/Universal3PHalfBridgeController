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


#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <mqx.h>
#include <bsp.h>
#include "tlsf.h"
#include "tlsfbits.h"

/*
** Debugging structure
*/
typedef struct integrity_t
{
	int prev_status;
	int status;
} integrity_t;

/*
** Cast and min/max macros.
*/

#define tlsf_cast(t, exp)	((t) (exp))
#define tlsf_min(a, b)		((a) < (b) ? (a) : (b))
#define tlsf_max(a, b)		((a) > (b) ? (a) : (b))

/*
** Set debug print output
*/
#if _DEBUG_TLSF
#define DEBUG_TLSF printf
/*
** Set assert macro, if it has not been provided by the user.
*/
#if !defined (tlsf_assert)
#define tlsf_assert assert
#endif
#else
#define DEBUG_TLSF( ... )
/*
** Set assert macro, if it has not been provided by the user.
*/
#if !defined (tlsf_assert)
#define tlsf_assert( ... )
#endif
#endif


/*
** Static assertion mechanism.
*/

#define _tlsf_glue2(x, y) x ## y
#define _tlsf_glue(x, y) _tlsf_glue2(x, y)
#define tlsf_static_assert(exp) \
	typedef char _tlsf_glue(static_assert, __LINE__) [(exp) ? 1 : -1]

/* This code has been tested on 32- and 64-bit (LP/LLP) architectures. */
tlsf_static_assert(sizeof(int) * CHAR_BIT == 32);
tlsf_static_assert(sizeof(size_t) * CHAR_BIT >= 32);
tlsf_static_assert(sizeof(size_t) * CHAR_BIT <= 64);

/* SL_INDEX_COUNT must be <= number of bits in sl_bitmap's storage type. */
tlsf_static_assert(sizeof(unsigned int) * CHAR_BIT >= SL_INDEX_COUNT);

/* Ensure we've properly tuned our sizes. */
tlsf_static_assert(ALIGN_SIZE == SMALL_BLOCK_SIZE / SL_INDEX_COUNT);

/*
** Data structures and associated constants.
*/



/*
** Since block sizes are always at least a multiple of 4, the two least
** significant bits of the size field are used to store the block status:
** - bit 0: whether block is busy or free
** - bit 1: whether previous block is busy or free
*/
static const size_t block_header_free_bit = 1 << 0;
static const size_t block_header_prev_free_bit = 1 << 1;

const size_t tlsf_alignment_size = ALIGN_SIZE;

/*
** The size of the block header exposed to used blocks is the size field.
** The prev_phys_block field is stored *inside* the previous free block.
*/
const size_t block_header_overhead = sizeof(size_t);

/* User data starts directly after the size field in a used block. */
static const size_t block_start_offset =
	offsetof(block_header_t, size) + sizeof(size_t);

/*
** A free block must be large enough to store its header minus the size of
** the prev_phys_block field, and no larger than the number of addressable
** bits for FL_INDEX.
*/
const size_t block_size_min = 
	sizeof(block_header_t) - sizeof(block_header_t*);
const size_t block_size_max = tlsf_cast(size_t, 1) << FL_INDEX_MAX;



/* A type used for casting when doing pointer arithmetic. */
typedef ptrdiff_t tlsfptr_t;

/*
** block_header_t member functions.
*/

#define block_size(_block_) ((_block_)->size & ~((block_header_free_bit) | (block_header_prev_free_bit)))


#define block_set_size(_block_, _size_) \
{ \
	const size_t oldsize = (_block_)->size; \
	(_block_)->size = (_size_) | (oldsize & ((block_header_free_bit) | (block_header_prev_free_bit))); \
} 

#define block_is_last(_block_) (0 == block_size((_block_)))


#define block_is_free(_block_) tlsf_cast(int, (_block_)->size & (block_header_free_bit))


#define block_set_free(_block_) \
{ \
    (_block_)->size |= (block_header_free_bit); \
}

#define block_set_used(_block_) \
{ \
    (_block_)->size &= ~(block_header_free_bit); \
}

#define block_is_prev_free(_block_) tlsf_cast(int, (_block_)->size & (block_header_prev_free_bit))

#define block_set_prev_free(_block_) \
{ \
	(_block_)->size |= (block_header_prev_free_bit); \
}

#define block_set_prev_used(_block_) \
{ \
	(_block_)->size &= ~(block_header_prev_free_bit); \
}

#define block_from_ptr(ptr) (tlsf_cast(block_header_t*, tlsf_cast(unsigned char*, (ptr)) - (block_start_offset)))

#define block_to_ptr(_block_) (tlsf_cast(void*, tlsf_cast(unsigned char*, (_block_)) + (block_start_offset)))

/* Return location of next block after block of given size. */
#define offset_to_block(_ptr_, _size_) (tlsf_cast(block_header_t*, tlsf_cast(tlsfptr_t, (_ptr_)) + (_size_)))

/* Return location of previous block. */
#define block_prev(_block_) ((_block_)->prev_phys_block)

/* Return location of next existing block. */
#define block_next(_block_) offset_to_block(block_to_ptr((_block_)),block_size((_block_)) - (block_header_overhead))



/* Link a new block with its physical neighbor, return the neighbor. */
static inline block_header_t* block_link_next(block_header_t* block)
{
	block_header_t* next = block_next(block);
	next->prev_phys_block = block;
	return next;
}

static inline void block_mark_as_free(block_header_t* block)
{
	/* Link the block to the next block, first. */
	block_header_t* next = block_link_next(block);
	block_set_prev_free(next);
	block_set_free(block);
}

static inline void block_mark_as_used(block_header_t* block)
{
	block_header_t* next = block_next(block);
	block_set_prev_used(next);
	block_set_used(block);
}

#define align_up(x, align) (((x) + ((align) - 1)) & ~((align) - 1))


#define align_down(x, align) ((x) - ((x) & ((align) - 1)))


#define align_ptr(_ptr_, _align_) tlsf_cast(void*, (tlsf_cast(tlsfptr_t, (_ptr_)) + ((_align_) - 1)) & ~((_align_) - 1))

/*
** Adjust an allocation size to be aligned to word size, and no smaller
** than internal minimum.
*/
static size_t adjust_request_size(size_t size, size_t align)
{
	size_t adjust = 0;
	if (size && size < block_size_max)
	{
		const size_t aligned = align_up(size, align);
		adjust = tlsf_max(aligned, block_size_min);
	}
	return adjust;
}

/*
** TLSF utility functions. In most cases, these are direct translations of
** the documentation found in the white paper.
*/

static void mapping_insert(size_t size, int* fli, int* sli)
{
	int fl, sl;
	if (size < SMALL_BLOCK_SIZE)
	{
		/* Store small blocks in first list. */
		fl = 0;
		sl = tlsf_cast(int, size) / (SMALL_BLOCK_SIZE / SL_INDEX_COUNT);
	}
	else
	{
		fl = tlsf_fls_sizet(size);
		sl = tlsf_cast(int, size >> (fl - SL_INDEX_COUNT_LOG2)) ^ (1 << SL_INDEX_COUNT_LOG2);
		fl -= (FL_INDEX_SHIFT - 1);
	}
	*fli = fl;
	*sli = sl;
}

/* This version rounds up to the next block size (for allocations) */
static void mapping_search(size_t size, int* fli, int* sli)
{
	if (size >= (1 << SL_INDEX_COUNT_LOG2))
	{
		const size_t round = (1 << (tlsf_fls_sizet(size) - SL_INDEX_COUNT_LOG2)) - 1;
		size += round;
	}
	mapping_insert(size, fli, sli);
}

static block_header_t* search_suitable_block(control_t* control, int* fli, int* sli)
{
	int fl = *fli;
	int sl = *sli;

	/*
	** First, search for a block in the list associated with the given
	** fl/sl index.
	*/
	unsigned int sl_map = control->sl_bitmap[fl] & (~0 << sl);
	if (!sl_map)
	{
		/* No block exists. Search in the next largest first-level list. */
		const unsigned int fl_map = control->fl_bitmap & (~0 << (fl + 1));
		if (!fl_map)
		{
			/* No free blocks available, memory has been exhausted. */
			return 0;
		}

		fl = tlsf_ffs(fl_map);
		*fli = fl;
		sl_map = control->sl_bitmap[fl];
	}
	tlsf_assert(sl_map && "internal error - second level bitmap is null");
	sl = tlsf_ffs(sl_map);
	*sli = sl;

	/* Return the first block in the free list. */
	return control->blocks[fl][sl];
}

/* Remove a free block from the free list.*/
static void remove_free_block(control_t* control, block_header_t* block, int fl, int sl)
{
	block_header_t* prev = block->prev_free;
	block_header_t* next = block->next_free;
	tlsf_assert(prev && "prev_free field can not be null");
	tlsf_assert(next && "next_free field can not be null");
	next->prev_free = prev;
	prev->next_free = next;

	/* If this block is the head of the free list, set new head. */
	if (control->blocks[fl][sl] == block)
	{
		control->blocks[fl][sl] = next;

		/* If the new head is null, clear the bitmap. */
		if (next == &control->block_null)
		{
			control->sl_bitmap[fl] &= ~(1 << sl);

			/* If the second bitmap is now empty, clear the fl bitmap. */
			if (!control->sl_bitmap[fl])
			{
				control->fl_bitmap &= ~(1 << fl);
			}
		}
	}
}

/* Insert a free block into the free block list. */
static void insert_free_block(control_t* control, block_header_t* block, int fl, int sl)
{
	block_header_t* current = control->blocks[fl][sl];
	tlsf_assert(current && "free list cannot have a null entry");
	tlsf_assert(block && "cannot insert a null entry into the free list");
	block->next_free = current;
	block->prev_free = &control->block_null;
	current->prev_free = block;

	//! tlsf_assert(block_to_ptr(block) == align_ptr(block_to_ptr(block), ALIGN_SIZE)
	//! 	&& "block not aligned properly");
    /* we should be 4bytes offset from the alignment here! */
        tlsf_assert(((tlsfptr_t)block_to_ptr(block) + (tlsfptr_t)block_header_overhead) == (tlsfptr_t)align_ptr((tlsfptr_t)block_to_ptr(block) + (tlsfptr_t)block_header_overhead, ALIGN_SIZE)
  	&& "block not aligned properly");
  
	/*
	** Insert the new block at the head of the list, and mark the first-
	** and second-level bitmaps appropriately.
	*/
	control->blocks[fl][sl] = block;
	control->fl_bitmap |= (1 << fl);
	control->sl_bitmap[fl] |= (1 << sl);
}

/* Remove a given block from the free list. */
static void block_remove(control_t* control, block_header_t* block)
{
	int fl, sl;
	mapping_insert(block_size(block), &fl, &sl);
	remove_free_block(control, block, fl, sl);
}

/* Insert a given block into the free list. */
static void block_insert(control_t* control, block_header_t* block)
{
	int fl, sl;
	mapping_insert(block_size(block), &fl, &sl);
	insert_free_block(control, block, fl, sl);
}

static int block_can_split(block_header_t* block, size_t size)
{
	return block_size(block) >= sizeof(block_header_t) + size;
}

/* Split a block into two, the second of which is free. */
static block_header_t* block_split(block_header_t* block, size_t size)
{
	/* Calculate the amount of space left in the remaining block. */
	block_header_t* remaining =
		offset_to_block(block_to_ptr(block), size - block_header_overhead);

	const size_t remain_size = block_size(block) - (size + block_header_overhead);

	tlsf_assert(block_to_ptr(remaining) == align_ptr(block_to_ptr(remaining), ALIGN_SIZE)
		&& "remaining block not aligned properly");

	tlsf_assert(block_size(block) == remain_size + size + block_header_overhead);
	block_set_size(remaining, remain_size);
	tlsf_assert(block_size(remaining) >= block_size_min && "block split with invalid size");

	block_set_size(block, size);
	block_mark_as_free(remaining);

	return remaining;
}

/* Absorb a free block's storage into an adjacent previous free block. */
static block_header_t* block_absorb(block_header_t* prev, block_header_t* block)
{
	tlsf_assert(!block_is_last(prev) && "previous block can't be last!");
	/* Note: Leaves flags untouched. */
	prev->size += block_size(block) + block_header_overhead;
	block_link_next(prev);
	return prev;
}

/* Merge a just-freed block with an adjacent previous free block. */
static block_header_t* block_merge_prev(control_t* control, block_header_t* block)
{
	if (block_is_prev_free(block))
	{
		block_header_t* prev = block_prev(block);
		tlsf_assert(prev && "prev physical block can't be null");
		tlsf_assert(block_is_free(prev) && "prev block is not free though marked as such");
		block_remove(control, prev);
		block = block_absorb(prev, block);
	}

	return block;
}

/* Merge a just-freed block with an adjacent free block. */
static block_header_t* block_merge_next(control_t* control, block_header_t* block)
{
	block_header_t* next = block_next(block);
	tlsf_assert(next && "next physical block can't be null");

	if (block_is_free(next))
	{
		tlsf_assert(!block_is_last(block) && "previous block can't be last!");
		block_remove(control, next);
		block = block_absorb(block, next);
	}

	return block;
}

/* Trim any trailing block space off the end of a block, return to pool. */
static void block_trim_free(control_t* control, block_header_t* block, size_t size)
{
	tlsf_assert(block_is_free(block) && "block must be free");
	if (block_can_split(block, size))
	{
		block_header_t* remaining_block = block_split(block, size);
		block_link_next(block);
		block_set_prev_free(remaining_block);
		block_insert(control, remaining_block);
	}
}

/* Trim any trailing block space off the end of a used block, return to pool. */
static void block_trim_used(control_t* control, block_header_t* block, size_t size)
{
	tlsf_assert(!block_is_free(block) && "block must be used");
	if (block_can_split(block, size))
	{
		/* If the next block is free, we must coalesce. */
		block_header_t* remaining_block = block_split(block, size);
		block_set_prev_used(remaining_block);

		remaining_block = block_merge_next(control, remaining_block);
		block_insert(control, remaining_block);
	}
}

static block_header_t* block_trim_free_leading(control_t* control, block_header_t* block, size_t size)
{
	block_header_t* remaining_block = block;
	if (block_can_split(block, size))
	{
		/* We want the 2nd block. */
		remaining_block = block_split(block, size - block_header_overhead);
		block_set_prev_free(remaining_block);

		block_link_next(block);
		block_insert(control, block);
	}

	return remaining_block;
}

static block_header_t* block_locate_free(control_t* control, size_t size)
{
	int fl = 0, sl = 0;
	block_header_t* block = 0;

	if (size)
	{
		mapping_search(size, &fl, &sl);
		block = search_suitable_block(control, &fl, &sl);
	}

	if (block)
	{
		tlsf_assert(block_size(block) >= size);
		remove_free_block(control, block, fl, sl);
	}

	return block;
}

static void* block_prepare_used(control_t* control, block_header_t* block, size_t size)
{
	void* p = 0;
	if (block)
	{
		block_trim_free(control, block, size);
		block_mark_as_used(block);
		p = block_to_ptr(block);
	}
	return p;
}

/* Clear structure and point all empty lists at the null block. */
static void control_construct(control_t* control)
{
	int i, j;

	control->block_null.next_free = &control->block_null;
	control->block_null.prev_free = &control->block_null;
        control->block_null.size = TLSF_CHECK_VALUE; /* for validity checking */

	control->fl_bitmap = 0;
	for (i = 0; i < FL_INDEX_COUNT; ++i)
	{
		control->sl_bitmap[i] = 0;
		for (j = 0; j < SL_INDEX_COUNT; ++j)
		{
			control->blocks[i][j] = &control->block_null;
		}
	}
}

/*
** Debugging utilities.
*/
#define tlsf_insist(x) { tlsf_assert(x); if (!(x)) { status--; } }

static void integrity_walker(void* ptr, size_t size, int used, void* user)
{
	block_header_t* block = block_from_ptr(ptr);
	integrity_t* integ = tlsf_cast(integrity_t*, user);
	const int this_prev_status = block_is_prev_free(block) ? 1 : 0;
	const int this_status = block_is_free(block) ? 1 : 0;
	const size_t this_block_size = block_size(block);

	int status = 0;
	tlsf_insist(integ->prev_status == this_prev_status && "prev status incorrect");
	tlsf_insist(size == this_block_size && "block size incorrect");

	integ->prev_status = this_status;
	integ->status += status;
}

int tlsf_check(tlsf_t tlsf)
{
	int i, j;

	control_t* control = tlsf_cast(control_t*, tlsf);
	int status = 0;

	/* Check that the free lists and bitmaps are accurate. */
	for (i = 0; i < FL_INDEX_COUNT; ++i)
	{
		for (j = 0; j < SL_INDEX_COUNT; ++j)
		{
			const int fl_map = control->fl_bitmap & (1 << i);
			const int sl_list = control->sl_bitmap[i];
			const int sl_map = sl_list & (1 << j);
			const block_header_t* block = control->blocks[i][j];

			/* Check that first- and second-level lists agree. */
			if (!fl_map)
			{
				tlsf_insist(!sl_map && "second-level map must be null");
			}

			if (!sl_map)
			{
				tlsf_insist(block == &control->block_null && "block list must be null");
				continue;
			}

			/* Check that there is at least one free block. */
			tlsf_insist(sl_list && "no free blocks in second-level map");
			tlsf_insist(block != &control->block_null && "block should not be null");

			while (block != &control->block_null)
			{
				int fli, sli;
				tlsf_insist(block_is_free(block) && "block should be free");
				tlsf_insist(!block_is_prev_free(block) && "blocks should have coalesced");
				tlsf_insist(!block_is_free(block_next(block)) && "blocks should have coalesced");
				tlsf_insist(block_is_prev_free(block_next(block)) && "block should be free");
				tlsf_insist(block_size(block) >= block_size_min && "block not minimum size");

				mapping_insert(block_size(block), &fli, &sli);
				tlsf_insist(fli == i && sli == j && "block size indexed in wrong list");
				block = block->next_free;
			}
		}
	}
	return status;
}

#undef tlsf_insist

static void default_walker(void* ptr, size_t size, int used, void* user)
{
	(void)user;
	DEBUG_TLSF("\t%p %s size: %x (%p)\n", ptr, used ? "used" : "free", (unsigned int)size, block_from_ptr(ptr));
}

void tlsf_walk_pool(pool_t pool, tlsf_walker walker, void* user)
{
	tlsf_walker pool_walker = walker ? walker : default_walker;
	block_header_t* block =
		offset_to_block(pool, -(int)block_header_overhead);

	while (block && !block_is_last(block))
	{
		pool_walker(
			block_to_ptr(block),
			block_size(block),
			!block_is_free(block),
			user);
		block = block_next(block);
	}
}

size_t tlsf_block_size(void* ptr)
{
	size_t size = 0;
	if (ptr)
	{
		const block_header_t* block = block_from_ptr(ptr);
		size = block_size(block);
	}
	return size;
}

int tlsf_check_pool(pool_t pool)
{
	/* Check that the blocks are physically correct. */
	integrity_t integ = { 0, 0 };
	tlsf_walk_pool(pool, integrity_walker, &integ);

	return integ.status;
}



/*
** Overhead of the TLSF structures in a given memory block passes to
** tlsf_add_pool, equal to the overhead of a free block and the
** sentinel block.
*/
#define tlsf_pool_overhead() (2 * (block_header_overhead))

#define tlsf_alloc_overhead() (block_header_overhead)

pool_t tlsf_add_pool(tlsf_t tlsf, void* mem, size_t bytes, size_t alignment)
{
	block_header_t* block;
	block_header_t* next;

	const size_t pool_overhead = tlsf_pool_overhead();
	const size_t pool_bytes = align_down(bytes - pool_overhead, alignment);

    mem = (void*)align_up(((uint32_t)mem), alignment);

	if (((ptrdiff_t)mem % ALIGN_SIZE) != 0)
	{
		DEBUG_TLSF("tlsf_add_pool: Memory must be aligned by %u bytes.\n",
			(unsigned int)ALIGN_SIZE);
                tlsf_assert(0&&"Memory must be aligned");
		return 0;
	}

	if (pool_bytes < block_size_min || pool_bytes > block_size_max)
	{
		DEBUG_TLSF("tlsf_add_pool: Memory size must be between %u and %u bytes.\n", 
			(unsigned int)(pool_overhead + block_size_min),
			(unsigned int)(pool_overhead + block_size_max));
                tlsf_assert(0&&"Memory size must be between limits! Have you properly set FL_INDEX_MAX?");
		return 0;
	}

	/*
	** Create the main free block. Offset the start of the block slightly
	** so that the prev_phys_block field falls outside of the pool -
	** it will never be used.
	*/
	block = offset_to_block(mem, -(tlsfptr_t)block_header_overhead);
	block_set_size(block, pool_bytes);
	block_set_free(block);
	block_set_prev_used(block);
	block_insert(tlsf_cast(control_t*, tlsf), block);

	/* Split the block to create a zero-size sentinel block. */
	next = block_link_next(block);
	block_set_size(next, 0);
	block_set_used(next);
	block_set_prev_free(next);

	return mem;
}

void tlsf_remove_pool(tlsf_t tlsf, pool_t pool)
{
	control_t* control = tlsf_cast(control_t*, tlsf);
	block_header_t* block = offset_to_block(pool, -(int)block_header_overhead);

	int fl = 0, sl = 0;

	tlsf_assert(block_is_free(block) && "block should be free");
	tlsf_assert(!block_is_free(block_next(block)) && "next block should not be free");
	tlsf_assert(block_size(block_next(block)) == 0 && "next block size should be zero");

	mapping_insert(block_size(block), &fl, &sl);
	remove_free_block(control, block, fl, sl);
}

/*
** TLSF main interface.
*/

#if _DEBUG_TLSF
int test_ffs_fls()
{
	/* Verify ffs/fls work properly. */
	int rv = 0;
	rv += (tlsf_ffs(0) == -1) ? 0 : 0x1;
	rv += (tlsf_fls(0) == -1) ? 0 : 0x2;
	rv += (tlsf_ffs(1) == 0) ? 0 : 0x4;
	rv += (tlsf_fls(1) == 0) ? 0 : 0x8;
	rv += (tlsf_ffs(0x80000000) == 31) ? 0 : 0x10;
	rv += (tlsf_ffs(0x80008000) == 15) ? 0 : 0x20;
	rv += (tlsf_fls(0x80000008) == 31) ? 0 : 0x40;
	rv += (tlsf_fls(0x7FFFFFFF) == 30) ? 0 : 0x80;

	if (rv)
	{
		DEBUG_TLSF("tlsf_create: %x ffs/fls tests failed!\n", rv);
	}
	return rv;
}
#endif

tlsf_t tlsf_create(void* mem)
{
#if _DEBUG_TLSF
	if (test_ffs_fls())
	{
		return 0;
	}
#endif

	if (((tlsfptr_t)mem % ALIGN_SIZE) != 0)
	{
		DEBUG_TLSF("tlsf_create: Memory must be aligned to %u bytes.\n",
			(unsigned int)ALIGN_SIZE);
		return 0;
	}

	control_construct(tlsf_cast(control_t*, mem));

	return tlsf_cast(tlsf_t, mem);
}

tlsf_t tlsf_create_with_pool(void* mem, size_t bytes, size_t alignment)
{
	tlsf_t tlsf;
	mem = (void*)align_up(((uint32_t)mem), alignment); 
    
	if(align_down(bytes, alignment) < tlsf_size())
	{
		/* Supplied memory block is not large enough to hold TLSF book keeping information */
		return NULL;
	}
    
	tlsf = tlsf_create(mem);
	tlsf_add_pool(tlsf, (char*)mem + tlsf_size(), bytes - tlsf_size(), alignment);
	return tlsf;
}

void tlsf_destroy(tlsf_t tlsf)
{
	/* Nothing to do. */
	(void)tlsf;
}

pool_t tlsf_get_pool(tlsf_t tlsf)
{
	return tlsf_cast(pool_t, (char*)tlsf + tlsf_size());
}

void* tlsf_malloc(tlsf_t tlsf, size_t size)
{
	control_t* control = tlsf_cast(control_t*, tlsf);
	const size_t adjust = adjust_request_size(size, ALIGN_SIZE);
	block_header_t* block = block_locate_free(control, adjust);
	return block_prepare_used(control, block, adjust);
}

/* This implementation keeps having a complexity of O(1), since to attain the 
   requested alignment, the allocator looks for a block which has the size of 
   requested_size + alignment + bytes_to_aligned, which is aligned up to the 
   minimum possible alignment in the system (e.g. 4bytes when no cache present,
   32/16bytes when a cache is used).
   However, when the requested allocation size is smaller than the requested payload
   alignment, there can exist a block, which would fulfill this purpose (it has a negative
   offset to an aligned address of bytes_to_aligned and a size of requested_size), nonetheless
   to find such a block, a sequential search would have to be used, which is extremely slow. */
void* tlsf_memalign(tlsf_t tlsf, size_t align, size_t size, size_t bytes_to_aligned, size_t header_align)
{
	control_t* control = tlsf_cast(control_t*, tlsf);
       /*
	** We must allocate an additional minimum block size bytes so that if
	** our free block will leave an alignment gap which is smaller, we can
	** trim a leading free block and release it back to the pool. We must
	** do this because the previous physical block is in use, therefore
	** the prev_phys_block field is not valid, and we can't simply adjust
	** the size of that block.
	*/
	const size_t gap_minimum = sizeof(block_header_t) + bytes_to_aligned;
	const size_t adjust = adjust_request_size(size + gap_minimum, header_align);
	const size_t size_with_gap = adjust_request_size(adjust + align, header_align);

	/* If alignment is less than or equals header alignment, we're done. */
	const size_t aligned_size = (align <= header_align) ? adjust : size_with_gap;

        tlsf_assert((size !=0) && ((align&(~(align-1)))==align) && ((header_align&(~(header_align-1)))==header_align)
                    && "Alignment must be power of 2 and size cannot be zero!");
        
	block_header_t* block = block_locate_free(control, aligned_size);

	/* This can't be a static assert. */
	tlsf_assert(sizeof(block_header_t) == block_size_min + block_header_overhead);

	if (block)
	{
		void* ptr = block_to_ptr(block);
		void* aligned = align_ptr(ptr, align);
		size_t gap = tlsf_cast(size_t,
			tlsf_cast(tlsfptr_t, aligned) - tlsf_cast(tlsfptr_t, ptr));

		/* If gap size is too small, offset to next aligned boundary. */
		if (gap < gap_minimum)
		{
			const size_t gap_remain = gap_minimum - gap;
			const size_t offset = tlsf_max(gap_remain, align);
			const void* next_aligned = tlsf_cast(void*,
				tlsf_cast(tlsfptr_t, aligned) + offset);

			aligned = align_ptr(next_aligned, align);
			gap = tlsf_cast(size_t,
				tlsf_cast(tlsfptr_t, aligned) - tlsf_cast(tlsfptr_t, ptr));
		}

                tlsf_assert(gap >= gap_minimum && "gap size too small");
                gap -= bytes_to_aligned;  /* will return pointer unaligned, so we can fit an additional header */
                block = block_trim_free_leading(control, block, gap);

	}
    else
    {
           /* There can be a sequential search for suitable blocks here, but it is extremely slow,
              especially for huge memories */
           return NULL;
    }

	return block_prepare_used(control, block, adjust);
}

void tlsf_free(tlsf_t tlsf, void* ptr)
{
	/* Don't attempt to free a NULL pointer. */
	if (ptr)
	{
		control_t* control = tlsf_cast(control_t*, tlsf);
		block_header_t* block = block_from_ptr(ptr);
		tlsf_assert(!block_is_free(block) && "block already marked as free");
		block_mark_as_free(block);
		block = block_merge_prev(control, block);
		block = block_merge_next(control, block);
		block_insert(control, block);
	}
}

/*
** The TLSF block information provides us with enough information to
** provide a reasonably intelligent implementation of realloc, growing or
** shrinking the currently allocated block as required.
**
** This routine handles the somewhat esoteric edge cases of realloc:
** - a non-zero size with a null pointer will behave like malloc
** - a zero size with a non-null pointer will behave like free
** - a request that cannot be satisfied will leave the original buffer
**   untouched
** - an extended buffer size will leave the newly-allocated area with
**   contents undefined
*/
void* tlsf_realloc(tlsf_t tlsf, void* ptr, size_t size)
{
	control_t* control = tlsf_cast(control_t*, tlsf);
	void* p = 0;

#ifdef TLSF_HANDLE_REALLOC_FREE_AND_ALLOC
	/* Zero-size requests are treated as free. */
	if (ptr && size == 0)
	{
		tlsf_free(tlsf, ptr);
	}
	/* Requests with NULL pointers are treated as malloc. */
	else if (!ptr)
	{
		p = tlsf_malloc(tlsf, size);
	}
	else
#endif	/* else: handled by higher layer*/
	{
		block_header_t* block = block_from_ptr(ptr);
		block_header_t* next = block_next(block);

		const size_t cursize = block_size(block);
		const size_t combined = cursize + block_size(next) + block_header_overhead;
		const size_t adjust = adjust_request_size(size, ALIGN_SIZE);

		tlsf_assert(!block_is_free(block) && "block already marked as free");

		/*
		** If the next block is used, or when combined with the current
		** block, does not offer enough space, we must reallocate and copy.
		*/
		if (adjust > cursize && (!block_is_free(next) || adjust > combined))
		{
			p = tlsf_malloc(tlsf, size);
			if (p)
			{
				const size_t minsize = tlsf_min(cursize, size);
				memcpy(p, ptr, minsize);
				tlsf_free(tlsf, ptr);
			}
		}
		else
		{
			/* Do we need to expand to the next block? */
			if (adjust > cursize)
			{
				block_merge_next(control, block);
				block_mark_as_used(block);
			}

			/* Trim the resulting block and return the original pointer. */
			block_trim_used(control, block, adjust);
			p = ptr;
		}
	}

	return p;
}


/*
** The TLSF block information provides us with enough information to
** provide a reasonably intelligent implementation of realloc, growing or
** shrinking the currently allocated block as required.
**
** This routine handles the somewhat esoteric edge cases of realloc:
** - a non-zero size with a null pointer will behave like malloc
** - a zero size with a non-null pointer will behave like free
** - a request that cannot be satisfied will leave the original buffer
**   untouched
** - an extended buffer size will leave the newly-allocated area with
**   contents undefined
*/
void* tlsf_realloc_aligned(tlsf_t tlsf, void* ptr, size_t size, size_t alignment, size_t bytes_to_aligned)
{
	control_t* control = tlsf_cast(control_t*, tlsf);
	void* p = 0;

#ifdef TLSF_HANDLE_REALLOC_FREE_AND_ALLOC
	/* Zero-size requests are treated as free. */
	if (ptr && size == 0)
	{
		tlsf_free(tlsf, ptr);
	}
	/* Requests with NULL pointers are treated as malloc. */
	else if (!ptr)
	{
		p = tlsf_memalign(tlsf, alignment, size, bytes_to_aligned, alignment);
	}
	else
#endif	/* else: handled by higher layer*/
	{
		block_header_t* block = block_from_ptr(ptr);
		block_header_t* next = block_next(block);

		const size_t cursize = block_size(block);
		const size_t combined = cursize + block_size(next) + block_header_overhead;
		const size_t adjust = adjust_request_size(size, ALIGN_SIZE);

		tlsf_assert(!block_is_free(block) && "block already marked as free");

		/*
		** If the next block is used, or when combined with the current
		** block, does not offer enough space, we must reallocate and copy.
		*/
		if (adjust > cursize && (!block_is_free(next) || adjust > combined))
		{
                        p = tlsf_memalign(tlsf, alignment, size, bytes_to_aligned, alignment);
			if (p)
			{
				const size_t minsize = tlsf_min(cursize, size);
				memcpy(p, ptr, minsize);
				tlsf_free(tlsf, ptr);
			}
		}
		else
		{
			/* Do we need to expand to the next block? */
			if (adjust > cursize)
			{
				block_merge_next(control, block);
				block_mark_as_used(block);
			}

			/* Trim the resulting block and return the original pointer. */
			block_trim_used(control, block, adjust);
			p = ptr;
		}
	}

	return p;
}


size_t tlsf_min_alloc_size(void)
{
   return tlsf_block_size_min();
}

size_t tlsf_allocator_overhead(void)
{
  return tlsf_alloc_overhead();
}

