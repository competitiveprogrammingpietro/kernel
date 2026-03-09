#ifndef GDT_H
#define GDT_H
#include <stdint.h>

// This is an entry to the GDT as descibed in
struct gdt
{
    uint16_t segment;
    uint16_t base_first;
    uint8_t base;
    uint8_t access;
    uint8_t high_flags;
    uint8_t base_24_31_bits;
} __attribute__((packed));

// This is our internal representation of the GDT entry, which we're going to
// translate into a GDT entry as x86 commands.
struct gdt_internal
{
    uint32_t base;
    uint32_t limit;
    uint8_t type;
};

void gdt_load(struct gdt *gdt, int size);
void gdt_internal_to_gdt(struct gdt *gdt, struct gdt_internal *structured_gdt, int total_entires);
#endif