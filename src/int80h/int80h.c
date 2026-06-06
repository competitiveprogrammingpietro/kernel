#include "int80h.h"
#include "task/task.h"
#include "kernel.h"
#include "keyboard/keyboard.h"
#include "task/process.h"
#include "task/task.h"
#include "string/string.h"

void *int80h_sum(struct interrupt_frame *iframe)
{
    int one = (int)task_stack_item(task_current(), 1);
    int two = (int)task_stack_item(task_current(), 0);
    return (void *)one + two;
}

void *int80h_print(struct interrupt_frame *iframe)
{
    void *user_space_buffer_ptr = task_stack_item(task_current(), 0);

    char buffer[1024];
    int res = task_copy_from_task_to_kernel(
        task_current(),
        user_space_buffer_ptr,
        buffer,
        sizeof(buffer));

    if (res != 0)
    {
        panic("task_copy_from_task_to_kernel: fatal error");
    }
    print(buffer);
    return 0;
}

void *int80h_getkey(struct interrupt_frame *iframe)
{
    char c = keyboard_pop();
    return (void *)((int)c);
}

void *int80h_putkey(struct interrupt_frame *iframe)
{
    char buf[2] = {0.};
    buf[0] = (char)(int)task_stack_item(task_current(), 0);
    print(buf);
    return (void *)0;
}

void *int80h_malloc(struct interrupt_frame *frame)
{
    size_t size = (int)task_stack_item(task_current(), 0);
    return process_malloc(task_current()->process, size);
}

void *int80h_free(struct interrupt_frame *frame)
{
    void *ptr = (void *)task_stack_item(task_current(), 0);
    process_free(task_current()->process, ptr);
    return 0;
}

void *int80h_exec_process(struct interrupt_frame *iframe)
{
    char executable_file_path[PEACHOS_MAX_PATH];
    void *executable_file_path_ptr = task_stack_item(task_current(), 0);
    int res = task_copy_from_task_to_kernel(
        task_current(),
        executable_file_path_ptr,
        executable_file_path,
        sizeof(executable_file_path));

    if (res < 0)
    {
        return (void *)res;
    }

    // Our 'PATH' is drive '0:/' - simple and effective for now.
    char path[PEACHOS_MAX_PATH];
    strcpy(path, "0:/");
    strcpy(&path[3], executable_file_path);

    struct process *process = 0;
    process_load_executable(path, &process);

    if (res < 0)
    {
        return (void *)res;
    }

    // Set the process as the current and switch to the task context,
    process_set_current(process);
    task_context(process->task);

    // Now we are ready to ask the CPU to execute the task's code, this makes
    // the function not to return any value, the return that follows is there
    // to make the compiler happy.
    task_execute_context(&process->task->registers);
    return 0;
}

void *isr80h_exit(struct interrupt_frame *frame)
{
    // terminate the process and start executing the next one
    process_terminate(task_current()->process);
    struct task *t = task_get_next();
    if (!t)
    {
        panic("There are no currently other task to run\n");
    }
    print("Executing next task as the current process has ended\n");
    process_set_current(t->process);
    task_context(t);
    task_execute_current();
    return (void *)0;
}

void int80h_register_commands()
{
    isr80h_register_command(SYSTEM_COMMAND_SUM, int80h_sum);
    isr80h_register_command(SYSTEM_COMMAND_PRINT, int80h_print);
    isr80h_register_command(SYSTEM_COMMAND_GETKEY, int80h_getkey);
    isr80h_register_command(SYSTEM_COMMAND_PUTKEY, int80h_putkey);
    isr80h_register_command(SYSTEM_COMMAND_MALLOC, int80h_malloc);
    isr80h_register_command(SYSTEM_COMMAND_FREE, int80h_free);
    isr80h_register_command(SYSTEM_COMMAND_EXEC_PROCESS, int80h_exec_process);
    isr80h_register_command(SYSTEM_COMMAND_EXIT, int80h_exec_process);
}
