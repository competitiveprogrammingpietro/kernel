#ifndef KERNEL_H
#define KERNEL_H

#define VGA_COLS 80
#define VGA_ROWS 20
#define PEACHOS_MAX_PATH 108
#define COLOR_TERMINAL(c,col) ((col << 8) | c)
void write_string(char * str);
void kernel_main();
#endif
