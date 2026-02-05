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
void print(const char *str)
{
  while (*str)
  {
    write_char(*str);
    str++;
  }
}

void write_string(char *str)
{
  return print(str);
}

void write_char(char c)
{

  if (c == '\n')
  {
    current_col = 0;
    current_row = (current_row + 1) % VGA_ROWS;
    return;
  }

  video_memory[current_row * VGA_COLS + current_col] = COLOR_TERMINAL(c, 0x2);
  current_col = (current_col + 1) % VGA_COLS;
  if (current_col == 0)
  {
    current_row = (current_row + 1) % VGA_ROWS;
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

void print_u32_hex(unsigned int x)
{
  static const char hex[] = "0123456789abcdef";

  // Special-case 0 so we print at least one digit
  if (x == 0)
  {
    write_char('0');
    return;
  }

  // Find highest non-zero nibble (4-bit chunk)
  int shift = 28;
  while (shift > 0 && ((x >> shift) & 0xFu) == 0)
    shift -= 4;

  // Print from highest nibble down
  for (; shift >= 0; shift -= 4)
  {
    write_char(hex[(x >> shift) & 0xFu]);
  }
}

void panic(const char *msg)
{
  print(msg);
  while (1)
  {
  }
}

void kernel_main()
{

  video_memory = (uint16_t *)0xB8000;

  // Blank out the screen
  for (int i = 0; i < VGA_ROWS; i++)
  {
    for (int j = 0; j < VGA_COLS; j++)
    {
      video_memory[i * VGA_COLS + j] = COLOR_TERMINAL(' ', 0x0);
    }
  }

  current_col = current_row = 0;

  // Test my precious functions
  // print_u32_binary((unsigned int)0x10);
  // print_u32_hex((unsigned int)0xf0);

  // base_framebuffer[0] = 0x0341; // Endianess makes it 0x41 0x3 - letter 'A' color green
  write_string("\n\n\nHello world\nThis is a BRAND NEW OS!\n");

  // Heap initialisation
  kheap_init();

  // Test heap addresses
  /*
  int i = 0;
  while (i < 10)
  {
    void *ptr = kzalloc(58);
    print_u32_hex((unsigned int)ptr);
    print("\n");
    i++;
  }
  */

  panic("The system cannot complete");

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
  if (fd > 0)
  {
    char buf[25];
    fread(buf, 25, 1, fd);
    buf[25] = 0x0;
    print(buf);
    print("\n");
    struct file_stat fs;
    fstat(fd, &fs);
    print_u32_hex((unsigned int)&fs);
    fclose(fd);
    print("OK");
  }
  while (1)
  {
  }
}
