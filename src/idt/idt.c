#include "idt.h"
#include "config.h"
#include "memory/memory.h"
#include "kernel.h"
#include "io/io.h"

struct idt_desc idt_descriptors[PEACOS_INTERRUPT_NUMBER];
struct idtr_desc idtr_descriptor;

extern void idt_load(void*);
extern void int21h();
extern void no_interrupt();

void idt_set(int int_num, void * address)
{
	struct idt_desc* idtd = &idt_descriptors[int_num];
	idtd->offset_1 = (uint32_t) address & 0x0000ffff;
	idtd->selector = KERNEL_CODE_SELECTOR;
	idtd->zero = 0x0; 
	idtd->type_attr = 0xee;
	idtd->offset_2 = (uint32_t) address >> 16;
}

void no_interrupt_handler()
{
//	write_string("No interrupt handler defined\n");
	outb(0x20, 0x20);
}

void int21_handler()
{
	write_string("Keyboard interrupt\n");
	outb(0x20, 0x20);
}

void idt_zero() {
	write_string("Divide by zero error");
}



void idt_init()
{
	memset(idt_descriptors, 0, sizeof(idt_descriptors));
	idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
	idtr_descriptor.base = (uint32_t) idt_descriptors;
	
	// If by the time this code runs an interrupt is raised
	// the CPU will reset
	for (int i = 0; i < PEACOS_INTERRUPT_NUMBER; i++) {
		idt_set(i, no_interrupt); 
	}

	idt_set(0, idt_zero);
	idt_set(0x21, int21_handler);

	// Load the IDTR up
	idt_load(&idtr_descriptor);
}
