#include "kernel.h"
#include "idt/idt.h"
#include <stdint.h>


#define COLOR_TERMINAL(c,col) ((col << 8) | c)
extern void divide_by_zero_error();

uint16_t * video_memory = 0;
uint16_t current_col, current_row;

// Simple function to write a string on the screen
void write_string(char * str) {
	
	while (*str) {

		if (*str == '\n') {
			current_col = 0;
			current_row = (current_row + 1) % VGA_ROWS;
			str++;
			continue;
		}


		video_memory[current_row * VGA_COLS + current_col] = COLOR_TERMINAL(*str, 0x2);
		
		if ( (current_col + 1) % VGA_COLS == 0) {
			current_col = 0;
			current_row = (current_row + 1) % VGA_ROWS;
			str++;
			continue;
		}
		current_col++;
		str++;
	}
}

void kernel_main() {
  
  video_memory = (uint16_t*) 0xB8000;

  /* Blank out the screen */
  for (int i = 0; i < VGA_ROWS; i++) {
  	for (int j = 0; j < VGA_COLS; j++) {
		video_memory[i * VGA_COLS + j] = COLOR_TERMINAL(' ', 0x0);
	}
  }

  current_col = current_row = 0;
  // base_framebuffer[0] = 0x0341; // Endianess makes it 0x41 0x3 - letter 'A' color green

  write_string("Hello world\nThis is a BRAND NEW OS!\n");
  idt_init();
  divide_by_zero_error();
}



