#include "task.h"
#include "kernel.h"
#include "status.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "memory/paging/paging.h"
#include "idt/idt.h"
#include "string/string.h"

// The current task that is running
static struct task *current_task = 0;

// Tasks linked list
static struct task *task_tail = 0;
static struct task *task_head = 0;

struct task *task_current()
{
    return current_task;
}

struct task *task_new(struct process *process)
{
    int res = 0;
    struct task *task = kzalloc(sizeof(struct task));
    if (!task)
    {
        return ERROR(-ENOMEM);
    }

    memset(task, 0, sizeof(struct task));

    // Map the entire 4GB address space to its self
    task->page_directory = paging_page_directory_new(PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    if (!task->page_directory)
    {
        return ERROR(-EIO);
    }

    task->registers.ip = PEACHOS_PROGRAM_VIRTUAL_ADDRESS;
    task->registers.ss = USER_DATA_SEGMENT;
    task->registers.cs = USER_CODE_SEGMENT;
    task->registers.esp = PEACHOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START;
    task->process = process;

    if (res != PEACHOS_OK)
    {
        task_free(task);
        return ERROR(res);
    }

    // First task to be run
    if (task_head == 0)
    {
        task_head = task;
        task_tail = task;
    }
    else
    {
        // Append it to the tasks' list
        task_tail->next = task;
        task->prev = task_tail;
        task_tail = task;
    }

    current_task = task;
    return task;
}

struct task *task_get_next()
{

    // We return the head if we are at the tail
    if (!current_task->next)
    {
        return task_head;
    }
    return current_task->next;
}

static void task_list_remove(struct task *task)
{
    // It is not allowed to remove the current task
    if (task == current_task)
    {
        panic(
            "task_list_remove(): it is not allowed to remove the current task");
    }

    // I am the head
    if (task_head == task)
    {
        task_head = task_head->next;
        return;
    }

    // I am the tail
    if (task_tail == task)
    {
        task_tail = task->prev;
        return;
    }

    // I am in somewhere the head and the tail
    task->prev->next = task->next;
    return;
}

int task_free(struct task *task)
{
    paging_page_directory_free(task->page_directory);
    task_list_remove(task); // I don't like that being done here

    // Finally free the task data
    kfree(task);
    return 0;
}

int task_context(struct task *task)
{
    task_user_segments(); // Switch segments to user segment
    current_task = task;
    paging_page_directory_switch(task->page_directory);
    return 0;
}

// Idiotic function here just for the time being ..
void task_run_head()
{
    if (!current_task)
    {
        panic("task_run_first_ever_task(): No current task exists!\n");
    }

    task_context(task_head);
    task_jump_to(&task_head->registers);
}

void task_save_state(struct task *task, struct interrupt_frame *frame)
{
    task->registers.ip = frame->ip;
    task->registers.cs = frame->cs;
    task->registers.flags = frame->flags;
    task->registers.esp = frame->esp;
    task->registers.ss = frame->ss;
    task->registers.eax = frame->eax;
    task->registers.ebp = frame->ebp;
    task->registers.ebx = frame->ebx;
    task->registers.ecx = frame->ecx;
    task->registers.edi = frame->edi;
    task->registers.edx = frame->edx;
    task->registers.esi = frame->esi;
}

// UNTESTED. Copy n bytes from base address virt - within task's address space -
// into the physical addres, kernel address, phys.
int task_copy_from_task_to_kernel(struct task *t, void *virt, void *phys, int n)
{
    // If we are to copy more then PAGING_PAGE_SIZE the algorithm would need
    // to be changed, it'd get more complex
    if (n >= PAGING_PAGE_SIZE)
    {
        return -EINVARGS;
    }

    char *buffer = kzalloc(n);
    if (!buffer)
    {
        return -ENOMEM;
    }

    // Get the page entry for the kernel space address of buffer - the task initial
    // mapping directory maps the entire 4GB space to itself, but that can change
    // since the kernel might relocate that to the address where the executable
    // was loaded, hence we save such page table entry here.
    uint32_t task_page_entry_for_buffer_address = paging_get_page_from_address(
        t->page_directory->entry,
        buffer);

    // Map the process page table entry for virtual address buffer to its physical
    // address
    int res = paging_map_single_page(
        t->page_directory,
        buffer,
        (uint32_t)buffer,
        PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

    if (res)
    {
        kfree(buffer);
        return res;
    }

    // Now the kernel and the task share some memory, that is, the virtual
    // address of buffer points to the physical address of buffer. The kernel
    // isn't relocated so it does not matter that much, hence we save the
    // kernel directory, switch to the task's directory and copy the content
    // of virt into buffer
    struct paging_page_directory kernel_directory =
        paging_get_current_directory();
    paging_page_directory_switch(t->page_directory);
    strncpy(buffer, virt, n);

    // Now the job is done, we can revert to the kernel directory, restore the
    // task's page entry and free the buffer, whose solely purpose was to share
    // memory among the process and the kernel.
    paging_page_directory_switch(&kernel_directory);

    res = paging_map_single_page(
        t->page_directory,
        buffer,
        task_page_entry_for_buffer_address & 0xfffff000, // Flags omitted
        task_page_entry_for_buffer_address & 0x00000fff  // Same flags
    );

    if (res)
    {
        kfree(buffer);
        return res;
    }

    // Now copy the data in the actual desired destination, and free the temporary
    // buffer created to share memory
    strncpy(phys, buffer, n);
    kfree(buffer);
    return 0;
}

// Retrieve an item on the task's stack by switching to its context, getting
// the item at the provided index and returning it. The entry context it is
// the kernel context and it is restored before exiting.
void *task_stack_item(struct task *task, int index)
{
    void *result;

    uint32_t *sp = (uint32_t *)task->registers.esp;
    task_context(task);
    result = (void *)sp[index];
    kernel_context();
    return result;
}