#include "streamer.h"
#include "disk.h"
#include "memory/heap/kheap.h"
#include "config.h"

struct disk_stream* disk_stream_new(int disk_id)
{
	struct disk* disk = disk_get(disk_id);
	if (!disk)
	{
		return 0;
	}

	struct disk_stream* stream = kzalloc(sizeof(struct disk_stream));
	stream->pos = 0;
	stream->disk = disk;
	return stream;
}



int disk_stream_seek(struct disk_stream* stream, int pos)
{
	stream->pos = pos;
	return 0;
}



int disk_stream_read(struct disk_stream* stream, void * out, int total)
{
	int disk_sector = stream->pos / PEACHOS_SECTOR_SIZE;
	int offset = stream->pos % PEACHOS_SECTOR_SIZE;

	char buf[PEACHOS_SECTOR_SIZE];

	// We read one block
	int res = disk_read_block(stream->disk, disk_sector, 1, buf);
	
	if (res < 0)
	{
		return res;
	}

	// Cap it to the sector size as if more than one reads is needed
	// that is going to come from a recursive call
	int total_limit = total > PEACHOS_SECTOR_SIZE ? PEACHOS_SECTOR_SIZE : total;
	
	// Copy the result
	for (int i = 0; i < total_limit; i++)
	{
		*(char*)out = buf[offset + i];
		out++;
	}
	
	stream->pos += total_limit;
	
	if (total <= PEACHOS_SECTOR_SIZE)
	{
		return 0;
	}

	// Need to read additioal data, make a recursive call. This is dangerous as we
	// might crash over the stack's mem however being a simple implementation it
	// is good enough.
	return disk_stream_read(stream, out, total - PEACHOS_SECTOR_SIZE);
}

void disk_stream_free(struct disk_stream* stream)
{
	kfree(stream);
}
