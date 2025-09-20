ENTRY(_start)
OUTPUT_FORMAT(binary)
SECTIONS
{
	
	/* Our kernel will be loaded at address 0x100_000. This is the origin as far as the linker is concerned */
	. = 1M; 

	.text :
	{
		*(.text)
	}

	.rodata :
	{
		*(.rodata)
	}

	.data : 
	{
		*(.data)
	}

	.bss : 
	{
		*(COMMON)
		*(.bss)
	}
}

