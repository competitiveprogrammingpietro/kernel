#include "keyboard.h"
#include "task/process.h"
#include "status.h"
#include "keyboard_classic.h"

// We might have to deal with different kinds of keyboard whose layout
// changes at runtime, hence we have a list of keyboards.
static struct keyboard *keyboard_list_head = 0;
static struct keyboard *keyboard_list_tail = 0;

int keyboard_insert(struct keyboard *keyboard)
{
    if (keyboard->init == 0)
    {
        return -EINVARGS;
    }

    if (keyboard_list_tail)
    {
        keyboard_list_tail->next = keyboard;
        keyboard_list_tail = keyboard;
        return keyboard->init();
    }

    // First keyboard in the system
    keyboard_list_head = keyboard;
    keyboard_list_tail = keyboard;
    return keyboard->init();
}

void keyboard_backspace(struct process *process)
{
    process->keyboard_buffer.tail--;
    process->keyboard_buffer.buffer[process->keyboard_buffer.tail] = 0x00;
}

void keyboard_push(char c)
{
    struct process *process = process_current();
    if (!process)
    {
        return;
    }

    process->keyboard_buffer.buffer[process->keyboard_buffer.tail] = c;
    process->keyboard_buffer.tail =
        (process->keyboard_buffer.tail + 1) % PEACHOS_KEYBOARD_BUFFER_SIZE;
}

char keyboard_pop()
{
    if (!task_current())
    {
        return 0;
    }

    struct process *process = task_current()->process;
    char c = process->keyboard_buffer.buffer[process->keyboard_buffer.head];

    if (c == 0x00)
    {
        // Nothing to pop return zero.
        return 0;
    }

    // Increment head
    process->keyboard_buffer.buffer[process->keyboard_buffer.head] = 0;
    process->keyboard_buffer.head =
        (process->keyboard_buffer.head + 1) % PEACHOS_KEYBOARD_BUFFER_SIZE;
    return c;
}

void keyboard_init()
{
    // Insert the classic keyboard
    keyboard_insert(keyboard_classic());
}