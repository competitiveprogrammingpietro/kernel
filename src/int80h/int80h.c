#include "int80h.h"

void *int80h_sum(struct interrupt_frame *iframe)
{
    return (void*)1;
}

void int80h_register_commands()
{
    isr80h_register_command(SYSTEM_COMMAND_SUM, int80h_sum);
}