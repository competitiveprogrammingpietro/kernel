#include "paging.h"
#include "memory/heap/kheap.h"

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
			ptable[j] = 
				(j * PAGING_PAGE_SIZE) + (i * PAGING_PAGE_SIZE *PAGING_ENTRIES_PER_TABLE);
		}
		
		pdirectory[i] = (uint32_t) ptable | flags | PAGING_IS_WRITEABLE; // This is still unclear it is pointer
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
