#ifndef ELF_LOADER_H
#define ELF_LOADER_H
#include "config.h"

struct elf_file
{
    char filename[PEACHOS_MAX_PATH];

    int in_memory_size;

    // The RAM address where this file is loaded at, physical memory
    void *elf_memory;

    // The base and end virtual addresses
    void *virtual_base_address;
    void *virtual_end_address;

    // The base and end physical address for this binary
    void *physical_base_address;
    void *physical_end_address;
};
int elf_load_file(const char *path, struct elf_file **file_out);
void elf_close(struct elf_file *file);
#endif