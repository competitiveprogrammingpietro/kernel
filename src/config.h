#ifndef CONFIG_H
#define CONFIG_H
#define PEACOS_INTERRUPT_NUMBER 512

// I don't get those yet
#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10

// For reference: https://wiki.osdev.org/Memory_Map_(x86)
// 100M heap size
#define PEACHOS_HEAP_SIZE_BYTES 0x6400000
#define PEACHOS_HEAP_BLOCK_SIZE 4096
#define PEACHOS_HEAP_ADDRESS 0x01000000
#define PEACHOS_HEAP_TABLE_ADDRESS 0x00007E00

// Hard disks stuff
#define PEACHOS_SECTOR_SIZE 512

// Filesystems related section
#define PEACHOS_MAX_PATH 108

// This is the maximum number of filesystem types this OS allows
#define PEACHOS_MAX_FILESYSTEMS 12

// Max number of file descriptors
#define PEACHOS_MAX_FILE_DESCRIPTORS 512

// Paging
#define PEACHOS_TOTAL_GDT_SEGMENTS 6

// Task/Process - those addresses are virtual
#define PEACHOS_PROGRAM_VIRTUAL_ADDRESS 0x400000
#define PEACHOS_USER_PROGRAM_STACK_SIZE (4 * 4096)
#define PEACHOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START 0x3FF000
#define PEACHOS_PROGRAM_VIRTUAL_STACK_ADDRESS_END PEACHOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START - PEACHOS_USER_PROGRAM_STACK_SIZE

// This is unusual for a commercial OS, but this is a learning one. We do not
// allow processes to allocate more than 1024 times.
#define PEACHOS_MAX_PROGRAM_ALLOCATIONS 1024

#define PEACHOS_MAX_PROCESSES 12

// Offset to the GDT of the table, see gdt_internal, although offset are relative
// to the real table.
#define USER_DATA_SEGMENT 0x23

// 0x1b : 0001 1011 - hence the segment selector for the user code segmnet holds
// 11 for the selector value, which is the 4th entry in the GDT as defined in
//    kernel.c
// 0 for the TI bit, hence global descriptor table
// 11 for the RPL: hence the lowest possible request privilege level
#define USER_CODE_SEGMENT 0x1b

#define PEACHOS_MAX_ISR80H_COMMANDS 1024
#define PEACHOS_KEYBOARD_BUFFER_SIZE 1024
#endif
