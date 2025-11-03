#include "kheap.h"
#include "heap.h"
#include "config.h"
#include "kernel.h"

struct heap kernel_heap;

void kheap_init()
{
	int total_blocks = PEACHOS_HEAP_SIZE_BYTES / PEACHOS_HEAP_BLOCK_SIZE;	
	
	kernel_heap.entries = (HEAP_BLOCK_TABLE_ENTRY*) PEACHOS_HEAP_TABLE_ADDRESS;
	kernel_heap.total_blocks = total_blocks;

	void* end = (void*) (PEACHOS_HEAP_ADDRESS + PEACHOS_HEAP_SIZE_BYTES);
	int error = heap_create(&kernel_heap, (void*)(PEACHOS_HEAP_ADDRESS), end);

	if (error < 0) 
	{
		write_string("Failed when creating the kernel heap\n");
	}
	
		
}

void* kmalloc(size_t size)
{
	return heap_malloc(&kernel_heap, size);
}

void kfree(void* addr)
{
	return heap_free(&kernel_heap, addr);	
}
