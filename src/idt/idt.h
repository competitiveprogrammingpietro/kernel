#ifndef IDT_H
#define IDT_H
#include <stdint.h>
// https://wiki.osdev.org/Interrupt_Descriptor_Table
struct idt_desc 
{
	uint16_t offset_1;
	uint16_t selector;
	uint8_t zero;
	uint8_t type_attr;
	uint16_t offset_2;
} __attribute__((packed));

struct idtr_desc 
{
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));

// This struct differs slightly from the task's register one - the real difference
// are the reserved bits however it is good to keep them in different compilation
// units as they are involved in a different scenarios. This is the state of the
// CPU when an interrupt was executed, as the ISR sees it.
struct interrupt_frame
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t reserved;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t esp;
    uint32_t ss;
} __attribute__((packed));

void idt_init();
extern void enable_interrupts();
extern void disable_interrupts();
# endif
