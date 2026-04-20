#include "int80h.h"
#include "task/task.h"
#include "kernel.h"

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
    print("hi");
    print(buffer);
    return 0;
}

void int80h_register_commands()
{
    isr80h_register_command(SYSTEM_COMMAND_SUM, int80h_sum);
    isr80h_register_command(SYSTEM_COMMAND_PRINT, int80h_print);
}
