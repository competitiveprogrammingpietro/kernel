# kernel. To be executed with:
qemu-system-x86_64 -hda boot.bin

# Debug
gdb target remote | qemu-system-x86_64 -hda boot.bin -S -gdb stdio

# Disassemble
ndisasm program

# Compile the compiler - instructions from:

https://wiki.osdev.org/GCC_Cross-Compiler


