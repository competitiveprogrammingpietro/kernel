#include "paging.h"
#include "memory/heap/kheap.h"
#include "status.h"

// Implemented in assenbly
void paging_load_directory(uint32_t *directory);

static struct paging_page_directory *current_directory = 0;

struct paging_page_directory paging_get_current_directory()
{
	return *current_directory; // this is a copy of.
}

/*
 * Initialise a page directory which addresses 4GB of memory, the page table
 * entries are set to the identity function, that is, addesses 0x123456 gets
 * mapped onto 0x123456
 */
struct paging_page_directory *paging_page_directory_new(uint8_t flags)
{
	uint32_t *pdirectory = kzalloc(sizeof(uint32_t) * PAGING_ENTRIES_PER_TABLE);
	for (int i = 0; i < PAGING_ENTRIES_PER_TABLE; i++)
	{
		uint32_t *ptable = kzalloc(sizeof(uint32_t) * PAGING_ENTRIES_PER_TABLE);

		for (int j = 0; j < PAGING_ENTRIES_PER_TABLE; j++)
		{
			ptable[j] = ((j * PAGING_PAGE_SIZE) + (i * PAGING_PAGE_SIZE * PAGING_ENTRIES_PER_TABLE)) | flags;
		}

		// The reason why this works lies in the fact that the ptable - which is 4bytes * 1024 = 4K
		// is a block of the heap, that is, the address stored in ptable is *always* going to be a
		// multiple of 0x100000 hence we still have the clean 12 bits left where metadata is stored.
		pdirectory[i] = (uint32_t)ptable | flags | PAGING_IS_WRITEABLE;
	}
	struct paging_page_directory *directory = kzalloc(sizeof(struct paging_page_directory));
	directory->entry = pdirectory;
	return directory;
}

void paging_page_directory_free(struct paging_page_directory *directory)
{
	// Each directory is made of 1024 page tables each of which is made of
	// 1024 4K pages. Hence the 4GB directory.
	for (int i = 0; i < 1024; i++)
	{
		uint32_t entry = directory->entry[i];
		uint32_t *table = (uint32_t *)(entry & 0xfffff000);
		kfree(table);
	}

	kfree(directory->entry);
	kfree(directory);
}

// Load the directory into CR3 and set the global pointer
void paging_page_directory_switch(struct paging_page_directory *directory)
{
	paging_load_directory(directory->entry);
	current_directory = directory;
}

// Given the base address of a virtual page map it into the given physical
// address
int paging_map_single_page(
	struct paging_page_directory *directory,
	void *virtual,
	uint32_t physical,
	int flags)
{
	// We can only extract those for aligned blocks
	if (!PAGE_IS_ALIGNED(virtual))
	{
		return -EINVARGS;
	}

	// We can only extract those for aligned blocks
	if (!PAGE_IS_ALIGNED(physical))
	{
		return -EINVARGS;
	}

	uint32_t directory_idx, table_idx;

	// The directory index is made of the first 10 bits
	directory_idx = ((uint32_t)virtual) >> 22;

	// While table index are bits between 21:12, the the first 10 out of the way
	// and discard the last 12
	table_idx = (((uint32_t)virtual) & 0x003fffff) >> 12;

	uint32_t dentry = directory->entry[directory_idx];

	// First 20 bits are the page table entry address, discard flags
	uint32_t *tentry = (uint32_t *)(dentry & 0xfffff000);

	// Set the page to be mapped at that physical address
	tentry[table_idx] = physical | flags;
	return 0;
}

// paging_map_directory maps the WHOLE directory for the virtual address virt
// into the phys, this relies in the phys_end being set to cover
int paging_map_directory(
	struct paging_page_directory *directory,
	void *virt,
	void *phys,
	void *phys_end,
	int flags)
{
	int res = 0;
	if ((uint32_t)virt % PAGING_PAGE_SIZE)
	{
		return -EINVARGS;
	}
	if ((uint32_t)phys % PAGING_PAGE_SIZE)
	{
		return -EINVARGS;
	}
	if ((uint32_t)phys_end % PAGING_PAGE_SIZE)
	{
		return -EINVARGS;
	}

	if ((uint32_t)phys_end < (uint32_t)phys)
	{
		return -EINVARGS;
	}

	// This is the interesting bit, we only map a single page at the time
	// and we only map what is given to us as a physical start and end.
	uint32_t total_bytes = phys_end - phys;
	int total_pages = total_bytes / PAGING_PAGE_SIZE;
	void *current_virt, *current_phys;
	current_virt = virt;
	current_phys = phys;

	for (int i = 0; i < total_pages; i++)
	{

		// Important detail: we OR in the page's flags
		res = paging_map_single_page(
			directory,
			current_virt,
			(uint32_t)current_phys,
			flags);

		if (res != 0)
			return res;

		current_virt += PAGING_PAGE_SIZE;
		current_phys += PAGING_PAGE_SIZE;
	}
	return 0;
}

// align the given address to the page size by increase it if necesary
void *paging_align_address(void *ptr)
{
	if (PAGE_IS_ALIGNED(ptr))
	{
		return ptr;
	}

	uint32_t address = (uint32_t)ptr;
	return (void *)(address + PAGING_PAGE_SIZE - (address % PAGING_PAGE_SIZE));
}

// Given a virtual address and page directory base address returns the page
// table entry for it as it is, flags included
uint32_t paging_get_page_from_address(uint32_t *page_directory, void *virt)
{
	uint32_t page_directory_entry = 0;
	uint32_t page_table_entry = 0;

	if (!PAGE_IS_ALIGNED(virt))
	{
		return -EINVARGS;
	}

	page_directory_entry = ((uint32_t)virt) >> 22;
	page_table_entry = ((uint32_t)virt & 0x003fffff) >> 12;

	uint32_t *page_table = (uint32_t *)page_directory[page_directory_entry];

	// This returns it with the page flags
	return page_table[page_table_entry];
}