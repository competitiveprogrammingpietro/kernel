#include "elf_loader.h"
#include "elf.h"
#include "status.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "fs/fs.h"

char elf_signature[] = {(char)0x7F, 'E', 'L', 'F'};

// We do not load every type of ELF file as our features set is quite limited
// compared to ELF's capabilities.
int elf_is_valid(struct elf_header *eh)
{
    // Not a valid ELF signature
    if (memcmp((void *)eh, elf_signature, sizeof(elf_signature)) != 0)
    {
        return -ELDELF;
    }

    // We only load 32 bits binaries, ELFCLASSNONE could be removed kept in case
    // is set as default
    if (eh->e_ident[EI_CLASS] != ELFCLASSNONE &&
        eh->e_ident[EI_CLASS] != ELFCLASS32)
    {
        return -ELDELF;
    }

    // Little endian encoding - same as x86 easier to implement
    if (eh->e_ident[EI_DATA] != ELFDATANONE &&
        eh->e_ident[EI_DATA] != ELFDATA2LSB)
    {
        return -ELDELF;
    }

    // Not an exec or loaded somewhere different than our dedicated RAM portion
    // allocated for it
    if (eh->e_type != ET_EXEC &&
        eh->e_entry < PEACHOS_PROGRAM_VIRTUAL_ADDRESS)
    {
        return -ELDELF;
    }

    // It's got to have a program header, with an valid offset
    return eh->e_phoff == 0;
}

int elf_phdr_pt_load(struct elf_file *elf_file, struct elf32_phdr *phdr)
{
    if (elf_file->virtual_base_address >= (void *)phdr->p_vaddr ||
        elf_file->virtual_base_address == 0x00)
    {
        elf_file->virtual_base_address = (void *)phdr->p_vaddr;
        elf_file->physical_base_address = elf_file->elf_memory + phdr->p_offset;
    }

    unsigned int end_virtual_address = phdr->p_vaddr + phdr->p_filesz;
    if (elf_file->virtual_end_address <= (void *)(end_virtual_address) ||
        elf_file->virtual_end_address == 0x00)
    {
        elf_file->virtual_end_address = (void *)end_virtual_address;
        elf_file->physical_end_address = elf_file->elf_memory +
                                         phdr->p_offset +
                                         phdr->p_filesz;
    }
    return 0;
}

int elf_load_file(const char *path, struct elf_file **file_out)
{
    struct elf_file *elf_file = kzalloc(sizeof(struct elf_file));
    int fd = 0;
    int res = fopen(path, "r");
    if (res <= 0)
    {
        return res;
    }

    fd = res;
    struct file_stat stat;
    res = fstat(fd, &stat);
    if (res != 0)
    {
        fclose(fd);
        return res;
    }

    // Read the whole file into memory
    elf_file->elf_memory = kzalloc(stat.filesize);
    res = fread(elf_file->elf_memory, stat.filesize, 1, fd);
    if (res < 0)
    {
        fclose(fd);
        return res;
    }

    // Now process the loaded file - header is right at the start of the loaded
    // file, as you'd expect it to be
    struct elf_header *header = elf_file->elf_memory;

    // Is it valid ?
    res = elf_is_valid(header);
    if (res != 0)
    {
        fclose(fd);
        return res;
    }

    // This is unrealistic but it is good enough for a very simple scenario, to
    // be improved though, as it stands we get the last PHDR only.
    // All we do here is to inspect the ELF file and store the segments' start
    // and end address.
    for (int i = 0; i < header->e_phnum; i++)
    {
        // Get to the actual program header
        struct elf32_phdr *phdr = elf_file->elf_memory +
                                  (header->e_phoff + sizeof(struct elf32_phdr) * i);

        if (phdr->p_type != PT_LOAD)
        {
            fclose(fd);
            return -ELDELF; // *must* not happen
        }

        // Load the addresses into our ELF header
        res = elf_phdr_pt_load(elf_file, phdr);
        if (res < 0)
        {
            fclose(fd);
            return -ELDELF;
        }
    }

    *file_out = elf_file;
    fclose(fd);
    return res;
}

void elf_close(struct elf_file *file)
{
    if (!file)
        return;

    kfree(file->elf_memory);
    kfree(file);
}