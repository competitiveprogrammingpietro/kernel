#ifndef KEYBOARD_H
#define KEYBOARD_H

// https://wiki.osdev.org/I8042_PS/2_Controller
#define PS2_PORT 0x64
#define PS2_COMMAND_ENABLE_FIRST_PORT 0xAE


typedef int (*KEYBOARD_INIT_FUNCTION)();

struct keyboard
{
    KEYBOARD_INIT_FUNCTION init;
    char name[20];
    struct keyboard *next;
};

void keyboard_init();
void keyboard_backspace();
void keyboard_push(char c);
char keyboard_pop();

#endif