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
#include "loader/elf/elf_loader.h"
#include "loader/elf/elf.h"

// The current process that is running
struct process *current_process = 0;
static uint32_t processes_next_free_index = 0;

static struct process *processes[PEACHOS_MAX_PROCESSES] = {};

int process_set_current(struct process *process)
{
    current_process = process;
    return 0;
}

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
    process->process_file_type = PROCESS_FILE_TYPE_BINARY;
    return fclose(fd);
}

static int process_load_executable_elf(
    const char *filename,
    struct process *process)
{
    int res;
    struct elf_file *ef = 0;
    res = elf_load_file(filename, &ef);

    if (res != 0)
    {
        return res;
    }

    process->process_file_type = PROCESS_FILE_TYPE_ELF;
    process->elf_file = ef;
    return 0;
}

// This is arguably among the most important functions of the lot as it
// creates a new process from an executable.
int process_load_executable(
    const char *filename,
    struct process **rprocess,
    char *input)
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

    // We are hopful for an ELF binary, however we fall back to a raw binary
    // if that fails.
    int r = process_load_executable_elf(filename, process);
    if (r != 0)
    {
        r = process_load_executable_binary(filename, process);
    }

    // Neither ELF nor raw binary, return error
    if (r < 0)
    {
        return r;
    }

    process_stack = kzalloc(PEACHOS_USER_PROGRAM_STACK_SIZE);
    if (!process_stack)
    {
        return -ENOMEM;
    }

    // Copy the source file across
    strncpy(process->filename, filename, sizeof(process->filename));

    // Copy the arguments onto the *end* of the stack, ..
    strncpy(process_stack, input, PEACHOS_PROGRAM_ARGUMENT_MAX_SIZE);

    // .. and onto the process arg, used for debugging
    strncpy(process->argument, input, PEACHOS_PROGRAM_ARGUMENT_MAX_SIZE);

    process->stack = process_stack;
    process->id = process_id;
    process->allocation_index = 0;

    // Create a task
    task = task_create_from_process(process);
    if (ERROR_I(task) == 0)
    {
        return ERROR_I(task);
    }

    process->task = task;

    task_queue(task);

    // The mapping logic beheaves differently for raw binaries or ELF, hence the
    // switch
    if (process->process_file_type == PROCESS_FILE_TYPE_BINARY)
    {

        // Map the loaded exec in virtual addresses, round it up to the nearest page
        // aligned size so we don't fail during the mapping
        // The first 1MB is reserved, the next 3MB are taken by the kernel, the process
        // is then loaded at 0x400000 virtual.
        r = paging_map_directory(
            process->task->page_directory,
            (void *)PEACHOS_PROGRAM_VIRTUAL_ADDRESS,
            process->physical_memory,
            paging_align_address_next(process->physical_memory + process->physical_memory_size),
            PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE);

        if (r < 0)
        {
            // TODO: FREE TASK DATA - such as stack ..
            task_free(task);
            return r;
        }
    }

    if (process->process_file_type == PROCESS_FILE_TYPE_ELF)
    {

        // The paging mapping uses the ELF addresses - we need to iterate through
        // the ELF segments to ensure that the correct permission flags are being
        // set.
        struct elf_header *eh = (struct elf_header *)process->elf_file->elf_memory;

        for (int i = 0; i < eh->e_phnum; i++)
        {
            struct elf32_phdr *phdr =
                process->elf_file->elf_memory +
                (eh->e_phoff + sizeof(struct elf32_phdr) * i);

            int flags = PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL;
            if (phdr->p_flags & PF_W)
            {
                flags = flags | PAGING_IS_WRITEABLE;
            }

            // Map the ELF segment
            r = paging_map_directory(
                process->task->page_directory,
                paging_align_address_previous((void *)phdr->p_vaddr),
                process->elf_file->elf_memory + phdr->p_offset, // PHYSICAL ADDRESS START

                // p_filesz: a static variable has zero size, but the p_memsz
                // is the size in memory
                paging_align_address_next(
                    process->elf_file->elf_memory + phdr->p_offset + phdr->p_memsz), // END ADDRESS
                flags);

            if (r < 0)
            {
                // TODO: FREE TASK DATA - such as stack ..
                task_free(task);
                return r;
            }
        }
    }

    // Map the stack addresses for the process - in read only for now. As the
    // stack grows upwards the end address is given first.
    r = paging_map_directory(
        process->task->page_directory,
        (void *)PEACHOS_PROGRAM_VIRTUAL_STACK_ADDRESS_END,
        process->stack,
        paging_align_address_next(process->stack + PEACHOS_USER_PROGRAM_STACK_SIZE),
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

void *process_malloc(struct process *process, size_t size)
{
    void *ptr = kzalloc(size);
    if (!ptr)
    {
        return ERROR(-ENOMEM);
    }

    // Map the memory to userspace, the mapping is extremely simple as we merely
    // map the address to itself, setting the correct flags.
    int res = paging_map_single_page(
        process->task->page_directory,
        ptr,
        (uint32_t)ptr,
        PAGING_IS_PRESENT | PAGING_IS_WRITEABLE | PAGING_ACCESS_FROM_ALL);

    if (res != 0)
    {
        kfree(ptr);
        return ERROR(res);
    }

    // Rudimentary, get the first empty spot, if the index has reached the max
    // amount of allocations wrap it around
    if (process->allocation_index == PEACHOS_MAX_PROGRAM_ALLOCATIONS)
    {
        process->allocation_index = 0;
    }

    int i = process->allocation_index;
    for (; i < PEACHOS_MAX_PROGRAM_ALLOCATIONS; i++)
    {
        // A size of zero marks it as free
        if (process->allocations[i].size == 0x00)
            break;
    }

    if (i == PEACHOS_MAX_PROGRAM_ALLOCATIONS)
    {
        return ERROR(-ENOMEM);
    }
    process->allocations[i].ptr = ptr;
    process->allocations[i].size = size;
    process->allocation_index = i + 1;
    return ptr;
}

void process_free(struct process *process, void *ptr)
{

    // We cannot allow the process to free someone else's memory
    int found = 0;
    for (int i = 0; i < PEACHOS_MAX_PROGRAM_ALLOCATIONS; i++)
    {
        if (process->allocations[i].ptr != ptr)
            continue;

        process->allocations[i].ptr = 0x00;
        process->allocations[i].size = 0x00;

        // All we do from a paging standpoint is to amend the page's flags
        // to ensure that a page fault is raised if the memory is accessed
        int res = paging_map_single_page(
            process->task->page_directory,
            ptr,
            (uint32_t)ptr,
            0x0); // These flags ensure that the page is not accessible
        if (res < 0)
        {
            panic("process_free(): could not map a single page of memory");
        }
        found = 1;
        break;
    }

    if (!found)
    {
        return;
    }

    // We can now free the kernel space memory
    kfree(ptr);
}

int process_terminate(struct process *process)
{

    // Free all data allocated during the lifetime of the process
    for (int i = 0; i < PEACHOS_MAX_PROGRAM_ALLOCATIONS; i++)
    {
        process_free(process, process->allocations[i].ptr);
    }

    // Free the binary image loaded into memory
    switch (process->process_file_type)
    {
    case PROCESS_FILE_TYPE_BINARY:
        kfree(process->physical_memory);
    case PROCESS_FILE_TYPE_ELF:
        elf_close(process->elf_file);
    default:
        return -EINVARGS;
    }

    kfree(process->stack);
    task_free(process->task);

    // Remove the process from our list of processes to run
    processes[process->id] = 0x0;
}