#ifndef PAGING_H
#define PAGING_H
#include <stdint.h>
#include <stddef.h>

// https://wiki.osdev.org/Paging#Articles
#define PAGING_CACHE_DISABLED 0b00010000
#define PAGING_WRITE_THROUGH 0b00001000
#define PAGING_ACCESS_FROM_ALL 0b00000100
#define PAGING_IS_WRITEABLE 0b00000010
#define PAGING_IS_PRESENT 0b00000001

/* A 'linear address' is made of:
   +----10----+----10----+----11-----+
     dir.       page tab.   offset

    Hence the page directory is 10 bits  1024
          the page table is 10 bits      1024
          the page size is 12 bits       4096
*/
#define PAGING_ENTRIES_PER_TABLE 1024
#define PAGING_PAGE_SIZE 4096

#define PAGE_IS_ALIGNED(ptr) (((uint32_t)ptr) % PAGING_PAGE_SIZE == 0)

// Each directory addresses 4GB.
struct paging_page_directory
{
    uint32_t *entry;
};

struct paging_page_directory *paging_page_directory_new(uint8_t flags);
void paging_page_directory_switch(struct paging_page_directory *directory);
void paging_enable(); // Implemented in assembly
int paging_map_single_page(struct paging_page_directory *, void *, uint32_t, int);
void paging_page_directory_free(struct paging_page_directory *d);
int paging_map_directory(
    struct paging_page_directory *directory,
    void *virt,
    void *phys,
    void *phys_end,
    int flags);
void *paging_align_address(void *ptr);
uint32_t paging_get_page_from_address(uint32_t *page_directory, void *virt);
struct paging_page_directory paging_get_current_directory();
#endif
