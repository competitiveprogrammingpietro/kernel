#ifndef KERNEL_H
#define KERNEL_H

#define VGA_COLS 80
#define VGA_ROWS 20
#define PEACHOS_MAX_PATH 108
#define COLOR_TERMINAL(c, col) ((col << 8) | c)
#define ERROR(value) (void *)(value)
#define ERROR_I(value) (int)(value)
#define ISERR(value) ((int)value < 0)
void write_string(char *str);
void write_char(char c);
void print(char *str);
void kernel_main();
void print_u32_binary(unsigned int x);
void print_u32_hex(unsigned int x);
#endif
