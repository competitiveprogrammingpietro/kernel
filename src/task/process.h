#ifndef PROCESS_H
#define PROCESS_H
#include <stdint.h>
#include "task.h"
#include "config.h"

struct process
{
    // The process id
    uint16_t id;

    // Name of the executable from which the process has been created
    char filename[PEACHOS_MAX_PATH];

    // The main process task
    struct task *task;

    // The memory (malloc) allocations of the process
    void *allocations[PEACHOS_MAX_PROGRAM_ALLOCATIONS];

    // The physical pointer to the start of the process memory and its size
    void *physical_memory;
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
#endif