#include "fs.h"
#include "config.h"
#include "memory/memory.h"
#include "string/string.h"
#include "disk/disk.h"
#include "memory/heap/kheap.h"
#include "status.h"
#include "kernel.h"
#include "fat/fat16.h"

// Globals for the number of FS and FDs
struct filesystem* 	filesystems[PEACHOS_MAX_FILESYSTEMS];
unsigned int filesystems_index;
struct file_descriptor* file_descriptors[PEACHOS_MAX_FILE_DESCRIPTORS];
unsigned int filedescriptors_index;

void fs_insert_filesystem(struct filesystem* filesystem)
{
	if (filesystems_index == PEACHOS_MAX_FILESYSTEMS)
	{
		write_string("Panic: cannot create new fs");
		while (1) {}
	}

	filesystems[filesystems_index] = filesystem;
	filesystems_index++;
}

void fs_init()
{
	memset(filesystems, 0, sizeof(filesystems));
	memset(file_descriptors, 0, sizeof(file_descriptors));
	filesystems_index = filedescriptors_index = 0;

	fs_insert_filesystem(fat16_init());
}


static int file_new_descriptor(struct file_descriptor** desc_out)
{
	if (filedescriptors_index == PEACHOS_MAX_FILE_DESCRIPTORS)
	{

		return -ENOMEM;
	}

	// Allocate a new one
    struct file_descriptor* desc = kzalloc(sizeof(struct file_descriptor));

    // Descriptors start at 1
    desc->index = filedescriptors_index + 1;
    file_descriptors[filedescriptors_index] = desc;
    *desc_out = desc;
	filedescriptors_index++;
    return 0;
}

/*
static struct file_descriptor* file_get_descriptor(int fd)
{
    if (fd <= 0 || fd >= PEACHOS_MAX_FILE_DESCRIPTORS)
    {
        return 0;
    }

    // Descriptors start at 1
    return file_descriptors[fd - 1];
}
	*/

struct filesystem* fs_resolve(struct disk* disk)
{
	// We basically try them all until one resolves
	struct filesystem* fs = 0;
	for (int i = 0; i < PEACHOS_MAX_FILESYSTEMS; i++)
	{
		if (filesystems[i] != 0 && filesystems[i]->resolve(disk) == 0)
		{
			fs = filesystems[i];
			break;
		}
	}
	return fs;
}

FILE_MODE file_get_mode_by_string(const char* str)
{
    if (strncmp(str, "r", 1) == 0)
    {
        return FILE_MODE_READ;
    }
    else if(strncmp(str, "w", 1) == 0)
    {
        return FILE_MODE_WRITE;
    }
    else if(strncmp(str, "a", 1) == 0)
    {
        return FILE_MODE_APPEND;
    }
    return FILE_MODE_INVALID;
}



int fopen(const char* filename, const char* mode)
{
    int res = 0;
    struct path_root* root_path = pathparser_parse(filename, NULL);
    if (!root_path)
    {
        return -EINVARGS;
    }

    // We cannot have just a root path 0:/
    if (!root_path->first)
    {
		return -EINVARGS;
    }

    // Ensure the disk we are reading from exists
    struct disk* disk = disk_get(root_path->drive_no);
    if (!disk)
    {
        return -EIO;
    }

	
    if (!disk->filesystem)
    {
        res = -EIO;
    }

    FILE_MODE file_mode = file_get_mode_by_string(mode);
    if (file_mode == FILE_MODE_INVALID)
    {
        return -EINVARGS;
    }

    void* descriptor_private_data = disk->filesystem->open(disk, root_path->first, file_mode);
    if (ISERR(descriptor_private_data))
    {
        res = ERROR_I(descriptor_private_data);
    }

    struct file_descriptor* desc = 0;
    res = file_new_descriptor(&desc);
    if (res < 0)
    {
        return res;
    }
    desc->filesystem = disk->filesystem;
    desc->private = descriptor_private_data;
    desc->disk = disk;
    res = desc->index;
    return res;
}
