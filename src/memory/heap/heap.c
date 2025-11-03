#include "heap.h"
#include "status.h"
#include "../memory.h"

// Initialise the structs for a working heap
int heap_create(struct heap* heap,
		void* ptr,
		void* end)
{
	if (!IS_ALIGNED(ptr)) {
		return -EINVARGS;		
	}

	if (!IS_ALIGNED(end)) {
		return -EINVARGS;
	}	
	
	// Compute the size of the heap, in term of number of blocks
	heap->source_address = ptr;
 	heap->total_blocks = ((size_t) (end - ptr)) / PEACHOS_HEAP_BLOCK_SIZE;
	memset(heap->entries, HEAP_BLOCK_TABLE_ENTRY_FREE, heap->total_blocks);
	return PEACHOS_OK;
}


// Allocate as many blocks as necessary
void* heap_malloc(struct heap * heap, size_t size)
{
	// Align size to the 4096 block size, that is, make it a multiple of
	// the block size
	size_t blocks_number;

        if (size % PEACHOS_HEAP_BLOCK_SIZE == 0)
	{
		blocks_number = size;
	}
	else
	{
		blocks_number = (size/PEACHOS_HEAP_BLOCK_SIZE) + 1;
	}
	
	int start_block = -1;
	int blocks_counter = 0;

	// Now look in the table entries for that many free blocks
	for (int i = 0; i < heap->total_blocks; i++)
	{
		unsigned char entry_block_state = heap->entries[i] & 0x0f;

		if (entry_block_state != HEAP_BLOCK_TABLE_ENTRY_FREE)
		{
			
			// Reset the search
			start_block = -1;
			blocks_counter = 0;
			continue;
		}		
		
		// Take note of the start block if still seeking it		
		start_block = start_block == -1 ? i : start_block;
		
		// Progress the counter as we are adding more blocks, return if enough blocks
		blocks_counter++;
		if (blocks_counter == blocks_number)
		{
			break;
		}
	}
	
	if (start_block == -1)
	{
		return (void*) 0; // Return a NULL pointer
	}
	
	// Mark blocks as taken and return the address
	for (int i = start_block; i < start_block + blocks_number; i++)
	{
		HEAP_BLOCK_TABLE_ENTRY entry;

		if (i == start_block)
		{
			entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_IS_FIRST;
		}
		else if (i == start_block + blocks_number - 1) // Last one is not marked with 'HAS_NEXT'
		{
			entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN;
		} 
		else 
		{
			entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_HAS_NEXT;
		}
		heap->entries[i] = entry;
	}

	// Return the address
	return heap->source_address + (start_block * PEACHOS_HEAP_BLOCK_SIZE);
}

void heap_free(struct heap * heap, size_t size)
{
	

}
