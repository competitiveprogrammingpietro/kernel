#include "paging.h"
#include "memory/heap/kheap.h"
#include "status.h"

// Implemented in assenbly
void paging_load_directory(uint32_t* directory);

static uint32_t* current_directory = 0;

/*
 * Initialise a page directory which addresses 4GB of memory, the page table
 * entries are set to the identity function, that is, addesses 0x123456 gets
 * mapped onto 0x123456
 */
struct page_directory_4GB* page_directory_new(uint8_t flags)
{
	uint32_t* pdirectory = kzalloc(sizeof(uint32_t) * PAGING_ENTRIES_PER_TABLE);
	for (int i = 0; i < PAGING_ENTRIES_PER_TABLE; i++)
	{
		uint32_t* ptable = kzalloc(sizeof(uint32_t) * PAGING_ENTRIES_PER_TABLE);
		
		for (int j = 0; j < PAGING_ENTRIES_PER_TABLE; j++)
		{
			ptable[j] = (
					(j * PAGING_PAGE_SIZE) 
					+ (i * PAGING_PAGE_SIZE *PAGING_ENTRIES_PER_TABLE))
				 | flags;
		}
		
		// The reason why this works lies in the fact that the ptable - which is 4bytes * 1024 = 4K
		// is a block of the heap, that is, the address stored in ptable is *always* going to be a
		// multiple of 0x100000 hence we still have the clean 12 bits left where metadata is stored.
		pdirectory[i] = (uint32_t) ptable | flags | PAGING_IS_WRITEABLE;
	}		
	struct page_directory_4GB* directory = kzalloc(sizeof(struct page_directory_4GB));
	directory->entry = pdirectory;
	return directory;
}

// Load the directory into CR3 and set the global pointer
void paging_switch(uint32_t* directory)
{
    paging_load_directory(directory);
    current_directory = directory;
}

// Given a 4k aligned virtual address extract the directory and page table
// indexes
int virtual_address_extract_directory_page_idxs(void* virtual_address,
						uint32_t* directory_index,
						uint32_t* table_index)
{
    
    // We can only extract those for aligned blocks 
    if (!PAGE_IS_ALIGNED(virtual_address))
    {
	return -EINVARGS;
    }  

    // The directory index is made of the first 10 bits hence we discard the 22 
    *directory_index = ((uint32_t) virtual_address) >> 22;
    
    // The table index are bits between 21:12, the the first 10 out of the way
    *table_index = (((uint32_t) virtual_address) & 0x0003ffff);
    *table_index = *table_index >> 12;
    return 0;
}

// Given a page representing a block of virtual addresses map it into the
// physical address given
int map_page_into_address(uint32_t* directory, void* virtual, uint32_t physical)
{
	// We can only extract those for aligned blocks 
	if (!PAGE_IS_ALIGNED(virtual))
	{
		return -EINVARGS;
    	}
	
	uint32_t directory_idx, table_idx;
	
	int res = virtual_address_extract_directory_page_idxs(
			virtual, 
			&directory_idx,
			&table_idx
			);
		
	if (res < 0)
	{
		return res;
	}

	uint32_t dentry = directory[directory_idx];
	
	// First 20 bits are the page table entry address, discard flags
	uint32_t* tentry = (uint32_t*) (dentry & 0xfffff000);
	
	// Set the page to be mapped at that physical address
	tentry[table_idx] = physical;
	return 0;
}
