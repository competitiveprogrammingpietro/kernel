#include "gdt.h"
#include "kernel.h"

// this function takes our somewhat simpler GDT entry representation and fills
// up a GDT entry x86 as described https://wiki.osdev.org/Global_Descriptor_Table#Segment_Descriptor
void gdt_internal_entry_to_gdt_entry(uint8_t *target, struct gdt_internal source)
{

    // Our internal representation uses 32 bits for the limit, while the GDT
    // scatters it in 16 bits, target[0/1] and the least significant four bits
    // of target[0]. We always fall into this case
    target[6] = 0x40; // FLAGS: 0100 GDLR D = 1 32 bits selectors
    if (source.limit > 65535)
    {
        source.limit = source.limit >> 12;
        target[6] = 0xC0; // FLAGS 1100 G = 1 as well.  Page granularity means that the limit is expressed in form of 4K blocks
    }

    // Encodes the limit
    target[0] = source.limit & 0xFF;
    target[1] = (source.limit >> 8) & 0xFF;
    target[6] |= (source.limit >> 16) & 0x0F;

    // Encode the base
    target[2] = source.base & 0xFF;
    target[3] = (source.base >> 8) & 0xFF;
    target[4] = (source.base >> 16) & 0xFF;
    target[7] = (source.base >> 24) & 0xFF;

    // Set the access byte
    target[5] = source.type;
}

void gdt_internal_to_gdt(struct gdt *gdt, struct gdt_internal *gdt_internal, int total_entires)
{
    for (int i = 0; i < total_entires; i++)
    {
        gdt_internal_entry_to_gdt_entry((uint8_t *)&gdt[i], gdt_internal[i]);
    }
}