#include "gdt.h"
#include "kernel.h"

// this function takes our somewhat simpler GDT entry representation and fills
// up a GDT entry x86 as described https://wiki.osdev.org/Global_Descriptor_Table#Segment_Descriptor
void gdt_internal_entry_to_gdt_entry(uint8_t *target, struct gdt_internal source)
{
    // This is the last part of the limit and it has been explaied at all by
    // the lecturer ..... !
    // Guesswork: limit is a 20 bits splits in two pieces, 16 and 4, if our
    // internal structure holds a number greater than what 16 bits would fit
    // into, we need to use the remaining 4 bits addressed by target[6]
    // 16 bits = 65535 ..... would not you write source.limit & 0xffff0000 != 0 ?
    target[6] = 0x40;
    if (source.limit > 65535)
    {
        source.limit = source.limit >> 12;
        target[6] = 0xC0;
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