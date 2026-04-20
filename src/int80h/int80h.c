#include "int80h.h"
#include "task/task.h"

void *int80h_sum(struct interrupt_frame *iframe)
{
    int one = (int)task_stack_item(task_current(), 1);
    int two = (int)task_stack_item(task_current(), 0);
    return (void *)one + two;
}

void int80h_register_commands()
{
    isr80h_register_command(SYSTEM_COMMAND_SUM, int80h_sum);
}