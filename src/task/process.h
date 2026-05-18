#ifndef PROCESS_H
#define PROCESS_H
#include <stdint.h>
#include "task.h"
#include "config.h"
#include "loader/elf/elf_loader.h"

#define PROCESS_FILE_TYPE_ELF 0
#define PROCESS_FILE_TYPE_BINARY 1

struct process_memory_allocation
{
    void *ptr;
    size_t size;
};

struct process
{
    // The process id
    uint16_t id;

    // Name of the executable from which the process has been created
    char filename[PEACHOS_MAX_PATH];

    // The main process task
    struct task *task;

    // The memory (malloc) allocations of the process
    struct process_memory_allocation allocations[PEACHOS_MAX_PROGRAM_ALLOCATIONS];

    int allocation_index;

    int process_file_type;

    // The physical pointer to the start of the process memory and its size, the
    // union makes sure both cases: ELF and binary, are covered.
    union
    {
        void *physical_memory;
        struct elf_file *elf_file;
    };

    uint32_t physical_memory_size;

    // The physical pointer to the stack memory
    void *stack;

    struct keyboard_buffer
    {
        char buffer[PEACHOS_KEYBOARD_BUFFER_SIZE]; // circular buffer
        int head, tail;
    } keyboard_buffer;
};

int process_load_executable(
    const char *filename,
    struct process **rprocess);
struct process *process_current();
int process_set_current(struct process *process);
void *process_malloc(struct process *process, size_t size);
void process_free(struct process *process, void *ptr);
#endif