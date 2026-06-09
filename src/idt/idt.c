#include "idt.h"
#include "task/task.h"
#include "config.h"
#include "status.h"
#include "memory/memory.h"
#include "task/process.h"
#include "kernel.h"
#include "io/io.h"

// Those handlers are defined in assembly using a NASM macro, all they do
// is to call the idt_interrupt_handler() feeding to it the interrupt
// number
extern void *asm_interrupt_pointer_table[PEACHOS_INTERRUPT_NUMBER];

struct idt_desc idt_descriptors[PEACHOS_INTERRUPT_NUMBER];
struct idtr_desc idtr_descriptor;

static ISR80H_COMMAND_HANDLER isr80h_commands[PEACHOS_MAX_ISR80H_COMMANDS];
static INTERRUPT_HANDLER idt_interrupt_handlers[PEACHOS_INTERRUPT_NUMBER];

extern void idt_load(void *);
extern void int80h();
extern void no_interrupt();

// This is the main interrupt handler for our system
void idt_interrupt_handler(int interrupt, struct interrupt_frame *frame)
{
	void *ptr = idt_interrupt_handlers[interrupt];
	ptr = ptr;
	kernel_context();
	if (idt_interrupt_handlers[interrupt] != 0)
	{

		// There are two main cases for this:
		// 1. Interrupt such as print, the task's registers are saved however
		//    those registers are going to be restored by the assembly code
		//	  in idt.asm which constitutes the entry point for all interrupt
		//    handlers.
		// 2. Interrupt clock, hence task switching, in this case the interrupt
		//    handler called in our vector of function 'idt_interrupt_handler'
		//    does not return, hence it is important to save the registers so
		//    that they can be restored later on at the next task switching
		task_save_state(task_current(), frame);
		idt_interrupt_handlers[interrupt](frame);
	}

	// Restore the task's paging directory so that execution can resume
	task_context(task_current());
	outb(0x20, 0x20);
}

void idt_clock()
{
	outb(0x20, 0x20);

	// get the next task and execute it
	struct task *t = task_switch_next();
	if (!t)
	{
		panic("There are no currently tasks to run\n");
	}
	task_execute_current();
}

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
}

// this is the handler called by the assembly section concerning the 0x80
// interrupt, this does not pass through the general handler
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

// this function registers the interrupt_callback to the vector of functions
// which is used by the int80_handler, which is main entry point for all the
// interrupts
int idt_register_interrupt(int interrupt,
						   INTERRUPT_HANDLER interrupt_callback)
{
	if (interrupt < 0 || interrupt >= PEACHOS_INTERRUPT_NUMBER)
	{
		return -EINVARGS;
	}

	idt_interrupt_handlers[interrupt] = interrupt_callback;
	return 0;
}

// this function handles the exceptions the CPU raises as specified by
// https://wiki.osdev.org/Exceptions. We do not differentiate among
// them and we merely terminate the process
void idt_handle_exception()
{
	print("Terminating process due to CPU exception\n");

	// terminate the process and start executing the next one
	process_terminate(task_current()->process);
	struct task *t = task_switch_next();
	if (!t)
	{
		panic("idt_handle_exception(): no task to execute");
	}
	task_execute_current();
}


void idt_init()
{
	memset(idt_descriptors, 0, sizeof(idt_descriptors));
	idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
	idtr_descriptor.base = (uint32_t)idt_descriptors;

	// If by the time this code runs an interrupt is raised
	// the CPU will reset
	for (int i = 0; i < PEACHOS_INTERRUPT_NUMBER; i++)
	{

		// All interrupts point to their 'numbered' handler defined in asm
		// which in turn forward the call to the int80_handler
		idt_set(i, asm_interrupt_pointer_table[i]);
	}

	idt_set(0, idt_zero); // old example could be removed

	// set the main entry point to the int80h assembly routine
	// which sets up the interrupt frame and calls the int80_handler
	// C function
	idt_set(0x80, int80h);

	// Handle all the exceptions
	for (int i = 0; i < 0x20; i++)
	{
		idt_register_interrupt(i, idt_handle_exception);
	}

	idt_register_interrupt(0x20, idt_clock);

	// Load the IDTD up
	idt_load(&idtr_descriptor);
}
