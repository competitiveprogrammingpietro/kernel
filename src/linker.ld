ENTRY(_start)
OUTPUT_FORMAT(binary)
SECTIONS
{
	
	/* Our kernel will be loaded at address 0x100_000. This is the origin as far as the linker is concerned */
	. = 1M; 

	.text : ALIGN(4096)
	{
		*(.text)
	}

	.rodata :ALIGN(4096)
	{
		*(.rodata)
	}

	.data : ALIGN(4096)
	{
		*(.data)
	}

	.bss : ALIGN(4096)
	{
		*(COMMON)
		*(.bss)
	}

	.asm : ALIGN(4096)
	{
		*(.asm)
	}
}

