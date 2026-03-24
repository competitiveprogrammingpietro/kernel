#include "idt.h"
#include "task/task.h"
#include "config.h"
#include "memory/memory.h"
#include "kernel.h"
#include "io/io.h"

struct idt_desc idt_descriptors[PEACOS_INTERRUPT_NUMBER];
struct idtr_desc idtr_descriptor;

static ISR80H_COMMAND_HANDLER isr80h_commands[PEACHOS_MAX_ISR80H_COMMANDS];

extern void idt_load(void *);
extern void int21h();
extern void no_interrupt();

void idt_set(int int_num, void *address)
{
	struct idt_desc *idtd = &idt_descriptors[int_num];
	idtd->offset_1 = (uint32_t)address & 0x0000ffff;
	idtd->selector = KERNEL_CODE_SELECTOR;
	idtd->zero = 0x0;
	idtd->type_attr = 0xee;
	idtd->offset_2 = (uint32_t)address >> 16;
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

void idt_zero()
{
	write_string("Divide by zero error");
}

void isr80h_register_command(int command_id, ISR80H_COMMAND_HANDLER command)
{
	if (command_id < 0 || command_id >= PEACHOS_MAX_ISR80H_COMMANDS)
	{
		panic("The command is out of bounds\n");
	}

	if (isr80h_commands[command_id])
	{
		panic("Your attempting to overwrite an existing command\n");
	}

	isr80h_commands[command_id] = command;

	int i = 1;
	command_id = i;
}

// This is the handler called by the assembly section concerning the 80H ISR
void *int80_handler(int command, struct interrupt_frame *iframe)
{
	void *res = 0;
	kernel_context();
	task_save_state(task_current(), iframe);
	// Handle the command

	if (!isr80h_commands[command])
	{
		panic("No handler registered for command\n");
	}
	res = isr80h_commands[command](iframe);

	// Restore the task
	task_context(task_current());
	return res;
}

void idt_init()
{
	memset(idt_descriptors, 0, sizeof(idt_descriptors));
	idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
	idtr_descriptor.base = (uint32_t)idt_descriptors;

	// If by the time this code runs an interrupt is raised
	// the CPU will reset
	for (int i = 0; i < PEACOS_INTERRUPT_NUMBER; i++)
	{
		idt_set(i, no_interrupt);
	}

	idt_set(0, idt_zero);
	idt_set(0x21, int21_handler);
	idt_set(0x80, int80_handler);
	// Load the IDTR up
	idt_load(&idtr_descriptor);
}
