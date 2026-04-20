#include "keyboard_classic.h"
#include "keyboard.h"
#include "io/io.h"
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


int keyboard_classic_init()
{
    outb(PS2_PORT, PS2_COMMAND_ENABLE_FIRST_PORT);
    return 0;
}

uint8_t classic_keyboard_scancode_to_char(uint8_t scancode)
{

    // Being uint8 it should work
    if (scancode >= sizeof(keyboard_classic_scan_set_one))
    {
        return 0;
    }
    return keyboard_classic_scan_set_one[scancode];
}

void clasic_keyboard_handle_interrupt()
{
}

struct keyboard classic_keyboard = {
    .name = "keyboard_classic",
    .init = keyboard_classic_init};
struct keyboard *keyboard_classic()
{
    return &classic_keyboard;
}