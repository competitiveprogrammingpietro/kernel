#ifndef CONFIG_H
#define CONFIG_H
#define PEACOS_INTERRUPT_NUMBER 512
#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10

// For reference: https://wiki.osdev.org/Memory_Map_(x86)
// 100M heap size
#define PEACHOS_HEAP_SIZE_BYTES 0x6400000
#define PEACHOS_HEAP_BLOCK_SIZE 4096  
#define PEACHOS_HEAP_ADDRESS    0x01000000
#define PEACHOS_HEAP_TABLE_ADDRESS 0x00007E00

// Hard disks stuff
#define PEACHOS_SECTOR_SIZE 512

#endif
