#include "disk.h"
#include "io/io.h"
#include "config.h"
#include "status.h"
#include "memory/heap/kheap.h"

static struct disk* disks;

static int disk_read_sector(int lba, int total, void* buf);
void disk_init()
{

	// For now we only handle a single disk
	disks = kzalloc(sizeof(struct disk));
	disks[0].type = PEACHOS_DISK_TYPE_REAL;
	disks[0].sector_size = PEACHOS_SECTOR_SIZE;
}


int disk_read_block(struct disk* idisk, unsigned int lba, int total, void* buf)
{
	if (idisk != &disks[0])
	{
		return -EIO;
	}

	return disk_read_sector(lba, total, buf);
}


struct disk* disk_get(int index)
{
	if (index != 0)
	{
		return 0;
	}
	
	return &disks[0];		
	
}


// This is a private function.
static int disk_read_sector(int lba, int total, void* buf)
{
	// Those ports are specific to the x86 IO bus
	// https://wiki.osdev.org/I/O_Ports

	// The addresses used are described by:
	// https://wiki.osdev.org/ATA_PIO_Mode#Registers
	// Wher 0xF0 is the base address.
	outb(0x1F6, (lba >> 24) | 0xE0); // Drive
    	outb(0x1F2, total); // Sector counts
    	outb(0x1F3, (unsigned char)(lba & 0xff)); // Sector number
    	outb(0x1F4, (unsigned char)(lba >> 8));   // Cylinder low
    	outb(0x1F5, (unsigned char)(lba >> 16));  // Cylinder high
    	outb(0x1F7, 0x20);			  // Command

    	unsigned short* ptr = (unsigned short*) buf;
    	for (int b = 0; b < total; b++)
    	{
        	// Poll state until ready
		char c;
		do 
		{
			c = insb(0x1F7);
		}
        	while(!(c & 0x08));
		

        	// Copy one word (2 bytes) 256 times hence a sector
        	for (int i = 0; i < 256; i++)
        	{
            		*ptr = insw(0x1F0);
            		ptr++;
        	}
    }
    return 0;

}
