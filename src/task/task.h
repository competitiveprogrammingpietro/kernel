#ifndef TASK_H
#define TASK_H

#include "config.h"
#include "memory/paging/paging.h"
#include "idt/idt.h"
// This is the view of the task as far as the CPU is concerned. This file
// is at the *core* of the whole OS
struct registers
{
    uint32_t edi; // 0
    uint32_t esi; // 4
    uint32_t ebp; // 8
    uint32_t ebx; // 12
    uint32_t edx; // 16
    uint32_t ecx; // 20
    uint32_t eax; // 24

    uint32_t ip;    // 28
    uint32_t cs;    // 32
    uint32_t flags; // 36
    uint32_t esp;   // 40
    uint32_t ss;    // 44
};

struct task
{
    // The page directory of the task
    struct paging_page_directory *page_directory;

    // The registers of the task when the task is not running
    struct registers registers;

    // Process originating this task
    struct process *process;

    // The next task in the linked list
    struct task *next;

    // Previous task in the linked list
    struct task *prev;
};

struct task *task_new(struct process *p);
struct task *task_current();
struct task *task_get_next();
int task_free(struct task *task);
void task_save_state(struct task *task, struct interrupt_frame *frame);
int task_context(struct task *task);
// Implemented in assembly
void task_restore_general_purpose_registers(struct registers *registers);
void task_jump_to(struct registers *registers);
void task_user_segments();
void task_run_head();
int task_copy_from_task_to_kernel(struct task *t, void *virt, void *phys, int n);
void *task_stack_item(struct task *task, int index);
#endif