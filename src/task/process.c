#include "process.h"
#include "config.h"
#include "status.h"
#include "task/task.h"
#include "memory/memory.h"
#include "string/string.h"
#include "fs/fs.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "kernel.h"

// The current process that is running
struct process *current_process = 0;
static uint32_t processes_next_free_index = 0;

static struct process *processes[PEACHOS_MAX_PROCESSES] = {};

struct process *process_current()
{
    return current_process;
}

static int process_load_executable_binary(
    const char *filename,
    struct process *process)
{
    int fd = fopen(filename, "r");
    if (!fd)
    {
        return -EIO;
    }

    struct file_stat file_stat;
    if (fstat(fd, &file_stat) != PEACHOS_OK)
    {
        fclose(fd);
        return -EIO;
    }

    void *program_data = kzalloc(file_stat.filesize);
    if (!program_data)
    {
        return -ENOMEM;
    }

    if (fread(program_data, file_stat.filesize, 1, fd) != 1)
    {
        kfree(program_data);
        return -EIO;
    }

    process->physical_memory = program_data;
    process->physical_memory_size = file_stat.filesize;
    return fclose(fd);
}

// This is arguably the most important function of the lot as it creates a new
// process from an executable. The current implemenation loads a straight binary
// not an ELF. What we refer as the ID is also the slot in the array of
// processes.
int process_load_executable(
    const char *filename,
    struct process **rprocess)
{
    struct task *task = 0;
    struct process *process;
    void *process_stack = 0;

    int process_id;
    if (processes_next_free_index + 1 >= PEACHOS_MAX_PROCESSES)
    {
        return -EISTKN;
    }
    process_id = processes_next_free_index;
    processes_next_free_index++;

    process = kzalloc(sizeof(struct process));
    if (!process)
    {
        return -ENOMEM;
    }

    memset(process, 0, sizeof(struct process));
    int r = process_load_executable_binary(filename, process);
    if (r < 0)
    {
        return r;
    }

    process_stack = kzalloc(PEACHOS_USER_PROGRAM_STACK_SIZE);
    if (!process_stack)
    {
        return -ENOMEM;
    }

    strncpy(process->filename, filename, sizeof(process->filename));
    process->stack = process_stack;
    process->id = process_id;

    // Create a task
    task = task_new(process);
    if (ERROR_I(task) == 0)
    {
        return ERROR_I(task);
    }

    process->task = task;

    // Map the loaded exec in virtual addresses, round it up to the nearest page
    // aligned size so we don't fail during the mapping
    r = paging_map_directory(
        process->task->page_directory,
        (void *)PEACHOS_PROGRAM_VIRTUAL_ADDRESS,
        process->physical_memory,
        paging_align_address(process->physical_memory + process->physical_memory_size),
        PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE);

    if (r < 0)
    {
        // TODO: FREE TASK DATA - such as stack ..
        task_free(task);
        return r;
    }

    // Map the stack addresses for the process - in read only for now. As the
    // stack grows upwards the end address is given first.
    r = paging_map_directory(
        process->task->page_directory,
        (void *)PEACHOS_PROGRAM_VIRTUAL_STACK_ADDRESS_END,
        process->stack,
        paging_align_address(process->stack + PEACHOS_USER_PROGRAM_STACK_SIZE),
        PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE);

    if (r < 0)
    {
        task_free(task);
        return r;
    }

    *rprocess = process;

    // Add the process to the array
    processes[process_id] = process;
    return 0;
}