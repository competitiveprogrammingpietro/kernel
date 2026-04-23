#include "keyboard_classic.h"
#include "keyboard.h"
#include "io/io.h"
#include "idt/idt.h"
#include "kernel.h"
#include "task/task.h"
#include <stdint.h>
#include <stddef.h>

// We do not bother with UNICODE or anthing else which is not ASCII for now
int classic_keyboard_init();

// https://wiki.osdev.org/PS/2_Keyboard#Scan_Code_Set_1
static uint8_t keyboard_classic_scan_set_one[] = {
    0x00, 0x1B, '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '0', '-', '=',
    0x08, '\t', 'Q', 'W', 'E', 'R', 'T',
    'Y', 'U', 'I', 'O', 'P', '[', ']',
    0x0d, 0x00, 'A', 'S', 'D', 'F', 'G',
    'H', 'J', 'K', 'L', ';', '\'', '`',
    0x00, '\\', 'Z', 'X', 'C', 'V', 'B',
    'N', 'M', ',', '.', '/', 0x00, '*',
    0x00, 0x20, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, '7', '8', '9', '-', '4', '5',
    '6', '+', '1', '2', '3', '0', '.'};


uint8_t classic_keyboard_scancode_to_char(uint8_t scancode)
{

    // Being uint8 it should work
    if (scancode >= sizeof(keyboard_classic_scan_set_one))
    {
        return 0;
    }
    return keyboard_classic_scan_set_one[scancode];
}

void keyboard_classic_handle_interrupt()
{
    kernel_context();
    uint8_t scancode = 0;
    scancode = insb(KEYBOARD_CLASSIC_INPUT_PORT);

    // discarded as we do not handle extended keys
    insb(KEYBOARD_CLASSIC_INPUT_PORT);

    // We only care about pressed keys
    if (scancode & KEYBOARD_CLASSIC_KEY_RELEASED)
    {
        return;
    }

    uint8_t c = classic_keyboard_scancode_to_char(scancode);
    if (c != 0)
    {
        keyboard_push(c);
    }
    task_context(task_current());
}

int keyboard_classic_init()
{
    idt_register_interrupt(KEYBOARD_CLASSIC_ISR_KEYBOARD_INTERRUPT,
                           keyboard_classic_handle_interrupt);
    outb(PS2_PORT, PS2_COMMAND_ENABLE_FIRST_PORT);
    return 0;
}
struct keyboard classic_keyboard = {
    .name = "keyboard_classic",
    .init = keyboard_classic_init};
struct keyboard *keyboard_classic()
{
    return &classic_keyboard;
}