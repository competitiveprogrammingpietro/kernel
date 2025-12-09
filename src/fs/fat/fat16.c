#include "fat16.h"
#include "string/string.h"
#include "status.h"
#include "config.h"
#include "disk/disk.h"
#include "disk/streamer.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "kernel.h"
#include <stdint.h>

// It is really hard to find the original FAT16 specifications, I used a combination
// of FAT32 specs plus the os wiki website. Check out the specs folder
// https://wiki.osdev.org/FAT#FAT_16

#define PEACHOS_FAT16_SIGNATURE 0x29
#define PEACHOS_FAT16_FAT_ENTRY_SIZE 0x02
#define PEACHOS_FAT16_BAD_SECTOR 0xFF7
#define PEACHOS_FAT16_UNUSED 0x00


typedef unsigned int FAT_ITEM_TYPE;
#define FAT_ITEM_TYPE_DIRECTORY 0
#define FAT_ITEM_TYPE_FILE 1

// Fat directory entry attributes bitmask
#define FAT_FILE_READ_ONLY 0x01
#define FAT_FILE_HIDDEN 0x02
#define FAT_FILE_SYSTEM 0x04
#define FAT_FILE_VOLUME_LABEL 0x08
#define FAT_FILE_SUBDIRECTORY 0x10
#define FAT_FILE_ARCHIVED 0x20
#define FAT_FILE_DEVICE 0x40
#define FAT_FILE_RESERVED 0x80


struct fat_header_extended
{
    uint8_t drive_number;
    uint8_t win_nt_bit;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_id_string[11];
    uint8_t system_id_string[8];
} __attribute__((packed));

struct fat_header
{
    uint8_t short_jmp_ins[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_copies;
    uint16_t root_dir_entries;
    uint16_t number_of_sectors;
    uint8_t media_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_setors;
    uint32_t sectors_big;
} __attribute__((packed));

struct fat_h
{
    struct fat_header primary_header;
    union fat_h_e
    {
        struct fat_header_extended extended_header;
    } shared;
};

struct fat_directory_item
{
    uint8_t filename[8];
    uint8_t ext[3];
    uint8_t attribute;
    uint8_t reserved;
    uint8_t creation_time_tenths_of_a_sec;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access;
    uint16_t high_16_bits_first_cluster;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t low_16_bits_first_cluster;
    uint32_t filesize;
} __attribute__((packed));

struct fat_directory
{
    struct fat_directory_item* item;
    int total;
    int sector_pos;
    int ending_sector_pos;
};

// Directory or file
struct fat_item
{
    union 
    {
        struct fat_directory_item* item;
        struct fat_directory* directory;
    };
    
    FAT_ITEM_TYPE type;
};

struct fat_file_descriptor
{
    struct fat_item* item;
    uint32_t pos;
};

struct fat_private
{
    struct fat_h header;
    struct fat_directory root_directory;

    // Used to stream data clusters
    struct disk_stream* cluster_read_stream;
    // Used to stream the file allocation table
    struct disk_stream* fat_read_stream;


    // Used in situations where we stream the directory
    struct disk_stream* directory_stream;
};

int fat16_resolve(struct disk* disk);
void* fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode);

struct filesystem fat16_fs;


static void fat16_init_private(struct disk* disk, int id, struct fat_private* private)
{
    memset(private, 0, sizeof(struct fat_private));
    private->cluster_read_stream = disk_stream_new(id);
    private->fat_read_stream = disk_stream_new(id);
    private->directory_stream = disk_stream_new(id);
}


// This function READS from the disk until we read out all items
int fat16_get_total_items_for_directory(struct disk* disk, uint32_t directory_start_sector)
{
    struct fat_directory_item item;
    
    struct fat_private* fat_private = disk->fs_private;

    int res = 0;
    int directory_start_pos = directory_start_sector * disk->sector_size;
    struct disk_stream* stream = fat_private->directory_stream;
    if(disk_stream_seek(stream, directory_start_pos) != PEACHOS_OK)
    {
        return -EIO;
    }

    while(1)
    {
        if (disk_stream_read(stream, &item, sizeof(item)) != PEACHOS_OK)
        {
            return -EIO;
        }

        if (item.filename[0] == 0x00)
        {
            // We are done, this marks the end
            break;
        }

        // Is the item unused
        if (item.filename[0] == 0xE5)
        {
            continue;
        }

        res++;
    }

    return res;
}

int fat16_get_root_directory(struct disk* disk, struct fat_private* fat_private, struct fat_directory* directory)
{
    int res = PEACHOS_OK;
    struct fat_header* primary_header = &fat_private->header.primary_header;
    int root_dir_sector_pos = (primary_header->fat_copies * primary_header->sectors_per_fat) + primary_header->reserved_sectors;
    int root_dir_entries = fat_private->header.primary_header.root_dir_entries;
    int root_dir_size = (root_dir_entries * sizeof(struct fat_directory_item));
    int total_sectors = root_dir_size / disk->sector_size;
    if (root_dir_size % disk->sector_size)
    {
        total_sectors += 1;
    }

    int total_items = fat16_get_total_items_for_directory(disk, root_dir_sector_pos);

    struct fat_directory_item* dir = kzalloc(root_dir_size);
    if (!dir)
    {
        return -ENOMEM;
    }

    struct disk_stream* stream = fat_private->directory_stream;

    if (disk_stream_seek(stream,  root_dir_sector_pos * disk->sector_size) != PEACHOS_OK)
    {
        return -EIO;
    }

    if (disk_stream_read(stream, dir, root_dir_size) != PEACHOS_OK)
    {
        return -EIO;
    }

    directory->item = dir;
    directory->total = total_items;
    directory->sector_pos = root_dir_sector_pos;
    directory->ending_sector_pos = root_dir_sector_pos + (root_dir_size / disk->sector_size);
    return res;
}

int fat16_resolve(struct disk* disk)
{
    int res = PEACHOS_OK;
    struct fat_private* fat_private = kzalloc(sizeof(struct fat_private));
    fat16_init_private(disk, 0, fat_private); // HARDCODED TO ZERO
    disk->fs_private = fat_private;           // We're optmimistic, at the end this is the only FS we're going to deal with
    disk->filesystem = &fat16_fs;
    
    struct disk_stream* stream = disk_stream_new(0);
    if(!stream)
    {
        return -ENOMEM;
    }

    if (disk_stream_read(stream, &fat_private->header, sizeof(fat_private->header)) != PEACHOS_OK)
    {
        return -EIO;
    }

    // FAT specs page 10
    if (fat_private->header.shared.extended_header.signature != 0x29)
    {
        return -EFSNOTUS;
    }

    if (fat16_get_root_directory(disk, fat_private, &fat_private->root_directory) != PEACHOS_OK)
    {
        return -EIO;
    }

    if (stream)
    {
        disk_stream_close(stream);
    }

    if (res < 0)
    {
        kfree(fat_private);
        disk->fs_private = 0;
    }
    return res;
}

// The FAT16 uses spaces (0x20) to pads strings taking fewer bytes than the allotted space, here
// we normalize the given FAT16 string into a NULL terminated C string.
void fat16_name_to_string(char** out, const char* in)
{
    while(*in != 0x00 && *in != 0x20)
    {
        **out = *in;
        *out += 1;
        in +=1;
    }

    if (*in == 0x20)
    {
        **out = 0x00;
    }
}

// FAT16 splits the filename into a name plus its extenson, here we normalise a given entry into a
// C string where the dot separates a filename from its ext.
void fat16_get_normalized_filename(struct fat_directory_item* item, char* out, int max_len)
{
    memset(out, 0x00, max_len);
    char *out_tmp = out;
    fat16_name_to_string(&out_tmp, (const char*) item->filename);
    if (item->ext[0] != 0x00 && item->ext[0] != 0x20)
    {
        *out_tmp++ = '.';
        fat16_name_to_string(&out_tmp, (const char*) item->ext);
    }
}

static uint32_t fat16_get_first_cluster(struct fat_directory_item* item)
{
    return (item->high_16_bits_first_cluster) | item->low_16_bits_first_cluster;
};

static int fat16_cluster_to_sector(struct fat_private* private, int cluster)
{
    // Specs page 29. The cluster number is an offset from the beginning of the 'Data Region' which starts
    // from the second sector.
    return private->root_directory.ending_sector_pos + ((cluster - 2) * private->header.primary_header.sectors_per_cluster);
}

static uint32_t fat16_get_first_fat_sector(struct fat_private* private)
{
    // Specs page 6.
    return private->header.primary_header.reserved_sectors;
}

static int fat16_get_fat_entry(struct disk* disk, int cluster)
{
    int res = -1;
    struct fat_private* private = disk->fs_private;
    struct disk_stream* stream = private->fat_read_stream;
    if (!stream)
    {
        goto out;
    }

    uint32_t fat_table_position = fat16_get_first_fat_sector(private) * disk->sector_size;
    res = disk_stream_seek(stream, fat_table_position * (cluster * PEACHOS_FAT16_FAT_ENTRY_SIZE));
    if (res < 0)
    {
        goto out;
    }

    uint16_t result = 0;
    res = disk_stream_read(stream, &result, sizeof(result));
    if (res < 0)
    {
        goto out;
    }

    res = result;
out:
    return res;
}

static int fat16_get_cluster_for_offset(struct disk* disk, int starting_cluster, int offset)
{
    int res = 0;
    struct fat_private* private = disk->fs_private;
    int size_of_cluster_bytes = private->header.primary_header.sectors_per_cluster * disk->sector_size;
    int cluster_to_use = starting_cluster;
    int clusters_ahead = offset / size_of_cluster_bytes;
    for (int i = 0; i < clusters_ahead; i++)
    {
        int entry = fat16_get_fat_entry(disk, cluster_to_use);
        if (entry == 0xFF8 || entry == 0xFFF)
        {
            // We are at the last entry in the file
            res = -EIO;
            goto out;
        }

        // Sector is marked as bad?
        if (entry == PEACHOS_FAT16_BAD_SECTOR)
        {
            res = -EIO;
            goto out;
        }

        // Reserved sector?
        if (entry == 0xFF0 || entry == 0xFF6)
        {
            res = -EIO;
            goto out;
        }

        if (entry == 0x00)
        {
            res = -EIO;
            goto out;
        }

        cluster_to_use = entry;
    }

    res = cluster_to_use;
out:
    return res;
}
static int fat16_read_internal_from_stream(struct disk* disk, struct disk_stream* stream, int cluster, int offset, int total, void* out)
{
    int res = 0;
    struct fat_private* private = disk->fs_private;
    int size_of_cluster_bytes = private->header.primary_header.sectors_per_cluster * disk->sector_size;
    int cluster_to_use = fat16_get_cluster_for_offset(disk, cluster, offset);
    if (cluster_to_use < 0)
    {
        res = cluster_to_use;
        goto out;
    }


    int offset_from_cluster = offset % size_of_cluster_bytes;

    int starting_sector = fat16_cluster_to_sector(private, cluster_to_use);
    int starting_pos = (starting_sector * disk->sector_size) * offset_from_cluster;
    int total_to_read = total > size_of_cluster_bytes ? size_of_cluster_bytes : total;
    res = disk_stream_seek(stream, starting_pos);
    if (res != PEACHOS_OK)
    {
        goto out;
    }

    res = disk_stream_read(stream, out, total_to_read);
    if (res != PEACHOS_OK)
    {
        goto out;
    }

    total -= total_to_read;
    if (total > 0)
    {
        // We still have more to read
        res = fat16_read_internal_from_stream(disk, stream, cluster, offset+total_to_read, total, out + total_to_read);
    }

out:
    return res;
}

void fat16_free_directory(struct fat_directory* directory)
{
    if (!directory)
    {
        return;
    }

    if (directory->item)
    {
        kfree(directory->item);
    }

    kfree(directory);
}


void fat16_fat_item_free(struct fat_item* item)
{
    if (item->type == FAT_ITEM_TYPE_DIRECTORY)
    {
        fat16_free_directory(item->directory);
    }
    else if(item->type == FAT_ITEM_TYPE_FILE)
    {
        kfree(item->item);
    }

    kfree(item);
}

// We need to load an item from the disk, i.e. read a cluster from the disl
struct fat_directory* fat16_load_fat_directory(struct disk* disk, struct fat_directory_item* item)
{
    int res = 0;
    struct fat_directory* directory = 0;
    struct fat_private* fat_private = disk->fs_private;

    // We're meant to read directories
    if (!(item->attribute & FAT_FILE_SUBDIRECTORY))
    {
        res = -EINVARGS;
        goto out;
    }

    directory = kzalloc(sizeof(struct fat_directory));
    if (!directory)
    {
        res = -ENOMEM;
        goto out;
    }

    // Read cluster from the item struct
    int cluster = fat16_get_first_cluster(item); 

    // Translate that into sector num
    int cluster_sector = fat16_cluster_to_sector(fat_private, cluster); 

    // Read from the disk stream all items
    int total_items = fat16_get_total_items_for_directory(disk, cluster_sector); 

    directory->total = total_items;
    int directory_size = directory->total * sizeof(struct fat_directory_item);

    // Allocate array for dir's items
    directory->item = kzalloc(directory_size);
    if (!directory->item)
    {
        res = -ENOMEM;
        goto out;
    }

    // Read the whole dir. in mem.
    struct fat_private* fs_private = disk->fs_private;
    struct disk_stream* stream = fs_private->cluster_read_stream;
    res = fat16_read_internal_from_stream(disk, stream, cluster, 0x00, directory_size, directory->item);
    if (res != PEACHOS_OK)
    {
        goto out;
    }
    

out:
    if (res != PEACHOS_OK)
    {
        fat16_free_directory(directory);
    }
    return directory;
}

struct fat_item* fat16_new_fat_item_for_directory_item(struct disk* disk, struct fat_directory_item* item)
{
    struct fat_item* f_item = kzalloc(sizeof(struct fat_item));
    if (!f_item)
    {
        return 0;
    }

    // Something is not quite right here about the type
    if (item->attribute & FAT_FILE_SUBDIRECTORY)
    {
        f_item->directory = fat16_load_fat_directory(disk, item);
        f_item->type = FAT_ITEM_TYPE_DIRECTORY;
    }

    f_item->type = FAT_ITEM_TYPE_FILE;

    // Copy the item
    struct fat_directory_item* item_copy = 0;
    item_copy = kzalloc(sizeof(struct fat_directory_item));
    if (!item_copy)
    {
        return 0;
    }
    memcpy(item_copy, item, sizeof(item_copy));
    f_item->item = item_copy;
    return f_item;
}

struct fat_item* fat16_find_item_in_directory(struct disk* disk, struct fat_directory* directory, const char* name)
{
    struct fat_item* f_item = 0;
    char tmp_filename[PEACHOS_MAX_PATH];
    for (int i = 0; i < directory->total; i++)
    {
        fat16_get_normalized_filename(&directory->item[i], tmp_filename, sizeof(tmp_filename));
        if (istrncmp(tmp_filename, name, sizeof(tmp_filename)) == 0)
        {
            // Found it let's create load the dir/file metadata from the disk and store it in a struct
            f_item = fat16_new_fat_item_for_directory_item(disk, &directory->item[i]);
        }
    }

    return f_item;
}
struct fat_item* fat16_get_directory_entry(struct disk* disk, struct path_part* path)
{

    // Here we start from the root directory and seek the file
    struct fat_private* fat_private = disk->fs_private;
    struct fat_item* current_item = 0;
    struct fat_item* root_item 
            = fat16_find_item_in_directory(disk, &fat_private->root_directory, path->part);
    if (!root_item)
    {
        return 0;
    }

    // Keep going until we find the last leaf of the path, that is, the file
    struct path_part* next_part = path->next;
    current_item = root_item;
    while(next_part != 0)
    {
        if (current_item->type != FAT_ITEM_TYPE_DIRECTORY) // BUG ?
        {
            current_item = 0;
            break;
        }

        struct fat_item* tmp_item = fat16_find_item_in_directory(disk, current_item->directory, next_part->part);
        fat16_fat_item_free(current_item);
        current_item = tmp_item;
        next_part = next_part->next;
    }
    return current_item;
}


void* fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode)
{
    if (mode != FILE_MODE_READ)
    {
        return ERROR(-ERDONLY);
    }

    struct fat_file_descriptor* descriptor = 0;
    descriptor = kzalloc(sizeof(struct fat_file_descriptor));
    if (!descriptor)
    {
        return ERROR(-ENOMEM);
    }

    descriptor->item = fat16_get_directory_entry(disk, path);

    // We could not find the file
    if (!descriptor->item)
    {
        return ERROR(-EIO);
    }

    descriptor->pos = 0;
    return descriptor;
}

struct filesystem* fat16_init()
{
    fat16_fs.resolve = fat16_resolve;
    fat16_fs.open = fat16_open;
    strcpy(fat16_fs.name, "FAT16");
    return &fat16_fs;
}