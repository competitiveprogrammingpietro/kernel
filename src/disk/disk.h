#ifndef DISK_H
#define DISK_H

// Represents a real physical hard disk
#define PEACHOS_DISK_TYPE_REAL 0

typedef unsigned int PEACHOS_DISK_TYPE;

struct disk
{
    PEACHOS_DISK_TYPE type;
    int sector_size;
};

void disk_init();
struct disk* disk_get(int index);
int disk_read_block(struct disk* idisk, unsigned int lba, int total, void* buf);
#endif
