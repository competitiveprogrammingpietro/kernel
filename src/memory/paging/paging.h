#ifndef PAGING_H
#define PAGING_H
#include <stdint.h>
#include <stddef.h>

// https://wiki.osdev.org/Paging#Articles
#define PAGING_CACHE_DISABLED  0b00010000
#define PAGING_WRITE_THROUGH   0b00001000
#define PAGING_ACCESS_FROM_ALL 0b00000100
#define PAGING_IS_WRITEABLE    0b00000010
#define PAGING_IS_PRESENT      0b00000001


#define PAGING_ENTRIES_PER_TABLE  1024
#define PAGING_PAGE_SIZE 	  4096


// Each directory addresses 4GB.
struct page_directory_4GB
{
    uint32_t* entry;
};

struct page_directory_4GB* page_directory_new(uint8_t flags);
void paging_switch(uint32_t* directory);
void enable_paging(); // Implemeted in assembly
#endif
