#ifndef INT80H_H
#define INT80H_H
#include "idt/idt.h"
enum SYSTEM_COMMAND
{

    SYSTEM_COMMAND_SUM,
    SYSTEM_COMMAND_PRINT,

};

void *int80h_sum(struct interrupt_frame *iframe);
void *int80h_print(struct interrupt_frame *iframe);
void int80h_register_commands();
#endif