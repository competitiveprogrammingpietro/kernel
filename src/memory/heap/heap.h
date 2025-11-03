#ifndef HEAP_H
#define HEAP_H
#include "config.h"
#include <stddef.h>
#include <stdint.h>


#define HEAP_BLOCK_TABLE_ENTRY_TAKEN 0x01
#define HEAP_BLOCK_TABLE_ENTRY_FREE  0x00
#define HEAP_BLOCK_HAS_NEXT          0b10000000
#define HEAP_BLOCK_IS_FIRST          0b01000000
#define IS_ALIGNED(ptr)              (((unsigned int) ptr) % PEACHOS_HEAP_BLOCK_SIZE == 0)


typedef unsigned char HEAP_BLOCK_TABLE_ENTRY;

/* The heap contains an array of entries plus a counter for the total
 * blocks avaiable in the heap, plus a source address.
 * Each entry is composed by 8 bits as follows:
 * 7         6        5         4      3       2      1       0
 * HAS_NEXT   IS_FIRST UNUSED------------ --------------ENTRY TYPE
 */
struct heap {
 	HEAP_BLOCK_TABLE_ENTRY* entries;
 	size_t total_blocks;
	void*  source_address;
};



int heap_create(struct heap* heap,
		void* ptr,
		void* end);
void heap_free(struct heap * heap, size_t size);
void* heap_malloc(struct heap * heap, size_t size);
#endif
