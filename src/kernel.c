#include "kernel.h"
#include "idt/idt.h"
#include "io/io.h"
#include "memory/paging/paging.h"
#include "memory/heap/kheap.h"
#include "disk/disk.h"
#include "disk/streamer.h"
#include "string/string.h"
#include "fs/pparser.h"
#include <stdint.h>


#define COLOR_TERMINAL(c,col) ((col << 8) | c)
extern void divide_by_zero_error();

// Video memory stuff
uint16_t * video_memory = 0;
uint16_t current_col, current_row;

// Kernel paging stuff
static struct page_directory_4GB* kernel_page_directory;

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
		video_memory[i * VGA_COLS + j] = COLOR_TERMINAL(' ', 0x1);
	}
  }

  current_col = current_row = 0;
  // base_framebuffer[0] = 0x0341; // Endianess makes it 0x41 0x3 - letter 'A' color green


  // Heap initialisation
  kheap_init();

  // Interrupt global table initialisation
  idt_init();

   disk_init();

  // Initialise the kernel page directory and load it up
  kernel_page_directory = page_directory_new( PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

  paging_switch(kernel_page_directory->entry);

  enable_paging();
  /* Test for paging 
  */
  size_t m1 = 50;
  void * ptr = kmalloc(m1);

  // Remap virtual adress to different physical to test that paging is working
  // correctly. The magic will happen as address 0x1000 is remapped to ptr
  if (map_page_into_address(kernel_page_directory->entry, (void*) 0x1000, (uint32_t)ptr | PAGING_ACCESS_FROM_ALL | PAGING_IS_PRESENT | PAGING_IS_WRITEABLE ) < 0)
  {
 	write_string("Could not map mem page");
  }

  char* remapptr = (char*) 0x1000;
  remapptr[0] = 'A';
  remapptr[1] = 'B';
  write_string(ptr);
  write_string(remapptr);
  
  // We're now ready to enable interrupts
  enable_interrupts();

  write_string("\n\n\nHello world\nThis is a BRAND NEW OS!\n");

  // Test the disk stremer stuff
  struct disk_stream * ds = disk_stream_new(0);
  
  disk_stream_seek(ds, 0x51e);
  unsigned char byte;
  disk_stream_read(ds, &byte, 1);
  while(1)
  {
     char out[2] = { (char) byte, (char) 0 };
     write_string(out);
  }
}



