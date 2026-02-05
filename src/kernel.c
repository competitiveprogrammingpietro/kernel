#include "kernel.h"
#include "idt/idt.h"
#include "io/io.h"
#include "memory/paging/paging.h"
#include "memory/heap/kheap.h"
#include "disk/disk.h"
#include "disk/streamer.h"
#include "string/string.h"
#include "fs/pparser.h"
#include "fs/fs.h"
#include <stdint.h>

#define COLOR_TERMINAL(c, col) ((col << 8) | c)
extern void divide_by_zero_error();

// Video memory stuff
uint16_t *video_memory = 0;
uint16_t current_col, current_row;

// Kernel paging stuff
static struct page_directory_4GB *kernel_page_directory;

// Simple function to write a string on the screen
void print(char *str)
{
  return write_string(str);
}

void write_string(char *str)
{

  while (*str)
  {

    if (*str == '\n')
    {
      current_col = 0;
      current_row = (current_row + 1) % VGA_ROWS;
      str++;
      continue;
    }

    video_memory[current_row * VGA_COLS + current_col] = COLOR_TERMINAL(*str, 0x2);

    if ((current_col + 1) % VGA_COLS == 0)
    {
      current_col = 0;
      current_row = (current_row + 1) % VGA_ROWS;
      str++;
      continue;
    }
    current_col++;
    str++;
  }
  return;
}

// AI generated method to write a binary
void print_u32_binary(unsigned int x)
{
  int started = 0;

  for (int i = 31; i >= 0; --i)
  {
    char bit = (char)('0' + ((x >> i) & 1u));
    if (!started)
    {
      if (bit == '0' && i != 0)
        continue; // skip leading zeros
      started = 1;
    }
    char buf[2];
    buf[0] = bit;
    buf[1] = 0;
    write_string(buf);
  }
  write_string("\n");
}

void kernel_main()
{

  video_memory = (uint16_t *)0xB8000;

  /* Blank out the screen */
  for (int i = 0; i < VGA_ROWS; i++)
  {
    for (int j = 0; j < VGA_COLS; j++)
    {
      video_memory[i * VGA_COLS + j] = COLOR_TERMINAL(' ', 0x0);
    }
  }

  current_col = current_row = 0;
  // base_framebuffer[0] = 0x0341; // Endianess makes it 0x41 0x3 - letter 'A' color green
  write_string("\n\n\nHello world\nThis is a BRAND NEW OS!\n");

  // Heap initialisation
  kheap_init();

  // Interrupt global table initialisation
  idt_init();

  fs_init();

  disk_init();

  // Initialise the kernel page directory and load it up
  kernel_page_directory = page_directory_new(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

  paging_switch(kernel_page_directory->entry);

  enable_paging();

  enable_interrupts();

  int fd = fopen("0:/file.txt", "r");
  print_u32_binary(fd);
  if (fd > 0)
  {
    char buf[14];
    write_string("We opened file.txt\n");
    fread(buf, 13, 1, fd);
    write_string("We read\n");
    buf[13] = 0x0;
    print(buf);
    unsigned int tmp = buf[0];
    print_u32_binary(tmp);
    write_string("We ended file.txt\n");
  }
  while (1)
  {
  }
}
