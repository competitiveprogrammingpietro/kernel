#include "task.h"
#include "kernel.h"
#include "status.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "memory/paging/paging.h"
#include "idt/idt.h"

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